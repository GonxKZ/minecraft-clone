#include "ShapedRecipe.hpp"
#include <algorithm>

namespace VoxelCraft {

    ShapedRecipe::ShapedRecipe(const std::vector<std::string>& pattern,
                             const KeyMap& key,
                             const RecipeResult& result)
        : CraftingRecipe(RecipeType::SHAPED, RecipeCategory::MISCELLANEOUS),
          m_key(key), m_result(result), m_width(0), m_height(0)
    {
        InitializePattern(pattern);
        m_name = GenerateRecipeName();
    }

    ShapedRecipe::ShapedRecipe(const std::vector<std::string>& pattern,
                             const KeyMap& key,
                             const RecipeResult& result,
                             RecipeCategory category)
        : CraftingRecipe(RecipeType::SHAPED, category),
          m_key(key), m_result(result), m_width(0), m_height(0)
    {
        InitializePattern(pattern);
        m_name = GenerateRecipeName();
    }

    std::string ShapedRecipe::GetName() const {
        return m_name;
    }

    bool ShapedRecipe::Matches(const std::vector<RecipeIngredient>& ingredients) const {
        // Convert ingredients to 3x3 grid
        CraftingGrid grid(3, 3);
        int idx = 0;

        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 3; ++x) {
                if (idx < ingredients.size()) {
                    grid.slots[y * 3 + x] = ingredients[idx];
                    idx++;
                }
            }
        }

        return GridMatchesPattern(grid);
    }

    std::vector<RecipeIngredient> ShapedRecipe::GetRequiredIngredients() const {
        std::vector<RecipeIngredient> ingredients;

        for (int y = 0; y < m_height; ++y) {
            for (int x = 0; x < m_width; ++x) {
                int patternItem = m_pattern[y][x];
                if (patternItem != 0) {
                    // Find ingredient in key
                    for (const auto& pair : m_key) {
                        if (pair.second.itemID == patternItem) {
                            ingredients.push_back(pair.second);
                            break;
                        }
                    }
                }
            }
        }

        return ingredients;
    }

    void ShapedRecipe::InitializePattern(const std::vector<std::string>& pattern) {
        m_height = pattern.size();
        m_width = 0;

        // Find maximum width
        for (const auto& row : pattern) {
            m_width = std::max(m_width, static_cast<int>(row.length()));
        }

        // Initialize pattern with zeros
        for (int y = 0; y < MAX_HEIGHT; ++y) {
            for (int x = 0; x < MAX_WIDTH; ++x) {
                m_pattern[y][x] = 0;
            }
        }

        // Fill pattern from string
        for (int y = 0; y < m_height; ++y) {
            const std::string& row = pattern[y];
            for (int x = 0; x < std::min(m_width, static_cast<int>(row.length())); ++x) {
                char symbol = row[x];
                if (m_key.find(symbol) != m_key.end()) {
                    m_pattern[y][x] = m_key.at(symbol).itemID;
                }
            }
        }
    }

    bool ShapedRecipe::GridMatchesPattern(const CraftingGrid& grid) const {
        // Try different offsets for pattern matching
        for (int offsetY = 0; offsetY <= 3 - m_height; ++offsetY) {
            for (int offsetX = 0; offsetX <= 3 - m_width; ++offsetX) {
                if (MatchesAtOffset(grid, offsetX, offsetY)) {
                    return true;
                }
            }
        }
        return false;
    }

    bool ShapedRecipe::MatchesAtOffset(const CraftingGrid& grid, int offsetX, int offsetY) const {
        // Check pattern match at specific offset
        for (int y = 0; y < m_height; ++y) {
            for (int x = 0; x < m_width; ++x) {
                int patternItem = m_pattern[y][x];
                int gridX = x + offsetX;
                int gridY = y + offsetY;
                int gridItem = grid.slots[gridY * 3 + gridX].itemID;

                if (patternItem != gridItem) {
                    return false;
                }
            }
        }
        return true;
    }

    std::string ShapedRecipe::GenerateRecipeName() const {
        std::stringstream ss;
        ss << "Shaped Recipe: " << m_result.itemName;
        return ss.str();
    }

    namespace Recipes {

        // Item IDs (simplified for this example)
        const int WOOD = 1;
        const int STONE = 2;
        const int IRON_INGOT = 3;
        const int DIAMOND = 4;
        const int LEATHER = 5;
        const int COAL = 6;
        const int STICK = 7;
        const int STRING = 8;
        const int WHEAT = 9;
        const int REDSTONE = 10;

        // Tool recipes
        ShapedRecipe WOODEN_PICKAXE(
            {"XXX", " S ", " S "},
            {{'X', RecipeIngredient(WOOD, 3, "Wood")}, {'S', RecipeIngredient(STICK, 2, "Stick")}},
            RecipeResult(100, 1, "Wooden Pickaxe"),
            RecipeCategory::TOOLS
        );

        ShapedRecipe STONE_PICKAXE(
            {"XXX", " S ", " S "},
            {{'X', RecipeIngredient(STONE, 3, "Stone")}, {'S', RecipeIngredient(STICK, 2, "Stick")}},
            RecipeResult(101, 1, "Stone Pickaxe"),
            RecipeCategory::TOOLS
        );

        ShapedRecipe IRON_PICKAXE(
            {"XXX", " S ", " S "},
            {{'X', RecipeIngredient(IRON_INGOT, 3, "Iron Ingot")}, {'S', RecipeIngredient(STICK, 2, "Stick")}},
            RecipeResult(102, 1, "Iron Pickaxe"),
            RecipeCategory::TOOLS
        );

        ShapedRecipe DIAMOND_PICKAXE(
            {"XXX", " S ", " S "},
            {{'X', RecipeIngredient(DIAMOND, 3, "Diamond")}, {'S', RecipeIngredient(STICK, 2, "Stick")}},
            RecipeResult(103, 1, "Diamond Pickaxe"),
            RecipeCategory::TOOLS
        );

        ShapedRecipe WOODEN_AXE(
            {"XX", "XS", " S"},
            {{'X', RecipeIngredient(WOOD, 3, "Wood")}, {'S', RecipeIngredient(STICK, 2, "Stick")}},
            RecipeResult(104, 1, "Wooden Axe"),
            RecipeCategory::TOOLS
        );

        ShapedRecipe STONE_AXE(
            {"XX", "XS", " S"},
            {{'X', RecipeIngredient(STONE, 3, "Stone")}, {'S', RecipeIngredient(STICK, 2, "Stick")}},
            RecipeResult(105, 1, "Stone Axe"),
            RecipeCategory::TOOLS
        );

        ShapedRecipe IRON_AXE(
            {"XX", "XS", " S"},
            {{'X', RecipeIngredient(IRON_INGOT, 3, "Iron Ingot")}, {'S', RecipeIngredient(STICK, 2, "Stick")}},
            RecipeResult(106, 1, "Iron Axe"),
            RecipeCategory::TOOLS
        );

        ShapedRecipe DIAMOND_AXE(
            {"XX", "XS", " S"},
            {{'X', RecipeIngredient(DIAMOND, 3, "Diamond")}, {'S', RecipeIngredient(STICK, 2, "Stick")}},
            RecipeResult(107, 1, "Diamond Axe"),
            RecipeCategory::TOOLS
        );

        ShapedRecipe WOODEN_SHOVEL(
            {"X", "S", "S"},
            {{'X', RecipeIngredient(WOOD, 1, "Wood")}, {'S', RecipeIngredient(STICK, 2, "Stick")}},
            RecipeResult(108, 1, "Wooden Shovel"),
            RecipeCategory::TOOLS
        );

        ShapedRecipe STONE_SHOVEL(
            {"X", "S", "S"},
            {{'X', RecipeIngredient(STONE, 1, "Stone")}, {'S', RecipeIngredient(STICK, 2, "Stick")}},
            RecipeResult(109, 1, "Stone Shovel"),
            RecipeCategory::TOOLS
        );

        ShapedRecipe IRON_SHOVEL(
            {"X", "S", "S"},
            {{'X', RecipeIngredient(IRON_INGOT, 1, "Iron Ingot")}, {'S', RecipeIngredient(STICK, 2, "Stick")}},
            RecipeResult(110, 1, "Iron Shovel"),
            RecipeCategory::TOOLS
        );

        ShapedRecipe DIAMOND_SHOVEL(
            {"X", "S", "S"},
            {{'X', RecipeIngredient(DIAMOND, 1, "Diamond")}, {'S', RecipeIngredient(STICK, 2, "Stick")}},
            RecipeResult(111, 1, "Diamond Shovel"),
            RecipeCategory::TOOLS
        );

        ShapedRecipe WOODEN_SWORD(
            {"X", "X", "S"},
            {{'X', RecipeIngredient(WOOD, 2, "Wood")}, {'S', RecipeIngredient(STICK, 1, "Stick")}},
            RecipeResult(112, 1, "Wooden Sword"),
            RecipeCategory::COMBAT
        );

        ShapedRecipe STONE_SWORD(
            {"X", "X", "S"},
            {{'X', RecipeIngredient(STONE, 2, "Stone")}, {'S', RecipeIngredient(STICK, 1, "Stick")}},
            RecipeResult(113, 1, "Stone Sword"),
            RecipeCategory::COMBAT
        );

        ShapedRecipe IRON_SWORD(
            {"X", "X", "S"},
            {{'X', RecipeIngredient(IRON_INGOT, 2, "Iron Ingot")}, {'S', RecipeIngredient(STICK, 1, "Stick")}},
            RecipeResult(114, 1, "Iron Sword"),
            RecipeCategory::COMBAT
        );

        ShapedRecipe DIAMOND_SWORD(
            {"X", "X", "S"},
            {{'X', RecipeIngredient(DIAMOND, 2, "Diamond")}, {'S', RecipeIngredient(STICK, 1, "Stick")}},
            RecipeResult(115, 1, "Diamond Sword"),
            RecipeCategory::COMBAT
        );

        // Armor recipes
        ShapedRecipe LEATHER_HELMET(
            {"XXX", "X X"},
            {{'X', RecipeIngredient(LEATHER, 5, "Leather")}},
            RecipeResult(200, 1, "Leather Helmet"),
            RecipeCategory::COMBAT
        );

        ShapedRecipe LEATHER_CHESTPLATE(
            {"X X", "XXX", "XXX"},
            {{'X', RecipeIngredient(LEATHER, 8, "Leather")}},
            RecipeResult(201, 1, "Leather Chestplate"),
            RecipeCategory::COMBAT
        );

        ShapedRecipe LEATHER_LEGGINGS(
            {"XXX", "X X", "X X"},
            {{'X', RecipeIngredient(LEATHER, 7, "Leather")}},
            RecipeResult(202, 1, "Leather Leggings"),
            RecipeCategory::COMBAT
        );

        ShapedRecipe LEATHER_BOOTS(
            {"X X", "X X"},
            {{'X', RecipeIngredient(LEATHER, 4, "Leather")}},
            RecipeResult(203, 1, "Leather Boots"),
            RecipeCategory::COMBAT
        );

        ShapedRecipe IRON_HELMET(
            {"XXX", "X X"},
            {{'X', RecipeIngredient(IRON_INGOT, 5, "Iron Ingot")}},
            RecipeResult(204, 1, "Iron Helmet"),
            RecipeCategory::COMBAT
        );

        ShapedRecipe IRON_CHESTPLATE(
            {"X X", "XXX", "XXX"},
            {{'X', RecipeIngredient(IRON_INGOT, 8, "Iron Ingot")}},
            RecipeResult(205, 1, "Iron Chestplate"),
            RecipeCategory::COMBAT
        );

        ShapedRecipe IRON_LEGGINGS(
            {"XXX", "X X", "X X"},
            {{'X', RecipeIngredient(IRON_INGOT, 7, "Iron Ingot")}},
            RecipeResult(206, 1, "Iron Leggings"),
            RecipeCategory::COMBAT
        );

        ShapedRecipe IRON_BOOTS(
            {"X X", "X X"},
            {{'X', RecipeIngredient(IRON_INGOT, 4, "Iron Ingot")}},
            RecipeResult(207, 1, "Iron Boots"),
            RecipeCategory::COMBAT
        );

        ShapedRecipe DIAMOND_HELMET(
            {"XXX", "X X"},
            {{'X', RecipeIngredient(DIAMOND, 5, "Diamond")}},
            RecipeResult(208, 1, "Diamond Helmet"),
            RecipeCategory::COMBAT
        );

        ShapedRecipe DIAMOND_CHESTPLATE(
            {"X X", "XXX", "XXX"},
            {{'X', RecipeIngredient(DIAMOND, 8, "Diamond")}},
            RecipeResult(209, 1, "Diamond Chestplate"),
            RecipeCategory::COMBAT
        );

        ShapedRecipe DIAMOND_LEGGINGS(
            {"XXX", "X X", "X X"},
            {{'X', RecipeIngredient(DIAMOND, 7, "Diamond")}},
            RecipeResult(210, 1, "Diamond Leggings"),
            RecipeCategory::COMBAT
        );

        ShapedRecipe DIAMOND_BOOTS(
            {"X X", "X X"},
            {{'X', RecipeIngredient(DIAMOND, 4, "Diamond")}},
            RecipeResult(211, 1, "Diamond Boots"),
            RecipeCategory::COMBAT
        );

        // Building blocks
        ShapedRecipe WOODEN_PLANKS(
            {"X"},
            {{'X', RecipeIngredient(17, 1, "Log")}},
            RecipeResult(WOOD, 4, "Wooden Planks"),
            RecipeCategory::BUILDING_BLOCKS
        );

        ShapedRecipe STONE_BRICKS(
            {"XX", "XX"},
            {{'X', RecipeIngredient(STONE, 4, "Stone")}},
            RecipeResult(18, 4, "Stone Bricks"),
            RecipeCategory::BUILDING_BLOCKS
        );

        ShapedRecipe STICK(
            {"X", "X"},
            {{'X', RecipeIngredient(WOOD, 2, "Wooden Planks")}},
            RecipeResult(7, 4, "Stick"),
            RecipeCategory::MISCELLANEOUS
        );

        ShapedRecipe TORCH(
            {"C", "S"},
            {{'C', RecipeIngredient(COAL, 1, "Coal")}, {'S', RecipeIngredient(STICK, 1, "Stick")}},
            RecipeResult(19, 4, "Torch"),
            RecipeCategory::DECORATION
        );

        ShapedRecipe CRAFTING_TABLE(
            {"XX", "XX"},
            {{'X', RecipeIngredient(WOOD, 4, "Wooden Planks")}},
            RecipeResult(20, 1, "Crafting Table"),
            RecipeCategory::MISCELLANEOUS
        );

        ShapedRecipe FURNACE(
            {"XXX", "X X", "XXX"},
            {{'X', RecipeIngredient(18, 8, "Stone Bricks")}},
            RecipeResult(21, 1, "Furnace"),
            RecipeCategory::MISCELLANEOUS
        );

        ShapedRecipe CHEST(
            {"XXX", "X X", "XXX"},
            {{'X', RecipeIngredient(WOOD, 8, "Wooden Planks")}},
            RecipeResult(22, 1, "Chest"),
            RecipeCategory::DECORATION
        );

        // Helper functions
        std::vector<std::shared_ptr<ShapedRecipe>> GetAllShapedRecipes() {
            return {
                std::make_shared<ShapedRecipe>(WOODEN_PICKAXE),
                std::make_shared<ShapedRecipe>(STONE_PICKAXE),
                std::make_shared<ShapedRecipe>(IRON_PICKAXE),
                std::make_shared<ShapedRecipe>(DIAMOND_PICKAXE),
                std::make_shared<ShapedRecipe>(WOODEN_AXE),
                std::make_shared<ShapedRecipe>(STONE_AXE),
                std::make_shared<ShapedRecipe>(IRON_AXE),
                std::make_shared<ShapedRecipe>(DIAMOND_AXE),
                std::make_shared<ShapedRecipe>(WOODEN_SHOVEL),
                std::make_shared<ShapedRecipe>(STONE_SHOVEL),
                std::make_shared<ShapedRecipe>(IRON_SHOVEL),
                std::make_shared<ShapedRecipe>(DIAMOND_SHOVEL),
                std::make_shared<ShapedRecipe>(WOODEN_SWORD),
                std::make_shared<ShapedRecipe>(STONE_SWORD),
                std::make_shared<ShapedRecipe>(IRON_SWORD),
                std::make_shared<ShapedRecipe>(DIAMOND_SWORD),
                std::make_shared<ShapedRecipe>(LEATHER_HELMET),
                std::make_shared<ShapedRecipe>(LEATHER_CHESTPLATE),
                std::make_shared<ShapedRecipe>(LEATHER_LEGGINGS),
                std::make_shared<ShapedRecipe>(LEATHER_BOOTS),
                std::make_shared<ShapedRecipe>(IRON_HELMET),
                std::make_shared<ShapedRecipe>(IRON_CHESTPLATE),
                std::make_shared<ShapedRecipe>(IRON_LEGGINGS),
                std::make_shared<ShapedRecipe>(IRON_BOOTS),
                std::make_shared<ShapedRecipe>(DIAMOND_HELMET),
                std::make_shared<ShapedRecipe>(DIAMOND_CHESTPLATE),
                std::make_shared<ShapedRecipe>(DIAMOND_LEGGINGS),
                std::make_shared<ShapedRecipe>(DIAMOND_BOOTS),
                std::make_shared<ShapedRecipe>(WOODEN_PLANKS),
                std::make_shared<ShapedRecipe>(STONE_BRICKS),
                std::make_shared<ShapedRecipe>(STICK),
                std::make_shared<ShapedRecipe>(TORCH),
                std::make_shared<ShapedRecipe>(CRAFTING_TABLE),
                std::make_shared<ShapedRecipe>(FURNACE),
                std::make_shared<ShapedRecipe>(CHEST)
            };
        }

        std::vector<std::shared_ptr<ShapedRecipe>> GetRecipesByCategory(RecipeCategory category) {
            auto allRecipes = GetAllShapedRecipes();
            std::vector<std::shared_ptr<ShapedRecipe>> filtered;

            for (const auto& recipe : allRecipes) {
                if (recipe->GetCategory() == category) {
                    filtered.push_back(recipe);
                }
            }

            return filtered;
        }

    } // namespace Recipes

} // namespace VoxelCraft
