/**
 * @file TextureAtlas.hpp
 * @brief VoxelCraft Texture Atlas System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the TextureAtlas class that manages texture packing,
 * optimization, and efficient rendering of block textures in the voxel world.
 */

#ifndef VOXELCRAFT_BLOCKS_TEXTURE_ATLAS_HPP
#define VOXELCRAFT_BLOCKS_TEXTURE_ATLAS_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <array>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>

#include "../core/Config.hpp"
#include "Block.hpp"

namespace VoxelCraft {

    // Forward declarations
    class ResourceManager;

    /**
     * @enum AtlasFormat
     * @brief Texture atlas formats
     */
    enum class AtlasFormat {
        RGBA8,               ///< Standard RGBA format
        RGB8,                ///< RGB format
        RGBA16F,             ///< HDR RGBA format
        RGB16F,              ///< HDR RGB format
        RGBA32F,             ///< High precision RGBA
        CompressedRGBA,      ///< Compressed RGBA
        CompressedRGB,       ///< Compressed RGB
        Auto                 ///< Automatic format selection
    };

    /**
     * @enum AtlasPackingStrategy
     * @brief Texture packing algorithms
     */
    enum class AtlasPackingStrategy {
        Basic,               ///< Simple row-based packing
        BinaryTree,          ///< Binary tree packing (better utilization)
        Guillotine,          ///< Guillotine algorithm
        Skyline,             ///< Skyline algorithm (best for irregular textures)
        Hybrid               ///< Hybrid approach
    };

    /**
     * @enum TextureFilterMode
     * @brief Texture filtering modes
     */
    enum class TextureFilterMode {
        Nearest,             ///< Nearest neighbor filtering
        Linear,              ///< Linear filtering
        Bilinear,            ///< Bilinear filtering
        Trilinear,           ///< Trilinear filtering
        Anisotropic          ///< Anisotropic filtering
    };

    /**
     * @struct AtlasTextureInfo
     * @brief Information about a texture in the atlas
     */
    struct AtlasTextureInfo {
        std::string name;             ///< Texture name
        uint32_t atlasIndex;          ///< Atlas texture index
        float u1, v1;                ///< Top-left UV coordinates
        float u2, v2;                ///< Bottom-right UV coordinates
        int x, y;                   ///< Position in atlas
        int width, height;          ///< Size in atlas
        int originalWidth;          ///< Original texture width
        int originalHeight;         ///< Original texture height
        bool hasAlpha;              ///< Texture has alpha channel
        bool isAnimated;            ///< Texture is animated
        int animationFrames;        ///< Number of animation frames
        float animationSpeed;       ///< Animation speed
        size_t memoryUsage;         ///< Memory usage estimate
        double loadTime;            ///< Time when texture was loaded
        std::unordered_map<std::string, std::any> metadata; ///< Custom metadata
    };

    /**
     * @struct AtlasRegion
     * @brief Represents a region in the texture atlas
     */
    struct AtlasRegion {
        int x, y;                   ///< Position in atlas
        int width, height;          ///< Size of region
        bool isOccupied;            ///< Region is occupied
        uint32_t textureIndex;      ///< Texture occupying this region
        std::string textureName;    ///< Name of texture in this region

        AtlasRegion(int x = 0, int y = 0, int w = 0, int h = 0)
            : x(x), y(y), width(w), height(h), isOccupied(false), textureIndex(0) {}
    };

    /**
     * @struct TextureAtlasConfig
     * @brief Configuration for texture atlas
     */
    struct TextureAtlasConfig {
        // Basic settings
        int atlasSize;                      ///< Atlas size (width/height)
        AtlasFormat format;                 ///< Texture format
        AtlasPackingStrategy packing;       ///< Packing strategy
        int maxMipmapLevels;               ///< Maximum mipmap levels
        bool generateMipmaps;              ///< Generate mipmaps automatically

