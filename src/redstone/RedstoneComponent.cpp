/**
 * @file RedstoneComponent.cpp
 * @brief VoxelCraft Redstone System - Redstone Components Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "RedstoneComponent.hpp"
#include "../world/World.hpp"
#include <algorithm>
#include <cmath>

namespace VoxelCraft {

// RedstoneComponent base implementation
RedstoneComponent::RedstoneComponent(RedstoneType type, const glm::ivec3& position, World* world)
    : m_type(type)
    , m_position(position)
    , m_world(world)
    , m_powerLevel(0)
    , m_state(RedstoneState::OFF)
    , m_facingDirection(0, 0, 1)
    , m_lastUpdateTime(std::chrono::steady_clock::now())
    , m_updateTimer(0.0f)
{
}

void RedstoneComponent::SetPowerLevel(int power) {
    power = std::max(0, std::min(15, power)); // Clamp to 0-15

    if (power != m_powerLevel) {
        int oldPower = m_powerLevel;
        m_powerLevel = power;
        UpdatePowerState();

        // Notify neighbors of power change
        NotifyNeighbors();

        // Call subclass-specific power change handling
        OnPowerLevelChanged(oldPower, power);
    }
}

std::vector<glm::ivec3> RedstoneComponent::GetConnectedComponents() const {
    std::vector<glm::ivec3> connections;

    // Check all 6 adjacent positions
    const glm::ivec3 directions[] = {
        glm::ivec3(1, 0, 0), glm::ivec3(-1, 0, 0),
        glm::ivec3(0, 1, 0), glm::ivec3(0, -1, 0),
        glm::ivec3(0, 0, 1), glm::ivec3(0, 0, -1)
    };

    for (const auto& dir : directions) {
        glm::ivec3 neighborPos = m_position + dir;
        if (CanConnectTo(neighborPos)) {
            connections.push_back(neighborPos);
        }
    }

    return connections;
}

bool RedstoneComponent::CanConnectTo(const glm::ivec3& otherPos) const {
    // Default implementation - can connect to adjacent positions
    glm::ivec3 diff = otherPos - m_position;
    int manhattanDistance = abs(diff.x) + abs(diff.y) + abs(diff.z);
    return manhattanDistance == 1;
}

void RedstoneComponent::UpdatePowerState() {
    if (m_powerLevel > 0) {
        if (m_state == RedstoneState::OFF) {
            m_state = RedstoneState::POWERING;
        } else if (m_state == RedstoneState::POWERING) {
            m_state = RedstoneState::ON;
        }
    } else {
        if (m_state == RedstoneState::ON) {
            m_state = RedstoneState::UNPOWERING;
        } else if (m_state == RedstoneState::UNPOWERING) {
            m_state = RedstoneState::OFF;
        }
    }
}

void RedstoneComponent::NotifyNeighbors() {
    auto connectedComponents = GetConnectedComponents();
    for (const auto& neighborPos : connectedComponents) {
        // Notify neighbor component of power change
        OnNeighborUpdate(neighborPos, m_powerLevel, m_powerLevel);
    }
}

void RedstoneComponent::OnPowerLevelChanged(int oldPower, int newPower) {
    // Base implementation - subclasses can override
}

// RedstoneWire implementation
RedstoneWire::RedstoneWire(const glm::ivec3& position, World* world)
    : RedstoneComponent(RedstoneType::WIRE, position, world)
{
}

void RedstoneWire::Update(float deltaTime) {
    m_updateTimer += deltaTime;
    if (m_updateTimer >= 0.05f) { // Update every 50ms (1 redstone tick)
        m_updateTimer = 0.0f;

        int newPower = CalculatePowerLevel();
        if (newPower != m_powerLevel) {
            SetPowerLevel(newPower);
        }
    }
}

std::vector<glm::ivec3> RedstoneWire::GetConnectedComponents() const {
    std::vector<glm::ivec3> connections;

    // Check horizontal directions first
    const glm::ivec3 horizontalDirs[] = {
        glm::ivec3(1, 0, 0), glm::ivec3(-1, 0, 0),
        glm::ivec3(0, 0, 1), glm::ivec3(0, 0, -1)
    };

    for (const auto& dir : horizontalDirs) {
        glm::ivec3 neighborPos = m_position + dir;
        if (IsWireAt(neighborPos)) {
            connections.push_back(neighborPos);
        }
    }

    // Check upward connection
    glm::ivec3 upPos = m_position + glm::ivec3(0, 1, 0);
    if (IsWireAt(upPos)) {
        connections.push_back(upPos);
    }

    return connections;
}

bool RedstoneWire::CanConnectTo(const glm::ivec3& otherPos) const {
    glm::ivec3 diff = otherPos - m_position;
    int manhattanDistance = abs(diff.x) + abs(diff.y) + abs(diff.z);

    // Wires can only connect horizontally or upward
    if (manhattanDistance != 1) return false;
    if (diff.y < 0) return false; // Can't connect downward

    return IsWireAt(otherPos);
}

int RedstoneWire::CalculatePowerLevel() {
    int maxPower = 0;

    // Get power from all connected components
    auto connections = GetConnectedComponents();
    for (const auto& connectionPos : connections) {
        int wirePower = GetWirePowerAt(connectionPos);
        if (wirePower > maxPower) {
            maxPower = wirePower;
        }
    }

    // Wires lose 1 power level per block
    return std::max(0, maxPower - 1);
}

bool RedstoneWire::IsWireAt(const glm::ivec3& pos) const {
    if (!m_world) return false;
    int blockID = m_world->GetBlock(pos.x, pos.y, pos.z);
    return blockID == 55; // Redstone wire block ID
}

int RedstoneWire::GetWirePowerAt(const glm::ivec3& pos) const {
    // This would need to query the redstone system for the power level at that position
    // For now, return a placeholder value
    return 15; // Maximum power
}

// RedstoneTorch implementation
RedstoneTorch::RedstoneTorch(const glm::ivec3& position, World* world)
    : RedstoneComponent(RedstoneType::TORCH, position, world)
    , m_lit(true)
{
    m_powerLevel = 15; // Torches output 15 by default
}

void RedstoneTorch::Update(float deltaTime) {
    m_updateTimer += deltaTime;
    if (m_updateTimer >= 0.05f) { // Update every 50ms
        m_updateTimer = 0.0f;

        int newPower = CalculatePowerLevel();
        if (newPower != m_powerLevel) {
            SetPowerLevel(newPower);
        }
    }
}

void RedstoneTorch::SetPowerLevel(int power) {
    // Torches invert their input - if powered, they turn off
    bool shouldBeLit = (power == 0);
    m_lit = shouldBeLit;

    RedstoneComponent::SetPowerLevel(shouldBeLit ? 15 : 0);
}

int RedstoneTorch::CalculatePowerLevel() {
    // Check if torch is receiving power from below
    glm::ivec3 belowPos = m_position + glm::ivec3(0, -1, 0);
    if (m_world) {
        // This would need to check the redstone power system
        // For now, assume no power from below
        int blockPower = 0; // Placeholder
        return blockPower > 0 ? 0 : 15;
    }

    return 15; // Default torch power
}

// RedstoneRepeater implementation
RedstoneRepeater::RedstoneRepeater(const glm::ivec3& position, World* world)
    : RedstoneComponent(RedstoneType::REPEATER, position, world)
    , m_delayTicks(1)
    , m_locked(false)
    , m_inputPower(0)
{
}

void RedstoneRepeater::Update(float deltaTime) {
    m_updateTimer += deltaTime;
    if (m_updateTimer >= 0.05f) { // Update every 50ms
        m_updateTimer = 0.0f;

        // Check if repeater is locked (side input has power)
        UpdateLockState();

        if (!m_locked) {
            int newPower = CalculatePowerLevel();
            if (newPower != m_powerLevel) {
                SetPowerLevel(newPower);
            }
        }
    }
}

void RedstoneRepeater::SetPowerLevel(int power) {
    if (power != m_inputPower) {
        m_inputPower = power;
        m_lastInputChange = std::chrono::steady_clock::now();

        // Schedule power change after delay
        // This would need to be implemented with a proper scheduling system
    }
}

void RedstoneRepeater::OnNeighborUpdate(const glm::ivec3& neighborPos, int oldPower, int newPower) {
    // Check if neighbor is the input side
    glm::ivec3 inputDir = -m_facingDirection; // Input is opposite to facing direction
    glm::ivec3 expectedInputPos = m_position + inputDir;

    if (neighborPos == expectedInputPos) {
        SetPowerLevel(newPower);
    }

    // Check for side inputs (locking)
    UpdateLockState();
}

int RedstoneRepeater::CalculatePowerLevel() {
    // If repeater has input power and delay has passed, output power
    auto now = std::chrono::steady_clock::now();
    auto timeSinceInput = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - m_lastInputChange).count();

    int delayMs = m_delayTicks * 50; // 50ms per tick

    if (m_inputPower > 0 && timeSinceInput >= delayMs) {
        return 15; // Repeater outputs 15 if input is powered
    }

    return 0;
}

void RedstoneRepeater::UpdateLockState() {
    // Check side inputs for locking
    const glm::ivec3 sideDirs[] = {
        glm::ivec3(-m_facingDirection.z, 0, m_facingDirection.x),  // Right
        glm::ivec3(m_facingDirection.z, 0, -m_facingDirection.x)   // Left
    };

    m_locked = false;
    for (const auto& sideDir : sideDirs) {
        glm::ivec3 sidePos = m_position + sideDir;
        if (m_world) {
            // Check if side has power
            // This would need to query the redstone system
            int sidePower = 0; // Placeholder
            if (sidePower > 0) {
                m_locked = true;
                break;
            }
        }
    }
}

// RedstoneComparator implementation
RedstoneComparator::RedstoneComparator(const glm::ivec3& position, World* world)
    : RedstoneComponent(RedstoneType::COMPARATOR, position, world)
    , m_mode(ComparatorMode::COMPARE)
    , m_inputPower1(0)
    , m_inputPower2(0)
{
}

void RedstoneComparator::Update(float deltaTime) {
    m_updateTimer += deltaTime;
    if (m_updateTimer >= 0.05f) {
        m_updateTimer = 0.0f;

        int newPower = CalculatePowerLevel();
        if (newPower != m_powerLevel) {
            SetPowerLevel(newPower);
        }
    }
}

void RedstoneComparator::OnNeighborUpdate(const glm::ivec3& neighborPos, int oldPower, int newPower) {
    // Update input powers based on neighbor positions
    glm::ivec3 backPos = m_position - m_facingDirection;
    glm::ivec3 side1Pos = m_position + glm::ivec3(-m_facingDirection.z, 0, m_facingDirection.x);
    glm::ivec3 side2Pos = m_position + glm::ivec3(m_facingDirection.z, 0, -m_facingDirection.x);

    if (neighborPos == backPos) {
        m_inputPower1 = newPower;
    } else if (neighborPos == side1Pos || neighborPos == side2Pos) {
        m_inputPower2 = newPower;
    }
}

int RedstoneComparator::CalculatePowerLevel() {
    if (m_mode == ComparatorMode::COMPARE) {
        // Compare mode: output power if back input >= side inputs
        return (m_inputPower1 >= m_inputPower2) ? m_inputPower1 : 0;
    } else {
        // Subtract mode: output back input - side inputs
        return std::max(0, m_inputPower1 - m_inputPower2);
    }
}

// RedstonePiston implementation
RedstonePiston::RedstonePiston(const glm::ivec3& position, World* world, bool sticky)
    : RedstoneComponent(RedstoneType::PISTON, position, world)
    , m_sticky(sticky)
    , m_pistonState(PistonState::RETRACTED)
    , m_extensionProgress(0.0f)
    , m_extensionSpeed(2.0f) // Blocks per second
{
}

void RedstonePiston::Update(float deltaTime) {
    // Update piston movement
    if (m_pistonState == PistonState::EXTENDING) {
        m_extensionProgress += deltaTime * m_extensionSpeed;
        if (m_extensionProgress >= 1.0f) {
            m_extensionProgress = 1.0f;
            m_pistonState = PistonState::EXTENDED;
            MoveBlocks();
        }
    } else if (m_pistonState == PistonState::RETRACTING) {
        m_extensionProgress -= deltaTime * m_extensionSpeed;
        if (m_extensionProgress <= 0.0f) {
            m_extensionProgress = 0.0f;
            m_pistonState = PistonState::RETRACTED;
            MoveBlocks();
        }
    }
}

void RedstonePiston::SetPowerLevel(int power) {
    bool isPowered = (power > 0);

    if (isPowered && m_pistonState == PistonState::RETRACTED) {
        Extend();
    } else if (!isPowered && m_pistonState == PistonState::EXTENDED) {
        Retract();
    }

    RedstoneComponent::SetPowerLevel(power);
}

void RedstonePiston::OnNeighborUpdate(const glm::ivec3& neighborPos, int oldPower, int newPower) {
    // Pistons react to power changes
    SetPowerLevel(newPower);
}

void RedstonePiston::Extend() {
    if (m_pistonState != PistonState::RETRACTED) return;
    if (!CanExtend()) return;

    m_pistonState = PistonState::EXTENDING;
    m_extensionProgress = 0.0f;
    m_lastStateChange = std::chrono::steady_clock::now();
}

void RedstonePiston::Retract() {
    if (m_pistonState != PistonState::EXTENDED) return;
    if (!CanRetract()) return;

    m_pistonState = PistonState::RETRACTING;
    m_lastStateChange = std::chrono::steady_clock::now();
}

bool RedstonePiston::CanExtend() const {
    // Check if there's space in front of the piston
    glm::ivec3 extensionPos = m_position + m_facingDirection;
    if (!m_world) return false;

    int blockID = m_world->GetBlock(extensionPos.x, extensionPos.y, extensionPos.z);
    return blockID == 0; // Air block
}

bool RedstonePiston::CanRetract() const {
    // Check if piston can retract (space behind piston)
    glm::ivec3 behindPos = m_position - m_facingDirection;
    if (!m_world) return false;

    int blockID = m_world->GetBlock(behindPos.x, behindPos.y, behindPos.z);
    return blockID == 0; // Air block
}

void RedstonePiston::MoveBlocks() {
    if (!m_world) return;

    if (m_pistonState == PistonState::EXTENDED) {
        // Move block in front of piston
        glm::ivec3 targetPos = m_position + m_facingDirection;
        glm::ivec3 newPos = targetPos + m_facingDirection;

        int blockID = m_world->GetBlock(targetPos.x, targetPos.y, targetPos.z);
        if (blockID != 0) {
            m_world->SetBlock(targetPos.x, targetPos.y, targetPos.z, 0); // Remove from current position
            m_world->SetBlock(newPos.x, newPos.y, newPos.z, blockID); // Place at new position
        }
    } else if (m_pistonState == PistonState::RETRACTED && m_sticky) {
        // Sticky piston pulls block back
        glm::ivec3 targetPos = m_position + m_facingDirection;
        glm::ivec3 blockPos = targetPos + m_facingDirection;

        int blockID = m_world->GetBlock(blockPos.x, blockPos.y, blockPos.z);
        if (blockID != 0) {
            m_world->SetBlock(blockPos.x, blockPos.y, blockPos.z, 0); // Remove from extended position
            m_world->SetBlock(targetPos.x, targetPos.y, targetPos.z, blockID); // Place at retracted position
        }
    }
}

} // namespace VoxelCraft
