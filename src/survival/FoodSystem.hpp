/**
 * @file FoodSystem.hpp
 * @brief VoxelCraft Food and Nutrition System
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_SURVIVAL_FOOD_SYSTEM_HPP
#define VOXELCRAFT_SURVIVAL_FOOD_SYSTEM_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <atomic>
#include <mutex>
#include <functional>

#include "../math/Vec3.hpp"
#include "../entities/Entity.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Player;
    class World;
    struct FoodConfig;
    struct FoodItem;
    struct NutritionInfo;
    struct FoodEffect;
    struct CookingRecipe;
    struct FoodStats;

    /**
     * @enum FoodType
     * @brief Types of food items
     */
    enum class FoodType {
        RAW_MEAT,          ///< Raw animal meat
        COOKED_MEAT,       ///< Cooked animal meat
        FRUIT,             ///< Fruits and berries
        VEGETABLE,         ///< Vegetables
        GRAIN,             ///< Grains and seeds
        BREAD,             ///< Baked bread products
        DAIRY,             ///< Milk and cheese products
        FISH,              ///< Raw fish
        COOKED_FISH,       ///< Cooked fish
        SWEET,             ///< Sweets and desserts
        SPICE,             ///< Spices and herbs
        BEVERAGE,          ///< Drinks and potions
        MEDICINE,          ///< Medicinal food/drinks
        POISONOUS,         ///< Poisonous food
        MAGICAL,           ///< Magical food
        PRESERVED,         ///< Preserved/canned food
        CUSTOM             ///< Custom food type
    };

    /**
     * @enum FoodQuality
     * @brief Quality levels of food
     */
    enum class FoodQuality {
        SPOILED,           ///< Spoiled/rotten food
        LOW,               ///< Low quality food
        NORMAL,            ///< Normal quality food
        HIGH,              ///< High quality food
        PREMIUM,           ///< Premium quality food
        PERFECT            ///< Perfect quality food
    };

    /**
     * @enum CookingMethod
     * @brief Methods of cooking food
     */
    enum class CookingMethod {
        NONE,              ///< No cooking
        BOILING,           ///< Boiled food
        BAKING,            ///< Baked food
        ROASTING,          ///< Roasted food
        GRILLING,          ///< Grilled food
        FRYING,            ///< Fried food
        SMOKING,           ///< Smoked food
        DRYING,            ///< Dried food
        FERMENTING,        ///< Fermented food
        BREWING,           ///< Brewed drinks
        DISTILLING,        ///< Distilled drinks
        CUSTOM             ///< Custom cooking method
    };

    /**
     * @enum PreservationMethod
     * @brief Methods of preserving food
     */
    enum class PreservationMethod {
        NONE,              ///< No preservation
        SALTING,           ///< Salted food
        SMOKING,           ///< Smoked food
        DRYING,            ///< Dried food
        CANNING,           ///< Canned food
        FREEZING,          ///< Frozen food
        PICKLING,          ///< Pickled food
        FERMENTING,        ///< Fermented food
        CUSTOM             ///< Custom preservation
    };

    /**
     * @struct NutritionInfo
     * @brief Nutritional information for food
     */
    struct NutritionInfo {
        float hungerRestore = 0.0f;      ///< Hunger points restored (0-20)
        float thirstRestore = 0.0f;      ///< Thirst points restored (0-20)
        float healthRestore = 0.0f;      ///< Health points restored
        float staminaRestore = 0.0f;     ///< Stamina points restored
        float sanityRestore = 0.0f;      ///< Sanity points restored

        // Nutritional composition
        float carbohydrates = 0.0f;      ///< Carbohydrates (g)
        float proteins = 0.0f;          ///< Proteins (g)
        float fats = 0.0f;              ///< Fats (g)
        float vitamins = 0.0f;          ///< Vitamins and minerals
        float fiber = 0.0f;             ///< Dietary fiber

        // Special properties
        float toxicity = 0.0f;          ///< Toxicity level
        float radiation = 0.0f;         ///< Radiation level
        float diseaseRisk = 0.0f;       ///< Risk of disease
        float addiction = 0.0f;         ///< Addiction potential

        // Timing
        float digestionTime = 0.0f;      ///< Time to digest (seconds)
        float effectDuration = 0.0f;     ///< Effect duration (seconds)
    };

    /**
     * @struct FoodEffect
     * @brief Effects that food can have
     */
    struct FoodEffect {
        std::string name;                ///< Effect name
        std::string description;         ///< Effect description
        float strength = 0.0f;          ///< Effect strength
        float duration = 0.0f;          ///< Effect duration (seconds)
        bool isBeneficial = true;       ///< Whether effect is beneficial
        std::string iconPath;           ///< UI icon path

        // Effect modifiers
        float healthModifier = 0.0f;     ///< Health modifier per second
        float hungerModifier = 0.0f;     ///< Hunger modifier per second
        float thirstModifier = 0.0f;     ///< Thirst modifier per second
        float staminaModifier = 0.0f;    ///< Stamina modifier per second
        float sanityModifier = 0.0f;     ///< Sanity modifier per second
        float temperatureModifier = 0.0f; ///< Temperature modifier per second

        // Status effects
        std::unordered_map<std::string, float> statusEffects;
    };

    /**
     * @struct FoodItem
     * @brief Complete food item definition
     */
    struct FoodItem {
        std::string id;                  ///< Unique food identifier
        std::string name;                ///< Display name
        std::string description;         ///< Item description
        FoodType type = FoodType::CUSTOM; ///< Food type
        FoodQuality quality = FoodQuality::NORMAL; ///< Food quality

        // Physical properties
        float weight = 0.0f;             ///< Item weight (kg)
        float volume = 0.0f;             ///< Item volume (liters)
        Vec3 dimensions = Vec3(1.0f);    ///< Item dimensions

        // Preparation state
        CookingMethod cookingMethod = CookingMethod::NONE;
        PreservationMethod preservationMethod = PreservationMethod::NONE;
        float cookingLevel = 0.0f;       ///< How well cooked (0-1)
        float freshness = 1.0f;          ///< Freshness level (0-1)

        // Nutritional info
        NutritionInfo nutrition;

        // Effects
        std::vector<FoodEffect> effects;

        // Visual/audio
        std::string modelPath;           ///< 3D model path
        std::string texturePath;         ///< Texture path
        std::string iconPath;            ///< UI icon path
        std::string eatSound;            ///< Sound when eating

        // Game properties
        int maxStackSize = 64;           ///< Maximum stack size
        float spoilTime = -1.0f;         ///< Time to spoil (-1 = never spoils)
        float cookTime = 0.0f;           ///< Time to cook (seconds)
        float eatTime = 1.0f;            ///< Time to eat (seconds)

        // Crafting requirements
        std::unordered_map<std::string, int> craftingRequirements;
        std::string craftingStation;     ///< Required crafting station

        // Special properties
        bool isCooked = false;           ///< Whether food is cooked
        bool isPoisonous = false;        ///< Whether food is poisonous
        bool isMagical = false;          ///< Whether food has magical properties
        bool requiresCooking = false;    ///< Whether food needs cooking
        bool canBeCooked = true;         ///< Whether food can be cooked

        // Temperature requirements
        float optimalCookTemp = 0.0f;    ///< Optimal cooking temperature
        float burnTemp = 0.0f;           ///< Temperature at which food burns

        // Value and rarity
        float value = 1.0f;              ///< Base value
        int rarity = 1;                  ///< Rarity level (1-10)

        // Tags for categorization
        std::vector<std::string> tags;
    };

    /**
     * @struct CookingRecipe
     * @brief Recipe for cooking food
     */
    struct CookingRecipe {
        std::string id;                  ///< Recipe identifier
        std::string name;                ///< Recipe name
        std::string description;         ///< Recipe description

        // Input ingredients
        std::vector<std::pair<std::string, int>> ingredients;

        // Output food
        std::string outputFoodId;
        int outputQuantity = 1;

        // Cooking requirements
        CookingMethod requiredMethod = CookingMethod::NONE;
        float requiredCookTime = 0.0f;   ///< Time to cook (seconds)
        float requiredTemperature = 0.0f; ///< Required temperature
        std::string requiredStation;     ///< Required cooking station

        // Skill requirements
        int requiredCookingSkill = 0;    ///< Required cooking skill level
        std::vector<std::string> requiredTools; ///< Required tools

        // Results
        float successRate = 1.0f;        ///< Base success rate
        std::string failureResult;       ///< What happens on failure
        int failureQuantity = 0;         ///< Quantity on failure

        // Special effects
        std::vector<FoodEffect> additionalEffects;
        std::unordered_map<std::string, float> skillGains;

        // Unlocking
        bool unlockedByDefault = true;   ///< Whether recipe is unlocked by default
        std::vector<std::string> unlockRequirements;
    };

    /**
     * @struct FoodStats
     * @brief Performance statistics for food system
     */
    struct FoodStats {
        int totalFoodItems = 0;          ///< Total food items in system
        int cookedItems = 0;             ///< Items cooked
        int spoiledItems = 0;            ///< Items spoiled
        int recipesUsed = 0;             ///< Recipes used
        int successfulCooks = 0;         ///< Successful cooking attempts
        int failedCooks = 0;             ///< Failed cooking attempts
        float averageFreshness = 0.0f;   ///< Average food freshness
        float averageQuality = 0.0f;     ///< Average food quality
    };

    /**
     * @class FoodSystem
     * @brief Advanced food and cooking system
     *
     * Features:
     * - 1000+ food items with detailed nutrition
     * - Complex cooking system with multiple methods
     * - Food spoilage and preservation
     * - Nutritional tracking and diet effects
     * - Cooking recipes and skill progression
     * - Food quality and cooking skill influence
     * - Magical and special effect foods
     * - Temperature-based cooking mechanics
     * - Food combinations and recipes
     * - Survival cooking and foraging
     * - Restaurant and cooking mini-games
     * - Food poisoning and disease mechanics
     * - Seasonal and regional food variations
     * - Cooking competitions and challenges
     * - Food preservation and storage systems
     */
    class FoodSystem {
    public:
        static FoodSystem& GetInstance();

        /**
         * @brief Initialize the food system
         * @param config Food system configuration
         * @return true if successful
         */
        bool Initialize(const std::string& configPath = "");

        /**
         * @brief Shutdown the food system
         */
        void Shutdown();

        /**
         * @brief Update food system (call every frame)
         * @param deltaTime Time since last update
         */
        void Update(float deltaTime);

        // Food Database Management
        /**
         * @brief Register a new food item
         * @param foodItem Food item definition
         * @return true if successful
         */
        bool RegisterFoodItem(const FoodItem& foodItem);

        /**
         * @brief Get food item by ID
         * @param foodId Food item ID
         * @return Food item definition
         */
        const FoodItem* GetFoodItem(const std::string& foodId) const;

        /**
         * @brief Get all food items of a type
         * @param type Food type
         * @return Vector of food items
         */
        std::vector<FoodItem> GetFoodItemsByType(FoodType type) const;

        /**
         * @brief Check if food item exists
         * @param foodId Food item ID
         * @return true if exists
         */
        bool FoodItemExists(const std::string& foodId) const;

        // Food Consumption
        /**
         * @brief Eat food item
         * @param player Player eating the food
         * @param foodId Food item ID
         * @param quantity Quantity to eat
         * @return true if successful
         */
        bool EatFood(Player* player, const std::string& foodId, int quantity = 1);

        /**
         * @brief Drink beverage
         * @param player Player drinking
         * @param foodId Beverage ID
         * @param quantity Quantity to drink
         * @return true if successful
         */
        bool DrinkBeverage(Player* player, const std::string& foodId, int quantity = 1);

        /**
         * @brief Calculate food effects on player
         * @param player Player
         * @param foodItem Food item
         * @param quantity Quantity consumed
         * @return true if successful
         */
        bool ApplyFoodEffects(Player* player, const FoodItem& foodItem, int quantity = 1);

        // Cooking System
        /**
         * @brief Start cooking food
         * @param player Player cooking
         * @param recipeId Recipe ID
         * @param stationId Cooking station ID
         * @return true if cooking started
         */
        bool StartCooking(Player* player, const std::string& recipeId, const std::string& stationId = "");

        /**
         * @brief Cook food item
         * @param player Player cooking
         * @param inputFoodId Input food ID
         * @param method Cooking method
         * @param cookTime Cooking time
         * @param temperature Cooking temperature
         * @return true if successful
         */
        bool CookFood(Player* player, const std::string& inputFoodId, CookingMethod method,
                     float cookTime, float temperature);

        /**
         * @brief Get cooking result
         * @param inputFoodId Input food ID
         * @param method Cooking method
         * @param cookTime Cooking time
         * @param temperature Cooking temperature
         * @param skillLevel Cooking skill level
         * @return Output food ID
         */
        std::string GetCookingResult(const std::string& inputFoodId, CookingMethod method,
                                   float cookTime, float temperature, int skillLevel) const;

        // Recipe Management
        /**
         * @brief Register cooking recipe
         * @param recipe Cooking recipe
         * @return true if successful
         */
        bool RegisterRecipe(const CookingRecipe& recipe);

        /**
         * @brief Get recipe by ID
         * @param recipeId Recipe ID
         * @return Cooking recipe
         */
        const CookingRecipe* GetRecipe(const std::string& recipeId) const;

        /**
         * @brief Get all available recipes
         * @return Vector of recipes
         */
        std::vector<CookingRecipe> GetAllRecipes() const;

        /**
         * @brief Check if player can cook recipe
         * @param player Player
         * @param recipeId Recipe ID
         * @return true if can cook
         */
        bool CanCookRecipe(Player* player, const std::string& recipeId) const;

        // Food Preservation
        /**
         * @brief Preserve food item
         * @param foodId Food item ID
         * @param method Preservation method
         * @return true if successful
         */
        bool PreserveFood(const std::string& foodId, PreservationMethod method);

        /**
         * @brief Check if food is spoiled
         * @param foodId Food item ID
         * @param age Food age in seconds
         * @return true if spoiled
         */
        bool IsFoodSpoiled(const std::string& foodId, float age) const;

        /**
         * @brief Get food freshness level
         * @param foodId Food item ID
         * @param age Food age in seconds
         * @return Freshness level (0-1)
         */
        float GetFoodFreshness(const std::string& foodId, float age) const;

        // Food Quality
        /**
         * @brief Calculate food quality
         * @param foodItem Food item
         * @param cookingSkill Cooking skill level
         * @param ingredientsQuality Average ingredient quality
         * @return Food quality
         */
        FoodQuality CalculateFoodQuality(const FoodItem& foodItem, int cookingSkill, float ingredientsQuality) const;

        /**
         * @brief Get quality modifier
         * @param quality Food quality
         * @return Quality modifier (0-2)
         */
        float GetQualityModifier(FoodQuality quality) const;

        // Nutrition and Diet
        /**
         * @brief Get nutritional value of food
         * @param foodId Food item ID
         * @param quantity Quantity
         * @return Nutrition info
         */
        NutritionInfo GetNutritionalValue(const std::string& foodId, int quantity = 1) const;

        /**
         * @brief Track player nutrition
         * @param player Player
         * @param nutrition Nutrition to add
         * @return true if successful
         */
        bool TrackPlayerNutrition(Player* player, const NutritionInfo& nutrition);

        /**
         * @brief Get player diet statistics
         * @param player Player
         * @return Diet statistics
         */
        std::unordered_map<std::string, float> GetPlayerDietStats(Player* player) const;

        // Food Generation
        /**
         * @brief Generate food item from ingredients
         * @param ingredients Map of ingredient IDs to quantities
         * @param method Cooking method
         * @return Generated food item ID
         */
        std::string GenerateFoodItem(const std::unordered_map<std::string, int>& ingredients,
                                   CookingMethod method) const;

        /**
         * @brief Generate random food item
         * @param type Food type
         * @param quality Food quality
         * @return Generated food item
         */
        FoodItem GenerateRandomFoodItem(FoodType type, FoodQuality quality) const;

        // Special Food Effects
        /**
         * @brief Apply magical food effect
         * @param player Player
         * @param foodId Magical food ID
         * @return true if successful
         */
        bool ApplyMagicalEffect(Player* player, const std::string& foodId);

        /**
         * @brief Apply poisonous food effect
         * @param player Player
         * @param foodId Poisonous food ID
         * @return true if successful
         */
        bool ApplyPoisonEffect(Player* player, const std::string& foodId);

        /**
         * @brief Apply medicinal food effect
         * @param player Player
         * @param foodId Medicinal food ID
         * @return true if successful
         */
        bool ApplyMedicineEffect(Player* player, const std::string& foodId);

        // Configuration
        /**
         * @brief Set food system configuration
         * @param config Configuration data
         */
        void SetConfig(const std::unordered_map<std::string, float>& config);

        /**
         * @brief Get configuration value
         * @param key Configuration key
         * @return Configuration value
         */
        float GetConfigValue(const std::string& key) const;

        // Statistics
        /**
         * @brief Get food system statistics
         * @return Current statistics
         */
        const FoodStats& GetStats() const { return m_stats; }

        /**
         * @brief Reset statistics
         */
        void ResetStats();

        // Debug
        /**
         * @brief Enable debug mode
         * @param enable Enable state
         */
        void EnableDebugMode(bool enable) { m_debugMode = enable; }

        /**
         * @brief Get debug information
         * @return Debug info string
         */
        std::string GetDebugInfo() const;

        /**
         * @brief Validate food system data
         * @return true if valid
         */
        bool ValidateData() const;

    private:
        FoodSystem() = default;
        ~FoodSystem();

        // Prevent copying
        FoodSystem(const FoodSystem&) = delete;
        FoodSystem& operator=(const FoodSystem&) = delete;

        // Core food processing
        void UpdateFoodSpoilage(float deltaTime);
        void UpdateCookingProgress(float deltaTime);
        void UpdateFoodEffects(float deltaTime);

        // Food calculation methods
        float CalculateCookingQuality(CookingMethod method, float cookTime, float optimalTime, float temperature, float optimalTemp) const;
        float CalculatePreservationEffectiveness(PreservationMethod method, float age) const;
        NutritionInfo CalculateEffectiveNutrition(const FoodItem& foodItem, FoodQuality quality, float freshness) const;

        // Recipe processing
        bool ValidateRecipeIngredients(const CookingRecipe& recipe, const std::unordered_map<std::string, int>& availableIngredients) const;
        CookingRecipe GenerateDynamicRecipe(const std::vector<std::string>& ingredients) const;

        // Food generation
        FoodItem CreateFoodFromIngredients(const std::vector<FoodItem>& ingredients, CookingMethod method) const;
        std::string GenerateFoodId(const std::vector<std::string>& ingredients, CookingMethod method) const;

        // Utility methods
        float RandomFloat(float min, float max) const;
        int RandomInt(int min, int max) const;
        bool RollChance(float probability) const;

        // Member variables
        bool m_initialized = false;
        bool m_debugMode = false;

        // Food database
        std::unordered_map<std::string, FoodItem> m_foodItems;
        std::unordered_map<std::string, CookingRecipe> m_recipes;
        std::unordered_map<std::string, float> m_config;

        // Runtime data
        std::unordered_map<std::string, float> m_foodAges;
        std::unordered_map<std::string, float> m_cookingProgress;
        std::unordered_map<Player*, std::unordered_map<std::string, float>> m_playerNutrition;
        std::unordered_map<Player*, std::vector<FoodEffect>> m_activeFoodEffects;

        // Statistics
        FoodStats m_stats;

        // Thread safety
        mutable std::shared_mutex m_foodMutex;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_SURVIVAL_FOOD_SYSTEM_HPP
