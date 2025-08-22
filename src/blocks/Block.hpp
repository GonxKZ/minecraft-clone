/**
 * @file Block.hpp
 * @brief VoxelCraft Block System Core
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the Block class and block system architecture that forms
 * the foundation of the voxel-based world in VoxelCraft.
 */

#ifndef VOXELCRAFT_BLOCKS_BLOCK_HPP
#define VOXELCRAFT_BLOCKS_BLOCK_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <optional>
#include <any>

#include "../core/Config.hpp"

namespace VoxelCraft {

    // Forward declarations
    class World;
    class Entity;
    class Player;
    struct BlockCoordinate;
    struct BlockProperties;
    struct BlockBehavior;
    class BlockMeshGenerator;

    /**
     * @typedef BlockID
     * @brief Unique identifier for block types
     */
    using BlockID = uint32_t;

    /**
     * @typedef BlockMetadata
     * @brief Block metadata value
     */
    using BlockMetadata = uint8_t;

    /**
     * @enum BlockFace
     * @brief Block face directions
     */
    enum class BlockFace {
        Front,      ///< Positive X
        Back,       ///< Negative X
        Right,      ///< Positive Z
        Left,       ///< Negative Z
        Top,        ///< Positive Y
        Bottom      ///< Negative Y
    };

    /**
     * @enum BlockRenderType
     * @brief Block rendering modes
     */
    enum class BlockRenderType {
        Solid,              ///< Standard solid block
        Transparent,        ///< Transparent block
        Translucent,        ///< Translucent block
        Invisible,          ///< Invisible block
        Liquid,             ///< Liquid block
        Cross,              ///< Cross/plant rendering
        Torch,              ///< Torch rendering
        Fence,              ///< Fence rendering
        Stairs,             ///< Stairs rendering
        Slab,               ///< Slab rendering
        Custom              ///< Custom rendering
    };

    /**
     * @enum BlockSoundType
     * @brief Block sound categories
     */
    enum class BlockSoundType {
        Stone,      ///< Stone sound
        Wood,       ///< Wood sound
        Gravel,     ///< Gravel sound
        Grass,      ///< Grass sound
        Metal,      ///< Metal sound
        Glass,      ///< Glass sound
        Cloth,      ///< Cloth sound
        Sand,       ///< Sand sound
        Snow,       ///< Snow sound
        Ladder,     ///< Ladder sound
        Anvil,      ///< Anvil sound
        Slime,      ///< Slime sound
        WetGrass,   ///< Wet grass sound
        Coral,      ///< Coral sound
        Bamboo,     ///< Bamboo sound
        BambooSapling, ///< Bamboo sapling sound
        Custom      ///< Custom sound
    };

    /**
     * @enum BlockMaterial
     * @brief Block material properties
     */
    enum class BlockMaterial {
        Air,        ///< Air material
        Solid,      ///< Solid material
        Liquid,     ///< Liquid material
        Plant,      ///< Plant material
        Replaceable, ///< Replaceable material
        Fire,       ///< Fire material
        Web,        ///< Web material
        Lava,       ///< Lava material
        Ice,        ///< Ice material
        Leaves,     ///< Leaves material
        Glass,      ///< Glass material
        Barrier,    ///< Barrier material
        Custom      ///< Custom material
    };

    /**
     * @struct BlockTextureCoords
     * @brief Texture coordinates for block faces
     */
    struct BlockTextureCoords {
        float u1, v1, u2, v2;    ///< UV coordinates (0.0 - 1.0)

        BlockTextureCoords(float u1 = 0.0f, float v1 = 0.0f,
                          float u2 = 1.0f, float v2 = 1.0f)
            : u1(u1), v1(v1), u2(u2), v2(v2) {}
    };

    /**
     * @struct BlockTextures
     * @brief Texture information for all block faces
     */
    struct BlockTextures {
        std::string textureName;                     ///< Base texture name
        std::unordered_map<BlockFace, BlockTextureCoords> faceTextures; ///< Face-specific textures
        BlockTextureCoords allFaces;                 ///< Default texture for all faces
        bool hasDifferentFaces;                      ///< Uses different textures per face
        bool animated;                              ///< Texture is animated
        int animationFrames;                        ///< Number of animation frames
        float animationSpeed;                       ///< Animation speed