        // Quality settings
        TextureFilterMode minFilter;        ///< Minification filter
        TextureFilterMode magFilter;        ///< Magnification filter
        float maxAnisotropy;               ///< Maximum anisotropy level
        bool enableCompression;            ///< Enable texture compression
        int compressionQuality;            ///< Compression quality (0-100)

        // Performance settings
        size_t maxTextureSize;             ///< Maximum individual texture size
        size_t maxAtlasSize;               ///< Maximum atlas size in memory
        bool enableStreaming;              ///< Enable texture streaming
        int streamingPoolSize;             ///< Streaming texture pool size

        // Animation settings
        bool enableAnimation;              ///< Enable texture animation
        int maxAnimationFrames;            ///< Maximum animation frames per texture
        float animationUpdateRate;         ///< Animation update rate (fps)

        // Advanced settings
        bool enablePadding;                ///< Enable texture padding to prevent bleeding
        int paddingSize;                   ///< Padding size in pixels
        bool enableBleedingFix;            ///< Enable texture bleeding fix
        bool premultiplyAlpha;             ///< Premultiply alpha channel
    };

    /**
     * @struct TextureAtlasMetrics
     * @brief Performance metrics for texture atlas
     */
    struct TextureAtlasMetrics {
        uint64_t totalTextures;            ///< Total textures in atlas
        size_t atlasMemoryUsage;           ///< Atlas memory usage (bytes)
        float utilization;                 ///< Atlas utilization (0.0 - 1.0)
        uint64_t textureLoads;             ///< Total texture loads
        uint64_t textureUnloads;           ///< Total texture unloads
        double averageLoadTime;            ///< Average texture load time
        uint64_t cacheHits;                ///< Texture cache hits
        uint64_t cacheMisses;              ///< Texture cache misses
        float cacheHitRate;                ///< Cache hit rate (0.0 - 1.0)
        uint64_t packingAttempts;          ///< Number of packing attempts
        uint64_t packingFailures;          ///< Number of packing failures
        double averagePackingTime;         ///< Average packing time
        uint32_t activeAnimations;         ///< Number of active animations
        uint64_t animationUpdates;         ///< Total animation updates
    };

    /**
     * @class TextureAtlasNode
     * @brief Node in the binary tree for texture packing
     */
    class TextureAtlasNode {
    public:
        /**
         * @brief Constructor
         * @param x X position
         * @param y Y position
         * @param width Node width
         * @param height Node height
         */
        TextureAtlasNode(int x, int y, int width, int height);

        /**
         * @brief Insert texture into node
         * @param textureWidth Texture width
         * @param textureHeight Texture height
         * @param textureName Texture name
         * @param textureIndex Texture index
         * @return Inserted region or nullptr if failed
         */
        std::unique_ptr<AtlasRegion> Insert(int textureWidth, int textureHeight,
                                          const std::string& textureName,
                                          uint32_t textureIndex);

        /**
         * @brief Check if node can fit texture
         * @param textureWidth Texture width
         * @param textureHeight Texture height
         * @return true if can fit, false otherwise
         */
        bool CanFit(int textureWidth, int textureHeight) const;

        /**
         * @brief Get node region
         * @return Node region
         */
        AtlasRegion GetRegion() const;

        /**
         * @brief Check if node is occupied
         * @return true if occupied, false otherwise
         */
        bool IsOccupied() const { return m_region != nullptr; }

        /**
         * @brief Get utilization of this subtree
         * @return Utilization (0.0 - 1.0)
         */
        float GetUtilization() const;

    private:
        /**
         * @brief Split node into children
         */
        void Split();

        int m_x, m_y;                           ///< Node position
        int m_width, m_height;                  ///< Node size
        std::unique_ptr<AtlasRegion> m_region;  ///< Occupying region
        std::unique_ptr<TextureAtlasNode> m_left;  ///< Left child
        std::unique_ptr<TextureAtlasNode> m_right; ///< Right child
    };

