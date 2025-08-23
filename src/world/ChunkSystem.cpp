#include "ChunkSystem.hpp"
#include "Chunk.hpp"
#include "TerrainGenerator.hpp"
#include "Biome.hpp"
#include "Block.hpp"
#include "World.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <zlib.h>
#include <memory>

namespace VoxelCraft {

	ChunkSystem::ChunkSystem(const ChunkSystemConfig& config)
		: m_config(config)
		, m_world(nullptr)
		, m_initialized(false)
		, m_generating(true)
		, m_saving(true)
		, m_playerChunk(0, 0)
	{
		// Initialize statistics
		std::memset(&m_stats, 0, sizeof(ChunkSystemStats));

		VOXELCRAFT_LOG_INFO("ChunkSystem initialized with config: maxChunks={}, renderDistance={}",
			config.maxLoadedChunks, config.renderDistance);
	}

	ChunkSystem::~ChunkSystem()
	{
		Shutdown();
	}

	bool ChunkSystem::Initialize(World* world)
	{
		if (m_initialized) {
			VOXELCRAFT_LOG_WARN("ChunkSystem already initialized");
			return true;
		}

		m_world = world;

		// Initialize terrain generator
		m_terrainGenerator = std::make_shared<TerrainGenerator>();

		// Start worker threads
		if (m_config.enableMultithreading) {
			m_generating = true;
			for (uint32_t i = 0; i < m_config.generationThreads; ++i) {
				m_generationThreads.emplace_back(&ChunkSystem::GenerationThreadFunction, this);
			}

			m_saving = true;
			m_saveThread = std::thread(&ChunkSystem::SaveThreadFunction, this);
		}

		// Initialize timing
		m_lastCleanupTime = std::chrono::steady_clock::now();
		m_lastStatsUpdate = std::chrono::steady_clock::now();

		m_initialized = true;
		VOXELCRAFT_LOG_INFO("ChunkSystem initialized successfully");
		return true;
	}

	void ChunkSystem::Shutdown()
	{
		if (!m_initialized) {
			return;
		}

		VOXELCRAFT_LOG_INFO("Shutting down ChunkSystem...");

		// Stop worker threads
		m_generating = false;
		m_saving = false;

		{
			std::unique_lock<std::mutex> lock(m_generationMutex);
			m_generationCV.notify_all();
		}

		{
			std::unique_lock<std::mutex> lock(m_saveMutex);
			m_saveCV.notify_all();
		}

		for (auto& thread : m_generationThreads) {
			if (thread.joinable()) {
				thread.join();
			}
		}
		m_generationThreads.clear();

		if (m_saveThread.joinable()) {
			m_saveThread.join();
		}

		// Save all chunks
		SaveAllChunks();

		// Clear all data
		m_chunks.clear();
		m_compressedChunks.clear();
		m_world = nullptr;
		m_initialized = false;

		VOXELCRAFT_LOG_INFO("ChunkSystem shutdown complete");
	}

	void ChunkSystem::Update(float deltaTime, const ChunkCoord& playerChunk)
	{
		if (!m_initialized) {
			return;
		}

		// Update player position
		if (playerChunk != m_playerChunk) {
			m_playerChunk = playerChunk;
			UpdateVisibleChunks();
		}

		// Update chunk priorities
		UpdateChunkPriorities();

		// Process generation queue
		ProcessGenerationQueue();

		// Process save queue
		ProcessSaveQueue();

		// Cleanup unused chunks
		auto now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::seconds>(now - m_lastCleanupTime).count() >= m_config.cleanupInterval) {
			CleanupChunks();
			m_lastCleanupTime = now;
		}

