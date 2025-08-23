/**
 * @file Block.hpp
 * @brief VoxelCraft Block System - Core Block Definition
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * Block system inspired by Minecraft with voxel-based world building
 */

#ifndef VOXELCRAFT_BLOCKS_BLOCK_HPP
#define VOXELCRAFT_BLOCKS_BLOCK_HPP

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <array>
#include <functional>

namespace VoxelCraft {

    // Basic type definitions
    using BlockID = uint32_t;
    using BlockMetadata = uint16_t;

    // Forward declarations
    class BlockState;
    class BlockBehavior;
    struct Vec3;

    /**
     * @enum BlockType
     * @brief All block types in VoxelCraft (Minecraft-like)
     */
    enum class BlockType {
        // Basic blocks
        AIR = 0,
        STONE,
        GRASS_BLOCK,
        DIRT,
        COBBLESTONE,
        WOOD_PLANKS,
        SAPLING,
        BEDROCK,

        // Ores and minerals
        COAL_ORE,
        IRON_ORE,
        GOLD_ORE,
        DIAMOND_ORE,
        REDSTONE_ORE,
        LAPIS_ORE,
        EMERALD_ORE,

        // Wood and nature
        OAK_LOG,
        OAK_LEAVES,
        SPONGE,
        GLASS,
        OAK_STAIRS,
        COBBLESTONE_STAIRS,

        // Building blocks
        BRICKS,
        TNT,
        BOOKSHELF,
        MOSSY_COBBLESTONE,
        OBSIDIAN,
        SPAWNER,
        DIAMOND_BLOCK,
        GOLD_BLOCK,
        IRON_BLOCK,

        // Redstone components
        REDSTONE_WIRE,
        REDSTONE_TORCH,
        REDSTONE_LAMP,
        LEVER,
        STONE_BUTTON,
        WOODEN_BUTTON,
        STONE_PRESSURE_PLATE,
        WOODEN_PRESSURE_PLATE,
        TRIPWIRE_HOOK,

        // Mechanisms
        DISPENSER,
        NOTE_BLOCK,
        STICKY_PISTON,
        PISTON,
        TNT,
        LEVER,
        STONE_BUTTON,

        // Plants and crops
        WHEAT,
        CARROTS,
        POTATOES,
        BEETROOT,
        MELON_STEM,
        PUMPKIN_STEM,
        MELON,
        PUMPKIN,

        // Liquids
        WATER,
        LAVA,

        // Total block types
        BLOCK_TYPE_COUNT
    };

    /**
     * @enum BlockFace
     * @brief Six faces of a block (like Minecraft)
     */
    enum class BlockFace {
        BOTTOM = 0,  // -Y
        TOP,         // +Y
        NORTH,       // -Z
        SOUTH,       // +Z
        WEST,        // -X
        EAST         // +X
    };

    /**
     * @enum BlockHardness
     * @brief Mining hardness values (Minecraft-like)
     */
    enum class BlockHardness {
        INSTANT = 0,     // Air, plants
        VERY_SOFT = 1,   // Dirt, grass
        SOFT = 2,        // Wood, sand
        MEDIUM = 3,      // Stone, iron
        HARD = 4,        // Obsidian, diamond
        VERY_HARD = 5,   // Bedrock
        UNBREAKABLE = 6  // Admin blocks
    };

    /**
     * @struct BlockProperties
     * @brief Physical and behavioral properties of a block
     */
    struct BlockProperties {
        std::string name;                    ///< Display name
        std::string textureName;             ///< Base texture name
        BlockHardness hardness;              ///< Mining difficulty
        float resistance;                    ///< Explosion resistance
        float lightLevel;                    ///< Light emitted (0-15)
        float lightOpacity;                  ///< Light blocked (0-15)
        bool isSolid;                        ///< Blocks movement
        bool isTransparent;                  ///< Allows light through
        bool isFlammable;                    ///< Can catch fire
        bool requiresTool;                   ///< Needs specific tool to mine
        std::string requiredTool;            ///< Tool needed to mine efficiently
        int harvestLevel;                    ///< Tool level required
        bool canHarvest;                     ///< Can be harvested
        int maxStackSize;                    ///< Maximum stack size in inventory
        bool hasGravity;                     ///< Falls when unsupported
        float slipperiness;                  ///< Slipperiness factor
        std::vector<BlockType> dropItems;    ///< Items dropped when broken
        std::vector<int> dropQuantities;     ///< Drop quantities

