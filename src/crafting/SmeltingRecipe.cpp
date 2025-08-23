/**
 * @file SmeltingRecipe.cpp
 * @brief VoxelCraft Smelting Recipe Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "SmeltingRecipe.hpp"
#include <sstream>

namespace VoxelCraft {

    SmeltingRecipe::SmeltingRecipe(const RecipeIngredient& input,
                                 const RecipeResult& result,
                                 int cookingTime,
                                 float experience)
        : CraftingRecipe(RecipeType::SMELTING, RecipeCategory::MISCELLANEOUS),
          m_input(input), m_result(result), m_smeltingType(SmeltingType::FURNACE),
          m_cookingTime(cookingTime), m_experience(experience)
    {
        m_name = GenerateRecipeName();
    }

    SmeltingRecipe::SmeltingRecipe(const RecipeIngredient& input,
                                 const RecipeResult& result,
                                 SmeltingType type,
                                 int cookingTime,
                                 float experience)
        : CraftingRecipe(RecipeType::SMELTING, RecipeCategory::MISCELLANEOUS),
          m_input(input), m_result(result), m_smeltingType(type),
          m_cookingTime(cookingTime), m_experience(experience)
    {
        m_name = GenerateRecipeName();
    }

    std::string SmeltingRecipe::GetName() const {
        return m_name;
    }

    bool SmeltingRecipe::Matches(const std::vector<RecipeIngredient>& ingredients) const {
        if (ingredients.size() != 1) {
            return false;
        }

        const auto& ingredient = ingredients[0];
        return ingredient.itemID == m_input.itemID && ingredient.count >= m_input.count;
    }

    std::vector<RecipeIngredient> SmeltingRecipe::GetRequiredIngredients() const {
        return {m_input};
    }

    std::string SmeltingRecipe::GenerateRecipeName() const {
        std::stringstream ss;
        ss << "Smelting Recipe: " << m_input.itemName << " -> " << m_result.itemName;
        return ss.str();
    }

    namespace SmeltingRecipes {

        // Item IDs (simplified for this example)
        const int IRON_ORE = 15;
        const int GOLD_ORE = 14;
        const int COPPER_ORE = 594;
        const int NETHERITE_SCRAP = 752;
        const int DIAMOND_ORE = 56;
        const int EMERALD_ORE = 129;
        const int LAPIS_ORE = 21;
        const int REDSTONE_ORE = 73;
        const int COAL_ORE = 16;
        const int NETHER_QUARTZ_ORE = 153;
        const int IRON_INGOT = 265;
        const int GOLD_INGOT = 266;
        const int COPPER_INGOT = 601;
        const int NETHERITE_INGOT = 742;
        const int DIAMOND = 264;
        const int EMERALD = 388;
        const int LAPIS_LAZULI = 351;
        const int REDSTONE = 331;
        const int COAL = 263;
        const int QUARTZ = 406;

        // Raw meats
        const int BEEF = 363;
        const int PORKCHOP = 319;
        const int CHICKEN = 365;
        const int RABBIT = 411;
        const int MUTTON = 423;
        const int SALMON = 349;
        const int COD = 349;
        const int POTATO = 392;
        const int KELP = 335;

        // Cooked foods
        const int COOKED_BEEF = 364;
        const int COOKED_PORKCHOP = 320;
        const int COOKED_CHICKEN = 366;
        const int COOKED_RABBIT = 412;
        const int COOKED_MUTTON = 424;
        const int COOKED_SALMON = 350;
        const int COOKED_COD = 350;
        const int BAKED_POTATO = 393;
        const int DRIED_KELP = 464;

        // Blocks
        const int SAND = 12;
        const int COBBLESTONE = 4;
        const int STONE = 1;
        const int CLAY = 337;
        const int NETHERRACK = 87;
        const int NETHER_BRICKS_ITEM = 405;
        const int STONE_BRICKS = 98;
        const int BLACKSTONE = 528;
        const int CACTUS = 81;
        const int LOG = 17;
        const int CHORUS_FRUIT = 432;

        // Results
        const int GLASS = 20;
        const int SMOOTH_STONE = 43;
        const int BRICK = 336;
        const int NETHER_BRICK = 405;
        const int CRACKED_NETHER_BRICKS = 405;
        const int CRACKED_STONE_BRICKS = 98;
        const int CRACKED_POLISHED_BLACKSTONE_BRICKS = 534;
        const int GREEN_DYE = 351;
        const int CHARCOAL = 263;
        const int POPPED_CHORUS_FRUIT = 433;

        // Ore smelting recipes
        SmeltingRecipe IRON_INGOT(
            RecipeIngredient(IRON_ORE, 1, "Iron Ore"),
            RecipeResult(IRON_INGOT, 1, "Iron Ingot"),
            SmeltingType::FURNACE,
            200, 0.7f
        );

        SmeltingRecipe GOLD_INGOT(
            RecipeIngredient(GOLD_ORE, 1, "Gold Ore"),
            RecipeResult(GOLD_INGOT, 1, "Gold Ingot"),
            SmeltingType::FURNACE,
            200, 1.0f
        );

        SmeltingRecipe COPPER_INGOT(
            RecipeIngredient(COPPER_ORE, 1, "Copper Ore"),
            RecipeResult(COPPER_INGOT, 1, "Copper Ingot"),
            SmeltingType::FURNACE,
            200, 0.7f
        );

        SmeltingRecipe NETHERITE_INGOT(
            RecipeIngredient(NETHERITE_SCRAP, 4, "Netherite Scrap"),
            RecipeResult(NETHERITE_INGOT, 1, "Netherite Ingot"),
            SmeltingType::FURNACE,
            200, 2.0f
        );

        SmeltingRecipe DIAMOND(
            RecipeIngredient(DIAMOND_ORE, 1, "Diamond Ore"),
            RecipeResult(DIAMOND, 1, "Diamond"),
            SmeltingType::FURNACE,
            200, 1.0f
        );

        SmeltingRecipe EMERALD(
            RecipeIngredient(EMERALD_ORE, 1, "Emerald Ore"),
            RecipeResult(EMERALD, 1, "Emerald"),
            SmeltingType::FURNACE,
            200, 1.0f
        );

        SmeltingRecipe LAPIS_LAZULI(
            RecipeIngredient(LAPIS_ORE, 1, "Lapis Lazuli Ore"),
            RecipeResult(LAPIS_LAZULI, 6, "Lapis Lazuli"),
            SmeltingType::FURNACE,
            200, 0.2f
        );

        SmeltingRecipe REDSTONE(
            RecipeIngredient(REDSTONE_ORE, 1, "Redstone Ore"),
            RecipeResult(REDSTONE, 4, "Redstone"),
            SmeltingType::FURNACE,
            200, 0.7f
        );

        SmeltingRecipe COAL(
            RecipeIngredient(COAL_ORE, 1, "Coal Ore"),
            RecipeResult(COAL, 1, "Coal"),
            SmeltingType::FURNACE,
            200, 0.1f
        );

        SmeltingRecipe QUARTZ(
            RecipeIngredient(NETHER_QUARTZ_ORE, 1, "Nether Quartz Ore"),
            RecipeResult(QUARTZ, 1, "Quartz"),
            SmeltingType::FURNACE,
            200, 0.2f
        );

        // Food smelting recipes
        SmeltingRecipe COOKED_BEEF(
            RecipeIngredient(BEEF, 1, "Raw Beef"),
            RecipeResult(COOKED_BEEF, 1, "Steak"),
            SmeltingType::FURNACE,
            200, 0.35f
        );

        SmeltingRecipe COOKED_PORKCHOP(
            RecipeIngredient(PORKCHOP, 1, "Raw Porkchop"),
            RecipeResult(COOKED_PORKCHOP, 1, "Cooked Porkchop"),
            SmeltingType::FURNACE,
            200, 0.35f
        );

        SmeltingRecipe COOKED_CHICKEN(
            RecipeIngredient(CHICKEN, 1, "Raw Chicken"),
            RecipeResult(COOKED_CHICKEN, 1, "Cooked Chicken"),
            SmeltingType::FURNACE,
            200, 0.35f
        );

        SmeltingRecipe COOKED_RABBIT(
            RecipeIngredient(RABBIT, 1, "Raw Rabbit"),
            RecipeResult(COOKED_RABBIT, 1, "Cooked Rabbit"),
            SmeltingType::FURNACE,
            200, 0.35f
        );

        SmeltingRecipe COOKED_MUTTON(
            RecipeIngredient(MUTTON, 1, "Raw Mutton"),
            RecipeResult(COOKED_MUTTON, 1, "Cooked Mutton"),
            SmeltingType::FURNACE,
            200, 0.35f
        );

        SmeltingRecipe COOKED_SALMON(
            RecipeIngredient(SALMON, 1, "Raw Salmon"),
            RecipeResult(COOKED_SALMON, 1, "Cooked Salmon"),
            SmeltingType::FURNACE,
            200, 0.35f
        );

        SmeltingRecipe COOKED_COD(
            RecipeIngredient(COD, 1, "Raw Cod"),
            RecipeResult(COOKED_COD, 1, "Cooked Cod"),
            SmeltingType::FURNACE,
            200, 0.35f
        );

        SmeltingRecipe BAKED_POTATO(
            RecipeIngredient(POTATO, 1, "Potato"),
            RecipeResult(BAKED_POTATO, 1, "Baked Potato"),
            SmeltingType::FURNACE,
            200, 0.35f
        );

        SmeltingRecipe DRIED_KELP(
            RecipeIngredient(KELP, 1, "Kelp"),
            RecipeResult(DRIED_KELP, 1, "Dried Kelp"),
            SmeltingType::FURNACE,
            200, 0.1f
        );

        // Block smelting recipes
        SmeltingRecipe GLASS(
            RecipeIngredient(SAND, 1, "Sand"),
            RecipeResult(GLASS, 1, "Glass"),
            SmeltingType::FURNACE,
            200, 0.1f
        );

        SmeltingRecipe STONE(
            RecipeIngredient(COBBLESTONE, 1, "Cobblestone"),
            RecipeResult(STONE, 1, "Stone"),
            SmeltingType::FURNACE,
            200, 0.1f
        );

        SmeltingRecipe SMOOTH_STONE(
            RecipeIngredient(STONE, 1, "Stone"),
            RecipeResult(SMOOTH_STONE, 1, "Smooth Stone"),
            SmeltingType::FURNACE,
            200, 0.1f
        );

        SmeltingRecipe BRICKS(
            RecipeIngredient(CLAY, 1, "Clay"),
            RecipeResult(BRICK, 1, "Brick"),
            SmeltingType::FURNACE,
            200, 0.3f
        );

        SmeltingRecipe NETHER_BRICKS(
            RecipeIngredient(NETHERRACK, 1, "Netherrack"),
            RecipeResult(NETHER_BRICK, 1, "Nether Brick"),
            SmeltingType::FURNACE,
            200, 0.1f
        );

        SmeltingRecipe CRACKED_NETHER_BRICKS(
            RecipeIngredient(NETHER_BRICKS_ITEM, 1, "Nether Bricks"),
            RecipeResult(CRACKED_NETHER_BRICKS, 1, "Cracked Nether Bricks"),
            SmeltingType::FURNACE,
            200, 0.1f
        );

        SmeltingRecipe CRACKED_STONE_BRICKS(
            RecipeIngredient(STONE_BRICKS, 1, "Stone Bricks"),
            RecipeResult(CRACKED_STONE_BRICKS, 1, "Cracked Stone Bricks"),
            SmeltingType::FURNACE,
            200, 0.1f
        );

        SmeltingRecipe CRACKED_POLISHED_BLACKSTONE_BRICKS(
            RecipeIngredient(BLACKSTONE, 1, "Polished Blackstone Bricks"),
            RecipeResult(CRACKED_POLISHED_BLACKSTONE_BRICKS, 1, "Cracked Polished Blackstone Bricks"),
            SmeltingType::FURNACE,
            200, 0.1f
        );

        SmeltingRecipe GREEN_DYE_FROM_CACTUS(
            RecipeIngredient(CACTUS, 1, "Cactus"),
            RecipeResult(GREEN_DYE, 1, "Green Dye"),
            SmeltingType::FURNACE,
            200, 1.0f
        );

        SmeltingRecipe CHARCOAL(
            RecipeIngredient(LOG, 1, "Log"),
            RecipeResult(CHARCOAL, 1, "Charcoal"),
            SmeltingType::FURNACE,
            200, 0.15f
        );

        SmeltingRecipe POPPED_CHORUS_FRUIT(
            RecipeIngredient(CHORUS_FRUIT, 1, "Chorus Fruit"),
            RecipeResult(POPPED_CHORUS_FRUIT, 1, "Popped Chorus Fruit"),
            SmeltingType::FURNACE,
            200, 0.1f
        );

        // Blast furnace recipes (ores only)
        SmeltingRecipe IRON_INGOT_BLAST(
            RecipeIngredient(IRON_ORE, 1, "Iron Ore"),
            RecipeResult(IRON_INGOT, 1, "Iron Ingot"),
            SmeltingType::BLAST_FURNACE,
            100, 0.7f
        );

        SmeltingRecipe GOLD_INGOT_BLAST(
            RecipeIngredient(GOLD_ORE, 1, "Gold Ore"),
            RecipeResult(GOLD_INGOT, 1, "Gold Ingot"),
            SmeltingType::BLAST_FURNACE,
            100, 1.0f
        );

        // Smoker recipes (food only)
        SmeltingRecipe COOKED_BEEF_SMOKER(
            RecipeIngredient(BEEF, 1, "Raw Beef"),
            RecipeResult(COOKED_BEEF, 1, "Steak"),
            SmeltingType::SMOKER,
            100, 0.35f
        );

        SmeltingRecipe COOKED_PORKCHOP_SMOKER(
            RecipeIngredient(PORKCHOP, 1, "Raw Porkchop"),
            RecipeResult(COOKED_PORKCHOP, 1, "Cooked Porkchop"),
            SmeltingType::SMOKER,
            100, 0.35f
        );

        // Helper functions
        std::vector<std::shared_ptr<SmeltingRecipe>> GetAllSmeltingRecipes() {
            return {
                // Ores
                std::make_shared<SmeltingRecipe>(IRON_INGOT),
                std::make_shared<SmeltingRecipe>(GOLD_INGOT),
                std::make_shared<SmeltingRecipe>(COPPER_INGOT),
                std::make_shared<SmeltingRecipe>(NETHERITE_INGOT),
                std::make_shared<SmeltingRecipe>(DIAMOND),
                std::make_shared<SmeltingRecipe>(EMERALD),
                std::make_shared<SmeltingRecipe>(LAPIS_LAZULI),
                std::make_shared<SmeltingRecipe>(REDSTONE),
                std::make_shared<SmeltingRecipe>(COAL),
                std::make_shared<SmeltingRecipe>(QUARTZ),

                // Foods
                std::make_shared<SmeltingRecipe>(COOKED_BEEF),
                std::make_shared<SmeltingRecipe>(COOKED_PORKCHOP),
                std::make_shared<SmeltingRecipe>(COOKED_CHICKEN),
                std::make_shared<SmeltingRecipe>(COOKED_RABBIT),
                std::make_shared<SmeltingRecipe>(COOKED_MUTTON),
                std::make_shared<SmeltingRecipe>(COOKED_SALMON),
                std::make_shared<SmeltingRecipe>(COOKED_COD),
                std::make_shared<SmeltingRecipe>(BAKED_POTATO),
                std::make_shared<SmeltingRecipe>(DRIED_KELP),

                // Blocks
                std::make_shared<SmeltingRecipe>(GLASS),
                std::make_shared<SmeltingRecipe>(STONE),
                std::make_shared<SmeltingRecipe>(SMOOTH_STONE),
                std::make_shared<SmeltingRecipe>(BRICKS),
                std::make_shared<SmeltingRecipe>(NETHER_BRICKS),
                std::make_shared<SmeltingRecipe>(CRACKED_NETHER_BRICKS),
                std::make_shared<SmeltingRecipe>(CRACKED_STONE_BRICKS),
                std::make_shared<SmeltingRecipe>(CRACKED_POLISHED_BLACKSTONE_BRICKS),
                std::make_shared<SmeltingRecipe>(GREEN_DYE_FROM_CACTUS),
                std::make_shared<SmeltingRecipe>(CHARCOAL),
                std::make_shared<SmeltingRecipe>(POPPED_CHORUS_FRUIT),

                // Blast furnace
                std::make_shared<SmeltingRecipe>(IRON_INGOT_BLAST),
                std::make_shared<SmeltingRecipe>(GOLD_INGOT_BLAST),

                // Smoker
                std::make_shared<SmeltingRecipe>(COOKED_BEEF_SMOKER),
                std::make_shared<SmeltingRecipe>(COOKED_PORKCHOP_SMOKER)
            };
        }

        std::vector<std::shared_ptr<SmeltingRecipe>> GetRecipesBySmeltingType(SmeltingType type) {
            auto allRecipes = GetAllSmeltingRecipes();
            std::vector<std::shared_ptr<SmeltingRecipe>> filtered;

            for (const auto& recipe : allRecipes) {
                if (recipe->GetSmeltingType() == type) {
                    filtered.push_back(recipe);
                }
            }

            return filtered;
        }

        std::vector<std::shared_ptr<SmeltingRecipe>> GetRecipesByCategory(RecipeCategory category) {
            auto allRecipes = GetAllSmeltingRecipes();
            std::vector<std::shared_ptr<SmeltingRecipe>> filtered;

            for (const auto& recipe : allRecipes) {
                if (recipe->GetCategory() == category) {
                    filtered.push_back(recipe);
                }
            }

            return filtered;
        }

    } // namespace SmeltingRecipes

} // namespace VoxelCraft
