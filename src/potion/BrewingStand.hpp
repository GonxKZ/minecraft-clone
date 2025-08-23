/**
 * @file BrewingStand.hpp
 * @brief VoxelCraft Brewing Stand System - Potion Brewing Interface
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_POTION_BREWING_STAND_HPP
#define VOXELCRAFT_POTION_BREWING_STAND_HPP

#include <memory>
#include <vector>
#include <array>
#include <functional>
#include <unordered_map>
#include <chrono>
#include <glm/glm.hpp>

#include "Potion.hpp"

namespace VoxelCraft {

    class Player;
    class World;
    class Inventory;

    /**
     * @enum BrewingSlot
     * @brief Slots in the brewing stand
     */
    enum class BrewingSlot {
        INPUT = 0,           ///< Input item (usually water bottle or potion)
        INGREDIENT,          ///< Brewing ingredient
        FUEL,                ///< Fuel for brewing (blaze powder)
        OUTPUT1,             ///< First output slot
        OUTPUT2,             ///< Second output slot
        OUTPUT3              ///< Third output slot
    };

    /**
     * @enum BrewingState
     * @brief Current brewing state
     */
    enum class BrewingState {
        IDLE = 0,            ///< Not brewing
        BREWING,             ///< Currently brewing
        COMPLETED,           ///< Brewing completed
        FAILED              ///< Brewing failed
    };

    /**
     * @struct BrewingRecipe
     * @brief Recipe for brewing potions
     */
    struct BrewingRecipe {
        int inputItemID;                     ///< Input item ID
        int ingredientItemID;                ///< Ingredient item ID
        int resultItemID;                    ///< Result item ID
        PotionType resultPotionType;         ///< Result potion type
        int brewingTime;                     ///< Time to brew in ticks
        int fuelCost;                        ///< Fuel cost
        bool requiresBasePotion;             ///< Whether input must be a potion
        std::string description;              ///< Recipe description
    };

    /**
     * @struct BrewingStandState
     * @brief Current state of the brewing stand
     */
    struct BrewingStandState {
        std::array<int, 6> slots;            ///< Item IDs in each slot
        std::array<int, 6> slotCounts;       ///< Item counts in each slot
        BrewingState state;                  ///< Current brewing state
        float brewingProgress;               ///< Brewing progress (0-1)
        int fuelLevel;                       ///< Current fuel level
        int maxFuelLevel;                    ///< Maximum fuel level
        float temperature;                   ///< Brewing stand temperature
        bool isActive;                       ///< Whether brewing stand is active
        std::chrono::steady_clock::time_point startTime;

        BrewingStandState() : state(BrewingState::IDLE), brewingProgress(0.0f),
                            fuelLevel(0), maxFuelLevel(20), temperature(20.0f),
                            isActive(false), startTime(std::chrono::steady_clock::now()) {
            slots.fill(0);
            slotCounts.fill(0);
        }
    };

    /**
     * @struct BrewingStandConfig
     * @brief Configuration for brewing stands
     */
    struct BrewingStandConfig {
        bool enableBrewingStand = true;
        int maxFuelLevel = 20;               ///< Maximum fuel level
        float baseBrewingTime = 20.0f;       ///< Base brewing time in seconds
        float fuelEfficiency = 1.0f;         ///< Fuel efficiency multiplier
        int maxSlots = 6;                    ///< Maximum number of slots
        bool requireFuel = true;             ///< Whether fuel is required
        bool enableOverBrewing = false;      ///< Whether to enable over-brewing
        float temperatureEffect = 0.1f;      ///< Temperature effect on brewing speed
    };

    /**
     * @class BrewingStand
     * @brief Brewing stand block and interface
     */
    class BrewingStand {
    public:
        /**
         * @brief Constructor
         * @param position World position
         * @param world World reference
         */
        BrewingStand(const glm::ivec3& position, World* world);

        /**
         * @brief Destructor
         */
        ~BrewingStand();

        /**
         * @brief Update brewing stand
         * @param deltaTime Time since last update
         */
        void Update(float deltaTime);

        /**
         * @brief Open brewing interface for player
         * @param player Player opening the stand
         * @return true if interface opened
         */
        bool OpenInterface(Player* player);

        /**
         * @brief Close brewing interface
         */
        void CloseInterface();

        /**
         * @brief Get current stand state
         * @return Current state
         */
        const BrewingStandState& GetState() const { return m_state; }

        /**
         * @brief Set item in slot
         * @param slot Slot to set
         * @param itemID Item ID
         * @param count Item count
         * @return true if item was set
         */
        bool SetItem(BrewingSlot slot, int itemID, int count);

        /**
         * @brief Get item from slot
         * @param slot Slot to get from
         * @return Pair of itemID and count
         */
        std::pair<int, int> GetItem(BrewingSlot slot) const;

        /**
         * @brief Start brewing process
         * @return true if brewing started
         */
        bool StartBrewing();

        /**
         * @brief Stop brewing process
         */
        void StopBrewing();

        /**
         * @brief Add fuel to brewing stand
         * @param amount Amount of fuel to add
         * @return true if fuel was added
         */
        bool AddFuel(int amount);

        /**
         * @brief Check if item can be used in slot
         * @param slot Target slot
         * @param itemID Item ID to check
         * @return true if item can be used
         */
        bool CanUseItemInSlot(BrewingSlot slot, int itemID) const;

        /**
         * @brief Get brewing progress
         * @return Progress (0-1)
         */
        float GetBrewingProgress() const { return m_state.brewingProgress; }

        /**
         * @brief Check if brewing stand is active
         * @return true if active
         */
        bool IsActive() const { return m_state.isActive; }

        /**
         * @brief Get fuel level
         * @return Current fuel level
         */
        int GetFuelLevel() const { return m_state.fuelLevel; }

        /**
         * @brief Get maximum fuel level
         * @return Maximum fuel level
         */
        int GetMaxFuelLevel() const { return m_state.maxFuelLevel; }

        /**
         * @brief Check if player can use brewing stand
         * @param player Player to check
         * @return true if can use
         */
        bool CanPlayerUse(Player* player) const;

        /**
         * @brief Get brewing stand position
         * @return World position
         */
        const glm::ivec3& GetPosition() const { return m_position; }

        /**
         * @brief Get current player using stand
         * @return Current player or nullptr
         */
        Player* GetCurrentPlayer() const { return m_currentPlayer; }

        /**
         * @brief Check if stand is in use
         * @return true if in use
         */
        bool IsInUse() const { return m_currentPlayer != nullptr; }

    private:
        glm::ivec3 m_position;
        World* m_world;
        Player* m_currentPlayer;
        BrewingStandState m_state;
        BrewingStandConfig m_config;

        // Brewing mechanics
        float m_brewingTimer;
        int m_currentRecipeIndex;
        std::vector<BrewingRecipe> m_availableRecipes;

        // Animation and effects
        float m_bubbleTimer;
        float m_temperatureChangeTimer;

        /**
         * @brief Update brewing process
         * @param deltaTime Time since last update
         */
        void UpdateBrewing(float deltaTime);

        /**
         * @brief Find matching brewing recipe
         * @return Recipe index or -1 if none found
         */
        int FindMatchingRecipe() const;

        /**
         * @brief Check if brewing can start
         * @return true if can start
         */
        bool CanStartBrewing() const;

        /**
         * @brief Consume brewing ingredients
         * @param recipe Recipe to consume for
         * @return true if consumed successfully
         */
        bool ConsumeIngredients(const BrewingRecipe& recipe);

        /**
         * @brief Produce brewed potions
         * @param recipe Recipe to produce
         * @return true if produced successfully
         */
        bool ProducePotions(const BrewingRecipe& recipe);

        /**
         * @brief Update temperature
         * @param deltaTime Time since last update
         */
        void UpdateTemperature(float deltaTime);

        /**
         * @brief Update visual effects
         * @param deltaTime Time since last update
         */
        void UpdateVisualEffects(float deltaTime);

        /**
         * @brief Play brewing effects
         */
        void PlayBrewingEffects();

        /**
         * @brief Load default brewing recipes
         */
        void LoadDefaultRecipes();

        /**
         * @brief Validate brewing setup
         * @return true if setup is valid
         */
        bool ValidateSetup() const;
    };

    /**
     * @class BrewingStandManager
     * @brief Manager for all brewing stands in the world
     */
    class BrewingStandManager {
    public:
        /**
         * @brief Get singleton instance
         * @return BrewingStandManager instance
         */
        static BrewingStandManager& GetInstance();

        /**
         * @brief Initialize manager
         * @param world World reference
         * @return true if successful
         */
        bool Initialize(World* world);

        /**
         * @brief Shutdown manager
         */
        void Shutdown();

        /**
         * @brief Update all brewing stands
         * @param deltaTime Time since last update
         */
        void Update(float deltaTime);

        /**
         * @brief Create brewing stand at position
         * @param position Position to create stand
         * @return true if created successfully
         */
        bool CreateBrewingStand(const glm::ivec3& position);

        /**
         * @brief Remove brewing stand at position
         * @param position Position to remove stand
         * @return true if removed successfully
         */
        bool RemoveBrewingStand(const glm::ivec3& position);

        /**
         * @brief Get brewing stand at position
         * @param position Position to check
         * @return Brewing stand or nullptr
         */
        std::shared_ptr<BrewingStand> GetBrewingStand(const glm::ivec3& position) const;

        /**
         * @brief Get all brewing stands
         * @return Vector of all stands
         */
        std::vector<std::shared_ptr<BrewingStand>> GetAllBrewingStands() const;

        /**
         * @brief Find nearest brewing stand to position
         * @param position Search position
         * @param maxDistance Maximum search distance
         * @return Nearest stand or nullptr
         */
        std::shared_ptr<BrewingStand> FindNearestBrewingStand(const glm::vec3& position, float maxDistance = 32.0f);

        /**
         * @brief Get brewing recipes
         * @return Vector of all brewing recipes
         */
        const std::vector<BrewingRecipe>& GetBrewingRecipes() const;

        /**
         * @brief Add custom brewing recipe
         * @param recipe Recipe to add
         * @return true if added successfully
         */
        bool AddBrewingRecipe(const BrewingRecipe& recipe);

        /**
         * @brief Get brewing stand statistics
         * @return Stand statistics
         */
        const BrewingStandStats& GetStats() const { return m_stats; }

    private:
        BrewingStandManager() = default;
        ~BrewingStandManager() = default;

        // Prevent copying
        BrewingStandManager(const BrewingStandManager&) = delete;
        BrewingStandManager& operator=(const BrewingStandManager&) = delete;

        World* m_world;
        std::unordered_map<glm::ivec3, std::shared_ptr<BrewingStand>> m_stands;
        std::vector<BrewingRecipe> m_recipes;
        BrewingStandStats m_stats;
        bool m_initialized = false;

        void LoadDefaultRecipes();
    };

    /**
     * @struct BrewingStandStats
     * @brief Statistics for brewing stands
     */
    struct BrewingStandStats {
        int totalStands = 0;
        int activeStands = 0;
        int potionsBrewed = 0;
        int fuelConsumed = 0;
        int failedBrews = 0;
        float totalBrewingTime = 0.0f;
        float averageTemperature = 0.0f;
        std::unordered_map<PotionType, int> potionsByType;
        std::unordered_map<int, int> ingredientsUsed;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_POTION_BREWING_STAND_HPP
