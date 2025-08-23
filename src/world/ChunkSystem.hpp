#pragma once
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <mutex>
#include <atomic>
#include <functional>
#include <cstdint>
#include <chrono>
#include "core/Logger.hpp"

namespace VoxelCraft {

	class World;
	class Chunk;
	class Block;
	class TerrainGenerator;
	class Biome;

	/**
	 * @brief Chunk coordinates structure
	 */
	struct ChunkCoord
	{
		int32_t x;
		int32_t z;

		ChunkCoord(int32_t x = 0, int32_t z = 0) : x(x), z(z) {}

		bool operator==(const ChunkCoord& other) const {
			return x == other.x && z == other.z;
		}

		bool operator!=(const ChunkCoord& other) const {
			return !(*this == other);
		}

		bool operator<(const ChunkCoord& other) const {
			return x < other.x || (x == other.x && z < other.z);
		}

		ChunkCoord operator+(const ChunkCoord& other) const {
			return ChunkCoord(x + other.x, z + other.z);
		}

		ChunkCoord operator-(const ChunkCoord& other) const {
			return ChunkCoord(x - other.x, z - other.z);
		}

		float distance(const ChunkCoord& other) const {
			int32_t dx = x - other.x;
			int32_t dz = z - other.z;
			return std::sqrt(dx * dx + dz * dz);
		}
	};

	/**
	 * @brief Block coordinates within a chunk
	 */
	struct BlockCoord
	{
		uint8_t x;
		uint8_t y;
		uint8_t z;

		BlockCoord(uint8_t x = 0, uint8_t y = 0, uint8_t z = 0) : x(x), y(y), z(z) {}

		bool operator==(const BlockCoord& other) const {
			return x == other.x && y == other.y && z == other.z;
		}

		bool operator!=(const BlockCoord& other) const {
			return !(*this == other);
		}
	};

	/**
	 * @brief World coordinates structure
	 */
	struct WorldCoord
	{
		int32_t x;
		int32_t y;
		int32_t z;

		WorldCoord(int32_t x = 0, int32_t y = 0, int32_t z = 0) : x(x), y(y), z(z) {}

		WorldCoord(const ChunkCoord& chunk, const BlockCoord& block) {
			x = chunk.x * 16 + block.x;
			y = block.y;
			z = chunk.z * 16 + block.z;
		}

		ChunkCoord toChunkCoord() const {
			return ChunkCoord(x >> 4, z >> 4);
		}

		BlockCoord toBlockCoord() const {
			return BlockCoord(x & 15, y, z & 15);
		}

		bool operator==(const WorldCoord& other) const {
			return x == other.x && y == other.y && z == other.z;
		}

		bool operator!=(const WorldCoord& other) const {
			return !(*this == other);
		}

		WorldCoord operator+(const WorldCoord& other) const {
			return WorldCoord(x + other.x, y + other.y, z + other.z);
		}

		WorldCoord operator-(const WorldCoord& other) const {
			return WorldCoord(x - other.x, y - other.y, z - other.z);
		}

		float distance(const WorldCoord& other) const {
			int32_t dx = x - other.x;
			int32_t dy = y - other.y;
			int32_t dz = z - other.z;
			return std::sqrt(dx * dx + dy * dy + dz * dz);
		}
	};

	/**
	 * @brief Chunk states for lifecycle management
	 */
	enum class ChunkState
	{
		EMPTY,          // Chunk slot is empty
		LOADING,        // Chunk is being loaded
		GENERATING,     // Chunk is being generated
		READY,          // Chunk is ready for use
		RENDERING,      // Chunk is being rendered
		UNLOADING,      // Chunk is being unloaded
		SAVING,         // Chunk is being saved
		ERROR           // Chunk is in error state
	};

	/**
	 * @brief Level of Detail levels
	 */
	enum class LODLevel
	{
		FULL,           // 16x16x16 full detail
		HALF,           // 8x8x8 half detail
		QUARTER,        // 4x4x4 quarter detail
		EIGHTH,         // 2x2x2 eighth detail
		SIXTEENTH,      // 1x1x1 sixteenth detail
		INACTIVE        // Not rendered
	};

