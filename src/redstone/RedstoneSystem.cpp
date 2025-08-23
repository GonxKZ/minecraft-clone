/**
 * @file RedstoneSystem.cpp
 * @brief VoxelCraft Redstone System - Main Redstone Management Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "RedstoneSystem.hpp"
#include "../world/World.hpp"
#include <algorithm>
#include <iostream>

namespace VoxelCraft {

RedstoneSystem& RedstoneSystem::GetInstance() {
    static RedstoneSystem instance;
    return instance;
}

bool RedstoneSystem::Initialize(World* world, const RedstoneSystemConfig& config) {
    if (!world) {
        return false;
    }

    m_world = world;
    m_config = config;
    m_lastUpdateTime = std::chrono::steady_clock::now();

    // Reset statistics
    m_stats = RedstoneSystemStats();

    std::cout << "RedstoneSystem initialized successfully" << std::endl;
    return true;
}

void RedstoneSystem::Shutdown() {
    Clear();

    m_world = nullptr;
    m_stats = RedstoneSystemStats();

    std::cout << "RedstoneSystem shutdown" << std::endl;
}

void RedstoneSystem::Update(float deltaTime) {
    if (!m_world || !m_config.enableRedstone) {
        return;
    }

    auto startTime = std::chrono::steady_clock::now();

    m_updateTimer += deltaTime;
    if (m_updateTimer >= m_config.updateInterval) {
        m_updateTimer = 0.0f;

        // Update components
        UpdateComponents(m_config.updateInterval);

        // Process power updates
        ProcessPowerUpdates();

        // Update circuits
        UpdateCircuits(m_config.updateInterval);

        // Cleanup if needed
        if (m_components.size() > 10000) { // Arbitrary large number
            CleanupInactiveComponents();
        }

        // Update statistics
        auto endTime = std::chrono::steady_clock::now();
        float updateTime = std::chrono::duration<float>(endTime - startTime).count() * 1000.0f;
        m_stats.averageUpdateTime = (m_stats.averageUpdateTime + updateTime) * 0.5f;
        m_stats.componentUpdatesPerSecond = static_cast<int>(m_components.size() / m_config.updateInterval);
    }
}

bool RedstoneSystem::AddComponent(std::shared_ptr<RedstoneComponent> component) {
    if (!component || !m_world) {
        return false;
    }

    const glm::ivec3& position = component->GetPosition();

    std::unique_lock<std::shared_mutex> lock(m_systemMutex);

    // Check if position is already occupied
    if (m_components.find(position) != m_components.end()) {
        return false;
    }

    // Add component
    m_components[position] = component;
    m_stats.totalComponents++;

    // Trigger event
    TriggerEvent("component_added", position);

    return true;
}

bool RedstoneSystem::RemoveComponent(const glm::ivec3& position) {
    std::unique_lock<std::shared_mutex> lock(m_systemMutex);

    auto it = m_components.find(position);
    if (it == m_components.end()) {
        return false;
    }

    // Get component before removal
    auto component = it->second;

    // Remove from storage
    m_components.erase(it);
    m_stats.totalComponents--;

    // Update power at neighboring positions
    auto neighbors = component->GetConnectedComponents();
    for (const auto& neighborPos : neighbors) {
        UpdatePower(neighborPos, 0, position);
    }

    // Trigger event
    TriggerEvent("component_removed", position);

    return true;
}

std::shared_ptr<RedstoneComponent> RedstoneSystem::GetComponent(const glm::ivec3& position) const {
    std::shared_lock<std::shared_mutex> lock(m_systemMutex);

    auto it = m_components.find(position);
    return (it != m_components.end()) ? it->second : nullptr;
}

int RedstoneSystem::GetPowerLevel(const glm::ivec3& position, const glm::ivec3& direction) const {
    auto component = GetComponent(position);
    if (component) {
        return component->GetPowerLevel();
    }

    return 0;
}

bool RedstoneSystem::ProvidesStrongPower(const glm::ivec3& position, const glm::ivec3& direction) const {
    auto component = GetComponent(position);
    if (component) {
        return component->ProvidesStrongPower();
    }

    return false;
}

bool RedstoneSystem::ProvidesWeakPower(const glm::ivec3& position, const glm::ivec3& direction) const {
    auto component = GetComponent(position);
    if (component) {
        return component->ProvidesWeakPower();
    }

    return false;
}

void RedstoneSystem::UpdatePower(const glm::ivec3& position, int powerLevel, const glm::ivec3& sourcePosition) {
    auto component = GetComponent(position);
    if (component) {
        int oldPower = component->GetPowerLevel();
        if (oldPower != powerLevel) {
            component->SetPowerLevel(powerLevel);

            // Queue power update for propagation
            m_powerUpdateQueue.push({position, powerLevel});
            m_updatedPositions.insert(position);

            m_stats.powerUpdatesPerSecond++;
            m_stats.signalPropagationEvents++;
        }
    }
}

std::shared_ptr<RedstoneComponent> RedstoneSystem::CreateComponent(RedstoneType type, const glm::ivec3& position) {
    if (!m_world) {
        return nullptr;
    }

    std::shared_ptr<RedstoneComponent> component;

    switch (type) {
        case RedstoneType::WIRE:
            component = std::make_shared<RedstoneWire>(position, m_world);
            break;
        case RedstoneType::TORCH:
            component = std::make_shared<RedstoneTorch>(position, m_world);
            break;
        case RedstoneType::REPEATER:
            component = std::make_shared<RedstoneRepeater>(position, m_world);
            break;
        case RedstoneType::COMPARATOR:
            component = std::make_shared<RedstoneComparator>(position, m_world);
            break;
        case RedstoneType::PISTON:
            component = std::make_shared<RedstonePiston>(position, m_world, false);
            break;
        case RedstoneType::STICKY_PISTON:
            component = std::make_shared<RedstonePiston>(position, m_world, true);
            break;
        default:
            // Other component types would be implemented here
            return nullptr;
    }

    if (AddComponent(component)) {
        return component;
    }

    return nullptr;
}

void RedstoneSystem::PropagatePower(const glm::ivec3& startPosition, int maxDepth) {
    if (maxDepth <= 0) {
        return;
    }

    PropagatePowerRecursive(startPosition, 0, startPosition, maxDepth);
}

void RedstoneSystem::PropagatePowerRecursive(const glm::ivec3& position, int powerLevel,
                                           const glm::ivec3& sourcePosition, int depth) {
    if (depth <= 0) {
        return;
    }

    auto component = GetComponent(position);
    if (!component) {
        return;
    }

    // Calculate power level at this position
    int currentPower = component->GetPowerLevel();
    int newPower = std::max(0, powerLevel - 1); // Power decreases by 1 per block

    if (newPower != currentPower) {
        UpdatePower(position, newPower, sourcePosition);
    }

    // Propagate to connected components
    if (newPower > 0) {
        auto connections = component->GetConnectedComponents();
        for (const auto& connectionPos : connections) {
            if (connectionPos != sourcePosition) {
                PropagatePowerRecursive(connectionPos, newPower, position, depth - 1);
            }
        }
    }
}

std::vector<RedstoneCircuit> RedstoneSystem::FindCircuits(const glm::ivec3& startPosition) {
    std::vector<RedstoneCircuit> circuits;

    std::set<glm::ivec3> visited;
    RedstoneCircuit currentCircuit;

    TraceCircuitConnections(startPosition, currentCircuit, visited, 0);

    if (!currentCircuit.components.empty()) {
        circuits.push_back(currentCircuit);
    }

    return circuits;
}

bool RedstoneSystem::OptimizeCircuit(RedstoneCircuit& circuit) {
    if (!m_config.enableCircuitOptimization) {
        return false;
    }

    bool optimized = false;

    // Simple optimization: consolidate adjacent wires
    auto& components = circuit.components;
    for (size_t i = 0; i < components.size(); ++i) {
        if (components[i]->GetType() == RedstoneType::WIRE) {
            // Check if this wire can be consolidated with neighbors
            // This is a simplified optimization - more complex ones would be implemented
            optimized = true;
            m_stats.circuitOptimizations++;
        }
    }

    return optimized;
}

void RedstoneSystem::Clear() {
    std::unique_lock<std::shared_mutex> lock(m_systemMutex);

    m_components.clear();
    m_circuits.clear();

    while (!m_powerUpdateQueue.empty()) {
        m_powerUpdateQueue.pop();
    }

    m_updatedPositions.clear();
    m_stats = RedstoneSystemStats();

    TriggerEvent("system_cleared", glm::ivec3(0, 0, 0));
}

void RedstoneSystem::RegisterEventCallback(std::function<void(const std::string&, const glm::ivec3&)> callback) {
    if (callback) {
        m_eventCallbacks.push_back(callback);
    }
}

std::vector<std::shared_ptr<RedstoneComponent>> RedstoneSystem::GetActiveComponents() const {
    std::shared_lock<std::shared_mutex> lock(m_systemMutex);

    std::vector<std::shared_ptr<RedstoneComponent>> components;
    components.reserve(m_components.size());

    for (const auto& pair : m_components) {
        components.push_back(pair.second);
    }

    return components;
}

std::vector<std::shared_ptr<RedstoneComponent>> RedstoneSystem::GetComponentsByType(RedstoneType type) const {
    std::shared_lock<std::shared_mutex> lock(m_systemMutex);

    std::vector<std::shared_ptr<RedstoneComponent>> components;

    for (const auto& pair : m_components) {
        if (pair.second->GetType() == type) {
            components.push_back(pair.second);
        }
    }

    return components;
}

void RedstoneSystem::UpdateComponents(float deltaTime) {
    std::shared_lock<std::shared_mutex> lock(m_systemMutex);

    for (const auto& pair : m_components) {
        if (pair.second) {
            pair.second->Update(deltaTime);
        }
    }
}

void RedstoneSystem::UpdateCircuits(float deltaTime) {
    // Update circuit analysis if needed
    // This would be more complex in a full implementation

    m_stats.activeCircuits = static_cast<int>(m_circuits.size());
}

void RedstoneSystem::ProcessPowerUpdates() {
    // Process queued power updates
    while (!m_powerUpdateQueue.empty()) {
        auto [position, powerLevel] = m_powerUpdateQueue.front();
        m_powerUpdateQueue.pop();

        // Propagate power from this position
        if (m_config.enablePowerPropagationDelay) {
            PropagatePower(position, m_config.maxCircuitDepth);
        }
    }

    m_updatedPositions.clear();
}

void RedstoneSystem::CleanupInactiveComponents() {
    std::unique_lock<std::shared_mutex> lock(m_systemMutex);

    // Remove components with zero power that haven't been updated recently
    auto now = std::chrono::steady_clock::now();

    std::vector<glm::ivec3> toRemove;
    for (const auto& pair : m_components) {
        auto component = pair.second;
        if (component->GetPowerLevel() == 0) {
            auto timeSinceUpdate = std::chrono::duration_cast<std::chrono::seconds>(
                now - component->GetLastUpdateTime()).count();

            if (timeSinceUpdate > 60) { // 1 minute
                toRemove.push_back(pair.first);
            }
        }
    }

    for (const auto& pos : toRemove) {
        RemoveComponent(pos);
    }
}

void RedstoneSystem::TraceCircuitConnections(const glm::ivec3& position, RedstoneCircuit& circuit,
                                           std::set<glm::ivec3>& visited, int depth) {
    if (depth >= m_config.maxCircuitDepth || visited.count(position)) {
        return;
    }

    visited.insert(position);
    m_stats.maxCircuitDepth = std::max(m_stats.maxCircuitDepth, depth);

    auto component = GetComponent(position);
    if (component) {
        circuit.components.push_back(component);

        auto connections = component->GetConnectedComponents();
        for (const auto& connectionPos : connections) {
            RedstoneConnection connection;
            connection.fromPos = position;
            connection.toPos = connectionPos;
            connection.powerLevel = component->GetPowerLevel();
            circuit.connections.push_back(connection);

            TraceCircuitConnections(connectionPos, circuit, visited, depth + 1);
        }
    }
}

void RedstoneSystem::TriggerEvent(const std::string& event, const glm::ivec3& position) {
    for (auto& callback : m_eventCallbacks) {
        if (callback) {
            callback(event, position);
        }
    }
}

// RedstoneWireNetwork implementation
RedstoneWireNetwork::RedstoneWireNetwork() {
}

RedstoneWireNetwork::~RedstoneWireNetwork() {
}

void RedstoneWireNetwork::AddWire(const glm::ivec3& position, int powerLevel) {
    std::unique_lock<std::shared_mutex> lock(m_networkMutex);
    m_wires[position] = powerLevel;
}

void RedstoneWireNetwork::RemoveWire(const glm::ivec3& position) {
    std::unique_lock<std::shared_mutex> lock(m_networkMutex);
    m_wires.erase(position);
}

void RedstoneWireNetwork::UpdatePowerLevels(float deltaTime) {
    // Simple power propagation through wire network
    RecalculatePowerLevels();
}

int RedstoneWireNetwork::GetPowerAt(const glm::ivec3& position) const {
    std::shared_lock<std::shared_mutex> lock(m_networkMutex);
    auto it = m_wires.find(position);
    return (it != m_wires.end()) ? it->second : 0;
}

void RedstoneWireNetwork::RecalculatePowerLevels() {
    // Find power sources first
    FindPowerSources();

    // Propagate power from sources
    for (const auto& sourcePos : m_powerSources) {
        int sourcePower = m_wires[sourcePos];
        // Simple flood fill to propagate power
        // In a full implementation, this would be more sophisticated
    }
}

void RedstoneWireNetwork::FindPowerSources() {
    m_powerSources.clear();

    for (const auto& pair : m_wires) {
        const glm::ivec3& position = pair.first;
        int powerLevel = pair.second;

        // Check if this wire is connected to a power source
        // This would check adjacent non-wire components
        if (powerLevel > 0) {
            // Simple check - if wire has power, consider it a source for now
            m_powerSources.push_back(position);
        }
    }
}

// RedstoneDebugger implementation
RedstoneDebugger::RedstoneDebugger(RedstoneSystem* system)
    : m_system(system) {
}

RedstoneDebugger::~RedstoneDebugger() {
}

void RedstoneDebugger::RenderDebugInfo() {
    if (!m_debugMode || !m_system) return;

    // This would render debug visualizations
    // Implementation would depend on the rendering system
}

std::string RedstoneDebugger::GetCircuitDebugInfo(const RedstoneCircuit& circuit) const {
    std::stringstream ss;

    ss << "Circuit Analysis:\n";
    ss << "Components: " << circuit.components.size() << "\n";
    ss << "Connections: " << circuit.connections.size() << "\n";
    ss << "Tick Delay: " << circuit.tickDelay << "\n";
    ss << "Active: " << (circuit.isActive ? "Yes" : "No") << "\n";

    return ss.str();
}

void RedstoneDebugger::HighlightPowerFlow(const glm::ivec3& startPosition, float duration) {
    PowerFlowHighlight highlight;
    highlight.position = startPosition;
    highlight.duration = duration;
    highlight.startTime = std::chrono::steady_clock::now();

    m_highlights.push_back(highlight);
}

std::vector<int> RedstoneDebugger::GetPowerHistory(const glm::ivec3& position) const {
    auto it = m_powerHistory.find(position);
    return (it != m_powerHistory.end()) ? it->second : std::vector<int>();
}

void RedstoneDebugger::UpdateHighlights() {
    auto now = std::chrono::steady_clock::now();

    m_highlights.erase(std::remove_if(m_highlights.begin(), m_highlights.end(),
        [now](const PowerFlowHighlight& highlight) {
            auto elapsed = std::chrono::duration<float>(now - highlight.startTime).count();
            return elapsed >= highlight.duration;
        }), m_highlights.end());
}

void RedstoneDebugger::RecordPowerHistory(const glm::ivec3& position, int powerLevel) {
    if (m_powerHistory[position].size() > 100) { // Keep last 100 readings
        m_powerHistory[position].erase(m_powerHistory[position].begin());
    }
    m_powerHistory[position].push_back(powerLevel);
}

} // namespace VoxelCraft
