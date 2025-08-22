/**
 * @file CraftingRecipe.hpp
 * @brief VoxelCraft Crafting System - Recipe Definitions
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_CRAFTING_CRAFTING_RECIPE_HPP
#define VOXELCRAFT_CRAFTING_CRAFTING_RECIPE_HPP

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace VoxelCraft {

    class Block;

    /**
     * @enum RecipeType
     * @brief Types of crafting recipes
     */
    enum class RecipeType {
        SHAPED = 0,      ///< Recipe requires specific shape/pattern
        SHAPELESS,       ///< Recipe only requires ingredients (no shape)
        SMELTING,        ///< Furnace smelting recipe
        BREWING,         ///< Brewing stand recipe
        SMITHING         ///< Smithing table recipe
    };

    /**
     * @enum RecipeCategory
     * @brief Categories for organizing recipes
     */
    enum class RecipeCategory {
        BUILDING_BLOCKS = 0,
        DECORATION,
        REDSTONE,
        TRANSPORTATION,
        MISCELLANEOUS,
        FOOD,
        TOOLS,
        COMBAT,
        BREWING
    };

    /**
     * @struct RecipeIngredient
     * @brief Represents an ingredient in a crafting recipe
     */
    struct RecipeIngredient {
        int itemID;              ///< Item/block ID
        int count;               ///< Required count
        std::string itemName;    ///< Item name for display

        RecipeIngredient(int id = 0, int cnt = 1, const std::string& name = "")
            : itemID(id), count(cnt), itemName(name) {}
    };

    /**
     * @struct RecipeResult
     * @brief Result of a crafting recipe
     */
    struct RecipeResult {
        int itemID;              ///< Result item/block ID
        int count;               ///< Result count
        std::string itemName;    ///< Result item name

        RecipeResult(int id = 0, int cnt = 1, const std::string& name = "")
            : itemID(id), count(cnt), itemName(name) {}
    };

    /**
     * @class CraftingRecipe
     * @brief Represents a crafting recipe
     */
    class CraftingRecipe {
    public:
        /**
         * @brief Constructor
         * @param type Recipe type
         * @param category Recipe category
         */
        CraftingRecipe(RecipeType type = RecipeType::SHAPED,
                      RecipeCategory category = RecipeCategory::MISCELLANEOUS);

        /**
         * @brief Destructor
         */
        virtual ~CraftingRecipe();

        /**
         * @brief Get recipe type
         * @return Recipe type
         */
        RecipeType GetType() const { return m_type; }

        /**
         * @brief Get recipe category
         * @return Recipe category
         */
        RecipeCategory GetCategory() const { return m_category; }

        /**
         * @brief Get recipe name
         * @return Recipe name
         */
        virtual std::string GetName() const = 0;

        /**
         * @brief Check if recipe matches given ingredients
         * @param ingredients Available ingredients
         * @return true if recipe can be crafted
         */
        virtual bool Matches(const std::vector<RecipeIngredient>& ingredients) const = 0;

        /**
         * @brief Get crafting result
         * @return Recipe result
         */
        virtual RecipeResult GetResult() const = 0;

        /**
         * @brief Get required ingredients
         * @return List of required ingredients
         */
        virtual std::vector<RecipeIngredient> GetRequiredIngredients() const = 0;

        /**
         * @brief Get recipe width (for shaped recipes)
         * @return Recipe width
         */
        virtual int GetWidth() const { return 3; }

        /**
         * @brief Get recipe height (for shaped recipes)
         * @return Recipe height
         */
        virtual int GetHeight() const { return 3; }

        /**
         * @brief Check if recipe is enabled
         * @return true if recipe is enabled
         */
        bool IsEnabled() const { return m_enabled; }

        /**
         * @brief Enable or disable recipe
         * @param enabled Whether recipe should be enabled
         */
        void SetEnabled(bool enabled) { m_enabled = enabled; }

        /**
         * @brief Get recipe ID
         * @return Recipe ID
         */
        int GetRecipeID() const { return m_recipeID; }

        /**
         * @brief Set recipe ID
         * @param id Recipe ID
         */
        void SetRecipeID(int id) { m_recipeID = id; }

    protected:
        RecipeType m_type;
        RecipeCategory m_category;
        bool m_enabled;
        int m_recipeID;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_CRAFTING_CRAFTING_RECIPE_HPP
