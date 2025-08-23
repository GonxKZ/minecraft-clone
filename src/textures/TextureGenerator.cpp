/**
 * @file TextureGenerator.cpp
 * @brief VoxelCraft Procedural Texture Generation System Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "TextureGenerator.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <queue>
#include <future>

#include "../math/MathUtils.hpp"
#include "../math/Noise.hpp"
#include "../utils/Logger.hpp"
#include "../utils/Random.hpp"
#include "../utils/ImageUtils.hpp"

namespace VoxelCraft {

    // Static instance
    static TextureGenerator* s_instance = nullptr;

    TextureGenerator& TextureGenerator::GetInstance() {
        if (!s_instance) {
            s_instance = new TextureGenerator();
        }
        return *s_instance;
    }

    bool TextureGenerator::Initialize(const std::string& configPath) {
        if (m_initialized) {
            Logger::Warning("TextureGenerator already initialized");
            return true;
        }

        // Initialize random engine
        std::random_device rd;
        m_randomEngine.seed(rd());
        m_nextHandle = 1;

        // Set default configuration
        m_config["maxTextureSize"] = 2048.0f;
        m_config["minTextureSize"] = 16.0f;
        m_config["defaultQuality"] = 5.0f;
        m_config["cacheSizeMB"] = 512.0f;
        m_config["maxConcurrentGenerations"] = 4.0f;
        m_config["enableCompression"] = 1.0f;
        m_config["enableMipmaps"] = 1.0f;
        m_config["enableAnisotropy"] = 0.0f;
        m_config["anisotropyLevel"] = 4.0f;
        m_config["noiseQuality"] = 1.0f;

        // Load configuration if provided
        if (!configPath.empty()) {
            // TODO: Load configuration from file
            Logger::Info("Loading texture generator config from: {}", configPath);
        }

        // Initialize texture presets
        InitializeTexturePresets();

        m_initialized = true;
        m_debugMode = false;

        Logger::Info("TextureGenerator initialized successfully");
        return true;
    }

    void TextureGenerator::Shutdown() {
        if (!m_initialized) {
            return;
        }

        // Cancel all async generations
        for (auto& pair : m_asyncGenerations) {
            pair.second.cancelled = true;
        }
        m_asyncGenerations.clear();

        // Clear cache
        m_textureCache.clear();
        m_texturePresets.clear();

        // Clear GPU resources
        if (m_computeShaderProgram != 0) {
            // TODO: Delete GPU resources
        }

        m_initialized = false;
        Logger::Info("TextureGenerator shutdown");
    }

    void TextureGenerator::Update(float deltaTime) {
        if (!m_initialized) {
            return;
        }

        // Update async generations
        UpdateAsyncGenerations();

        // Clean up old cache entries if needed
        if (m_textureCache.size() > 1000) {
            ClearCache(300.0f); // Clear textures older than 5 minutes
        }

        // Update statistics
        m_stats.texturesInCache = static_cast<int>(m_textureCache.size());
        m_stats.activeGenerators = static_cast<int>(m_asyncGenerations.size());
    }

    std::shared_ptr<TextureData> TextureGenerator::GenerateTexture(const ProceduralTexture& textureDef, uint32_t seed) {
        if (!m_initialized) {
            Logger::Error("TextureGenerator not initialized");
            return nullptr;
        }

        auto startTime = std::chrono::high_resolution_clock::now();

        try {
            // Create texture data
            auto textureData = std::make_shared<TextureData>();
            textureData->textureId = textureDef.id;
            textureData->format = textureDef.format;
            textureData->width = textureDef.width;
            textureData->height = textureDef.height;
            textureData->depth = textureDef.depth;
            textureData->generatorId = "TextureGenerator";
            textureData->generationTime = static_cast<uint64_t>(
                std::chrono::system_clock::now().time_since_epoch().count());

            // Set random seed
            if (seed == 0) {
                seed = RandomSeed();
            }

            // Calculate data size
            size_t bytesPerPixel = GetBytesPerPixel(textureDef.format);
            textureData->dataSize = textureDef.width * textureDef.height * textureDef.depth * bytesPerPixel;
            textureData->pixelData.resize(textureData->dataSize);

            // Generate texture layers
            std::vector<std::shared_ptr<TextureData>> layerTextures;
            for (const auto& layer : textureDef.layers) {
                if (layer.enabled) {
                    auto layerTexture = GenerateLayerTexture(layer, textureDef.width, textureDef.height);
                    if (layerTexture) {
                        layerTextures.push_back(layerTexture);
                    }
                }
            }

            // Composite layers
            if (!layerTextures.empty()) {
                // Start with first layer
                textureData = layerTextures[0];

                // Blend remaining layers
                for (size_t i = 1; i < layerTextures.size(); ++i) {
                    textureData = BlendTextures(textureData, layerTextures[i],
                                              textureDef.layers[i].blendMode,
                                              textureDef.layers[i].opacity);
                }
            }

            // Apply global effects
            if (textureDef.globalColorConfig.saturation != 1.0f ||
                textureDef.globalColorConfig.brightness != 1.0f ||
                textureDef.globalColorConfig.contrast != 1.0f) {
                std::unordered_map<std::string, float> effects;
                effects["saturation"] = textureDef.globalColorConfig.saturation;
                effects["brightness"] = textureDef.globalColorConfig.brightness;
                effects["contrast"] = textureDef.globalColorConfig.contrast;
                textureData = ApplyTextureEffects(textureData, effects);
            }

            // Generate mipmaps if requested
            if (textureDef.useMipmaps) {
                GenerateMipmaps(textureData);
            }

            // Compress if requested
            if (textureDef.useCompression) {
                CompressTexture(textureData);
            }

            // Calculate generation time
            auto endTime = std::chrono::high_resolution_clock::now();
            textureData->loadTime = std::chrono::duration<float>(endTime - startTime).count();

            // Update statistics
            m_stats.totalTexturesGenerated++;
            m_stats.averageGenerationTime = (m_stats.averageGenerationTime + textureData->loadTime) * 0.5f;
            m_stats.totalMemoryUsage += textureData->dataSize;

            Logger::Debug("Generated texture '{}' in {:.3f}s, size: {}x{}, {} bytes",
                         textureDef.id, textureData->loadTime, textureDef.width, textureDef.height,
                         textureData->dataSize);

            return textureData;

        } catch (const std::exception& e) {
            Logger::Error("Failed to generate texture '{}': {}", textureDef.id, e.what());
            m_stats.failedGenerations++;
            return nullptr;
        }
    }

    uint64_t TextureGenerator::GenerateTextureAsync(const ProceduralTexture& textureDef,
                                                   uint32_t seed,
                                                   std::function<void(std::shared_ptr<TextureData>)> callback) {
        if (!m_initialized) {
            Logger::Error("TextureGenerator not initialized");
            return 0;
        }

        uint64_t handle = m_nextHandle++;
        AsyncGeneration asyncGen;
        asyncGen.handle = handle;
        asyncGen.textureDef = textureDef;
        asyncGen.seed = seed;
        asyncGen.callback = callback;
        asyncGen.cancelled = false;

        m_asyncGenerations[handle] = asyncGen;

        // Launch async generation
        std::thread([this, handle]() {
            auto it = m_asyncGenerations.find(handle);
            if (it == m_asyncGenerations.end()) {
                return;
            }

            auto& asyncGen = it->second;
            if (asyncGen.cancelled) {
                return;
            }

            auto textureData = GenerateTexture(asyncGen.textureDef, asyncGen.seed);

            // Call callback on main thread
            if (asyncGen.callback && !asyncGen.cancelled) {
                // TODO: Queue for main thread execution
                asyncGen.callback(textureData);
            }

            m_asyncGenerations.erase(handle);
        }).detach();

        return handle;
    }

    bool TextureGenerator::CancelAsyncGeneration(uint64_t handle) {
        auto it = m_asyncGenerations.find(handle);
        if (it == m_asyncGenerations.end()) {
            return false;
        }

        it->second.cancelled = true;
        return true;
    }

    std::shared_ptr<TextureData> TextureGenerator::GenerateBlockTexture(const std::string& blockType,
                                                                      const std::string& biome,
                                                                      int quality) {
        ProceduralTexture textureDef;
        textureDef.id = "block_" + blockType + "_" + biome;
        textureDef.name = blockType + " Block";
        textureDef.type = TextureType::BLOCK;
        textureDef.width = 64;
        textureDef.height = 64;
        textureDef.qualityLevel = quality;

        // Configure based on block type
        if (blockType == "stone") {
            GenerateStoneTexture(textureDef);
        } else if (blockType == "dirt") {
            GenerateDirtTexture(textureDef, biome);
        } else if (blockType == "grass") {
            GenerateGrassTexture(textureDef, biome);
        } else if (blockType == "sand") {
            GenerateSandTexture(textureDef, biome);
        } else if (blockType == "wood") {
            GenerateWoodTexture(textureDef, biome);
        } else if (blockType == "leaves") {
            GenerateLeafTexture(textureDef, biome);
        } else if (blockType == "water") {
            GenerateWaterTexture(textureDef);
        } else {
            // Default block texture
            GenerateDefaultBlockTexture(textureDef);
        }

        return GenerateTexture(textureDef);
    }

    std::shared_ptr<TextureData> TextureGenerator::GenerateTerrainTexture(const Vec3& position,
                                                                        const std::string& biome,
                                                                        float height,
                                                                        float temperature,
                                                                        float moisture) {
        ProceduralTexture textureDef;
        textureDef.id = "terrain_" + biome + "_" + std::to_string(static_cast<int>(position.x)) +
                       "_" + std::to_string(static_cast<int>(position.z));
        textureDef.name = "Terrain Texture";
        textureDef.type = TextureType::TERRAIN;
        textureDef.width = 256;
        textureDef.height = 256;

        // Configure terrain texture based on parameters
        GenerateTerrainTexture(textureDef, biome, height, temperature, moisture);

        return GenerateTexture(textureDef);
    }

    std::shared_ptr<TextureData> TextureGenerator::GenerateBiomeTexture(const std::string& biome,
                                                                      const std::string& season,
                                                                      const std::string& weather) {
        ProceduralTexture textureDef;
        textureDef.id = "biome_" + biome + "_" + season + "_" + weather;
        textureDef.name = biome + " Biome Texture";
        textureDef.type = TextureType::BIOME;
        textureDef.width = 512;
        textureDef.height = 512;

        // Configure biome texture
        GenerateBiomeTexture(textureDef, biome, season, weather);

        return GenerateTexture(textureDef);
    }

    // Block texture generation methods
    void TextureGenerator::GenerateStoneTexture(ProceduralTexture& textureDef) {
        TextureLayer baseLayer;
        baseLayer.name = "stone_base";
        baseLayer.opacity = 1.0f;

        // Base color
        baseLayer.colorConfig.baseColor = Color(0.6f, 0.6f, 0.6f, 1.0f);
        baseLayer.colorConfig.secondaryColor = Color(0.5f, 0.5f, 0.5f, 1.0f);
        baseLayer.colorConfig.colorVariation = 0.2f;

        // Noise configuration
        baseLayer.noiseConfig.type = NoiseType::PERLIN;
        baseLayer.noiseConfig.frequency = 0.1f;
        baseLayer.noiseConfig.amplitude = 0.3f;
        baseLayer.noiseConfig.octaves = 3;

        // Pattern configuration
        baseLayer.patternConfig.patternType = "noise";
        baseLayer.patternConfig.patternScale = 0.5f;

        textureDef.layers.push_back(baseLayer);

        // Add detail layer
        TextureLayer detailLayer;
        detailLayer.name = "stone_detail";
        detailLayer.opacity = 0.3f;
        detailLayer.blendMode = "overlay";

        detailLayer.noiseConfig.type = NoiseType::SIMPLEX;
        detailLayer.noiseConfig.frequency = 0.5f;
        detailLayer.noiseConfig.amplitude = 0.2f;
        detailLayer.noiseConfig.octaves = 2;

        textureDef.layers.push_back(detailLayer);
    }

    void TextureGenerator::GenerateDirtTexture(ProceduralTexture& textureDef, const std::string& biome) {
        TextureLayer baseLayer;
        baseLayer.name = "dirt_base";
        baseLayer.opacity = 1.0f;

        // Base color based on biome
        if (biome == "desert") {
            baseLayer.colorConfig.baseColor = Color(0.8f, 0.7f, 0.5f, 1.0f);
        } else if (biome == "forest") {
            baseLayer.colorConfig.baseColor = Color(0.4f, 0.3f, 0.2f, 1.0f);
        } else {
            baseLayer.colorConfig.baseColor = Color(0.5f, 0.4f, 0.3f, 1.0f);
        }

        baseLayer.colorConfig.colorVariation = 0.15f;

        // Noise configuration
        baseLayer.noiseConfig.type = NoiseType::VALUE;
        baseLayer.noiseConfig.frequency = 0.08f;
        baseLayer.noiseConfig.amplitude = 0.4f;
        baseLayer.noiseConfig.octaves = 4;

        textureDef.layers.push_back(baseLayer);

        // Add grain layer
        TextureLayer grainLayer;
        grainLayer.name = "dirt_grain";
        grainLayer.opacity = 0.2f;
        grainLayer.blendMode = "multiply";

        grainLayer.noiseConfig.type = NoiseType::SIMPLEX;
        grainLayer.noiseConfig.frequency = 1.0f;
        grainLayer.noiseConfig.amplitude = 0.3f;

        textureDef.layers.push_back(grainLayer);
    }

    void TextureGenerator::GenerateGrassTexture(ProceduralTexture& textureDef, const std::string& biome) {
        TextureLayer baseLayer;
        baseLayer.name = "grass_base";
        baseLayer.opacity = 1.0f;

        // Base color based on biome
        if (biome == "plains") {
            baseLayer.colorConfig.baseColor = Color(0.2f, 0.6f, 0.1f, 1.0f);
        } else if (biome == "forest") {
            baseLayer.colorConfig.baseColor = Color(0.1f, 0.5f, 0.1f, 1.0f);
        } else if (biome == "jungle") {
            baseLayer.colorConfig.baseColor = Color(0.0f, 0.4f, 0.0f, 1.0f);
        } else {
            baseLayer.colorConfig.baseColor = Color(0.15f, 0.55f, 0.05f, 1.0f);
        }

        baseLayer.colorConfig.colorVariation = 0.25f;

        // Noise configuration for grass pattern
        baseLayer.noiseConfig.type = NoiseType::PERLIN;
        baseLayer.noiseConfig.frequency = 0.2f;
        baseLayer.noiseConfig.amplitude = 0.3f;
        baseLayer.noiseConfig.octaves = 3;

        textureDef.layers.push_back(baseLayer);

        // Add variation layer
        TextureLayer variationLayer;
        variationLayer.name = "grass_variation";
        variationLayer.opacity = 0.4f;
        variationLayer.blendMode = "overlay";

        variationLayer.noiseConfig.type = NoiseType::SIMPLEX;
        variationLayer.noiseConfig.frequency = 0.8f;
        variationLayer.noiseConfig.amplitude = 0.2f;

        textureDef.layers.push_back(variationLayer);
    }

    void TextureGenerator::GenerateSandTexture(ProceduralTexture& textureDef, const std::string& biome) {
        TextureLayer baseLayer;
        baseLayer.name = "sand_base";
        baseLayer.opacity = 1.0f;

        // Base color
        baseLayer.colorConfig.baseColor = Color(0.9f, 0.8f, 0.6f, 1.0f);
        baseLayer.colorConfig.secondaryColor = Color(0.8f, 0.7f, 0.5f, 1.0f);
        baseLayer.colorConfig.colorVariation = 0.1f;

        // Noise configuration
        baseLayer.noiseConfig.type = NoiseType::PERLIN;
        baseLayer.noiseConfig.frequency = 0.15f;
        baseLayer.noiseConfig.amplitude = 0.2f;
        baseLayer.noiseConfig.octaves = 4;

        textureDef.layers.push_back(baseLayer);

        // Add dune pattern
        TextureLayer duneLayer;
        duneLayer.name = "sand_dunes";
        duneLayer.opacity = 0.3f;
        duneLayer.blendMode = "soft_light";

        duneLayer.noiseConfig.type = NoiseType::RIDGED;
        duneLayer.noiseConfig.frequency = 0.05f;
        duneLayer.noiseConfig.amplitude = 0.4f;
        duneLayer.noiseConfig.octaves = 2;

        textureDef.layers.push_back(duneLayer);
    }

    void TextureGenerator::GenerateWoodTexture(ProceduralTexture& textureDef, const std::string& biome) {
        TextureLayer baseLayer;
        baseLayer.name = "wood_base";
        baseLayer.opacity = 1.0f;

        // Base color based on biome
        if (biome == "forest") {
            baseLayer.colorConfig.baseColor = Color(0.4f, 0.3f, 0.2f, 1.0f);
        } else if (biome == "jungle") {
            baseLayer.colorConfig.baseColor = Color(0.5f, 0.4f, 0.3f, 1.0f);
        } else {
            baseLayer.colorConfig.baseColor = Color(0.45f, 0.35f, 0.25f, 1.0f);
        }

        baseLayer.colorConfig.colorVariation = 0.15f;

        // Use wood pattern
        baseLayer.patternConfig.useWood = true;
        baseLayer.patternConfig.woodScale = 0.5f;

        textureDef.layers.push_back(baseLayer);

        // Add grain layer
        TextureLayer grainLayer;
        grainLayer.name = "wood_grain";
        grainLayer.opacity = 0.4f;
        grainLayer.blendMode = "overlay";

        grainLayer.noiseConfig.type = NoiseType::PERLIN;
        grainLayer.noiseConfig.frequency = 2.0f;
        grainLayer.noiseConfig.amplitude = 0.3f;

        textureDef.layers.push_back(grainLayer);
    }

    void TextureGenerator::GenerateLeafTexture(ProceduralTexture& textureDef, const std::string& biome) {
        TextureLayer baseLayer;
        baseLayer.name = "leaf_base";
        baseLayer.opacity = 1.0f;

        // Base color based on biome
        if (biome == "forest") {
            baseLayer.colorConfig.baseColor = Color(0.1f, 0.5f, 0.1f, 1.0f);
        } else if (biome == "jungle") {
            baseLayer.colorConfig.baseColor = Color(0.0f, 0.4f, 0.0f, 1.0f);
        } else if (biome == "autumn") {
            baseLayer.colorConfig.baseColor = Color(0.6f, 0.3f, 0.1f, 1.0f);
        } else {
            baseLayer.colorConfig.baseColor = Color(0.05f, 0.45f, 0.05f, 1.0f);
        }

        baseLayer.colorConfig.colorVariation = 0.3f;

        // Noise configuration
        baseLayer.noiseConfig.type = NoiseType::SIMPLEX;
        baseLayer.noiseConfig.frequency = 0.3f;
        baseLayer.noiseConfig.amplitude = 0.4f;
        baseLayer.noiseConfig.octaves = 3;

        textureDef.layers.push_back(baseLayer);

        // Add vein detail
        TextureLayer veinLayer;
        veinLayer.name = "leaf_veins";
        veinLayer.opacity = 0.2f;
        veinLayer.blendMode = "darken";

        veinLayer.noiseConfig.type = NoiseType::RIDGED;
        veinLayer.noiseConfig.frequency = 1.0f;
        veinLayer.noiseConfig.amplitude = 0.2f;

        textureDef.layers.push_back(veinLayer);
    }

    void TextureGenerator::GenerateWaterTexture(ProceduralTexture& textureDef) {
        TextureLayer baseLayer;
        baseLayer.name = "water_base";
        baseLayer.opacity = 1.0f;

        // Base color
        baseLayer.colorConfig.baseColor = Color(0.0f, 0.3f, 0.8f, 0.8f);
        baseLayer.colorConfig.secondaryColor = Color(0.0f, 0.4f, 1.0f, 0.6f);

        // Animated water
        baseLayer.animated = true;
        baseLayer.animationSpeed = 1.0f;

        // Noise configuration for waves
        baseLayer.noiseConfig.type = NoiseType::PERLIN;
        baseLayer.noiseConfig.frequency = 0.2f;
        baseLayer.noiseConfig.amplitude = 0.3f;
        baseLayer.noiseConfig.octaves = 3;

        textureDef.layers.push_back(baseLayer);

        // Add reflection layer
        TextureLayer reflectionLayer;
        reflectionLayer.name = "water_reflection";
        reflectionLayer.opacity = 0.3f;
        reflectionLayer.blendMode = "screen";

        reflectionLayer.noiseConfig.type = NoiseType::SIMPLEX;
        reflectionLayer.noiseConfig.frequency = 0.5f;
        reflectionLayer.noiseConfig.amplitude = 0.2f;

        textureDef.layers.push_back(reflectionLayer);
    }

    void TextureGenerator::GenerateDefaultBlockTexture(ProceduralTexture& textureDef) {
        TextureLayer baseLayer;
        baseLayer.name = "default_base";
        baseLayer.opacity = 1.0f;

        baseLayer.colorConfig.baseColor = Color(0.5f, 0.5f, 0.5f, 1.0f);
        baseLayer.colorConfig.colorVariation = 0.1f;

        baseLayer.noiseConfig.type = NoiseType::VALUE;
        baseLayer.noiseConfig.frequency = 0.1f;
        baseLayer.noiseConfig.amplitude = 0.2f;

        textureDef.layers.push_back(baseLayer);
    }

    void TextureGenerator::GenerateTerrainTexture(ProceduralTexture& textureDef,
                                                const std::string& biome,
                                                float height,
                                                float temperature,
                                                float moisture) {
        // Base terrain layer
        TextureLayer baseLayer;
        baseLayer.name = "terrain_base";
        baseLayer.opacity = 1.0f;

        // Color based on height and biome
        if (height > 0.7f) {
            // Mountain/snow
            baseLayer.colorConfig.baseColor = Color(0.9f, 0.9f, 0.95f, 1.0f);
        } else if (height > 0.4f) {
            // Hills/forest
            if (biome == "forest") {
                baseLayer.colorConfig.baseColor = Color(0.2f, 0.6f, 0.1f, 1.0f);
            } else {
                baseLayer.colorConfig.baseColor = Color(0.4f, 0.7f, 0.2f, 1.0f);
            }
        } else {
            // Plains/desert
            if (biome == "desert") {
                baseLayer.colorConfig.baseColor = Color(0.8f, 0.7f, 0.5f, 1.0f);
            } else {
                baseLayer.colorConfig.baseColor = Color(0.3f, 0.6f, 0.1f, 1.0f);
            }
        }

        baseLayer.noiseConfig.type = NoiseType::PERLIN;
        baseLayer.noiseConfig.frequency = 0.05f + height * 0.1f;
        baseLayer.noiseConfig.amplitude = 0.3f;
        baseLayer.noiseConfig.octaves = 4;

        textureDef.layers.push_back(baseLayer);

        // Add detail based on moisture
        if (moisture > 0.6f) {
            TextureLayer moistureLayer;
            moistureLayer.name = "moisture_detail";
            moistureLayer.opacity = moisture * 0.5f;
            moistureLayer.blendMode = "overlay";

            moistureLayer.noiseConfig.type = NoiseType::SIMPLEX;
            moistureLayer.noiseConfig.frequency = 0.2f;
            moistureLayer.noiseConfig.amplitude = 0.2f;

            textureDef.layers.push_back(moistureLayer);
        }
    }

    void TextureGenerator::GenerateBiomeTexture(ProceduralTexture& textureDef,
                                              const std::string& biome,
                                              const std::string& season,
                                              const std::string& weather) {
        // Configure based on biome
        if (biome == "plains") {
            GeneratePlainsBiomeTexture(textureDef, season, weather);
        } else if (biome == "forest") {
            GenerateForestBiomeTexture(textureDef, season, weather);
        } else if (biome == "desert") {
            GenerateDesertBiomeTexture(textureDef, season, weather);
        } else if (biome == "tundra") {
            GenerateTundraBiomeTexture(textureDef, season, weather);
        } else {
            GenerateDefaultBiomeTexture(textureDef);
        }
    }

    void TextureGenerator::GeneratePlainsBiomeTexture(ProceduralTexture& textureDef,
                                                    const std::string& season,
                                                    const std::string& weather) {
        TextureLayer grassLayer;
        grassLayer.name = "plains_grass";
        grassLayer.opacity = 1.0f;

        // Seasonal colors
        if (season == "spring") {
            grassLayer.colorConfig.baseColor = Color(0.2f, 0.7f, 0.2f, 1.0f);
        } else if (season == "summer") {
            grassLayer.colorConfig.baseColor = Color(0.15f, 0.6f, 0.15f, 1.0f);
        } else if (season == "autumn") {
            grassLayer.colorConfig.baseColor = Color(0.6f, 0.4f, 0.1f, 1.0f);
        } else {
            grassLayer.colorConfig.baseColor = Color(0.1f, 0.5f, 0.1f, 1.0f);
        }

        grassLayer.noiseConfig.type = NoiseType::PERLIN;
        grassLayer.noiseConfig.frequency = 0.1f;
        grassLayer.noiseConfig.amplitude = 0.3f;

        textureDef.layers.push_back(grassLayer);
    }

    void TextureGenerator::GenerateForestBiomeTexture(ProceduralTexture& textureDef,
                                                    const std::string& season,
                                                    const std::string& weather) {
        TextureLayer forestLayer;
        forestLayer.name = "forest_base";
        forestLayer.opacity = 1.0f;

        // Dense forest colors
        if (season == "spring") {
            forestLayer.colorConfig.baseColor = Color(0.1f, 0.6f, 0.1f, 1.0f);
        } else if (season == "autumn") {
            forestLayer.colorConfig.baseColor = Color(0.7f, 0.3f, 0.1f, 1.0f);
        } else {
            forestLayer.colorConfig.baseColor = Color(0.05f, 0.5f, 0.05f, 1.0f);
        }

        forestLayer.noiseConfig.type = NoiseType::SIMPLEX;
        forestLayer.noiseConfig.frequency = 0.15f;
        forestLayer.noiseConfig.amplitude = 0.4f;

        textureDef.layers.push_back(forestLayer);

        // Add undergrowth
        TextureLayer undergrowthLayer;
        undergrowthLayer.name = "forest_undergrowth";
        undergrowthLayer.opacity = 0.3f;
        undergrowthLayer.blendMode = "multiply";

        undergrowthLayer.noiseConfig.type = NoiseType::VORONOI;
        undergrowthLayer.noiseConfig.frequency = 0.3f;

        textureDef.layers.push_back(undergrowthLayer);
    }

    void TextureGenerator::GenerateDesertBiomeTexture(ProceduralTexture& textureDef,
                                                    const std::string& season,
                                                    const std::string& weather) {
        TextureLayer sandLayer;
        sandLayer.name = "desert_sand";
        sandLayer.opacity = 1.0f;

        sandLayer.colorConfig.baseColor = Color(0.9f, 0.8f, 0.6f, 1.0f);
        sandLayer.colorConfig.colorVariation = 0.1f;

        sandLayer.noiseConfig.type = NoiseType::PERLIN;
        sandLayer.noiseConfig.frequency = 0.08f;
        sandLayer.noiseConfig.amplitude = 0.2f;

        textureDef.layers.push_back(sandLayer);

        // Add dune patterns
        if (weather == "windy") {
            TextureLayer windLayer;
            windLayer.name = "wind_patterns";
            windLayer.opacity = 0.4f;
            windLayer.blendMode = "overlay";

            windLayer.noiseConfig.type = NoiseType::RIDGED;
            windLayer.noiseConfig.frequency = 0.03f;
            windLayer.noiseConfig.amplitude = 0.3f;

            textureDef.layers.push_back(windLayer);
        }
    }

    void TextureGenerator::GenerateTundraBiomeTexture(ProceduralTexture& textureDef,
                                                    const std::string& season,
                                                    const std::string& weather) {
        TextureLayer tundraLayer;
        tundraLayer.name = "tundra_base";
        tundraLayer.opacity = 1.0f;

        // Cold colors
        tundraLayer.colorConfig.baseColor = Color(0.7f, 0.7f, 0.8f, 1.0f);

        tundraLayer.noiseConfig.type = NoiseType::PERLIN;
        tundraLayer.noiseConfig.frequency = 0.1f;
        tundraLayer.noiseConfig.amplitude = 0.3f;

        textureDef.layers.push_back(tundraLayer);

        // Add snow/ice effects
        TextureLayer snowLayer;
        snowLayer.name = "snow_effect";
        snowLayer.opacity = 0.6f;
        snowLayer.blendMode = "screen";

        snowLayer.noiseConfig.type = NoiseType::SIMPLEX;
        snowLayer.noiseConfig.frequency = 0.2f;
        snowLayer.noiseConfig.amplitude = 0.4f;

        textureDef.layers.push_back(snowLayer);
    }

    void TextureGenerator::GenerateDefaultBiomeTexture(ProceduralTexture& textureDef) {
        TextureLayer defaultLayer;
        defaultLayer.name = "default_biome";
        defaultLayer.opacity = 1.0f;

        defaultLayer.colorConfig.baseColor = Color(0.3f, 0.6f, 0.2f, 1.0f);
        defaultLayer.noiseConfig.type = NoiseType::PERLIN;
        defaultLayer.noiseConfig.frequency = 0.1f;

        textureDef.layers.push_back(defaultLayer);
    }

    // Texture processing methods
    std::shared_ptr<TextureData> TextureGenerator::ApplyTextureEffects(std::shared_ptr<TextureData> texture,
                                                                     const std::unordered_map<std::string, float>& effects) {
        if (!texture) {
            return nullptr;
        }

        // Apply color effects
        for (const auto& effect : effects) {
            if (effect.first == "saturation") {
                ApplySaturation(texture, effect.second);
            } else if (effect.first == "brightness") {
                ApplyBrightness(texture, effect.second);
            } else if (effect.first == "contrast") {
                ApplyContrast(texture, effect.second);
            } else if (effect.first == "hue_shift") {
                ApplyHueShift(texture, effect.second);
            }
        }

        return texture;
    }

    std::shared_ptr<TextureData> TextureGenerator::BlendTextures(std::shared_ptr<TextureData> texture1,
                                                               std::shared_ptr<TextureData> texture2,
                                                               const std::string& blendMode,
                                                               float opacity) {
        if (!texture1 || !texture2) {
            return texture1 ? texture1 : texture2;
        }

        auto result = std::make_shared<TextureData>();
        result->textureId = texture1->textureId + "_blended";
        result->width = texture1->width;
        result->height = texture1->height;
        result->format = texture1->format;
        result->dataSize = texture1->dataSize;
        result->pixelData.resize(texture1->dataSize);

        // Blend pixels
        size_t pixelCount = texture1->width * texture1->height;
        for (size_t i = 0; i < pixelCount; ++i) {
            // Get pixel data (simplified RGBA blending)
            size_t pixelOffset = i * 4;
            if (pixelOffset + 3 < texture1->pixelData.size() &&
                pixelOffset + 3 < texture2->pixelData.size()) {

                Color color1(
                    texture1->pixelData[pixelOffset] / 255.0f,
                    texture1->pixelData[pixelOffset + 1] / 255.0f,
                    texture1->pixelData[pixelOffset + 2] / 255.0f,
                    texture1->pixelData[pixelOffset + 3] / 255.0f
                );

                Color color2(
                    texture2->pixelData[pixelOffset] / 255.0f,
                    texture2->pixelData[pixelOffset + 1] / 255.0f,
                    texture2->pixelData[pixelOffset + 2] / 255.0f,
                    texture2->pixelData[pixelOffset + 3] / 255.0f
                );

                Color blended = BlendColors(color1, color2, blendMode, opacity);

                result->pixelData[pixelOffset] = static_cast<uint8_t>(blended.r * 255.0f);
                result->pixelData[pixelOffset + 1] = static_cast<uint8_t>(blended.g * 255.0f);
                result->pixelData[pixelOffset + 2] = static_cast<uint8_t>(blended.b * 255.0f);
                result->pixelData[pixelOffset + 3] = static_cast<uint8_t>(blended.a * 255.0f);
            }
        }

        return result;
    }

    std::shared_ptr<TextureData> TextureGenerator::GenerateNormalMap(std::shared_ptr<TextureData> heightMap,
                                                                   float strength) {
        if (!heightMap) {
            return nullptr;
        }

        auto normalMap = std::make_shared<TextureData>();
        normalMap->textureId = heightMap->textureId + "_normal";
        normalMap->width = heightMap->width;
        normalMap->height = heightMap->height;
        normalMap->format = TextureFormat::RGBA8;
        normalMap->dataSize = heightMap->width * heightMap->height * 4;
        normalMap->pixelData.resize(normalMap->dataSize);

        // Generate normal map from height map
        for (int y = 0; y < heightMap->height; ++y) {
            for (int x = 0; x < heightMap->width; ++x) {
                // Sample neighboring heights
                float heightL = GetHeightAt(heightMap, x - 1, y);
                float heightR = GetHeightAt(heightMap, x + 1, y);
                float heightD = GetHeightAt(heightMap, x, y - 1);
                float heightU = GetHeightAt(heightMap, x, y + 1);

                // Calculate normal
                Vec3 normal(
                    (heightL - heightR) * strength,
                    (heightD - heightU) * strength,
                    1.0f
                );

                normal = normal.Normalize();

                // Convert to RGB
                size_t offset = (y * heightMap->width + x) * 4;
                normalMap->pixelData[offset] = static_cast<uint8_t>((normal.x + 1.0f) * 127.5f);
                normalMap->pixelData[offset + 1] = static_cast<uint8_t>((normal.y + 1.0f) * 127.5f);
                normalMap->pixelData[offset + 2] = static_cast<uint8_t>((normal.z + 1.0f) * 127.5f);
                normalMap->pixelData[offset + 3] = 255;
            }
        }

        return normalMap;
    }

    std::shared_ptr<TextureData> TextureGenerator::GenerateRoughnessMap(std::shared_ptr<TextureData> baseTexture,
                                                                      float roughness) {
        if (!baseTexture) {
            return nullptr;
        }

        auto roughnessMap = std::make_shared<TextureData>();
        roughnessMap->textureId = baseTexture->textureId + "_roughness";
        roughnessMap->width = baseTexture->width;
        roughnessMap->height = baseTexture->height;
        roughnessMap->format = TextureFormat::R8;
        roughnessMap->dataSize = baseTexture->width * baseTexture->height;
        roughnessMap->pixelData.resize(roughnessMap->dataSize);

        // Generate roughness based on base texture variation
        for (int y = 0; y < baseTexture->height; ++y) {
            for (int x = 0; x < baseTexture->width; ++x) {
                size_t offset = (y * baseTexture->width + x) * 4;
                if (offset + 2 < baseTexture->pixelData.size()) {
                    // Calculate luminance variation
                    float r = baseTexture->pixelData[offset] / 255.0f;
                    float g = baseTexture->pixelData[offset + 1] / 255.0f;
                    float b = baseTexture->pixelData[offset + 2] / 255.0f;
                    float luminance = 0.299f * r + 0.587f * g + 0.114f * b;

                    // Add noise-based roughness
                    float noise = GeneratePerlinNoise(x * 0.1f, y * 0.1f, 0.0f, NoiseConfig());
                    float finalRoughness = roughness * (0.5f + 0.5f * noise);

                    size_t targetOffset = y * baseTexture->width + x;
                    roughnessMap->pixelData[targetOffset] = static_cast<uint8_t>(finalRoughness * 255.0f);
                }
            }
        }

        return roughnessMap;
    }

    // Helper methods
    float TextureGenerator::GetHeightAt(std::shared_ptr<TextureData> heightMap, int x, int y) {
        // Clamp coordinates
        x = std::clamp(x, 0, heightMap->width - 1);
        y = std::clamp(y, 0, heightMap->height - 1);

        size_t offset = (y * heightMap->width + x) * 4;
        if (offset + 2 < heightMap->pixelData.size()) {
            float r = heightMap->pixelData[offset] / 255.0f;
            float g = heightMap->pixelData[offset + 1] / 255.0f;
            float b = heightMap->pixelData[offset + 2] / 255.0f;
            return 0.299f * r + 0.587f * g + 0.114f * b;
        }

        return 0.0f;
    }

    void TextureGenerator::ApplySaturation(std::shared_ptr<TextureData> texture, float saturation) {
        for (size_t i = 0; i < texture->pixelData.size(); i += 4) {
            if (i + 2 < texture->pixelData.size()) {
                float r = texture->pixelData[i] / 255.0f;
                float g = texture->pixelData[i + 1] / 255.0f;
                float b = texture->pixelData[i + 2] / 255.0f;

                // Convert to HSV, adjust saturation, convert back to RGB
                float h, s, v;
                MathUtils::RGBtoHSV(r, g, b, h, s, v);
                s = std::clamp(s * saturation, 0.0f, 1.0f);
                MathUtils::HSVtoRGB(h, s, v, r, g, b);

                texture->pixelData[i] = static_cast<uint8_t>(r * 255.0f);
                texture->pixelData[i + 1] = static_cast<uint8_t>(g * 255.0f);
                texture->pixelData[i + 2] = static_cast<uint8_t>(b * 255.0f);
            }
        }
    }

    void TextureGenerator::ApplyBrightness(std::shared_ptr<TextureData> texture, float brightness) {
        for (size_t i = 0; i < texture->pixelData.size(); i += 4) {
            if (i + 2 < texture->pixelData.size()) {
                texture->pixelData[i] = static_cast<uint8_t>(std::clamp(
                    texture->pixelData[i] * brightness, 0.0f, 255.0f));
                texture->pixelData[i + 1] = static_cast<uint8_t>(std::clamp(
                    texture->pixelData[i + 1] * brightness, 0.0f, 255.0f));
                texture->pixelData[i + 2] = static_cast<uint8_t>(std::clamp(
                    texture->pixelData[i + 2] * brightness, 0.0f, 255.0f));
            }
        }
    }

    void TextureGenerator::ApplyContrast(std::shared_ptr<TextureData> texture, float contrast) {
        float factor = (259.0f * (contrast + 255.0f)) / (255.0f * (259.0f - contrast));

        for (size_t i = 0; i < texture->pixelData.size(); i += 4) {
            if (i + 2 < texture->pixelData.size()) {
                texture->pixelData[i] = static_cast<uint8_t>(std::clamp(
                    factor * (texture->pixelData[i] - 128.0f) + 128.0f, 0.0f, 255.0f));
                texture->pixelData[i + 1] = static_cast<uint8_t>(std::clamp(
                    factor * (texture->pixelData[i + 1] - 128.0f) + 128.0f, 0.0f, 255.0f));
                texture->pixelData[i + 2] = static_cast<uint8_t>(std::clamp(
                    factor * (texture->pixelData[i + 2] - 128.0f) + 128.0f, 0.0f, 255.0f));
            }
        }
    }

    void TextureGenerator::ApplyHueShift(std::shared_ptr<TextureData> texture, float hueShift) {
        for (size_t i = 0; i < texture->pixelData.size(); i += 4) {
            if (i + 2 < texture->pixelData.size()) {
                float r = texture->pixelData[i] / 255.0f;
                float g = texture->pixelData[i + 1] / 255.0f;
                float b = texture->pixelData[i + 2] / 255.0f;

                float h, s, v;
                MathUtils::RGBtoHSV(r, g, b, h, s, v);
                h = std::fmod(h + hueShift, 360.0f);
                MathUtils::HSVtoRGB(h, s, v, r, g, b);

                texture->pixelData[i] = static_cast<uint8_t>(r * 255.0f);
                texture->pixelData[i + 1] = static_cast<uint8_t>(g * 255.0f);
                texture->pixelData[i + 2] = static_cast<uint8_t>(b * 255.0f);
            }
        }
    }

    // Noise generation methods
    float TextureGenerator::GeneratePerlinNoise(float x, float y, float z, const NoiseConfig& config) const {
        return Noise::Perlin(x * config.frequency, y * config.frequency, z * config.frequency);
    }

    float TextureGenerator::GenerateSimplexNoise(float x, float y, float z, const NoiseConfig& config) const {
        return Noise::Simplex(x * config.frequency, y * config.frequency, z * config.frequency);
    }

    float TextureGenerator::GenerateValueNoise(float x, float y, float z, const NoiseConfig& config) const {
        return Noise::Value(x * config.frequency, y * config.frequency, z * config.frequency);
    }

    float TextureGenerator::GenerateVoronoiNoise(float x, float y, float z, const NoiseConfig& config) const {
        return Noise::Voronoi(x * config.frequency, y * config.frequency, z * config.frequency);
    }

    float TextureGenerator::GenerateWorleyNoise(float x, float y, float z, const NoiseConfig& config) const {
        return Noise::Worley(x * config.frequency, y * config.frequency, z * config.frequency);
    }

    float TextureGenerator::GenerateRidgedNoise(float x, float y, float z, const NoiseConfig& config) const {
        return Noise::Ridged(x * config.frequency, y * config.frequency, z * config.frequency, config.octaves);
    }

    float TextureGenerator::GenerateBillowNoise(float x, float y, float z, const NoiseConfig& config) const {
        return Noise::Billow(x * config.frequency, y * config.frequency, z * config.frequency, config.octaves);
    }

    // Utility methods
    size_t TextureGenerator::GetBytesPerPixel(TextureFormat format) const {
        switch (format) {
            case TextureFormat::R8: return 1;
            case TextureFormat::RG8: return 2;
            case TextureFormat::RGB5_A1: return 2;
            case TextureFormat::RGBA4: return 2;
            case TextureFormat::RGB8: return 3;
            case TextureFormat::RGBA8: return 4;
            case TextureFormat::RGBA16: return 8;
            case TextureFormat::RGBA32F: return 16;
            case TextureFormat::DXT1: return 0; // Compressed
            case TextureFormat::DXT3: return 0;
            case TextureFormat::DXT5: return 0;
            case TextureFormat::ETC2: return 0;
            case TextureFormat::ASTC: return 0;
            default: return 4;
        }
    }

    Color TextureGenerator::BlendColors(const Color& color1, const Color& color2,
                                      const std::string& blendMode, float opacity) const {
        Color result = color1;

        if (blendMode == "normal") {
            result = Color::Lerp(color1, color2, opacity);
        } else if (blendMode == "multiply") {
            result = Color(color1.r * color2.r, color1.g * color2.g, color1.b * color2.b, color1.a);
        } else if (blendMode == "screen") {
            result = Color(1.0f - (1.0f - color1.r) * (1.0f - color2.r),
                          1.0f - (1.0f - color1.g) * (1.0f - color2.g),
                          1.0f - (1.0f - color1.b) * (1.0f - color2.b), color1.a);
        } else if (blendMode == "overlay") {
            result = Color(
                color1.r < 0.5f ? 2.0f * color1.r * color2.r : 1.0f - 2.0f * (1.0f - color1.r) * (1.0f - color2.r),
                color1.g < 0.5f ? 2.0f * color1.g * color2.g : 1.0f - 2.0f * (1.0f - color1.g) * (1.0f - color2.g),
                color1.b < 0.5f ? 2.0f * color1.b * color2.b : 1.0f - 2.0f * (1.0f - color1.b) * (1.0f - color2.b),
                color1.a
            );
        }

        return result;
    }

    float TextureGenerator::RandomFloat(float min, float max) const {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(m_randomEngine);
    }

    int TextureGenerator::RandomInt(int min, int max) const {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(m_randomEngine);
    }

    uint32_t TextureGenerator::RandomSeed() const {
        return RandomInt(0, UINT32_MAX);
    }

    // Stub implementations for missing methods
    std::shared_ptr<TextureData> TextureGenerator::GenerateLayerTexture(const TextureLayer& layer, int width, int height) {
        auto texture = std::make_shared<TextureData>();
        texture->width = width;
        texture->height = height;
        texture->format = TextureFormat::RGBA8;
        texture->dataSize = width * height * 4;
        texture->pixelData.resize(texture->dataSize);

        // Generate noise-based texture
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float noise = GeneratePerlinNoise(x * 0.1f, y * 0.1f, 0.0f, layer.noiseConfig);
                Color color = layer.colorConfig.baseColor;

                // Apply noise to color
                color.r = std::clamp(color.r + noise * layer.colorConfig.colorVariation, 0.0f, 1.0f);
                color.g = std::clamp(color.g + noise * layer.colorConfig.colorVariation, 0.0f, 1.0f);
                color.b = std::clamp(color.b + noise * layer.colorConfig.colorVariation, 0.0f, 1.0f);

                size_t offset = (y * width + x) * 4;
                texture->pixelData[offset] = static_cast<uint8_t>(color.r * 255.0f);
                texture->pixelData[offset + 1] = static_cast<uint8_t>(color.g * 255.0f);
                texture->pixelData[offset + 2] = static_cast<uint8_t>(color.b * 255.0f);
                texture->pixelData[offset + 3] = static_cast<uint8_t>(color.a * 255.0f);
            }
        }

        return texture;
    }

    void TextureGenerator::InitializeTexturePresets() {
        // Initialize common texture presets
        // This would load from files or generate programmatically
        Logger::Info("Initialized texture presets");
    }

    void TextureGenerator::UpdateAsyncGenerations() {
        // Process completed async generations
        // This would be more complex in a real implementation
    }

    void TextureGenerator::GenerateMipmaps(std::shared_ptr<TextureData> texture) {
        // Simple mipmap generation
        if (texture->width > 1 && texture->height > 1) {
            texture->pixelData.reserve(texture->pixelData.size() * 2); // Reserve space for mipmaps
        }
    }

    void TextureGenerator::CompressTexture(std::shared_ptr<TextureData> texture) {
        // Simple texture compression placeholder
        // Real implementation would use DXT, ETC2, etc.
        Logger::Debug("Texture compression not implemented yet");
    }

    std::string TextureGenerator::GetDebugInfo() const {
        std::stringstream ss;
        ss << "TextureGenerator Debug Info:\n";
        ss << "Textures generated: " << m_stats.totalTexturesGenerated << "\n";
        ss << "Textures in cache: " << m_stats.texturesInCache << "\n";
        ss << "Cache hits: " << m_stats.cacheHits << "\n";
        ss << "Cache misses: " << m_stats.cacheMisses << "\n";
        ss << "Average generation time: " << m_stats.averageGenerationTime << "s\n";
        ss << "Active generators: " << m_stats.activeGenerators << "\n";
        ss << "Failed generations: " << m_stats.failedGenerations << "\n";
        return ss.str();
    }

} // namespace VoxelCraft