    /**
     * @class TextureAtlas
     * @brief Main texture atlas management system
     *
     * The TextureAtlas efficiently packs multiple textures into a single large
     * texture to minimize texture switching and improve rendering performance.
     *
     * Key Features:
     * - Multiple packing algorithms (binary tree, skyline, guillotine)
     * - Support for texture animation
     * - Mipmap generation and management
     * - Texture compression and optimization
     * - Memory usage monitoring
     * - Hot-reloading support
     * - Padding and bleeding prevention
     *
     * The atlas automatically manages:
     * - Texture coordinates calculation
     * - Memory allocation and deallocation
     * - Texture streaming for large atlases
     * - Cache management for frequently used textures
     */
    class TextureAtlas {
    public:
        /**
         * @brief Constructor
         * @param config Atlas configuration
         */
        explicit TextureAtlas(const TextureAtlasConfig& config);

        /**
         * @brief Destructor
         */
        ~TextureAtlas();

        /**
         * @brief Deleted copy constructor
         */
        TextureAtlas(const TextureAtlas&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        TextureAtlas& operator=(const TextureAtlas&) = delete;

        // Texture management

        /**
         * @brief Add texture to atlas
         * @param name Texture name
         * @param width Texture width
         * @param height Texture height
         * @param data Texture data (RGBA)
         * @param hasAlpha Texture has alpha channel
         * @return Texture info or empty optional if failed
         */
        std::optional<AtlasTextureInfo> AddTexture(const std::string& name,
                                                 int width, int height,
                                                 const unsigned char* data,
                                                 bool hasAlpha = true);

        /**
         * @brief Remove texture from atlas
         * @param name Texture name
         * @return true if removed, false if not found
         */
        bool RemoveTexture(const std::string& name);

        /**
         * @brief Update texture data
         * @param name Texture name
         * @param data New texture data
         * @return true if updated, false if not found
         */
        bool UpdateTexture(const std::string& name, const unsigned char* data);

        /**
         * @brief Get texture information
         * @param name Texture name
         * @return Texture info or empty optional if not found
         */
        std::optional<AtlasTextureInfo> GetTextureInfo(const std::string& name) const;

        /**
         * @brief Check if texture exists in atlas
         * @param name Texture name
         * @return true if exists, false otherwise
         */
        bool HasTexture(const std::string& name) const;

        // Animation support

        /**
         * @brief Add animated texture
         * @param name Animation name
         * @param frameNames Names of frame textures
         * @param frameDuration Duration per frame (seconds)
         * @return Animation info or empty optional if failed
         */
        std::optional<AtlasTextureInfo> AddAnimatedTexture(
            const std::string& name,
            const std::vector<std::string>& frameNames,
            float frameDuration);

        /**
         * @brief Update texture animations
         * @param deltaTime Time elapsed since last update
         * @return Number of animations updated
         */
        size_t UpdateAnimations(double deltaTime);

        /**
         * @brief Get current animation frame
         * @param name Animation name
         * @return Current frame texture info
         */
        std::optional<AtlasTextureInfo> GetAnimationFrame(const std::string& name) const;

        // Atlas operations

        /**
         * @brief Rebuild entire atlas
         * @return true if successful, false otherwise
         */
        bool RebuildAtlas();

        /**
         * @brief Optimize atlas layout
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> OptimizeAtlas();

        /**
         * @brief Defragment atlas
         * @return Number of textures relocated
         */
        size_t DefragmentAtlas();

        /**
         * @brief Generate mipmaps for atlas
         * @return true if successful, false otherwise
         */
        bool GenerateMipmaps();

        // Data access

        /**
         * @brief Get atlas texture data
         * @param mipmapLevel Mipmap level (0 = full size)
         * @return Texture data pointer or nullptr if not available
         */
        const unsigned char* GetAtlasData(int mipmapLevel = 0) const;

        /**
         * @brief Get atlas size
         * @return Atlas size in pixels
         */
        int GetAtlasSize() const { return m_config.atlasSize; }

        /**
         * @brief Get number of mipmaps
         * @return Number of mipmap levels
         */
        int GetMipmapLevels() const { return m_mipmapLevels; }

        // Configuration

        /**
         * @brief Get atlas configuration
         * @return Current configuration
         */
        const TextureAtlasConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set atlas configuration
         * @param config New configuration
         */
        void SetConfig(const TextureAtlasConfig& config);

        /**
         * @brief Get atlas metrics
         * @return Performance metrics
         */
        const TextureAtlasMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

        // Utility functions

        /**
         * @brief Convert texture coordinates to atlas coordinates
         * @param textureInfo Texture information
         * @param u Texture U coordinate (0.0 - 1.0)
         * @param v Texture V coordinate (0.0 - 1.0)
         * @return Atlas UV coordinates
         */
        std::pair<float, float> TextureToAtlasCoords(const AtlasTextureInfo& textureInfo,
                                                   float u, float v) const;

        /**
         * @brief Convert atlas coordinates to texture coordinates
         * @param textureInfo Texture information
         * @param u Atlas U coordinate (0.0 - 1.0)
         * @param v Atlas V coordinate (0.0 - 1.0)
         * @return Texture UV coordinates
         */
        std::pair<float, float> AtlasToTextureCoords(const AtlasTextureInfo& textureInfo,
                                                   float u, float v) const;

        /**
         * @brief Get texture memory usage
         * @param textureInfo Texture information
         * @return Memory usage in bytes
         */
        size_t GetTextureMemoryUsage(const AtlasTextureInfo& textureInfo) const;

        /**
         * @brief Get total atlas memory usage
         * @return Memory usage in bytes
         */
        size_t GetTotalMemoryUsage() const;

        // Debug and monitoring

        /**
         * @brief Get atlas utilization map
         * @return 2D vector representing atlas occupancy
         */
        std::vector<std::vector<bool>> GetUtilizationMap() const;

        /**
         * @brief Get packing efficiency report
         * @return Detailed packing report
         */
        std::string GetPackingReport() const;

        /**
         * @brief Export atlas layout to image
         * @param filename Output filename
         * @return true if successful, false otherwise
         */
        bool ExportAtlasLayout(const std::string& filename) const;

    private:
        /**
         * @brief Initialize atlas
         */
        void InitializeAtlas();

        /**
         * @brief Create root packing node
         */
        void CreateRootNode();

        /**
         * @brief Insert texture using binary tree packing
         * @param textureWidth Texture width
         * @param textureHeight Texture height
         * @param textureName Texture name
         * @param textureIndex Texture index
         * @return Inserted region or nullptr if failed
         */
        std::unique_ptr<AtlasRegion> InsertTextureBinaryTree(int textureWidth, int textureHeight,
                                                           const std::string& textureName,
                                                           uint32_t textureIndex);

        /**
         * @brief Insert texture using skyline algorithm
         * @param textureWidth Texture width
         * @param textureHeight Texture height
         * @param textureName Texture name
         * @param textureIndex Texture index
         * @return Inserted region or nullptr if failed
         */
        std::unique_ptr<AtlasRegion> InsertTextureSkyline(int textureWidth, int textureHeight,
                                                        const std::string& textureName,
                                                        uint32_t textureIndex);

        /**
         * @brief Copy texture data to atlas
         * @param region Atlas region
         * @param textureData Source texture data
         * @param textureWidth Source texture width
         * @param textureHeight Source texture height
         */
        void CopyTextureToAtlas(const AtlasRegion& region,
                              const unsigned char* textureData,
                              int textureWidth, int textureHeight);

        /**
         * @brief Remove texture from atlas data
         * @param region Atlas region to clear
         */
        void ClearAtlasRegion(const AtlasRegion& region);

        /**
         * @brief Add padding to texture region
         * @param region Atlas region
         */
        void AddTexturePadding(const AtlasRegion& region);

        /**
         * @brief Update texture coordinates
         * @param textureInfo Texture information
         * @param region Atlas region
         */
        void UpdateTextureCoordinates(AtlasTextureInfo& textureInfo, const AtlasRegion& region);

        /**
         * @brief Update atlas metrics
         * @param operation Operation type
         */
        void UpdateMetrics(const std::string& operation);

        /**
         * @brief Allocate atlas data
         */
        void AllocateAtlasData();

        /**
         * @brief Deallocate atlas data
         */
        void DeallocateAtlasData();

        /**
         * @brief Generate mipmap level
         * @param sourceLevel Source mipmap level
         * @param targetLevel Target mipmap level
         */
        void GenerateMipmapLevel(int sourceLevel, int targetLevel);

        // Configuration
        TextureAtlasConfig m_config;

        // Atlas data
        int m_atlasSize;                          ///< Atlas size in pixels
        int m_mipmapLevels;                       ///< Number of mipmap levels
        std::vector<std::unique_ptr<unsigned char[]>> m_atlasData; ///< Atlas texture data
        std::unique_ptr<TextureAtlasNode> m_rootNode; ///< Root packing node

        // Texture management
        std::unordered_map<std::string, AtlasTextureInfo> m_textures; ///< Texture information
        std::unordered_map<uint32_t, std::string> m_indexToName; ///< Index to name mapping
        std::atomic<uint32_t> m_nextTextureIndex; ///< Next texture index

        // Animation support
        struct AnimationData {
            std::vector<std::string> frameNames;  ///< Animation frame names
            float frameDuration;                  ///< Duration per frame
            float currentTime;                    ///< Current animation time
            int currentFrame;                     ///< Current frame index
            bool isLooping;                       ///< Animation loops
        };
        std::unordered_map<std::string, AnimationData> m_animations; ///< Animation data

        // Metrics and monitoring
        TextureAtlasMetrics m_metrics;
        mutable std::shared_mutex m_metricsMutex;
        mutable std::shared_mutex m_atlasMutex;
        mutable std::shared_mutex m_textureMutex;

        // Constants
        static constexpr int MIN_ATLAS_SIZE = 64;    ///< Minimum atlas size
        static constexpr int MAX_ATLAS_SIZE = 8192;  ///< Maximum atlas size
        static constexpr int DEFAULT_PADDING = 1;    ///< Default padding size
    };

