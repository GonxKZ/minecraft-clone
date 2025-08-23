/**
 * @file CraftingTable.cpp
 * @brief VoxelCraft Crafting Table Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "CraftingTable.hpp"
#include "../player/Player.hpp"
#include "../inventory/Inventory.hpp"
#include <algorithm>
#include <chrono>

namespace VoxelCraft {

    CraftingTable::CraftingTable(CraftingTableType type)
        : m_type(type), m_craftingGrid(3, 3) {
        InitializeDefaultRecipes();
    }

    CraftingTable::~CraftingTable() {
        // Cleanup resources
    }

    std::string CraftingTable::GetName() const {
        switch (m_type) {
            case CraftingTableType::WORKBENCH: return "Crafting Table";
            case CraftingTableType::FURNACE: return "Furnace";
            case CraftingTableType::BREWING_STAND: return "Brewing Stand";
            case CraftingTableType::SMITHING_TABLE: return "Smithing Table";
            case CraftingTableType::ANVIL: return "Anvil";
            case CraftingTableType::ENCHANTMENT_TABLE: return "Enchantment Table";
            case CraftingTableType::CARTOGRAPHY_TABLE: return "Cartography Table";
            case CraftingTableType::FLETCHING_TABLE: return "Fletching Table";
            case CraftingTableType::LOOM: return "Loom";
            case CraftingTableType::STONECUTTER: return "Stonecutter";
            case CraftingTableType::GRINDSTONE: return "Grindstone";
            default: return "Unknown Crafting Table";
        }
    }

    void CraftingTable::AddRecipe(std::shared_ptr<CraftingRecipe> recipe) {
        if (recipe) {
            m_recipes[recipe->GetRecipeID()] = recipe;
        }
    }

    void CraftingTable::RemoveRecipe(int recipeID) {
        m_recipes.erase(recipeID);
    }

    std::shared_ptr<CraftingRecipe> CraftingTable::FindMatchingRecipe() const {
        for (const auto& pair : m_recipes) {
            if (GridMatchesRecipe(pair.second.get())) {
                return pair.second;
            }
        }
        return nullptr;
    }

    std::vector<std::shared_ptr<CraftingRecipe>> CraftingTable::GetAvailableRecipes() const {
        std::vector<std::shared_ptr<CraftingRecipe>> recipes;
        recipes.reserve(m_recipes.size());

        for (const auto& pair : m_recipes) {
            recipes.push_back(pair.second);
        }

        return recipes;
    }

    bool CraftingTable::Craft(Inventory* inventory) {
        if (!inventory) return false;

        auto recipe = FindMatchingRecipe();
        if (!recipe) return false;

        // Check if we can craft
        if (!CanCraft(inventory)) return false;

        // Consume ingredients
        if (!ConsumeIngredients(recipe.get(), inventory)) return false;

        // Add result to inventory
        auto result = recipe->GetResult();
        if (!AddResultToInventory(result, inventory)) {
            // If we can't add result, we need to refund ingredients
            // This is a simplified version - in reality you'd want to refund
            return false;
        }

        return true;
    }

    bool CraftingTable::CanCraft(const Inventory* inventory) const {
        if (!inventory) return false;

        auto recipe = FindMatchingRecipe();
        if (!recipe) return false;

        // Check if player has required ingredients
        auto requiredIngredients = recipe->GetRequiredIngredients();

        for (const auto& required : requiredIngredients) {
            if (inventory->GetItemCount(required.itemID) < required.count) {
                return false;
            }
        }

        // Check if there's space for the result
        auto result = recipe->GetResult();
        return inventory->CanAddItem(result.itemID, result.count);
    }

    RecipeResult CraftingTable::GetCraftingResult() const {
        auto recipe = FindMatchingRecipe();
        if (recipe) {
            return recipe->GetResult();
        }
        return RecipeResult();
    }

    void CraftingTable::ClearGrid() {
        for (auto& slot : m_craftingGrid.slots) {
            slot = RecipeIngredient();
        }
    }

    void CraftingTable::SetGridItem(int x, int y, const RecipeIngredient& ingredient) {
        if (x >= 0 && x < m_craftingGrid.width && y >= 0 && y < m_craftingGrid.height) {
            m_craftingGrid.slots[y * m_craftingGrid.width + x] = ingredient;
        }
    }

    RecipeIngredient CraftingTable::GetGridItem(int x, int y) const {
        if (x >= 0 && x < m_craftingGrid.width && y >= 0 && y < m_craftingGrid.height) {
            return m_craftingGrid.slots[y * m_craftingGrid.width + x];
        }
        return RecipeIngredient();
    }

    void CraftingTable::InitializeDefaultRecipes() {
        // This would be populated with all default Minecraft recipes
        // For now, we'll add a few basic ones as examples

        // Basic recipes would be loaded here
        // This is where you'd add all the default game recipes
    }

    bool CraftingTable::GridMatchesRecipe(const CraftingRecipe* recipe) const {
        if (!recipe) return false;

        // Convert grid to ingredients list for matching
        std::vector<RecipeIngredient> gridIngredients;
        for (const auto& slot : m_craftingGrid.slots) {
            if (slot.itemID != 0) {
                gridIngredients.push_back(slot);
            }
        }

        return recipe->Matches(gridIngredients);
    }

    bool CraftingTable::ConsumeIngredients(const CraftingRecipe* recipe, Inventory* inventory) {
        if (!recipe || !inventory) return false;

        auto requiredIngredients = recipe->GetRequiredIngredients();

        // Check if we have all ingredients before consuming
        for (const auto& required : requiredIngredients) {
            if (inventory->GetItemCount(required.itemID) < required.count) {
                return false;
            }
        }

        // Consume ingredients
        for (const auto& required : requiredIngredients) {
            if (!inventory->RemoveItem(required.itemID, required.count)) {
                return false;
            }
        }

        return true;
    }

    bool CraftingTable::AddResultToInventory(const RecipeResult& result, Inventory* inventory) {
        if (!inventory) return false;
        return inventory->AddItem(result.itemID, result.count);
    }

} // namespace VoxelCraft