        BlockTextures(const std::string& name = "")
            : textureName(name), allFaces(0.0f, 0.0f, 1.0f, 1.0f),
              hasDifferentFaces(false), animated(false),
              animationFrames(1), animationSpeed(1.0f) {}
    };

    /**
     * @struct BlockPhysics
     * @brief Physics properties of a block
     */
    struct BlockPhysics {
        float hardness;              ///< Block hardness (mining time)
        float resistance;            ///< Explosion resistance
        float friction;              ///< Movement friction
        float slipperiness;          ///< Slipperiness factor
        float velocityMultiplier;    ///< Velocity multiplier
        float jumpVelocity;          ///< Jump velocity multiplier
        bool isSolid;               ///< Is solid for collision
        bool isOpaque;              ///< Is opaque for lighting
        bool isFullBlock;           ///< Is full 1x1x1 block
        bool hasGravity;            ///< Affected by gravity
        bool canFall;               ///< Can fall if unsupported
        bool isLiquid;              ///< Is liquid block
        bool isFlammable;           ///< Can catch fire
        int flammability;           ///< Flammability level
        int fireSpreadSpeed;        ///< Fire spread speed
        bool needsRandomTick;       ///< Needs random ticks
        bool needsScheduledTick;    ///< Needs scheduled ticks
    };

    /**
     * @struct BlockBounds
     * @brief Collision and selection bounds
     */
    struct BlockBounds {
        float minX, minY, minZ;    ///< Minimum bounds
        float maxX, maxY, maxZ;    ///< Maximum bounds

        BlockBounds(float minX = 0.0f, float minY = 0.0f, float minZ = 0.0f,
                   float maxX = 1.0f, float maxY = 1.0f, float maxZ = 1.0f)
            : minX(minX), minY(minY), minZ(minZ),
              maxX(maxX), maxY(maxY), maxZ(maxZ) {}

        /**
         * @brief Check if bounds are full block
         * @return true if full block bounds
         */
        bool IsFullBlock() const {
            return minX == 0.0f && minY == 0.0f && minZ == 0.0f &&
                   maxX == 1.0f && maxY == 1.0f && maxZ == 1.0f;
        }

        /**
         * @brief Check if point is inside bounds
         * @param x X coordinate
         * @param y Y coordinate
         * @param z Z coordinate
         * @return true if inside bounds
         */
        bool Contains(float x, float y, float z) const {
            return x >= minX && x <= maxX &&
                   y >= minY && y <= maxY &&
                   z >= minZ && z <= maxZ;
        }
    };

    /**
     * @struct BlockDrop
     * @brief Item drop information for blocks
     */
    struct BlockDrop {
        BlockID itemId;             ///< Item ID to drop
        BlockMetadata metadata;     ///< Item metadata
        int minCount;              ///< Minimum drop count
        int maxCount;              ///< Maximum drop count
        float fortuneMultiplier;   ///< Fortune enchantment multiplier
        bool requiresTool;         ///< Requires specific tool to drop
        std::string requiredTool;  ///< Required tool type
        int requiredToolLevel;     ///< Required tool level

        BlockDrop(BlockID itemId = 0, BlockMetadata metadata = 0,
                 int minCount = 1, int maxCount = 1, float fortuneMultiplier = 1.0f)
            : itemId(itemId), metadata(metadata), minCount(minCount), maxCount(maxCount),
              fortuneMultiplier(fortuneMultiplier), requiresTool(false), requiredToolLevel(0) {}
    };

    /**
     * @struct BlockState
     * @brief Dynamic block state information
     */
    struct BlockState {
        BlockID blockId;           ///< Block ID
        BlockMetadata metadata;    ///< Block metadata
        double placementTime;      ///< When block was placed
        std::string placer;        ///< Who placed the block
        int lightLevel;            ///< Block light level
        int skyLightLevel;         ///< Sky light level
        bool isNatural;           ///< Naturally generated
        std::unordered_map<std::string, std::any> properties; ///< Custom properties
    };

