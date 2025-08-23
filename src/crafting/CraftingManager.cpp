/**
 * @file CraftingManager.cpp
 * @brief VoxelCraft Crafting Manager Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "CraftingManager.hpp"
#include "../player/Player.hpp"
#include "../inventory/Inventory.hpp"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <sstream>

namespace VoxelCraft {

    CraftingManager& CraftingManager::GetInstance() {
        static CraftingManager instance;
        return instance;
    }

    bool CraftingManager::Initialize(const CraftingManagerConfig& config) {
        if (m_initialized) {
            return true;
        }

        m_config = config;

        // Load default recipes
        size_t loaded = LoadDefaultRecipes();
        if (loaded == 0) {
            return false;
        }

        // Initialize caches
        UpdateCaches();

        m_initialized = true;
        NotifyRecipeChange("CraftingManager initialized with " + std::to_string(loaded) + " recipes");

        return true;
    }

    void CraftingManager::Shutdown() {
        if (!m_initialized) {
            return;
        }

        // Save custom recipes if enabled
        if (m_config.autoSaveCustomRecipes && !m_customRecipes.empty()) {
            SaveCustomRecipes(m_config.customRecipesPath + "autosave.json");
        }

        // Clear all data
        m_recipes.clear();
        m_defaultRecipes.clear();
        m_customRecipes.clear();
        ClearCache();

        m_initialized = false;
        NotifyRecipeChange("CraftingManager shutdown");
    }

    bool CraftingManager::RegisterRecipe(std::shared_ptr<CraftingRecipe> recipe) {
        if (!recipe || !ValidateRecipe(recipe)) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_mutex);

        // Check if recipe already exists
        if (m_recipes.find(recipe->GetRecipeID()) != m_recipes.end()) {
            return false;
        }

        m_recipes[recipe->GetRecipeID()] = recipe;

        // Add to appropriate list
        if (recipe->GetRecipeID() >= 10000) {
            m_customRecipes.push_back(recipe);
            m_stats.customRecipes++;
        } else {
            m_defaultRecipes.push_back(recipe);
        }

        // Update type counters
        switch (recipe->GetType()) {
            case RecipeType::SHAPED:
                m_stats.shapedRecipes++;
                break;
            case RecipeType::SHAPELESS:
                m_stats.shapelessRecipes++;
                break;
            case RecipeType::SMELTING:
                m_stats.smeltingRecipes++;
                break;
            default:
                break;
        }

        m_stats.totalRecipes++;

        UpdateCaches();
        NotifyRecipeChange("Recipe registered: " + recipe->GetName());

        return true;
    }

    bool CraftingManager::UnregisterRecipe(int recipeID) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);

        auto it = m_recipes.find(recipeID);
        if (it == m_recipes.end()) {
            return false;
        }

        auto recipe = it->second;
        m_recipes.erase(it);

        // Remove from appropriate list
        if (recipeID >= 10000) {
            m_customRecipes.erase(std::remove(m_customRecipes.begin(), m_customRecipes.end(), recipe), m_customRecipes.end());
            m_stats.customRecipes--;
        } else {
            m_defaultRecipes.erase(std::remove(m_defaultRecipes.begin(), m_defaultRecipes.end(), recipe), m_defaultRecipes.end());
        }

        // Update type counters
        switch (recipe->GetType()) {
            case RecipeType::SHAPED:
                m_stats.shapedRecipes--;
                break;
            case RecipeType::SHAPELESS:
                m_stats.shapelessRecipes--;
                break;
            case RecipeType::SMELTING:
                m_stats.smeltingRecipes--;
                break;
            default:
                break;
        }

        m_stats.totalRecipes--;

        UpdateCaches();
        NotifyRecipeChange("Recipe unregistered: " + recipe->GetName());

        return true;
    }

    std::shared_ptr<CraftingRecipe> CraftingManager::GetRecipe(int recipeID) const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);

        auto it = m_recipes.find(recipeID);
        return (it != m_recipes.end()) ? it->second : nullptr;
    }

    std::vector<std::shared_ptr<CraftingRecipe>> CraftingManager::FindMatchingRecipes(
        const std::vector<RecipeIngredient>& ingredients) const {

        std::shared_lock<std::shared_mutex> lock(m_mutex);
        std::vector<std::shared_ptr<CraftingRecipe>> matches;

        for (const auto& pair : m_recipes) {
            if (pair.second->Matches(ingredients)) {
                matches.push_back(pair.second);
            }
        }

        return matches;
    }

    RecipeSearchResult CraftingManager::SearchRecipes(const std::string& query, int maxResults) const {
        auto startTime = std::chrono::high_resolution_clock::now();

        // Check cache first
        std::string cacheKey = query + "_" + std::to_string(maxResults);
        auto cacheIt = m_searchCache.find(cacheKey);
        if (cacheIt != m_searchCache.end()) {
            m_stats.cacheHits++;
            return cacheIt->second;
        }

        m_stats.cacheMisses++;
        RecipeSearchResult result;

        std::shared_lock<std::shared_mutex> lock(m_mutex);

        // Simple text search in recipe names
        std::string lowerQuery = query;
        std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);

        for (const auto& pair : m_recipes) {
            std::string recipeName = pair.second->GetName();
            std::transform(recipeName.begin(), recipeName.end(), recipeName.begin(), ::tolower);

            if (recipeName.find(lowerQuery) != std::string::npos) {
                result.recipes.push_back(pair.second);
                if (result.recipes.size() >= static_cast<size_t>(maxResults)) {
                    result.hasMore = true;
                    break;
                }
            }
        }

        result.totalFound = result.recipes.size();

        auto endTime = std::chrono::high_resolution_clock::now();
        result.searchTime = std::chrono::duration<float>(endTime - startTime).count();
        m_stats.totalSearchTime += result.searchTime;

        // Cache result
        if (m_searchCache.size() < static_cast<size_t>(m_config.maxRecipeCacheSize)) {
            m_searchCache[cacheKey] = result;
        }

        return result;
    }

    std::vector<std::shared_ptr<CraftingRecipe>> CraftingManager::GetRecipesByCategory(RecipeCategory category) const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);

        // Check cache first
        auto cacheIt = m_categoryCache.find(category);
        if (cacheIt != m_categoryCache.end()) {
            return cacheIt->second;
        }

        // Build result
        std::vector<std::shared_ptr<CraftingRecipe>> result;
        for (const auto& pair : m_recipes) {
            if (pair.second->GetCategory() == category) {
                result.push_back(pair.second);
            }
        }

        // Cache result
        m_categoryCache[category] = result;

        return result;
    }

    std::vector<std::shared_ptr<CraftingRecipe>> CraftingManager::GetRecipesByType(RecipeType type) const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);

        // Check cache first
        auto cacheIt = m_typeCache.find(type);
        if (cacheIt != m_typeCache.end()) {
            return cacheIt->second;
        }

        // Build result
        std::vector<std::shared_ptr<CraftingRecipe>> result;
        for (const auto& pair : m_recipes) {
            if (pair.second->GetType() == type) {
                result.push_back(pair.second);
            }
        }

        // Cache result
        m_typeCache[type] = result;

        return result;
    }

    std::vector<std::shared_ptr<CraftingRecipe>> CraftingManager::GetAllRecipes() const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);

        std::vector<std::shared_ptr<CraftingRecipe>> allRecipes;
        allRecipes.reserve(m_recipes.size());

        for (const auto& pair : m_recipes) {
            allRecipes.push_back(pair.second);
        }

        return allRecipes;
    }

    bool CraftingManager::TryCraftRecipe(std::shared_ptr<CraftingRecipe> recipe, Player* player) {
        if (!recipe || !player) {
            return false;
        }

        if (!CanCraftRecipe(recipe, player)) {
            m_stats.failedCrafts++;
            return false;
        }

        auto startTime = std::chrono::high_resolution_clock::now();

        // Get required ingredients
        auto requiredIngredients = recipe->GetRequiredIngredients();
        auto playerInventory = player->GetInventory();

        // Consume ingredients
        for (const auto& ingredient : requiredIngredients) {
            if (!playerInventory->RemoveItem(ingredient.itemID, ingredient.count)) {
                m_stats.failedCrafts++;
                return false;
            }
        }

        // Add result
        auto result = recipe->GetResult();
        if (!playerInventory->AddItem(result.itemID, result.count)) {
            m_stats.failedCrafts++;
            return false;
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        float craftTime = std::chrono::duration<float>(endTime - startTime).count();

        // Update statistics
        m_stats.totalCrafts++;
        m_stats.successfulCrafts++;
        m_stats.averageCraftTime = (m_stats.averageCraftTime * (m_stats.totalCrafts - 1) + craftTime) / m_stats.totalCrafts;

        return true;
    }

    bool CraftingManager::CanCraftRecipe(std::shared_ptr<CraftingRecipe> recipe, Player* player) const {
        if (!recipe || !player) {
            return false;
        }

        auto requiredIngredients = recipe->GetRequiredIngredients();
        auto playerInventory = player->GetInventory();

        // Check if player has all required ingredients
        for (const auto& ingredient : requiredIngredients) {
            if (playerInventory->GetItemCount(ingredient.itemID) < ingredient.count) {
                return false;
            }
        }

        // Check if there's space for the result
        auto result = recipe->GetResult();
        return playerInventory->CanAddItem(result.itemID, result.count);
    }

    size_t CraftingManager::LoadDefaultRecipes() {
        size_t loaded = 0;

        // Load shaped recipes
        LoadShapedRecipes();
        loaded += m_stats.shapedRecipes;

        // Load shapeless recipes
        LoadShapelessRecipes();
        loaded += m_stats.shapelessRecipes;

        // Load smelting recipes
        LoadSmeltingRecipes();
        loaded += m_stats.smeltingRecipes;

        return loaded;
    }

    void CraftingManager::LoadShapedRecipes() {
        auto shapedRecipes = Recipes::GetAllShapedRecipes();
        for (auto& recipe : shapedRecipes) {
            RegisterRecipe(recipe);
        }
    }

    void CraftingManager::LoadShapelessRecipes() {
        auto shapelessRecipes = ShapelessRecipes::GetAllShapelessRecipes();
        for (auto& recipe : shapelessRecipes) {
            RegisterRecipe(recipe);
        }
    }

    void CraftingManager::LoadSmeltingRecipes() {
        auto smeltingRecipes = SmeltingRecipes::GetAllSmeltingRecipes();
        for (auto& recipe : smeltingRecipes) {
            RegisterRecipe(recipe);
        }
    }

    size_t CraftingManager::LoadCustomRecipes(const std::string& filename) {
        // TODO: Implement JSON loading of custom recipes
        return 0;
    }

    bool CraftingManager::SaveCustomRecipes(const std::string& filename) const {
        // TODO: Implement JSON saving of custom recipes
        return false;
    }

    int CraftingManager::CreateCustomRecipe(std::shared_ptr<CraftingRecipe> recipe) {
        if (!recipe || !m_config.enableCustomRecipes) {
            return -1;
        }

        // Generate custom recipe ID
        int customID = GenerateRecipeID();
        recipe->SetRecipeID(customID);

        if (RegisterRecipe(recipe)) {
            return customID;
        }

        return -1;
    }

    bool CraftingManager::RemoveCustomRecipe(int recipeID) {
        if (recipeID < 10000) {
            return false; // Not a custom recipe
        }

        return UnregisterRecipe(recipeID);
    }

    void CraftingManager::ClearCache() {
        m_searchCache.clear();
        m_categoryCache.clear();
        m_typeCache.clear();
    }

    void CraftingManager::SetConfig(const CraftingManagerConfig& config) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        m_config = config;
        ClearCache();
    }

    void CraftingManager::RegisterRecipeChangeCallback(std::function<void(const std::string&)> callback) {
        if (callback) {
            m_changeCallbacks.push_back(callback);
        }
    }

    int CraftingManager::GenerateRecipeID() {
        return m_nextRecipeID++;
    }

    bool CraftingManager::ValidateRecipe(std::shared_ptr<CraftingRecipe> recipe) const {
        if (!recipe) {
            return false;
        }

        // Check recipe name
        if (recipe->GetName().empty()) {
            return false;
        }

        // Check ingredients
        auto ingredients = recipe->GetRequiredIngredients();
        if (ingredients.empty()) {
            return false;
        }

        // Check result
        auto result = recipe->GetResult();
        if (result.itemID == 0 || result.count == 0) {
            return false;
        }

        return true;
    }

    void CraftingManager::UpdateCaches() {
        ClearCache();
        // Caches will be rebuilt on demand
    }

    void CraftingManager::NotifyRecipeChange(const std::string& message) {
        for (const auto& callback : m_changeCallbacks) {
            if (callback) {
                callback(message);
            }
        }
    }

    void CraftingManager::InitializeRecipeBook() {
        // TODO: Initialize recipe book system
    }

} // namespace VoxelCraft
