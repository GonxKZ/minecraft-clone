/**
 * @file Crop.hpp
 * @brief VoxelCraft Farming System - Crop Definitions and Management
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_FARMING_CROP_HPP
#define VOXELCRAFT_FARMING_CROP_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <chrono>
#include <glm/glm.hpp>

namespace VoxelCraft {

    class World;
    struct Vec3;

    /**
     * @enum CropType
     * @brief Types of crops available in the farming system
     */
    enum class CropType {
        WHEAT = 0,          ///< Wheat crop
        CARROT,             ///< Carrot crop
        POTATO,             ///< Potato crop
        BEETROOT,           ///< Beetroot crop
        MELON,              ///< Melon crop
        PUMPKIN,            ///< Pumpkin crop
        SUGAR_CANE,         ///< Sugar cane crop
        COCOA,              ///< Cocoa beans
        NETHER_WART,        ///< Nether wart
        CHORUS_FRUIT,       ///< Chorus fruit
        KELP,               ///< Kelp plant
        BAMBOO,             ///< Bamboo plant
        SWEET_BERRY,        ///< Sweet berry bush
        APPLE,              ///< Apple tree
        OAK_SAPLING,        ///< Oak tree sapling
        SPRUCE_SAPLING,     ///< Spruce tree sapling
        BIRCH_SAPLING,      ///< Birch tree sapling
        JUNGLE_SAPLING,     ///< Jungle tree sapling
        ACACIA_SAPLING,     ///< Acacia tree sapling
        DARK_OAK_SAPLING,   ///< Dark oak tree sapling
        MUSHROOM,           ///< Mushroom crop
        FLOWER,             ///< Flower crop
        GRASS,              ///< Grass crop
        FERN,               ///< Fern crop
        CUSTOM              ///< Custom crop type
    };

    /**
     * @enum GrowthStage
     * @brief Growth stages of crops
     */
    enum class GrowthStage {
        SEEDLING = 0,       ///< Just planted
        SPROUTING,          ///< Starting to grow
        GROWING,            ///< Actively growing
        MATURE,             ///< Fully grown, ready to harvest
        WITHERED,           ///< Dead/dried up
        HARVESTED           ///< Recently harvested
    };

    /**
     * @enum SoilQuality
     * @brief Quality of soil for crop growth
     */
    enum class SoilQuality {
        STERILE = 0,        ///< Cannot grow crops
        POOR,               ///< Slow growth
        AVERAGE,            ///< Normal growth
        GOOD,               ///< Fast growth
        EXCELLENT,          ///< Very fast growth with bonuses
        PERFECT             ///< Optimal growth conditions
    };

    /**
     * @struct CropGrowthRequirements
     * @brief Environmental requirements for crop growth
     */
    struct CropGrowthRequirements {
        float minTemperature = 0.0f;      ///< Minimum temperature
        float maxTemperature = 30.0f;     ///< Maximum temperature
        float optimalTemperature = 20.0f; ///< Optimal temperature
        float minHumidity = 0.0f;         ///< Minimum humidity (0-1)
        float maxHumidity = 1.0f;         ///< Maximum humidity (0-1)
        float optimalHumidity = 0.6f;     ///< Optimal humidity
        int minLightLevel = 8;            ///< Minimum light level
        int maxLightLevel = 15;           ///< Maximum light level
        int optimalLightLevel = 12;       ///< Optimal light level
        SoilQuality minSoilQuality = SoilQuality::POOR;
        bool needsWater = false;          ///< Whether crop needs water nearby
        bool needsSunlight = true;        ///< Whether crop needs direct sunlight
        bool canGrowUnderground = false;  ///< Whether crop can grow underground
        float growthRateMultiplier = 1.0f; ///< Growth rate multiplier
    };

    /**
     * @struct CropProperties
     * @brief Properties of a crop type
     */
    struct CropProperties {
        CropType type;
        std::string name;
        std::string displayName;
        int maxGrowthStages = 7;          ///< Maximum growth stages
        float baseGrowthTime = 30.0f;     ///< Base growth time in minutes
        float growthTimeVariation = 0.2f; ///< Growth time variation (±20%)
        int minYield = 1;                 ///< Minimum harvest yield
        int maxYield = 3;                 ///< Maximum harvest yield
        int seedItemID = 0;               ///< Seed item ID
        int harvestItemID = 0;            ///< Harvest item ID
        bool regrowsAfterHarvest = false; ///< Whether crop regrows after harvest
        int regrowthTime = 0;             ///< Time to regrow in minutes
        float diseaseResistance = 1.0f;   ///< Resistance to diseases (0-1)
        float pestResistance = 1.0f;      ///< Resistance to pests (0-1)
        bool canBeGrafted = false;        ///< Whether crop can be grafted
        std::vector<int> compatibleSoils; ///< Compatible soil block IDs
        CropGrowthRequirements requirements;
        std::unordered_map<std::string, std::any> customProperties;
    };

    /**
     * @struct CropInstance
     * @brief Instance of a crop in the world
     */
    struct CropInstance {
        glm::ivec3 position;              ///< World position
        CropType type;                    ///< Crop type
        GrowthStage stage;                ///< Current growth stage
        float growthProgress = 0.0f;      ///< Growth progress (0-1)
        float health = 1.0f;              ///< Crop health (0-1)
        std::chrono::steady_clock::time_point plantTime;
        std::chrono::steady_clock::time_point lastGrowthUpdate;
        std::chrono::steady_clock::time_point lastWateredTime;
        int waterLevel = 0;               ///< Water level (0-3)
        float fertilizerLevel = 0.0f;     ///< Fertilizer level (0-1)
        SoilQuality soilQuality = SoilQuality::AVERAGE;
        bool isDiseased = false;          ///< Whether crop has disease
        bool hasPests = false;            ///< Whether crop has pests
        bool isGrafted = false;           ///< Whether crop is grafted
        std::string graftType;            ///< Type of graft applied
        std::unordered_map<std::string, float> growthModifiers;

        CropInstance() : plantTime(std::chrono::steady_clock::now()),
                        lastGrowthUpdate(std::chrono::steady_clock::now()),
                        lastWateredTime(std::chrono::steady_clock::now()) {}

        /**
         * @brief Get age of crop in minutes
         * @return Age in minutes
         */
        float GetAge() const {
            auto now = std::chrono::steady_clock::now();
            return std::chrono::duration<float>(now - plantTime).count() / 60.0f;
        }

        /**
         * @brief Check if crop is ready for harvest
         * @return true if ready
         */
        bool IsReadyForHarvest() const {
            return stage == GrowthStage::MATURE && health > 0.5f && !isDiseased;
        }

        /**
         * @brief Get current growth percentage
         * @return Growth percentage (0-100)
         */
        float GetGrowthPercentage() const {
            if (stage == GrowthStage::MATURE) return 100.0f;
            return (static_cast<int>(stage) + growthProgress) /
                   static_cast<float>(static_cast<int>(GrowthStage::MATURE)) * 100.0f;
        }
    };

    /**
     * @class Crop
     * @brief Base class for all crop types
     */
    class Crop {
    public:
        /**
         * @brief Constructor
         * @param properties Crop properties
         */
        Crop(const CropProperties& properties);

        /**
         * @brief Destructor
         */
        virtual ~Crop() = default;

        /**
         * @brief Get crop type
         * @return Crop type
         */
        CropType GetType() const { return m_properties.type; }

        /**
         * @brief Get crop name
         * @return Crop name
         */
        const std::string& GetName() const { return m_properties.name; }

        /**
         * @brief Get crop properties
         * @return Crop properties
         */
        const CropProperties& GetProperties() const { return m_properties; }

        /**
         * @brief Update crop growth
         * @param instance Crop instance to update
         * @param deltaTime Time since last update
         * @param world World reference
         */
        virtual void UpdateGrowth(CropInstance& instance, float deltaTime, World* world);

        /**
         * @brief Harvest crop
         * @param instance Crop instance
         * @return Vector of harvested items (itemID, count pairs)
         */
        virtual std::vector<std::pair<int, int>> Harvest(CropInstance& instance);

        /**
         * @brief Check if crop can be planted at position
         * @param position Position to check
         * @param world World reference
         * @return true if can be planted
         */
        virtual bool CanPlantAt(const glm::ivec3& position, World* world) const;

        /**
         * @brief Get growth stage for visual representation
         * @param instance Crop instance
         * @return Growth stage for rendering
         */
        virtual int GetVisualGrowthStage(const CropInstance& instance) const;

        /**
         * @brief Apply fertilizer to crop
         * @param instance Crop instance
         * @param fertilizerType Type of fertilizer
         * @param amount Amount of fertilizer
         */
        virtual void ApplyFertilizer(CropInstance& instance, int fertilizerType, float amount);

        /**
         * @brief Water crop
         * @param instance Crop instance
         * @param amount Amount of water
         */
        virtual void WaterCrop(CropInstance& instance, float amount);

        /**
         * @brief Check for diseases
         * @param instance Crop instance
         * @param world World reference
         */
        virtual void CheckForDiseases(CropInstance& instance, World* world);

        /**
         * @brief Treat disease
         * @param instance Crop instance
         * @param treatmentType Type of treatment
         */
        virtual void TreatDisease(CropInstance& instance, int treatmentType);

    protected:
        CropProperties m_properties;

        /**
         * @brief Calculate growth rate for instance
         * @param instance Crop instance
         * @param world World reference
         * @return Growth rate multiplier
         */
        virtual float CalculateGrowthRate(const CropInstance& instance, World* world) const;

        /**
         * @brief Advance growth stage
         * @param instance Crop instance
         */
        virtual void AdvanceGrowthStage(CropInstance& instance);

        /**
         * @brief Handle environmental effects on growth
         * @param instance Crop instance
         * @param world World reference
         */
        virtual void HandleEnvironmentalEffects(CropInstance& instance, World* world);
    };

    /**
     * @class WheatCrop
     * @brief Wheat crop implementation
     */
    class WheatCrop : public Crop {
    public:
        WheatCrop();

        void UpdateGrowth(CropInstance& instance, float deltaTime, World* world) override;
        std::vector<std::pair<int, int>> Harvest(CropInstance& instance) override;
        bool CanPlantAt(const glm::ivec3& position, World* world) const override;
    };

    /**
     * @class CarrotCrop
     * @brief Carrot crop implementation
     */
    class CarrotCrop : public Crop {
    public:
        CarrotCrop();

        void UpdateGrowth(CropInstance& instance, float deltaTime, World* world) override;
        std::vector<std::pair<int, int>> Harvest(CropInstance& instance) override;
        bool CanPlantAt(const glm::ivec3& position, World* world) const override;
    };

    /**
     * @class TreeCrop
     * @brief Tree crop base class
     */
    class TreeCrop : public Crop {
    public:
        TreeCrop(const CropProperties& properties);

        void UpdateGrowth(CropInstance& instance, float deltaTime, World* world) override;
        std::vector<std::pair<int, int>> Harvest(CropInstance& instance) override;
        bool CanPlantAt(const glm::ivec3& position, World* world) const override;
        int GetVisualGrowthStage(const CropInstance& instance) const override;

    protected:
        int m_maxTreeHeight = 6;          ///< Maximum tree height
        float m_fruitGrowthTime = 10.0f;  ///< Time for fruit to grow in minutes
    };

    /**
     * @class AppleTreeCrop
     * @brief Apple tree crop implementation
     */
    class AppleTreeCrop : public TreeCrop {
    public:
        AppleTreeCrop();

        std::vector<std::pair<int, int>> Harvest(CropInstance& instance) override;
    };

    /**
     * @class CropManager
     * @brief Simple manager for crop planting and harvesting
     */
    class CropManager {
    public:
        /**
         * @brief Get singleton instance
         * @return CropManager instance
         */
        static CropManager& GetInstance();

        /**
         * @brief Plant crop at position
         * @param cropType Crop type to plant
         * @param position Position to plant
         * @param world World reference
         * @return true if planted successfully
         */
        bool PlantCrop(CropType cropType, const glm::ivec3& position, World* world);

        /**
         * @brief Harvest crop at position
         * @param position Position to harvest
         * @param world World reference
         * @return Vector of harvested items (itemID, count)
         */
        std::vector<std::pair<int, int>> HarvestCrop(const glm::ivec3& position, World* world);

        /**
         * @brief Check if position has a crop
         * @param position Position to check
         * @return true if has crop
         */
        bool HasCropAt(const glm::ivec3& position) const;

    private:
        CropManager() = default;
        std::unordered_map<glm::ivec3, CropInstance> m_cropInstances;
    };



} // namespace VoxelCraft

#endif // VOXELCRAFT_FARMING_CROP_HPP