    /**
     * @class TextureAtlasFactory
     * @brief Factory for creating texture atlases with different configurations
     */
    class TextureAtlasFactory {
    public:
        /**
         * @brief Create default texture atlas
         * @return Default texture atlas
         */
        static std::unique_ptr<TextureAtlas> CreateDefaultAtlas();

        /**
         * @brief Create high-quality texture atlas
         * @return High-quality texture atlas
         */
        static std::unique_ptr<TextureAtlas> CreateHighQualityAtlas();

        /**
         * @brief Create low-memory texture atlas
         * @return Low-memory texture atlas
         */
        static std::unique_ptr<TextureAtlas> CreateLowMemoryAtlas();

        /**
         * @brief Create custom texture atlas
         * @param config Custom configuration
         * @return Custom texture atlas
         */
        static std::unique_ptr<TextureAtlas> CreateCustomAtlas(const TextureAtlasConfig& config);

        /**
         * @brief Get default configuration
         * @return Default configuration
         */
        static TextureAtlasConfig GetDefaultConfig();

        /**
         * @brief Get high-quality configuration
         * @return High-quality configuration
         */
        static TextureAtlasConfig GetHighQualityConfig();

        /**
         * @brief Get low-memory configuration
         * @return Low-memory configuration
         */
        static TextureAtlasConfig GetLowMemoryConfig();
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_BLOCKS_TEXTURE_ATLAS_HPP
