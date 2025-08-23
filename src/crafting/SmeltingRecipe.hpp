/**
 * @file SmeltingRecipe.hpp
 * @brief VoxelCraft Smelting Recipe Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_CRAFTING_SMELTING_RECIPE_HPP
#define VOXELCRAFT_CRAFTING_SMELTING_RECIPE_HPP

#include "CraftingRecipe.hpp"

namespace VoxelCraft {

    /**
     * @enum SmeltingType
     * @brief Types of smelting operations
     */
    enum class SmeltingType {
        FURNACE = 0,    ///< Regular furnace smelting
        BLAST_FURNACE,  ///< Blast furnace (ores only)
        SMOKER         ///< Smoker (food only)
    };

    /**
     * @class SmeltingRecipe
     * @brief Recipe for smelting/furnace operations
     */
    class SmeltingRecipe : public CraftingRecipe {
    public:
        /**
         * @brief Constructor
         * @param input Input ingredient
         * @param result Output result
         * @param cookingTime Time to cook in ticks
         * @param experience Experience gained
         */
        SmeltingRecipe(const RecipeIngredient& input,
                      const RecipeResult& result,
                      int cookingTime = 200,
                      float experience = 0.1f);

        /**
         * @brief Constructor with smelting type
         * @param input Input ingredient
         * @param result Output result
         * @param type Smelting type
         * @param cookingTime Time to cook in ticks
         * @param experience Experience gained
         */
        SmeltingRecipe(const RecipeIngredient& input,
                      const RecipeResult& result,
                      SmeltingType type,
                      int cookingTime = 200,
                      float experience = 0.1f);

        ~SmeltingRecipe() override = default;

        /**
         * @brief Get recipe name
         * @return Recipe name
         */
        std::string GetName() const override;

        /**
         * @brief Check if recipe matches given ingredients
         * @param ingredients Available ingredients
         * @return true if recipe can be smelted
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
         * @brief Get cooking time
         * @return Cooking time in ticks
         */
        int GetCookingTime() const { return m_cookingTime; }

        /**
         * @brief Get experience gained
         * @return Experience amount
         */
        float GetExperience() const { return m_experience; }

        /**
         * @brief Get smelting type
         * @return Smelting type
         */
        SmeltingType GetSmeltingType() const { return m_smeltingType; }

        /**
         * @brief Get recipe width
         * @return Recipe width (1 for smelting)
         */
        int GetWidth() const override { return 1; }

        /**
         * @brief Get recipe height
         * @return Recipe height (1 for smelting)
         */
        int GetHeight() const override { return 1; }

    private:
        RecipeIngredient m_input;
        RecipeResult m_result;
        SmeltingType m_smeltingType;
        int m_cookingTime;
        float m_experience;
        std::string m_name;

        /**
         * @brief Generate recipe name from input and result
         * @return Generated name
         */
        std::string GenerateRecipeName() const;
    };

    // Common Minecraft smelting recipes
    namespace SmeltingRecipes {

        // Ore smelting
        extern SmeltingRecipe IRON_INGOT;
        extern SmeltingRecipe GOLD_INGOT;
        extern SmeltingRecipe COPPER_INGOT;
        extern SmeltingRecipe NETHERITE_INGOT;
        extern SmeltingRecipe DIAMOND;
        extern SmeltingRecipe EMERALD;
        extern SmeltingRecipe LAPIS_LAZULI;
        extern SmeltingRecipe REDSTONE;
        extern SmeltingRecipe COAL;
        extern SmeltingRecipe QUARTZ;

        // Food smelting
        extern SmeltingRecipe COOKED_BEEF;
        extern SmeltingRecipe COOKED_PORKCHOP;
        extern SmeltingRecipe COOKED_CHICKEN;
        extern SmeltingRecipe COOKED_RABBIT;
        extern SmeltingRecipe COOKED_MUTTON;
        extern SmeltingRecipe COOKED_SALMON;
        extern SmeltingRecipe COOKED_COD;
        extern SmeltingRecipe BAKED_POTATO;
        extern SmeltingRecipe DRIED_KELP;

        // Block smelting
        extern SmeltingRecipe GLASS;
        extern SmeltingRecipe STONE;
        extern SmeltingRecipe SMOOTH_STONE;
        extern SmeltingRecipe BRICKS;
        extern SmeltingRecipe NETHER_BRICKS;
        extern SmeltingRecipe CRACKED_NETHER_BRICKS;
        extern SmeltingRecipe CRACKED_STONE_BRICKS;
        extern SmeltingRecipe CRACKED_POLISHED_BLACKSTONE_BRICKS;
        extern SmeltingRecipe GREEN_DYE_FROM_CACTUS;
        extern SmeltingRecipe CHARCOAL;
        extern SmeltingRecipe POPPED_CHORUS_FRUIT;

        // Blast furnace only
        extern SmeltingRecipe IRON_INGOT_BLAST;
        extern SmeltingRecipe GOLD_INGOT_BLAST;

        // Smoker only
        extern SmeltingRecipe COOKED_BEEF_SMOKER;
        extern SmeltingRecipe COOKED_PORKCHOP_SMOKER;

        // Helper functions
        std::vector<std::shared_ptr<SmeltingRecipe>> GetAllSmeltingRecipes();
        std::vector<std::shared_ptr<SmeltingRecipe>> GetRecipesBySmeltingType(SmeltingType type);
        std::vector<std::shared_ptr<SmeltingRecipe>> GetRecipesByCategory(RecipeCategory category);

    } // namespace SmeltingRecipes

} // namespace VoxelCraft

#endif // VOXELCRAFT_CRAFTING_SMELTING_RECIPE_HPP