	/**
	 * @brief Chunk priority levels
	 */
	enum class ChunkPriority
	{
		CRITICAL,       // Player is in this chunk
		HIGH,           // Adjacent to player
		MEDIUM,         // In player view range
		LOW,            // Outside view range but in memory
		IDLE            // Background loading
	};

	/**
	 * @brief Chunk generation request structure
	 */
	struct ChunkRequest
	{
		ChunkCoord coord;
		ChunkPriority priority;
		std::chrono::steady_clock::time_point timestamp;
		std::function<void(std::shared_ptr<Chunk>)> callback;

		bool operator<(const ChunkRequest& other) const {
			if (priority != other.priority) {
				return static_cast<int>(priority) < static_cast<int>(other.priority);
			}
			return timestamp > other.timestamp; // Earlier requests first
		}
	};

	/**
	 * @brief Chunk system configuration
	 */
	struct ChunkSystemConfig
	{
		// Chunk dimensions
		static constexpr uint8_t CHUNK_SIZE = 16;
		static constexpr uint8_t CHUNK_HEIGHT = 256;
		static constexpr uint32_t CHUNK_VOLUME = CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE;

		// Memory management
		uint32_t maxLoadedChunks = 1024;     // Maximum chunks in memory
		uint32_t maxRenderChunks = 256;      // Maximum chunks to render
		uint32_t chunkCacheSize = 512;       // Size of compressed chunk cache

		// Distance settings
		uint32_t loadDistance = 8;           // Chunks to load around player
		uint32_t renderDistance = 6;         // Chunks to render around player
		uint32_t unloadDistance = 12;        // Distance to unload chunks

		// Performance settings
		uint32_t maxChunksPerFrame = 4;      // Max chunks to generate per frame
		uint32_t maxChunksToSave = 8;        // Max chunks to save per frame
		float cleanupInterval = 30.0f;       // Cleanup interval in seconds

		// Threading
		uint32_t generationThreads = 2;      // Number of generation threads
		uint32_t saveThreads = 1;            // Number of save threads

		// Features
		bool enableLOD = true;               // Enable level of detail
		bool enableCompression = true;       // Enable chunk compression
		bool enableStreaming = true;         // Enable chunk streaming
		bool enableMultithreading = true;    // Enable multithreaded generation
		bool enableProfiling = true;         // Enable performance profiling
	};

	/**
	 * @brief Chunk system statistics
	 */
	struct ChunkSystemStats
	{
		// Counts
		uint32_t totalChunks;
		uint32_t loadedChunks;
		uint32_t renderedChunks;
		uint32_t cachedChunks;
		uint32_t generatingChunks;
		uint32_t savingChunks;

		// Performance
		float averageGenerationTime;
		float averageLoadTime;
		float averageSaveTime;
		float averageRenderTime;
		uint32_t chunksGeneratedPerSecond;
		uint32_t chunksLoadedPerSecond;
		uint32_t chunksSavedPerSecond;

		// Memory
		size_t memoryUsed;
		size_t memoryAvailable;
		size_t compressedMemory;
		float compressionRatio;

		// LOD stats
		uint32_t lodFullChunks;
		uint32_t lodHalfChunks;
		uint32_t lodQuarterChunks;
		uint32_t lodEighthChunks;

		// Cache stats
		float cacheHitRate;
		uint32_t cacheMisses;
		uint32_t cacheHits;
	};

	/**
	 * @brief Main Chunk System class
	 *
	 * The ChunkSystem manages:
	 * - Infinite world chunk loading and unloading
	 * - Procedural terrain generation
	 * - Level of Detail (LOD) rendering
	 * - Chunk compression and caching
	 * - Multi-threaded generation
	 * - Memory management for large worlds
	 */
	class ChunkSystem
	{
	public:
		/**
		 * @brief Constructor
		 */
		ChunkSystem(const ChunkSystemConfig& config = ChunkSystemConfig());

