/**
 * @file BlockGenerator.cpp
 * @brief VoxelCraft Procedural Block Generator - Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "BlockGenerator.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace VoxelCraft {

    BlockGenerator::BlockGenerator() {
        m_seed = std::chrono::system_clock::now().time_since_epoch().count();
        m_randomEngine.seed(m_seed);
    }

    BlockGenerator::~BlockGenerator() {
        // Cleanup any generated data
    }

    void BlockGenerator::Initialize(uint64_t seed) {
        m_seed = seed;
        m_randomEngine.seed(seed);
        InitializeBlockTemplates();
        InitializeGenerators();
        InitializeTextureGenerators();
        InitializeModelGenerators();
        InitializeSoundGenerators();
        InitializeParticleGenerators();
    }

    void BlockGenerator::InitializeBlockTemplates() {
        // Stone block template
        BlockProperties stoneProps;
        stoneProps.hardness = 1.5f;
        stoneProps.resistance = 6.0f;
        stoneProps.lightLevel = 0.0f;
        stoneProps.friction = 0.6f;
        stoneProps.slipperiness = 0.6f;
        stoneProps.isSolid = true;
        stoneProps.isOpaque = true;
        stoneProps.isTransparent = false;
        stoneProps.isFlammable = false;
        stoneProps.isReplaceable = false;
        stoneProps.requiresTool = true;
        stoneProps.hasGravity = false;
        stoneProps.emitsParticles = true;
        stoneProps.hasAnimation = false;
        stoneProps.miningLevel = 0;
        stoneProps.requiredTool = "pickaxe";
        stoneProps.drops = {"cobblestone"};
        m_blockPropertiesTemplates["stone"] = stoneProps;

        // Dirt block template
        BlockProperties dirtProps;
        dirtProps.hardness = 0.5f;
        dirtProps.resistance = 0.5f;
        dirtProps.lightLevel = 0.0f;
        dirtProps.friction = 0.6f;
        dirtProps.slipperiness = 0.6f;
        dirtProps.isSolid = true;
        dirtProps.isOpaque = true;
        dirtProps.isTransparent = false;
        dirtProps.isFlammable = false;
        dirtProps.isReplaceable = true;
        dirtProps.requiresTool = false;
        dirtProps.hasGravity = false;
        dirtProps.emitsParticles = true;
        dirtProps.hasAnimation = false;
        dirtProps.miningLevel = 0;
        dirtProps.requiredTool = "shovel";
        dirtProps.drops = {"dirt"};
        m_blockPropertiesTemplates["dirt"] = dirtProps;

        // Grass block template
        BlockProperties grassProps;
        grassProps.hardness = 0.6f;
        grassProps.resistance = 0.6f;
        grassProps.lightLevel = 0.0f;
        grassProps.friction = 0.6f;
        grassProps.slipperiness = 0.6f;
        grassProps.isSolid = true;
        grassProps.isOpaque = true;
        grassProps.isTransparent = false;
        grassProps.isFlammable = true;
        grassProps.isReplaceable = false;
        grassProps.requiresTool = false;
        grassProps.hasGravity = false;
        grassProps.emitsParticles = true;
        grassProps.hasAnimation = false;
        grassProps.miningLevel = 0;
        grassProps.requiredTool = "shovel";
        grassProps.drops = {"dirt"};
        m_blockPropertiesTemplates["grass"] = grassProps;

        // Wood block template
        BlockProperties woodProps;
        woodProps.hardness = 2.0f;
        woodProps.resistance = 2.0f;
        woodProps.lightLevel = 0.0f;
        woodProps.friction = 0.6f;
        woodProps.slipperiness = 0.6f;
        woodProps.isSolid = true;
        woodProps.isOpaque = true;
        woodProps.isTransparent = false;
        woodProps.isFlammable = true;
        woodProps.isReplaceable = false;
        woodProps.requiresTool = false;
        woodProps.hasGravity = false;
        woodProps.emitsParticles = true;
        woodProps.hasAnimation = false;
        woodProps.miningLevel = 0;
        woodProps.requiredTool = "axe";
        woodProps.drops = {"wood_planks"};
        m_blockPropertiesTemplates["wood"] = woodProps;

        // Ore block template
        BlockProperties oreProps;
        oreProps.hardness = 3.0f;
        oreProps.resistance = 3.0f;
        oreProps.lightLevel = 0.0f;
        oreProps.friction = 0.6f;
        oreProps.slipperiness = 0.6f;
        oreProps.isSolid = true;
        oreProps.isOpaque = true;
        oreProps.isTransparent = false;
        oreProps.isFlammable = false;
        oreProps.isReplaceable = false;
        oreProps.requiresTool = true;
        oreProps.hasGravity = false;
        oreProps.emitsParticles = true;
        oreProps.hasAnimation = false;
        oreProps.miningLevel = 1;
        oreProps.requiredTool = "pickaxe";
        oreProps.drops = {"ore_item"};
        m_blockPropertiesTemplates["ore"] = oreProps;

        // Crystal block template
        BlockProperties crystalProps;
        crystalProps.hardness = 1.0f;
        crystalProps.resistance = 2.0f;
        crystalProps.lightLevel = 0.5f;
        crystalProps.friction = 0.6f;
        crystalProps.slipperiness = 0.6f;
        crystalProps.isSolid = true;
        crystalProps.isOpaque = false;
        crystalProps.isTransparent = true;
        crystalProps.isFlammable = false;
        crystalProps.isReplaceable = false;
        crystalProps.requiresTool = true;
        crystalProps.hasGravity = false;
        crystalProps.emitsParticles = true;
        crystalProps.hasAnimation = true;
        crystalProps.miningLevel = 2;
        crystalProps.requiredTool = "pickaxe";
        crystalProps.drops = {"crystal_shard"};
        crystalProps.specialEffects = {"glowing", "magical"};
        m_blockPropertiesTemplates["crystal"] = crystalProps;
    }

    void BlockGenerator::InitializeGenerators() {
        // Terrain generators
        m_terrainGenerators["stone"] = [this](const std::string& biome, int depth, float temp, float humidity) -> GeneratedBlock {
            return GenerateStoneBlock(biome, depth, temp, humidity);
        };

        m_terrainGenerators["dirt"] = [this](const std::string& biome, int depth, float temp, float humidity) -> GeneratedBlock {
            return GenerateDirtBlock(biome, depth, temp, humidity);
        };

        m_terrainGenerators["grass"] = [this](const std::string& biome, int depth, float temp, float humidity) -> GeneratedBlock {
            return GenerateGrassBlock(biome, depth, temp, humidity);
        };

        m_terrainGenerators["sand"] = [this](const std::string& biome, int depth, float temp, float humidity) -> GeneratedBlock {
            return GenerateSandBlock(biome, depth, temp, humidity);
        };

        m_terrainGenerators["gravel"] = [this](const std::string& biome, int depth, float temp, float humidity) -> GeneratedBlock {
            return GenerateGravelBlock(biome, depth, temp, humidity);
        };

        m_terrainGenerators["clay"] = [this](const std::string& biome, int depth, float temp, float humidity) -> GeneratedBlock {
            return GenerateClayBlock(biome, depth, temp, humidity);
        };

        m_terrainGenerators["snow"] = [this](const std::string& biome, int depth, float temp, float humidity) -> GeneratedBlock {
            return GenerateSnowBlock(biome, depth, temp, humidity);
        };

        // Ore generators
        m_oreGenerators["coal"] = [this](const std::string& oreType, const std::string& biome, int depth) -> GeneratedBlock {
            return GenerateCoalOreBlock(biome, depth);
        };

        m_oreGenerators["iron"] = [this](const std::string& oreType, const std::string& biome, int depth) -> GeneratedBlock {
            return GenerateIronOreBlock(biome, depth);
        };

        m_oreGenerators["gold"] = [this](const std::string& oreType, const std::string& biome, int depth) -> GeneratedBlock {
            return GenerateGoldOreBlock(biome, depth);
        };

        m_oreGenerators["diamond"] = [this](const std::string& oreType, const std::string& biome, int depth) -> GeneratedBlock {
            return GenerateDiamondOreBlock(biome, depth);
        };

        m_oreGenerators["redstone"] = [this](const std::string& oreType, const std::string& biome, int depth) -> GeneratedBlock {
            return GenerateRedstoneOreBlock(biome, depth);
        };

        m_oreGenerators["lapis"] = [this](const std::string& oreType, const std::string& biome, int depth) -> GeneratedBlock {
            return GenerateLapisOreBlock(biome, depth);
        };

        m_oreGenerators["emerald"] = [this](const std::string& oreType, const std::string& biome, int depth) -> GeneratedBlock {
            return GenerateEmeraldOreBlock(biome, depth);
        };

        // Vegetation generators
        m_vegetationGenerators["oak"] = [this](const std::string& plantType, const std::string& biome, const std::string& season) -> GeneratedBlock {
            return GenerateTreeBlock("oak", biome, season);
        };

        m_vegetationGenerators["spruce"] = [this](const std::string& plantType, const std::string& biome, const std::string& season) -> GeneratedBlock {
            return GenerateTreeBlock("spruce", biome, season);
        };

        m_vegetationGenerators["birch"] = [this](const std::string& plantType, const std::string& biome, const std::string& season) -> GeneratedBlock {
            return GenerateTreeBlock("birch", biome, season);
        };

        m_vegetationGenerators["rose"] = [this](const std::string& plantType, const std::string& biome, const std::string& season) -> GeneratedBlock {
            return GenerateFlowerBlock("rose", biome, season);
        };

        m_vegetationGenerators["tulip"] = [this](const std::string& plantType, const std::string& biome, const std::string& season) -> GeneratedBlock {
            return GenerateFlowerBlock("tulip", biome, season);
        };

        m_vegetationGenerators["grass"] = [this](const std::string& plantType, const std::string& biome, const std::string& season) -> GeneratedBlock {
            return GenerateGrassPlantBlock(biome, season);
        };

        // Structure generators
        m_structureGenerators["wood"] = [this](const std::string& structureType, const std::string& material) -> GeneratedBlock {
            return GenerateWoodBlock("oak", material);
        };

        m_structureGenerators["brick"] = [this](const std::string& structureType, const std::string& material) -> GeneratedBlock {
            return GenerateBrickBlock("red", material);
        };

        m_structureGenerators["stone_brick"] = [this](const std::string& structureType, const std::string& material) -> GeneratedBlock {
            return GenerateStoneBrickBlock("normal", material);
        };

        m_structureGenerators["glass"] = [this](const std::string& structureType, const std::string& material) -> GeneratedBlock {
            return GenerateGlassBlock("clear", material);
        };

        // Magical generators
        m_magicalGenerators["crystal"] = [this](const std::string& magicType, int powerLevel) -> GeneratedBlock {
            return GenerateCrystalBlock("power", powerLevel);
        };

        m_magicalGenerators["runestone"] = [this](const std::string& magicType, int powerLevel) -> GeneratedBlock {
            return GenerateRunestoneBlock("ancient", powerLevel);
        };

        m_magicalGenerators["obsidian"] = [this](const std::string& magicType, int powerLevel) -> GeneratedBlock {
            return GenerateObsidianBlock("infused", powerLevel);
        };
    }

    void BlockGenerator::InitializeTextureGenerators() {
        m_textureGenerators[BlockTextureType::SOLID] = [this](const GeneratedBlock& block) -> void* {
            return GenerateSolidTexture(block);
        };

        m_textureGenerators[BlockTextureType::NOISE] = [this](const GeneratedBlock& block) -> void* {
            return GenerateNoiseTexture(block);
        };

        m_textureGenerators[BlockTextureType::GRADIENT] = [this](const GeneratedBlock& block) -> void* {
            return GenerateGradientTexture(block);
        };

        m_textureGenerators[BlockTextureType::PATTERN] = [this](const GeneratedBlock& block) -> void* {
            return GeneratePatternTexture(block);
        };

        m_textureGenerators[BlockTextureType::CRYSTAL] = [this](const GeneratedBlock& block) -> void* {
            return GenerateCrystalTexture(block);
        };

        m_textureGenerators[BlockTextureType::ORGANIC] = [this](const GeneratedBlock& block) -> void* {
            return GenerateOrganicTexture(block);
        };

        m_textureGenerators[BlockTextureType::METAL] = [this](const GeneratedBlock& block) -> void* {
            return GenerateMetalTexture(block);
        };

        m_textureGenerators[BlockTextureType::STONE] = [this](const GeneratedBlock& block) -> void* {
            return GenerateStoneTexture(block);
        };

        m_textureGenerators[BlockTextureType::WOOD] = [this](const GeneratedBlock& block) -> void* {
            return GenerateWoodTexture(block);
        };

        m_textureGenerators[BlockTextureType::LEAF] = [this](const GeneratedBlock& block) -> void* {
            return GenerateLeafTexture(block);
        };

        m_textureGenerators[BlockTextureType::FLUID] = [this](const GeneratedBlock& block) -> void* {
            return GenerateFluidTexture(block);
        };
    }

    void BlockGenerator::InitializeModelGenerators() {
        m_modelGenerators["standard"] = [this](const GeneratedBlock& block) -> void* {
            return GenerateStandardBlockModel(block);
        };

        m_modelGenerators["complex"] = [this](const GeneratedBlock& block) -> void* {
            return GenerateComplexBlockModel(block);
        };

        m_modelGenerators["vegetation"] = [this](const GeneratedBlock& block) -> void* {
            return GenerateVegetationModel(block);
        };

        m_modelGenerators["crystal"] = [this](const GeneratedBlock& block) -> void* {
            return GenerateCrystalModel(block);
        };

        m_modelGenerators["magical"] = [this](const GeneratedBlock& block) -> void* {
            return GenerateMagicalModel(block);
        };
    }

    void BlockGenerator::InitializeSoundGenerators() {
        m_soundGenerators["stone"] = [this](const GeneratedBlock& block) -> void* {
            return GenerateStoneBlockSounds(block);
        };

        m_soundGenerators["wood"] = [this](const GeneratedBlock& block) -> void* {
            return GenerateWoodBlockSounds(block);
        };

        m_soundGenerators["metal"] = [this](const GeneratedBlock& block) -> void* {
            return GenerateMetalBlockSounds(block);
        };

        m_soundGenerators["organic"] = [this](const GeneratedBlock& block) -> void* {
            return GenerateOrganicBlockSounds(block);
        };

        m_soundGenerators["magical"] = [this](const GeneratedBlock& block) -> void* {
            return GenerateMagicalBlockSounds(block);
        };
    }

    void BlockGenerator::InitializeParticleGenerators() {
        m_particleGenerators["stone"] = [this](const GeneratedBlock& block) -> void* {
            return GenerateStoneParticles(block);
        };

        m_particleGenerators["wood"] = [this](const GeneratedBlock& block) -> void* {
            return GenerateWoodParticles(block);
        };

        m_particleGenerators["leaf"] = [this](const GeneratedBlock& block) -> void* {
            return GenerateLeafParticles(block);
        };

        m_particleGenerators["crystal"] = [this](const GeneratedBlock& block) -> void* {
            return GenerateCrystalParticles(block);
        };

        m_particleGenerators["magical"] = [this](const GeneratedBlock& block) -> void* {
            return GenerateMagicalParticles(block);
        };
    }

    GeneratedBlock BlockGenerator::GenerateTerrainBlock(const std::string& biome, int depth, float temperature, float humidity) {
        std::vector<std::string> possibleBlocks;

        // Determine blocks based on biome and depth
        if (depth == 0) { // Surface
            if (biome == "plains" || biome == "forest") {
                possibleBlocks = {"grass"};
            } else if (biome == "desert") {
                possibleBlocks = {"sand"};
            } else if (biome == "snow") {
                possibleBlocks = {"snow"};
            } else if (biome == "swamp") {
                possibleBlocks = {"grass", "clay"};
            } else {
                possibleBlocks = {"grass"};
            }
        } else if (depth <= 3) { // Near surface
            if (biome == "desert") {
                possibleBlocks = {"sand", "sandstone"};
            } else if (biome == "snow") {
                possibleBlocks = {"dirt", "snow"};
            } else {
                possibleBlocks = {"dirt"};
            }
        } else { // Underground
            possibleBlocks = {"stone"};
        }

        std::uniform_int_distribution<size_t> dist(0, possibleBlocks.size() - 1);
        std::string selectedBlock = possibleBlocks[dist(m_randomEngine)];

        auto it = m_terrainGenerators.find(selectedBlock);
        if (it != m_terrainGenerators.end()) {
            GeneratedBlock block = it->second(biome, depth, temperature, humidity);
            ApplyBiomeModifications(block, biome);
            ApplyDepthModifications(block, depth);
            ApplyTemperatureEffects(block, temperature);
            return block;
        }

        // Fallback to stone
        return GenerateStoneBlock(biome, depth, temperature, humidity);
    }

    GeneratedBlock BlockGenerator::GenerateOreBlock(const std::string& oreType, const std::string& biome, int depth) {
        auto it = m_oreGenerators.find(oreType);
        if (it != m_oreGenerators.end()) {
            GeneratedBlock block = it->second(oreType, biome, depth);
            ApplyDepthModifications(block, depth);
            return block;
        }

        return GenerateCustomOreBlock(oreType, biome, depth);
    }

    GeneratedBlock BlockGenerator::GenerateVegetationBlock(const std::string& plantType, const std::string& biome, const std::string& season) {
        auto it = m_vegetationGenerators.find(plantType);
        if (it != m_vegetationGenerators.end()) {
            return it->second(plantType, biome, season);
        }

        // Fallback to grass
        return GenerateGrassPlantBlock(biome, season);
    }

    GeneratedBlock BlockGenerator::GenerateStructureBlock(const std::string& structureType, const std::string& material) {
        auto it = m_structureGenerators.find(structureType);
        if (it != m_structureGenerators.end()) {
            return it->second(structureType, material);
        }

        // Fallback to wood
        return GenerateWoodBlock("oak", material);
    }

    GeneratedBlock BlockGenerator::GenerateMagicalBlock(const std::string& magicType, int powerLevel) {
        auto it = m_magicalGenerators.find(magicType);
        if (it != m_magicalGenerators.end()) {
            return it->second(magicType, powerLevel);
        }

        // Fallback to crystal
        return GenerateCrystalBlock("power", powerLevel);
    }

    GeneratedBlock BlockGenerator::GenerateCustomBlock(const std::string& customType, const std::unordered_map<std::string, std::string>& parameters) {
        GeneratedBlock block;
        block.type = BlockGenerationType::CUSTOM;
        block.name = customType;
        block.displayName = "Custom " + customType + " Block";
        block.materialType = customType;

        // Generate unique ID
        std::stringstream ss;
        ss << "custom_" << customType << "_" << std::hex << std::hash<std::string>()(customType);
        block.id = ss.str();

        // Default properties
        block.properties = m_blockPropertiesTemplates["stone"];
        block.appearance.textureType = BlockTextureType::CUSTOM;
        block.appearance.variant = std::uniform_int_distribution<int>(0, 100)(m_randomEngine);

        // Apply custom parameters
        for (const auto& param : parameters) {
            if (param.first == "hardness") {
                block.properties.hardness = std::stof(param.second);
            } else if (param.first == "color") {
                // Parse color
                std::istringstream iss(param.second);
                std::string token;
                std::getline(iss, token, ',');
                block.appearance.primaryColor.x = std::stof(token) / 255.0f;
                std::getline(iss, token, ',');
                block.appearance.primaryColor.y = std::stof(token) / 255.0f;
                std::getline(iss, token, ',');
                block.appearance.primaryColor.z = std::stof(token) / 255.0f;
            }
        }

        // Generate procedural content
        block.textureData = GenerateBlockTexture(block);
        block.modelData = GenerateBlockModel(block);
        block.soundData = GenerateBlockSounds(block);
        block.particleData = GenerateBlockParticles(block);

        m_generationStats["custom_blocks"]++;

        return block;
    }

    GeneratedBlock BlockGenerator::GenerateRandomBlock(BlockGenerationType generationType, const std::string& biome) {
        std::uniform_int_distribution<int> typeDist(0, 2);
        int randomType = typeDist(m_randomEngine);

        switch (generationType) {
            case BlockGenerationType::TERRAIN:
                return GenerateTerrainBlock(biome, randomType, 0.5f, 0.5f);
            case BlockGenerationType::ORE:
                {
                    std::vector<std::string> ores = {"coal", "iron", "gold", "diamond"};
                    std::uniform_int_distribution<size_t> oreDist(0, ores.size() - 1);
                    return GenerateOreBlock(ores[oreDist(m_randomEngine)], biome, randomType * 10);
                }
            case BlockGenerationType::VEGETATION:
                {
                    std::vector<std::string> plants = {"oak", "spruce", "rose", "grass"};
                    std::uniform_int_distribution<size_t> plantDist(0, plants.size() - 1);
                    return GenerateVegetationBlock(plants[plantDist(m_randomEngine)], biome, "spring");
                }
            case BlockGenerationType::STRUCTURE:
                {
                    std::vector<std::string> structures = {"wood", "brick", "stone_brick", "glass"};
                    std::uniform_int_distribution<size_t> structDist(0, structures.size() - 1);
                    return GenerateStructureBlock(structures[structDist(m_randomEngine)], "stone");
                }
            case BlockGenerationType::MAGICAL:
                {
                    std::vector<std::string> magical = {"crystal", "runestone", "obsidian"};
                    std::uniform_int_distribution<size_t> magicDist(0, magical.size() - 1);
                    return GenerateMagicalBlock(magical[magicDist(m_randomEngine)], randomType + 1);
                }
            default:
                return GenerateTerrainBlock(biome, 0, 0.5f, 0.5f);
        }
    }

    void* BlockGenerator::GenerateBlockTexture(const GeneratedBlock& block) {
        auto it = m_textureGenerators.find(block.appearance.textureType);
        if (it != m_textureGenerators.end()) {
            return it->second(block);
        }
        return GenerateSolidTexture(block);
    }

    void* BlockGenerator::GenerateBlockModel(const GeneratedBlock& block) {
        std::string modelType = "standard";
        if (block.type == BlockGenerationType::VEGETATION) {
            modelType = "vegetation";
        } else if (block.type == BlockGenerationType::MAGICAL && block.name.find("crystal") != std::string::npos) {
            modelType = "crystal";
        } else if (block.properties.hasAnimation) {
            modelType = "magical";
        }

        auto it = m_modelGenerators.find(modelType);
        if (it != m_modelGenerators.end()) {
            return it->second(block);
        }
        return GenerateStandardBlockModel(block);
    }

    void* BlockGenerator::GenerateBlockSounds(const GeneratedBlock& block) {
        std::string soundType = "stone";
        if (block.materialType.find("wood") != std::string::npos) {
            soundType = "wood";
        } else if (block.materialType.find("metal") != std::string::npos || block.materialType.find("iron") != std::string::npos) {
            soundType = "metal";
        } else if (block.type == BlockGenerationType::VEGETATION) {
            soundType = "organic";
        } else if (block.type == BlockGenerationType::MAGICAL) {
            soundType = "magical";
        }

        auto it = m_soundGenerators.find(soundType);
        if (it != m_soundGenerators.end()) {
            return it->second(block);
        }
        return GenerateStoneBlockSounds(block);
    }

    void* BlockGenerator::GenerateBlockParticles(const GeneratedBlock& block) {
        std::string particleType = "stone";
        if (block.materialType.find("wood") != std::string::npos) {
            particleType = "wood";
        } else if (block.type == BlockGenerationType::VEGETATION && block.name.find("leaf") != std::string::npos) {
            particleType = "leaf";
        } else if (block.name.find("crystal") != std::string::npos) {
            particleType = "crystal";
        } else if (block.type == BlockGenerationType::MAGICAL) {
            particleType = "magical";
        }

        auto it = m_particleGenerators.find(particleType);
        if (it != m_particleGenerators.end()) {
            return it->second(block);
        }
        return GenerateStoneParticles(block);
    }

    std::shared_ptr<Entity> BlockGenerator::CreateBlockEntity(const GeneratedBlock& block, const Vec3& position, EntityManager* entityManager) {
        auto entity = entityManager->CreateEntity(block.name + "_block");

        // Add transform component
        auto transform = entity->AddComponent<TransformComponent>();
        transform->SetPosition(position);
        transform->SetScale(Vec3(1.0f, 1.0f, 1.0f));

        // Add physics component for block physics
        auto physics = entity->AddComponent<PhysicsComponent>();
        physics->SetMass(block.properties.hardness * 10.0f);

        // Add render component with generated data
        if (block.modelData != nullptr && block.textureData != nullptr) {
            auto render = entity->AddComponent<RenderComponent>();
            render->SetRenderType(RenderType::STATIC_MESH);
            // Set custom model and texture data
        }

        return entity;
    }

    std::unordered_map<std::string, int> BlockGenerator::GetGenerationStats() const {
        return m_generationStats;
    }

    void BlockGenerator::ResetGenerationStats() {
        m_generationStats.clear();
    }

    std::string BlockGenerator::GenerateUniqueBlockName(const std::string& baseName, uint64_t seed) {
        std::mt19937_64 gen(seed);
        std::uniform_int_distribution<int> nameDist(1000, 9999);

        std::stringstream ss;
        ss << baseName << "_" << nameDist(gen);
        return ss.str();
    }

    void BlockGenerator::ApplyBiomeModifications(GeneratedBlock& block, const std::string& biome) {
        if (biome == "desert") {
            if (block.name.find("stone") != std::string::npos) {
                block.appearance.primaryColor = Vec3(0.9f, 0.8f, 0.6f); // Sandy stone
            }
        } else if (biome == "snow") {
            block.appearance.primaryColor = Vec3(
                block.appearance.primaryColor.x * 0.9f,
                block.appearance.primaryColor.y * 0.9f,
                block.appearance.primaryColor.z * 1.1f
            ); // Blue-tinted
        } else if (biome == "forest") {
            block.appearance.primaryColor = Vec3(
                block.appearance.primaryColor.x * 0.9f,
                block.appearance.primaryColor.y * 1.1f,
                block.appearance.primaryColor.z * 0.9f
            ); // Green-tinted
        }
    }

    void BlockGenerator::ApplyDepthModifications(GeneratedBlock& block, int depth) {
        if (depth > 50) {
            // Deep underground - darker colors
            float depthFactor = 1.0f - (depth - 50) * 0.002f;
            depthFactor = std::max(0.3f, depthFactor);
            block.appearance.primaryColor = Vec3(
                block.appearance.primaryColor.x * depthFactor,
                block.appearance.primaryColor.y * depthFactor,
                block.appearance.primaryColor.z * depthFactor
            );
        }
    }

    void BlockGenerator::ApplyTemperatureEffects(GeneratedBlock& block, float temperature) {
        if (temperature > 0.8f) {
            // Hot temperatures - reddish tint
            block.appearance.primaryColor.x *= 1.1f;
            block.appearance.primaryColor.y *= 0.95f;
            block.appearance.primaryColor.z *= 0.9f;
        } else if (temperature < 0.2f) {
            // Cold temperatures - bluish tint
            block.appearance.primaryColor.x *= 0.9f;
            block.appearance.primaryColor.y *= 0.95f;
            block.appearance.primaryColor.z *= 1.1f;
        }
    }

    std::vector<std::string> BlockGenerator::GenerateBlockTags(const GeneratedBlock& block) {
        std::vector<std::string> tags;

        if (block.properties.isSolid) tags.push_back("solid");
        if (block.properties.isOpaque) tags.push_back("opaque");
        if (block.properties.isTransparent) tags.push_back("transparent");
        if (block.properties.isFlammable) tags.push_back("flammable");
        if (block.properties.requiresTool) tags.push_back("requires_tool");
        if (block.properties.hasGravity) tags.push_back("gravity");
        if (block.properties.hasGlow) tags.push_back("glowing");
        if (block.isNatural) tags.push_back("natural");
        if (block.isRare) tags.push_back("rare");
        if (block.isMagical) tags.push_back("magical");

        return tags;
    }

    // Terrain block generation implementations
    GeneratedBlock BlockGenerator::GenerateStoneBlock(const std::string& biome, int depth, float temperature, float humidity) {
        GeneratedBlock block;
        block.type = BlockGenerationType::TERRAIN;
        block.name = "stone";
        block.displayName = "Stone";
        block.materialType = "stone";
        block.isNatural = true;

        block.id = GenerateUniqueBlockName("stone", m_seed + depth * 1000);
        block.properties = m_blockPropertiesTemplates["stone"];

        // Appearance
        block.appearance.textureType = BlockTextureType::STONE;
        block.appearance.primaryColor = Vec3(0.5f, 0.5f, 0.5f);
        block.appearance.secondaryColor = Vec3(0.4f, 0.4f, 0.4f);
        block.appearance.accentColor = Vec3(0.3f, 0.3f, 0.3f);
        block.appearance.variant = std::uniform_int_distribution<int>(0, 5)(m_randomEngine);

        // Generate procedural content
        block.textureData = GenerateBlockTexture(block);
        block.modelData = GenerateBlockModel(block);
        block.soundData = GenerateBlockSounds(block);
        block.particleData = GenerateBlockParticles(block);

        block.tags = GenerateBlockTags(block);
        m_generationStats["stone_blocks"]++;

        return block;
    }

    GeneratedBlock BlockGenerator::GenerateDirtBlock(const std::string& biome, int depth, float temperature, float humidity) {
        GeneratedBlock block;
        block.type = BlockGenerationType::TERRAIN;
        block.name = "dirt";
        block.displayName = "Dirt";
        block.materialType = "dirt";
        block.isNatural = true;

        block.id = GenerateUniqueBlockName("dirt", m_seed + depth * 1001);
        block.properties = m_blockPropertiesTemplates["dirt"];

        block.appearance.textureType = BlockTextureType::ORGANIC;
        block.appearance.primaryColor = Vec3(0.6f, 0.4f, 0.2f);
        block.appearance.secondaryColor = Vec3(0.5f, 0.3f, 0.1f);
        block.appearance.accentColor = Vec3(0.4f, 0.2f, 0.0f);
        block.appearance.variant = std::uniform_int_distribution<int>(0, 3)(m_randomEngine);

        block.textureData = GenerateBlockTexture(block);
        block.modelData = GenerateBlockModel(block);
        block.soundData = GenerateBlockSounds(block);
        block.particleData = GenerateBlockParticles(block);

        block.tags = GenerateBlockTags(block);
        m_generationStats["dirt_blocks"]++;

        return block;
    }

    GeneratedBlock BlockGenerator::GenerateGrassBlock(const std::string& biome, int depth, float temperature, float humidity) {
        GeneratedBlock block;
        block.type = BlockGenerationType::TERRAIN;
        block.name = "grass_block";
        block.displayName = "Grass Block";
        block.materialType = "grass";
        block.isNatural = true;

        block.id = GenerateUniqueBlockName("grass", m_seed + depth * 1002);
        block.properties = m_blockPropertiesTemplates["grass"];

        block.appearance.textureType = BlockTextureType::ORGANIC;
        block.appearance.primaryColor = Vec3(0.2f, 0.8f, 0.1f); // Top
        block.appearance.secondaryColor = Vec3(0.6f, 0.4f, 0.2f); // Sides
        block.appearance.accentColor = Vec3(0.5f, 0.3f, 0.1f); // Bottom
        block.appearance.variant = std::uniform_int_distribution<int>(0, 4)(m_randomEngine);

        block.textureData = GenerateBlockTexture(block);
        block.modelData = GenerateBlockModel(block);
        block.soundData = GenerateBlockSounds(block);
        block.particleData = GenerateBlockParticles(block);

        block.tags = GenerateBlockTags(block);
        m_generationStats["grass_blocks"]++;

        return block;
    }

    GeneratedBlock BlockGenerator::GenerateSandBlock(const std::string& biome, int depth, float temperature, float humidity) {
        GeneratedBlock block;
        block.type = BlockGenerationType::TERRAIN;
        block.name = "sand";
        block.displayName = "Sand";
        block.materialType = "sand";
        block.isNatural = true;

        block.id = GenerateUniqueBlockName("sand", m_seed + depth * 1003);
        block.properties = m_blockPropertiesTemplates["dirt"]; // Similar to dirt
        block.properties.hardness = 0.5f;
        block.properties.hasGravity = true;

        block.appearance.textureType = BlockTextureType::ORGANIC;
        block.appearance.primaryColor = Vec3(0.9f, 0.8f, 0.6f);
        block.appearance.secondaryColor = Vec3(0.8f, 0.7f, 0.5f);
        block.appearance.accentColor = Vec3(0.7f, 0.6f, 0.4f);
        block.appearance.variant = std::uniform_int_distribution<int>(0, 2)(m_randomEngine);

        block.textureData = GenerateBlockTexture(block);
        block.modelData = GenerateBlockModel(block);
        block.soundData = GenerateBlockSounds(block);
        block.particleData = GenerateBlockParticles(block);

        block.tags = GenerateBlockTags(block);
        m_generationStats["sand_blocks"]++;

        return block;
    }

    GeneratedBlock BlockGenerator::GenerateGravelBlock(const std::string& biome, int depth, float temperature, float humidity) {
        GeneratedBlock block;
        block.type = BlockGenerationType::TERRAIN;
        block.name = "gravel";
        block.displayName = "Gravel";
        block.materialType = "gravel";
        block.isNatural = true;

        block.id = GenerateUniqueBlockName("gravel", m_seed + depth * 1004);
        block.properties = m_blockPropertiesTemplates["dirt"];
        block.properties.hardness = 0.6f;
        block.properties.hasGravity = true;

        block.appearance.textureType = BlockTextureType::STONE;
        block.appearance.primaryColor = Vec3(0.4f, 0.4f, 0.4f);
        block.appearance.secondaryColor = Vec3(0.5f, 0.5f, 0.5f);
        block.appearance.accentColor = Vec3(0.3f, 0.3f, 0.3f);
        block.appearance.variant = std::uniform_int_distribution<int>(0, 3)(m_randomEngine);

        block.textureData = GenerateBlockTexture(block);
        block.modelData = GenerateBlockModel(block);
        block.soundData = GenerateBlockSounds(block);
        block.particleData = GenerateBlockParticles(block);

        block.tags = GenerateBlockTags(block);
        m_generationStats["gravel_blocks"]++;

        return block;
    }

    GeneratedBlock BlockGenerator::GenerateClayBlock(const std::string& biome, int depth, float temperature, float humidity) {
        GeneratedBlock block;
        block.type = BlockGenerationType::TERRAIN;
        block.name = "clay";
        block.displayName = "Clay";
        block.materialType = "clay";
        block.isNatural = true;

        block.id = GenerateUniqueBlockName("clay", m_seed + depth * 1005);
        block.properties = m_blockPropertiesTemplates["dirt"];
        block.properties.hardness = 0.6f;

        block.appearance.textureType = BlockTextureType::ORGANIC;
        block.appearance.primaryColor = Vec3(0.6f, 0.7f, 0.8f);
        block.appearance.secondaryColor = Vec3(0.5f, 0.6f, 0.7f);
        block.appearance.accentColor = Vec3(0.4f, 0.5f, 0.6f);
        block.appearance.variant = std::uniform_int_distribution<int>(0, 2)(m_randomEngine);

        block.textureData = GenerateBlockTexture(block);
        block.modelData = GenerateBlockModel(block);
        block.soundData = GenerateBlockSounds(block);
        block.particleData = GenerateBlockParticles(block);

        block.tags = GenerateBlockTags(block);
        m_generationStats["clay_blocks"]++;

        return block;
    }

    GeneratedBlock BlockGenerator::GenerateSnowBlock(const std::string& biome, int depth, float temperature, float humidity) {
        GeneratedBlock block;
        block.type = BlockGenerationType::TERRAIN;
        block.name = "snow";
        block.displayName = "Snow";
        block.materialType = "snow";
        block.isNatural = true;

        block.id = GenerateUniqueBlockName("snow", m_seed + depth * 1006);
        block.properties = m_blockPropertiesTemplates["dirt"];
        block.properties.hardness = 0.1f;
        block.properties.isFlammable = false;

        block.appearance.textureType = BlockTextureType::ORGANIC;
        block.appearance.primaryColor = Vec3(0.95f, 0.95f, 1.0f);
        block.appearance.secondaryColor = Vec3(0.9f, 0.9f, 0.95f);
        block.appearance.accentColor = Vec3(0.85f, 0.85f, 0.9f);
        block.appearance.variant = std::uniform_int_distribution<int>(0, 2)(m_randomEngine);

        block.textureData = GenerateBlockTexture(block);
        block.modelData = GenerateBlockModel(block);
        block.soundData = GenerateBlockSounds(block);
        block.particleData = GenerateBlockParticles(block);

        block.tags = GenerateBlockTags(block);
        m_generationStats["snow_blocks"]++;

        return block;
    }

    // Ore generation implementations
    GeneratedBlock BlockGenerator::GenerateCoalOreBlock(const std::string& biome, int depth) {
        GeneratedBlock block;
        block.type = BlockGenerationType::ORE;
        block.name = "coal_ore";
        block.displayName = "Coal Ore";
        block.materialType = "ore";
        block.isNatural = true;

        block.id = GenerateUniqueBlockName("coal_ore", m_seed + depth * 2000);
        block.properties = m_blockPropertiesTemplates["ore"];
        block.properties.lightLevel = 0.0f;
        block.properties.drops = {"coal"};

        block.appearance.textureType = BlockTextureType::STONE;
        block.appearance.primaryColor = Vec3(0.5f, 0.5f, 0.5f);
        block.appearance.secondaryColor = Vec3(0.2f, 0.2f, 0.2f);
        block.appearance.accentColor = Vec3(0.1f, 0.1f, 0.1f);
        block.appearance.variant = std::uniform_int_distribution<int>(0, 3)(m_randomEngine);

        block.textureData = GenerateBlockTexture(block);
        block.modelData = GenerateBlockModel(block);
        block.soundData = GenerateBlockSounds(block);
        block.particleData = GenerateBlockParticles(block);

        block.tags = GenerateBlockTags(block);
        m_generationStats["coal_ore_blocks"]++;

        return block;
    }

    GeneratedBlock BlockGenerator::GenerateIronOreBlock(const std::string& biome, int depth) {
        GeneratedBlock block;
        block.type = BlockGenerationType::ORE;
        block.name = "iron_ore";
        block.displayName = "Iron Ore";
        block.materialType = "ore";
        block.isNatural = true;

        block.id = GenerateUniqueBlockName("iron_ore", m_seed + depth * 2001);
        block.properties = m_blockPropertiesTemplates["ore"];
        block.properties.miningLevel = 1;
        block.properties.drops = {"iron_ore"};

        block.appearance.textureType = BlockTextureType::METAL;
        block.appearance.primaryColor = Vec3(0.5f, 0.5f, 0.5f);
        block.appearance.secondaryColor = Vec3(0.7f, 0.6f, 0.5f);
        block.appearance.accentColor = Vec3(0.6f, 0.5f, 0.4f);
        block.appearance.variant = std::uniform_int_distribution<int>(0, 4)(m_randomEngine);

        block.textureData = GenerateBlockTexture(block);
        block.modelData = GenerateBlockModel(block);
        block.soundData = GenerateBlockSounds(block);
        block.particleData = GenerateBlockParticles(block);

        block.tags = GenerateBlockTags(block);
        m_generationStats["iron_ore_blocks"]++;

        return block;
    }

    GeneratedBlock BlockGenerator::GenerateGoldOreBlock(const std::string& biome, int depth) {
        GeneratedBlock block;
        block.type = BlockGenerationType::ORE;
        block.name = "gold_ore";
        block.displayName = "Gold Ore";
        block.materialType = "ore";
        block.isNatural = true;
        block.isRare = true;

        block.id = GenerateUniqueBlockName("gold_ore", m_seed + depth * 2002);
        block.properties = m_blockPropertiesTemplates["ore"];
        block.properties.miningLevel = 2;
        block.properties.drops = {"gold_ore"};

        block.appearance.textureType = BlockTextureType::METAL;
        block.appearance.primaryColor = Vec3(0.5f, 0.5f, 0.5f);
        block.appearance.secondaryColor = Vec3(0.9f, 0.8f, 0.2f);
        block.appearance.accentColor = Vec3(0.8f, 0.7f, 0.1f);
        block.appearance.variant = std::uniform_int_distribution<int>(0, 3)(m_randomEngine);

        block.textureData = GenerateBlockTexture(block);
        block.modelData = GenerateBlockModel(block);
        block.soundData = GenerateBlockSounds(block);
        block.particleData = GenerateBlockParticles(block);

        block.tags = GenerateBlockTags(block);
        m_generationStats["gold_ore_blocks"]++;

        return block;
    }

    GeneratedBlock BlockGenerator::GenerateDiamondOreBlock(const std::string& biome, int depth) {
        GeneratedBlock block;
        block.type = BlockGenerationType::ORE;
        block.name = "diamond_ore";
        block.displayName = "Diamond Ore";
        block.materialType = "ore";
        block.isNatural = true;
        block.isRare = true;

        block.id = GenerateUniqueBlockName("diamond_ore", m_seed + depth * 2003);
        block.properties = m_blockPropertiesTemplates["ore"];
        block.properties.miningLevel = 2;
        block.properties.drops = {"diamond"};

        block.appearance.textureType = BlockTextureType::CRYSTAL;
        block.appearance.primaryColor = Vec3(0.5f, 0.5f, 0.5f);
        block.appearance.secondaryColor = Vec3(0.3f, 0.8f, 0.9f);
        block.appearance.accentColor = Vec3(0.2f, 0.7f, 0.8f);
        block.appearance.variant = std::uniform_int_distribution<int>(0, 2)(m_randomEngine);

        block.textureData = GenerateBlockTexture(block);
        block.modelData = GenerateBlockModel(block);
        block.soundData = GenerateBlockSounds(block);
        block.particleData = GenerateBlockParticles(block);

        block.tags = GenerateBlockTags(block);
        m_generationStats["diamond_ore_blocks"]++;

        return block;
    }

    GeneratedBlock BlockGenerator::GenerateRedstoneOreBlock(const std::string& biome, int depth) {
        GeneratedBlock block;
        block.type = BlockGenerationType::ORE;
        block.name = "redstone_ore";
        block.displayName = "Redstone Ore";
        block.materialType = "ore";
        block.isNatural = true;

        block.id = GenerateUniqueBlockName("redstone_ore", m_seed + depth * 2004);
        block.properties = m_blockPropertiesTemplates["ore"];
        block.properties.miningLevel = 2;
        block.properties.lightLevel = 0.3f;
        block.properties.drops = {"redstone"};

        block.appearance.textureType = BlockTextureType::STONE;
        block.appearance.primaryColor = Vec3(0.5f, 0.5f, 0.5f);
        block.appearance.secondaryColor = Vec3(0.8f, 0.2f, 0.1f);
        block.appearance.accentColor = Vec3(0.7f, 0.1f, 0.0f);
        block.appearance.variant = std::uniform_int_distribution<int>(0, 3)(m_randomEngine);

        block.textureData = GenerateBlockTexture(block);
        block.modelData = GenerateBlockModel(block);
        block.soundData = GenerateBlockSounds(block);
        block.particleData = GenerateBlockParticles(block);

        block.tags = GenerateBlockTags(block);
        m_generationStats["redstone_ore_blocks"]++;

        return block;
    }

    GeneratedBlock BlockGenerator::GenerateLapisOreBlock(const std::string& biome, int depth) {
        GeneratedBlock block;
        block.type = BlockGenerationType::ORE;
        block.name = "lapis_ore";
        block.displayName = "Lapis Lazuli Ore";
        block.materialType = "ore";
        block.isNatural = true;
        block.isRare = true;

        block.id = GenerateUniqueBlockName("lapis_ore", m_seed + depth * 2005);
        block.properties = m_blockPropertiesTemplates["ore"];
        block.properties.miningLevel = 1;
        block.properties.drops = {"lapis_lazuli"};

        block.appearance.textureType = BlockTextureType::STONE;
        block.appearance.primaryColor = Vec3(0.5f, 0.5f, 0.5f);
        block.appearance.secondaryColor = Vec3(0.2f, 0.3f, 0.8f);
        block.appearance.accentColor = Vec3(0.1f, 0.2f, 0.7f);
        block.appearance.variant = std::uniform_int_distribution<int>(0, 2)(m_randomEngine);

        block.textureData = GenerateBlockTexture(block);
        block.modelData = GenerateBlockModel(block);
        block.soundData = GenerateBlockSounds(block);
        block.particleData = GenerateBlockParticles(block);

        block.tags = GenerateBlockTags(block);
        m_generationStats["lapis_ore_blocks"]++;

        return block;
    }

    GeneratedBlock BlockGenerator::GenerateEmeraldOreBlock(const std::string& biome, int depth) {
        GeneratedBlock block;
        block.type = BlockGenerationType::ORE;
        block.name = "emerald_ore";
        block.displayName = "Emerald Ore";
        block.materialType = "ore";
        block.isNatural = true;
        block.isRare = true;

        block.id = GenerateUniqueBlockName("emerald_ore", m_seed + depth * 2006);
        block.properties = m_blockPropertiesTemplates["ore"];
        block.properties.miningLevel = 2;
        block.properties.drops = {"emerald"};

        block.appearance.textureType = BlockTextureType::STONE;
        block.appearance.primaryColor = Vec3(0.5f, 0.5f, 0.5f);
        block.appearance.secondaryColor = Vec3(0.1f, 0.8f, 0.3f);
        block.appearance.accentColor = Vec3(0.0f, 0.7f, 0.2f);
        block.appearance.variant = std::uniform_int_distribution<int>(0, 2)(m_randomEngine);

        block.textureData = GenerateBlockTexture(block);
        block.modelData = GenerateBlockModel(block);
        block.soundData = GenerateBlockSounds(block);
        block.particleData = GenerateBlockParticles(block);

        block.tags = GenerateBlockTags(block);
        m_generationStats["emerald_ore_blocks"]++;

        return block;
    }

    GeneratedBlock BlockGenerator::GenerateCustomOreBlock(const std::string& oreType, const std::string& biome, int depth) {
        GeneratedBlock block;
        block.type = BlockGenerationType::ORE;
        block.name = oreType + "_ore";
        block.displayName = std::string(1, std::toupper(oreType[0])) + oreType.substr(1) + " Ore";
        block.materialType = "ore";
        block.isNatural = true;

        block.id = GenerateUniqueBlockName(oreType + "_ore", m_seed + depth * 2007);
        block.properties = m_blockPropertiesTemplates["ore"];
        block.properties.drops = {oreType};

        // Random colors for custom ore
        std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);
        block.appearance.textureType = BlockTextureType::METAL;
        block.appearance.primaryColor = Vec3(colorDist(m_randomEngine), colorDist(m_randomEngine), colorDist(m_randomEngine));
        block.appearance.secondaryColor = Vec3(colorDist(m_randomEngine), colorDist(m_randomEngine), colorDist(m_randomEngine));
        block.appearance.accentColor = Vec3(colorDist(m_randomEngine), colorDist(m_randomEngine), colorDist(m_randomEngine));
        block.appearance.variant = std::uniform_int_distribution<int>(0, 5)(m_randomEngine);

        block.textureData = GenerateBlockTexture(block);
        block.modelData = GenerateBlockModel(block);
        block.soundData = GenerateBlockSounds(block);
        block.particleData = GenerateBlockParticles(block);

        block.tags = GenerateBlockTags(block);
        m_generationStats["custom_ore_blocks"]++;

        return block;
    }

    // Vegetation generation implementations
    GeneratedBlock BlockGenerator::GenerateTreeBlock(const std::string& treeType, const std::string& biome, const std::string& season) {
        GeneratedBlock block;
        block.type = BlockGenerationType::VEGETATION;
        block.name = treeType + "_log";
        block.displayName = std::string(1, std::toupper(treeType[0])) + treeType.substr(1) + " Log";
        block.materialType = "wood";
        block.isNatural = true;

        block.id = GenerateUniqueBlockName(treeType + "_log", m_seed + std::hash<std::string>()(biome));
        block.properties = m_blockPropertiesTemplates["wood"];
        block.properties.drops = {treeType + "_log"};

        block.appearance.textureType = BlockTextureType::WOOD;

        // Tree-specific colors
        if (treeType == "oak") {
            block.appearance.primaryColor = Vec3(0.6f, 0.4f, 0.2f);
            block.appearance.secondaryColor = Vec3(0.5f, 0.3f, 0.1f);
        } else if (treeType == "spruce") {
            block.appearance.primaryColor = Vec3(0.4f, 0.3f, 0.2f);
            block.appearance.secondaryColor = Vec3(0.3f, 0.2f, 0.1f);
        } else if (treeType == "birch") {
            block.appearance.primaryColor = Vec3(0.8f, 0.8f, 0.7f);
            block.appearance.secondaryColor = Vec3(0.7f, 0.7f, 0.6f);
        }

        block.appearance.accentColor = Vec3(0.4f, 0.2f, 0.0f);
        block.appearance.variant = std::uniform_int_distribution<int>(0, 3)(m_randomEngine);

        block.textureData = GenerateBlockTexture(block);
        block.modelData = GenerateBlockModel(block);
        block.soundData = GenerateBlockSounds(block);
        block.particleData = GenerateBlockParticles(block);

        block.tags = GenerateBlockTags(block);
        m_generationStats["tree_blocks"]++;

        return block;
    }

    GeneratedBlock BlockGenerator::GenerateFlowerBlock(const std::string& flowerType, const std::string& biome, const std::string& season) {
        GeneratedBlock block;
        block.type = BlockGenerationType::VEGETATION;
        block.name = flowerType;
        block.displayName = std::string(1, std::toupper(flowerType[0])) + flowerType.substr(1);
        block.materialType = "plant";
        block.isNatural = true;

        block.id = GenerateUniqueBlockName(flowerType, m_seed + std::hash<std::string>()(biome + season));
        block.properties = m_blockPropertiesTemplates["dirt"];
        block.properties.hardness = 0.0f;
        block.properties.isSolid = false;
        block.properties.isReplaceable = true;
        block.properties.drops = {flowerType};

        block.appearance.textureType = BlockTextureType::ORGANIC;

        // Flower-specific colors
        if (flowerType == "rose") {
            block.appearance.primaryColor = Vec3(0.8f, 0.2f, 0.3f);
            block.appearance.secondaryColor = Vec3(0.6f, 0.1f, 0.2f);
        } else if (flowerType == "tulip") {
            block.appearance.primaryColor = Vec3(0.9f, 0.7f, 0.2f);
            block.appearance.secondaryColor = Vec3(0.8f, 0.6f, 0.1f);
        }

        block.appearance.accentColor = Vec3(0.2f, 0.8f, 0.2f);
        block.appearance.variant = std::uniform_int_distribution<int>(0, 4)(m_randomEngine);

        block.textureData = GenerateBlockTexture(block);
        block.modelData = GenerateBlockModel(block);
        block.soundData = GenerateBlockSounds(block);
        block.particleData = GenerateBlockParticles(block);

        block.tags = GenerateBlockTags(block);
        m_generationStats["flower_blocks"]++;

        return block;
    }

    GeneratedBlock BlockGenerator::GenerateGrassPlantBlock(const std::string& biome, const std::string& season) {
        GeneratedBlock block;
        block.type = BlockGenerationType::VEGETATION;
        block.name = "grass";
        block.displayName = "Grass";
        block.materialType = "plant";
        block.isNatural = true;

        block.id = GenerateUniqueBlockName("grass", m_seed + std::hash<std::string>()(biome + season));
        block.properties = m_blockPropertiesTemplates["dirt"];
        block.properties.hardness = 0.0f;
        block.properties.isSolid = false;
        block.properties.isReplaceable = true;
        block.properties.drops = {"wheat_seeds"};

        block.appearance.textureType = BlockTextureType::LEAF;
        block.appearance.primaryColor = Vec3(0.2f, 0.8f, 0.1f);
        block.appearance.secondaryColor = Vec3(0.1f, 0.6f, 0.0f);
        block.appearance.accentColor = Vec3(0.3f, 0.9f, 0.2f);
        block.appearance.variant = std::uniform_int_distribution<int>(0, 6)(m_randomEngine);

        block.textureData = GenerateBlockTexture(block);
        block.modelData = GenerateBlockModel(block);
        block.soundData = GenerateBlockSounds(block);
        block.particleData = GenerateBlockParticles(block);

        block.tags = GenerateBlockTags(block);
        m_generationStats["grass_blocks"]++;

        return block;
    }

    GeneratedBlock BlockGenerator::GenerateCropBlock(const std::string& cropType, const std::string& biome, const std::string& season) {
        GeneratedBlock block;
        block.type = BlockGenerationType::VEGETATION;
        block.name = cropType + "_crop";
        block.displayName = std::string(1, std::toupper(cropType[0])) + cropType.substr(1) + " Crop";
        block.materialType = "plant";
        block.isNatural = true;

        block.id = GenerateUniqueBlockName(cropType + "_crop", m_seed + std::hash<std::string>()(biome + season));
        block.properties = m_blockPropertiesTemplates["dirt"];
        block.properties.hardness = 0.0f;
        block.properties.isSolid = false;
        block.properties.isReplaceable = true;
        block.properties.drops = {cropType};

        block.appearance.textureType = BlockTextureType::ORGANIC;

        // Crop-specific colors
        if (cropType == "wheat") {
            block.appearance.primaryColor = Vec3(0.8f, 0.7f, 0.3f);
            block.appearance.secondaryColor = Vec3(0.6f, 0.5f, 0.2f);
        } else if (cropType == "carrot") {
            block.appearance.primaryColor = Vec3(0.9f, 0.5f, 0.1f);
            block.appearance.secondaryColor = Vec3(0.8f, 0.4f, 0.0f);
        }

        block.appearance.accentColor = Vec3(0.2f, 0.8f, 0.2f);
        block.appearance.variant = std::uniform_int_distribution<int>(0, 3)(m_randomEngine);

        block.textureData = GenerateBlockTexture(block);
        block.modelData = GenerateBlockModel(block);
        block.soundData = GenerateBlockSounds(block);
        block.particleData = GenerateBlockParticles(block);

        block.tags = GenerateBlockTags(block);
        m_generationStats["crop_blocks"]++;

        return block;
    }

    GeneratedBlock BlockGenerator::GenerateMushroomBlock(const std::string& mushroomType, const std::string& biome, const std::string& season) {
        GeneratedBlock block;
        block.type = BlockGenerationType::VEGETATION;
        block.name = mushroomType + "_mushroom";
        block.displayName = std::string(1, std::toupper(mushroomType[0])) + mushroomType.substr(1) + " Mushroom";
        block.materialType = "fungus";
        block.isNatural = true;

        block.id = GenerateUniqueBlockName(mushroomType + "_mushroom", m_seed + std::hash<std::string>()(biome + season));
        block.properties = m_blockPropertiesTemplates["dirt"];
        block.properties.hardness = 0.0f;
        block.properties.isSolid = false;
        block.properties.isReplaceable = true;
        block.properties.drops = {mushroomType + "_mushroom"};

        block.appearance.textureType = BlockTextureType::ORGANIC;

        // Mushroom-specific colors
        if (mushroomType == "red") {
            block.appearance.primaryColor = Vec3(0.8f, 0.1f, 0.1f);
            block.appearance.secondaryColor = Vec3(0.9f, 0.8f, 0.8f);
        } else if (mushroomType == "brown") {
            block.appearance.primaryColor = Vec3(0.6f, 0.4f, 0.2f);
            block.appearance.secondaryColor = Vec3(0.7f, 0.5f, 0.3f);
        }

        block.appearance.accentColor = Vec3(0.5f, 0.5f, 0.5f);
        block.appearance.variant = std::uniform_int_distribution<int>(0, 2)(m_randomEngine);

        block.textureData = GenerateBlockTexture(block);
        block.modelData = GenerateBlockModel(block);
        block.soundData = GenerateBlockSounds(block);
        block.particleData = GenerateBlockParticles(block);

        block.tags = GenerateBlockTags(block);
        m_generationStats["mushroom_blocks"]++;

        return block;
    }

    // Structure generation implementations
    GeneratedBlock BlockGenerator::GenerateWoodBlock(const std::string& woodType, const std::string& material) {
        GeneratedBlock block;
        block.type = BlockGenerationType::STRUCTURE;
        block.name = woodType + "_planks";
        block.displayName = std::string(1, std::toupper(woodType[0])) + woodType.substr(1) + " Planks";
        block.materialType = "wood";

        block.id = GenerateUniqueBlockName(woodType + "_planks", m_seed + std::hash<std::string>()(material));
        block.properties = m_blockPropertiesTemplates["wood"];
        block.properties.drops = {woodType + "_planks"};

        block.appearance.textureType = BlockTextureType::WOOD;

        // Wood-specific colors
        if (woodType == "oak") {
            block.appearance.primaryColor = Vec3(0.6f, 0.4f, 0.2f);
            block.appearance.secondaryColor = Vec3(0.5f, 0.3f, 0.1f);
        } else if (woodType == "spruce") {
            block.appearance.primaryColor = Vec3(0.4f, 0.3f, 0.2f);
            block.appearance.secondaryColor = Vec3(0.3f, 0.2f, 0.1f);
        } else if (woodType == "birch") {
            block.appearance.primaryColor = Vec3(0.8f, 0.8f, 0.7f);
            block.appearance.secondaryColor = Vec3(0.7f, 0.7f, 0.6f);
        }

        block.appearance.accentColor = Vec3(0.4f, 0.2f, 0.0f);
        block.appearance.variant = std::uniform_int_distribution<int>(0, 3)(m_randomEngine);

        block.textureData = GenerateBlockTexture(block);
        block.modelData = GenerateBlockModel(block);
        block.soundData = GenerateBlockSounds(block);
        block.particleData = GenerateBlockParticles(block);

        block.tags = GenerateBlockTags(block);
        m_generationStats["wood_structure_blocks"]++;

        return block;
    }

    GeneratedBlock BlockGenerator::GenerateBrickBlock(const std::string& brickType, const std::string& material) {
        GeneratedBlock block;
        block.type = BlockGenerationType::STRUCTURE;
        block.name = brickType + "_bricks";
        block.displayName = std::string(1, std::toupper(brickType[0])) + brickType.substr(1) + " Bricks";
        block.materialType = "stone";

        block.id = GenerateUniqueBlockName(brickType + "_bricks", m_seed + std::hash<std::string>()(material));
        block.properties = m_blockPropertiesTemplates["stone"];
        block.properties.hardness = 2.0f;
        block.properties.drops = {brickType + "_bricks"};

        block.appearance.textureType = BlockTextureType::STONE;

        // Brick-specific colors
        if (brickType == "red") {
            block.appearance.primaryColor = Vec3(0.7f, 0.3f, 0.2f);
            block.appearance.secondaryColor = Vec3(0.6f, 0.2f, 0.1f);
        } else if (brickType == "stone") {
            block.appearance.primaryColor = Vec3(0.5f, 0.5f, 0.5f);
            block.appearance.secondaryColor = Vec3(0.4f, 0.4f, 0.4f);
        }

        block.appearance.accentColor = Vec3(0.3f, 0.3f, 0.3f);
        block.appearance.variant = std::uniform_int_distribution<int>(0, 4)(m_randomEngine);

        block.textureData = GenerateBlockTexture(block);
        block.modelData = GenerateBlockModel(block);
        block.soundData = GenerateBlockSounds(block);
        block.particleData = GenerateBlockParticles(block);

        block.tags = GenerateBlockTags(block);
        m_generationStats["brick_structure_blocks"]++;

        return block;
    }

    GeneratedBlock BlockGenerator::GenerateStoneBrickBlock(const std::string& stoneType, const std::string& material) {
        GeneratedBlock block;
        block.type = BlockGenerationType::STRUCTURE;
        block.name = stoneType + "_stone_bricks";
        block.displayName = std::string(1, std::toupper(stoneType[0])) + stoneType.substr(1) + " Stone Bricks";
        block.materialType = "stone";

        block.id = GenerateUniqueBlockName(stoneType + "_stone_bricks", m_seed + std::hash<std::string>()(material));
        block.properties = m_blockPropertiesTemplates["stone"];
        block.properties.hardness = 1.5f;
        block.properties.drops = {stoneType + "_stone_bricks"};

        block.appearance.textureType = BlockTextureType::STONE;
        block.appearance.primaryColor = Vec3(0.5f, 0.5f, 0.5f);
        block.appearance.secondaryColor = Vec3(0.4f, 0.4f, 0.4f);
        block.appearance.accentColor = Vec3(0.3f, 0.3f, 0.3f);
        block.appearance.variant = std::uniform_int_distribution<int>(0, 3)(m_randomEngine);

        block.textureData = GenerateBlockTexture(block);
        block.modelData = GenerateBlockModel(block);
        block.soundData = GenerateBlockSounds(block);
        block.particleData = GenerateBlockParticles(block);

        block.tags = GenerateBlockTags(block);
        m_generationStats["stone_brick_structure_blocks"]++;

        return block;
    }

    GeneratedBlock BlockGenerator::GenerateGlassBlock(const std::string& glassType, const std::string& material) {
        GeneratedBlock block;
        block.type = BlockGenerationType::STRUCTURE;
        block.name = glassType + "_glass";
        block.displayName = std::string(1, std::toupper(glassType[0])) + glassType.substr(1) + " Glass";
        block.materialType = "glass";

        block.id = GenerateUniqueBlockName(glassType + "_glass", m_seed + std::hash<std::string>()(material));
        block.properties = m_blockPropertiesTemplates["stone"];
        block.properties.hardness = 0.3f;
        block.properties.isTransparent = true;
        block.properties.isOpaque = false;
        block.properties.drops = {glassType + "_glass"};

        block.appearance.textureType = BlockTextureType::FLUID;

        // Glass-specific colors
        if (glassType == "clear") {
            block.appearance.primaryColor = Vec3(0.9f, 0.9f, 1.0f);
            block.appearance.secondaryColor = Vec3(0.8f, 0.8f, 0.9f);
        } else if (glassType == "tinted") {
            block.appearance.primaryColor = Vec3(0.3f, 0.3f, 0.4f);
            block.appearance.secondaryColor = Vec3(0.2f, 0.2f, 0.3f);
        }

        block.appearance.accentColor = Vec3(0.7f, 0.7f, 0.8f);
        block.appearance.variant = std::uniform_int_distribution<int>(0, 2)(m_randomEngine);

        block.textureData = GenerateBlockTexture(block);
        block.modelData = GenerateBlockModel(block);
        block.soundData = GenerateBlockSounds(block);
        block.particleData = GenerateBlockParticles(block);

        block.tags = GenerateBlockTags(block);
        m_generationStats["glass_structure_blocks"]++;

        return block;
    }

    GeneratedBlock BlockGenerator::GenerateConcreteBlock(const std::string& concreteType, const std::string& material) {
        GeneratedBlock block;
        block.type = BlockGenerationType::STRUCTURE;
        block.name = concreteType + "_concrete";
        block.displayName = std::string(1, std::toupper(concreteType[0])) + concreteType.substr(1) + " Concrete";
        block.materialType = "stone";

        block.id = GenerateUniqueBlockName(concreteType + "_concrete", m_seed + std::hash<std::string>()(material));
        block.properties = m_blockPropertiesTemplates["stone"];
        block.properties.hardness = 1.8f;
        block.properties.drops = {concreteType + "_concrete"};

        block.appearance.textureType = BlockTextureType::SOLID;

        // Concrete-specific colors
        if (concreteType == "white") {
            block.appearance.primaryColor = Vec3(0.9f, 0.9f, 0.9f);
            block.appearance.secondaryColor = Vec3(0.8f, 0.8f, 0.8f);
        } else if (concreteType == "gray") {
            block.appearance.primaryColor = Vec3(0.5f, 0.5f, 0.5f);
            block.appearance.secondaryColor = Vec3(0.4f, 0.4f, 0.4f);
        } else if (concreteType == "black") {
            block.appearance.primaryColor = Vec3(0.1f, 0.1f, 0.1f);
            block.appearance.secondaryColor = Vec3(0.2f, 0.2f, 0.2f);
        }

        block.appearance.accentColor = Vec3(0.6f, 0.6f, 0.6f);
        block.appearance.variant = std::uniform_int_distribution<int>(0, 3)(m_randomEngine);

        block.textureData = GenerateBlockTexture(block);
        block.modelData = GenerateBlockModel(block);
        block.soundData = GenerateBlockSounds(block);
        block.particleData = GenerateBlockParticles(block);

        block.tags = GenerateBlockTags(block);
        m_generationStats["concrete_structure_blocks"]++;

        return block;
    }

    // Magical generation implementations
    GeneratedBlock BlockGenerator::GenerateCrystalBlock(const std::string& crystalType, int powerLevel) {
        GeneratedBlock block;
        block.type = BlockGenerationType::MAGICAL;
        block.name = crystalType + "_crystal_block";
        block.displayName = std::string(1, std::toupper(crystalType[0])) + crystalType.substr(1) + " Crystal Block";
        block.materialType = "crystal";
        block.isMagical = true;

        block.id = GenerateUniqueBlockName(crystalType + "_crystal_block", m_seed + powerLevel * 1000);
        block.properties = m_blockPropertiesTemplates["crystal"];
        block.properties.hardness = 1.0f + powerLevel * 0.5f;
        block.properties.lightLevel = 0.5f + powerLevel * 0.1f;
        block.properties.drops = {crystalType + "_crystal"};

        block.appearance.textureType = BlockTextureType::CRYSTAL;
        block.appearance.primaryColor = Vec3(0.8f, 0.8f, 0.9f);
        block.appearance.secondaryColor = Vec3(0.6f, 0.6f, 0.8f);
        block.appearance.accentColor = Vec3(1.0f, 1.0f, 1.0f);
        block.appearance.hasGlow = true;
        block.appearance.variant = std::uniform_int_distribution<int>(0, 5)(m_randomEngine);

        block.textureData = GenerateBlockTexture(block);
        block.modelData = GenerateBlockModel(block);
        block.soundData = GenerateBlockSounds(block);
        block.particleData = GenerateBlockParticles(block);

        block.tags = GenerateBlockTags(block);
        m_generationStats["crystal_magical_blocks"]++;

        return block;
    }

    GeneratedBlock BlockGenerator::GenerateMagicalStoneBlock(const std::string& magicType, int powerLevel) {
        GeneratedBlock block;
        block.type = BlockGenerationType::MAGICAL;
        block.name = magicType + "_magical_stone";
        block.displayName = std::string(1, std::toupper(magicType[0])) + magicType.substr(1) + " Magical Stone";
        block.materialType = "magical_stone";
        block.isMagical = true;

        block.id = GenerateUniqueBlockName(magicType + "_magical_stone", m_seed + powerLevel * 1001);
        block.properties = m_blockPropertiesTemplates["stone"];
        block.properties.hardness = 2.0f + powerLevel * 0.3f;
        block.properties.lightLevel = 0.3f + powerLevel * 0.1f;
        block.properties.drops = {magicType + "_magical_stone"};

        block.appearance.textureType = BlockTextureType::STONE;
        block.appearance.primaryColor = Vec3(0.4f, 0.4f, 0.5f);
        block.appearance.secondaryColor = Vec3(0.5f, 0.5f, 0.6f);
        block.appearance.accentColor = Vec3(0.6f, 0.6f, 0.7f);
        block.appearance.hasGlow = (powerLevel > 3);
        block.appearance.variant = std::uniform_int_distribution<int>(0, 4)(m_randomEngine);

        block.textureData = GenerateBlockTexture(block);
        block.modelData = GenerateBlockModel(block);
        block.soundData = GenerateBlockSounds(block);
        block.particleData = GenerateBlockParticles(block);

        block.tags = GenerateBlockTags(block);
        m_generationStats["magical_stone_blocks"]++;

        return block;
    }

    GeneratedBlock BlockGenerator::GenerateRunestoneBlock(const std::string& runeType, int powerLevel) {
        GeneratedBlock block;
        block.type = BlockGenerationType::MAGICAL;
        block.name = runeType + "_runestone";
        block.displayName = std::string(1, std::toupper(runeType[0])) + runeType.substr(1) + " Runestone";
        block.materialType = "runestone";
        block.isMagical = true;

        block.id = GenerateUniqueBlockName(runeType + "_runestone", m_seed + powerLevel * 1002);
        block.properties = m_blockPropertiesTemplates["stone"];
        block.properties.hardness = 1.5f + powerLevel * 0.2f;
        block.properties.lightLevel = 0.4f + powerLevel * 0.1f;
        block.properties.drops = {runeType + "_runestone"};

        block.appearance.textureType = BlockTextureType::PATTERN;
        block.appearance.primaryColor = Vec3(0.3f, 0.3f, 0.4f);
        block.appearance.secondaryColor = Vec3(0.8f, 0.8f, 0.9f);
        block.appearance.accentColor = Vec3(1.0f, 1.0f, 1.0f);
        block.appearance.hasGlow = true;
        block.appearance.variant = std::uniform_int_distribution<int>(0, 8)(m_randomEngine);

        block.textureData = GenerateBlockTexture(block);
        block.modelData = GenerateBlockModel(block);
        block.soundData = GenerateBlockSounds(block);
        block.particleData = GenerateBlockParticles(block);

        block.tags = GenerateBlockTags(block);
        m_generationStats["runestone_magical_blocks"]++;

        return block;
    }

    GeneratedBlock BlockGenerator::GenerateObsidianBlock(const std::string& obsidianType, int powerLevel) {
        GeneratedBlock block;
        block.type = BlockGenerationType::MAGICAL;
        block.name = obsidianType + "_obsidian";
        block.displayName = std::string(1, std::toupper(obsidianType[0])) + obsidianType.substr(1) + " Obsidian";
        block.materialType = "obsidian";
        block.isMagical = true;

        block.id = GenerateUniqueBlockName(obsidianType + "_obsidian", m_seed + powerLevel * 1003);
        block.properties = m_blockPropertiesTemplates["stone"];
        block.properties.hardness = 50.0f + powerLevel * 10.0f;
        block.properties.resistance = 1200.0f + powerLevel * 200.0f;
        block.properties.lightLevel = 0.1f + powerLevel * 0.1f;
        block.properties.drops = {obsidianType + "_obsidian"};

        block.appearance.textureType = BlockTextureType::CRYSTAL;
        block.appearance.primaryColor = Vec3(0.1f, 0.1f, 0.2f);
        block.appearance.secondaryColor = Vec3(0.2f, 0.2f, 0.3f);
        block.appearance.accentColor = Vec3(0.3f, 0.3f, 0.4f);
        block.appearance.hasGlow = (powerLevel > 2);
        block.appearance.variant = std::uniform_int_distribution<int>(0, 3)(m_randomEngine);

        block.textureData = GenerateBlockTexture(block);
        block.modelData = GenerateBlockModel(block);
        block.soundData = GenerateBlockSounds(block);
        block.particleData = GenerateBlockParticles(block);

        block.tags = GenerateBlockTags(block);
        m_generationStats["obsidian_magical_blocks"]++;

        return block;
    }

    // Texture generation implementations (simplified)
    void* BlockGenerator::GenerateSolidTexture(const GeneratedBlock& block) {
        BlockTexture* texture = new BlockTexture();
        texture->width = 16;
        texture->height = 16;
        texture->depth = 1;
        texture->type = BlockTextureType::SOLID;
        texture->data.resize(texture->width * texture->height * 4);

        // Generate solid color texture
        for (int y = 0; y < texture->height; ++y) {
            for (int x = 0; x < texture->width; ++x) {
                int index = (y * texture->width + x) * 4;
                texture->data[index] = static_cast<unsigned char>(block.appearance.primaryColor.x * 255);
                texture->data[index + 1] = static_cast<unsigned char>(block.appearance.primaryColor.y * 255);
                texture->data[index + 2] = static_cast<unsigned char>(block.appearance.primaryColor.z * 255);
                texture->data[index + 3] = 255;
            }
        }

        return texture;
    }

    void* BlockGenerator::GenerateNoiseTexture(const GeneratedBlock& block) {
        BlockTexture* texture = new BlockTexture();
        texture->width = 32;
        texture->height = 32;
        texture->depth = 1;
        texture->type = BlockTextureType::NOISE;
        texture->data.resize(texture->width * texture->height * 4);

        // Generate noise-based texture
        for (int y = 0; y < texture->height; ++y) {
            for (int x = 0; x < texture->width; ++x) {
                int index = (y * texture->width + x) * 4;
                float noise = static_cast<float>((x + y + block.appearance.variant) % 256) / 255.0f;

                texture->data[index] = static_cast<unsigned char>((block.appearance.primaryColor.x + noise * 0.2f) * 255);
                texture->data[index + 1] = static_cast<unsigned char>((block.appearance.primaryColor.y + noise * 0.2f) * 255);
                texture->data[index + 2] = static_cast<unsigned char>((block.appearance.primaryColor.z + noise * 0.2f) * 255);
                texture->data[index + 3] = 255;
            }
        }

        return texture;
    }

    void* BlockGenerator::GenerateGradientTexture(const GeneratedBlock& block) {
        BlockTexture* texture = new BlockTexture();
        texture->width = 16;
        texture->height = 16;
        texture->depth = 1;
        texture->type = BlockTextureType::GRADIENT;
        texture->data.resize(texture->width * texture->height * 4);

        // Generate gradient texture
        for (int y = 0; y < texture->height; ++y) {
            for (int x = 0; x < texture->width; ++x) {
                int index = (y * texture->width + x) * 4;
                float factor = static_cast<float>(y) / texture->height;

                texture->data[index] = static_cast<unsigned char>(((1 - factor) * block.appearance.primaryColor.x + factor * block.appearance.secondaryColor.x) * 255);
                texture->data[index + 1] = static_cast<unsigned char>(((1 - factor) * block.appearance.primaryColor.y + factor * block.appearance.secondaryColor.y) * 255);
                texture->data[index + 2] = static_cast<unsigned char>(((1 - factor) * block.appearance.primaryColor.z + factor * block.appearance.secondaryColor.z) * 255);
                texture->data[index + 3] = 255;
            }
        }

        return texture;
    }

    void* BlockGenerator::GeneratePatternTexture(const GeneratedBlock& block) {
        BlockTexture* texture = new BlockTexture();
        texture->width = 32;
        texture->height = 32;
        texture->depth = 1;
        texture->type = BlockTextureType::PATTERN;
        texture->data.resize(texture->width * texture->height * 4);

        // Generate pattern-based texture
        for (int y = 0; y < texture->height; ++y) {
            for (int x = 0; x < texture->width; ++x) {
                int index = (y * texture->width + x) * 4;
                bool pattern = ((x / 4) + (y / 4)) % 2 == 0;

                if (pattern) {
                    texture->data[index] = static_cast<unsigned char>(block.appearance.primaryColor.x * 255);
                    texture->data[index + 1] = static_cast<unsigned char>(block.appearance.primaryColor.y * 255);
                    texture->data[index + 2] = static_cast<unsigned char>(block.appearance.primaryColor.z * 255);
                } else {
                    texture->data[index] = static_cast<unsigned char>(block.appearance.secondaryColor.x * 255);
                    texture->data[index + 1] = static_cast<unsigned char>(block.appearance.secondaryColor.y * 255);
                    texture->data[index + 2] = static_cast<unsigned char>(block.appearance.secondaryColor.z * 255);
                }
                texture->data[index + 3] = 255;
            }
        }

        return texture;
    }

    void* BlockGenerator::GenerateCrystalTexture(const GeneratedBlock& block) {
        BlockTexture* texture = new BlockTexture();
        texture->width = 32;
        texture->height = 32;
        texture->depth = 1;
        texture->type = BlockTextureType::CRYSTAL;
        texture->data.resize(texture->width * texture->height * 4);

        // Generate crystal-like texture with transparency
        for (int y = 0; y < texture->height; ++y) {
            for (int x = 0; x < texture->width; ++x) {
                int index = (y * texture->width + x) * 4;
                float crystal = std::sin(x * 0.3f) * std::cos(y * 0.3f);
                float alpha = 0.7f + crystal * 0.3f;

                texture->data[index] = static_cast<unsigned char>(block.appearance.primaryColor.x * 255);
                texture->data[index + 1] = static_cast<unsigned char>(block.appearance.primaryColor.y * 255);
                texture->data[index + 2] = static_cast<unsigned char>(block.appearance.primaryColor.z * 255);
                texture->data[index + 3] = static_cast<unsigned char>(alpha * 255);
            }
        }

        texture->hasTransparency = true;
        return texture;
    }

    void* BlockGenerator::GenerateOrganicTexture(const GeneratedBlock& block) {
        BlockTexture* texture = new BlockTexture();
        texture->width = 32;
        texture->height = 32;
        texture->depth = 1;
        texture->type = BlockTextureType::ORGANIC;
        texture->data.resize(texture->width * texture->height * 4);

        // Generate organic texture with natural variations
        for (int y = 0; y < texture->height; ++y) {
            for (int x = 0; x < texture->width; ++x) {
                int index = (y * texture->width + x) * 4;
                float organic = std::sin(x * 0.1f + y * 0.1f) * 0.3f + 0.7f;

                texture->data[index] = static_cast<unsigned char>(block.appearance.primaryColor.x * organic * 255);
                texture->data[index + 1] = static_cast<unsigned char>(block.appearance.primaryColor.y * organic * 255);
                texture->data[index + 2] = static_cast<unsigned char>(block.appearance.primaryColor.z * organic * 255);
                texture->data[index + 3] = 255;
            }
        }

        return texture;
    }

    void* BlockGenerator::GenerateMetalTexture(const GeneratedBlock& block) {
        BlockTexture* texture = new BlockTexture();
        texture->width = 32;
        texture->height = 32;
        texture->depth = 1;
        texture->type = BlockTextureType::METAL;
        texture->data.resize(texture->width * texture->height * 4);

        // Generate metal texture with reflections
        for (int y = 0; y < texture->height; ++y) {
            for (int x = 0; x < texture->width; ++x) {
                int index = (y * texture->width + x) * 4;
                float metal = 0.8f + std::sin(x * 0.2f) * std::cos(y * 0.2f) * 0.2f;

                texture->data[index] = static_cast<unsigned char>(block.appearance.primaryColor.x * metal * 255);
                texture->data[index + 1] = static_cast<unsigned char>(block.appearance.primaryColor.y * metal * 255);
                texture->data[index + 2] = static_cast<unsigned char>(block.appearance.primaryColor.z * metal * 255);
                texture->data[index + 3] = 255;
            }
        }

        return texture;
    }

    void* BlockGenerator::GenerateStoneTexture(const GeneratedBlock& block) {
        BlockTexture* texture = new BlockTexture();
        texture->width = 32;
        texture->height = 32;
        texture->depth = 1;
        texture->type = BlockTextureType::STONE;
        texture->data.resize(texture->width * texture->height * 4);

        // Generate stone texture with cracks and variations
        for (int y = 0; y < texture->height; ++y) {
            for (int x = 0; x < texture->width; ++x) {
                int index = (y * texture->width + x) * 4;
                float stone = 0.6f + std::sin(x * 0.1f + y * 0.1f) * 0.3f;

                texture->data[index] = static_cast<unsigned char>(block.appearance.primaryColor.x * stone * 255);
                texture->data[index + 1] = static_cast<unsigned char>(block.appearance.primaryColor.y * stone * 255);
                texture->data[index + 2] = static_cast<unsigned char>(block.appearance.primaryColor.z * stone * 255);
                texture->data[index + 3] = 255;
            }
        }

        return texture;
    }

    void* BlockGenerator::GenerateWoodTexture(const GeneratedBlock& block) {
        BlockTexture* texture = new BlockTexture();
        texture->width = 32;
        texture->height = 32;
        texture->depth = 1;
        texture->type = BlockTextureType::WOOD;
        texture->data.resize(texture->width * texture->height * 4);

        // Generate wood texture with grain patterns
        for (int y = 0; y < texture->height; ++y) {
            for (int x = 0; x < texture->width; ++x) {
                int index = (y * texture->width + x) * 4;
                float grain = 0.7f + std::sin(x * 0.05f) * std::cos(y * 0.3f) * 0.3f;

                texture->data[index] = static_cast<unsigned char>(block.appearance.primaryColor.x * grain * 255);
                texture->data[index + 1] = static_cast<unsigned char>(block.appearance.primaryColor.y * grain * 255);
                texture->data[index + 2] = static_cast<unsigned char>(block.appearance.primaryColor.z * grain * 255);
                texture->data[index + 3] = 255;
            }
        }

        return texture;
    }

    void* BlockGenerator::GenerateLeafTexture(const GeneratedBlock& block) {
        BlockTexture* texture = new BlockTexture();
        texture->width = 16;
        texture->height = 16;
        texture->depth = 1;
        texture->type = BlockTextureType::LEAF;
        texture->data.resize(texture->width * texture->height * 4);

        // Generate leaf texture with transparency
        for (int y = 0; y < texture->height; ++y) {
            for (int x = 0; x < texture->width; ++x) {
                int index = (y * texture->width + x) * 4;
                float leaf = std::sin(x * 0.5f) * std::cos(y * 0.5f);
                float alpha = std::max(0.0f, leaf * 0.8f + 0.2f);

                texture->data[index] = static_cast<unsigned char>(block.appearance.primaryColor.x * 255);
                texture->data[index + 1] = static_cast<unsigned char>(block.appearance.primaryColor.y * 255);
                texture->data[index + 2] = static_cast<unsigned char>(block.appearance.primaryColor.z * 255);
                texture->data[index + 3] = static_cast<unsigned char>(alpha * 255);
            }
        }

        texture->hasTransparency = true;
        return texture;
    }

    void* BlockGenerator::GenerateFluidTexture(const GeneratedBlock& block) {
        BlockTexture* texture = new BlockTexture();
        texture->width = 32;
        texture->height = 32;
        texture->depth = 1;
        texture->type = BlockTextureType::FLUID;
        texture->data.resize(texture->width * texture->height * 4);

        // Generate fluid texture with wave effects
        for (int y = 0; y < texture->height; ++y) {
            for (int x = 0; x < texture->width; ++x) {
                int index = (y * texture->width + x) * 4;
                float wave = std::sin(x * 0.2f + y * 0.2f) * 0.2f + 0.8f;

                texture->data[index] = static_cast<unsigned char>(block.appearance.primaryColor.x * wave * 255);
                texture->data[index + 1] = static_cast<unsigned char>(block.appearance.primaryColor.y * wave * 255);
                texture->data[index + 2] = static_cast<unsigned char>(block.appearance.primaryColor.z * wave * 255);
                texture->data[index + 3] = 200; // Semi-transparent
            }
        }

        texture->hasTransparency = true;
        return texture;
    }

    // Model generation implementations (simplified)
    void* BlockGenerator::GenerateStandardBlockModel(const GeneratedBlock& block) {
        BlockModel* model = new BlockModel();

        // Standard 1x1x1 cube model
        model->vertices = {
            -0.5f, -0.5f, -0.5f,  // 0
             0.5f, -0.5f, -0.5f,  // 1
             0.5f,  0.5f, -0.5f,  // 2
            -0.5f,  0.5f, -0.5f,  // 3
            -0.5f, -0.5f,  0.5f,  // 4
             0.5f, -0.5f,  0.5f,  // 5
             0.5f,  0.5f,  0.5f,  // 6
            -0.5f,  0.5f,  0.5f   // 7
        };

        model->normals = {
            0.0f, 0.0f, -1.0f,  // Front
            0.0f, 0.0f, 1.0f,   // Back
            -1.0f, 0.0f, 0.0f,  // Left
            1.0f, 0.0f, 0.0f,   // Right
            0.0f, -1.0f, 0.0f,  // Bottom
            0.0f, 1.0f, 0.0f    // Top
        };

        model->texCoords = {
            0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,  // Front
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,  // Back
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,  // Left
            0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,  // Right
            0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,  // Bottom
            0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f   // Top
        };

        model->indices = {
            0, 1, 2, 0, 2, 3,    // Front
            4, 5, 6, 4, 6, 7,    // Back
            0, 3, 7, 0, 7, 4,    // Left
            1, 2, 6, 1, 6, 5,    // Right
            0, 1, 5, 0, 5, 4,    // Bottom
            3, 2, 6, 3, 6, 7     // Top
        };

        model->boundingBoxMin = Vec3(-0.5f, -0.5f, -0.5f);
        model->boundingBoxMax = Vec3(0.5f, 0.5f, 0.5f);
        model->scale = 1.0f;
        model->hasTransparency = block.properties.isTransparent;

        return model;
    }

    void* BlockGenerator::GenerateComplexBlockModel(const GeneratedBlock& block) {
        BlockModel* model = new BlockModel();

        // More complex model with additional geometry
        model->vertices = {
            // Base cube
            -0.5f, -0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
            -0.5f,  0.5f, -0.5f,
            -0.5f, -0.5f,  0.5f,
             0.5f, -0.5f,  0.5f,
             0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,
            // Additional details
            -0.3f,  0.6f, -0.3f,
             0.3f,  0.6f, -0.3f,
             0.3f,  0.6f,  0.3f,
            -0.3f,  0.6f,  0.3f
        };

        model->boundingBoxMin = Vec3(-0.5f, -0.5f, -0.5f);
        model->boundingBoxMax = Vec3(0.5f, 0.6f, 0.5f);
        model->scale = 1.0f;
        model->hasTransparency = block.properties.isTransparent;
        model->isDoubleSided = block.type == BlockGenerationType::VEGETATION;

        return model;
    }

    void* BlockGenerator::GenerateVegetationModel(const GeneratedBlock& block) {
        BlockModel* model = new BlockModel();

        // Cross-shaped vegetation model (like Minecraft grass/flowers)
        model->vertices = {
            // First cross
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.5f,  0.5f, 0.0f,
            -0.5f,  0.5f, 0.0f,
            // Second cross
            0.0f, -0.5f, -0.5f,
            0.0f, -0.5f,  0.5f,
            0.0f,  0.5f,  0.5f,
            0.0f,  0.5f, -0.5f
        };

        model->indices = {
            0, 1, 2, 0, 2, 3,    // First cross
            4, 5, 6, 4, 6, 7     // Second cross
        };

        model->boundingBoxMin = Vec3(-0.5f, -0.5f, -0.5f);
        model->boundingBoxMax = Vec3(0.5f, 0.5f, 0.5f);
        model->scale = 1.0f;
        model->hasTransparency = true;
        model->isDoubleSided = true;

        return model;
    }

    void* BlockGenerator::GenerateCrystalModel(const GeneratedBlock& block) {
        BlockModel* model = new BlockModel();

        // Crystal-shaped model with pointed top
        model->vertices = {
            -0.3f, -0.5f, -0.3f,
             0.3f, -0.5f, -0.3f,
             0.3f, -0.5f,  0.3f,
            -0.3f, -0.5f,  0.3f,
            -0.1f,  0.5f, -0.1f,
             0.1f,  0.5f, -0.1f,
             0.1f,  0.5f,  0.1f,
            -0.1f,  0.5f,  0.1f
        };

        model->indices = {
            0, 1, 4, 1, 5, 4, 1, 2, 5, 2, 6, 5,  // Bottom to top
            2, 3, 6, 3, 7, 6, 3, 0, 7, 0, 4, 7,  // Continue
            4, 5, 6, 4, 6, 7    // Top
        };

        model->boundingBoxMin = Vec3(-0.3f, -0.5f, -0.3f);
        model->boundingBoxMax = Vec3(0.3f, 0.5f, 0.3f);
        model->scale = 1.0f;
        model->hasTransparency = true;

        return model;
    }

    void* BlockGenerator::GenerateMagicalModel(const GeneratedBlock& block) {
        BlockModel* model = new BlockModel();

        // Magical floating model with effects
        model->vertices = {
            -0.4f, -0.4f, -0.4f,
             0.4f, -0.4f, -0.4f,
             0.4f,  0.4f, -0.4f,
            -0.4f,  0.4f, -0.4f,
            -0.4f, -0.4f,  0.4f,
             0.4f, -0.4f,  0.4f,
             0.4f,  0.4f,  0.4f,
            -0.4f,  0.4f,  0.4f,
            // Magical effects
             0.0f,  0.6f,  0.0f,
             0.0f, -0.6f,  0.0f
        };

        model->boundingBoxMin = Vec3(-0.4f, -0.6f, -0.4f);
        model->boundingBoxMax = Vec3(0.4f, 0.6f, 0.4f);
        model->scale = 1.0f;
        model->hasTransparency = true;

        return model;
    }

    // Sound generation implementations (simplified)
    void* BlockGenerator::GenerateStoneBlockSounds(const GeneratedBlock& block) {
        BlockSounds* sounds = new BlockSounds();
        sounds->placeSound = new char[20]{"stone_place"};
        sounds->breakSound = new char[20]{"stone_break"};
        sounds->stepSound = new char[20]{"stone_step"};
        sounds->hitSound = new char[20]{"stone_hit"};
        sounds->volume = 1.0f;
        sounds->pitch = 1.0f;
        sounds->variants = 4;
        return sounds;
    }

    void* BlockGenerator::GenerateWoodBlockSounds(const GeneratedBlock& block) {
        BlockSounds* sounds = new BlockSounds();
        sounds->placeSound = new char[20]{"wood_place"};
        sounds->breakSound = new char[20]{"wood_break"};
        sounds->stepSound = new char[20]{"wood_step"};
        sounds->hitSound = new char[20]{"wood_hit"};
        sounds->volume = 0.8f;
        sounds->pitch = 1.0f;
        sounds->variants = 3;
        return sounds;
    }

    void* BlockGenerator::GenerateMetalBlockSounds(const GeneratedBlock& block) {
        BlockSounds* sounds = new BlockSounds();
        sounds->placeSound = new char[20]{"metal_place"};
        sounds->breakSound = new char[20]{"metal_break"};
        sounds->stepSound = new char[20]{"metal_step"};
        sounds->hitSound = new char[20]{"metal_hit"};
        sounds->volume = 1.2f;
        sounds->pitch = 1.2f;
        sounds->variants = 2;
        return sounds;
    }

    void* BlockGenerator::GenerateOrganicBlockSounds(const GeneratedBlock& block) {
        BlockSounds* sounds = new BlockSounds();
        sounds->placeSound = new char[20]{"grass_place"};
        sounds->breakSound = new char[20]{"grass_break"};
        sounds->stepSound = new char[20]{"grass_step"};
        sounds->hitSound = new char[20]{"grass_hit"};
        sounds->volume = 0.6f;
        sounds->pitch = 1.0f;
        sounds->variants = 3;
        return sounds;
    }

    void* BlockGenerator::GenerateMagicalBlockSounds(const GeneratedBlock& block) {
        BlockSounds* sounds = new BlockSounds();
        sounds->placeSound = new char[20]{"magic_place"};
        sounds->breakSound = new char[20]{"magic_break"};
        sounds->stepSound = new char[20]{"magic_step"};
        sounds->hitSound = new char[20]{"magic_hit"};
        sounds->specialSound = new char[20]{"magic_special"};
        sounds->volume = 0.8f;
        sounds->pitch = 0.8f + (block.properties.lightLevel * 0.2f);
        sounds->variants = 5;
        sounds->hasEcho = true;
        sounds->echoDelay = 0.5f;
        return sounds;
    }

    // Particle generation implementations (simplified)
    void* BlockGenerator::GenerateStoneParticles(const GeneratedBlock& block) {
        BlockParticles* particles = new BlockParticles();
        particles->particleType = "stone";
        particles->particleColor = block.appearance.primaryColor;
        particles->particleVelocity = Vec3(0, 0, 0);
        particles->particleSize = 0.1f;
        particles->particleLifetime = 1.0f;
        particles->particleCount = 4;
        particles->spawnRate = 20.0f;
        particles->hasGravity = true;
        return particles;
    }

    void* BlockGenerator::GenerateWoodParticles(const GeneratedBlock& block) {
        BlockParticles* particles = new BlockParticles();
        particles->particleType = "wood";
        particles->particleColor = block.appearance.primaryColor;
        particles->particleVelocity = Vec3(0, 0, 0);
        particles->particleSize = 0.15f;
        particles->particleLifetime = 1.5f;
        particles->particleCount = 6;
        particles->spawnRate = 15.0f;
        particles->hasGravity = true;
        return particles;
    }

    void* BlockGenerator::GenerateLeafParticles(const GeneratedBlock& block) {
        BlockParticles* particles = new BlockParticles();
        particles->particleType = "leaf";
        particles->particleColor = block.appearance.primaryColor;
        particles->particleVelocity = Vec3(0, -0.5f, 0);
        particles->particleSize = 0.2f;
        particles->particleLifetime = 2.0f;
        particles->particleCount = 8;
        particles->spawnRate = 10.0f;
        particles->hasGravity = true;
        return particles;
    }

    void* BlockGenerator::GenerateCrystalParticles(const GeneratedBlock& block) {
        BlockParticles* particles = new BlockParticles();
        particles->particleType = "crystal";
        particles->particleColor = block.appearance.primaryColor;
        particles->particleVelocity = Vec3(0, 0.2f, 0);
        particles->particleSize = 0.05f;
        particles->particleLifetime = 3.0f;
        particles->particleCount = 3;
        particles->spawnRate = 5.0f;
        particles->hasGravity = false;
        particles->hasLight = true;
        particles->lightColor = block.appearance.emissiveColor;
        particles->lightIntensity = block.appearance.emissiveColor.x;
        return particles;
    }

    void* BlockGenerator::GenerateMagicalParticles(const GeneratedBlock& block) {
        BlockParticles* particles = new BlockParticles();
        particles->particleType = "magical";
        particles->particleColor = block.appearance.primaryColor;
        particles->particleVelocity = Vec3(0, 0.1f, 0);
        particles->particleSize = 0.08f;
        particles->particleLifetime = 4.0f;
        particles->particleCount = 5;
        particles->spawnRate = 8.0f;
        particles->hasGravity = false;
        particles->hasLight = true;
        particles->lightColor = block.appearance.emissiveColor;
        particles->lightIntensity = block.appearance.emissiveColor.x * 2.0f;
        return particles;
    }

} // namespace VoxelCraft
