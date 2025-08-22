/**
 * @file BlockInteraction.cpp
 * @brief VoxelCraft Block Interaction System - Player-World Interaction Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "BlockInteraction.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace VoxelCraft {

// Simple vector structures
struct Vec2 {
    float x, y;
    Vec2(float x = 0, float y = 0) : x(x), y(y) {}
};

struct Vec3 {
    float x, y, z;
    Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& other) const {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }

    Vec3 operator-(const Vec3& other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }

    Vec3 operator*(float scalar) const {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }

    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vec3 normalized() const {
        float len = length();
        if (len > 0) {
            return Vec3(x / len, y / len, z / len);
        }
        return Vec3(0, 0, 0);
    }

    float dot(const Vec3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }
};

BlockInteraction::BlockInteraction(std::shared_ptr<World> world,
                                 std::shared_ptr<Player> player,
                                 std::shared_ptr<AudioManager> audioManager,
                                 const InteractionConfig& config)
    : m_world(world)
    , m_player(player)
    , m_audioManager(audioManager)
    , m_config(config)
    , m_blockPlaceCooldownTimer(0.0f)
    , m_blockBreakCooldownTimer(0.0f)
    , m_lastInteractionTime(0.0f)
    , m_nextCallbackId(1)
{
    m_currentTarget.valid = false;
}

BlockInteraction::~BlockInteraction() {
    // Cleanup will be handled by smart pointers
}

void BlockInteraction::Update(float deltaTime) {
    UpdateCooldowns(deltaTime);
    UpdateCurrentTarget();
}

void BlockInteraction::UpdateCurrentTarget() {
    if (!m_player || !m_world) return;

    Vec3 playerPos = m_player->GetEyePosition();
    Vec3 lookDir = m_player->GetLookDirection();

    m_currentTarget = FindTargetedBlock(playerPos, lookDir);
}

BlockTarget BlockInteraction::FindTargetedBlock(const Vec3& playerPosition, const Vec3& lookDirection) const {
    BlockTarget target;
    target.valid = false;

    if (!m_world) return target;

    Vec3 hitPosition, hitNormal;
    bool hit = m_world->Raycast(playerPosition, lookDirection, m_config.maxInteractionDistance,
                               hitPosition, hitNormal);

    if (hit) {
        // Convert hit position to block coordinates
        target.position = Vec3(std::floor(hitPosition.x), std::floor(hitPosition.y), std::floor(hitPosition.z));

        // Adjust position based on normal to get the actual block face
        target.position = target.position + hitNormal * 0.5f;
        target.position = Vec3(std::floor(target.position.x + 0.5f),
                              std::floor(target.position.y + 0.5f),
                              std::floor(target.position.z + 0.5f));

        target.normal = hitNormal;
        target.blockType = m_world->GetBlock(static_cast<int>(target.position.x),
                                           static_cast<int>(target.position.y),
                                           static_cast<int>(target.position.z));
        target.distance = (hitPosition - playerPosition).length();
        target.valid = (target.blockType != BlockType::AIR);
    }

    return target;
}

InteractionResult BlockInteraction::DestroyBlock(const BlockTarget& target) {
    if (!target.valid || !m_player || !m_world) {
        return InteractionResult::INVALID_TARGET;
    }

    // Check cooldown
    if (IsOnCooldown(InteractionType::DESTROY_BLOCK)) {
        return InteractionResult::COOLDOWN;
    }

    // Check distance
    if (target.distance > m_config.maxInteractionDistance) {
        return InteractionResult::OUT_OF_RANGE;
    }

    // Check line of sight if required
    if (m_config.requireLineOfSight) {
        // TODO: Implement proper line of sight check
    }

    // Check if player has appropriate tool
    if (!HasRequiredTool(target.blockType)) {
        // Allow breaking without tool, but slower
        std::cout << "No appropriate tool for breaking " << static_cast<int>(target.blockType) << std::endl;
    }

    // Play break sound
    if (m_audioManager) {
        m_audioManager->PlaySound3D(SoundType::BLOCK_BREAK,
                                  target.position.x, target.position.y, target.position.z);
    }

    // Break the block
    m_world->SetBlock(static_cast<int>(target.position.x),
                     static_cast<int>(target.position.y),
                     static_cast<int>(target.position.z),
                     BlockType::AIR);

    // Start cooldown
    StartCooldown(InteractionType::DESTROY_BLOCK);

    // Notify callbacks
    InteractionContext context;
    context.type = InteractionType::DESTROY_BLOCK;
    context.playerPosition = m_player->GetPosition();
    context.lookDirection = m_player->GetLookDirection();
    context.maxDistance = m_config.maxInteractionDistance;
    context.player = m_player;
    context.world = m_world;

    NotifyCallbacks(context, InteractionResult::SUCCESS);

    std::cout << "Block destroyed at " << target.position.x << ", "
              << target.position.y << ", " << target.position.z << std::endl;

    return InteractionResult::SUCCESS;
}

InteractionResult BlockInteraction::PlaceBlock(const BlockTarget& target) {
    if (!target.valid || !m_player || !m_world) {
        return InteractionResult::INVALID_TARGET;
    }

    // Check cooldown
    if (IsOnCooldown(InteractionType::PLACE_BLOCK)) {
        return InteractionResult::COOLDOWN;
    }

    // Calculate placement position
    Vec3 placePos = GetPlacementPosition(target);

    // Check if placement is valid
    if (!IsValidPlacement(placePos, m_player->GetPosition())) {
        return InteractionResult::BLOCKED;
    }

    // Get block to place from player's inventory
    auto inventory = m_player->GetInventory();
    if (!inventory) {
        return InteractionResult::INSUFFICIENT_ITEMS;
    }

    // For now, place a simple stone block as test
    // TODO: Get selected item from inventory
    BlockType blockToPlace = BlockType::STONE;

    // Play place sound
    if (m_audioManager) {
        m_audioManager->PlaySound3D(SoundType::BLOCK_PLACE,
                                  placePos.x, placePos.y, placePos.z);
    }

    // Place the block
    m_world->SetBlock(static_cast<int>(placePos.x),
                     static_cast<int>(placePos.y),
                     static_cast<int>(placePos.z),
                     blockToPlace);

    // Start cooldown
    StartCooldown(InteractionType::PLACE_BLOCK);

    // Notify callbacks
    InteractionContext context;
    context.type = InteractionType::PLACE_BLOCK;
    context.playerPosition = m_player->GetPosition();
    context.lookDirection = m_player->GetLookDirection();
    context.maxDistance = m_config.maxInteractionDistance;
    context.player = m_player;
    context.world = m_world;

    NotifyCallbacks(context, InteractionResult::SUCCESS);

    std::cout << "Block placed at " << placePos.x << ", "
              << placePos.y << ", " << placePos.z << std::endl;

    return InteractionResult::SUCCESS;
}

InteractionResult BlockInteraction::UseItem(const BlockTarget& target) {
    if (!target.valid) {
        return InteractionResult::INVALID_TARGET;
    }

    // Handle special interactions with blocks
    return HandleSpecialInteraction(target.blockType, target);
}

bool BlockInteraction::CanInteractWith(const BlockTarget& target) const {
    if (!target.valid) return false;

    // Check distance
    if (target.distance > m_config.maxInteractionDistance) return false;

    // Check if block is air (can't interact with air)
    if (target.blockType == BlockType::AIR) return false;

    // Check line of sight if required
    if (m_config.requireLineOfSight) {
        // TODO: Implement line of sight check
    }

    return true;
}

int BlockInteraction::RegisterCallback(InteractionCallback callback) {
    int id = m_nextCallbackId++;
    m_callbacks.push_back(callback);
    return id;
}

void BlockInteraction::UnregisterCallback(int callbackId) {
    // Simple implementation - just remove by index
    if (callbackId > 0 && callbackId <= static_cast<int>(m_callbacks.size())) {
        m_callbacks.erase(m_callbacks.begin() + callbackId - 1);
    }
}

void BlockInteraction::SetConfig(const InteractionConfig& config) {
    m_config = config;
}

// Private methods

void BlockInteraction::UpdateCooldowns(float deltaTime) {
    m_blockPlaceCooldownTimer = std::max(0.0f, m_blockPlaceCooldownTimer - deltaTime);
    m_blockBreakCooldownTimer = std::max(0.0f, m_blockBreakCooldownTimer - deltaTime);
}

bool BlockInteraction::IsOnCooldown(InteractionType actionType) const {
    switch (actionType) {
        case InteractionType::PLACE_BLOCK:
            return m_blockPlaceCooldownTimer > 0.0f;
        case InteractionType::DESTROY_BLOCK:
            return m_blockBreakCooldownTimer > 0.0f;
        default:
            return false;
    }
}

void BlockInteraction::StartCooldown(InteractionType actionType) {
    switch (actionType) {
        case InteractionType::PLACE_BLOCK:
            m_blockPlaceCooldownTimer = m_config.blockPlaceCooldown;
            break;
        case InteractionType::DESTROY_BLOCK:
            m_blockBreakCooldownTimer = m_config.blockBreakCooldown;
            break;
        default:
            break;
    }
}

bool BlockInteraction::HasRequiredTool(BlockType blockType) const {
    // For now, always return true - any tool can break any block
    // TODO: Implement proper tool requirements based on block hardness
    return true;
}

float BlockInteraction::CalculateBreakTime(BlockType blockType, const Tool* tool) const {
    // Simple break time calculation
    // TODO: Implement proper break time based on tool efficiency and block hardness
    return 0.5f; // Half second for now
}

const Tool* BlockInteraction::GetAppropriateTool(BlockType blockType) const {
    // TODO: Get best tool from player's inventory
    return nullptr;
}

bool BlockInteraction::IsValidPlacement(const Vec3& position, const Vec3& playerPos) const {
    // Check if position is too far
    Vec3 diff = position - playerPos;
    if (diff.length() > m_config.maxInteractionDistance) {
        return false;
    }

    // Check if block already exists at position
    BlockType existingBlock = m_world->GetBlock(static_cast<int>(position.x),
                                              static_cast<int>(position.y),
                                              static_cast<int>(position.z));
    if (existingBlock != BlockType::AIR) {
        return false;
    }

    // Check if player is trying to place block inside themselves
    Vec3 playerBlockPos = Vec3(std::floor(playerPos.x), std::floor(playerPos.y), std::floor(playerPos.z));
    if (position.x == playerBlockPos.x && position.y == playerBlockPos.y && position.z == playerBlockPos.z) {
        return false;
    }

    return true;
}

Vec3 BlockInteraction::GetPlacementPosition(const BlockTarget& target) const {
    // Place block adjacent to the targeted face
    return target.position + target.normal;
}

void BlockInteraction::NotifyCallbacks(const InteractionContext& context, InteractionResult result) {
    for (const auto& callback : m_callbacks) {
        callback(context, result);
    }
}

InteractionResult BlockInteraction::HandleSpecialInteraction(BlockType blockType, const BlockTarget& target) {
    // Handle special block interactions
    switch (blockType) {
        case BlockType::CRAFTING_TABLE:
            // TODO: Open crafting interface
            std::cout << "Interacted with crafting table" << std::endl;
            break;

        case BlockType::CHEST:
            // TODO: Open chest inventory
            std::cout << "Interacted with chest" << std::endl;
            break;

        case BlockType::FURNACE:
            // TODO: Open furnace interface
            std::cout << "Interacted with furnace" << std::endl;
            break;

        default:
            // No special interaction
            break;
    }

    return InteractionResult::SUCCESS;
}

} // namespace VoxelCraft