    /**
     * @class Block
     * @brief Represents a block type in the voxel world
     *
     * A Block defines the properties, behavior, and appearance of a specific
     * type of block in the world. Each block type has its own characteristics
     * including textures, physics properties, sound effects, and behaviors.
     *
     * Key Features:
     * - Unique identification with ID and metadata
     * - Configurable textures for each face
     * - Physics properties (hardness, resistance, etc.)
     * - Custom collision bounds
     * - Drop tables and loot
     * - Interactive behaviors
     * - Sound effects and particles
     * - Animation support
     */
    class Block {
    public:
        /**
         * @brief Constructor
         * @param id Block ID
         * @param name Block name
         */
        Block(BlockID id, const std::string& name);

        /**
         * @brief Destructor
         */
        virtual ~Block();

        /**
         * @brief Deleted copy constructor
         */
        Block(const Block&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        Block& operator=(const Block&) = delete;

        // Block identification

        /**
         * @brief Get block ID
         * @return Block ID
         */
        BlockID GetID() const { return m_id; }

        /**
         * @brief Get block name
         * @return Block name
         */
        const std::string& GetName() const { return m_name; }

        /**
         * @brief Get block display name
         * @return Display name
         */
        virtual std::string GetDisplayName() const { return m_name; }

        /**
         * @brief Get block description
         * @return Block description
         */
        virtual std::string GetDescription() const { return "A block"; }

        // Block properties

        /**
         * @brief Get block textures
         * @return Block textures
         */
        const BlockTextures& GetTextures() const { return m_textures; }

        /**
         * @brief Set block textures
         * @param textures New textures
         */
        void SetTextures(const BlockTextures& textures) { m_textures = textures; }

        /**
         * @brief Get physics properties
         * @return Physics properties
         */
        const BlockPhysics& GetPhysics() const { return m_physics; }

        /**
         * @brief Set physics properties
         * @param physics New physics properties
         */
        void SetPhysics(const BlockPhysics& physics) { m_physics = physics; }

        /**
         * @brief Get collision bounds
         * @return Collision bounds
         */
        const BlockBounds& GetBounds() const { return m_bounds; }

        /**
         * @brief Set collision bounds
         * @param bounds New bounds
         */
        void SetBounds(const BlockBounds& bounds) { m_bounds = bounds; }

        /**
         * @brief Get render type
         * @return Render type
         */
        BlockRenderType GetRenderType() const { return m_renderType; }

        /**
         * @brief Set render type
         * @param type New render type
         */
        void SetRenderType(BlockRenderType type) { m_renderType = type; }

        /**
         * @brief Get material type
         * @return Material type
         */
        BlockMaterial GetMaterial() const { return m_material; }

        /**
         * @brief Set material type
         * @param material New material
         */
        void SetMaterial(BlockMaterial material) { m_material = material; }

        /**
         * @brief Get sound type
         * @return Sound type
         */
        BlockSoundType GetSoundType() const { return m_soundType; }

        /**
         * @brief Set sound type
         * @param soundType New sound type
         */
        void SetSoundType(BlockSoundType soundType) { m_soundType = soundType; }

        // Block state

        /**
         * @brief Get maximum metadata value
         * @return Maximum metadata
         */
        virtual BlockMetadata GetMaxMetadata() const { return 0; }

        /**
         * @brief Check if metadata is valid
         * @param metadata Metadata to check
         * @return true if valid, false otherwise
         */
        virtual bool IsValidMetadata(BlockMetadata metadata) const {
            return metadata <= GetMaxMetadata();
        }

        /**
         * @brief Get light level emitted by block
         * @param metadata Block metadata
         * @return Light level (0-15)
         */
        virtual int GetLightLevel(BlockMetadata metadata = 0) const { return 0; }

        /**
         * @brief Check if block is opaque
         * @param metadata Block metadata
         * @return true if opaque, false otherwise
         */
        virtual bool IsOpaque(BlockMetadata metadata = 0) const {
            return m_physics.isOpaque;
        }

        /**
         * @brief Check if block is solid
         * @param metadata Block metadata
         * @return true if solid, false otherwise
         */
        virtual bool IsSolid(BlockMetadata metadata = 0) const {
            return m_physics.isSolid;
        }

        // Block drops

        /**
         * @brief Get block drops
         * @param metadata Block metadata
         * @return Vector of possible drops
         */
        virtual std::vector<BlockDrop> GetDrops(BlockMetadata metadata = 0) const;

        /**
         * @brief Add drop to block
         * @param drop Drop information
         */
        void AddDrop(const BlockDrop& drop);

        /**
         * @brief Clear all drops
         */
        void ClearDrops();

        // Block behaviors

        /**
         * @brief Called when block is placed
         * @param world World instance
         * @param x Block X coordinate
         * @param y Block Y coordinate
         * @param z Block Z coordinate
         * @param player Player who placed the block
         * @param metadata Block metadata
         */
        virtual void OnPlace(World* world, int x, int y, int z,
                           Player* player = nullptr, BlockMetadata metadata = 0);

        /**
         * @brief Called when block is broken
         * @param world World instance
         * @param x Block X coordinate
         * @param y Block Y coordinate
         * @param z Block Z coordinate
         * @param player Player who broke the block
         * @param metadata Block metadata
         */
        virtual void OnBreak(World* world, int x, int y, int z,
                           Player* player = nullptr, BlockMetadata metadata = 0);

        /**
         * @brief Called when block is interacted with
         * @param world World instance
         * @param x Block X coordinate
         * @param y Block Y coordinate
         * @param z Block Z coordinate
         * @param player Player interacting with block
         * @param metadata Block metadata
         * @return true if interaction was handled, false otherwise
         */
        virtual bool OnInteract(World* world, int x, int y, int z,
                              Player* player, BlockMetadata metadata = 0);

        /**
         * @brief Called when block receives a random tick
         * @param world World instance
         * @param x Block X coordinate
         * @param y Block Y coordinate
         * @param z Block Z coordinate
         * @param metadata Block metadata
         */
        virtual void OnRandomTick(World* world, int x, int y, int z, BlockMetadata metadata = 0);

        /**
         * @brief Called when block receives a scheduled tick
         * @param world World instance
         * @param x Block X coordinate
         * @param y Block Y coordinate
         * @param z Block Z coordinate
         * @param metadata Block metadata
         */
        virtual void OnScheduledTick(World* world, int x, int y, int z, BlockMetadata metadata = 0);

        /**
         * @brief Called when neighboring block changes
         * @param world World instance
         * @param x Block X coordinate
         * @param y Block Y coordinate
         * @param z Block Z coordinate
         * @param neighborX Neighbor X coordinate
         * @param neighborY Neighbor Y coordinate
         * @param neighborZ Neighbor Z coordinate
         * @param metadata Block metadata
         */
        virtual void OnNeighborChange(World* world, int x, int y, int z,
                                    int neighborX, int neighborY, int neighborZ,
                                    BlockMetadata metadata = 0);

        /**
         * @brief Called when entity collides with block
         * @param world World instance
         * @param x Block X coordinate
         * @param y Block Y coordinate
         * @param z Block Z coordinate
         * @param entity Entity that collided
         * @param metadata Block metadata
         */
        virtual void OnEntityCollide(World* world, int x, int y, int z,
                                   Entity* entity, BlockMetadata metadata = 0);

        // Block utilities

        /**
         * @brief Get mining time for block
         * @param toolEfficiency Tool efficiency multiplier
         * @param metadata Block metadata
         * @return Mining time in seconds
         */
        virtual float GetMiningTime(float toolEfficiency = 1.0f, BlockMetadata metadata = 0) const;

        /**
         * @brief Get explosion resistance
         * @param metadata Block metadata
         * @return Explosion resistance
         */
        virtual float GetExplosionResistance(BlockMetadata metadata = 0) const {
            return m_physics.resistance;
        }

        /**
         * @brief Check if block can be replaced
         * @param metadata Block metadata
         * @return true if can be replaced, false otherwise
         */
        virtual bool CanBeReplaced(BlockMetadata metadata = 0) const {
            return m_material == BlockMaterial::Replaceable;
        }

        /**
         * @brief Check if block can be placed at position
         * @param world World instance
         * @param x Block X coordinate
         * @param y Block Y coordinate
         * @param z Block Z coordinate
         * @param player Player placing the block
         * @return true if can be placed, false otherwise
         */
        virtual bool CanPlaceAt(World* world, int x, int y, int z, Player* player = nullptr) const;

        // Serialization

        /**
         * @brief Serialize block data
         * @param data Output data map
         * @return true if successful, false otherwise
         */
        virtual bool Serialize(std::unordered_map<std::string, std::any>& data) const;

        /**
         * @brief Deserialize block data
         * @param data Input data map
         * @return true if successful, false otherwise
         */
        virtual bool Deserialize(const std::unordered_map<std::string, std::any>& data);

        // Static block creation

        /**
         * @brief Create default air block
         * @return Air block instance
         */
        static std::unique_ptr<Block> CreateAir();

        /**
         * @brief Create default stone block
         * @return Stone block instance
         */
        static std::unique_ptr<Block> CreateStone();

        /**
         * @brief Create default dirt block
         * @return Dirt block instance
         */
        static std::unique_ptr<Block> CreateDirt();

        /**
         * @brief Create default grass block
         * @return Grass block instance
         */
        static std::unique_ptr<Block> CreateGrass();

        /**
         * @brief Create default water block
         * @return Water block instance
         */
        static std::unique_ptr<Block> CreateWater();

    protected:
        /**
         * @brief Initialize default block properties
         */
        void InitializeDefaults();

        BlockID m_id;                              ///< Unique block ID
        std::string m_name;                        ///< Block name

        BlockTextures m_textures;                  ///< Block textures
        BlockPhysics m_physics;                    ///< Physics properties
        BlockBounds m_bounds;                      ///< Collision bounds
        BlockRenderType m_renderType;              ///< Render type
        BlockMaterial m_material;                  ///< Material type
        BlockSoundType m_soundType;                ///< Sound type

        std::vector<BlockDrop> m_drops;            ///< Block drops

        static BlockID s_nextBlockId;              ///< Next block ID counter
    };

