#include "TerrainGenerator.hpp"
#include "Chunk.hpp"
#include "Biome.hpp"
#include "Block.hpp"
#include "World.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace VoxelCraft {

	TerrainGenerator::TerrainGenerator()
		: m_initialized(false)
		, m_randomFloat(0.0f, 1.0f)
	{
		// Initialize statistics
		std::memset(&m_stats, 0, sizeof(TerrainStats));

		VOXELCRAFT_LOG_INFO("TerrainGenerator initialized");
	}

	TerrainGenerator::~TerrainGenerator()
	{
		Shutdown();
	}

	bool TerrainGenerator::Initialize(const WorldSeed& seed, const TerrainParams& params)
	{
		if (m_initialized) {
			VOXELCRAFT_LOG_WARN("TerrainGenerator already initialized");
			return true;
		}

		m_seed = seed;
		m_params = params;

		// Initialize random number generator
		m_randomGen.seed(m_seed.masterSeed);

		// Initialize noise generators
		InitializeNoiseGenerators();

		// Initialize biomes
		InitializeBiomes();

		// Initialize structure generators
		InitializeStructureGenerators();

		// Initialize timing
		m_lastStatsUpdate = std::chrono::steady_clock::now();

		m_initialized = true;
		VOXELCRAFT_LOG_INFO("TerrainGenerator initialized successfully with seed: {}", m_seed.masterSeed);
		return true;
	}

	void TerrainGenerator::Shutdown()
	{
		if (!m_initialized) {
			return;
		}

		VOXELCRAFT_LOG_INFO("Shutting down TerrainGenerator...");

		// Clear all generators
		m_terrainNoise.reset();
		m_biomeNoise.reset();
		m_caveNoise.reset();
		m_structureNoise.reset();
		m_ridgeNoise.reset();
		m_temperatureNoise.reset();
		m_humidityNoise.reset();

		// Clear biomes
		m_biomes.clear();
		m_biomeMap.clear();

		m_initialized = false;

		VOXELCRAFT_LOG_INFO("TerrainGenerator shutdown complete");
	}

	void TerrainGenerator::GenerateChunk(std::shared_ptr<Chunk> chunk)
	{
		if (!m_initialized || !chunk) {
			return;
		}

		auto startTime = std::chrono::steady_clock::now();

		try {
			// Set chunk state to generating
			chunk->SetState(ChunkState::GENERATING);

			// Generate base terrain
			GenerateBaseTerrain(chunk);

			// Apply biome modifications
			if (m_params.enableBiomes) {
				ApplyBiomeModifications(chunk);
			}

			// Generate caves
			if (m_params.enableCaves) {
				GenerateCaveSystems(chunk);
			}

			// Generate underground features
			GenerateUnderground(chunk);

			// Generate surface features
			GenerateSurface(chunk);

			// Generate ores
			if (m_params.enableOres) {
				GenerateOres(chunk);
			}

			// Generate structures
			if (m_params.enableStructures) {
				GenerateUndergroundStructures(chunk);
			}

			// Post-process terrain
			PostProcessTerrain(chunk);

			// Validate terrain
			if (!ValidateTerrain(chunk)) {
				VOXELCRAFT_LOG_WARN("Generated terrain validation failed for chunk ({}, {})",
					chunk->GetCoord().x, chunk->GetCoord().z);
			}

			// Set chunk state to ready
			chunk->SetState(ChunkState::READY);

			auto endTime = std::chrono::steady_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

			// Update statistics
			m_stats.chunksGenerated++;
			m_stats.averageGenerationTime = (m_stats.averageGenerationTime * (m_stats.chunksGenerated - 1) + duration.count()) / m_stats.chunksGenerated;

			VOXELCRAFT_LOG_DEBUG("Generated chunk ({}, {}) in {}ms", chunk->GetCoord().x, chunk->GetCoord().z, duration.count());

		} catch (const std::exception& e) {
			VOXELCRAFT_LOG_ERROR("Failed to generate chunk ({}, {}): {}", chunk->GetCoord().x, chunk->GetCoord().z, e.what());
			chunk->SetState(ChunkState::ERROR);
		}
	}

	float TerrainGenerator::GenerateHeight(int32_t worldX, int32_t worldZ)
	{
		if (!m_initialized || !m_terrainNoise) {
			return m_params.baseHeight;
		}

		// Get biome region
		auto region = GetBiomeRegion(worldX, worldZ);

		// Calculate base height using multiple noise functions
		float baseNoise = GetNoiseValue(worldX, 0, worldZ, m_terrainNoise.get());
		float ridgeNoise = GetNoiseValue(worldX, 0, worldZ, m_ridgeNoise.get());
		float biomeNoise = region.biome ? region.biome->GetHeightVariation() : 1.0f;

		// Combine noises
		float combinedNoise = baseNoise * (1.0f - m_params.ridgeWeight) + ridgeNoise * m_params.ridgeWeight;
		combinedNoise *= biomeNoise;

		// Apply height modifications
		float finalHeight = ApplyHeightModifications(combinedNoise, worldX, worldZ);

		return m_params.baseHeight + finalHeight * m_params.heightVariation;
	}

	std::shared_ptr<Biome> TerrainGenerator::GetBiome(int32_t worldX, int32_t worldZ)
	{
		auto region = GetBiomeRegion(worldX, worldZ);
		return region.biome;
	}

	BiomeRegion TerrainGenerator::GetBiomeRegion(int32_t worldX, int32_t worldZ)
	{
		BiomeRegion region;

		// Get biome noise values
		GetBiomeNoiseValues(worldX, worldZ, region.temperature, region.humidity,
			region.continentalness, region.erosion, region.depth, region.weirdness);

		// Determine biome based on noise values
		region.biome = DetermineBiome(region.temperature, region.humidity,
			region.continentalness, region.erosion, region.depth, region.weirdness);

		return region;
	}

	bool TerrainGenerator::GenerateCave(int32_t worldX, int32_t worldY, int32_t worldZ)
	{
		if (!m_initialized || !m_caveNoise || !m_params.enableCaves) {
			return false;
		}

		return GenerateCaveAt(worldX, worldY, worldZ);
	}

	void TerrainGenerator::GenerateStructure(int32_t worldX, int32_t worldY, int32_t worldZ, std::shared_ptr<Chunk> chunk)
	{
		if (!m_initialized || !m_params.enableStructures) {
			return;
		}

		// Determine structure type based on location and biome
		auto biome = GetBiome(worldX, worldZ);
		if (!biome) return;

		auto structureTypes = biome->GetStructureTypes();
		if (structureTypes.empty()) return;

		// Select random structure type
		std::uniform_int_distribution<size_t> dist(0, structureTypes.size() - 1);
		std::string structureType = structureTypes[dist(m_randomGen)];

		// Check if location is suitable
		if (IsValidStructureLocation(worldX, worldY, worldZ, structureType)) {
			GenerateStructureType(structureType, worldX, worldY, worldZ, chunk);
			m_stats.structuresGenerated++;
		}
	}

	void TerrainGenerator::GenerateOres(std::shared_ptr<Chunk> chunk)
	{
		if (!m_initialized || !chunk) {
			return;
		}

		auto startTime = std::chrono::steady_clock::now();

		GenerateOreDeposits(chunk);

		auto endTime = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

		VOXELCRAFT_LOG_DEBUG("Generated ores in chunk ({}, {}) in {}ms",
			chunk->GetCoord().x, chunk->GetCoord().z, duration.count());
	}

	void TerrainGenerator::GenerateSurface(std::shared_ptr<Chunk> chunk)
	{
		if (!m_initialized || !chunk) {
			return;
		}

		// Generate vegetation
		GenerateVegetation(chunk);

		// Generate decorations
		GenerateDecorations(chunk);
	}

	void TerrainGenerator::GenerateUnderground(std::shared_ptr<Chunk> chunk)
	{
		if (!m_initialized || !chunk) {
			return;
		}

		// Generate underground structures
		GenerateUndergroundStructures(chunk);
	}

	void TerrainGenerator::SetParams(const TerrainParams& params)
	{
		m_params = params;
		VOXELCRAFT_LOG_INFO("TerrainGenerator parameters updated");
	}

	WorldSeed TerrainGenerator::SeedFromString(const std::string& seedString)
	{
		if (seedString.empty()) {
			return WorldSeed();
		}

		// Simple hash function for string to seed conversion
		uint64_t hash = 0;
		for (char c : seedString) {
			hash = hash * 31 + static_cast<uint64_t>(c);
		}

		return WorldSeed(hash);
	}

	std::string TerrainGenerator::SeedToString(const WorldSeed& seed)
	{
		// Convert seed to hexadecimal string
		char buffer[17];
		std::snprintf(buffer, sizeof(buffer), "%016llx", seed.masterSeed);
		return std::string(buffer);
	}

	void TerrainGenerator::InitializeNoiseGenerators()
	{
		// Initialize terrain noise generator
		m_terrainNoise = std::make_unique<NoiseGenerator>(m_seed.terrainSeed);
		m_terrainNoise->SetOctaves(m_params.octaves);
		m_terrainNoise->SetPersistence(m_params.persistence);
		m_terrainNoise->SetLacunarity(m_params.lacunarity);
		m_terrainNoise->SetScale(m_params.noiseScale);

		// Initialize biome noise generator
		m_biomeNoise = std::make_unique<NoiseGenerator>(m_seed.biomeSeed);
		m_biomeNoise->SetOctaves(4);
		m_biomeNoise->SetPersistence(0.6f);
		m_biomeNoise->SetLacunarity(2.2f);
		m_biomeNoise->SetScale(m_params.biomeScale);

		// Initialize cave noise generator
		m_caveNoise = std::make_unique<NoiseGenerator>(m_seed.caveSeed);
		m_caveNoise->SetOctaves(3);
		m_caveNoise->SetPersistence(0.7f);
		m_caveNoise->SetLacunarity(2.0f);
		m_caveNoise->SetScale(m_params.caveScale);

		// Initialize structure noise generator
		m_structureNoise = std::make_unique<NoiseGenerator>(m_seed.structureSeed);
		m_structureNoise->SetOctaves(2);
		m_structureNoise->SetPersistence(0.8f);
		m_structureNoise->SetLacunarity(1.8f);
		m_structureNoise->SetScale(m_params.structureScale);

		// Initialize ridge noise generator
		m_ridgeNoise = std::make_unique<NoiseGenerator>(m_seed.terrainSeed + 1);
		m_ridgeNoise->SetOctaves(m_params.octaves);
		m_ridgeNoise->SetPersistence(m_params.persistence);
		m_ridgeNoise->SetLacunarity(m_params.lacunarity);
		m_ridgeNoise->SetScale(m_params.noiseScale * 0.5f);

		// Initialize temperature noise generator
		m_temperatureNoise = std::make_unique<NoiseGenerator>(m_seed.biomeSeed + 1);
		m_temperatureNoise->SetOctaves(4);
		m_temperatureNoise->SetPersistence(0.6f);
		m_temperatureNoise->SetLacunarity(2.2f);
		m_temperatureNoise->SetScale(m_params.biomeScale);

		// Initialize humidity noise generator
		m_humidityNoise = std::make_unique<NoiseGenerator>(m_seed.biomeSeed + 2);
		m_humidityNoise->SetOctaves(4);
		m_humidityNoise->SetPersistence(0.6f);
		m_humidityNoise->SetLacunarity(2.2f);
		m_humidityNoise->SetScale(m_params.biomeScale);

		VOXELCRAFT_LOG_INFO("Noise generators initialized");
	}

	void TerrainGenerator::InitializeBiomes()
	{
		// Create all available biomes
		auto plainsBiome = std::make_shared<Biome>("plains");
		plainsBiome->SetTemperature(0.8f);
		plainsBiome->SetHumidity(0.4f);
		plainsBiome->SetHeightVariation(1.0f);
		plainsBiome->SetSurfaceBlock("grass");
		plainsBiome->SetFillerBlock("dirt");
		m_biomes.push_back(plainsBiome);
		m_biomeMap["plains"] = plainsBiome;

		auto desertBiome = std::make_shared<Biome>("desert");
		desertBiome->SetTemperature(2.0f);
		desertBiome->SetHumidity(0.0f);
		desertBiome->SetHeightVariation(0.8f);
		desertBiome->SetSurfaceBlock("sand");
		desertBiome->SetFillerBlock("sandstone");
		m_biomes.push_back(desertBiome);
		m_biomeMap["desert"] = desertBiome;

		auto forestBiome = std::make_shared<Biome>("forest");
		forestBiome->SetTemperature(0.7f);
		forestBiome->SetHumidity(0.8f);
		forestBiome->SetHeightVariation(1.2f);
		forestBiome->SetSurfaceBlock("grass");
		forestBiome->SetFillerBlock("dirt");
		forestBiome->AddStructureType("tree");
		m_biomes.push_back(forestBiome);
		m_biomeMap["forest"] = forestBiome;

		auto mountainBiome = std::make_shared<Biome>("mountain");
		mountainBiome->SetTemperature(0.2f);
		mountainBiome->SetHumidity(0.3f);
		mountainBiome->SetHeightVariation(2.5f);
		mountainBiome->SetSurfaceBlock("stone");
		mountainBiome->SetFillerBlock("stone");
		m_biomes.push_back(mountainBiome);
		m_biomeMap["mountain"] = mountainBiome;

		auto oceanBiome = std::make_shared<Biome>("ocean");
		oceanBiome->SetTemperature(0.5f);
		oceanBiome->SetHumidity(1.0f);
		oceanBiome->SetHeightVariation(0.1f);
		oceanBiome->SetSurfaceBlock("water");
		oceanBiome->SetFillerBlock("sand");
		m_biomes.push_back(oceanBiome);
		m_biomeMap["ocean"] = oceanBiome;

		auto swampBiome = std::make_shared<Biome>("swamp");
		swampBiome->SetTemperature(0.8f);
		swampBiome->SetHumidity(1.0f);
		swampBiome->SetHeightVariation(0.9f);
		swampBiome->SetSurfaceBlock("grass");
		swampBiome->SetFillerBlock("dirt");
		swampBiome->AddStructureType("swamp_tree");
		m_biomes.push_back(swampBiome);
		m_biomeMap["swamp"] = swampBiome;

		auto snowBiome = std::make_shared<Biome>("snow");
		snowBiome->SetTemperature(-0.5f);
		snowBiome->SetHumidity(0.2f);
		snowBiome->SetHeightVariation(1.5f);
		snowBiome->SetSurfaceBlock("snow");
		snowBiome->SetFillerBlock("dirt");
		m_biomes.push_back(snowBiome);
		m_biomeMap["snow"] = snowBiome;

		VOXELCRAFT_LOG_INFO("Biomes initialized: {} total", m_biomes.size());
	}

	void TerrainGenerator::InitializeStructureGenerators()
	{
		// Initialize structure generators
		// This would include tree generators, village generators, dungeon generators, etc.
		VOXELCRAFT_LOG_INFO("Structure generators initialized");
	}

	void TerrainGenerator::GenerateBaseTerrain(std::shared_ptr<Chunk> chunk)
	{
		if (!chunk) return;

		auto chunkCoord = chunk->GetCoord();

		// Generate height map first
		GenerateHeightMap(chunk);

		// Generate base terrain blocks
		for (uint8_t x = 0; x < Chunk::CHUNK_SIZE; ++x) {
			for (uint8_t z = 0; z < Chunk::CHUNK_SIZE; ++z) {
				// Get world coordinates
				int32_t worldX = chunkCoord.x * Chunk::CHUNK_SIZE + x;
				int32_t worldZ = chunkCoord.z * Chunk::CHUNK_SIZE + z;

				// Get biome and height at this location
				auto biome = GetBiome(worldX, worldZ);
				uint8_t height = chunk->GetHeight(x, z);

				// Generate terrain layers
				for (uint8_t y = 0; y < Chunk::CHUNK_HEIGHT; ++y) {
					if (y < height - 3) {
						// Deep underground - stone
						chunk->SetBlockId(x, y, z, 1); // Stone
					} else if (y < height) {
						// Underground - dirt or biome-specific filler
						if (biome && !biome->GetFillerBlock().empty()) {
							chunk->SetBlockId(x, y, z, 2); // Dirt (simplified)
						} else {
							chunk->SetBlockId(x, y, z, 2); // Dirt
						}
					} else if (y == height) {
						// Surface - grass or biome-specific surface
						if (biome && !biome->GetSurfaceBlock().empty()) {
							chunk->SetBlockId(x, y, z, 3); // Grass (simplified)
						} else {
							chunk->SetBlockId(x, y, z, 3); // Grass
						}
					} else {
						// Air
						chunk->SetBlockId(x, y, z, 0);
					}
				}
			}
		}
	}

	void TerrainGenerator::ApplyBiomeModifications(std::shared_ptr<Chunk> chunk)
	{
		if (!chunk) return;

		auto chunkCoord = chunk->GetCoord();

		for (uint8_t x = 0; x < Chunk::CHUNK_SIZE; ++x) {
			for (uint8_t z = 0; z < Chunk::CHUNK_SIZE; ++z) {
				int32_t worldX = chunkCoord.x * Chunk::CHUNK_SIZE + x;
				int32_t worldZ = chunkCoord.z * Chunk::CHUNK_SIZE + z;

				auto biome = GetBiome(worldX, worldZ);
				if (!biome) continue;

				uint8_t height = chunk->GetHeight(x, z);

				// Apply biome-specific modifications
				biome->ModifyTerrain(chunk, x, height, z);
			}
		}
	}

	void TerrainGenerator::GenerateHeightMap(std::shared_ptr<Chunk> chunk)
	{
		if (!chunk) return;

		auto chunkCoord = chunk->GetCoord();

		for (uint8_t x = 0; x < Chunk::CHUNK_SIZE; ++x) {
			for (uint8_t z = 0; z < Chunk::CHUNK_SIZE; ++z) {
				int32_t worldX = chunkCoord.x * Chunk::CHUNK_SIZE + x;
				int32_t worldZ = chunkCoord.z * Chunk::CHUNK_SIZE + z;

				float height = GenerateHeight(worldX, worldZ);
				chunk->SetHeight(x, z, static_cast<uint8_t>(std::clamp(height, 0.0f, 255.0f)));
			}
		}
	}

	void TerrainGenerator::GenerateCaveSystems(std::shared_ptr<Chunk> chunk)
	{
		if (!chunk || !m_params.enableCaves) return;

		auto startTime = std::chrono::steady_clock::now();

		auto chunkCoord = chunk->GetCoord();

		for (uint8_t x = 0; x < Chunk::CHUNK_SIZE; ++x) {
			for (uint8_t y = 0; y < Chunk::CHUNK_HEIGHT; ++y) {
				for (uint8_t z = 0; z < Chunk::CHUNK_SIZE; ++z) {
					int32_t worldX = chunkCoord.x * Chunk::CHUNK_SIZE + x;
					int32_t worldY = y;
					int32_t worldZ = chunkCoord.z * Chunk::CHUNK_SIZE + z;

					if (GenerateCaveAt(worldX, worldY, worldZ)) {
						chunk->SetBlockId(x, y, z, 0); // Air
						m_stats.cavesGenerated++;
					}
				}
			}
		}

		auto endTime = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

		m_stats.averageCaveTime = (m_stats.averageCaveTime * (m_stats.chunksGenerated - 1) + duration.count()) / m_stats.chunksGenerated;
	}

	void TerrainGenerator::GenerateRiverSystems(std::shared_ptr<Chunk> chunk)
	{
		if (!chunk || !m_params.enableRivers) return;

		// Simplified river generation
		// In a full implementation, this would create river networks
		// For now, just ensure water flows to the lowest points
		auto chunkCoord = chunk->GetCoord();

		for (uint8_t x = 0; x < Chunk::CHUNK_SIZE; ++x) {
			for (uint8_t z = 0; z < Chunk::CHUNK_SIZE; ++z) {
				uint8_t height = chunk->GetHeight(x, z);

				// Simple water level at height 62
				if (height < 62) {
					for (uint8_t y = height + 1; y <= 62; ++y) {
						chunk->SetBlockId(x, y, z, 4); // Water
					}
				}
			}
		}
	}

	void TerrainGenerator::GenerateOreDeposits(std::shared_ptr<Chunk> chunk)
	{
		if (!chunk) return;

		auto chunkCoord = chunk->GetCoord();

		// Generate different types of ores at different depths
		GenerateOreVein(chunk, 5, 1, 16, 64, 0.02f);  // Coal
		GenerateOreVein(chunk, 6, 1, 8, 32, 0.01f);   // Iron
		GenerateOreVein(chunk, 7, 1, 4, 16, 0.005f);  // Gold
		GenerateOreVein(chunk, 8, 1, 2, 8, 0.002f);   // Diamond

		m_stats.oresGenerated++;
	}

	void TerrainGenerator::GenerateOreVein(std::shared_ptr<Chunk> chunk, uint16_t oreId,
		uint8_t minSize, uint8_t maxSize, uint8_t maxHeight, float density)
	{
		auto chunkCoord = chunk->GetCoord();

		// Generate ore veins using noise
		for (uint8_t x = 0; x < Chunk::CHUNK_SIZE; ++x) {
			for (uint8_t z = 0; z < Chunk::CHUNK_SIZE; ++z) {
				int32_t worldX = chunkCoord.x * Chunk::CHUNK_SIZE + x;
				int32_t worldZ = chunkCoord.z * Chunk::CHUNK_SIZE + z;

				float oreNoise = GetNoiseValue(worldX, 0, worldZ, m_structureNoise.get());

				if (oreNoise > (1.0f - density)) {
					// Generate ore vein
					uint8_t veinSize = minSize + static_cast<uint8_t>((maxSize - minSize) * m_randomFloat(m_randomGen));

					for (uint8_t dx = 0; dx < veinSize; ++dx) {
						for (uint8_t dy = 0; dy < veinSize; ++dy) {
							for (uint8_t dz = 0; dz < veinSize; ++dz) {
								uint8_t blockX = x + dx;
								uint8_t blockY = static_cast<uint8_t>(maxHeight * m_randomFloat(m_randomGen));
								uint8_t blockZ = z + dz;

								if (blockX < Chunk::CHUNK_SIZE && blockY < Chunk::CHUNK_HEIGHT && blockZ < Chunk::CHUNK_SIZE) {
									// Only replace stone
									if (chunk->GetBlockId(blockX, blockY, blockZ) == 1) { // Stone
										chunk->SetBlockId(blockX, blockY, blockZ, oreId);
									}
								}
							}
						}
					}
				}
			}
		}
	}

	void TerrainGenerator::GenerateVegetation(std::shared_ptr<Chunk> chunk)
	{
		if (!chunk) return;

		auto chunkCoord = chunk->GetCoord();

		for (uint8_t x = 0; x < Chunk::CHUNK_SIZE; ++x) {
			for (uint8_t z = 0; z < Chunk::CHUNK_SIZE; ++z) {
				int32_t worldX = chunkCoord.x * Chunk::CHUNK_SIZE + x;
				int32_t worldZ = chunkCoord.z * Chunk::CHUNK_SIZE + z;

				auto biome = GetBiome(worldX, worldZ);
				if (!biome) continue;

				uint8_t height = chunk->GetHeight(x, z);

				// Generate trees based on biome
				if (biome->HasStructureType("tree")) {
					float treeNoise = GetNoiseValue(worldX, height, worldZ, m_structureNoise.get());
					if (treeNoise > 0.95f) {
						GenerateTree(chunk, x, height, z);
						m_stats.treesGenerated++;
					}
				}
			}
		}
	}

	void TerrainGenerator::GenerateTree(std::shared_ptr<Chunk> chunk, uint8_t x, uint8_t y, uint8_t z)
	{
		// Simple tree generation
		uint8_t treeHeight = 4 + static_cast<uint8_t>(3 * m_randomFloat(m_randomGen));

		// Generate trunk
		for (uint8_t h = 0; h < treeHeight; ++h) {
			if (y + h < Chunk::CHUNK_HEIGHT) {
				chunk->SetBlockId(x, y + h, z, 9); // Log
			}
		}

		// Generate leaves
		uint8_t leafStart = treeHeight - 2;
		for (uint8_t h = leafStart; h < treeHeight + 2; ++h) {
			for (int8_t dx = -2; dx <= 2; ++dx) {
				for (int8_t dz = -2; dz <= 2; ++dz) {
					if (std::abs(dx) + std::abs(dz) <= 3) {
						uint8_t leafX = x + dx;
						uint8_t leafZ = z + dz;
						uint8_t leafY = y + h;

						if (leafX < Chunk::CHUNK_SIZE && leafY < Chunk::CHUNK_HEIGHT && leafZ < Chunk::CHUNK_SIZE) {
							if (chunk->GetBlockId(leafX, leafY, leafZ) == 0) { // Only replace air
								chunk->SetBlockId(leafX, leafY, leafZ, 10); // Leaves
							}
						}
					}
				}
			}
		}
	}

	void TerrainGenerator::GenerateDecorations(std::shared_ptr<Chunk> chunk)
	{
		if (!chunk) return;

		// Generate grass, flowers, etc.
		auto chunkCoord = chunk->GetCoord();

		for (uint8_t x = 0; x < Chunk::CHUNK_SIZE; ++x) {
			for (uint8_t z = 0; z < Chunk::CHUNK_SIZE; ++z) {
				uint8_t height = chunk->GetHeight(x, z);

				// Add grass on surface
				if (height < Chunk::CHUNK_HEIGHT - 1) {
					float grassNoise = m_randomFloat(m_randomGen);
					if (grassNoise > 0.7f) {
						chunk->SetBlockId(x, height + 1, z, 11); // Tall grass
					}
				}
			}
		}
	}

	void TerrainGenerator::GenerateUndergroundStructures(std::shared_ptr<Chunk> chunk)
	{
		if (!chunk) return;

		auto chunkCoord = chunk->GetCoord();

		// Generate dungeons, caves, etc.
		for (uint8_t x = 0; x < Chunk::CHUNK_SIZE; ++x) {
			for (uint8_t z = 0; z < Chunk::CHUNK_SIZE; ++z) {
				int32_t worldX = chunkCoord.x * Chunk::CHUNK_SIZE + x;
				int32_t worldZ = chunkCoord.z * Chunk::CHUNK_SIZE + z;

				GenerateStructure(worldX, 0, worldZ, chunk);
			}
		}
	}

	std::shared_ptr<Biome> TerrainGenerator::DetermineBiome(float temperature, float humidity,
		float continentalness, float erosion, float depth, float weirdness)
	{
		// Simple biome selection based on temperature and humidity
		std::shared_ptr<Biome> bestBiome = m_biomes[0];
		float bestScore = std::numeric_limits<float>::max();

		for (auto& biome : m_biomes) {
			float tempDiff = std::abs(biome->GetTemperature() - temperature);
			float humidDiff = std::abs(biome->GetHumidity() - humidity);
			float score = tempDiff * tempDiff + humidDiff * humidDiff;

			if (score < bestScore) {
				bestScore = score;
				bestBiome = biome;
			}
		}

		m_stats.biomesGenerated++;
		return bestBiome;
	}

	float TerrainGenerator::CalculateTerrainHeight(int32_t worldX, int32_t worldZ, const BiomeRegion& region)
	{
		if (!m_terrainNoise) return m_params.baseHeight;

		float noise = GetNoiseValue(worldX, 0, worldZ, m_terrainNoise.get());
		float biomeFactor = region.biome ? region.biome->GetHeightVariation() : 1.0f;

		return noise * biomeFactor * m_params.heightVariation + m_params.baseHeight;
	}

	float TerrainGenerator::ApplyHeightModifications(float baseHeight, int32_t worldX, int32_t worldZ)
	{
		// Apply erosion and continentalness modifications
		auto region = GetBiomeRegion(worldX, worldZ);

		// Erosion reduces height variation
		float erosionFactor = 1.0f - region.erosion * 0.5f;

		// Continentalness affects overall height
		float continentalFactor = 1.0f + region.continentalness * 0.3f;

		return baseHeight * erosionFactor * continentalFactor;
	}

	bool TerrainGenerator::GenerateCaveAt(int32_t worldX, int32_t worldY, int32_t worldZ)
	{
		if (!m_caveNoise) return false;

		float caveNoise = GetNoiseValue(worldX, worldY, worldZ, m_caveNoise.get());

		// Caves are generated where noise is above threshold
		// Adjust threshold based on depth
		float threshold = 0.8f - (static_cast<float>(worldY) / Chunk::CHUNK_HEIGHT) * 0.3f;

		return caveNoise > threshold;
	}

	bool TerrainGenerator::IsValidStructureLocation(int32_t worldX, int32_t worldY, int32_t worldZ,
		const std::string& structureType)
	{
		// Check if location is suitable for the given structure type
		float structureNoise = GetNoiseValue(worldX, worldY, worldZ, m_structureNoise.get());

		if (structureType == "tree") {
			return structureNoise > 0.95f;
		} else if (structureType == "village") {
			return structureNoise > 0.99f;
		} else if (structureType == "dungeon") {
			return structureNoise > 0.98f && worldY < 50;
		}

		return false;
	}

	void TerrainGenerator::GenerateStructureType(const std::string& type, int32_t worldX, int32_t worldY, int32_t worldZ,
		std::shared_ptr<Chunk> chunk)
	{
		// Generate specific structure types
		if (type == "tree") {
			GenerateTree(chunk, worldX % Chunk::CHUNK_SIZE, worldY, worldZ % Chunk::CHUNK_SIZE);
		} else if (type == "village") {
			// Generate village (simplified)
			GenerateVillage(chunk, worldX % Chunk::CHUNK_SIZE, worldY, worldZ % Chunk::CHUNK_SIZE);
		} else if (type == "dungeon") {
			// Generate dungeon (simplified)
			GenerateDungeon(chunk, worldX % Chunk::CHUNK_SIZE, worldY, worldZ % Chunk::CHUNK_SIZE);
		}
	}

	void TerrainGenerator::GenerateVillage(std::shared_ptr<Chunk> chunk, uint8_t x, uint8_t y, uint8_t z)
	{
		// Simple village generation - just a few houses
		for (int8_t dx = -3; dx <= 3; ++dx) {
			for (int8_t dz = -3; dz <= 3; ++dz) {
				if (std::abs(dx) <= 1 && std::abs(dz) <= 1) {
					uint8_t houseX = x + dx;
					uint8_t houseZ = z + dz;

					if (houseX < Chunk::CHUNK_SIZE && houseZ < Chunk::CHUNK_SIZE) {
						// Build a simple house
						for (uint8_t h = 0; h < 3; ++h) {
							for (int8_t hx = -1; hx <= 1; ++hx) {
								for (int8_t hz = -1; hz <= 1; ++hz) {
									if (hx == 0 && hz == 0 && h < 2) continue; // Leave space for door

									uint8_t blockX = houseX + hx;
									uint8_t blockY = y + h;
									uint8_t blockZ = houseZ + hz;

									if (blockX < Chunk::CHUNK_SIZE && blockY < Chunk::CHUNK_HEIGHT && blockZ < Chunk::CHUNK_SIZE) {
										chunk->SetBlockId(blockX, blockY, blockZ, 12); // Wood planks
									}
								}
							}
						}
					}
				}
			}
		}
	}

	void TerrainGenerator::GenerateDungeon(std::shared_ptr<Chunk> chunk, uint8_t x, uint8_t y, uint8_t z)
	{
		// Simple dungeon generation
		uint8_t dungeonSize = 5;

		for (uint8_t dx = 0; dx < dungeonSize; ++dx) {
			for (uint8_t dy = 0; dy < 3; ++dy) {
				for (uint8_t dz = 0; dz < dungeonSize; ++dz) {
					if (dx == 0 || dx == dungeonSize - 1 || dz == 0 || dz == dungeonSize - 1 || dy == 0 || dy == 2) {
						uint8_t blockX = x + dx;
						uint8_t blockY = y + dy;
						uint8_t blockZ = z + dz;

						if (blockX < Chunk::CHUNK_SIZE && blockY < Chunk::CHUNK_HEIGHT && blockZ < Chunk::CHUNK_SIZE) {
							chunk->SetBlockId(blockX, blockY, blockZ, 13); // Stone bricks
						}
					}
				}
			}
		}
	}

	void TerrainGenerator::UpdateStats()
	{
		auto now = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastStatsUpdate);

		if (duration.count() >= 60) { // Update every minute
			VOXELCRAFT_LOG_INFO("TerrainGenerator Stats - Chunks: {}, Trees: {}, Caves: {}, Ores: {}, Structures: {}",
				m_stats.chunksGenerated, m_stats.treesGenerated, m_stats.cavesGenerated,
				m_stats.oresGenerated, m_stats.structuresGenerated);
			m_lastStatsUpdate = now;
		}
	}

	float TerrainGenerator::GetNoiseValue(int32_t x, int32_t y, int32_t z, NoiseGenerator* noiseGen)
	{
		if (!noiseGen) return 0.0f;

		return noiseGen->GetNoise(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
	}

	void TerrainGenerator::GetBiomeNoiseValues(int32_t worldX, int32_t worldZ, float& temperature,
		float& humidity, float& continentalness, float& erosion, float& depth, float& weirdness)
	{
		temperature = m_temperatureNoise ? GetNoiseValue(worldX, 0, worldZ, m_temperatureNoise.get()) : 0.5f;
		humidity = m_humidityNoise ? GetNoiseValue(worldX, 0, worldZ, m_humidityNoise.get()) : 0.5f;
		continentalness = m_biomeNoise ? GetNoiseValue(worldX, 0, worldZ, m_biomeNoise.get()) : 0.5f;
		erosion = m_structureNoise ? GetNoiseValue(worldX, 100, worldZ, m_structureNoise.get()) : 0.5f;
		depth = static_cast<float>(worldX * worldZ % 100) / 100.0f; // Simple depth calculation
		weirdness = m_ridgeNoise ? GetNoiseValue(worldX, 200, worldZ, m_ridgeNoise.get()) : 0.5f;

		// Normalize values to 0-1 range
		temperature = (temperature + 1.0f) * 0.5f;
		humidity = (humidity + 1.0f) * 0.5f;
		continentalness = (continentalness + 1.0f) * 0.5f;
		erosion = (erosion + 1.0f) * 0.5f;
		depth = (depth + 1.0f) * 0.5f;
		weirdness = (weirdness + 1.0f) * 0.5f;
	}

	void TerrainGenerator::PostProcessTerrain(std::shared_ptr<Chunk> chunk)
	{
		if (!chunk) return;

		// Apply post-processing effects
		SmoothTerrainTransitions(chunk);

		// Ensure water flows correctly
		if (m_params.enableRivers) {
			GenerateRiverSystems(chunk);
		}
	}

	void TerrainGenerator::SmoothTerrainTransitions(std::shared_ptr<Chunk> chunk)
	{
		if (!chunk) return;

		// Simple terrain smoothing to reduce sharp transitions
		for (uint8_t x = 1; x < Chunk::CHUNK_SIZE - 1; ++x) {
			for (uint8_t z = 1; z < Chunk::CHUNK_SIZE - 1; ++z) {
				uint8_t centerHeight = chunk->GetHeight(x, z);
				uint8_t avgHeight = 0;
				uint8_t count = 0;

				// Average with neighboring heights
				for (int8_t dx = -1; dx <= 1; ++dx) {
					for (int8_t dz = -1; dz <= 1; ++dz) {
						avgHeight += chunk->GetHeight(x + dx, z + dz);
						count++;
					}
				}

				avgHeight /= count;

				// Smooth transition
				if (std::abs(centerHeight - avgHeight) > 2) {
					chunk->SetHeight(x, z, (centerHeight + avgHeight) / 2);
				}
			}
		}
	}

	bool TerrainGenerator::ValidateTerrain(std::shared_ptr<Chunk> chunk)
	{
		if (!chunk) return false;

		// Basic terrain validation
		bool isValid = true;

		for (uint8_t x = 0; x < Chunk::CHUNK_SIZE; ++x) {
			for (uint8_t z = 0; z < Chunk::CHUNK_SIZE; ++z) {
				uint8_t height = chunk->GetHeight(x, z);

				// Check height is within valid range
				if (height >= Chunk::CHUNK_HEIGHT) {
					isValid = false;
					chunk->SetHeight(x, z, Chunk::CHUNK_HEIGHT - 1);
				}

				// Check surface block exists
				if (chunk->GetBlockId(x, height, z) == 0) { // Air at surface
					isValid = false;
					// Fix by placing grass
					chunk->SetBlockId(x, height, z, 3);
				}
			}
		}

		return isValid;
	}

} // namespace VoxelCraft
