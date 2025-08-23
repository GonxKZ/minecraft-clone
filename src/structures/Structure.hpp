/**
 * @file Structure.hpp
 * @brief VoxelCraft Structure System - Generated Structures and Buildings
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_STRUCTURES_STRUCTURE_HPP
#define VOXELCRAFT_STRUCTURES_STRUCTURE_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <array>
#include <glm/glm.hpp>

#include "../world/World.hpp"
#include "../biome/Biome.hpp"

namespace VoxelCraft {

    class Player;
    class MobManager;

    /**
     * @enum StructureType
     * @brief Types of structures that can be generated
     */
    enum class StructureType {
        VILLAGE = 0,              ///< Village with houses and villagers
        FORTRESS,                 ///< Nether fortress
        STRONGHOLD,               ///< Stronghold with portal room
        MINESHAFT,                ///< Abandoned mineshaft
        DUNGEON,                  ///< Monster spawner dungeon
        DESERT_TEMPLE,            ///< Desert pyramid
        JUNGLE_TEMPLE,            ///< Jungle temple
        WITCH_HUT,                ///< Swamp witch hut
        OCEAN_MONUMENT,           ///< Ocean monument
        WOODLAND_MANSION,         ///< Woodland mansion
        BURIED_TREASURE,          ///< Buried treasure chest
        SHIPWRECK,                ///< Shipwreck on beach
        RUINED_PORTAL,            ///< Ruined nether portal
        BASTION_REMNANT,          ///< Bastion remnant
        END_CITY,                 ///< End city
        IGLOO,                    ///< Snow igloo
        PILLAGER_OUTPOST,         ///< Pillager outpost
        RUINS,                    ///< Ocean ruins
        FOSSIL,                   ///< Fossil dig site
        PORTAL_RUINS,             ///< Portal ruins
        TRAIL_RUINS,              ///< Trail ruins
        ANCIENT_CITY,             ///< Ancient city
        MAX_STRUCTURE_TYPES
    };

    /**
     * @enum StructureSize
     * @brief Size categories for structures
     */
    enum class StructureSize {
        SMALL = 0,                ///< Small structures (1-3 chunks)
        MEDIUM,                   ///< Medium structures (4-7 chunks)
        LARGE,                    ///< Large structures (8-15 chunks)
        HUGE                     ///< Huge structures (16+ chunks)
    };

    /**
     * @enum StructureRarity
     * @brief Rarity levels for structure generation
     */
    enum class StructureRarity {
        COMMON = 0,               ///< Common structures
        UNCOMMON,                 ///< Uncommon structures
        RARE,                    ///< Rare structures
        EPIC,                    ///< Very rare structures
        LEGENDARY                ///< Extremely rare structures
    };

    /**
     * @struct StructureDefinition
     * @brief Complete definition of a structure
     */
    struct StructureDefinition {
        StructureType type;
        std::string name;
        std::string displayName;
        std::string description;
        StructureSize size;
        StructureRarity rarity;
        int minY;                          ///< Minimum Y level for generation
        int maxY;                          ///< Maximum Y level for generation
        int spacing;                       ///< Minimum spacing between structures
        int separation;                    ///< Minimum separation from other structures
        float spawnChance;                 ///< Chance to spawn (0-1)
        bool canSpawnInWater;              ///< Whether structure can spawn in water
        bool canSpawnUnderground;          ///< Whether structure can spawn underground
        bool canSpawnAboveGround;          ///< Whether structure can spawn above ground
        std::vector<BiomeType> validBiomes; ///< Biomes where structure can spawn
        std::vector<BiomeType> invalidBiomes; ///< Biomes where structure cannot spawn
        std::vector<MobType> spawnsMobs;   ///< Mobs that spawn in this structure
        std::vector<int> chestLoot;        ///< Items that can spawn in chests
        std::unordered_map<std::string, std::any> customProperties;
    };

    /**
     * @struct StructureInstance
     * @brief Instance of a generated structure
     */
    struct StructureInstance {
        StructureType type;
        glm::ivec3 position;               ///< Bottom-center position of structure
        glm::ivec3 size;                   ///< Size of structure in blocks
        int rotation;                      ///< Rotation in degrees (0, 90, 180, 270)
        bool isGenerated;                  ///< Whether structure has been generated
        bool isValid;                      ///< Whether structure is valid for generation
        std::chrono::steady_clock::time_point generatedTime;
        std::vector<glm::ivec3> chestPositions; ///< Positions of chests in structure
        std::vector<glm::ivec3> spawnerPositions; ///< Positions of mob spawners
        std::unordered_map<std::string, std::any> customData;

        StructureInstance() : rotation(0), isGenerated(false), isValid(true),
                            generatedTime(std::chrono::steady_clock::now()) {}

        /**
         * @brief Get bounding box of structure
         * @return Vector of min and max positions
         */
        std::pair<glm::ivec3, glm::ivec3> GetBoundingBox() const {
            glm::ivec3 halfSize = size / 2;
            return {
                position - halfSize,
                position + halfSize
            };
        }

        /**
         * @brief Check if position is inside structure
         * @param pos Position to check
         * @return true if inside
         */
        bool ContainsPosition(const glm::ivec3& pos) const {
            auto [min, max] = GetBoundingBox();
            return pos.x >= min.x && pos.x <= max.x &&
                   pos.y >= min.y && pos.y <= max.y &&
                   pos.z >= min.z && pos.z <= max.z;
        }
    };

    /**
     * @struct StructureTemplate
     * @brief Template for structure generation
     */
    struct StructureTemplate {
        std::string name;
        StructureType type;
        std::vector<std::vector<std::vector<int>>> blocks; ///< 3D array of block IDs
        std::vector<std::pair<glm::ivec3, std::string>> specialBlocks; ///< Positions and types of special blocks
        int width;                       ///< Template width
        int height;                      ///< Template height
        int length;                      ///< Template length
        glm::ivec3 offset;               ///< Offset from structure origin

        /**
         * @brief Get block at relative position
         * @param x X coordinate
         * @param y Y coordinate
         * @param z Z coordinate
         * @return Block ID
         */
        int GetBlock(int x, int y, int z) const {
            if (x < 0 || x >= width || y < 0 || y >= height || z < 0 || z >= length) {
                return 0; // Air
            }
            return blocks[y][z][x];
        }

        /**
         * @brief Set block at relative position
         * @param x X coordinate
         * @param y Y coordinate
         * @param z Z coordinate
         * @param blockID Block ID
         */
        void SetBlock(int x, int y, int z, int blockID) {
            if (x >= 0 && x < width && y >= 0 && y < height && z >= 0 && z < length) {
                blocks[y][z][x] = blockID;
            }
        }
    };

    /**
     * @struct StructurePiece
     * @brief A piece of a larger structure
     */
    struct StructurePiece {
        std::string name;
        StructureTemplate templateData;
        glm::ivec3 relativePosition;      ///< Position relative to structure origin
        int rotation;                     ///< Rotation of this piece
        std::vector<std::shared_ptr<StructurePiece>> children; ///< Child pieces
        std::function<bool(const glm::ivec3&)> placementCondition; ///< Condition for placement

        /**
         * @brief Check if piece can be placed at position
         * @param worldPos World position
         * @return true if can be placed
         */
        bool CanPlaceAt(const glm::ivec3& worldPos) const {
            if (placementCondition) {
                return placementCondition(worldPos);
            }
            return true;
        }
    };

    /**
     * @struct StructureGenerationRules
     * @brief Rules for generating structures
     */
    struct StructureGenerationRules {
        bool respectTerrain;              ///< Whether to respect terrain when placing
        bool clearTrees;                  ///< Whether to clear trees in structure area
        bool clearVegetation;             ///< Whether to clear vegetation
        bool preserveWater;               ///< Whether to preserve water bodies
        bool avoidLakes;                  ///< Whether to avoid placing in lakes
        bool avoidRivers;                 ///< Whether to avoid placing near rivers
        bool avoidOceans;                 ///< Whether to avoid placing in oceans
        int minDistanceFromSpawn;         ///< Minimum distance from world spawn
        int maxDistanceFromSpawn;         ///< Maximum distance from world spawn
        int minDistanceFromPlayers;       ///< Minimum distance from players during generation
        int maxInstancesPerBiome;         ///< Maximum instances per biome
        int minInstancesPerBiome;         ///< Minimum instances per biome
        float terrainModificationChance;  ///< Chance to modify terrain
    };

    /**
     * @class Structure
     * @brief Base class for all structures
     */
    class Structure {
    public:
        /**
         * @brief Constructor
         * @param definition Structure definition
         */
        Structure(const StructureDefinition& definition);

        /**
         * @brief Destructor
         */
        virtual ~Structure() = default;

        /**
         * @brief Get structure type
         * @return Structure type
         */
        StructureType GetType() const { return m_definition.type; }

        /**
         * @brief Get structure name
         * @return Structure name
         */
        const std::string& GetName() const { return m_definition.name; }

        /**
         * @brief Get structure definition
         * @return Structure definition
         */
        const StructureDefinition& GetDefinition() const { return m_definition; }

        /**
         * @brief Check if structure can generate at position
         * @param position World position
         * @param world World reference
         * @return true if can generate
         */
        virtual bool CanGenerateAt(const glm::ivec3& position, World* world) const;

        /**
         * @brief Generate structure at position
         * @param position World position
         * @param world World reference
         * @return Generated structure instance
         */
        virtual StructureInstance GenerateAt(const glm::ivec3& position, World* world);

        /**
         * @brief Get structure size
         * @return Structure size in blocks
         */
        virtual glm::ivec3 GetSize() const = 0;

        /**
         * @brief Get structure rarity
         * @return Structure rarity
         */
        StructureRarity GetRarity() const { return m_definition.rarity; }

        /**
         * @brief Check if structure spawns in biome
         * @param biome Biome type
         * @return true if can spawn
         */
        bool CanSpawnInBiome(BiomeType biome) const;

        /**
         * @brief Get structure generation rules
         * @return Generation rules
         */
        virtual StructureGenerationRules GetGenerationRules() const;

    protected:
        StructureDefinition m_definition;
        std::vector<StructureTemplate> m_templates;
        StructureGenerationRules m_generationRules;

        /**
         * @brief Load structure templates
         */
        virtual void LoadTemplates() = 0;

        /**
         * @brief Generate structure pieces
         * @param position Base position
         * @return Vector of structure pieces
         */
        virtual std::vector<StructurePiece> GeneratePieces(const glm::ivec3& position) = 0;

        /**
         * @brief Place structure piece in world
         * @param piece Structure piece
         * @param worldPos World position
         * @param world World reference
         * @return true if placed successfully
         */
        bool PlacePiece(const StructurePiece& piece, const glm::ivec3& worldPos, World* world);

        /**
         * @brief Clear area for structure
         * @param position Center position
         * @param size Structure size
         * @param world World reference
         */
        void ClearArea(const glm::ivec3& position, const glm::ivec3& size, World* world);

        /**
         * @brief Place foundation blocks
         * @param position Position
         * @param size Size
         * @param world World reference
         */
        void PlaceFoundation(const glm::ivec3& position, const glm::ivec3& size, World* world);

        /**
         * @brief Generate structure loot
         * @param instance Structure instance
         * @param world World reference
         */
        virtual void GenerateLoot(StructureInstance& instance, World* world);

        /**
         * @brief Spawn structure mobs
         * @param instance Structure instance
         * @param world World reference
         */
        virtual void SpawnMobs(StructureInstance& instance, World* world);

        /**
         * @brief Validate structure placement
         * @param position Position
         * @param size Size
         * @param world World reference
         * @return true if valid
         */
        bool ValidatePlacement(const glm::ivec3& position, const glm::ivec3& size, World* world) const;
    };

    /**
     * @class VillageStructure
     * @brief Village structure implementation
     */
    class VillageStructure : public Structure {
    public:
        VillageStructure();

        glm::ivec3 GetSize() const override;

    protected:
        void LoadTemplates() override;
        std::vector<StructurePiece> GeneratePieces(const glm::ivec3& position) override;
        void GenerateLoot(StructureInstance& instance, World* world) override;
        void SpawnMobs(StructureInstance& instance, World* world) override;
    };

    /**
     * @class DungeonStructure
     * @brief Dungeon structure implementation
     */
    class DungeonStructure : public Structure {
    public:
        DungeonStructure();

        glm::ivec3 GetSize() const override;

    protected:
        void LoadTemplates() override;
        std::vector<StructurePiece> GeneratePieces(const glm::ivec3& position) override;
        void GenerateLoot(StructureInstance& instance, World* world) override;
        void SpawnMobs(StructureInstance& instance, World* world) override;
    };

    /**
     * @class TempleStructure
     * @brief Temple structure implementation
     */
    class TempleStructure : public Structure {
    public:
        TempleStructure();

        glm::ivec3 GetSize() const override;

    protected:
        void LoadTemplates() override;
        std::vector<StructurePiece> GeneratePieces(const glm::ivec3& position) override;
        void GenerateLoot(StructureInstance& instance, World* world) override;
        void SpawnMobs(StructureInstance& instance, World* world) override;
    };

    /**
     * @class FortressStructure
     * @brief Fortress structure implementation
     */
    class FortressStructure : public Structure {
    public:
        FortressStructure();

        glm::ivec3 GetSize() const override;

    protected:
        void LoadTemplates() override;
        std::vector<StructurePiece> GeneratePieces(const glm::ivec3& position) override;
        void GenerateLoot(StructureInstance& instance, World* world) override;
        void SpawnMobs(StructureInstance& instance, World* world) override;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_STRUCTURES_STRUCTURE_HPP
