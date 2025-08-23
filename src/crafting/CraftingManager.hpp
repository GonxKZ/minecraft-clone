/**
 * @file CraftingManager.hpp
 * @brief VoxelCraft Crafting Manager - Central Recipe Management
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_CRAFTING_CRAFTING_MANAGER_HPP
#define VOXELCRAFT_CRAFTING_CRAFTING_MANAGER_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <mutex>
#include <shared_mutex>
#include <functional>
#include "CraftingRecipe.hpp"
#include "ShapedRecipe.hpp"
#include "ShapelessRecipe.hpp"
#include "SmeltingRecipe.hpp"

namespace VoxelCraft {

    class Player;
    class Inventory;
    class World;

    /**
     * @struct CraftingManagerConfig
     * @brief Configuration for crafting manager
     */
    struct CraftingManagerConfig {
        bool enableAdvancedRecipes = true;
        bool enableCustomRecipes = true;
        bool enableRecipeDiscovery = true;
        bool enableRecipeBook = true;
        int maxRecipeCacheSize = 1000;
        int maxCustomRecipes = 500;
        bool autoSaveCustomRecipes = true;
        std::string customRecipesPath = "data/custom_recipes/";
    };

    /**
     * @struct RecipeSearchResult
     * @brief Result of recipe search
     */
    struct RecipeSearchResult {
        std::vector<std::shared_ptr<CraftingRecipe>> recipes;
        bool hasMore = false;
        int totalFound = 0;
        float searchTime = 0.0f;
    };

    /**
     * @class CraftingManager
     * @brief Central manager for all crafting systems
     */
    class CraftingManager {
    public:
        /**
         * @brief Get singleton instance
         * @return CraftingManager instance
         */
        static CraftingManager& GetInstance();

        /**
         * @brief Initialize crafting manager
         * @param config Configuration
         * @return true if initialized successfully
         */
        bool Initialize(const CraftingManagerConfig& config);

        /**
         * @brief Shutdown crafting manager
         */
        void Shutdown();

        /**
         * @brief Register a crafting recipe
         * @param recipe Recipe to register
         * @return true if registered successfully
         */
        bool RegisterRecipe(std::shared_ptr<CraftingRecipe> recipe);

        /**
         * @brief Unregister a crafting recipe
         * @param recipeID Recipe ID to unregister
         * @return true if unregistered successfully
         */
        bool UnregisterRecipe(int recipeID);

        /**
         * @brief Get recipe by ID
         * @param recipeID Recipe ID
         * @return Recipe pointer or nullptr
         */
        std::shared_ptr<CraftingRecipe> GetRecipe(int recipeID) const;

        /**
         * @brief Find recipes that match ingredients
         * @param ingredients Available ingredients
         * @return Matching recipes
         */
        std::vector<std::shared_ptr<CraftingRecipe>> FindMatchingRecipes(
            const std::vector<RecipeIngredient>& ingredients) const;

        /**
         * @brief Search recipes by name
         * @param query Search query
         * @param maxResults Maximum results to return
         * @return Search results
         */
        RecipeSearchResult SearchRecipes(const std::string& query, int maxResults = 50) const;

        /**
         * @brief Get recipes by category
         * @param category Recipe category
         * @return Recipes in category
         */
        std::vector<std::shared_ptr<CraftingRecipe>> GetRecipesByCategory(RecipeCategory category) const;

        /**
         * @brief Get recipes by type
         * @param type Recipe type
         * @return Recipes of type
         */
        std::vector<std::shared_ptr<CraftingRecipe>> GetRecipesByType(RecipeType type) const;

        /**
         * @brief Get all available recipes
         * @return All recipes
         */
        std::vector<std::shared_ptr<CraftingRecipe>> GetAllRecipes() const;

        /**
         * @brief Try to craft a recipe
         * @param recipe Recipe to craft
         * @param player Player doing the crafting
         * @return true if crafted successfully
         */
        bool TryCraftRecipe(std::shared_ptr<CraftingRecipe> recipe, Player* player);

        /**
         * @brief Check if player can craft recipe
         * @param recipe Recipe to check
         * @param player Player to check
         * @return true if can craft
         */
        bool CanCraftRecipe(std::shared_ptr<CraftingRecipe> recipe, Player* player) const;

        /**
         * @brief Load default recipes
         * @return Number of recipes loaded
         */
        size_t LoadDefaultRecipes();

        /**
         * @brief Load custom recipes from file
         * @param filename Recipe file path
         * @return Number of recipes loaded
         */
        size_t LoadCustomRecipes(const std::string& filename);

        /**
         * @brief Save custom recipes to file
         * @param filename Recipe file path
         * @return true if saved successfully
         */
        bool SaveCustomRecipes(const std::string& filename) const;

        /**
         * @brief Create custom recipe
         * @param recipe Recipe to add as custom
         * @return Recipe ID if successful, -1 if failed
         */
        int CreateCustomRecipe(std::shared_ptr<CraftingRecipe> recipe);

        /**
         * @brief Remove custom recipe
         * @param recipeID Custom recipe ID
         * @return true if removed successfully
         */
        bool RemoveCustomRecipe(int recipeID);

        /**
         * @brief Get crafting statistics
         * @return Crafting stats
         */
        const CraftingStats& GetStats() const { return m_stats; }

        /**
         * @brief Clear recipe cache
         */
        void ClearCache();

        /**
         * @brief Get configuration
         * @return Current configuration
         */
        const CraftingManagerConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set configuration
         * @param config New configuration
         */
        void SetConfig(const CraftingManagerConfig& config);

        /**
         * @brief Register recipe change callback
         * @param callback Callback function
         */
        void RegisterRecipeChangeCallback(std::function<void(const std::string&)> callback);

    private:
        CraftingManager() = default;
        ~CraftingManager() = default;
        CraftingManager(const CraftingManager&) = delete;
        CraftingManager& operator=(const CraftingManager&) = delete;

        CraftingManagerConfig m_config;
        mutable std::shared_mutex m_mutex;

        // Recipe storage
        std::unordered_map<int, std::shared_ptr<CraftingRecipe>> m_recipes;
        std::vector<std::shared_ptr<CraftingRecipe>> m_defaultRecipes;
        std::vector<std::shared_ptr<CraftingRecipe>> m_customRecipes;

        // Caching
        mutable std::unordered_map<std::string, RecipeSearchResult> m_searchCache;
        mutable std::unordered_map<RecipeCategory, std::vector<std::shared_ptr<CraftingRecipe>>> m_categoryCache;
        mutable std::unordered_map<RecipeType, std::vector<std::shared_ptr<CraftingRecipe>>> m_typeCache;

        // Statistics
        CraftingStats m_stats;
        int m_nextRecipeID = 10000; // Custom recipes start from 10000

        // Callbacks
        std::vector<std::function<void(const std::string&)>> m_changeCallbacks;

        bool m_initialized = false;

        /**
         * @brief Generate unique recipe ID
         * @return Unique recipe ID
         */
        int GenerateRecipeID();

        /**
         * @brief Validate recipe before registration
         * @param recipe Recipe to validate
         * @return true if valid
         */
        bool ValidateRecipe(std::shared_ptr<CraftingRecipe> recipe) const;

        /**
         * @brief Update caches after recipe changes
         */
        void UpdateCaches();

        /**
         * @brief Notify recipe change listeners
         * @param message Change message
         */
        void NotifyRecipeChange(const std::string& message);

        /**
         * @brief Load shaped recipes
         */
        void LoadShapedRecipes();

        /**
         * @brief Load shapeless recipes
         */
        void LoadShapelessRecipes();

        /**
         * @brief Load smelting recipes
         */
        void LoadSmeltingRecipes();

        /**
         * @brief Initialize recipe book
         */
        void InitializeRecipeBook();
    };

    /**
     * @struct CraftingStats
     * @brief Statistics for crafting manager
     */
    struct CraftingStats {
        int totalRecipes = 0;
        int shapedRecipes = 0;
        int shapelessRecipes = 0;
        int smeltingRecipes = 0;
        int customRecipes = 0;
        int totalCrafts = 0;
        int successfulCrafts = 0;
        int failedCrafts = 0;
        float averageCraftTime = 0.0f;
        int cacheHits = 0;
        int cacheMisses = 0;
        float totalSearchTime = 0.0f;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_CRAFTING_CRAFTING_MANAGER_HPP