		// Update statistics
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastStatsUpdate).count() >= 1000) {
			UpdateStats();
			m_lastStatsUpdate = now;
		}
	}

	void ChunkSystem::Render()
	{
		if (!m_initialized) {
			return;
		}

		std::unique_lock<std::mutex> lock(m_chunkMutex);

		for (auto& pair : m_chunks) {
			auto& chunk = pair.second;
			if (chunk && chunk->GetState() == ChunkState::READY) {
				// Check if chunk is visible and render it
				float distance = m_playerChunk.distance(pair.first);
				if (distance <= m_config.renderDistance) {
					chunk->Render();
					m_stats.renderedChunks++;
				}
			}
		}
	}

	std::shared_ptr<Chunk> ChunkSystem::GetChunk(const ChunkCoord& coord)
	{
		std::unique_lock<std::mutex> lock(m_chunkMutex);

		auto it = m_chunks.find(coord);
		if (it != m_chunks.end()) {
			return it->second;
		}

		return nullptr;
	}

	bool ChunkSystem::HasChunk(const ChunkCoord& coord) const
	{
		std::unique_lock<std::mutex> lock(const_cast<std::mutex&>(m_chunkMutex));
		return m_chunks.find(coord) != m_chunks.end();
	}

	void ChunkSystem::LoadChunkAsync(const ChunkCoord& coord, ChunkPriority priority,
		std::function<void(std::shared_ptr<Chunk>)> callback)
	{
		if (!m_initialized) {
			return;
		}

		ChunkRequest request;
		request.coord = coord;
		request.priority = priority;
		request.timestamp = std::chrono::steady_clock::now();
		request.callback = callback;

		{
			std::unique_lock<std::mutex> lock(m_generationMutex);
			m_generationQueue.push(request);
		}

		m_generationCV.notify_one();
	}

	void ChunkSystem::UnloadChunk(const ChunkCoord& coord)
	{
		std::unique_lock<std::mutex> lock(m_chunkMutex);

		auto it = m_chunks.find(coord);
		if (it != m_chunks.end()) {
			auto& chunk = it->second;
			if (chunk) {
				chunk->SetState(ChunkState::UNLOADING);

				// Save chunk before unloading if modified
				if (chunk->IsModified()) {
					SaveChunk(coord);
				}

				// Compress and cache chunk
				if (m_config.enableCompression) {
					auto compressed = CompressChunk(chunk);
					if (compressed.size() < m_config.chunkCacheSize) {
						m_compressedChunks[coord] = compressed;
					}
				}
			}

			m_chunks.erase(it);
		}
	}

	void ChunkSystem::SaveChunk(const ChunkCoord& coord)
	{
		auto chunk = GetChunk(coord);
		if (!chunk || !chunk->IsModified()) {
			return;
		}

		{
			std::unique_lock<std::mutex> lock(m_saveMutex);
			m_saveQueue.push(coord);
		}

		m_saveCV.notify_one();
	}

	void ChunkSystem::SaveAllChunks()
	{
		VOXELCRAFT_LOG_INFO("Saving all chunks...");

		std::unique_lock<std::mutex> lock(m_chunkMutex);

		for (auto& pair : m_chunks) {
			auto& chunk = pair.second;
			if (chunk && chunk->IsModified()) {
				auto compressed = CompressChunk(chunk);
				SaveChunkToDisk(pair.first, compressed);
				chunk->SetModified(false);
			}
		}

		VOXELCRAFT_LOG_INFO("All chunks saved");
	}

	std::shared_ptr<Block> ChunkSystem::GetBlock(const WorldCoord& coord)
	{
		auto chunkCoord = coord.toChunkCoord();
		auto chunk = GetChunk(chunkCoord);

		if (!chunk) {
			return nullptr;
		}

		auto blockCoord = coord.toBlockCoord();
		return chunk->GetBlock(blockCoord.x, blockCoord.y, blockCoord.z);
	}

	void ChunkSystem::SetBlock(const WorldCoord& coord, std::shared_ptr<Block> block)
	{
		auto chunkCoord = coord.toChunkCoord();
		auto chunk = GetChunk(chunkCoord);

		if (!chunk) {
			// Create chunk if it doesn't exist
			chunk = ForceGenerateChunk(chunkCoord);
			if (!chunk) {
				return;
			}
		}

		auto blockCoord = coord.toBlockCoord();
		chunk->SetBlock(blockCoord.x, blockCoord.y, blockCoord.z, block);
		chunk->SetModified(true);

		// Update neighboring chunks if on boundary
		if (blockCoord.x == 0) {
			auto neighbor = GetChunk(ChunkCoord(chunkCoord.x - 1, chunkCoord.z));
			if (neighbor) neighbor->SetModified(true);
		} else if (blockCoord.x == 15) {
			auto neighbor = GetChunk(ChunkCoord(chunkCoord.x + 1, chunkCoord.z));
			if (neighbor) neighbor->SetModified(true);
		}

		if (blockCoord.z == 0) {
			auto neighbor = GetChunk(ChunkCoord(chunkCoord.x, chunkCoord.z - 1));
			if (neighbor) neighbor->SetModified(true);
		} else if (blockCoord.z == 15) {
			auto neighbor = GetChunk(ChunkCoord(chunkCoord.x, chunkCoord.z + 1));
			if (neighbor) neighbor->SetModified(true);
		}
	}

	std::shared_ptr<Biome> ChunkSystem::GetBiome(const WorldCoord& coord)
	{
		if (!m_terrainGenerator) {
			return nullptr;
		}

		return m_terrainGenerator->GetBiome(coord.x, coord.z);
	}

	void ChunkSystem::SetConfig(const ChunkSystemConfig& config)
	{
		m_config = config;
		VOXELCRAFT_LOG_INFO("ChunkSystem configuration updated");
	}

	std::vector<std::shared_ptr<Chunk>> ChunkSystem::GetAllChunks() const
	{
		std::unique_lock<std::mutex> lock(const_cast<std::mutex&>(m_chunkMutex));

		std::vector<std::shared_ptr<Chunk>> chunks;
		chunks.reserve(m_chunks.size());

		for (auto& pair : m_chunks) {
			chunks.push_back(pair.second);
		}

		return chunks;
	}

	std::vector<std::shared_ptr<Chunk>> ChunkSystem::GetChunksInRadius(const ChunkCoord& center, uint32_t radius)
	{
		std::vector<std::shared_ptr<Chunk>> result;

		for (int32_t x = center.x - radius; x <= center.x + radius; ++x) {
			for (int32_t z = center.z - radius; z <= center.z + radius; ++z) {
				ChunkCoord coord(x, z);
				if (IsChunkInRange(coord, center, radius)) {
					auto chunk = GetChunk(coord);
					if (chunk) {
						result.push_back(chunk);
					}
				}
			}
		}

		return result;
	}

	std::vector<std::shared_ptr<Chunk>> ChunkSystem::GetVisibleChunks() const
	{
		std::unique_lock<std::mutex> lock(const_cast<std::mutex&>(m_chunkMutex));

		std::vector<std::shared_ptr<Chunk>> visible;

		for (auto& pair : m_chunks) {
			auto& chunk = pair.second;
			if (chunk && chunk->GetState() == ChunkState::READY) {
				float distance = m_playerChunk.distance(pair.first);
				if (distance <= m_config.renderDistance) {
					visible.push_back(chunk);
				}
			}
		}

		return visible;
	}

	std::shared_ptr<Chunk> ChunkSystem::ForceGenerateChunk(const ChunkCoord& coord)
	{
		// Check if already exists
		auto existing = GetChunk(coord);
		if (existing) {
			return existing;
		}

		// Generate new chunk
		auto chunk = GenerateChunk(coord);
		if (chunk) {
			std::unique_lock<std::mutex> lock(m_chunkMutex);
			m_chunks[coord] = chunk;
		}

		return chunk;
	}

	void ChunkSystem::ClearCache()
	{
		std::unique_lock<std::mutex> lock(m_chunkMutex);
		m_compressedChunks.clear();
		VOXELCRAFT_LOG_INFO("Chunk cache cleared");
	}

	size_t ChunkSystem::GetMemoryUsage() const
	{
		std::unique_lock<std::mutex> lock(const_cast<std::mutex&>(m_chunkMutex));

		size_t memory = 0;

		// Calculate memory used by loaded chunks
		for (auto& pair : m_chunks) {
			if (pair.second) {
				memory += pair.second->GetMemoryUsage();
			}
		}

		// Add compressed chunks memory
		for (auto& pair : m_compressedChunks) {
			memory += pair.second.size();
		}

		return memory;
	}

	std::shared_ptr<Chunk> ChunkSystem::GenerateChunk(const ChunkCoord& coord)
	{
		auto startTime = std::chrono::steady_clock::now();

		try {
			// Create new chunk
			auto chunk = std::make_shared<Chunk>(coord);
			chunk->SetState(ChunkState::GENERATING);

			// Generate terrain
			if (m_terrainGenerator) {
				m_terrainGenerator->GenerateChunk(chunk);
			}

			chunk->SetState(ChunkState::READY);

			auto endTime = std::chrono::steady_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

			VOXELCRAFT_LOG_DEBUG("Generated chunk ({}, {}) in {}ms", coord.x, coord.z, duration.count());

			return chunk;
		}
		catch (const std::exception& e) {
			VOXELCRAFT_LOG_ERROR("Failed to generate chunk ({}, {}): {}", coord.x, coord.z, e.what());
			return nullptr;
		}
	}

	std::shared_ptr<Chunk> ChunkSystem::LoadChunk(const ChunkCoord& coord)
	{
		auto startTime = std::chrono::steady_clock::now();

		try {
			// Check compressed cache first
			{
				std::unique_lock<std::mutex> lock(m_chunkMutex);
				auto it = m_compressedChunks.find(coord);
				if (it != m_compressedChunks.end()) {
					auto chunk = DecompressChunk(coord, it->second);
					if (chunk) {
						m_compressedChunks.erase(it); // Remove from cache
						auto endTime = std::chrono::steady_clock::now();
						auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
						VOXELCRAFT_LOG_DEBUG("Loaded chunk ({}, {}) from cache in {}ms", coord.x, coord.z, duration.count());
						return chunk;
					}
				}
			}

			// Load from disk
			auto filePath = GetChunkFilePath(coord);
			if (std::filesystem::exists(filePath)) {
				std::ifstream file(filePath, std::ios::binary);
				if (file) {
					std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
					auto chunk = DecompressChunk(coord, data);
					if (chunk) {
						auto endTime = std::chrono::steady_clock::now();
						auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
						VOXELCRAFT_LOG_DEBUG("Loaded chunk ({}, {}) from disk in {}ms", coord.x, coord.z, duration.count());
						return chunk;
					}
				}
			}

			// Generate new chunk if not found
			return GenerateChunk(coord);
		}
		catch (const std::exception& e) {
			VOXELCRAFT_LOG_ERROR("Failed to load chunk ({}, {}): {}", coord.x, coord.z, e.what());
			return nullptr;
		}
	}

	void ChunkSystem::SaveChunkToDisk(const ChunkCoord& coord, const std::vector<uint8_t>& data)
	{
		try {
			auto filePath = GetChunkFilePath(coord);

			// Create directory if it doesn't exist
			std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());

			std::ofstream file(filePath, std::ios::binary);
			if (file) {
				file.write(reinterpret_cast<const char*>(data.data()), data.size());
				VOXELCRAFT_LOG_DEBUG("Saved chunk ({}, {}) to disk", coord.x, coord.z);
			} else {
				VOXELCRAFT_LOG_ERROR("Failed to save chunk ({}, {}) to disk", coord.x, coord.z);
			}
		}
		catch (const std::exception& e) {
			VOXELCRAFT_LOG_ERROR("Failed to save chunk ({}, {}): {}", coord.x, coord.z, e.what());
		}
	}

	std::vector<uint8_t> ChunkSystem::CompressChunk(const std::shared_ptr<Chunk>& chunk)
	{
		if (!m_config.enableCompression) {
			// Return raw data if compression is disabled
			return chunk->Serialize();
		}

		auto rawData = chunk->Serialize();
		if (rawData.empty()) {
			return {};
		}

		uLongf compressedSize = compressBound(rawData.size());
		std::vector<uint8_t> compressed(compressedSize);

		int result = compress2(compressed.data(), &compressedSize,
			rawData.data(), rawData.size(), Z_BEST_COMPRESSION);

		if (result != Z_OK) {
			VOXELCRAFT_LOG_ERROR("Failed to compress chunk ({}, {})", chunk->GetCoord().x, chunk->GetCoord().z);
			return rawData; // Return uncompressed data on failure
		}

		compressed.resize(compressedSize);
		return compressed;
	}

	std::shared_ptr<Chunk> ChunkSystem::DecompressChunk(const ChunkCoord& coord, const std::vector<uint8_t>& data)
	{
		if (!m_config.enableCompression) {
			// Data is not compressed
			auto chunk = std::make_shared<Chunk>(coord);
			if (chunk->Deserialize(data)) {
				return chunk;
			}
			return nullptr;
		}

		// Estimate decompressed size (chunk volume * sizeof(Block) + header)
		uLongf decompressedSize = ChunkSystemConfig::CHUNK_VOLUME * sizeof(uint16_t) + 1024;
		std::vector<uint8_t> decompressed(decompressedSize);

		int result = uncompress(decompressed.data(), &decompressedSize,
			data.data(), data.size());

		if (result != Z_OK) {
			VOXELCRAFT_LOG_ERROR("Failed to decompress chunk ({}, {})", coord.x, coord.z);
			return nullptr;
		}

		decompressed.resize(decompressedSize);

		auto chunk = std::make_shared<Chunk>(coord);
		if (chunk->Deserialize(decompressed)) {
			return chunk;
		}

		return nullptr;
	}

	void ChunkSystem::UpdateChunkLOD(std::shared_ptr<Chunk>& chunk, float distance)
	{
		if (!m_config.enableLOD) {
			return;
		}

		auto lod = CalculateLOD(distance);
		chunk->SetLODLevel(lod);
	}

	LODLevel ChunkSystem::CalculateLOD(float distance) const
	{
		if (!m_config.enableLOD) {
			return LODLevel::FULL;
		}

		if (distance <= 2.0f) {
			return LODLevel::FULL;
		} else if (distance <= 4.0f) {
			return LODLevel::HALF;
		} else if (distance <= 8.0f) {
			return LODLevel::QUARTER;
		} else if (distance <= 16.0f) {
			return LODLevel::EIGHTH;
		} else {
			return LODLevel::SIXTEENTH;
		}
	}

	void ChunkSystem::UpdateVisibleChunks()
	{
		// Calculate chunks to load and unload
		auto chunksToLoad = GetChunksToLoad(m_playerChunk);
		auto chunksToUnload = GetChunksToUnload(m_playerChunk);

		// Queue chunks for loading
		for (const auto& coord : chunksToLoad) {
			if (!HasChunk(coord)) {
				auto priority = CalculatePriority(coord);
				LoadChunkAsync(coord, priority);
			}
		}

		// Queue chunks for unloading
		for (const auto& coord : chunksToUnload) {
			UnloadChunk(coord);
		}
	}

	void ChunkSystem::UpdateChunkPriorities()
	{
		std::unique_lock<std::mutex> lock(m_chunkMutex);

		for (auto& pair : m_chunks) {
			auto& chunk = pair.second;
			if (chunk) {
				float distance = m_playerChunk.distance(pair.first);
				UpdateChunkLOD(chunk, distance);
			}
		}
	}

	void ChunkSystem::ProcessGenerationQueue()
	{
		if (m_config.enableMultithreading) {
			// Worker threads handle generation
			return;
		}

		// Single-threaded generation
		uint32_t processed = 0;
		while (processed < m_config.maxChunksPerFrame && !m_generationQueue.empty()) {
			ChunkRequest request;
			{
				std::unique_lock<std::mutex> lock(m_generationMutex);
				if (m_generationQueue.empty()) break;
				request = m_generationQueue.top();
				m_generationQueue.pop();
			}

			auto chunk = LoadChunk(request.coord);
			if (chunk) {
				{
					std::unique_lock<std::mutex> lock(m_chunkMutex);
					m_chunks[request.coord] = chunk;
				}

				if (request.callback) {
					request.callback(chunk);
				}
			}

			processed++;
		}
	}

	void ChunkSystem::ProcessSaveQueue()
	{
		if (m_config.enableMultithreading) {
			// Worker threads handle saving
			return;
		}

		// Single-threaded saving
		uint32_t processed = 0;
		while (processed < m_config.maxChunksToSave && !m_saveQueue.empty()) {
			ChunkCoord coord;
			{
				std::unique_lock<std::mutex> lock(m_saveMutex);
				if (m_saveQueue.empty()) break;
				coord = m_saveQueue.front();
				m_saveQueue.pop();
			}

			auto chunk = GetChunk(coord);
			if (chunk && chunk->IsModified()) {
				auto compressed = CompressChunk(chunk);
				SaveChunkToDisk(coord, compressed);
				chunk->SetModified(false);
			}

			processed++;
		}
	}

	void ChunkSystem::CleanupChunks()
	{
		std::unique_lock<std::mutex> lock(m_chunkMutex);

		// Remove chunks that are too far
		std::vector<ChunkCoord> toRemove;
		for (auto& pair : m_chunks) {
			float distance = m_playerChunk.distance(pair.first);
			if (distance > m_config.unloadDistance) {
				toRemove.push_back(pair.first);
			}
		}

		for (auto& coord : toRemove) {
			auto it = m_chunks.find(coord);
			if (it != m_chunks.end()) {
				auto& chunk = it->second;
				if (chunk && chunk->IsModified()) {
					SaveChunk(coord);
				}
				m_chunks.erase(it);
			}
		}

		// Cleanup compressed cache if too large
		while (m_compressedChunks.size() > m_config.chunkCacheSize) {
			auto it = m_compressedChunks.begin();
			m_compressedChunks.erase(it);
		}

		VOXELCRAFT_LOG_DEBUG("Cleaned up {} chunks", toRemove.size());
	}

	void ChunkSystem::UpdateStats()
	{
		std::unique_lock<std::mutex> lock(m_chunkMutex);

		m_stats.totalChunks = m_chunks.size();
		m_stats.loadedChunks = 0;
		m_stats.cachedChunks = m_compressedChunks.size();
		m_stats.generatingChunks = 0;
		m_stats.savingChunks = 0;

		// Count chunks by state and LOD
		m_stats.lodFullChunks = 0;
		m_stats.lodHalfChunks = 0;
		m_stats.lodQuarterChunks = 0;
		m_stats.lodEighthChunks = 0;

		for (auto& pair : m_chunks) {
			auto& chunk = pair.second;
			if (chunk) {
				m_stats.loadedChunks++;

				switch (chunk->GetState()) {
				case ChunkState::GENERATING:
					m_stats.generatingChunks++;
					break;
				case ChunkState::SAVING:
					m_stats.savingChunks++;
					break;
				default:
					break;
				}

				switch (chunk->GetLODLevel()) {
				case LODLevel::FULL:
					m_stats.lodFullChunks++;
					break;
				case LODLevel::HALF:
					m_stats.lodHalfChunks++;
					break;
				case LODLevel::QUARTER:
					m_stats.lodQuarterChunks++;
					break;
				case LODLevel::EIGHTH:
					m_stats.lodEighthChunks++;
					break;
				default:
					break;
				}
			}
		}

		// Calculate memory usage
		m_stats.memoryUsed = GetMemoryUsage();
		m_stats.memoryAvailable = m_config.maxLoadedChunks * ChunkSystemConfig::CHUNK_VOLUME * sizeof(uint16_t);
	}

	void ChunkSystem::GenerationThreadFunction()
	{
		VOXELCRAFT_LOG_INFO("Generation thread started");

		while (m_generating) {
			ChunkRequest request;

			{
				std::unique_lock<std::mutex> lock(m_generationMutex);
				m_generationCV.wait(lock, [this]() {
					return !m_generating || !m_generationQueue.empty();
				});

				if (!m_generating) break;

				if (m_generationQueue.empty()) continue;

				request = m_generationQueue.top();
				m_generationQueue.pop();
			}

			auto chunk = LoadChunk(request.coord);
			if (chunk) {
				{
					std::unique_lock<std::mutex> lock(m_chunkMutex);
					m_chunks[request.coord] = chunk;
				}

				if (request.callback) {
					request.callback(chunk);
				}
			}
		}

		VOXELCRAFT_LOG_INFO("Generation thread stopped");
	}

	void ChunkSystem::SaveThreadFunction()
	{
		VOXELCRAFT_LOG_INFO("Save thread started");

		while (m_saving) {
			ChunkCoord coord;

			{
				std::unique_lock<std::mutex> lock(m_saveMutex);
				m_saveCV.wait(lock, [this]() {
					return !m_saving || !m_saveQueue.empty();
				});

				if (!m_saving) break;

				if (m_saveQueue.empty()) continue;

				coord = m_saveQueue.front();
				m_saveQueue.pop();
			}

			auto chunk = GetChunk(coord);
			if (chunk && chunk->IsModified()) {
				auto compressed = CompressChunk(chunk);
				SaveChunkToDisk(coord, compressed);
				chunk->SetModified(false);
			}
		}

		VOXELCRAFT_LOG_INFO("Save thread stopped");
	}

	ChunkPriority ChunkSystem::CalculatePriority(const ChunkCoord& coord) const
	{
		float distance = m_playerChunk.distance(coord);

		if (distance <= 1.0f) {
			return ChunkPriority::CRITICAL;
		} else if (distance <= 2.0f) {
			return ChunkPriority::HIGH;
		} else if (distance <= m_config.loadDistance) {
			return ChunkPriority::MEDIUM;
		} else {
			return ChunkPriority::LOW;
		}
	}

	std::vector<ChunkCoord> ChunkSystem::GetChunksToLoad(const ChunkCoord& playerChunk)
	{
		std::vector<ChunkCoord> coords;

		int32_t radius = m_config.loadDistance;
		for (int32_t x = playerChunk.x - radius; x <= playerChunk.x + radius; ++x) {
			for (int32_t z = playerChunk.z - radius; z <= playerChunk.z + radius; ++z) {
				ChunkCoord coord(x, z);
				if (!HasChunk(coord)) {
					coords.push_back(coord);
				}
			}
		}

		return coords;
	}

	std::vector<ChunkCoord> ChunkSystem::GetChunksToUnload(const ChunkCoord& playerChunk)
	{
		std::unique_lock<std::mutex> lock(m_chunkMutex);

		std::vector<ChunkCoord> coords;

		int32_t radius = m_config.unloadDistance;
		for (auto& pair : m_chunks) {
			float distance = playerChunk.distance(pair.first);
			if (distance > radius) {
				coords.push_back(pair.first);
			}
		}

		return coords;
	}

	bool ChunkSystem::IsChunkInRange(const ChunkCoord& coord, const ChunkCoord& center, uint32_t range) const
	{
		return std::abs(coord.x - center.x) <= range && std::abs(coord.z - center.z) <= range;
	}

	std::string ChunkSystem::GetChunkFilePath(const ChunkCoord& coord) const
	{
		std::stringstream ss;
		ss << "world/chunks/" << coord.x << "_" << coord.z << ".chunk";
		return ss.str();
	}

} // namespace VoxelCraft
