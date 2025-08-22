/**
 * @file ProceduralGenerator.cpp
 * @brief VoxelCraft Procedural Generation System Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "ProceduralGenerator.hpp"
#include "EntityManager.hpp"
#include "Entity.hpp"
#include "TransformComponent.hpp"
#include "RenderComponent.hpp"
#include "Logger.hpp"

#include <algorithm>
#include <cmath>

namespace VoxelCraft {

    // NoiseGenerator implementation

    NoiseGenerator::NoiseGenerator(int seed)
        : m_seed(seed)
        , m_rng(seed)
    {
        // Initialize permutation table
        m_permutation.resize(256);
        for (int i = 0; i < 256; ++i) {
            m_permutation[i] = i;
        }

        // Shuffle permutation table
        std::shuffle(m_permutation.begin(), m_permutation.end(), m_rng);

        // Duplicate for overflow
        m_permutation.insert(m_permutation.end(), m_permutation.begin(), m_permutation.end());

        VOXELCRAFT_TRACE("NoiseGenerator initialized with seed {}", seed);
    }

    float NoiseGenerator::PerlinNoise2D(float x, float y, float scale, int octaves, float persistence, float lacunarity) {
        x *= scale;
        y *= scale;

        float value = 0.0f;
        float amplitude = 1.0f;
        float frequency = 1.0f;

        for (int i = 0; i < octaves; ++i) {
            float noiseValue = 0.0f;

            // Get grid coordinates
            int x0 = static_cast<int>(std::floor(x * frequency));
            int y0 = static_cast<int>(std::floor(y * frequency));
            int x1 = x0 + 1;
            int y1 = y0 + 1;

            // Get fractional parts
            float fx = (x * frequency) - x0;
            float fy = (y * frequency) - y0;

            // Get gradients
            float g00 = Grad(m_permutation[(m_permutation[x0 & 255] + y0) & 255], fx, fy);
            float g10 = Grad(m_permutation[(m_permutation[x1 & 255] + y0) & 255], fx - 1, fy);
            float g01 = Grad(m_permutation[(m_permutation[x0 & 255] + y1) & 255], fx, fy - 1);
            float g11 = Grad(m_permutation[(m_permutation[x1 & 255] + y1) & 255], fx - 1, fy - 1);

            // Interpolate
            float u = Fade(fx);
            float v = Fade(fy);
            float nx0 = Lerp(g00, g10, u);
            float nx1 = Lerp(g01, g11, u);
            noiseValue = Lerp(nx0, nx1, v);

            value += noiseValue * amplitude;
            amplitude *= persistence;
            frequency *= lacunarity;
        }

        return value;
    }

    float NoiseGenerator::PerlinNoise3D(float x, float y, float z, float scale, int octaves, float persistence, float lacunarity) {
        x *= scale;
        y *= scale;
        z *= scale;

        float value = 0.0f;
        float amplitude = 1.0f;
        float frequency = 1.0f;

        for (int i = 0; i < octaves; ++i) {
            float noiseValue = 0.0f;

            // Get grid coordinates
            int x0 = static_cast<int>(std::floor(x * frequency));
            int y0 = static_cast<int>(std::floor(y * frequency));
            int z0 = static_cast<int>(std::floor(z * frequency));
            int x1 = x0 + 1;
            int y1 = y0 + 1;
            int z1 = z0 + 1;

            // Get fractional parts
            float fx = (x * frequency) - x0;
            float fy = (y * frequency) - y0;
            float fz = (z * frequency) - z0;

            // Get gradients
            float g000 = Grad(m_permutation[(m_permutation[(m_permutation[x0 & 255] + y0) & 255] + z0) & 255], fx, fy, fz);
            float g100 = Grad(m_permutation[(m_permutation[(m_permutation[x1 & 255] + y0) & 255] + z0) & 255], fx - 1, fy, fz);
            float g010 = Grad(m_permutation[(m_permutation[(m_permutation[x0 & 255] + y1) & 255] + z0) & 255], fx, fy - 1, fz);
            float g110 = Grad(m_permutation[(m_permutation[(m_permutation[x1 & 255] + y1) & 255] + z0) & 255], fx - 1, fy - 1, fz);
            float g001 = Grad(m_permutation[(m_permutation[(m_permutation[x0 & 255] + y0) & 255] + z1) & 255], fx, fy, fz - 1);
            float g101 = Grad(m_permutation[(m_permutation[(m_permutation[x1 & 255] + y0) & 255] + z1) & 255], fx - 1, fy, fz - 1);
            float g011 = Grad(m_permutation[(m_permutation[(m_permutation[x0 & 255] + y1) & 255] + z1) & 255], fx, fy - 1, fz - 1);
            float g111 = Grad(m_permutation[(m_permutation[(m_permutation[x1 & 255] + y1) & 255] + z1) & 255], fx - 1, fy - 1, fz - 1);

            // Interpolate
            float u = Fade(fx);
            float v = Fade(fy);
            float w = Fade(fz);

            float nx00 = Lerp(g000, g100, u);
            float nx01 = Lerp(g001, g101, u);
            float nx10 = Lerp(g010, g110, u);
            float nx11 = Lerp(g011, g111, u);

            float nxy0 = Lerp(nx00, nx10, v);
            float nxy1 = Lerp(nx01, nx11, v);

            noiseValue = Lerp(nxy0, nxy1, w);

            value += noiseValue * amplitude;
            amplitude *= persistence;
            frequency *= lacunarity;
        }

        return value;
    }

    float NoiseGenerator::RidgedNoise(float x, float y, float scale, int octaves) {
        x *= scale;
        y *= scale;

        float value = 0.0f;
        float amplitude = 1.0f;
        float frequency = 1.0f;

        for (int i = 0; i < octaves; ++i) {
            float noise = PerlinNoise2D(x * frequency, y * frequency, 1.0f, 1, 1.0f, 1.0f);
            noise = std::abs(noise); // Absolute value for ridges
            noise = 1.0f - noise;    // Invert for ridges
            value += noise * amplitude;
            amplitude *= 0.5f;
            frequency *= 2.0f;
        }

        return value;
    }

    float NoiseGenerator::CellularNoise(float x, float y, float scale) {
        x *= scale;
        y *= scale;

        // Simple cellular noise implementation
        int xi = static_cast<int>(x);
        int yi = static_cast<int>(y);

        float minDist = 1.0f;
        for (int i = -1; i <= 1; ++i) {
            for (int j = -1; j <= 1; ++j) {
                int px = xi + i;
                int py = yi + j;

                // Use hash for pseudo-random point
                int hash = m_permutation[(m_permutation[px & 255] + py) & 255];
                float rx = static_cast<float>(hash % 1000) / 1000.0f;
                float ry = static_cast<float>((hash / 1000) % 1000) / 1000.0f;

                float dx = x - (px + rx);
                float dy = y - (py + ry);
                float dist = std::sqrt(dx * dx + dy * dy);
                minDist = std::min(minDist, dist);
            }
        }

        return minDist;
    }

    void NoiseGenerator::SetSeed(int seed) {
        m_seed = seed;
        m_rng.seed(seed);

        // Reinitialize permutation table
        for (int i = 0; i < 256; ++i) {
            m_permutation[i] = i;
        }
        std::shuffle(m_permutation.begin(), m_permutation.end(), m_rng);
        m_permutation.insert(m_permutation.end(), m_permutation.begin(), m_permutation.end());
    }

    float NoiseGenerator::Fade(float t) const {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }

    float NoiseGenerator::Lerp(float a, float b, float t) const {
        return a + t * (b - a);
    }

    float NoiseGenerator::Grad(int hash, float x, float y) const {
        int h = hash & 7;
        float u = h < 4 ? x : y;
        float v = h < 4 ? y : x;
        return ((h & 1) ? -u : u) + ((h & 2) ? -2.0f * v : 2.0f * v);
    }

    float NoiseGenerator::Grad(int hash, float x, float y, float z) const {
        int h = hash & 15;
        float u = h < 8 ? x : y;
        float v = h < 4 ? y : h == 12 || h == 14 ? x : z;
        return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
    }

    // ProceduralGenerator implementation

    ProceduralGenerator::ProceduralGenerator()
        : m_entityManager(nullptr)
        , m_parameters()
        , m_noiseGenerator(m_parameters.seed)
        , m_initialized(false)
    {
        VOXELCRAFT_TRACE("ProceduralGenerator created");
    }

    ProceduralGenerator::~ProceduralGenerator() {
        VOXELCRAFT_TRACE("ProceduralGenerator destroyed");
    }

    bool ProceduralGenerator::Initialize(EntityManager* entityManager) {
        if (m_initialized) {
            return true;
        }

        m_entityManager = entityManager;

        // Set noise generator seed
        m_noiseGenerator.SetSeed(m_parameters.seed);

        // Register default generators
        RegisterGenerator(GenerationType::Terrain,
            [this](const Vec3& center, float radius) {
                return GenerateTerrain(center, radius);
            });

        RegisterGenerator(GenerationType::Vegetation,
            [this](const Vec3& center, float radius) {
                return GenerateVegetation(center, radius);
            });

        RegisterGenerator(GenerationType::Structures,
            [this](const Vec3& center, float radius) {
                return GenerateStructures(center, radius);
            });

        m_initialized = true;
        VOXELCRAFT_INFO("ProceduralGenerator initialized with seed {}", m_parameters.seed);
        return true;
    }

    void ProceduralGenerator::SetParameters(const GenerationParameters& params) {
        m_parameters = params;
        m_noiseGenerator.SetSeed(m_parameters.seed);
    }

    std::vector<ProceduralObject> ProceduralGenerator::GenerateTerrain(const Vec3& center, float radius) {
        std::vector<ProceduralObject> objects;

        // Generate terrain chunks
        int chunkSize = 16;
        int numChunks = static_cast<int>(radius / chunkSize) + 1;

        for (int x = -numChunks; x <= numChunks; ++x) {
            for (int z = -numChunks; z <= numChunks; ++z) {
                Vec3 chunkPos(
                    center.x + x * chunkSize,
                    0.0f,
                    center.z + z * chunkSize
                );

                // Skip if too far
                float dist = std::sqrt(chunkPos.x * chunkPos.x + chunkPos.z * chunkPos.z);
                if (dist > radius) continue;

                ProceduralObject terrainChunk;
                terrainChunk.type = "terrain";
                terrainChunk.name = "TerrainChunk_" + std::to_string(x) + "_" + std::to_string(z);
                terrainChunk.position = chunkPos;
                terrainChunk.scale = Vec3(static_cast<float>(chunkSize), 1.0f, static_cast<float>(chunkSize));
                terrainChunk.properties["height"] = GenerateTerrainHeight(chunkPos.x, chunkPos.z);

                objects.push_back(terrainChunk);
            }
        }

        VOXELCRAFT_INFO("Generated {} terrain chunks", objects.size());
        return objects;
    }

    std::vector<ProceduralObject> ProceduralGenerator::GenerateVegetation(const Vec3& center, float radius) {
        std::vector<ProceduralObject> objects;

        // Generate trees and plants
        int numVegetation = static_cast<int>(radius * radius * 0.01f);

        for (int i = 0; i < numVegetation; ++i) {
            Vec3 pos = GetRandomPositionInCircle(center, radius);

            // Determine vegetation type based on biome
            std::string biome = DetermineBiome(pos.x, pos.z);
            std::string vegType = "tree";

            if (biome == "forest") {
                vegType = (m_noiseGenerator.PerlinNoise2D(pos.x, pos.z, 0.1f) > 0.0f) ? "tree" : "bush";
            } else if (biome == "plains") {
                vegType = "grass";
            }

            ProceduralObject vegetation;
            vegetation.type = "vegetation";
            vegetation.name = vegType + "_" + std::to_string(i);
            vegetation.position = pos;
            vegetation.properties["biome"] = 0.0f; // Could map biome to float

            objects.push_back(vegetation);
        }

        VOXELCRAFT_INFO("Generated {} vegetation objects", objects.size());
        return objects;
    }

    std::vector<ProceduralObject> ProceduralGenerator::GenerateStructures(const Vec3& center, float radius) {
        std::vector<ProceduralObject> objects;

        // Generate structures (houses, caves, etc.)
        int numStructures = static_cast<int>(radius * 0.1f);
        if (numStructures < 1) numStructures = 1;

        for (int i = 0; i < numStructures; ++i) {
            Vec3 pos = GetRandomPositionInCircle(center, radius * 0.7f); // Keep away from edges

            ProceduralObject structure;
            structure.type = "structure";
            structure.name = "Structure_" + std::to_string(i);
            structure.position = pos;

            // Random rotation
            structure.rotation = Vec3(0.0f, m_noiseGenerator.PerlinNoise2D(pos.x, pos.z) * 360.0f, 0.0f);

            objects.push_back(structure);
        }

        VOXELCRAFT_INFO("Generated {} structures", objects.size());
        return objects;
    }

    std::vector<ProceduralObject> ProceduralGenerator::GenerateObjects(const Vec3& center, float radius) {
        std::vector<ProceduralObject> objects;

        // Generate environmental objects (rocks, debris, etc.)
        int numObjects = static_cast<int>(radius * radius * 0.05f);

        for (int i = 0; i < numObjects; ++i) {
            Vec3 pos = GetRandomPositionInCircle(center, radius);

            ProceduralObject object;
            object.type = "object";
            object.name = "Object_" + std::to_string(i);
            object.position = pos;
            object.scale = Vec3(
                0.5f + m_noiseGenerator.PerlinNoise2D(pos.x, pos.z) * 0.5f,
                0.5f + m_noiseGenerator.PerlinNoise2D(pos.x + 1000, pos.z) * 0.5f,
                0.5f + m_noiseGenerator.PerlinNoise2D(pos.x, pos.z + 1000) * 0.5f
            );

            objects.push_back(object);
        }

        VOXELCRAFT_INFO("Generated {} objects", objects.size());
        return objects;
    }

    std::vector<ProceduralObject> ProceduralGenerator::GenerateEnemies(const Vec3& center, float radius) {
        std::vector<ProceduralObject> objects;

        // Generate enemy spawn points
        int numEnemies = static_cast<int>(radius * 0.2f);
        if (numEnemies < 1) numEnemies = 1;

        for (int i = 0; i < numEnemies; ++i) {
            Vec3 pos = GetRandomPositionInCircle(center, radius * 0.8f);

            ProceduralObject enemy;
            enemy.type = "enemy";
            enemy.name = "Enemy_" + std::to_string(i);
            enemy.position = pos;
            enemy.properties["difficulty"] = m_noiseGenerator.PerlinNoise2D(pos.x, pos.z, 0.1f) * 10.0f;

            objects.push_back(enemy);
        }

        VOXELCRAFT_INFO("Generated {} enemy spawn points", objects.size());
        return objects;
    }

    std::vector<ProceduralObject> ProceduralGenerator::GenerateResources(const Vec3& center, float radius) {
        std::vector<ProceduralObject> objects;

        // Generate resource nodes
        int numResources = static_cast<int>(radius * radius * 0.02f);

        for (int i = 0; i < numResources; ++i) {
            Vec3 pos = GetRandomPositionInCircle(center, radius);

            ProceduralObject resource;
            resource.type = "resource";
            resource.name = "Resource_" + std::to_string(i);
            resource.position = pos;
            resource.properties["amount"] = 50.0f + m_noiseGenerator.PerlinNoise2D(pos.x, pos.z) * 50.0f;

            objects.push_back(resource);
        }

        VOXELCRAFT_INFO("Generated {} resource nodes", objects.size());
        return objects;
    }

    std::vector<ProceduralObject> ProceduralGenerator::GenerateAll(const Vec3& center, float radius) {
        std::vector<ProceduralObject> allObjects;

        // Generate all types
        auto terrain = GenerateTerrain(center, radius);
        auto vegetation = GenerateVegetation(center, radius);
        auto structures = GenerateStructures(center, radius);
        auto objects = GenerateObjects(center, radius);
        auto enemies = GenerateEnemies(center, radius);
        auto resources = GenerateResources(center, radius);

        // Combine all
        allObjects.insert(allObjects.end(), terrain.begin(), terrain.end());
        allObjects.insert(allObjects.end(), vegetation.begin(), vegetation.end());
        allObjects.insert(allObjects.end(), structures.begin(), structures.end());
        allObjects.insert(allObjects.end(), objects.begin(), objects.end());
        allObjects.insert(allObjects.end(), enemies.begin(), enemies.end());
        allObjects.insert(allObjects.end(), resources.begin(), resources.end());

        m_generatedObjects = allObjects;
        VOXELCRAFT_INFO("Generated {} total procedural objects", allObjects.size());

        return allObjects;
    }

    size_t ProceduralGenerator::CreateEntitiesFromObjects(const std::vector<ProceduralObject>& objects) {
        if (!m_entityManager) {
            return 0;
        }

        size_t created = 0;
        for (const auto& object : objects) {
            auto entity = m_entityManager->CreateEntity(object.name);
            if (entity) {
                // Add transform component
                auto transform = entity->AddComponent<TransformComponent>(object.position);
                if (transform) {
                    transform->SetRotation(object.rotation);
                    transform->SetScale(object.scale);
                }

                // Add render component for visual objects
                if (object.type != "enemy" && object.type != "resource") {
                    entity->AddComponent<RenderComponent>();
                }

                created++;
            }
        }

        VOXELCRAFT_INFO("Created {} entities from procedural objects", created);
        return created;
    }

    size_t ProceduralGenerator::ClearArea(const Vec3& center, float radius) {
        if (!m_entityManager) {
            return 0;
        }

        size_t cleared = 0;
        auto entities = m_entityManager->GetAllEntities();

        for (auto entity : entities) {
            if (entity->GetName().find("Procedural") != std::string::npos ||
                entity->GetName().find("Terrain") != std::string::npos) {

                auto transform = entity->GetComponent<TransformComponent>();
                if (transform) {
                    Vec3 pos = transform->GetPosition();
                    float dist = std::sqrt(
                        (pos.x - center.x) * (pos.x - center.x) +
                        (pos.z - center.z) * (pos.z - center.z)
                    );

                    if (dist <= radius) {
                        m_entityManager->DestroyEntity(entity);
                        cleared++;
                    }
                }
            }
        }

        VOXELCRAFT_INFO("Cleared {} entities in area", cleared);
        return cleared;
    }

    void ProceduralGenerator::RegisterGenerator(GenerationType type,
                                              std::function<std::vector<ProceduralObject>(const Vec3&, float)> function) {
        m_generators[type] = function;
    }

    float ProceduralGenerator::GenerateTerrainHeight(float x, float z) {
        // Base height
        float baseHeight = m_noiseGenerator.PerlinNoise2D(x, z, m_parameters.noiseScale,
                                                         m_parameters.octaves,
                                                         m_parameters.persistence,
                                                         m_parameters.lacunarity) * 10.0f;

        // Add hills and mountains
        float hills = m_noiseGenerator.RidgedNoise(x, z, m_parameters.noiseScale * 2.0f) * 5.0f;

        // Add small details
        float details = m_noiseGenerator.PerlinNoise2D(x, z, m_parameters.noiseScale * 4.0f) * 2.0f;

        return baseHeight + hills + details;
    }

    std::string ProceduralGenerator::DetermineBiome(float x, float z) {
        float moisture = m_noiseGenerator.PerlinNoise2D(x + 1000, z + 1000, 0.01f);
        float temperature = m_noiseGenerator.PerlinNoise2D(x + 2000, z + 2000, 0.01f);

        if (temperature > 0.6f) {
            return moisture > 0.5f ? "desert" : "savanna";
        } else if (temperature > 0.3f) {
            return moisture > 0.5f ? "forest" : "plains";
        } else {
            return moisture > 0.5f ? "taiga" : "tundra";
        }
    }

    Vec3 ProceduralGenerator::GetRandomPositionInCircle(const Vec3& center, float radius) {
        // Generate random angle and distance
        float angle = m_noiseGenerator.PerlinNoise2D(center.x, center.z) * 6.28318f; // 2*PI
        float distance = std::sqrt(m_noiseGenerator.PerlinNoise2D(center.x + 1000, center.z + 1000)) * radius;

        return Vec3(
            center.x + std::cos(angle) * distance,
            0.0f,
            center.z + std::sin(angle) * distance
        );
    }

} // namespace VoxelCraft
