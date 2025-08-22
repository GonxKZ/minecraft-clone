/**
 * @file VillageSystem.hpp
 * @brief VoxelCraft Village System - NPC Villages and Civilization
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the VillageSystem class that manages NPC villages,
 * civilization mechanics, social structures, and complex AI behaviors
 * for creating living, breathing villages in the game world.
 */

#ifndef VOXELCRAFT_AI_VILLAGE_SYSTEM_HPP
#define VOXELCRAFT_AI_VILLAGE_SYSTEM_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <chrono>
#include <any>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"
#include "../world/World.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Mob;
    class ProceduralGenerator;
    class Village;

    /**
     * @enum VillageType
     * @brief Types of villages that can be generated
     */
    enum class VillageType {
        Rural,                  ///< Small rural village
        Town,                   ///< Medium-sized town
        City,                   ///< Large city
        Fortress,               ///< Military fortress
        Monastery,              ///< Religious monastery
        TradingPost,            ///< Trading outpost
        Farmstead,              ///< Agricultural farm
        MiningColony,           ///< Mining settlement
        FishingVillage,         ///< Coastal fishing village
        NomadicCamp,            ///< Temporary nomadic camp
        Custom                 ///< Custom village type
    };

    /**
     * @enum BuildingType
     * @brief Types of buildings in villages
     */
    enum class BuildingType {
        House,                  ///< Residential house
        Blacksmith,             ///< Blacksmith shop
        Farm,                   ///< Farm building
        Mill,                   ///< Grain mill
        Inn,                    ///< Inn/tavern
        Shop,                   ///< General shop
        Temple,                 ///< Religious temple
        GuardTower,             ///< Guard tower
        Wall,                   ///< Defensive wall
        Gate,                   ///< Village gate
        Well,                   ///< Water well
        Stable,                 ///< Animal stable
        Warehouse,              ///< Storage warehouse
        Workshop,               ///< Craft workshop
        Library,                ///< Knowledge library
        Market,                 ///< Marketplace
        Custom                 ///< Custom building type
    };

    /**
     * @enum VillageRole
     * @brief Roles that NPCs can have in villages
     */
    enum class VillageRole {
        Villager,               ///< Basic villager
        Elder,                  ///< Village elder/leader
        Blacksmith,             ///< Blacksmith
        Farmer,                 ///< Farmer
        Merchant,               ///< Merchant/trader
        Guard,                  ///< Village guard
        Priest,                 ///< Religious priest
        Teacher,                ///< Village teacher
        Healer,                 ///< Village healer
        Innkeeper,              ///< Inn/tavern keeper
        Child,                  ///< Village child
        Animal,                 ///< Village animal
        Custom                 ///< Custom role
    };

    /**
     * @enum VillageState
     * @brief Current state of a village
     */
    enum class VillageState {
        Peaceful,               ///< Village is peaceful
        Threatened,             ///< Village is under threat
        UnderAttack,            ///< Village is under attack
        Destroyed,              ///< Village is destroyed
        Rebuilding,             ///< Village is rebuilding
        Abandoned,              ///< Village is abandoned
        Prosperous,             ///< Village is prosperous
        Suffering               ///< Village is suffering (famine, plague, etc.)
    };

    /**
     * @struct VillageConfig
     * @brief Configuration for village generation and management
     */
    struct VillageConfig {
        // Generation settings
        VillageType defaultType;                ///< Default village type
        int minVillageSize;                     ///< Minimum village size
        int maxVillageSize;                     ///< Maximum village size
        float minVillageSpacing;                ///< Minimum spacing between villages
        float maxVillageRadius;                 ///< Maximum village radius
        bool enableVillageExpansion;            ///< Enable village growth
        int maxVillagesPerWorld;                ///< Maximum villages per world

        // Building settings
        float buildingDensity;                  ///< Building placement density
        int minBuildingsPerVillage;             ///< Minimum buildings per village
        int maxBuildingsPerVillage;             ///< Maximum buildings per village
        bool enableBuildingUpgrades;            ///< Enable building upgrades
        float roadWidth;                        ///< Village road width

        // NPC settings
        int minNPCsPerVillage;                  ///< Minimum NPCs per village
        int maxNPCsPerVillage;                  ///< Maximum NPCs per village
        bool enableNPCRelationships;            ///< Enable NPC social relationships
        bool enableNPCTrading;                  ///< Enable NPC trading
        bool enableNPCQuests;                   ///< Enable NPC quests

        // Resource settings
        bool enableResourceManagement;          ///< Enable village resource management
        float foodConsumptionRate;              ///< Daily food consumption per NPC
        float resourceRegenerationRate;         ///< Resource regeneration rate
        bool enableFamineSystem;                ///< Enable famine mechanics
        bool enablePlagueSystem;                ///< Enable plague mechanics

        // Defense settings
        bool enableVillageDefense;              ///< Enable village defense system
        int maxGuardsPerVillage;                ///< Maximum guards per village
        float defenseActivationRange;           ///< Defense activation range
        bool enableFortifications;              ///< Enable defensive structures

        // Economy settings
        bool enableVillageEconomy;              ///< Enable village economy
        float baseTaxRate;                      ///< Base village tax rate
        bool enableTradingRoutes;               ///< Enable inter-village trading
        float marketFluctuationRate;            ///< Market price fluctuation rate

        // Social settings
        bool enableSocialEvents;                ///< Enable village social events
        float festivalFrequency;                ///< Festival frequency (days)
        bool enableCulturalTraditions;          ///< Enable cultural traditions
        bool enableVillagePolitics;             ///< Enable village politics

        // Environmental settings
        bool respectTerrain;                    ///< Respect terrain when building
        bool enableSeasonalEffects;             ///< Enable seasonal effects on villages
        bool enableWeatherEffects;              ///< Enable weather effects on villages
        float environmentalAdaptationRate;      ///< Environmental adaptation rate
    };

    /**
     * @struct BuildingData
     * @brief Data for individual village buildings
     */
    struct BuildingData {
        uint32_t buildingId;                    ///< Unique building ID
        BuildingType type;                      ///< Building type
        std::string name;                       ///< Building name
        glm::vec3 position;                     ///< Building world position
        glm::vec3 size;                         ///< Building dimensions
        float rotation;                         ///< Building rotation (degrees)
        int level;                              ///< Building level/upgrade
        float condition;                        ///< Building condition (0.0 - 1.0)
        bool isOccupied;                        ///< Building is occupied
        Mob* owner;                             ///< Building owner (if any)
        std::vector<Mob*> occupants;            ///< Building occupants
        std::unordered_map<std::string, std::any> properties; ///< Custom properties
    };

    /**
     * @struct VillageData
     * @brief Core data for a village
     */
    struct VillageData {
        uint32_t villageId;                     ///< Unique village ID
        std::string name;                       ///< Village name
        VillageType type;                       ///< Village type
        VillageState state;                     ///< Village state
        glm::vec3 center;                       ///< Village center position
        float radius;                           ///< Village radius
        AABB bounds;                            ///< Village bounding box

        // Population
        std::vector<Mob*> villagers;            ///< Village NPCs
        int population;                         ///< Current population
        int maxPopulation;                      ///< Maximum population
        std::unordered_map<VillageRole, std::vector<Mob*>> roleAssignments; ///< Role assignments

        // Buildings and structures
        std::vector<BuildingData> buildings;    ///< Village buildings
        std::vector<glm::vec3> roads;           ///< Village road network
        std::vector<AABB> defensiveStructures;  ///< Defensive structures

        // Resources and economy
        std::unordered_map<std::string, float> resources; ///< Village resources
        float wealth;                           ///< Village wealth
        float reputation;                       ///< Village reputation (0.0 - 1.0)
        std::vector<std::string> tradeRoutes;   ///< Connected trade routes

        // Social and cultural
        float happiness;                        ///< Village happiness level
        float culture;                          ///< Cultural development level
        std::vector<std::string> traditions;    ///< Village traditions
        std::vector<std::string> activeEvents;  ///< Currently active events

        // Defense and security
        float securityLevel;                    ///< Village security level
        int guardCount;                         ///< Number of guards
        bool underAttack;                       ///< Village is under attack
        double lastAttackTime;                  ///< Last attack timestamp

        // Environmental
        float environmentalHealth;              ///< Environmental health
        std::string biome;                      ///< Village biome
        bool isCoastal;                         ///< Village is coastal
        float altitude;                         ///< Village altitude
    };

    /**
     * @struct VillageMetrics
     * @brief Performance and statistics for village system
     */
    struct VillageMetrics {
        // Performance metrics
        uint64_t updateCount;                   ///< Number of updates performed
        double totalUpdateTime;                 ///< Total update time (ms)
        double averageUpdateTime;               ///< Average update time (ms)
        double maxUpdateTime;                   ///< Maximum update time (ms)

        // Generation metrics
        uint64_t villagesGenerated;             ///< Total villages generated
        uint64_t buildingsCreated;              ///< Total buildings created
        uint64_t npcsSpawned;                   ///< Total NPCs spawned
        double averageGenerationTime;           ///< Average generation time

        // Population metrics
        uint32_t totalVillagers;                ///< Total villagers across all villages
        uint32_t villagesActive;                ///< Active villages
        uint32_t villagesDestroyed;             ///< Destroyed villages
        uint32_t villagesAbandoned;             ///< Abandoned villages

        // Economic metrics
        float totalWealth;                      ///< Total wealth across villages
        uint64_t tradesCompleted;               ///< Completed trades
        uint64_t resourcesGathered;             ///< Resources gathered
        float averageVillageWealth;             ///< Average village wealth

        // Social metrics
        uint64_t festivalsHeld;                 ///< Festivals held
        uint64_t marriages;                     ///< Village marriages
        uint64_t births;                        ///< Village births
        uint64_t deaths;                        ///< Village deaths

        // Conflict metrics
        uint64_t attacksRepelled;               ///< Attacks successfully repelled
        uint64_t villagesLost;                  ///< Villages lost to attacks
        float averageSecurityLevel;             ///< Average village security
        uint64_t guardsTrained;                 ///< Guards trained

        // Resource metrics
        uint64_t famines;                       ///< Famine events
        uint64_t plagues;                       ///< Plague events
        uint64_t resourceShortages;             ///< Resource shortage events
        float averageResourceLevel;             ///< Average resource levels
    };

    /**
     * @struct VillageEvent
     * @brief Events that can occur in villages
     */
    struct VillageEvent {
        std::string eventId;                    ///< Unique event ID
        std::string name;                       ///< Event name
        std::string description;                ///< Event description
        VillageState requiredState;             ///< Required village state
        float probability;                      ///< Event probability
        float duration;                         ///< Event duration (days)
        std::unordered_map<std::string, float> effects; ///< Event effects
        std::vector<std::string> requirements;  ///< Event requirements
        bool isPositive;                        ///< Event is positive
        double timestamp;                       ///< Event timestamp
    };

    /**
     * @class Village
     * @brief Individual village entity with full simulation
     *
     * Village represents a complete simulated village with NPCs, buildings,
     * economy, social structures, and dynamic behaviors. Villages can grow,
     * decline, defend themselves, trade, and evolve over time.
     */
    class Village {
    public:
        /**
         * @brief Constructor
         * @param data Village data
         * @param world Game world
         */
        Village(const VillageData& data, World* world);

        /**
         * @brief Destructor
         */
        ~Village();

        /**
         * @brief Deleted copy constructor
         */
        Village(const Village&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        Village& operator=(const Village&) = delete;

        // Village lifecycle

        /**
         * @brief Initialize village
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Update village
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Shutdown village
         */
        void Shutdown();

        // Village information

        /**
         * @brief Get village ID
         * @return Village ID
         */
        uint32_t GetId() const { return m_data.villageId; }

        /**
         * @brief Get village name
         * @return Village name
         */
        const std::string& GetName() const { return m_data.name; }

        /**
         * @brief Set village name
         * @param name New village name
         */
        void SetName(const std::string& name) { m_data.name = name; }

        /**
         * @brief Get village type
         * @return Village type
         */
        VillageType GetType() const { return m_data.type; }

        /**
         * @brief Get village state
         * @return Village state
         */
        VillageState GetState() const { return m_data.state; }

        /**
         * @brief Set village state
         * @param state New village state
         */
        void SetState(VillageState state);

        /**
         * @brief Get village data
         * @return Village data
         */
        const VillageData& GetData() const { return m_data; }

        /**
         * @brief Get village center
         * @return Village center position
         */
        const glm::vec3& GetCenter() const { return m_data.center; }

        /**
         * @brief Get village radius
         * @return Village radius
         */
        float GetRadius() const { return m_data.radius; }

        /**
         * @brief Get village bounds
         * @return Village bounding box
         */
        const AABB& GetBounds() const { return m_data.bounds; }

        // Population management

        /**
         * @brief Get population count
         * @return Current population
         */
        int GetPopulation() const { return m_data.population; }

        /**
         * @brief Get maximum population
         * @return Maximum population
         */
        int GetMaxPopulation() const { return m_data.maxPopulation; }

        /**
         * @brief Add villager to village
         * @param villager Villager to add
         * @return true if added, false otherwise
         */
        bool AddVillager(Mob* villager);

        /**
         * @brief Remove villager from village
         * @param villager Villager to remove
         * @return true if removed, false otherwise
         */
        bool RemoveVillager(Mob* villager);

        /**
         * @brief Get villagers by role
         * @param role Village role
         * @return Vector of villagers with role
         */
        std::vector<Mob*> GetVillagersByRole(VillageRole role) const;

        /**
         * @brief Assign role to villager
         * @param villager Villager to assign
         * @param role Role to assign
         * @return true if assigned, false otherwise
         */
        bool AssignRole(Mob* villager, VillageRole role);

        // Building management

        /**
         * @brief Add building to village
         * @param building Building data
         * @return true if added, false otherwise
         */
        bool AddBuilding(const BuildingData& building);

        /**
         * @brief Remove building from village
         * @param buildingId Building ID
         * @return true if removed, false otherwise
         */
        bool RemoveBuilding(uint32_t buildingId);

        /**
         * @brief Get building by ID
         * @param buildingId Building ID
         * @return Building data or empty optional
         */
        std::optional<BuildingData> GetBuilding(uint32_t buildingId) const;

        /**
         * @brief Get buildings by type
         * @param type Building type
         * @return Vector of buildings of type
         */
        std::vector<BuildingData> GetBuildingsByType(BuildingType type) const;

        /**
         * @brief Upgrade building
         * @param buildingId Building ID
         * @return true if upgraded, false otherwise
         */
        bool UpgradeBuilding(uint32_t buildingId);

        // Resource management

        /**
         * @brief Get resource amount
         * @param resourceType Resource type
         * @return Resource amount
         */
        float GetResource(const std::string& resourceType) const;

        /**
         * @brief Set resource amount
         * @param resourceType Resource type
         * @param amount Resource amount
         */
        void SetResource(const std::string& resourceType, float amount);

        /**
         * @brief Add resource to village
         * @param resourceType Resource type
         * @param amount Amount to add
         * @return Actual amount added
         */
        float AddResource(const std::string& resourceType, float amount);

        /**
         * @brief Remove resource from village
         * @param resourceType Resource type
         * @param amount Amount to remove
         * @return Actual amount removed
         */
        float RemoveResource(const std::string& resourceType, float amount);

        /**
         * @brief Check if village has enough resources
         * @param requirements Resource requirements
         * @return true if has enough, false otherwise
         */
        bool HasResources(const std::unordered_map<std::string, float>& requirements) const;

        // Economy and trading

        /**
         * @brief Get village wealth
         * @return Village wealth
         */
        float GetWealth() const { return m_data.wealth; }

        /**
         * @brief Add wealth to village
         * @param amount Amount to add
         */
        void AddWealth(float amount);

        /**
         * @brief Remove wealth from village
         * @param amount Amount to remove
         * @return Actual amount removed
         */
        float RemoveWealth(float amount);

        /**
         * @brief Trade with another village
         * @param otherVillage Other village
         * @param tradeDeal Trade deal
         * @return true if trade successful, false otherwise
         */
        bool TradeWith(Village* otherVillage, const std::unordered_map<std::string, float>& tradeDeal);

        // Defense and security

        /**
         * @brief Check if village is under attack
         * @return true if under attack, false otherwise
         */
        bool IsUnderAttack() const { return m_data.underAttack; }

        /**
         * @brief Set attack status
         * @param underAttack Attack status
         */
        void SetUnderAttack(bool underAttack);

        /**
         * @brief Get security level
         * @return Security level
         */
        float GetSecurityLevel() const { return m_data.securityLevel; }

        /**
         * @brief Train guards
         * @param count Number of guards to train
         * @return Actual number trained
         */
        int TrainGuards(int count);

        /**
         * @brief Call for help from nearby villages
         * @return true if help called, false otherwise
         */
        bool CallForHelp();

        // Events and social

        /**
         * @brief Trigger village event
         * @param event Event to trigger
         * @return true if triggered, false otherwise
         */
        bool TriggerEvent(const VillageEvent& event);

        /**
         * @brief Get active events
         * @return Vector of active events
         */
        const std::vector<std::string>& GetActiveEvents() const { return m_data.activeEvents; }

        /**
         * @brief Get village happiness
         * @return Happiness level
         */
        float GetHappiness() const { return m_data.happiness; }

        /**
         * @brief Get village reputation
         * @return Reputation level
         */
        float GetReputation() const { return m_data.reputation; }

        // Utility functions

        /**
         * @brief Check if position is in village
         * @param position World position
         * @return true if in village, false otherwise
         */
        bool IsPositionInVillage(const glm::vec3& position) const;

        /**
         * @brief Find nearest building of type
         * @param position Search position
         * @param type Building type
         * @return Nearest building or nullptr
         */
        BuildingData* FindNearestBuilding(const glm::vec3& position, BuildingType type);

        /**
         * @brief Get village debug info
         * @return Debug information string
         */
        std::string GetDebugInfo() const;

        /**
         * @brief Validate village state
         * @return true if valid, false otherwise
         */
        bool Validate() const;

    private:
        /**
         * @brief Update village simulation
         * @param deltaTime Time elapsed
         */
        void UpdateSimulation(double deltaTime);

        /**
         * @brief Update village population
         * @param deltaTime Time elapsed
         */
        void UpdatePopulation(double deltaTime);

        /**
         * @brief Update village economy
         * @param deltaTime Time elapsed
         */
        void UpdateEconomy(double deltaTime);

        /**
         * @brief Update village defense
         * @param deltaTime Time elapsed
         */
        void UpdateDefense(double deltaTime);

        /**
         * @brief Update village events
         * @param deltaTime Time elapsed
         */
        void UpdateEvents(double deltaTime);

        /**
         * @brief Handle village attack
         * @param attacker Attacking entity
         */
        void HandleAttack(Mob* attacker);

        /**
         * @brief Handle village destruction
         */
        void HandleDestruction();

        /**
         * @brief Generate village name
         * @return Generated name
         */
        std::string GenerateVillageName() const;

        VillageData m_data;                          ///< Village data
        World* m_world;                              ///< Game world
        bool m_isInitialized;                        ///< Village is initialized
        double m_lastUpdateTime;                     ///< Last update timestamp
    };

    /**
     * @class VillageSystem
     * @brief Main village management system
     *
     * VillageSystem manages all villages in the game world, handling generation,
     * simulation, interactions, and coordination between villages. It creates
     * a living, breathing world with complex social structures and economies.
     */
    class VillageSystem {
    public:
        /**
         * @brief Constructor
         * @param config Village system configuration
         */
        explicit VillageSystem(const VillageConfig& config);

        /**
         * @brief Destructor
         */
        ~VillageSystem();

        /**
         * @brief Deleted copy constructor
         */
        VillageSystem(const VillageSystem&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        VillageSystem& operator=(const VillageSystem&) = delete;

        // System lifecycle

        /**
         * @brief Initialize village system
         * @param world Game world
         * @param proceduralGenerator Procedural generator
         * @return true if successful, false otherwise
         */
        bool Initialize(World* world, ProceduralGenerator* proceduralGenerator);

        /**
         * @brief Shutdown village system
         */
        void Shutdown();

        /**
         * @brief Update village system
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        // Village generation

        /**
         * @brief Generate village at position
         * @param position Generation position
         * @param type Village type
         * @return Generated village or nullptr
         */
        Village* GenerateVillage(const glm::vec3& position, VillageType type = VillageType::Rural);

        /**
         * @brief Generate village in biome
         * @param biome Biome name
         * @param center Village center
         * @return Generated village or nullptr
         */
        Village* GenerateVillageInBiome(const std::string& biome, const glm::vec3& center);

        /**
         * @brief Generate random village
         * @return Generated village or nullptr
         */
        Village* GenerateRandomVillage();

        // Village management

        /**
         * @brief Get village by ID
         * @param villageId Village ID
         * @return Village instance or nullptr
         */
        Village* GetVillage(uint32_t villageId) const;

        /**
         * @brief Get village at position
         * @param position World position
         * @return Village at position or nullptr
         */
        Village* GetVillageAtPosition(const glm::vec3& position) const;

        /**
         * @brief Get all villages
         * @return Vector of all villages
         */
        std::vector<Village*> GetAllVillages() const;

        /**
         * @brief Get villages by type
         * @param type Village type
         * @return Vector of villages of type
         */
        std::vector<Village*> GetVillagesByType(VillageType type) const;

        /**
         * @brief Get villages in radius
         * @param position Center position
         * @param radius Search radius
         * @return Vector of villages in radius
         */
        std::vector<Village*> GetVillagesInRadius(const glm::vec3& position, float radius) const;

        /**
         * @brief Destroy village
         * @param villageId Village ID
         * @return true if destroyed, false otherwise
         */
        bool DestroyVillage(uint32_t villageId);

        // System configuration

        /**
         * @brief Get village configuration
         * @return Current configuration
         */
        const VillageConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set village configuration
         * @param config New configuration
         */
        void SetConfig(const VillageConfig& config);

        /**
         * @brief Get village metrics
         * @return Performance metrics
         */
        const VillageMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

        // Village events

        /**
         * @brief Register village event
         * @param event Village event
         * @return true if registered, false otherwise
         */
        bool RegisterEvent(const VillageEvent& event);

        /**
         * @brief Unregister village event
         * @param eventId Event ID
         * @return true if unregistered, false otherwise
         */
        bool UnregisterEvent(const std::string& eventId);

        /**
         * @brief Trigger random event in village
         * @param villageId Village ID
         * @return true if event triggered, false otherwise
         */
        bool TriggerRandomEvent(uint32_t villageId);

        // Inter-village relations

        /**
         * @brief Establish trade route between villages
         * @param village1Id First village ID
         * @param village2Id Second village ID
         * @return true if established, false otherwise
         */
        bool EstablishTradeRoute(uint32_t village1Id, uint32_t village2Id);

        /**
         * @brief Break trade route between villages
         * @param village1Id First village ID
         * @param village2Id Second village ID
         * @return true if broken, false otherwise
         */
        bool BreakTradeRoute(uint32_t village1Id, uint32_t village2Id);

        // Debug and visualization

        /**
         * @brief Enable debug drawing
         * @param enabled Enable state
         * @return Previous state
         */
        bool SetDebugDrawingEnabled(bool enabled);

        /**
         * @brief Check if debug drawing is enabled
         * @return true if enabled, false otherwise
         */
        bool IsDebugDrawingEnabled() const { return m_config.enableVillageExpansion; }

        /**
         * @brief Get debug render data
         * @return Debug render data
         */
        const std::vector<DebugRenderData>& GetDebugRenderData() const;

        /**
         * @brief Clear debug render data
         */
        void ClearDebugRenderData();

        // World integration

        /**
         * @brief Get world the village system is in
         * @return World instance
         */
        World* GetWorld() const { return m_world; }

        /**
         * @brief Set world for village system
         * @param world New world
         */
        void SetWorld(World* world);

        // Utility functions

        /**
         * @brief Validate village system
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Get system status report
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

        /**
         * @brief Optimize village system performance
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> OptimizeSystem();

    private:
        /**
         * @brief Initialize village generation
         * @return true if successful, false otherwise
         */
        bool InitializeGeneration();

        /**
         * @brief Update all villages
         * @param deltaTime Time elapsed
         */
        void UpdateVillages(double deltaTime);

        /**
         * @brief Update inter-village relations
         * @param deltaTime Time elapsed
         */
        void UpdateRelations(double deltaTime);

        /**
         * @brief Generate village layout
         * @param village Village to generate for
         * @return true if generated, false otherwise
         */
        bool GenerateVillageLayout(Village* village);

        /**
         * @brief Place buildings in village
         * @param village Village to place buildings in
         * @return Number of buildings placed
         */
        size_t PlaceBuildings(Village* village);

        /**
         * @brief Generate road network
         * @param village Village to generate roads for
         * @return true if generated, false otherwise
         */
        bool GenerateRoadNetwork(Village* village);

        /**
         * @brief Spawn village NPCs
         * @param village Village to spawn NPCs in
         * @return Number of NPCs spawned
         */
        size_t SpawnNPCs(Village* village);

        /**
         * @brief Update village metrics
         * @param deltaTime Time elapsed
         */
        void UpdateMetrics(double deltaTime);

        /**
         * @brief Collect debug render data
         */
        void CollectDebugRenderData();

        /**
         * @brief Handle village system errors
         * @param error Error message
         */
        void HandleSystemError(const std::string& error);

        // Village system data
        VillageConfig m_config;                        ///< System configuration
        VillageMetrics m_metrics;                      ///< Performance metrics

        // Village management
        std::unordered_map<uint32_t, std::unique_ptr<Village>> m_villages; ///< Active villages
        mutable std::shared_mutex m_villageMutex;      ///< Village synchronization

        // Event system
        std::unordered_map<std::string, VillageEvent> m_registeredEvents; ///< Registered events
        mutable std::shared_mutex m_eventMutex;        ///< Event synchronization

        // World integration
        World* m_world;                                ///< Game world
        ProceduralGenerator* m_proceduralGenerator;    ///< Procedural generator

        // Debug data
        std::vector<DebugRenderData> m_debugRenderData;
        mutable std::shared_mutex m_debugMutex;        ///< Debug synchronization

        // State flags
        bool m_isInitialized;                          ///< System is initialized
        double m_lastUpdateTime;                       ///< Last update timestamp
        double m_creationTime;                         ///< System creation timestamp

        static std::atomic<uint32_t> s_nextVillageId;  ///< Next village ID counter
        static std::atomic<uint32_t> s_nextBuildingId; ///< Next building ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_AI_VILLAGE_SYSTEM_HPP
