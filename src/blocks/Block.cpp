/**
 * @file Block.cpp
 * @brief VoxelCraft Block System - Block Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "Block.hpp"
#include <stdexcept>

namespace VoxelCraft {

    Block::Block(BlockType type, const BlockProperties& properties)
        : m_type(type)
        , m_properties(properties)
    {
        InitializeFaceTextures();
    }

    std::string Block::GetTextureName(BlockFace face) const {
        return m_faceTextures[static_cast<size_t>(face)];
    }

    void Block::SetTextureName(BlockFace face, const std::string& textureName) {
        m_faceTextures[static_cast<size_t>(face)] = textureName;
    }

    void Block::InitializeFaceTextures() {
        // Set all faces to default texture initially
        for (auto& texture : m_faceTextures) {
            texture = m_properties.textureName;
        }

        // Special handling for blocks with different textures per face
        switch (m_type) {
            case BlockType::GRASS_BLOCK:
                m_faceTextures[static_cast<size_t>(BlockFace::TOP)] = "grass_block_top";
                m_faceTextures[static_cast<size_t>(BlockFace::BOTTOM)] = "dirt";
                m_faceTextures[static_cast<size_t>(BlockFace::NORTH)] = "grass_block_side";
                m_faceTextures[static_cast<size_t>(BlockFace::SOUTH)] = "grass_block_side";
                m_faceTextures[static_cast<size_t>(BlockFace::WEST)] = "grass_block_side";
                m_faceTextures[static_cast<size_t>(BlockFace::EAST)] = "grass_block_side";
                break;

            case BlockType::OAK_LOG:
                m_faceTextures[static_cast<size_t>(BlockFace::TOP)] = "oak_log_top";
                m_faceTextures[static_cast<size_t>(BlockFace::BOTTOM)] = "oak_log_top";
                m_faceTextures[static_cast<size_t>(BlockFace::NORTH)] = "oak_log";
                m_faceTextures[static_cast<size_t>(BlockFace::SOUTH)] = "oak_log";
                m_faceTextures[static_cast<size_t>(BlockFace::WEST)] = "oak_log";
                m_faceTextures[static_cast<size_t>(BlockFace::EAST)] = "oak_log";
                break;

            case BlockType::OAK_LEAVES:
                // All faces use the same texture
                break;

            default:
                // Use default texture for all faces
                break;
        }
    }

    std::shared_ptr<Block> Block::CreateBlock(BlockType type) {
        BlockProperties props;

        switch (type) {
            case BlockType::AIR:
                props.name = "Air";
                props.textureName = "air";
                props.hardness = BlockHardness::INSTANT;
                props.isSolid = false;
                props.isTransparent = true;
                props.lightOpacity = 0.0f;
                props.canHarvest = false;
                props.maxStackSize = 0;
                break;

            case BlockType::STONE:
                props.name = "Stone";
                props.textureName = "stone";
                props.hardness = BlockHardness::MEDIUM;
                props.resistance = 6.0f;
                props.dropItems = {BlockType::COBBLESTONE};
                props.dropQuantities = {1};
                break;

            case BlockType::GRASS_BLOCK:
                props.name = "Grass Block";
                props.textureName = "grass_block_side";
                props.hardness = BlockHardness::VERY_SOFT;
                props.resistance = 0.6f;
                props.dropItems = {BlockType::DIRT};
                props.dropQuantities = {1};
                break;

            case BlockType::DIRT:
                props.name = "Dirt";
                props.textureName = "dirt";
                props.hardness = BlockHardness::VERY_SOFT;
                props.resistance = 0.5f;
                props.dropItems = {BlockType::DIRT};
                props.dropQuantities = {1};
                break;

            case BlockType::COBBLESTONE:
                props.name = "Cobblestone";
                props.textureName = "cobblestone";
                props.hardness = BlockHardness::MEDIUM;
                props.resistance = 6.0f;
                props.dropItems = {BlockType::COBBLESTONE};
                props.dropQuantities = {1};
                break;

            case BlockType::WOOD_PLANKS:
                props.name = "Oak Planks";
                props.textureName = "oak_planks";
                props.hardness = BlockHardness::SOFT;
                props.resistance = 3.0f;
                props.isFlammable = true;
                props.dropItems = {BlockType::WOOD_PLANKS};
                props.dropQuantities = {1};
                break;

            case BlockType::SAPLING:
                props.name = "Oak Sapling";
                props.textureName = "oak_sapling";
                props.hardness = BlockHardness::INSTANT;
                props.isSolid = false;
                props.isTransparent = true;
                props.lightOpacity = 0.0f;
                props.dropItems = {BlockType::SAPLING};
                props.dropQuantities = {1};
                break;

            case BlockType::BEDROCK:
                props.name = "Bedrock";
                props.textureName = "bedrock";
                props.hardness = BlockHardness::UNBREAKABLE;
                props.resistance = 3600000.0f;
                props.canHarvest = false;
                break;

            case BlockType::COAL_ORE:
                props.name = "Coal Ore";
                props.textureName = "coal_ore";
                props.hardness = BlockHardness::MEDIUM;
                props.resistance = 3.0f;
                props.requiresTool = true;
                props.requiredTool = "pickaxe";
                props.harvestLevel = 1;
                props.dropItems = {BlockType::COAL_ORE}; // In real game, drops coal item
                props.dropQuantities = {1};
                break;

            case BlockType::IRON_ORE:
                props.name = "Iron Ore";
                props.textureName = "iron_ore";
                props.hardness = BlockHardness::MEDIUM;
                props.resistance = 3.0f;
                props.requiresTool = true;
                props.requiredTool = "pickaxe";
                props.harvestLevel = 2;
                props.dropItems = {BlockType::IRON_ORE}; // In real game, drops iron ore item
                props.dropQuantities = {1};
                break;

            case BlockType::GOLD_ORE:
                props.name = "Gold Ore";
                props.textureName = "gold_ore";
                props.hardness = BlockHardness::MEDIUM;
                props.resistance = 3.0f;
                props.requiresTool = true;
                props.requiredTool = "pickaxe";
                props.harvestLevel = 3;
                props.dropItems = {BlockType::GOLD_ORE}; // In real game, drops gold ore item
                props.dropQuantities = {1};
                break;

            case BlockType::DIAMOND_ORE:
                props.name = "Diamond Ore";
                props.textureName = "diamond_ore";
                props.hardness = BlockHardness::MEDIUM;
                props.resistance = 3.0f;
                props.requiresTool = true;
                props.requiredTool = "pickaxe";
                props.harvestLevel = 3;
                props.dropItems = {BlockType::DIAMOND_ORE}; // In real game, drops diamond item
                props.dropQuantities = {1};
                break;

            case BlockType::OAK_LOG:
                props.name = "Oak Log";
                props.textureName = "oak_log";
                props.hardness = BlockHardness::SOFT;
                props.resistance = 2.0f;
                props.isFlammable = true;
                props.dropItems = {BlockType::OAK_LOG};
                props.dropQuantities = {1};
                break;

            case BlockType::OAK_LEAVES:
                props.name = "Oak Leaves";
                props.textureName = "oak_leaves";
                props.hardness = BlockHardness::INSTANT;
                props.isSolid = false;
                props.isTransparent = true;
                props.lightOpacity = 1.0f;
                props.isFlammable = true;
                props.dropItems = {BlockType::OAK_LEAVES}; // Can also drop saplings
                props.dropQuantities = {1};
                break;

            case BlockType::GLASS:
                props.name = "Glass";
                props.textureName = "glass";
                props.hardness = BlockHardness::VERY_SOFT;
                props.resistance = 0.3f;
                props.isSolid = true;
                props.isTransparent = true;
                props.lightOpacity = 0.0f;
                props.dropItems = {BlockType::GLASS};
                props.dropQuantities = {1};
                break;

            case BlockType::BRICKS:
                props.name = "Bricks";
                props.textureName = "bricks";
                props.hardness = BlockHardness::MEDIUM;
                props.resistance = 6.0f;
                props.dropItems = {BlockType::BRICKS};
                props.dropQuantities = {1};
                break;

            case BlockType::TNT:
                props.name = "TNT";
                props.textureName = "tnt";
                props.hardness = BlockHardness::INSTANT;
                props.isFlammable = true;
                props.dropItems = {BlockType::TNT};
                props.dropQuantities = {1};
                break;

            case BlockType::BOOKSHELF:
                props.name = "Bookshelf";
                props.textureName = "bookshelf";
                props.hardness = BlockHardness::SOFT;
                props.resistance = 1.5f;
                props.isFlammable = true;
                props.dropItems = {BlockType::BOOKSHELF}; // Drops books and planks in real game
                props.dropQuantities = {1};
                break;

            case BlockType::OBSIDIAN:
                props.name = "Obsidian";
                props.textureName = "obsidian";
                props.hardness = BlockHardness::HARD;
                props.resistance = 1200.0f;
                props.requiresTool = true;
                props.requiredTool = "pickaxe";
                props.harvestLevel = 3;
                props.dropItems = {BlockType::OBSIDIAN};
                props.dropQuantities = {1};
                break;

            case BlockType::DIAMOND_BLOCK:
                props.name = "Diamond Block";
                props.textureName = "diamond_block";
                props.hardness = BlockHardness::HARD;
                props.resistance = 30.0f;
                props.requiresTool = true;
                props.requiredTool = "pickaxe";
                props.harvestLevel = 3;
                props.dropItems = {BlockType::DIAMOND_BLOCK}; // In real game, drops diamond items
                props.dropQuantities = {1};
                break;

            case BlockType::GOLD_BLOCK:
                props.name = "Gold Block";
                props.textureName = "gold_block";
                props.hardness = BlockHardness::MEDIUM;
                props.resistance = 30.0f;
                props.requiresTool = true;
                props.requiredTool = "pickaxe";
                props.harvestLevel = 3;
                props.dropItems = {BlockType::GOLD_BLOCK}; // In real game, drops gold ingots
                props.dropQuantities = {1};
                break;

            case BlockType::IRON_BLOCK:
                props.name = "Iron Block";
                props.textureName = "iron_block";
                props.hardness = BlockHardness::MEDIUM;
                props.resistance = 30.0f;
                props.requiresTool = true;
                props.requiredTool = "pickaxe";
                props.harvestLevel = 2;
                props.dropItems = {BlockType::IRON_BLOCK}; // In real game, drops iron ingots
                props.dropQuantities = {1};
                break;

            case BlockType::WATER:
                props.name = "Water";
                props.textureName = "water";
                props.hardness = BlockHardness::UNBREAKABLE;
                props.isSolid = false;
                props.isTransparent = true;
                props.lightOpacity = 2.0f;
                props.canHarvest = false;
                break;

            case BlockType::LAVA:
                props.name = "Lava";
                props.textureName = "lava";
                props.hardness = BlockHardness::UNBREAKABLE;
                props.isSolid = false;
                props.isTransparent = true;
                props.lightOpacity = 0.0f;
                props.lightLevel = 15.0f;
                props.canHarvest = false;
                break;

            case BlockType::SPAWNER:
                props.name = "Spawner";
                props.textureName = "spawner";
                props.hardness = BlockHardness::HARD;
                props.resistance = 25.0f;
                props.requiresTool = true;
                props.requiredTool = "pickaxe";
                props.harvestLevel = 3;
                props.dropItems = {}; // Doesn't drop anything
                props.dropQuantities = {};
                break;

            case BlockType::MOSSY_COBBLESTONE:
                props.name = "Mossy Cobblestone";
                props.textureName = "mossy_cobblestone";
                props.hardness = BlockHardness::MEDIUM;
                props.resistance = 6.0f;
                props.dropItems = {BlockType::MOSSY_COBBLESTONE};
                props.dropQuantities = {1};
                break;

            default:
                props.name = "Unknown Block";
                props.textureName = "unknown";
                props.hardness = BlockHardness::MEDIUM;
                props.dropItems = {type};
                props.dropQuantities = {1};
                break;
        }

        return std::make_shared<Block>(type, props);
    }

} // namespace VoxelCraft

