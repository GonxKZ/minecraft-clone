/**
 * @file RedstoneSystem.hpp
 * @brief VoxelCraft Redstone System - Main Redstone Management
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_REDSTONE_REDSTONE_SYSTEM_HPP
#define VOXELCRAFT_REDSTONE_REDSTONE_SYSTEM_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <queue>
#include <set>
#include <functional>
#include <chrono>
#include <mutex>
#include <shared_mutex>
#include <glm/glm.hpp>

#include "RedstoneComponent.hpp"

namespace VoxelCraft {

    class World;
    class Chunk;
    struct Vec3;

    /**
     * @struct RedstoneSystemConfig
     * @brief Configuration for redstone system
     */
    struct RedstoneSystemConfig {
        bool enableRedstone = true;                    ///< Enable redstone system
        int maxComponentsPerChunk = 1000;              ///< Max redstone components per chunk
        int maxCircuitDepth = 100;                     ///< Max circuit recursion depth
        float updateInterval = 1.0f / 20.0f;           ///< Update interval (50ms for 20 TPS)
        int maxPowerLevel = 15;                        ///< Maximum redstone power level
        int minPowerLevel = 0;                         ///< Minimum redstone power level
        bool enableCircuitOptimization = true;         ///< Enable circuit optimization
        bool enablePowerPropagationDelay = true;       ///< Enable signal propagation delays
        int maxSignalRange = 64;                       ///< Maximum signal transmission range
        bool enableObserverUpdates = true;             ///< Enable observer block updates
        bool enableQuasiConnectivity = true;           ///< Enable quasi-connectivity rules
    };

    /**
     * @struct RedstoneSystemStats
     * @brief Performance statistics for redstone system
     */
    struct RedstoneSystemStats {
        int totalComponents = 0;                       ///< Total active redstone components
        int activeCircuits = 0;                        ///< Number of active circuits
        int powerUpdatesPerSecond = 0;                 ///< Power updates per second
        int componentUpdatesPerSecond = 0;             ///< Component updates per second
        float averageUpdateTime = 0.0f;                ///< Average update time in ms
        int maxCircuitDepth = 0;                       ///< Maximum circuit depth reached
        int signalPropagationEvents = 0;               ///< Signal propagation events
        int circuitOptimizations = 0;                  ///< Circuit optimizations performed
    };

    /**
     * @class RedstoneSystem
     * @brief Main redstone system managing all redstone components and circuits
     */
    class RedstoneSystem {
    public:
        /**
         * @brief Get singleton instance
         * @return RedstoneSystem instance
         */
        static RedstoneSystem& GetInstance();

        /**
         * @brief Initialize the redstone system
         * @param world World reference
         * @param config System configuration
         * @return true if successful
         */
        bool Initialize(World* world, const RedstoneSystemConfig& config);

        /**
         * @brief Shutdown the redstone system
         */
        void Shutdown();

        /**
         * @brief Update all redstone components
         * @param deltaTime Time since last update
         */
        void Update(float deltaTime);

        /**
         * @brief Add redstone component to the system
         * @param component Component to add
         * @return true if added successfully
         */
        bool AddComponent(std::shared_ptr<RedstoneComponent> component);

        /**
         * @brief Remove redstone component from the system
         * @param position Component position
         * @return true if removed successfully
         */
        bool RemoveComponent(const glm::ivec3& position);

        /**
         * @brief Get redstone component at position
         * @param position Position to check
         * @return Component pointer or nullptr
         */
        std::shared_ptr<RedstoneComponent> GetComponent(const glm::ivec3& position) const;

        /**
         * @brief Get power level at position
         * @param position Position to check
         * @param direction Direction of power check (for strong vs weak power)
         * @return Power level (0-15)
         */
        int GetPowerLevel(const glm::ivec3& position, const glm::ivec3& direction = glm::ivec3(0, 0, 0)) const;

        /**
         * @brief Check if position provides strong power
         * @param position Position to check
         * @param direction Direction to check
         * @return true if provides strong power
         */
        bool ProvidesStrongPower(const glm::ivec3& position, const glm::ivec3& direction) const;

        /**
         * @brief Check if position provides weak power
         * @param position Position to check
         * @param direction Direction to check
         * @return true if provides weak power
         */
        bool ProvidesWeakPower(const glm::ivec3& position, const glm::ivec3& direction) const;

        /**
         * @brief Update power at position
         * @param position Position to update
         * @param powerLevel New power level
         * @param sourcePosition Source of power change
         */
        void UpdatePower(const glm::ivec3& position, int powerLevel, const glm::ivec3& sourcePosition);

        /**
         * @brief Create redstone component
         * @param type Component type
         * @param position Component position
         * @return New component or nullptr if failed
         */
        std::shared_ptr<RedstoneComponent> CreateComponent(RedstoneType type, const glm::ivec3& position);

        /**
         * @brief Propagate power changes through the circuit
         * @param startPosition Starting position
         * @param maxDepth Maximum propagation depth
         */
        void PropagatePower(const glm::ivec3& startPosition, int maxDepth = 100);

        /**
         * @brief Find redstone circuits
         * @param startPosition Starting position
         * @return List of circuits found
         */
        std::vector<RedstoneCircuit> FindCircuits(const glm::ivec3& startPosition);

        /**
         * @brief Optimize redstone circuit
         * @param circuit Circuit to optimize
         * @return true if optimized
         */
        bool OptimizeCircuit(RedstoneCircuit& circuit);

        /**
         * @brief Get system configuration
         * @return Current configuration
         */
        const RedstoneSystemConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set system configuration
         * @param config New configuration
         */
        void SetConfig(const RedstoneSystemConfig& config) { m_config = config; }

        /**
         * @brief Get system statistics
         * @return Current statistics
         */
        const RedstoneSystemStats& GetStats() const { return m_stats; }

        /**
         * @brief Clear all components and reset system
         */
        void Clear();

        /**
         * @brief Register redstone event callback
         * @param callback Callback function
         */
        void RegisterEventCallback(std::function<void(const std::string&, const glm::ivec3&)> callback);

        /**
         * @brief Get all active components
         * @return Vector of active components
         */
        std::vector<std::shared_ptr<RedstoneComponent>> GetActiveComponents() const;

        /**
         * @brief Get components by type
         * @param type Component type
         * @return Vector of components of specified type
         */
        std::vector<std::shared_ptr<RedstoneComponent>> GetComponentsByType(RedstoneType type) const;

    private:
        RedstoneSystem() = default;
        ~RedstoneSystem() = default;

        // Prevent copying
        RedstoneSystem(const RedstoneSystem&) = delete;
        RedstoneSystem& operator=(const RedstoneSystem&) = delete;

        // Core functionality
        void UpdateComponents(float deltaTime);
        void UpdateCircuits(float deltaTime);
        void ProcessPowerUpdates();
        void CleanupInactiveComponents();

        // Power propagation
        void PropagatePowerRecursive(const glm::ivec3& position, int powerLevel,
                                   const glm::ivec3& sourcePosition, int depth);
        void CalculateComponentPower(std::shared_ptr<RedstoneComponent> component);

        // Circuit analysis
        RedstoneCircuit AnalyzeCircuit(const glm::ivec3& startPosition);
        void TraceCircuitConnections(const glm::ivec3& position, RedstoneCircuit& circuit,
                                   std::set<glm::ivec3>& visited, int depth);

        // Optimization
        void OptimizePowerNetwork();
        void RemoveRedundantConnections();
        void ConsolidateWireNetworks();

        // Event system
        void TriggerEvent(const std::string& event, const glm::ivec3& position);

        World* m_world = nullptr;
        RedstoneSystemConfig m_config;
        RedstoneSystemStats m_stats;

        // Component storage
        std::unordered_map<glm::ivec3, std::shared_ptr<RedstoneComponent>, std::hash<glm::ivec3::value_type>> m_components;
        std::vector<RedstoneCircuit> m_circuits;

        // Update queues
        std::queue<std::pair<glm::ivec3, int>> m_powerUpdateQueue;
        std::set<glm::ivec3> m_updatedPositions;

        // Timing
        float m_updateTimer = 0.0f;
        std::chrono::steady_clock::time_point m_lastUpdateTime;

        // Callbacks
        std::vector<std::function<void(const std::string&, const glm::ivec3&)>> m_eventCallbacks;

        // Thread safety
        mutable std::shared_mutex m_systemMutex;
    };

    /**
     * @class RedstoneWireNetwork
     * @brief Manages networks of connected redstone wires for optimization
     */
    class RedstoneWireNetwork {
    public:
        RedstoneWireNetwork();
        ~RedstoneWireNetwork();

        /**
         * @brief Add wire to network
         * @param position Wire position
         * @param powerLevel Wire power level
         */
        void AddWire(const glm::ivec3& position, int powerLevel);

        /**
         * @brief Remove wire from network
         * @param position Wire position
         */
        void RemoveWire(const glm::ivec3& position);

        /**
         * @brief Update wire power levels
         * @param deltaTime Time since last update
         */
        void UpdatePowerLevels(float deltaTime);

        /**
         * @brief Get power level at position
         * @param position Position to check
         * @return Power level
         */
        int GetPowerAt(const glm::ivec3& position) const;

        /**
         * @brief Check if network is empty
         * @return true if empty
         */
        bool IsEmpty() const { return m_wires.empty(); }

        /**
         * @brief Get network size
         * @return Number of wires in network
         */
        size_t GetSize() const { return m_wires.size(); }

    private:
        std::unordered_map<glm::ivec3, int> m_wires;
        std::vector<glm::ivec3> m_powerSources;
        mutable std::shared_mutex m_networkMutex;

        void RecalculatePowerLevels();
        void FindPowerSources();
    };

    /**
     * @class RedstoneDebugger
     * @brief Debug visualization and analysis for redstone systems
     */
    class RedstoneDebugger {
    public:
        RedstoneDebugger(RedstoneSystem* system);
        ~RedstoneDebugger();

        /**
         * @brief Enable debug mode
         * @param enable Enable state
         */
        void EnableDebugMode(bool enable) { m_debugMode = enable; }

        /**
         * @brief Render debug information
         */
        void RenderDebugInfo();

        /**
         * @brief Get circuit debug info
         * @param circuit Circuit to analyze
         * @return Debug information string
         */
        std::string GetCircuitDebugInfo(const RedstoneCircuit& circuit) const;

        /**
         * @brief Highlight power flow
         * @param startPosition Starting position
         * @param duration Highlight duration in seconds
         */
        void HighlightPowerFlow(const glm::ivec3& startPosition, float duration);

        /**
         * @brief Get power level history at position
         * @param position Position to check
         * @return Vector of power levels over time
         */
        std::vector<int> GetPowerHistory(const glm::ivec3& position) const;

    private:
        RedstoneSystem* m_system;
        bool m_debugMode = false;

        struct PowerFlowHighlight {
            glm::ivec3 position;
            float duration;
            std::chrono::steady_clock::time_point startTime;
        };

        std::vector<PowerFlowHighlight> m_highlights;
        std::unordered_map<glm::ivec3, std::vector<int>> m_powerHistory;

        void UpdateHighlights();
        void RecordPowerHistory(const glm::ivec3& position, int powerLevel);
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_REDSTONE_REDSTONE_SYSTEM_HPP
