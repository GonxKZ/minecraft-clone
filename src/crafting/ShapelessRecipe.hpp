/**
 * @file ShapelessRecipe.hpp
 * @brief VoxelCraft Shapeless Recipe Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_CRAFTING_SHAPELESS_RECIPE_HPP
#define VOXELCRAFT_CRAFTING_SHAPELESS_RECIPE_HPP

#include "CraftingRecipe.hpp"
#include <vector>
#include <unordered_map>

namespace VoxelCraft {

    /**
     * @class ShapelessRecipe
     * @brief Recipe that doesn't require specific shape, only ingredients
     */
    class ShapelessRecipe : public CraftingRecipe {
    public:
        /**
         * @brief Constructor
         * @param ingredients List of required ingredients
         * @param result Recipe result
         */
        ShapelessRecipe(const std::vector<RecipeIngredient>& ingredients,
                       const RecipeResult& result);

        /**
         * @brief Constructor with category
         * @param ingredients List of required ingredients
         * @param result Recipe result
         * @param category Recipe category
         */
        ShapelessRecipe(const std::vector<RecipeIngredient>& ingredients,
                       const RecipeResult& result,
                       RecipeCategory category);

        ~ShapelessRecipe() override = default;

        /**
         * @brief Get recipe name
         * @return Recipe name
         */
        std::string GetName() const override;

        /**
         * @brief Check if recipe matches given ingredients
         * @param ingredients Available ingredients
         * @return true if recipe can be crafted
         */
        bool Matches(const std::vector<RecipeIngredient>& ingredients) const override;

        /**
         * @brief Get crafting result
         * @return Recipe result
         */
        RecipeResult GetResult() const override { return m_result; }

        /**
         * @brief Get required ingredients
         * @return List of required ingredients
         */
        std::vector<RecipeIngredient> GetRequiredIngredients() const override { return m_ingredients; }

        /**
         * @brief Get recipe width
         * @return Recipe width (not applicable for shapeless)
         */
        int GetWidth() const override { return 3; }

        /**
         * @brief Get recipe height
         * @return Recipe height (not applicable for shapeless)
         */
        int GetHeight() const override { return 3; }

    private:
        std::vector<RecipeIngredient> m_ingredients;
        RecipeResult m_result;
        std::string m_name;

        /**
         * @brief Generate recipe name from ingredients and result
         * @return Generated name
         */
        std::string GenerateRecipeName() const;

        /**
         * @brief Check if ingredient lists match (order doesn't matter)
         * @param available Available ingredients
         * @param required Required ingredients
         * @return true if ingredients match
         */
        bool IngredientsMatch(const std::vector<RecipeIngredient>& available,
                             const std::vector<RecipeIngredient>& required) const;
    };

    // Common Minecraft shapeless recipes
    namespace ShapelessRecipes {

        // Food recipes
        extern ShapelessRecipe BREAD;
        extern ShapelessRecipe COOKIE;
        extern ShapelessRecipe GOLDEN_APPLE;
        extern ShapelessRecipe PUMPKIN_PIE;
        extern ShapelessRecipe CAKE;
        extern ShapelessRecipe MUSHROOM_STEW;
        extern ShapelessRecipe RABBIT_STEW;
        extern ShapelessRecipe BEETROOT_SOUP;
        extern ShapelessRecipe SUSPICIOUS_STEW;

        // Dye recipes
        extern ShapelessRecipe LIGHT_GRAY_DYE;
        extern ShapelessRecipe GRAY_DYE;
        extern ShapelessRecipe BLACK_DYE;
        extern ShapelessRecipe BROWN_DYE;
        extern ShapelessRecipe BLUE_DYE;
        extern ShapelessRecipe LIGHT_BLUE_DYE;
        extern ShapelessRecipe CYAN_DYE;
        extern ShapelessRecipe LIME_DYE;
        extern ShapelessRecipe GREEN_DYE;
        extern ShapelessRecipe YELLOW_DYE;
        extern ShapelessRecipe ORANGE_DYE;
        extern ShapelessRecipe RED_DYE;
        extern ShapelessRecipe PINK_DYE;
        extern ShapelessRecipe MAGENTA_DYE;
        extern ShapelessRecipe PURPLE_DYE;
        extern ShapelessRecipe WHITE_DYE;

        // Firework recipes
        extern ShapelessRecipe FIREWORK_ROCKET;
        extern ShapelessRecipe FIREWORK_STAR;

        // Miscellaneous
        extern ShapelessRecipe CHEST_MINECART;
        extern ShapelessRecipe FURNACE_MINECART;
        extern ShapelessRecipe TNT_MINECART;
        extern ShapelessRecipe HOPPER_MINECART;
        extern ShapelessRecipe BOOK;
        extern ShapelessRecipe WRITABLE_BOOK;
        extern ShapelessRecipe MAP;
        extern ShapelessRecipe FIRE_CHARGE;
        extern ShapelessRecipe IRON_NUGGET_FROM_INGOT;
        extern ShapelessRecipe GOLD_NUGGET_FROM_INGOT;
        extern ShapelessRecipe FERMENTED_SPIDER_EYE;

        // Armor dyeing
        extern ShapelessRecipe DYE_LEATHER_HELMET;
        extern ShapelessRecipe DYE_LEATHER_CHESTPLATE;
        extern ShapelessRecipe DYE_LEATHER_LEGGINGS;
        extern ShapelessRecipe DYE_LEATHER_BOOTS;

        // Banner patterns
        extern ShapelessRecipe CREEPER_BANNER_PATTERN;
        extern ShapelessRecipe SKULL_BANNER_PATTERN;
        extern ShapelessRecipe FLOWER_BANNER_PATTERN;
        extern ShapelessRecipe MOJANG_BANNER_PATTERN;

        // Helper functions
        std::vector<std::shared_ptr<ShapelessRecipe>> GetAllShapelessRecipes();
        std::vector<std::shared_ptr<ShapelessRecipe>> GetRecipesByCategory(RecipeCategory category);

    } // namespace ShapelessRecipes

} // namespace VoxelCraft

#endif // VOXELCRAFT_CRAFTING_SHAPELESS_RECIPE_HPP
