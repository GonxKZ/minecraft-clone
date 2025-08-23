#pragma once
#include <memory>
#include <vector>
#include <unordered_map>
#include <random>
#include <cstdint>
#include "core/Logger.hpp"
#include "world/ChunkSystem.hpp"

namespace VoxelCraft {

	class Biome;
	class Block;
	class NoiseGenerator;
	class World;

	/**
	 * @brief Structure representing a biome region
	 */
	struct BiomeRegion
	{
		std::shared_ptr<Biome> biome;
		float temperature;
		float humidity;
		float continentalness;
		float erosion;
		float depth;
		float weirdness;
	};

	/**
	 * @brief Structure representing terrain generation parameters
	 */
	struct TerrainParams
	{
		float baseHeight = 64.0f;        // Base world height
		float heightVariation = 32.0f;   // Maximum height variation
		float noiseScale = 0.01f;        // Base noise scale
		float biomeScale = 0.005f;       // Biome distribution scale
		float caveScale = 0.02f;         // Cave generation scale
		float structureScale = 0.1f;     // Structure placement scale

		// Advanced parameters
		int octaves = 6;                 // Number of noise octaves
		float persistence = 0.5f;        // Noise persistence
		float lacunarity = 2.0f;         // Noise lacunarity
		float ridgeWeight = 0.5f;        // Ridge noise weight

		// Performance settings
		bool enableCaves = true;         // Enable cave generation
		bool enableRivers = true;        // Enable river generation
		bool enableStructures = true;    // Enable structure generation
		bool enableBiomes = true;        // Enable biome-based generation
		bool enableOres = true;          // Enable ore generation
	};

	/**
	 * @brief Structure representing a world generation seed
	 */
	struct WorldSeed
	{
		uint64_t masterSeed;             // Master seed for the world
		uint64_t terrainSeed;            // Terrain generation seed
		uint64_t biomeSeed;              // Biome distribution seed
		uint64_t structureSeed;          // Structure placement seed
		uint64_t caveSeed;               // Cave generation seed
		uint64_t oreSeed;                // Ore generation seed

		WorldSeed(uint64_t master = 0) {
			if (master == 0) {
				std::random_device rd;
				masterSeed = (static_cast<uint64_t>(rd()) << 32) | rd();
			} else {
				masterSeed = master;
			}

			// Generate derived seeds
			std::mt19937_64 gen(masterSeed);
			terrainSeed = gen();
			biomeSeed = gen();
			structureSeed = gen();
			caveSeed = gen();
			oreSeed = gen();
		}
	};

	/**
	 * @brief Terrain generation statistics
	 */
	struct TerrainStats
	{
		uint32_t chunksGenerated;
		uint32_t structuresGenerated;
		uint32_t cavesGenerated;
		uint32_t treesGenerated;
		uint32_t oresGenerated;
		uint32_t biomesGenerated;
		double averageGenerationTime;
		double averageBiomeTime;
		double averageStructureTime;
		double averageCaveTime;
	};

	/**
	 * @brief Advanced procedural terrain generator
	 *
	 * The TerrainGenerator creates infinite, diverse worlds with:
	 * - Multiple biomes with unique characteristics
	 * - Natural terrain features (mountains, valleys, plains)
	 * - Cave systems and underground structures
	 * - Surface and underground resources
	 * - Rivers, lakes, and water features
	 * - Trees, plants, and natural decorations
	 * - Weather and climate variations
	 * - Procedural structures (villages, dungeons, etc.)
	 */
	class TerrainGenerator
	{
	public:
		/**
		 * @brief Constructor
		 */
		TerrainGenerator();

		/**
		 * @brief Destructor
		 */
		~TerrainGenerator();

		/**
		 * @brief Initialize terrain generator
		 */
		bool Initialize(const WorldSeed& seed, const TerrainParams& params = TerrainParams());

		/**
		 * @brief Shutdown terrain generator
		 */
		void Shutdown();

		/**
		 * @brief Generate a complete chunk
		 */
		void GenerateChunk(std::shared_ptr<Chunk> chunk);

		/**
		 * @brief Generate terrain height at world coordinates
		 */
		float GenerateHeight(int32_t worldX, int32_t worldZ);

		/**
		 * @brief Get biome at world coordinates
		 */
		std::shared_ptr<Biome> GetBiome(int32_t worldX, int32_t worldZ);

		/**
		 * @brief Get biome region at world coordinates
		 */
		BiomeRegion GetBiomeRegion(int32_t worldX, int32_t worldZ);

		/**
		 * @brief Generate cave at coordinates
		 */
		bool GenerateCave(int32_t worldX, int32_t worldY, int32_t worldZ);

		/**
		 * @brief Generate structure at coordinates
		 */
		void GenerateStructure(int32_t worldX, int32_t worldY, int32_t worldZ, std::shared_ptr<Chunk> chunk);

		/**
		 * @brief Generate ores in chunk
		 */
		void GenerateOres(std::shared_ptr<Chunk> chunk);

		/**
		 * @brief Generate surface features
		 */
		void GenerateSurface(std::shared_ptr<Chunk> chunk);

		/**
		 * @brief Generate underground features
		 */
		void GenerateUnderground(std::shared_ptr<Chunk> chunk);