		/**
		 * @brief Destructor
		 */
		~ChunkSystem();

		/**
		 * @brief Initialize chunk system
		 */
		bool Initialize(World* world);

		/**
		 * @brief Shutdown chunk system
		 */
		void Shutdown();

		/**
		 * @brief Update chunk system
		 */
		void Update(float deltaTime, const ChunkCoord& playerChunk);

		/**
		 * @brief Render visible chunks
		 */
		void Render();

		/**
		 * @brief Get chunk at coordinates
		 */
		std::shared_ptr<Chunk> GetChunk(const ChunkCoord& coord);

		/**
		 * @brief Check if chunk exists
		 */
		bool HasChunk(const ChunkCoord& coord) const;

		/**
		 * @brief Load chunk asynchronously
		 */
		void LoadChunkAsync(const ChunkCoord& coord, ChunkPriority priority = ChunkPriority::MEDIUM,
			std::function<void(std::shared_ptr<Chunk>)> callback = nullptr);

		/**
		 * @brief Unload chunk
		 */
		void UnloadChunk(const ChunkCoord& coord);

		/**
		 * @brief Save chunk
		 */
		void SaveChunk(const ChunkCoord& coord);

		/**
		 * @brief Save all chunks
		 */
		void SaveAllChunks();

		/**
		 * @brief Get block at world coordinates
		 */
		std::shared_ptr<Block> GetBlock(const WorldCoord& coord);

		/**
		 * @brief Set block at world coordinates
		 */
		void SetBlock(const WorldCoord& coord, std::shared_ptr<Block> block);

		/**
		 * @brief Get biome at world coordinates
		 */
		std::shared_ptr<Biome> GetBiome(const WorldCoord& coord);

		/**
		 * @brief Get chunk system statistics
		 */
		const ChunkSystemStats& GetStats() const { return m_stats; }

		/**
		 * @brief Get chunk system configuration
		 */
		const ChunkSystemConfig& GetConfig() const { return m_config; }

		/**
		 * @brief Set chunk system configuration
		 */
		void SetConfig(const ChunkSystemConfig& config);

		/**
		 * @brief Get all loaded chunks
		 */
		std::vector<std::shared_ptr<Chunk>> GetAllChunks() const;

		/**
		 * @brief Get chunks in radius
		 */
		std::vector<std::shared_ptr<Chunk>> GetChunksInRadius(const ChunkCoord& center, uint32_t radius);

		/**
		 * @brief Get visible chunks for rendering
		 */
		std::vector<std::shared_ptr<Chunk>> GetVisibleChunks() const;

		/**
		 * @brief Force chunk generation
		 */
		std::shared_ptr<Chunk> ForceGenerateChunk(const ChunkCoord& coord);

		/**
		 * @brief Clear chunk cache
		 */
		void ClearCache();

		/**
		 * @brief Get memory usage
		 */
		size_t GetMemoryUsage() const;

		/**
		 * @brief Check if system is initialized
		 */
		bool IsInitialized() const { return m_initialized; }

		/**
		 * @brief Get player chunk coordinate
		 */
		const ChunkCoord& GetPlayerChunk() const { return m_playerChunk; }

	private:
		ChunkSystemConfig m_config;
		ChunkSystemStats m_stats;
		World* m_world;
		std::shared_ptr<TerrainGenerator> m_terrainGenerator;
		bool m_initialized;

		// Chunk storage
		std::unordered_map<ChunkCoord, std::shared_ptr<Chunk>> m_chunks;
		std::unordered_map<ChunkCoord, std::vector<uint8_t>> m_compressedChunks; // Cache
		std::priority_queue<ChunkRequest> m_generationQueue;
		std::queue<ChunkCoord> m_saveQueue;

		// Player tracking
		ChunkCoord m_playerChunk;
		std::vector<ChunkCoord> m_chunksToLoad;
		std::vector<ChunkCoord> m_chunksToUnload;