        BlockProperties()
            : name("Unknown Block")
            , textureName("unknown")
            , hardness(BlockHardness::MEDIUM)
            , resistance(1.0f)
            , lightLevel(0.0f)
            , lightOpacity(15.0f)
            , isSolid(true)
            , isTransparent(false)
            , isFlammable(false)
            , requiresTool(false)
            , requiredTool("none")
            , harvestLevel(0)
            , canHarvest(true)
            , maxStackSize(64)
            , hasGravity(false)
            , slipperiness(0.6f)
        {}
    };

    /**
     * @class Block
     * @brief Represents a single block type in the voxel world
     */
    class Block {
    public:
        /**
         * @brief Constructor
         * @param type Block type
         * @param properties Block properties
         */
        Block(BlockType type, const BlockProperties& properties);

        /**
         * @brief Get block type
         * @return Block type
         */
        BlockType GetType() const { return m_type; }

        /**
         * @brief Get block properties
         * @return Block properties
         */
        const BlockProperties& GetProperties() const { return m_properties; }

        /**
         * @brief Get block name
         * @return Block name
         */
        const std::string& GetName() const { return m_properties.name; }

        /**
         * @brief Check if block is solid
         * @return true if solid
         */
        bool IsSolid() const { return m_properties.isSolid; }

        /**
         * @brief Check if block is transparent
         * @return true if transparent
         */
        bool IsTransparent() const { return m_properties.isTransparent; }

        /**
         * @brief Get light level emitted by block
         * @return Light level (0-15)
         */
        float GetLightLevel() const { return m_properties.lightLevel; }

        /**
         * @brief Get light opacity (how much light is blocked)
         * @return Light opacity (0-15)
         */
        float GetLightOpacity() const { return m_properties.lightOpacity; }

        /**
         * @brief Get mining hardness
         * @return Block hardness
         */
        BlockHardness GetHardness() const { return m_properties.hardness; }

        /**
         * @brief Check if block can be harvested
         * @return true if can be harvested
         */
        bool CanHarvest() const { return m_properties.canHarvest; }

        /**
         * @brief Get maximum stack size
         * @return Maximum stack size
         */
        int GetMaxStackSize() const { return m_properties.maxStackSize; }

        /**
         * @brief Get texture name for specific face
         * @param face Block face
         * @return Texture name
         */
        std::string GetTextureName(BlockFace face = BlockFace::NORTH) const;

        /**
         * @brief Set custom texture for face
         * @param face Block face
         * @param textureName Texture name
         */
        void SetTextureName(BlockFace face, const std::string& textureName);

        /**
         * @brief Get drop items when block is broken
         * @return Vector of block types dropped
         */
        const std::vector<BlockType>& GetDropItems() const { return m_properties.dropItems; }

        /**
         * @brief Get drop quantities
         * @return Vector of drop quantities
         */
        const std::vector<int>& GetDropQuantities() const { return m_properties.dropQuantities; }

        /**
         * @brief Check if block has gravity
         * @return true if block has gravity
         */
        bool HasGravity() const { return m_properties.hasGravity; }

        /**
         * @brief Get slipperiness factor
         * @return Slipperiness factor
         */
        float GetSlipperiness() const { return m_properties.slipperiness; }

        /**
         * @brief Create default block of specified type
         * @param type Block type
         * @return New block instance
         */
        static std::shared_ptr<Block> CreateBlock(BlockType type);

    private:
        BlockType m_type;                                          ///< Block type identifier
        BlockProperties m_properties;                              ///< Block properties
        std::array<std::string, 6> m_faceTextures;                ///< Textures for each face

        /**
         * @brief Initialize face textures with default texture
         */
        void InitializeFaceTextures();
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_BLOCKS_BLOCK_HPP