		/**
		 * @brief Get terrain generation statistics
		 */
		const TerrainStats& GetStats() const { return m_stats; }

		/**
		 * @brief Get terrain parameters
		 */
		const TerrainParams& GetParams() const { return m_params; }

		/**
		 * @brief Set terrain parameters
		 */
		void SetParams(const TerrainParams& params);

		/**
		 * @brief Get world seed
		 */
		const WorldSeed& GetSeed() const { return m_seed; }

		/**
		 * @brief Check if generator is initialized
		 */
		bool IsInitialized() const { return m_initialized; }

		/**
		 * @brief Generate world seed from string
		 */
		static WorldSeed SeedFromString(const std::string& seedString);

		/**
		 * @brief Convert seed to string
		 */
		static std::string SeedToString(const WorldSeed& seed);

	private:
		WorldSeed m_seed;
		TerrainParams m_params;
		TerrainStats m_stats;
		bool m_initialized;

		// Noise generators
		std::unique_ptr<NoiseGenerator> m_terrainNoise;
		std::unique_ptr<NoiseGenerator> m_biomeNoise;
		std::unique_ptr<NoiseGenerator> m_caveNoise;
		std::unique_ptr<NoiseGenerator> m_structureNoise;
		std::unique_ptr<NoiseGenerator> m_ridgeNoise;
		std::unique_ptr<NoiseGenerator> m_temperatureNoise;
		std::unique_ptr<NoiseGenerator> m_humidityNoise;

		// Biome registry
		std::vector<std::shared_ptr<Biome>> m_biomes;
		std::unordered_map<std::string, std::shared_ptr<Biome>> m_biomeMap;

		// Random number generators
		std::mt19937_64 m_randomGen;
		std::uniform_real_distribution<float> m_randomFloat;

		// Performance tracking
		std::chrono::steady_clock::time_point m_lastStatsUpdate;

		/**
		 * @brief Initialize noise generators
		 */
		void InitializeNoiseGenerators();

		/**
		 * @brief Initialize biomes
		 */
		void InitializeBiomes();

		/**
		 * @brief Generate base terrain
		 */
		void GenerateBaseTerrain(std::shared_ptr<Chunk> chunk);

		/**
		 * @brief Apply biome modifications
		 */
		void ApplyBiomeModifications(std::shared_ptr<Chunk> chunk);

		/**
		 * @brief Generate terrain height map
		 */
		void GenerateHeightMap(std::shared_ptr<Chunk> chunk);

		/**
		 * @brief Generate cave systems
		 */
		void GenerateCaveSystems(std::shared_ptr<Chunk> chunk);

		/**
		 * @brief Generate river systems
		 */
		void GenerateRiverSystems(std::shared_ptr<Chunk> chunk);

		/**
		 * @brief Generate ore deposits
		 */
		void GenerateOreDeposits(std::shared_ptr<Chunk> chunk);

		/**
		 * @brief Generate trees and vegetation
		 */
		void GenerateVegetation(std::shared_ptr<Chunk> chunk);

		/**
		 * @brief Generate surface decorations
		 */
		void GenerateDecorations(std::shared_ptr<Chunk> chunk);

		/**
		 * @brief Generate underground structures
		 */
		void GenerateUndergroundStructures(std::shared_ptr<Chunk> chunk);

		/**
		 * @brief Determine biome at coordinates
		 */
		std::shared_ptr<Biome> DetermineBiome(float temperature, float humidity,
			float continentalness, float erosion, float depth, float weirdness);

		/**
		 * @brief Calculate terrain height at coordinates
		 */
		float CalculateTerrainHeight(int32_t worldX, int32_t worldZ, const BiomeRegion& region);

		/**
		 * @brief Apply height modifications
		 */
		float ApplyHeightModifications(float baseHeight, int32_t worldX, int32_t worldZ);

		/**
		 * @brief Generate cave at specific location
		 */
		bool GenerateCaveAt(int32_t worldX, int32_t worldY, int32_t worldZ);

		/**
		 * @brief Check if location is suitable for structure
		 */
		bool IsValidStructureLocation(int32_t worldX, int32_t worldY, int32_t worldZ,
			const std::string& structureType);

		/**
		 * @brief Generate specific structure type
		 */
		void GenerateStructureType(const std::string& type, int32_t worldX, int32_t worldY, int32_t worldZ,
			std::shared_ptr<Chunk> chunk);

		/**
		 * @brief Update generation statistics
		 */
		void UpdateStats();

		/**
		 * @brief Get noise value with multiple octaves
		 */
		float GetNoiseValue(int32_t x, int32_t y, int32_t z, NoiseGenerator* noiseGen);

		/**
		 * @brief Get biome noise values
		 */
		void GetBiomeNoiseValues(int32_t worldX, int32_t worldZ, float& temperature,
			float& humidity, float& continentalness, float& erosion, float& depth, float& weirdness);

		/**
		 * @brief Apply post-processing to terrain
		 */
		void PostProcessTerrain(std::shared_ptr<Chunk> chunk);

		/**
		 * @brief Smooth terrain transitions
		 */
		void SmoothTerrainTransitions(std::shared_ptr<Chunk> chunk);

		/**
		 * @brief Validate generated terrain
		 */
		bool ValidateTerrain(std::shared_ptr<Chunk> chunk);
	};

} // namespace VoxelCraft