		// Threading
		std::vector<std::thread> m_generationThreads;
		std::thread m_saveThread;
		std::mutex m_chunkMutex;
		std::mutex m_generationMutex;
		std::mutex m_saveMutex;
		std::condition_variable m_generationCV;
		std::condition_variable m_saveCV;
		std::atomic<bool> m_generating;
		std::atomic<bool> m_saving;

		// Performance tracking
		std::chrono::steady_clock::time_point m_lastCleanupTime;
		std::chrono::steady_clock::time_point m_lastStatsUpdate;

		/**
		 * @brief Generate chunk
		 */
		std::shared_ptr<Chunk> GenerateChunk(const ChunkCoord& coord);

		/**
		 * @brief Load chunk from disk
		 */
		std::shared_ptr<Chunk> LoadChunk(const ChunkCoord& coord);

		/**
		 * @brief Save chunk to disk
		 */
		void SaveChunkToDisk(const ChunkCoord& coord, const std::vector<uint8_t>& data);

		/**
		 * @brief Compress chunk data
		 */
		std::vector<uint8_t> CompressChunk(const std::shared_ptr<Chunk>& chunk);

		/**
		 * @brief Decompress chunk data
		 */
		std::shared_ptr<Chunk> DecompressChunk(const ChunkCoord& coord, const std::vector<uint8_t>& data);

		/**
		 * @brief Update chunk LOD
		 */
		void UpdateChunkLOD(std::shared_ptr<Chunk>& chunk, float distance);

		/**
		 * @brief Calculate chunk LOD based on distance
		 */
		LODLevel CalculateLOD(float distance) const;

		/**
		 * @brief Update visible chunks
		 */
		void UpdateVisibleChunks();

		/**
		 * @brief Update chunk priorities
		 */
		void UpdateChunkPriorities();

		/**
		 * @brief Process generation queue
		 */
		void ProcessGenerationQueue();

		/**
		 * @brief Process save queue
		 */
		void ProcessSaveQueue();

		/**
		 * @brief Cleanup unused chunks
		 */
		void CleanupChunks();

		/**
		 * @brief Update statistics
		 */
		void UpdateStats();

		/**
		 * @brief Generation thread function
		 */
		void GenerationThreadFunction();

		/**
		 * @brief Save thread function
		 */
		void SaveThreadFunction();

		/**
		 * @brief Calculate chunk priority
		 */
		ChunkPriority CalculatePriority(const ChunkCoord& coord) const;

		/**
		 * @brief Get chunks to load
		 */
		std::vector<ChunkCoord> GetChunksToLoad(const ChunkCoord& playerChunk);

		/**
		 * @brief Get chunks to unload
		 */
		std::vector<ChunkCoord> GetChunksToUnload(const ChunkCoord& playerChunk);

		/**
		 * @brief Is chunk in range
		 */
		bool IsChunkInRange(const ChunkCoord& coord, const ChunkCoord& center, uint32_t range) const;

		/**
		 * @brief Get chunk file path
		 */
		std::string GetChunkFilePath(const ChunkCoord& coord) const;
	};

} // namespace VoxelCraft

// Hash function for ChunkCoord
template<>
struct std::hash<VoxelCraft::ChunkCoord>
{
	size_t operator()(const VoxelCraft::ChunkCoord& coord) const
	{
		return std::hash<int32_t>()(coord.x) ^ (std::hash<int32_t>()(coord.z) << 1);
	}
};

// Hash function for BlockCoord
template<>
struct std::hash<VoxelCraft::BlockCoord>
{
	size_t operator()(const VoxelCraft::BlockCoord& coord) const
	{
		return std::hash<uint8_t>()(coord.x) ^
			   (std::hash<uint8_t>()(coord.y) << 8) ^
			   (std::hash<uint8_t>()(coord.z) << 16);
	}
};

// Hash function for WorldCoord
template<>
struct std::hash<VoxelCraft::WorldCoord>
{
	size_t operator()(const VoxelCraft::WorldCoord& coord) const
	{
		return std::hash<int32_t>()(coord.x) ^
			   (std::hash<int32_t>()(coord.y) << 1) ^
			   (std::hash<int32_t>()(coord.z) << 2);
	}
};
