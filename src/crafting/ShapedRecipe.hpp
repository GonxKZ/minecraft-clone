#pragma once

#include "CraftingRecipe.hpp"
#include <vector>
#include <array>

namespace VoxelCraft {

    /**
     * @class ShapedRecipe
     * @brief Recipe that requires specific shape/pattern
     */
    class ShapedRecipe : public CraftingRecipe {
    public:
        static const int MAX_WIDTH = 3;
        static const int MAX_HEIGHT = 3;

        using Pattern = std::array<std::array<int, MAX_WIDTH>, MAX_HEIGHT>;
        using KeyMap = std::unordered_map<char, RecipeIngredient>;

        /**
         * @brief Constructor
         * @param pattern 3x3 crafting pattern
         * @param key Key mapping for pattern symbols
         * @param result Recipe result
         */
        ShapedRecipe(const std::vector<std::string>& pattern,
                    const KeyMap& key,
                    const RecipeResult& result);

        /**
         * @brief Constructor with category
         */
        ShapedRecipe(const std::vector<std::string>& pattern,
                    const KeyMap& key,
                    const RecipeResult& result,
                    RecipeCategory category);

        ~ShapedRecipe() override = default;

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
        std::vector<RecipeIngredient> GetRequiredIngredients() const override;

        /**
         * @brief Get recipe width
         * @return Recipe width
         */
        int GetWidth() const override { return m_width; }

        /**
         * @brief Get recipe height
         * @return Recipe height
         */
        int GetHeight() const override { return m_height; }

    private:
        Pattern m_pattern;
        KeyMap m_key;
        RecipeResult m_result;
        int m_width;
        int m_height;
        std::string m_name;

        /**
         * @brief Initialize pattern from string vector
         * @param pattern String pattern
         */
        void InitializePattern(const std::vector<std::string>& pattern);

        /**
         * @brief Check if grid matches pattern
         * @param grid Crafting grid
         * @return true if matches
         */
        bool GridMatchesPattern(const CraftingGrid& grid) const;

        /**
         * @brief Generate recipe name from pattern and result
         * @return Generated name
         */
        std::string GenerateRecipeName() const;
    };

    // Common Minecraft shaped recipes
    namespace Recipes {

        // Tools
        extern ShapedRecipe WOODEN_PICKAXE;
        extern ShapedRecipe STONE_PICKAXE;
        extern ShapedRecipe IRON_PICKAXE;
        extern ShapedRecipe DIAMOND_PICKAXE;
        extern ShapedRecipe WOODEN_AXE;
        extern ShapedRecipe STONE_AXE;
        extern ShapedRecipe IRON_AXE;
        extern ShapedRecipe DIAMOND_AXE;
        extern ShapedRecipe WOODEN_SHOVEL;
        extern ShapedRecipe STONE_SHOVEL;
        extern ShapedRecipe IRON_SHOVEL;
        extern ShapedRecipe DIAMOND_SHOVEL;
        extern ShapedRecipe WOODEN_SWORD;
        extern ShapedRecipe STONE_SWORD;
        extern ShapedRecipe IRON_SWORD;
        extern ShapedRecipe DIAMOND_SWORD;

        // Armor
        extern ShapedRecipe LEATHER_HELMET;
        extern ShapedRecipe LEATHER_CHESTPLATE;
        extern ShapedRecipe LEATHER_LEGGINGS;
        extern ShapedRecipe LEATHER_BOOTS;
        extern ShapedRecipe IRON_HELMET;
        extern ShapedRecipe IRON_CHESTPLATE;
        extern ShapedRecipe IRON_LEGGINGS;
        extern ShapedRecipe IRON_BOOTS;
        extern ShapedRecipe DIAMOND_HELMET;
        extern ShapedRecipe DIAMOND_CHESTPLATE;
        extern ShapedRecipe DIAMOND_LEGGINGS;
        extern ShapedRecipe DIAMOND_BOOTS;

        // Building blocks
        extern ShapedRecipe WOODEN_PLANKS;
        extern ShapedRecipe STONE_BRICKS;
        extern ShapedRecipe BRICKS;
        extern ShapedRecipe GLASS_PANE;
        extern ShapedRecipe STICK;
        extern ShapedRecipe TORCH;
        extern ShapedRecipe CRAFTING_TABLE;
        extern ShapedRecipe FURNACE;
        extern ShapedRecipe CHEST;
        extern ShapedRecipe BED;
        extern ShapedRecipe DOOR;
        extern ShapedRecipe FENCE;
        extern ShapedRecipe STAIRS;
        extern ShapedRecipe SLAB;

        // Transportation
        extern ShapedRecipe BOAT;
        extern ShapedRecipe MINECART;
        extern ShapedRecipe RAIL;
        extern ShapedRecipe POWERED_RAIL;
        extern ShapedRecipe DETECTOR_RAIL;

        // Redstone
        extern ShapedRecipe REDSTONE_TORCH;
        extern ShapedRecipe REPEATER;
        extern ShapedRecipe COMPARATOR;
        extern ShapedRecipe PISTON;
        extern ShapedRecipe STICKY_PISTON;
        extern ShapedRecipe LEVER;
        extern ShapedRecipe BUTTON;
        extern ShapedRecipe PRESSURE_PLATE;

        // Food
        extern ShapedRecipe BREAD;
        extern ShapedRecipe CAKE;
        extern ShapedRecipe COOKIE;
        extern ShapedRecipe PUMPKIN_PIE;

        // Miscellaneous
        extern ShapedRecipe BOWL;
        extern ShapedRecipe BUCKET;
        extern ShapedRecipe SHEARS;
        extern ShapedRecipe FISHING_ROD;
        extern ShapedRecipe CARROT_ON_A_STICK;
        extern ShapedRecipe FLINT_AND_STEEL;
        extern ShapedRecipe BOOK;
        extern ShapedRecipe BOOKSHELF;
        extern ShapedRecipe PAINTING;
        extern ShapedRecipe ITEM_FRAME;
        extern ShapedRecipe SIGN;

        // Helper functions
        std::vector<std::shared_ptr<ShapedRecipe>> GetAllShapedRecipes();
        std::vector<std::shared_ptr<ShapedRecipe>> GetRecipesByCategory(RecipeCategory category);

    } // namespace Recipes

} // namespace VoxelCraft
