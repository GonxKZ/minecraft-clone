/**
 * @file TextureAtlas.hpp
 * @brief VoxelCraft Texture Atlas System
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_BLOCKS_TEXTURE_ATLAS_HPP
#define VOXELCRAFT_BLOCKS_TEXTURE_ATLAS_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace VoxelCraft {

    /**
     * @struct TextureCoords
     * @brief Texture coordinates in the atlas
     */
    struct TextureCoords {
        float u1, v1;  // Top-left corner
        float u2, v2;  // Bottom-right corner

        TextureCoords() : u1(0.0f), v1(0.0f), u2(1.0f), v2(1.0f) {}
        TextureCoords(float u1, float v1, float u2, float v2)
            : u1(u1), v1(v1), u2(u2), v2(v2) {}
    };

    /**
     * @class TextureAtlas
     * @brief Manages texture atlas for block textures
     */
    class TextureAtlas {
    public:
        TextureAtlas();
        ~TextureAtlas();

        /**
         * @brief Initialize the texture atlas
         * @return true if successful
         */
        bool Initialize();

        /**
         * @brief Shutdown the texture atlas
         */
        void Shutdown();

        /**
         * @brief Add texture to atlas
         * @param name Texture name
         * @param width Texture width
         * @param height Texture height
         * @param data Texture data
         * @return true if added successfully
         */
        bool AddTexture(const std::string& name, int width, int height, const unsigned char* data);

        /**
         * @brief Get texture coordinates
         * @param name Texture name
         * @return Texture coordinates or default if not found
         */
        TextureCoords GetTextureCoords(const std::string& name) const;

        /**
         * @brief Check if texture exists
         * @param name Texture name
         * @return true if exists
         */
        bool HasTexture(const std::string& name) const;

        /**
         * @brief Get atlas texture ID
         * @return OpenGL texture ID
         */
        unsigned int GetAtlasTexture() const { return m_atlasTexture; }

        /**
         * @brief Get atlas size
         * @return Atlas size in pixels
         */
        int GetAtlasSize() const { return m_atlasSize; }

    private:
        unsigned int m_atlasTexture;                    ///< OpenGL texture ID
        int m_atlasSize;                               ///< Atlas size in pixels
        std::unordered_map<std::string, TextureCoords> m_textureCoords; ///< Texture coordinate map
        bool m_initialized;                            ///< Initialization flag
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_BLOCKS_TEXTURE_ATLAS_HPP