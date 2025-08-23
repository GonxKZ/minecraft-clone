#pragma once
#include <memory>
#include <vector>
#include <array>
#include <atomic>
#include <mutex>
#include <cstdint>
#include "core/Logger.hpp"
#include "world/ChunkSystem.hpp"

namespace VoxelCraft {

	class Block;
	class Biome;
	class World;

	/**
	 * @brief Individual chunk class representing 16x16x16 blocks
	 *
	 * The Chunk class manages:
	 * - Block storage and access
	 * - Lighting information
	 * - Biome data
	 * - Level of Detail (LOD)
	 * - Serialization/Deserialization
	 * - Memory management
	 * - Rendering preparation
	 */
	class Chunk
	{
	public:
		static constexpr uint8_t CHUNK_SIZE = 16;
		static constexpr uint8_t CHUNK_HEIGHT = 256;
		static constexpr uint32_t CHUNK_VOLUME = CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE;

		/**
		 * @brief Constructor
		 */
		Chunk(const ChunkCoord& coord);

		/**
		 * @brief Destructor
		 */
		~Chunk();

		/**
		 * @brief Get chunk coordinate
		 */
		const ChunkCoord& GetCoord() const { return m_coord; }

		/**
		 * @brief Get chunk state
		 */
		ChunkState GetState() const { return m_state.load(); }

		/**
		 * @brief Set chunk state
		 */
		void SetState(ChunkState state) { m_state.store(state); }

		/**
		 * @brief Get LOD level
		 */
		LODLevel GetLODLevel() const { return m_lodLevel.load(); }

		/**
		 * @brief Set LOD level
		 */
		void SetLODLevel(LODLevel level);

		/**
		 * @brief Check if chunk is modified
		 */
		bool IsModified() const { return m_modified.load(); }

		/**
		 * @brief Set modification flag
		 */
		void SetModified(bool modified) { m_modified.store(modified); }

		/**
		 * @brief Get block at local coordinates
		 */
		std::shared_ptr<Block> GetBlock(uint8_t x, uint8_t y, uint8_t z) const;

		/**
		 * @brief Set block at local coordinates
		 */
		void SetBlock(uint8_t x, uint8_t y, uint8_t z, std::shared_ptr<Block> block);

		/**
		 * @brief Get block at local coordinates (fast access)
		 */
		uint16_t GetBlockId(uint8_t x, uint8_t y, uint8_t z) const;

		/**
		 * @brief Set block ID at local coordinates (fast access)
		 */
		void SetBlockId(uint8_t x, uint8_t y, uint8_t z, uint16_t blockId);

		/**
		 * @brief Get light level at coordinates
		 */
		uint8_t GetLightLevel(uint8_t x, uint8_t y, uint8_t z) const;

		/**
		 * @brief Set light level at coordinates
		 */
		void SetLightLevel(uint8_t x, uint8_t y, uint8_t z, uint8_t level);

		/**
		 * @brief Get sky light level
		 */
		uint8_t GetSkyLight(uint8_t x, uint8_t y, uint8_t z) const;

		/**
		 * @brief Set sky light level
		 */
		void SetSkyLight(uint8_t x, uint8_t y, uint8_t z, uint8_t level);

		/**
		 * @brief Get biome at coordinates
		 */
		std::shared_ptr<Biome> GetBiome(uint8_t x, uint8_t z) const;

		/**
		 * @brief Set biome at coordinates
		 */
		void SetBiome(uint8_t x, uint8_t z, std::shared_ptr<Biome> biome);

		/**
		 * @brief Get height at coordinates
		 */
		uint8_t GetHeight(uint8_t x, uint8_t z) const;

		/**
		 * @brief Set height at coordinates
		 */
		void SetHeight(uint8_t x, uint8_t z, uint8_t height);

		/**
		 * @brief Check if block is air
		 */
		bool IsAir(uint8_t x, uint8_t y, uint8_t z) const;

		/**
		 * @brief Check if block is solid
		 */
		bool IsSolid(uint8_t x, uint8_t y, uint8_t z) const;

		/**
		 * @brief Check if block is transparent
		 */
		bool IsTransparent(uint8_t x, uint8_t y, uint8_t z) const;

		/**
		 * @brief Get memory usage
		 */
		size_t GetMemoryUsage() const;

		/**
		 * @brief Serialize chunk data
		 */
		std::vector<uint8_t> Serialize() const;

		/**
		 * @brief Deserialize chunk data
		 */
		bool Deserialize(const std::vector<uint8_t>& data);

		/**
		 * @brief Render the chunk
		 */
		void Render();

		/**
		 * @brief Update chunk (lighting, etc.)
		 */
		void Update(float deltaTime);

		/**
		 * @brief Generate mesh for rendering
		 */
		void GenerateMesh();

