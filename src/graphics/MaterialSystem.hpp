/**
 * @file MaterialSystem.hpp
 * @brief VoxelCraft Advanced Material System - PBR Materials
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the MaterialSystem class that provides comprehensive material
 * management for the VoxelCraft game engine, including PBR workflows, advanced
 * material properties, texture management, and material optimization with support
 * for complex shaders and rendering techniques.
 */

#ifndef VOXELCRAFT_GRAPHICS_MATERIAL_SYSTEM_HPP
#define VOXELCRAFT_GRAPHICS_MATERIAL_SYSTEM_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <chrono>
#include <optional>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Renderer;
    class ShaderManager;
    class TextureManager;

    /**
     * @enum MaterialType
     * @brief Types of materials supported
     */
    enum class MaterialType {
        PBR,                    ///< Physically Based Rendering material
        Standard,               ///< Standard material (legacy)
        Terrain,                ///< Terrain material with layers
        Water,                  ///< Water material with waves
        Sky,                    ///< Sky material
        Particle,               ///< Particle material
        UI,                     ///< UI material
        Custom                  ///< Custom material type
    };

    /**
     * @enum PBRMaterialType
     * @brief PBR material classifications
     */
    enum class PBRMaterialType {
        Opaque,                 ///< Opaque material
        Transparent,            ///< Transparent material
        Translucent,            ///< Translucent material
        Emissive,               ///< Emissive material
        Subsurface,             ///< Subsurface scattering material
        Anisotropic,            ///< Anisotropic material
        ClearCoat,              ///< Clear coat material
        Sheen,                  ///< Sheen material
        Transmission,           ///< Transmission material
        Volume                  ///< Volumetric material
    };

    /**
     * @enum BlendMode
     * @brief Material blending modes
     */
    enum class BlendMode {
        Opaque,                 ///< No blending
        Alpha,                  ///< Alpha blending
        Additive,               ///< Additive blending
        Multiply,               ///< Multiply blending
        Screen,                 ///< Screen blending
        Overlay,                ///< Overlay blending
        Custom                  ///< Custom blend mode
    };

    /**
     * @enum CullingMode
     * @brief Face culling modes
     */
    enum class CullingMode {
        None,                   ///< No culling
        Back,                   ///< Back face culling
        Front,                  ///< Front face culling
        Both                    ///< Both faces culled
    };

    /**
     * @struct MaterialProperties
     * @brief Base material properties
     */
    struct MaterialProperties {
        std::string name;                           ///< Material name
        MaterialType type;                          ///< Material type
        PBRMaterialType pbrType;                    ///< PBR material type

        // Basic properties
        glm::vec4 baseColor;                        ///< Base color (albedo)
        float metallic;                             ///< Metallic factor (0.0 - 1.0)
        float roughness;                            ///< Roughness factor (0.0 - 1.0)
        float ao;                                   ///< Ambient occlusion (0.0 - 1.0)
        glm::vec3 emissiveColor;                    ///< Emissive color
        float emissiveIntensity;                    ///< Emissive intensity

        // Transparency
        BlendMode blendMode;                        ///< Blending mode
        float alphaCutoff;                          ///< Alpha test cutoff
        float opacity;                              ///< Opacity factor
        bool alphaTest;                             ///< Enable alpha testing

        // Geometry
        float heightScale;                          ///< Height map scale
        float displacementScale;                    ///< Displacement scale
        bool enableTessellation;                    ///< Enable tessellation

        // Advanced PBR properties
        float specular;                             ///< Specular factor
        float specularTint;                         ///< Specular tint
        float sheen;                                ///< Sheen factor
        float sheenTint;                            ///< Sheen tint
        float clearCoat;                            ///< Clear coat factor
        float clearCoatRoughness;                   ///< Clear coat roughness
        float transmission;                         ///< Transmission factor
        float ior;                                  ///< Index of refraction
        float subsurface;                           ///< Subsurface scattering
        glm::vec3 subsurfaceColor;                  ///< Subsurface color
        float subsurfaceRadius;                     ///< Subsurface radius

        // Anisotropy
        float anisotropy;                           ///< Anisotropy factor
        float anisotropyRotation;                   ///< Anisotropy rotation

        // Volume properties
        glm::vec3 absorptionColor;                  ///< Volume absorption color
        float absorptionDistance;                   ///< Absorption distance
        glm::vec3 scatteringColor;                  ///< Volume scattering color
        float scatteringDistance;                   ///< Scattering distance

        // Rendering properties
        CullingMode cullMode;                       ///< Face culling mode
        bool doubleSided;                           ///< Double sided rendering
        bool enableShadows;                         ///< Receive shadows
        bool castShadows;                           ///< Cast shadows
        int renderQueue;                            ///< Render queue order

        // Performance properties
        bool enableInstancing;                      ///< Enable GPU instancing
        bool enableLOD;                             ///< Enable level of detail
        float lodDistance;                          ///< LOD transition distance
        int maxLODLevel;                            ///< Maximum LOD level

        MaterialProperties()
            : type(MaterialType::PBR)
            , pbrType(PBRMaterialType::Opaque)
            , baseColor(1.0f, 1.0f, 1.0f, 1.0f)
            , metallic(0.0f)
            , roughness(0.5f)
            , ao(1.0f)
            , emissiveColor(0.0f, 0.0f, 0.0f)
            , emissiveIntensity(0.0f)
            , blendMode(BlendMode::Opaque)
            , alphaCutoff(0.5f)
            , opacity(1.0f)
            , alphaTest(false)
            , heightScale(0.1f)
            , displacementScale(0.0f)
            , enableTessellation(false)
            , specular(0.5f)
            , specularTint(0.0f)
            , sheen(0.0f)
            , sheenTint(0.0f)
            , clearCoat(0.0f)
            , clearCoatRoughness(0.0f)
            , transmission(0.0f)
            , ior(1.5f)
            , subsurface(0.0f)
            , subsurfaceRadius(1.0f)
            , anisotropy(0.0f)
            , anisotropyRotation(0.0f)
            , cullMode(CullingMode::Back)
            , doubleSided(false)
            , enableShadows(true)
            , castShadows(true)
            , renderQueue(1000)
            , enableInstancing(true)
            , enableLOD(false)
            , lodDistance(100.0f)
            , maxLODLevel(3)
        {}
    };

    /**
     * @struct MaterialTextures
     * @brief Material texture assignments
     */
    struct MaterialTextures {
        uint32_t baseColor;                          ///< Base color texture
        uint32_t metallicRoughness;                  ///< Metallic-roughness texture
        uint32_t normal;                             ///< Normal map texture
        uint32_t occlusion;                          ///< Ambient occlusion texture
        uint32_t emissive;                           ///< Emissive texture
        uint32_t height;                             ///< Height map texture
        uint32_t displacement;                       ///< Displacement texture

        // Advanced PBR textures
        uint32_t specular;                           ///< Specular texture
        uint32_t specularTint;                       ///< Specular tint texture
        uint32_t sheen;                              ///< Sheen texture
        uint32_t sheenTint;                          ///< Sheen tint texture
        uint32_t clearCoat;                          ///< Clear coat texture
        uint32_t clearCoatRoughness;                 ///< Clear coat roughness texture
        uint32_t transmission;                       ///< Transmission texture
        uint32_t subsurface;                         ///< Subsurface texture
        uint32_t anisotropy;                         ///< Anisotropy texture
        uint32_t anisotropyRotation;                 ///< Anisotropy rotation texture

        // Custom textures
        std::unordered_map<std::string, uint32_t> customTextures; ///< Custom texture slots

        MaterialTextures()
            : baseColor(0)
            , metallicRoughness(0)
            , normal(0)
            , occlusion(0)
            , emissive(0)
            , height(0)
            , displacement(0)
            , specular(0)
            , specularTint(0)
            , sheen(0)
            , sheenTint(0)
            , clearCoat(0)
            , clearCoatRoughness(0)
            , transmission(0)
            , subsurface(0)
            , anisotropy(0)
            , anisotropyRotation(0)
        {}
    };

    /**
     * @struct MaterialParameters
     * @brief Material shader parameters
     */
    struct MaterialParameters {
        // Standard PBR parameters
        glm::vec4 baseColorFactor;                   ///< Base color factor
        float metallicFactor;                        ///< Metallic factor
        float roughnessFactor;                       ///< Roughness factor
        float normalScale;                           ///< Normal scale
        float occlusionStrength;                     ///< AO strength
        glm::vec3 emissiveFactor;                    ///< Emissive factor

        // Advanced PBR parameters
        float specularFactor;                        ///< Specular factor
        float specularTintFactor;                    ///< Specular tint factor
        float sheenFactor;                           ///< Sheen factor
        float sheenTintFactor;                       ///< Sheen tint factor
        float clearCoatFactor;                       ///< Clear coat factor
        float clearCoatRoughnessFactor;              ///< Clear coat roughness factor
        float transmissionFactor;                    ///< Transmission factor
        float iorFactor;                             ///< Index of refraction factor
        float subsurfaceFactor;                      ///< Subsurface factor
        glm::vec3 subsurfaceColorFactor;             ///< Subsurface color factor
        float subsurfaceRadiusFactor;                ///< Subsurface radius factor

        // Anisotropy parameters
        float anisotropyFactor;                      ///< Anisotropy factor
        float anisotropyRotationFactor;              ///< Anisotropy rotation factor

        // Volume parameters
        glm::vec3 absorptionColorFactor;             ///< Absorption color factor
        float absorptionDistanceFactor;              ///< Absorption distance factor
        glm::vec3 scatteringColorFactor;             ///< Scattering color factor
        float scatteringDistanceFactor;              ///< Scattering distance factor

        // Custom parameters
        std::unordered_map<std::string, std::any> customParameters; ///< Custom shader parameters

        MaterialParameters()
            : baseColorFactor(1.0f, 1.0f, 1.0f, 1.0f)
            , metallicFactor(0.0f)
            , roughnessFactor(0.5f)
            , normalScale(1.0f)
            , occlusionStrength(1.0f)
            , emissiveFactor(0.0f, 0.0f, 0.0f)
            , specularFactor(0.5f)
            , specularTintFactor(0.0f)
            , sheenFactor(0.0f)
            , sheenTintFactor(0.0f)
            , clearCoatFactor(0.0f)
            , clearCoatRoughnessFactor(0.0f)
            , transmissionFactor(0.0f)
            , iorFactor(1.5f)
            , subsurfaceFactor(0.0f)
            , subsurfaceColorFactor(1.0f, 1.0f, 1.0f)
            , subsurfaceRadiusFactor(1.0f)
            , anisotropyFactor(0.0f)
            , anisotropyRotationFactor(0.0f)
            , absorptionColorFactor(0.0f, 0.0f, 0.0f)
            , absorptionDistanceFactor(1.0f)
            , scatteringColorFactor(1.0f, 1.0f, 1.0f)
            , scatteringDistanceFactor(1.0f)
        {}
    };

    /**
     * @struct Material
     * @brief Complete material definition
     */
    struct Material {
        uint32_t materialId;                         ///< Unique material ID
        std::string name;                            ///< Material name
        MaterialProperties properties;               ///< Material properties
        MaterialTextures textures;                   ///< Material textures
        MaterialParameters parameters;               ///< Shader parameters

        // Rendering state
        uint32_t shaderProgram;                      ///< Associated shader program
        uint32_t renderState;                        ///< OpenGL render state
        bool isDirty;                               ///< Material needs update

        // Performance data
        size_t memoryUsage;                          ///< Memory usage (bytes)
        int referenceCount;                          ///< Reference count
        double lastUsed;                             ///< Last used timestamp

        Material()
            : materialId(0)
            , shaderProgram(0)
            , renderState(0)
            , isDirty(true)
            , memoryUsage(0)
            , referenceCount(0)
            , lastUsed(0.0)
        {}

        /**
         * @brief Check if material has texture
         * @param textureType Texture type to check
         * @return true if has texture, false otherwise
         */
        bool HasTexture(const std::string& textureType) const;

        /**
         * @brief Get texture ID by type
         * @param textureType Texture type
         * @return Texture ID or 0 if not found
         */
        uint32_t GetTexture(const std::string& textureType) const;

        /**
         * @brief Set texture by type
         * @param textureType Texture type
         * @param textureId Texture ID
         */
        void SetTexture(const std::string& textureType, uint32_t textureId);

        /**
         * @brief Calculate memory usage
         * @return Memory usage in bytes
         */
        size_t CalculateMemoryUsage() const;
    };

    /**
     * @struct MaterialStats
     * @brief Material system performance statistics
     */
    struct MaterialStats {
        // Material counts
        uint32_t totalMaterials;                     ///< Total materials loaded
        uint32_t activeMaterials;                    ///< Active materials
        uint32_t cachedMaterials;                    ///< Cached materials

        // Texture usage
        uint32_t totalTextures;                      ///< Total textures used
        uint32_t textureMemoryUsage;                 ///< Texture memory usage
        uint32_t textureBindings;                    ///< Texture bindings

        // Performance metrics
        uint64_t materialUpdates;                    ///< Material updates
        uint64_t shaderSwitches;                     ///< Shader switches
        uint64_t stateChanges;                       ///< Render state changes
        double averageUpdateTime;                    ///< Average update time

        // Memory metrics
        size_t totalMemoryUsage;                     ///< Total memory usage
        size_t peakMemoryUsage;                      ///< Peak memory usage
        uint32_t materialAllocations;                ///< Material allocations
        uint32_t materialDeallocations;              ///< Material deallocations

        // Cache metrics
        uint64_t cacheHits;                          ///< Cache hits
        uint64_t cacheMisses;                        ///< Cache misses
        float cacheHitRate;                          ///< Cache hit rate
        uint32_t cacheEvictions;                     ///< Cache evictions
    };

    /**
     * @class MaterialSystem
     * @brief Advanced material management system
     *
     * The MaterialSystem class provides comprehensive material management for the
     * VoxelCraft game engine, featuring PBR workflows, advanced material properties,
     * texture management, shader integration, and performance optimization with
     * support for complex rendering techniques and material caching.
     *
     * Key Features:
     * - Physically Based Rendering (PBR) with multiple workflows
     * - Advanced material properties (subsurface, anisotropy, clear coat, etc.)
     * - Texture management with compression and optimization
     * - Material caching and hot-reloading
     * - Shader integration with automatic parameter binding
     * - Material variants and instancing support
     * - Performance monitoring and optimization
     * - Material serialization and deserialization
     * - Quality scaling for different hardware levels
     * - Real-time material editing and preview
     *
     * The material system is designed to be highly extensible and optimized,
     * supporting both cutting-edge material features and efficient rendering
     * on a wide range of hardware configurations.
     */
    class MaterialSystem {
    public:
        /**
         * @brief Constructor
         * @param renderer Renderer instance
         */
        explicit MaterialSystem(Renderer* renderer);

        /**
         * @brief Destructor
         */
        ~MaterialSystem();

        /**
         * @brief Deleted copy constructor
         */
        MaterialSystem(const MaterialSystem&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        MaterialSystem& operator=(const MaterialSystem&) = delete;

        // Material system lifecycle

        /**
         * @brief Initialize material system
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Shutdown material system
         */
        void Shutdown();

        /**
         * @brief Update material system
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        // Material creation and management

        /**
         * @brief Create new material
         * @param name Material name
         * @param type Material type
         * @return Material instance or nullptr if failed
         */
        std::shared_ptr<Material> CreateMaterial(const std::string& name,
                                                MaterialType type = MaterialType::PBR);

        /**
         * @brief Get material by name
         * @param name Material name
         * @return Material instance or nullptr if not found
         */
        std::shared_ptr<Material> GetMaterial(const std::string& name) const;

        /**
         * @brief Get material by ID
         * @param materialId Material ID
         * @return Material instance or nullptr if not found
         */
        std::shared_ptr<Material> GetMaterial(uint32_t materialId) const;

        /**
         * @brief Destroy material
         * @param material Material to destroy
         * @return true if successful, false otherwise
         */
        bool DestroyMaterial(std::shared_ptr<Material> material);

        /**
         * @brief Clone material
         * @param sourceMaterial Source material
         * @param newName New material name
         * @return Cloned material or nullptr if failed
         */
        std::shared_ptr<Material> CloneMaterial(std::shared_ptr<Material> sourceMaterial,
                                               const std::string& newName);

        // Material loading and saving

        /**
         * @brief Load material from file
         * @param filePath Material file path
         * @return Material instance or nullptr if failed
         */
        std::shared_ptr<Material> LoadMaterial(const std::string& filePath);

        /**
         * @brief Save material to file
         * @param material Material to save
         * @param filePath File path
         * @return true if successful, false otherwise
         */
        bool SaveMaterial(std::shared_ptr<Material> material, const std::string& filePath);

        /**
         * @brief Import material from external format
         * @param filePath External material file
         * @param format Import format
         * @return Material instance or nullptr if failed
         */
        std::shared_ptr<Material> ImportMaterial(const std::string& filePath,
                                                const std::string& format = "auto");

        /**
         * @brief Export material to external format
         * @param material Material to export
         * @param filePath Export file path
         * @param format Export format
         * @return true if successful, false otherwise
         */
        bool ExportMaterial(std::shared_ptr<Material> material,
                           const std::string& filePath,
                           const std::string& format = "json");

        // Material properties management

        /**
         * @brief Set material property
         * @param material Target material
         * @param property Property name
         * @param value Property value
         * @return true if successful, false otherwise
         */
        bool SetMaterialProperty(std::shared_ptr<Material> material,
                                const std::string& property,
                                const std::any& value);

        /**
         * @brief Get material property
         * @param material Source material
         * @param property Property name
         * @return Property value or nullopt if not found
         */
        std::optional<std::any> GetMaterialProperty(std::shared_ptr<Material> material,
                                                   const std::string& property) const;

        /**
         * @brief Set material texture
         * @param material Target material
         * @param textureType Texture type
         * @param textureId Texture ID
         * @return true if successful, false otherwise
         */
        bool SetMaterialTexture(std::shared_ptr<Material> material,
                               const std::string& textureType,
                               uint32_t textureId);

        /**
         * @brief Get material texture
         * @param material Source material
         * @param textureType Texture type
         * @return Texture ID or 0 if not found
         */
        uint32_t GetMaterialTexture(std::shared_ptr<Material> material,
                                   const std::string& textureType) const;

        // Material variants and instancing

        /**
         * @brief Create material variant
         * @param baseMaterial Base material
         * @param variantName Variant name
         * @param propertyOverrides Property overrides
         * @return Material variant or nullptr if failed
         */
        std::shared_ptr<Material> CreateMaterialVariant(std::shared_ptr<Material> baseMaterial,
                                                       const std::string& variantName,
                                                       const std::unordered_map<std::string, std::any>& propertyOverrides);

        /**
         * @brief Get material variants
         * @param baseMaterial Base material
         * @return Vector of material variants
         */
        std::vector<std::shared_ptr<Material>> GetMaterialVariants(std::shared_ptr<Material> baseMaterial) const;

        /**
         * @brief Enable material instancing
         * @param material Target material
         * @param enable Enable state
         * @return true if successful, false otherwise
         */
        bool EnableMaterialInstancing(std::shared_ptr<Material> material, bool enable);

        // Material optimization

        /**
         * @brief Optimize material for current hardware
         * @param material Material to optimize
         * @return true if successful, false otherwise
         */
        bool OptimizeMaterial(std::shared_ptr<Material> material);

        /**
         * @brief Optimize all materials
         * @return Number of materials optimized
         */
        size_t OptimizeAllMaterials();

        /**
         * @brief Compress material textures
         * @param material Material to compress
         * @return true if successful, false otherwise
         */
        bool CompressMaterialTextures(std::shared_ptr<Material> material);

        /**
         * @brief Generate material LOD
         * @param material Base material
         * @param lodLevel LOD level
         * @return LOD material or nullptr if failed
         */
        std::shared_ptr<Material> GenerateMaterialLOD(std::shared_ptr<Material> material,
                                                     int lodLevel);

        // Material caching

        /**
         * @brief Enable material caching
         * @param enabled Enable state
         */
        void SetMaterialCachingEnabled(bool enabled);

        /**
         * @brief Clear material cache
         * @return Number of cached materials removed
         */
        size_t ClearMaterialCache();

        /**
         * @brief Get material cache statistics
         * @return Cache statistics
         */
        std::unordered_map<std::string, size_t> GetCacheStatistics() const;

        // Shader integration

        /**
         * @brief Bind material to shader
         * @param material Material to bind
         * @param shaderProgram Shader program ID
         * @return true if successful, false otherwise
         */
        bool BindMaterialToShader(std::shared_ptr<Material> material, uint32_t shaderProgram);

        /**
         * @brief Unbind material from shader
         * @param material Material to unbind
         * @return true if successful, false otherwise
         */
        bool UnbindMaterialFromShader(std::shared_ptr<Material> material);

        /**
         * @brief Update material shader parameters
         * @param material Material to update
         * @return true if successful, false otherwise
         */
        bool UpdateMaterialShaderParameters(std::shared_ptr<Material> material);

        // Predefined materials

        /**
         * @brief Create PBR material
         * @param name Material name
         * @param baseColor Base color
         * @param metallic Metallic factor
         * @param roughness Roughness factor
         * @return PBR material or nullptr if failed
         */
        std::shared_ptr<Material> CreatePBRMaterial(const std::string& name,
                                                   const glm::vec4& baseColor = glm::vec4(1.0f),
                                                   float metallic = 0.0f,
                                                   float roughness = 0.5f);

        /**
         * @brief Create terrain material
         * @param name Material name
         * @param layerCount Number of terrain layers
         * @return Terrain material or nullptr if failed
         */
        std::shared_ptr<Material> CreateTerrainMaterial(const std::string& name, int layerCount = 4);

        /**
         * @brief Create water material
         * @param name Material name
         * @param waterColor Water color
         * @return Water material or nullptr if failed
         */
        std::shared_ptr<Material> CreateWaterMaterial(const std::string& name,
                                                     const glm::vec4& waterColor = glm::vec4(0.0f, 0.3f, 0.6f, 0.8f));

        /**
         * @brief Create particle material
         * @param name Material name
         * @param particleColor Particle color
         * @return Particle material or nullptr if failed
         */
        std::shared_ptr<Material> CreateParticleMaterial(const std::string& name,
                                                        const glm::vec4& particleColor = glm::vec4(1.0f));

        // Material debugging

        /**
         * @brief Enable material debug visualization
         * @param material Target material
         * @param debugType Debug visualization type
         * @param enabled Enable state
         */
        void SetMaterialDebugVisualization(std::shared_ptr<Material> material,
                                          const std::string& debugType,
                                          bool enabled);

        /**
         * @brief Get material debug information
         * @param material Target material
         * @return Debug information string
         */
        std::string GetMaterialDebugInfo(std::shared_ptr<Material> material) const;

        // Performance monitoring

        /**
         * @brief Get material system statistics
         * @return Performance statistics
         */
        const MaterialStats& GetStats() const { return m_stats; }

        /**
         * @brief Reset statistics
         */
        void ResetStats();

        /**
         * @brief Get performance report
         * @return Detailed performance report
         */
        std::string GetPerformanceReport() const;

        // Memory management

        /**
         * @brief Get memory usage
         * @return Memory usage in bytes
         */
        size_t GetMemoryUsage() const;

        /**
         * @brief Cleanup unused materials
         * @return Number of materials cleaned up
         */
        size_t CleanupUnusedMaterials();

        // Utility functions

        /**
         * @brief Validate material
         * @param material Material to validate
         * @return true if valid, false otherwise
         */
        bool ValidateMaterial(std::shared_ptr<Material> material) const;

        /**
         * @brief Get material names
         * @return Vector of material names
         */
        std::vector<std::string> GetMaterialNames() const;

        /**
         * @brief Get material count
         * @return Number of loaded materials
         */
        size_t GetMaterialCount() const { return m_materials.size(); }

        /**
         * @brief Check if material exists
         * @param name Material name
         * @return true if exists, false otherwise
         */
        bool MaterialExists(const std::string& name) const;

        /**
         * @brief Get status report
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

        /**
         * @brief Optimize material system
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> Optimize();

    private:
        /**
         * @brief Initialize material system
         * @return true if successful, false otherwise
         */
        bool InitializeSystem();

        /**
         * @brief Create default materials
         */
        void CreateDefaultMaterials();

        /**
         * @brief Load material from cache
         * @param name Material name
         * @return Cached material or nullptr
         */
        std::shared_ptr<Material> LoadFromCache(const std::string& name);

        /**
         * @brief Save material to cache
         * @param material Material to cache
         * @return true if cached, false otherwise
         */
        bool SaveToCache(std::shared_ptr<Material> material);

        /**
         * @brief Update material shader bindings
         * @param material Material to update
         * @return true if successful, false otherwise
         */
        bool UpdateMaterialShaderBindings(std::shared_ptr<Material> material);

        /**
         * @brief Validate material properties
         * @param material Material to validate
         * @return true if valid, false otherwise
         */
        bool ValidateMaterialProperties(std::shared_ptr<Material> material);

        /**
         * @brief Generate material hash
         * @param material Material to hash
         * @return Material hash string
         */
        std::string GenerateMaterialHash(std::shared_ptr<Material> material) const;

        /**
         * @brief Update performance statistics
         * @param deltaTime Time elapsed
         */
        void UpdateStats(double deltaTime);

        /**
         * @brief Handle material system errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        // Material system data
        Renderer* m_renderer;                         ///< Renderer instance
        ShaderManager* m_shaderManager;               ///< Shader manager
        TextureManager* m_textureManager;             ///< Texture manager

        // Material storage
        std::unordered_map<std::string, std::shared_ptr<Material>> m_materials; ///< Loaded materials
        std::unordered_map<uint32_t, std::shared_ptr<Material>> m_materialsById; ///< Materials by ID
        mutable std::shared_mutex m_materialsMutex;   ///< Materials synchronization

        // Material cache
        std::unordered_map<std::string, std::shared_ptr<Material>> m_materialCache; ///< Material cache
        mutable std::shared_mutex m_cacheMutex;       ///< Cache synchronization
        bool m_cachingEnabled;                        ///< Caching enabled

        // Material variants
        std::unordered_map<std::string, std::vector<std::shared_ptr<Material>>> m_materialVariants; ///< Material variants

        // Performance statistics
        MaterialStats m_stats;                        ///< Performance statistics

        // State tracking
        bool m_isInitialized;                          ///< System is initialized
        double m_lastUpdateTime;                       ///< Last update time
        std::string m_lastError;                       ///< Last error message

        static std::atomic<uint32_t> s_nextMaterialId; ///< Next material ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_GRAPHICS_MATERIAL_SYSTEM_HPP
