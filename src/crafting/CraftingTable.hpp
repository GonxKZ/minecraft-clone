/**
 * @file CraftingTable.hpp
 * @brief VoxelCraft Crafting System - Crafting Table Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_CRAFTING_CRAFTING_TABLE_HPP
#define VOXELCRAFT_CRAFTING_CRAFTING_TABLE_HPP

#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include "CraftingRecipe.hpp"

namespace VoxelCraft {

    class Inventory;

    /**
     * @enum CraftingTableType
     * @brief Types of crafting tables
     */
    enum class CraftingTableType {
        WORKBENCH = 0,      ///< 3x3 crafting grid
        FURNACE,            ///< Smelting furnace
        BREWING_STAND,      ///< Brewing stand
        SMITHING_TABLE,     ///< Smithing table
        ANVIL,              ///< Anvil for repairs
        ENCHANTMENT_TABLE,  ///< Enchantment table
        CARTOGRAPHY_TABLE,  ///< Cartography table
        FLETCHING_TABLE,    ///< Fletching table
        LOOM,               ///< Loom for banners
        STONECUTTER,        ///< Stonecutter
        GRINDSTONE          ///< Grindstone for disenchanting
    };

    /**
     * @struct CraftingGrid
     * @brief Represents the crafting grid
     */
    struct CraftingGrid {
        static const int MAX_WIDTH = 3;
        static const int MAX_HEIGHT = 3;

        int width;
        int height;
        std::vector<RecipeIngredient> slots;

        CraftingGrid(int w = 3, int h = 3)
            : width(w), height(h), slots(w * h) {}
    };

    /**
     * @class CraftingTable
     * @brief Base class for crafting tables
     */
    class CraftingTable {
    public:
        /**
         * @brief Constructor
         * @param type Crafting table type
         */
        CraftingTable(CraftingTableType type = CraftingTableType::WORKBENCH);

        /**
         * @brief Destructor
         */
        virtual ~CraftingTable();

        /**
         * @brief Get crafting table type
         * @return Table type
         */
        CraftingTableType GetType() const { return m_type; }

        /**
         * @brief Get crafting table name
         * @return Table name
         */
        virtual std::string GetName() const;

        /**
         * @brief Set crafting grid
         * @param grid New crafting grid
         */
        void SetCraftingGrid(const CraftingGrid& grid) { m_craftingGrid = grid; }

        /**
         * @brief Get current crafting grid
         * @return Current crafting grid
         */
        const CraftingGrid& GetCraftingGrid() const { return m_craftingGrid; }

        /**
         * @brief Add recipe to crafting table
         * @param recipe Recipe to add
         */
        void AddRecipe(std::shared_ptr<CraftingRecipe> recipe);

        /**
         * @brief Remove recipe from crafting table
         * @param recipeID Recipe ID to remove
         */
        void RemoveRecipe(int recipeID);

        /**
         * @brief Find matching recipe for current grid
         * @return Matching recipe, or nullptr if none found
         */
        std::shared_ptr<CraftingRecipe> FindMatchingRecipe() const;

        /**
         * @brief Get all available recipes
         * @return List of available recipes
         */
        std::vector<std::shared_ptr<CraftingRecipe>> GetAvailableRecipes() const;

        /**
         * @brief Attempt to craft current recipe
         * @param inventory Player inventory for ingredients/result
         * @return true if crafting was successful
         */
        virtual bool Craft(Inventory* inventory);

        /**
         * @brief Check if crafting is possible with current ingredients
         * @param inventory Player inventory
         * @return true if crafting is possible
         */
        virtual bool CanCraft(const Inventory* inventory) const;

        /**
         * @brief Get crafting result preview
         * @return Expected crafting result
         */
        virtual RecipeResult GetCraftingResult() const;

        /**
         * @brief Clear crafting grid
         */
        void ClearGrid();

        /**
         * @brief Get grid width
         * @return Grid width
         */
        int GetGridWidth() const { return m_craftingGrid.width; }

        /**
         * @brief Get grid height
         * @return Grid height
         */
        int GetGridHeight() const { return m_craftingGrid.height; }

        /**
         * @brief Set grid item at position
         * @param x X position
         * @param y Y position
         * @param ingredient Ingredient to place
         */
        void SetGridItem(int x, int y, const RecipeIngredient& ingredient);

        /**
         * @brief Get grid item at position
         * @param x X position
         * @param y Y position
         * @return Ingredient at position
         */
        RecipeIngredient GetGridItem(int x, int y) const;

    protected:
        CraftingTableType m_type;
        CraftingGrid m_craftingGrid;
        std::unordered_map<int, std::shared_ptr<CraftingRecipe>> m_recipes;

        /**
         * @brief Initialize default recipes
         */
        virtual void InitializeDefaultRecipes();

        /**
         * @brief Check if grid matches recipe
         * @param recipe Recipe to check
         * @return true if grid matches recipe
         */
        bool GridMatchesRecipe(const CraftingRecipe* recipe) const;

        /**
         * @brief Consume ingredients from inventory
         * @param recipe Recipe to consume ingredients for
         * @param inventory Player inventory
         * @return true if ingredients were consumed successfully
         */
        bool ConsumeIngredients(const CraftingRecipe* recipe, Inventory* inventory);

        /**
         * @brief Add result to inventory
         * @param result Recipe result
         * @param inventory Player inventory
         * @return true if result was added successfully
         */
        bool AddResultToInventory(const RecipeResult& result, Inventory* inventory);
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_CRAFTING_CRAFTING_TABLE_HPP
