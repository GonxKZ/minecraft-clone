/**
 * @file ShapelessRecipe.cpp
 * @brief VoxelCraft Shapeless Recipe Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "ShapelessRecipe.hpp"
#include <algorithm>
#include <sstream>

namespace VoxelCraft {

    ShapelessRecipe::ShapelessRecipe(const std::vector<RecipeIngredient>& ingredients,
                                   const RecipeResult& result)
        : CraftingRecipe(RecipeType::SHAPELESS, RecipeCategory::MISCELLANEOUS),
          m_ingredients(ingredients), m_result(result)
    {
        m_name = GenerateRecipeName();
    }

    ShapelessRecipe::ShapelessRecipe(const std::vector<RecipeIngredient>& ingredients,
                                   const RecipeResult& result,
                                   RecipeCategory category)
        : CraftingRecipe(RecipeType::SHAPELESS, category),
          m_ingredients(ingredients), m_result(result)
    {
        m_name = GenerateRecipeName();
    }

    std::string ShapelessRecipe::GetName() const {
        return m_name;
    }

    bool ShapelessRecipe::Matches(const std::vector<RecipeIngredient>& ingredients) const {
        return IngredientsMatch(ingredients, m_ingredients);
    }

    std::string ShapelessRecipe::GenerateRecipeName() const {
        std::stringstream ss;
        ss << "Shapeless Recipe: " << m_result.itemName;
        return ss.str();
    }

    bool ShapelessRecipe::IngredientsMatch(const std::vector<RecipeIngredient>& available,
                                          const std::vector<RecipeIngredient>& required) const {
        if (available.size() != required.size()) {
            return false;
        }

        // Create copies for counting
        std::vector<RecipeIngredient> availableCopy = available;
        std::vector<RecipeIngredient> requiredCopy = required;

        // Sort both lists by itemID for easier comparison
        auto sortById = [](const RecipeIngredient& a, const RecipeIngredient& b) {
            return a.itemID < b.itemID;
        };

        std::sort(availableCopy.begin(), availableCopy.end(), sortById);
        std::sort(requiredCopy.begin(), requiredCopy.end(), sortById);

        // Check if both lists have the same items with same counts
        for (size_t i = 0; i < requiredCopy.size(); ++i) {
            if (availableCopy[i].itemID != requiredCopy[i].itemID ||
                availableCopy[i].count < requiredCopy[i].count) {
                return false;
            }
        }

        return true;
    }

    namespace ShapelessRecipes {

        // Item IDs (simplified for this example)
        const int WHEAT = 296;
        const int COCOA_BEANS = 351;
        const int GOLD_INGOT = 266;
        const int APPLE = 260;
        const int PUMPKIN = 86;
        const int SUGAR = 353;
        const int EGG = 344;
        const int MILK_BUCKET = 335;
        const int BROWN_MUSHROOM = 39;
        const int RED_MUSHROOM = 40;
        const int RABBIT = 411;
        const int CARROT = 391;
        const int POTATO = 392;
        const int BEETROOT = 457;
        const int BOWL = 281;
        const int PAPER = 339;
        const int LEATHER_HELMET = 298;
        const int LEATHER_CHESTPLATE = 299;
        const int LEATHER_LEGGINGS = 300;
        const int LEATHER_BOOTS = 301;
        const int BONE = 352;
        const int GUNPOWDER = 289;
        const int DYE_WHITE = 351;
        const int DYE_ORANGE = 351;
        const int DYE_MAGENTA = 351;
        const int DYE_LIGHT_BLUE = 351;
        const int DYE_YELLOW = 351;
        const int DYE_LIME = 351;
        const int DYE_PINK = 351;
        const int DYE_GRAY = 351;
        const int DYE_LIGHT_GRAY = 351;
        const int DYE_CYAN = 351;
        const int DYE_PURPLE = 351;
        const int DYE_BLUE = 351;
        const int DYE_BROWN = 351;
        const int DYE_GREEN = 351;
        const int DYE_RED = 351;
        const int DYE_BLACK = 351;

        // Food recipes
        ShapelessRecipe BREAD(
            {RecipeIngredient(WHEAT, 3, "Wheat")},
            RecipeResult(297, 1, "Bread"),
            RecipeCategory::FOOD
        );

        ShapelessRecipe COOKIE(
            {RecipeIngredient(WHEAT, 2, "Wheat"), RecipeIngredient(COCOA_BEANS, 1, "Cocoa Beans")},
            RecipeResult(357, 8, "Cookie"),
            RecipeCategory::FOOD
        );

        ShapelessRecipe GOLDEN_APPLE(
            {RecipeIngredient(APPLE, 1, "Apple"), RecipeIngredient(GOLD_INGOT, 8, "Gold Ingot")},
            RecipeResult(322, 1, "Golden Apple"),
            RecipeCategory::FOOD
        );

        ShapelessRecipe PUMPKIN_PIE(
            {RecipeIngredient(PUMPKIN, 1, "Pumpkin"), RecipeIngredient(SUGAR, 1, "Sugar"),
             RecipeIngredient(EGG, 1, "Egg")},
            RecipeResult(400, 1, "Pumpkin Pie"),
            RecipeCategory::FOOD
        );

        ShapelessRecipe CAKE(
            {RecipeIngredient(WHEAT, 3, "Wheat"), RecipeIngredient(SUGAR, 2, "Sugar"),
             RecipeIngredient(EGG, 1, "Egg"), RecipeIngredient(MILK_BUCKET, 3, "Milk Bucket")},
            RecipeResult(354, 1, "Cake"),
            RecipeCategory::FOOD
        );

        ShapelessRecipe MUSHROOM_STEW(
            {RecipeIngredient(BROWN_MUSHROOM, 1, "Brown Mushroom"),
             RecipeIngredient(RED_MUSHROOM, 1, "Red Mushroom"), RecipeIngredient(BOWL, 1, "Bowl")},
            RecipeResult(282, 1, "Mushroom Stew"),
            RecipeCategory::FOOD
        );

        ShapelessRecipe RABBIT_STEW(
            {RecipeIngredient(RABBIT, 1, "Rabbit"), RecipeIngredient(CARROT, 1, "Carrot"),
             RecipeIngredient(POTATO, 1, "Potato"), RecipeIngredient(BROWN_MUSHROOM, 1, "Brown Mushroom"),
             RecipeIngredient(BOWL, 1, "Bowl")},
            RecipeResult(413, 1, "Rabbit Stew"),
            RecipeCategory::FOOD
        );

        ShapelessRecipe BEETROOT_SOUP(
            {RecipeIngredient(BEETROOT, 6, "Beetroot"), RecipeIngredient(BOWL, 1, "Bowl")},
            RecipeResult(459, 1, "Beetroot Soup"),
            RecipeCategory::FOOD
        );

        ShapelessRecipe SUSPICIOUS_STEW(
            {RecipeIngredient(RED_MUSHROOM, 1, "Red Mushroom"), RecipeIngredient(BROWN_MUSHROOM, 1, "Brown Mushroom"),
             RecipeIngredient(BOWL, 1, "Bowl")},
            RecipeResult(734, 1, "Suspicious Stew"),
            RecipeCategory::FOOD
        );

        // Dye recipes
        ShapelessRecipe LIGHT_GRAY_DYE(
            {RecipeIngredient(DYE_WHITE, 1, "White Dye"), RecipeIngredient(DYE_GRAY, 1, "Gray Dye")},
            RecipeResult(351, 2, "Light Gray Dye"),
            RecipeCategory::MISCELLANEOUS
        );

        ShapelessRecipe GRAY_DYE(
            {RecipeIngredient(DYE_WHITE, 1, "White Dye"), RecipeIngredient(BONE, 1, "Bone Meal")},
            RecipeResult(351, 1, "Gray Dye"),
            RecipeCategory::MISCELLANEOUS
        );

        ShapelessRecipe BLACK_DYE(
            {RecipeIngredient(DYE_WHITE, 1, "White Dye"), RecipeIngredient(BONE, 1, "Ink Sac")},
            RecipeResult(351, 1, "Black Dye"),
            RecipeCategory::MISCELLANEOUS
        );

        ShapelessRecipe BROWN_DYE(
            {RecipeIngredient(COCOA_BEANS, 1, "Cocoa Beans")},
            RecipeResult(351, 1, "Brown Dye"),
            RecipeCategory::MISCELLANEOUS
        );

        ShapelessRecipe BLUE_DYE(
            {RecipeIngredient(76, 1, "Lapis Lazuli")},
            RecipeResult(351, 1, "Blue Dye"),
            RecipeCategory::MISCELLANEOUS
        );

        ShapelessRecipe LIGHT_BLUE_DYE(
            {RecipeIngredient(DYE_WHITE, 1, "White Dye"), RecipeIngredient(DYE_BLUE, 1, "Blue Dye")},
            RecipeResult(351, 2, "Light Blue Dye"),
            RecipeCategory::MISCELLANEOUS
        );

        ShapelessRecipe CYAN_DYE(
            {RecipeIngredient(DYE_GREEN, 1, "Green Dye"), RecipeIngredient(DYE_BLUE, 1, "Blue Dye")},
            RecipeResult(351, 2, "Cyan Dye"),
            RecipeCategory::MISCELLANEOUS
        );

        ShapelessRecipe LIME_DYE(
            {RecipeIngredient(DYE_GREEN, 1, "Green Dye"), RecipeIngredient(BONE, 1, "Bone Meal")},
            RecipeResult(351, 2, "Lime Dye"),
            RecipeCategory::MISCELLANEOUS
        );

        ShapelessRecipe GREEN_DYE(
            {RecipeIngredient(COCOA_BEANS, 1, "Cactus Green")},
            RecipeResult(351, 1, "Green Dye"),
            RecipeCategory::MISCELLANEOUS
        );

        ShapelessRecipe YELLOW_DYE(
            {RecipeIngredient(37, 1, "Dandelion")},
            RecipeResult(351, 1, "Yellow Dye"),
            RecipeCategory::MISCELLANEOUS
        );

        ShapelessRecipe ORANGE_DYE(
            {RecipeIngredient(DYE_RED, 1, "Red Dye"), RecipeIngredient(DYE_YELLOW, 1, "Yellow Dye")},
            RecipeResult(351, 2, "Orange Dye"),
            RecipeCategory::MISCELLANEOUS
        );

        ShapelessRecipe RED_DYE(
            {RecipeIngredient(38, 1, "Rose")},
            RecipeResult(351, 1, "Red Dye"),
            RecipeCategory::MISCELLANEOUS
        );

        ShapelessRecipe PINK_DYE(
            {RecipeIngredient(DYE_RED, 1, "Red Dye"), RecipeIngredient(BONE, 1, "Bone Meal")},
            RecipeResult(351, 2, "Pink Dye"),
            RecipeCategory::MISCELLANEOUS
        );

        ShapelessRecipe MAGENTA_DYE(
            {RecipeIngredient(DYE_PURPLE, 1, "Purple Dye"), RecipeIngredient(DYE_PINK, 1, "Pink Dye")},
            RecipeResult(351, 2, "Magenta Dye"),
            RecipeCategory::MISCELLANEOUS
        );

        ShapelessRecipe PURPLE_DYE(
            {RecipeIngredient(DYE_RED, 1, "Red Dye"), RecipeIngredient(DYE_BLUE, 1, "Blue Dye")},
            RecipeResult(351, 2, "Purple Dye"),
            RecipeCategory::MISCELLANEOUS
        );

        ShapelessRecipe WHITE_DYE(
            {RecipeIngredient(BONE, 1, "Bone Meal")},
            RecipeResult(351, 1, "White Dye"),
            RecipeCategory::MISCELLANEOUS
        );

        // Firework recipes
        ShapelessRecipe FIREWORK_ROCKET(
            {RecipeIngredient(PAPER, 1, "Paper"), RecipeIngredient(GUNPOWDER, 1, "Gunpowder")},
            RecipeResult(401, 3, "Firework Rocket"),
            RecipeCategory::MISCELLANEOUS
        );

        ShapelessRecipe FIREWORK_STAR(
            {RecipeIngredient(GUNPOWDER, 1, "Gunpowder"), RecipeIngredient(DYE_WHITE, 1, "Dye")},
            RecipeResult(402, 1, "Firework Star"),
            RecipeCategory::MISCELLANEOUS
        );

        // Helper functions
        std::vector<std::shared_ptr<ShapelessRecipe>> GetAllShapelessRecipes() {
            return {
                std::make_shared<ShapelessRecipe>(BREAD),
                std::make_shared<ShapelessRecipe>(COOKIE),
                std::make_shared<ShapelessRecipe>(GOLDEN_APPLE),
                std::make_shared<ShapelessRecipe>(PUMPKIN_PIE),
                std::make_shared<ShapelessRecipe>(CAKE),
                std::make_shared<ShapelessRecipe>(MUSHROOM_STEW),
                std::make_shared<ShapelessRecipe>(RABBIT_STEW),
                std::make_shared<ShapelessRecipe>(BEETROOT_SOUP),
                std::make_shared<ShapelessRecipe>(SUSPICIOUS_STEW),
                // Dye recipes
                std::make_shared<ShapelessRecipe>(LIGHT_GRAY_DYE),
                std::make_shared<ShapelessRecipe>(GRAY_DYE),
                std::make_shared<ShapelessRecipe>(BLACK_DYE),
                std::make_shared<ShapelessRecipe>(BROWN_DYE),
                std::make_shared<ShapelessRecipe>(BLUE_DYE),
                std::make_shared<ShapelessRecipe>(LIGHT_BLUE_DYE),
                std::make_shared<ShapelessRecipe>(CYAN_DYE),
                std::make_shared<ShapelessRecipe>(LIME_DYE),
                std::make_shared<ShapelessRecipe>(GREEN_DYE),
                std::make_shared<ShapelessRecipe>(YELLOW_DYE),
                std::make_shared<ShapelessRecipe>(ORANGE_DYE),
                std::make_shared<ShapelessRecipe>(RED_DYE),
                std::make_shared<ShapelessRecipe>(PINK_DYE),
                std::make_shared<ShapelessRecipe>(MAGENTA_DYE),
                std::make_shared<ShapelessRecipe>(PURPLE_DYE),
                std::make_shared<ShapelessRecipe>(WHITE_DYE),
                // Firework recipes
                std::make_shared<ShapelessRecipe>(FIREWORK_ROCKET),
                std::make_shared<ShapelessRecipe>(FIREWORK_STAR)
            };
        }

        std::vector<std::shared_ptr<ShapelessRecipe>> GetRecipesByCategory(RecipeCategory category) {
            auto allRecipes = GetAllShapelessRecipes();
            std::vector<std::shared_ptr<ShapelessRecipe>> filtered;

            for (const auto& recipe : allRecipes) {
                if (recipe->GetCategory() == category) {
                    filtered.push_back(recipe);
                }
            }

            return filtered;
        }

    } // namespace ShapelessRecipes

} // namespace VoxelCraft