    /**
     * @class BlockRegistry
     * @brief Central registry for all block types
     */
    class BlockRegistry {
    public:
        /**
         * @brief Register a block type
         * @param block Block to register
         * @return true if registered, false if ID already exists
         */
        static bool RegisterBlock(std::unique_ptr<Block> block);

        /**
         * @brief Get block by ID
         * @param id Block ID
         * @return Block pointer or nullptr if not found
         */
        static Block* GetBlock(BlockID id);

        /**
         * @brief Get block by name
         * @param name Block name
         * @return Block pointer or nullptr if not found
         */
        static Block* GetBlock(const std::string& name);

        /**
         * @brief Check if block is registered
         * @param id Block ID
         * @return true if registered, false otherwise
         */
        static bool IsRegistered(BlockID id);

        /**
         * @brief Check if block name is registered
         * @param name Block name
         * @return true if registered, false otherwise
         */
        static bool IsRegistered(const std::string& name);

        /**
         * @brief Get all registered blocks
         * @return Vector of block pointers
         */
        static std::vector<Block*> GetAllBlocks();

        /**
         * @brief Get number of registered blocks
         * @return Number of registered blocks
         */
        static size_t GetBlockCount();

        /**
         * @brief Clear all registered blocks
         */
        static void ClearRegistry();

        /**
         * @brief Initialize default blocks
         */
        static void InitializeDefaults();

        /**
         * @brief Get block ID by name
         * @param name Block name
         * @return Block ID or 0 if not found
         */
        static BlockID GetBlockId(const std::string& name);

        /**
         * @brief Get block name by ID
         * @param id Block ID
         * @return Block name or empty string if not found
         */
        static std::string GetBlockName(BlockID id);

    private:
        static std::unordered_map<BlockID, std::unique_ptr<Block>> s_blocks;
        static std::unordered_map<std::string, BlockID> s_nameToId;
        static std::unordered_map<BlockID, std::string> s_idToName;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_BLOCKS_BLOCK_HPP
