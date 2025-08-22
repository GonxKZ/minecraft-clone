/**
 * @file ProceduralGenerator.cpp
 * @brief VoxelCraft Procedural Generation System - Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "ProceduralGenerator.hpp"
#include <algorithm>
#include <cmath>
#include <ctime>
#include <sstream>
#include <iomanip>

namespace VoxelCraft {

    // PerlinNoise Implementation
    PerlinNoise::PerlinNoise(uint64_t seed) : m_seed(seed) {
        InitializePermutation();
    }

    void PerlinNoise::InitializePermutation() {
        // Create permutation table
        for (int i = 0; i < 256; ++i) {
            m_permutation[i] = i;
        }

        // Shuffle using seed
        std::mt19937_64 engine(m_seed);
        std::shuffle(m_permutation.begin(), m_permutation.end(), engine);

        // Duplicate for overflow
        for (int i = 0; i < 256; ++i) {
            m_permutation[256 + i] = m_permutation[i];
        }
    }

    float PerlinNoise::Fade(float t) {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }

    float PerlinNoise::Gradient(int hash, float x, float z) {
        int h = hash & 15;
        float u = h < 8 ? x : z;
        float v = h < 4 ? z : (h == 12 || h == 14 ? x : 0);
        return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
    }

    float PerlinNoise::Noise2D(float x, float z) {
        int X = static_cast<int>(std::floor(x)) & 255;
        int Z = static_cast<int>(std::floor(z)) & 255;

        x -= std::floor(x);
        z -= std::floor(z);

        float u = Fade(x);
        float v = Fade(z);

        int A = m_permutation[X] + Z;
        int B = m_permutation[X + 1] + Z;

        return Lerp(v,
                   Lerp(u, Gradient(m_permutation[A], x, z),
                       Gradient(m_permutation[B], x - 1, z)),
                   Lerp(u, Gradient(m_permutation[A + 1], x, z - 1),
                       Gradient(m_permutation[B + 1], x - 1, z - 1)));
    }

    float PerlinNoise::Noise3D(float x, float y, float z) {
        int X = static_cast<int>(std::floor(x)) & 255;
        int Y = static_cast<int>(std::floor(y)) & 255;
        int Z = static_cast<int>(std::floor(z)) & 255;

        x -= std::floor(x);
        y -= std::floor(y);
        z -= std::floor(z);

        float u = Fade(x);
        float v = Fade(y);
        float w = Fade(z);

        int A = m_permutation[X] + Y;
        int AA = m_permutation[A] + Z;
        int AB = m_permutation[A + 1] + Z;
        int B = m_permutation[X + 1] + Y;
        int BA = m_permutation[B] + Z;
        int BB = m_permutation[B + 1] + Z;

        return Lerp(w, Lerp(v, Lerp(u, Gradient(m_permutation[AA], x, y, z),
                                   Gradient(m_permutation[BA], x - 1, y, z)),
                           Lerp(u, Gradient(m_permutation[AB], x, y - 1, z),
                               Gradient(m_permutation[BB], x - 1, y - 1, z))),
                   Lerp(v, Lerp(u, Gradient(m_permutation[AA + 1], x, y, z - 1),
                               Gradient(m_permutation[BA + 1], x - 1, y, z - 1)),
                           Lerp(u, Gradient(m_permutation[AB + 1], x, y - 1, z - 1),
                               Gradient(m_permutation[BB + 1], x - 1, y - 1, z - 1))));
    }

    float PerlinNoise::OctaveNoise2D(float x, float z, int octaves, float persistence) {
        float value = 0.0f;
        float amplitude = 1.0f;
        float frequency = 1.0f;
        float maxValue = 0.0f;

        for (int i = 0; i < octaves; ++i) {
            value += Noise2D(x * frequency, z * frequency) * amplitude;
            maxValue += amplitude;
            amplitude *= persistence;
            frequency *= 2.0f;
        }

        return value / maxValue;
    }

    // ProceduralGenerator Implementation
    ProceduralGenerator::ProceduralGenerator() {
        // Initialize with default parameters
        m_params = GenerationParameters();
        m_randomEngine.seed(std::chrono::system_clock::now().time_since_epoch().count());
    }

    ProceduralGenerator::~ProceduralGenerator() {
        // Cleanup generated data
    }

    void ProceduralGenerator::Initialize(const GenerationParameters& params) {
        m_params = params;
        m_randomEngine.seed(m_params.seed);
        InitializeNoiseGenerators();
        InitializeSkinGenerators();
        InitializeSoundGenerators();
    }

    void ProceduralGenerator::SetSeed(uint64_t seed) {
        m_params.seed = seed;
        m_randomEngine.seed(seed);
        InitializeNoiseGenerators();
    }

    void ProceduralGenerator::InitializeNoiseGenerators() {
        m_terrainNoise = std::make_unique<PerlinNoise>(m_params.seed);
        m_caveNoise = std::make_unique<PerlinNoise>(m_params.seed + 1);
        m_oreNoise = std::make_unique<PerlinNoise>(m_params.seed + 2);
        m_vegetationNoise = std::make_unique<PerlinNoise>(m_params.seed + 3);
    }

    void ProceduralGenerator::InitializeSkinGenerators() {
        // Player skin generator
        m_skinGenerators["player"] = [this](uint64_t seed) -> void* {
            return GeneratePlayerSkin(seed);
        };

        // Mob skin generators
        m_skinGenerators["zombie"] = [this](uint64_t seed) -> void* {
            return GenerateZombieSkin(seed);
        };

        m_skinGenerators["skeleton"] = [this](uint64_t seed) -> void* {
            return GenerateSkeletonSkin(seed);
        };

        m_skinGenerators["creeper"] = [this](uint64_t seed) -> void* {
            return GenerateCreeperSkin(seed);
        };

        m_skinGenerators["spider"] = [this](uint64_t seed) -> void* {
            return GenerateSpiderSkin(seed);
        };

        m_skinGenerators["cow"] = [this](uint64_t seed) -> void* {
            return GenerateCowSkin(seed);
        };

        m_skinGenerators["pig"] = [this](uint64_t seed) -> void* {
            return GeneratePigSkin(seed);
        };

        m_skinGenerators["chicken"] = [this](uint64_t seed) -> void* {
            return GenerateChickenSkin(seed);
        };

        // Block texture generators
        m_skinGenerators["grass_block"] = [this](uint64_t seed) -> void* {
            return GenerateGrassBlockTexture(seed);
        };

        m_skinGenerators["stone_block"] = [this](uint64_t seed) -> void* {
            return GenerateStoneBlockTexture(seed);
        };

        m_skinGenerators["dirt_block"] = [this](uint64_t seed) -> void* {
            return GenerateDirtBlockTexture(seed);
        };

        m_skinGenerators["wood_block"] = [this](uint64_t seed) -> void* {
            return GenerateWoodBlockTexture(seed);
        };

        m_skinGenerators["ore_block"] = [this](uint64_t seed) -> void* {
            return GenerateOreBlockTexture(seed);
        };

        // Item skin generators
        m_skinGenerators["tool"] = [this](uint64_t seed) -> void* {
            return GenerateToolSkin(seed);
        };

        m_skinGenerators["weapon"] = [this](uint64_t seed) -> void* {
            return GenerateWeaponSkin(seed);
        };
    }

    void ProceduralGenerator::InitializeSoundGenerators() {
        // Player sound generators
        m_soundGenerators["player_walk"] = [this](uint64_t seed) -> void* {
            return GeneratePlayerWalkSound(seed);
        };

        m_soundGenerators["player_hurt"] = [this](uint64_t seed) -> void* {
            return GeneratePlayerHurtSound(seed);
        };

        m_soundGenerators["player_die"] = [this](uint64_t seed) -> void* {
            return GeneratePlayerDeathSound(seed);
        };

        // Block sound generators
        m_soundGenerators["block_break"] = [this](uint64_t seed) -> void* {
            return GenerateBlockBreakSound(seed);
        };

        m_soundGenerators["block_place"] = [this](uint64_t seed) -> void* {
            return GenerateBlockPlaceSound(seed);
        };

        m_soundGenerators["block_step"] = [this](uint64_t seed) -> void* {
            return GenerateBlockStepSound(seed);
        };

        // Mob sound generators
        m_soundGenerators["zombie_idle"] = [this](uint64_t seed) -> void* {
            return GenerateZombieIdleSound(seed);
        };

        m_soundGenerators["zombie_hurt"] = [this](uint64_t seed) -> void* {
            return GenerateZombieHurtSound(seed);
        };

        m_soundGenerators["skeleton_shoot"] = [this](uint64_t seed) -> void* {
            return GenerateSkeletonShootSound(seed);
        };

        m_soundGenerators["creeper_hiss"] = [this](uint64_t seed) -> void* {
            return GenerateCreeperHissSound(seed);
        };

        m_soundGenerators["creeper_explosion"] = [this](uint64_t seed) -> void* {
            return GenerateCreeperExplosionSound(seed);
        };

        // Item sound generators
        m_soundGenerators["tool_break"] = [this](uint64_t seed) -> void* {
            return GenerateToolBreakSound(seed);
        };

        m_soundGenerators["item_pickup"] = [this](uint64_t seed) -> void* {
            return GenerateItemPickupSound(seed);
        };

        // Environmental sound generators
        m_soundGenerators["ambient_cave"] = [this](uint64_t seed) -> void* {
            return GenerateAmbientCaveSound(seed);
        };

        m_soundGenerators["ambient_forest"] = [this](uint64_t seed) -> void* {
            return GenerateAmbientForestSound(seed);
        };

        m_soundGenerators["weather_rain"] = [this](uint64_t seed) -> void* {
            return GenerateRainSound(seed);
        };

        m_soundGenerators["weather_thunder"] = [this](uint64_t seed) -> void* {
            return GenerateThunderSound(seed);
        };
    }

    std::vector<ProceduralObject> ProceduralGenerator::GenerateAll(const Vec3& position, float radius) {
        std::vector<ProceduralObject> allObjects;

        // Generate terrain
        auto terrainObjects = GenerateTerrain(position);
        allObjects.insert(allObjects.end(), terrainObjects.begin(), terrainObjects.end());

        // Generate biomes
        auto biomeObjects = GenerateBiomes(position);
        allObjects.insert(allObjects.end(), biomeObjects.begin(), biomeObjects.end());

        // Generate caves if enabled
        if (m_params.generateCaves) {
            auto caveObjects = GenerateCaves(position);
            allObjects.insert(allObjects.end(), caveObjects.begin(), caveObjects.end());
        }

        // Generate structures if enabled
        if (m_params.generateStructures) {
            auto structureObjects = GenerateStructures(position);
            allObjects.insert(allObjects.end(), structureObjects.begin(), structureObjects.end());
        }

        // Generate ores if enabled
        if (m_params.generateOres) {
            auto oreObjects = GenerateOres(position);
            allObjects.insert(allObjects.end(), oreObjects.begin(), oreObjects.end());
        }

        // Generate vegetation if enabled
        if (m_params.generateVegetation) {
            auto vegetationObjects = GenerateVegetation(position);
            allObjects.insert(allObjects.end(), vegetationObjects.begin(), vegetationObjects.end());
        }

        // Generate mobs if enabled
        if (m_params.generateMobs) {
            auto mobObjects = GenerateMobs(position);
            allObjects.insert(allObjects.end(), mobObjects.begin(), mobObjects.end());
        }

        // Generate items
        auto itemObjects = GenerateItems(position);
        allObjects.insert(allObjects.end(), itemObjects.begin(), itemObjects.end());

        return allObjects;
    }

    std::vector<ProceduralObject> ProceduralGenerator::GenerateTerrain(const Vec3& position) {
        std::vector<ProceduralObject> objects;
        int chunkRadius = static_cast<int>(std::ceil(100.0f / 16.0f)); // 100 block radius

        for (int x = -chunkRadius; x <= chunkRadius; ++x) {
            for (int z = -chunkRadius; z <= chunkRadius; ++z) {
                Vec3 chunkPos = Vec3(
                    position.x + x * 16.0f,
                    position.y,
                    position.z + z * 16.0f
                );

                // Generate terrain height for each block in chunk
                for (int localX = 0; localX < 16; ++localX) {
                    for (int localZ = 0; localZ < 16; ++localZ) {
                        float worldX = chunkPos.x + localX;
                        float worldZ = chunkPos.z + localZ;

                        float height = GenerateTerrainHeight(worldX, worldZ);
                        std::string biome = GenerateBiome(worldX, worldZ);

                        // Generate blocks from bottom to top
                        for (int y = m_params.minHeight; y <= height; ++y) {
                            ProceduralObject block;
                            block.position = Vec3(worldX, static_cast<float>(y), worldZ);

                            if (y == height) {
                                // Surface block
                                if (biome == "forest" || biome == "plains") {
                                    block.type = "grass_block";
                                    block.properties["biome"] = biome;
                                } else if (biome == "desert") {
                                    block.type = "sand_block";
                                } else if (biome == "snow") {
                                    block.type = "snow_block";
                                } else {
                                    block.type = "grass_block";
                                }
                            } else if (y > height - 3) {
                                // Sub-surface
                                if (biome == "desert") {
                                    block.type = "sand_block";
                                } else {
                                    block.type = "dirt_block";
                                }
                            } else {
                                // Underground
                                block.type = "stone_block";
                            }

                            // Generate unique ID
                            std::stringstream ss;
                            ss << "block_" << worldX << "_" << y << "_" << worldZ;
                            block.id = ss.str();

                            // Generate texture/skin for block
                            block.generatedData = GenerateSkin(block.type, m_params.seed + (x * 16 + localX) * 31 + (z * 16 + localZ) * 37 + y * 41);

                            objects.push_back(block);
                        }
                    }
                }
            }
        }

        return objects;
    }

    std::vector<ProceduralObject> ProceduralGenerator::GenerateBiomes(const Vec3& position) {
        std::vector<ProceduralObject> objects;
        int radius = 100;

        for (int x = -radius; x <= radius; x += 16) {
            for (int z = -radius; z <= radius; z += 16) {
                Vec3 biomePos = Vec3(position.x + x, position.y, position.z + z);
                std::string biomeType = GenerateBiome(biomePos.x, biomePos.z);

                ProceduralObject biome;
                biome.position = biomePos;
                biome.type = "biome";
                biome.properties["biome_type"] = biomeType;
                biome.properties["temperature"] = std::to_string(GenerateBiomeTemperature(biomeType));
                biome.properties["humidity"] = std::to_string(GenerateBiomeHumidity(biomeType));

                std::stringstream ss;
                ss << "biome_" << x << "_" << z;
                biome.id = ss.str();

                objects.push_back(biome);
            }
        }

        return objects;
    }

    std::vector<ProceduralObject> ProceduralGenerator::GenerateStructures(const Vec3& position) {
        std::vector<ProceduralObject> objects;
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        int radius = 200;

        for (int x = -radius; x <= radius; x += 32) {
            for (int z = -radius; z <= radius; z += 32) {
                if (dist(m_randomEngine) < 0.01f) { // 1% chance per chunk
                    Vec3 structurePos = Vec3(position.x + x, position.y, position.z + z);
                    std::string structureType = GenerateStructureType(structurePos);

                    auto structureObjects = GenerateStructure(structurePos, structureType);
                    objects.insert(objects.end(), structureObjects.begin(), structureObjects.end());
                }
            }
        }

        return objects;
    }

    std::vector<ProceduralObject> ProceduralGenerator::GenerateCaves(const Vec3& position) {
        std::vector<ProceduralObject> objects;
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        int radius = 100;

        for (int x = -radius; x <= radius; x += 16) {
            for (int z = -radius; z <= radius; z += 16) {
                if (dist(m_randomEngine) < m_params.caveDensity) {
                    Vec3 cavePos = Vec3(position.x + x, position.y, position.z + z);
                    ProceduralObject cave = GenerateCave(cavePos);
                    objects.push_back(cave);
                }
            }
        }

        return objects;
    }

    std::vector<ProceduralObject> ProceduralGenerator::GenerateOres(const Vec3& position) {
        std::vector<ProceduralObject> objects;
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        int radius = 50;

        for (int x = -radius; x <= radius; x += 8) {
            for (int z = -radius; z <= radius; z += 8) {
                if (dist(m_randomEngine) < m_params.oreDensity) {
                    Vec3 orePos = Vec3(position.x + x, position.y, position.z + z);
                    auto oreObjects = GenerateOreDeposit(orePos);
                    objects.insert(objects.end(), oreObjects.begin(), oreObjects.end());
                }
            }
        }

        return objects;
    }

    std::vector<ProceduralObject> ProceduralGenerator::GenerateVegetation(const Vec3& position) {
        std::vector<ProceduralObject> objects;
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        int radius = 50;

        for (int x = -radius; x <= radius; x += 3) {
            for (int z = -radius; z <= radius; z += 3) {
                if (dist(m_randomEngine) < m_params.vegetationDensity) {
                    Vec3 vegPos = Vec3(position.x + x, position.y, position.z + z);
                    std::string biome = GenerateBiome(vegPos.x, vegPos.z);
                    auto vegObjects = GenerateTree(vegPos, biome);
                    objects.insert(objects.end(), vegObjects.begin(), vegObjects.end());
                }
            }
        }

        return objects;
    }

    std::vector<ProceduralObject> ProceduralGenerator::GenerateMobs(const Vec3& position) {
        std::vector<ProceduralObject> objects;
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        int radius = 30;

        for (int x = -radius; x <= radius; x += 10) {
            for (int z = -radius; z <= radius; z += 10) {
                if (dist(m_randomEngine) < m_params.mobDensity) {
                    Vec3 mobPos = Vec3(position.x + x, position.y, position.z + z);
                    std::string biome = GenerateBiome(mobPos.x, mobPos.z);
                    ProceduralObject mob = GenerateMob(mobPos, biome);
                    objects.push_back(mob);
                }
            }
        }

        return objects;
    }

    std::vector<ProceduralObject> ProceduralGenerator::GenerateItems(const Vec3& position) {
        std::vector<ProceduralObject> objects;
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        int radius = 20;

        for (int x = -radius; x <= radius; x += 5) {
            for (int z = -radius; z <= radius; z += 5) {
                if (dist(m_randomEngine) < 0.02f) { // 2% chance for items
                    Vec3 itemPos = Vec3(position.x + x, position.y, position.z + z);
                    ProceduralObject item = GenerateItem(itemPos);
                    objects.push_back(item);
                }
            }
        }

        return objects;
    }

    void* ProceduralGenerator::GenerateSkin(const std::string& objectType, uint64_t seed) {
        auto it = m_skinGenerators.find(objectType);
        if (it != m_skinGenerators.end()) {
            return it->second(seed);
        }
        return nullptr;
    }

    void* ProceduralGenerator::GenerateSound(const std::string& objectType, uint64_t seed) {
        auto it = m_soundGenerators.find(objectType);
        if (it != m_soundGenerators.end()) {
            return it->second(seed);
        }
        return nullptr;
    }

    // Helper methods for generation
    float ProceduralGenerator::GenerateTerrainHeight(float x, float z) {
        return m_terrainNoise->OctaveNoise2D(x, z, m_params.octaves, m_params.persistence) *
               m_params.amplitude + 64.0f;
    }

    std::string ProceduralGenerator::GenerateBiome(float x, float z) {
        float temperature = m_terrainNoise->Noise2D(x * 0.001f, z * 0.001f);
        float humidity = m_terrainNoise->Noise2D(x * 0.001f + 1000.0f, z * 0.001f + 1000.0f);

        if (temperature < -0.3f && humidity > 0.3f) {
            return "snow";
        } else if (temperature > 0.5f && humidity < -0.3f) {
            return "desert";
        } else if (temperature > 0.2f && humidity > 0.2f) {
            return "forest";
        } else {
            return "plains";
        }
    }

    bool ProceduralGenerator::IsInCave(float x, float y, float z) {
        float caveNoise = m_caveNoise->Noise3D(x * 0.01f, y * 0.01f, z * 0.01f);
        return caveNoise > 0.8f;
    }

    ProceduralObject ProceduralGenerator::GenerateCave(const Vec3& position) {
        ProceduralObject cave;
        cave.position = position;
        cave.type = "cave";
        cave.properties["size"] = "large";
        cave.properties["depth"] = "deep";
        return cave;
    }

    std::vector<ProceduralObject> ProceduralGenerator::GenerateTree(const Vec3& position, const std::string& biome) {
        std::vector<ProceduralObject> objects;
        float height = 5.0f + (std::uniform_real_distribution<float>(0.0f, 2.0f)(m_randomEngine));

        // Tree trunk
        for (int y = 0; y < height; ++y) {
            ProceduralObject trunkBlock;
            trunkBlock.position = Vec3(position.x, position.y + y, position.z);
            trunkBlock.type = "wood_block";
            trunkBlock.properties["tree_part"] = "trunk";
            objects.push_back(trunkBlock);
        }

        // Tree leaves
        for (int x = -2; x <= 2; ++x) {
            for (int z = -2; z <= 2; ++z) {
                for (int y = 3; y <= 5; ++y) {
                    if (abs(x) + abs(z) + abs(y - 4) <= 4) {
                        ProceduralObject leafBlock;
                        leafBlock.position = Vec3(position.x + x, position.y + y, position.z + z);
                        leafBlock.type = "leaves_block";
                        leafBlock.properties["tree_part"] = "leaves";
                        objects.push_back(leafBlock);
                    }
                }
            }
        }

        return objects;
    }

    std::vector<ProceduralObject> ProceduralGenerator::GenerateOreDeposit(const Vec3& position) {
        std::vector<ProceduralObject> objects;
        std::string oreType = GenerateOreType();
        int size = 3 + std::uniform_int_distribution<int>(0, 3)(m_randomEngine);

        for (int x = -size; x <= size; ++x) {
            for (int y = -size; y <= size; ++y) {
                for (int z = -size; z <= size; ++z) {
                    if (x * x + y * y + z * z <= size * size) {
                        ProceduralObject oreBlock;
                        oreBlock.position = Vec3(position.x + x, position.y + y, position.z + z);
                        oreBlock.type = oreType + "_ore_block";
                        oreBlock.properties["ore_type"] = oreType;
                        objects.push_back(oreBlock);
                    }
                }
            }
        }

        return objects;
    }

    ProceduralObject ProceduralGenerator::GenerateMob(const Vec3& position, const std::string& biome) {
        ProceduralObject mob;
        mob.position = position;
        mob.type = "mob";

        std::string mobType = GenerateMobType(biome);
        mob.properties["mob_type"] = mobType;
        mob.properties["health"] = GenerateMobHealth(mobType);
        mob.properties["damage"] = GenerateMobDamage(mobType);
        mob.properties["speed"] = GenerateMobSpeed(mobType);

        // Generate unique skin for this mob
        mob.generatedData = GenerateSkin(mobType, m_params.seed + static_cast<uint64_t>(position.x * 100 + position.z * 200));

        return mob;
    }

    ProceduralObject ProceduralGenerator::GenerateItem(const Vec3& position) {
        ProceduralObject item;
        item.position = Vec3(position.x, position.y + 0.5f, position.z);
        item.type = "item";

        std::string itemType = GenerateItemType();
        item.properties["item_type"] = itemType;
        item.properties["durability"] = GenerateItemDurability(itemType);
        item.properties["damage"] = GenerateItemDamage(itemType);

        // Generate unique appearance for this item
        item.generatedData = GenerateSkin("tool", m_params.seed + static_cast<uint64_t>(position.x * 150 + position.z * 250));

        return item;
    }

    std::vector<ProceduralObject> ProceduralGenerator::GenerateStructure(const Vec3& position, const std::string& structureType) {
        std::vector<ProceduralObject> objects;

        if (structureType == "house") {
            // Generate simple house
            for (int x = 0; x < 7; ++x) {
                for (int z = 0; z < 7; ++z) {
                    // Floor
                    ProceduralObject floorBlock;
                    floorBlock.position = Vec3(position.x + x, position.y, position.z + z);
                    floorBlock.type = "wood_planks_block";
                    objects.push_back(floorBlock);

                    // Walls (except doors)
                    if (x == 0 || x == 6 || z == 0 || z == 6) {
                        if (!(x == 3 && z == 0)) { // Leave door opening
                            for (int y = 1; y <= 3; ++y) {
                                ProceduralObject wallBlock;
                                wallBlock.position = Vec3(position.x + x, position.y + y, position.z + z);
                                wallBlock.type = "wood_planks_block";
                                objects.push_back(wallBlock);
                            }
                        }
                    }

                    // Roof
                    if (x >= 1 && x <= 5 && z >= 1 && z <= 5) {
                        ProceduralObject roofBlock;
                        roofBlock.position = Vec3(position.x + x, position.y + 4, position.z + z);
                        roofBlock.type = "wood_planks_block";
                        objects.push_back(roofBlock);
                    }
                }
            }
        }

        return objects;
    }

    // Private helper methods
    std::string ProceduralGenerator::GenerateOreType() {
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        float rand = dist(m_randomEngine);

        if (rand < 0.3f) return "coal";
        else if (rand < 0.5f) return "iron";
        else if (rand < 0.65f) return "gold";
        else if (rand < 0.8f) return "redstone";
        else if (rand < 0.9f) return "diamond";
        else return "emerald";
    }

    std::string ProceduralGenerator::GenerateMobType(const std::string& biome) {
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        float rand = dist(m_randomEngine);

        if (biome == "forest") {
            if (rand < 0.4f) return "zombie";
            else if (rand < 0.7f) return "skeleton";
            else if (rand < 0.9f) return "spider";
            else return "creeper";
        } else if (biome == "plains") {
            if (rand < 0.3f) return "zombie";
            else if (rand < 0.6f) return "cow";
            else if (rand < 0.8f) return "pig";
            else return "chicken";
        } else if (biome == "desert") {
            if (rand < 0.6f) return "zombie";
            else return "skeleton";
        } else { // snow
            if (rand < 0.5f) return "zombie";
            else return "skeleton";
        }
    }

    std::string ProceduralGenerator::GenerateItemType() {
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        float rand = dist(m_randomEngine);

        if (rand < 0.2f) return "wooden_sword";
        else if (rand < 0.4f) return "wooden_pickaxe";
        else if (rand < 0.6f) return "wooden_axe";
        else if (rand < 0.8f) return "wooden_shovel";
        else return "stick";
    }

    std::string ProceduralGenerator::GenerateStructureType(const Vec3& position) {
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        float rand = dist(m_randomEngine);

        if (rand < 0.7f) return "house";
        else if (rand < 0.9f) return "tower";
        else return "dungeon";
    }

    std::string ProceduralGenerator::GenerateMobHealth(const std::string& mobType) {
        if (mobType == "zombie" || mobType == "skeleton") return "20";
        else if (mobType == "creeper" || mobType == "spider") return "20";
        else if (mobType == "cow" || mobType == "pig") return "10";
        else if (mobType == "chicken") return "4";
        return "20";
    }

    std::string ProceduralGenerator::GenerateMobDamage(const std::string& mobType) {
        if (mobType == "zombie") return "3";
        else if (mobType == "skeleton") return "3";
        else if (mobType == "creeper") return "0"; // Special case for explosion
        else if (mobType == "spider") return "2";
        else return "0"; // Passive mobs
    }

    std::string ProceduralGenerator::GenerateMobSpeed(const std::string& mobType) {
        if (mobType == "zombie" || mobType == "pig") return "0.23";
        else if (mobType == "skeleton") return "0.25";
        else if (mobType == "creeper") return "0.2";
        else if (mobType == "spider") return "0.3";
        else if (mobType == "cow" || mobType == "chicken") return "0.2";
        return "0.2";
    }

    std::string ProceduralGenerator::GenerateItemDurability(const std::string& itemType) {
        if (itemType.find("wooden") != std::string::npos) return "59";
        else if (itemType == "stone") return "131";
        else if (itemType == "iron") return "250";
        else if (itemType == "diamond") return "1561";
        else if (itemType == "golden") return "32";
        return "1"; // For non-tools
    }

    std::string ProceduralGenerator::GenerateItemDamage(const std::string& itemType) {
        if (itemType.find("sword") != std::string::npos) return "4";
        else if (itemType.find("axe") != std::string::npos) return "3";
        else if (itemType.find("pickaxe") != std::string::npos) return "2";
        else if (itemType.find("shovel") != std::string::npos) return "2";
        return "1";
    }

    float ProceduralGenerator::GenerateBiomeTemperature(const std::string& biome) {
        if (biome == "snow") return -0.5f;
        else if (biome == "desert") return 1.0f;
        else if (biome == "forest") return 0.3f;
        else return 0.5f; // plains
    }

    float ProceduralGenerator::GenerateBiomeHumidity(const std::string& biome) {
        if (biome == "desert") return -0.5f;
        else if (biome == "forest") return 0.8f;
        else if (biome == "snow") return 0.4f;
        else return 0.3f; // plains
    }

    // Skin generation methods (simplified implementations)
    void* ProceduralGenerator::GeneratePlayerSkin(uint64_t seed) {
        // Generate player skin with procedural colors and features
        std::mt19937_64 gen(seed);
        std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);

        struct PlayerSkin {
            float skinColor[3];
            float hairColor[3];
            float eyeColor[3];
            float shirtColor[3];
            float pantsColor[3];
            int hairStyle;
            int faceFeatures;
        };

        PlayerSkin* skin = new PlayerSkin();
        skin->skinColor[0] = colorDist(gen) * 0.4f + 0.4f; // Red component
        skin->skinColor[1] = colorDist(gen) * 0.3f + 0.4f; // Green component
        skin->skinColor[2] = colorDist(gen) * 0.2f + 0.3f; // Blue component

        // Similar for other colors...
        return skin;
    }

    void* ProceduralGenerator::GenerateZombieSkin(uint64_t seed) {
        // Generate zombie skin with rotten appearance
        std::mt19937_64 gen(seed);
        std::uniform_real_distribution<float> decayDist(0.0f, 1.0f);

        struct ZombieSkin {
            float decayLevel;
            float skinTone[3];
            int missingParts;
            bool hasArmor;
        };

        ZombieSkin* skin = new ZombieSkin();
        skin->decayLevel = decayDist(gen);
        return skin;
    }

    void* ProceduralGenerator::GenerateSkeletonSkin(uint64_t seed) {
        // Generate skeleton appearance
        std::mt19937_64 gen(seed);
        std::uniform_int_distribution<int> boneDist(0, 3);

        struct SkeletonSkin {
            int boneCondition;
            bool hasHelmet;
            bool hasArmor;
            float glowLevel;
        };

        SkeletonSkin* skin = new SkeletonSkin();
        skin->boneCondition = boneDist(gen);
        return skin;
    }

    void* ProceduralGenerator::GenerateCreeperSkin(uint64_t seed) {
        // Generate creeper appearance
        std::mt19937_64 gen(seed);
        std::uniform_real_distribution<float> sizeDist(0.8f, 1.2f);

        struct CreeperSkin {
            float size;
            float color[3];
            int pattern;
            bool isCharged;
        };

        CreeperSkin* skin = new CreeperSkin();
        skin->size = sizeDist(gen);
        return skin;
    }

    void* ProceduralGenerator::GenerateSpiderSkin(uint64_t seed) {
        // Generate spider appearance
        std::mt19937_64 gen(seed);
        std::uniform_real_distribution<float> patternDist(0.0f, 1.0f);

        struct SpiderSkin {
            float patternIntensity;
            float size;
            int eyeCount;
        };

        SpiderSkin* skin = new SpiderSkin();
        skin->patternIntensity = patternDist(gen);
        return skin;
    }

    void* ProceduralGenerator::GenerateCowSkin(uint64_t seed) {
        // Generate cow appearance
        std::mt19937_64 gen(seed);
        std::uniform_real_distribution<float> spotDist(0.0f, 1.0f);

        struct CowSkin {
            float spotDensity;
            float furColor[3];
            int patternType;
        };

        CowSkin* skin = new CowSkin();
        skin->spotDensity = spotDist(gen);
        return skin;
    }

    void* ProceduralGenerator::GeneratePigSkin(uint64_t seed) {
        // Generate pig appearance
        std::mt19937_64 gen(seed);
        std::uniform_real_distribution<float> pinkDist(0.8f, 1.0f);

        struct PigSkin {
            float pinkLevel;
            int spotCount;
            bool hasSaddle;
        };

        PigSkin* skin = new PigSkin();
        skin->pinkLevel = pinkDist(gen);
        return skin;
    }

    void* ProceduralGenerator::GenerateChickenSkin(uint64_t seed) {
        // Generate chicken appearance
        std::mt19937_64 gen(seed);
        std::uniform_real_distribution<float> featherDist(0.0f, 1.0f);

        struct ChickenSkin {
            float featherColor[3];
            int featherPattern;
            float combSize;
        };

        ChickenSkin* skin = new ChickenSkin();
        skin->combSize = featherDist(gen);
        return skin;
    }

    void* ProceduralGenerator::GenerateGrassBlockTexture(uint64_t seed) {
        // Generate grass texture with procedural variations
        std::mt19937_64 gen(seed);
        std::uniform_real_distribution<float> grassDist(0.0f, 1.0f);

        struct GrassTexture {
            float grassColor[3];
            float dirtColor[3];
            int grassHeight;
            int bladeCount;
        };

        GrassTexture* texture = new GrassTexture();
        texture->grassHeight = static_cast<int>(grassDist(gen) * 3) + 1;
        return texture;
    }

    void* ProceduralGenerator::GenerateStoneBlockTexture(uint64_t seed) {
        // Generate stone texture with cracks and variations
        std::mt19937_64 gen(seed);
        std::uniform_real_distribution<float> crackDist(0.0f, 1.0f);

        struct StoneTexture {
            float baseColor[3];
            int crackLevel;
            int grainPattern;
            bool hasMoss;
        };

        StoneTexture* texture = new StoneTexture();
        texture->crackLevel = static_cast<int>(crackDist(gen) * 4);
        return texture;
    }

    void* ProceduralGenerator::GenerateDirtBlockTexture(uint64_t seed) {
        // Generate dirt texture with variations
        std::mt19937_64 gen(seed);
        std::uniform_real_distribution<float> moistureDist(0.0f, 1.0f);

        struct DirtTexture {
            float color[3];
            int moistureLevel;
            int particleSize;
        };

        DirtTexture* texture = new DirtTexture();
        texture->moistureLevel = static_cast<int>(moistureDist(gen) * 3);
        return texture;
    }

    void* ProceduralGenerator::GenerateWoodBlockTexture(uint64_t seed) {
        // Generate wood texture with grain patterns
        std::mt19937_64 gen(seed);
        std::uniform_real_distribution<float> grainDist(0.0f, 1.0f);

        struct WoodTexture {
            float baseColor[3];
            int grainIntensity;
            int ringPattern;
            bool hasKnots;
        };

        WoodTexture* texture = new WoodTexture();
        texture->grainIntensity = static_cast<int>(grainDist(gen) * 3) + 1;
        return texture;
    }

    void* ProceduralGenerator::GenerateOreBlockTexture(uint64_t seed) {
        // Generate ore texture with mineral deposits
        std::mt19937_64 gen(seed);
        std::uniform_real_distribution<float> depositDist(0.0f, 1.0f);

        struct OreTexture {
            float stoneColor[3];
            float oreColor[3];
            int depositSize;
            int orePattern;
        };

        OreTexture* texture = new OreTexture();
        texture->depositSize = static_cast<int>(depositDist(gen) * 4) + 1;
        return texture;
    }

    void* ProceduralGenerator::GenerateToolSkin(uint64_t seed) {
        // Generate tool appearance
        std::mt19937_64 gen(seed);
        std::uniform_real_distribution<float> wearDist(0.0f, 1.0f);

        struct ToolSkin {
            float materialColor[3];
            int wearLevel;
            int patternType;
            bool hasEnchantment;
        };

        ToolSkin* skin = new ToolSkin();
        skin->wearLevel = static_cast<int>(wearDist(gen) * 5);
        return skin;
    }

    void* ProceduralGenerator::GenerateWeaponSkin(uint64_t seed) {
        // Generate weapon appearance
        std::mt19937_64 gen(seed);
        std::uniform_real_distribution<float> bloodDist(0.0f, 1.0f);

        struct WeaponSkin {
            float materialColor[3];
            int bloodLevel;
            int damageLevel;
            bool hasEnchantment;
        };

        WeaponSkin* skin = new WeaponSkin();
        skin->bloodLevel = static_cast<int>(bloodDist(gen) * 3);
        return skin;
    }

    // Sound generation methods (simplified implementations)
    void* ProceduralGenerator::GeneratePlayerWalkSound(uint64_t seed) {
        // Generate footstep sounds
        struct WalkSound {
            float frequency;
            float duration;
            int stepType;
        };
        return new WalkSound();
    }

    void* ProceduralGenerator::GeneratePlayerHurtSound(uint64_t seed) {
        struct HurtSound {
            float painLevel;
            int voiceType;
        };
        return new HurtSound();
    }

    void* ProceduralGenerator::GeneratePlayerDeathSound(uint64_t seed) {
        struct DeathSound {
            float finalBreath;
            int deathType;
        };
        return new DeathSound();
    }

    void* ProceduralGenerator::GenerateBlockBreakSound(uint64_t seed) {
        struct BreakSound {
            float pitch;
            int materialType;
        };
        return new BreakSound();
    }

    void* ProceduralGenerator::GenerateBlockPlaceSound(uint64_t seed) {
        struct PlaceSound {
            float volume;
            int placeType;
        };
        return new PlaceSound();
    }

    void* ProceduralGenerator::GenerateBlockStepSound(uint64_t seed) {
        struct StepSound {
            float echo;
            int surfaceType;
        };
        return new StepSound();
    }

    void* ProceduralGenerator::GenerateZombieIdleSound(uint64_t seed) {
        struct ZombieIdleSound {
            float groanLevel;
            int zombieType;
        };
        return new ZombieIdleSound();
    }

    void* ProceduralGenerator::GenerateZombieHurtSound(uint64_t seed) {
        struct ZombieHurtSound {
            float painLevel;
            int hurtType;
        };
        return new ZombieHurtSound();
    }

    void* ProceduralGenerator::GenerateSkeletonShootSound(uint64_t seed) {
        struct SkeletonShootSound {
            float arrowSpeed;
            int bowType;
        };
        return new SkeletonShootSound();
    }

    void* ProceduralGenerator::GenerateCreeperHissSound(uint64_t seed) {
        struct CreeperHissSound {
            float hissIntensity;
            int fuseState;
        };
        return new CreeperHissSound();
    }

    void* ProceduralGenerator::GenerateCreeperExplosionSound(uint64_t seed) {
        struct CreeperExplosionSound {
            float explosionSize;
            int explosionType;
        };
        return new CreeperExplosionSound();
    }

    void* ProceduralGenerator::GenerateToolBreakSound(uint64_t seed) {
        struct ToolBreakSound {
            float crackLevel;
            int materialType;
        };
        return new ToolBreakSound();
    }

    void* ProceduralGenerator::GenerateItemPickupSound(uint64_t seed) {
        struct ItemPickupSound {
            float itemSize;
            int itemType;
        };
        return new ItemPickupSound();
    }

    void* ProceduralGenerator::GenerateAmbientCaveSound(uint64_t seed) {
        struct AmbientCaveSound {
            float echoLevel;
            int caveSize;
        };
        return new AmbientCaveSound();
    }

    void* ProceduralGenerator::GenerateAmbientForestSound(uint64_t seed) {
        struct AmbientForestSound {
            float birdDensity;
            int forestType;
        };
        return new AmbientForestSound();
    }

    void* ProceduralGenerator::GenerateRainSound(uint64_t seed) {
        struct RainSound {
            float intensity;
            int rainType;
        };
        return new RainSound();
    }

    void* ProceduralGenerator::GenerateThunderSound(uint64_t seed) {
        struct ThunderSound {
            float distance;
            int thunderType;
        };
        return new ThunderSound();
    }

    std::vector<std::shared_ptr<Entity>> ProceduralGenerator::CreateEntitiesFromObjects(
        const std::vector<ProceduralObject>& objects,
        EntityManager* entityManager
    ) {
        std::vector<std::shared_ptr<Entity>> entities;

        for (const auto& obj : objects) {
            auto entity = entityManager->CreateEntity(obj.id);

            // Add transform component
            auto transform = entity->AddComponent<TransformComponent>();
            transform->SetPosition(obj.position);
            transform->SetRotation(obj.rotation);
            transform->SetScale(obj.scale);

            // Add render component if applicable
            if (obj.generatedData != nullptr) {
                auto render = entity->AddComponent<RenderComponent>();
                render->SetRenderType(RenderType::STATIC_MESH);
                // Set custom render data (texture, model, etc.)
            }

            entities.push_back(entity);
        }

        return entities;
    }

} // namespace VoxelCraft