		/**
		 * @brief Clear chunk data
		 */
		void Clear();

		/**
		 * @brief Check if chunk is empty (all air)
		 */
		bool IsEmpty() const;

		/**
		 * @brief Get chunk bounds
		 */
		void GetBounds(int32_t& minX, int32_t& minY, int32_t& minZ,
			int32_t& maxX, int32_t& maxY, int32_t& maxZ) const;

		/**
		 * @brief Get world coordinates from local coordinates
		 */
		WorldCoord GetWorldCoord(uint8_t x, uint8_t y, uint8_t z) const;

		/**
		 * @brief Check if coordinates are valid
		 */
		static bool IsValidCoord(uint8_t x, uint8_t y, uint8_t z);

		/**
		 * @brief Get index from coordinates
		 */
		static uint32_t GetIndex(uint8_t x, uint8_t y, uint8_t z);

		/**
		 * @brief Get coordinates from index
		 */
		static void GetCoordFromIndex(uint32_t index, uint8_t& x, uint8_t& y, uint8_t& z);

	private:
		ChunkCoord m_coord;
		std::atomic<ChunkState> m_state;
		std::atomic<LODLevel> m_lodLevel;
		std::atomic<bool> m_modified;
		mutable std::mutex m_dataMutex;

		// Block data storage (16x256x16 = 65,536 blocks)
		// Using uint16_t for block IDs to support up to 65,536 different block types
		std::unique_ptr<uint16_t[]> m_blocks;

		// Lighting data (4 bits per level, packed as uint8_t)
		// Lower 4 bits: block light, Upper 4 bits: sky light
		std::unique_ptr<uint8_t[]> m_lighting;

		// Height map (16x16)
		std::unique_ptr<uint8_t[]> m_heightMap;

		// Biome data (16x16, one biome per column)
		std::vector<std::shared_ptr<Biome>> m_biomes;

		// Render data
		std::vector<float> m_vertexBuffer;
		std::vector<uint32_t> m_indexBuffer;
		std::atomic<bool> m_meshDirty;
		uint32_t m_vertexArrayObject;
		uint32_t m_vertexBufferObject;
		uint32_t m_indexBufferObject;

		// Statistics
		uint32_t m_solidBlockCount;
		uint32_t m_airBlockCount;
		uint32_t m_transparentBlockCount;

		/**
		 * @brief Initialize chunk data
		 */
		void Initialize();

		/**
		 * @brief Allocate memory for chunk data
		 */
		void AllocateData();

		/**
		 * @brief Free memory
		 */
		void FreeData();

		/**
		 * @brief Calculate lighting
		 */
		void CalculateLighting();

		/**
		 * @brief Propagate light
		 */
		void PropagateLight();

		/**
		 * @brief Update height map
		 */
		void UpdateHeightMap();

		/**
		 * @brief Generate optimized mesh based on LOD
		 */
		void GenerateLODMesh();

		/**
		 * @brief Add face to mesh
		 */
		void AddFaceToMesh(uint8_t x, uint8_t y, uint8_t z, uint8_t face);

		/**
		 * @brief Check if face should be rendered
		 */
		bool ShouldRenderFace(uint8_t x, uint8_t y, uint8_t z, uint8_t face) const;

		/**
		 * @brief Get neighboring block
		 */
		std::shared_ptr<Block> GetNeighborBlock(int8_t x, int8_t y, int8_t z) const;

		/**
		 * @brief Update block statistics
		 */
		void UpdateBlockStats();

		/**
		 * @brief Compress block data
		 */
		std::vector<uint8_t> CompressBlockData() const;

		/**
		 * @brief Decompress block data
		 */
		bool DecompressBlockData(const std::vector<uint8_t>& data);

		/**
		 * @brief Compress lighting data
		 */
		std::vector<uint8_t> CompressLightingData() const;

		/**
		 * @brief Decompress lighting data
		 */
		bool DecompressLightingData(const std::vector<uint8_t>& data);

		/**
		 * @brief Compress height map
		 */
		std::vector<uint8_t> CompressHeightMap() const;

		/**
		 * @brief Decompress height map
		 */
		bool DecompressHeightMap(const std::vector<uint8_t>& data);

		// Constants for face directions
		static constexpr uint8_t FACE_FRONT = 0;
		static constexpr uint8_t FACE_BACK = 1;
		static constexpr uint8_t FACE_LEFT = 2;
		static constexpr uint8_t FACE_RIGHT = 3;
		static constexpr uint8_t FACE_TOP = 4;
		static constexpr uint8_t FACE_BOTTOM = 5;

		// Constants for lighting
		static constexpr uint8_t MAX_LIGHT_LEVEL = 15;
		static constexpr uint8_t SUNLIGHT_LEVEL = 15;
	};

} // namespace VoxelCraft