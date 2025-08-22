/**
 * @file GameStateSync.cpp
 * @brief VoxelCraft Game State Synchronization Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "GameStateSync.hpp"
#include "Logger.hpp"
#include "../world/World.hpp"
#include "../player/Player.hpp"

#include <algorithm>
#include <cstring>

namespace VoxelCraft {

GameStateSync::GameStateSync()
    : m_initialized(false)
    , m_networkManager(nullptr)
    , m_snapshotInterval(0.05) // 20 snapshots per second
    , m_interpolationTime(0.1) // 100ms interpolation
    , m_extrapolationTime(0.5) // 500ms extrapolation
    , m_compressionEnabled(true)
    , m_deltaCompressionEnabled(true)
    , m_isServer(false)
{
    std::memset(&m_metrics, 0, sizeof(SyncMetrics));
}

GameStateSync::~GameStateSync() {
    Shutdown();
}

bool GameStateSync::Initialize(const Config& config) {
    if (m_initialized) {
        return true;
    }

    VOXELCRAFT_INFO("Initializing Game State Synchronization");

    try {
        // Load configuration
        m_snapshotInterval = config.Get("sync.snapshot_interval", 0.05);
        m_interpolationTime = config.Get("sync.interpolation_time", 0.1);
        m_extrapolationTime = config.Get("sync.extrapolation_time", 0.5);
        m_compressionEnabled = config.Get("sync.compression_enabled", true);
        m_deltaCompressionEnabled = config.Get("sync.delta_compression_enabled", true);
        m_isServer = config.Get("network.server_mode", false);

        m_initialized = true;
        VOXELCRAFT_INFO("Game State Sync initialized successfully");
        return true;

    } catch (const std::exception& e) {
        VOXELCRAFT_ERROR("Failed to initialize Game State Sync: {}", e.what());
        return false;
    }
}

void GameStateSync::Shutdown() {
    if (!m_initialized) {
        return;
    }

    VOXELCRAFT_INFO("Shutting down Game State Synchronization");

    // Clear all state
    m_playerStates.clear();
    m_entityStates.clear();

    while (!m_snapshotBuffer.empty()) {
        m_snapshotBuffer.pop();
    }

    m_predictionHistory.clear();

    m_initialized = false;
    VOXELCRAFT_INFO("Game State Sync shutdown complete");
}

void GameStateSync::Update(double deltaTime) {
    if (!m_initialized) {
        return;
    }

    // Client-side interpolation and prediction
    if (!m_isServer) {
        // Apply client-side interpolation
        ApplySnapshot();

        // Predict local player state
        for (const auto& pair : m_playerStates) {
            PredictPlayerState(pair.first, deltaTime);
        }
    }

    // Server-side snapshot creation
    if (m_isServer && ShouldCreateSnapshot()) {
        // Snapshot creation would be triggered externally
        // This is just for timing
        m_lastSnapshotTime = std::chrono::steady_clock::now();
    }

    // Clean up old snapshots
    CleanupOldSnapshots();

    // Update metrics
    CalculateLatency();
}

void GameStateSync::CreateSnapshot(World* world, const std::vector<Player*>& players) {
    if (!m_initialized || !m_isServer) {
        return;
    }

    GameStateSnapshot snapshot;
    snapshot.timestamp = GetCurrentTimestamp();
    snapshot.sequenceNumber = m_currentSnapshot.sequenceNumber + 1;

    // Serialize world state
    if (world) {
        // This would serialize world chunks, entities, etc.
        // For now, just store a placeholder
        snapshot.worldData = std::vector<uint8_t>(1024, 0);
    }

    // Serialize player states
    for (const Player* player : players) {
        if (player) {
            PlayerState state;
            state.playerId = 0; // Would get from player
            state.position = player->GetPosition();
            state.rotation = player->GetRotation();
            state.velocity = Vec3(0, 0, 0); // Would get from player
            state.health = player->GetHealth();
            state.lastUpdateTime = snapshot.timestamp;

            auto serializedState = SerializePlayerState(state);
            snapshot.playerData.insert(snapshot.playerData.end(),
                                     serializedState.begin(),
                                     serializedState.end());
        }
    }

    // Apply compression if enabled
    if (m_compressionEnabled) {
        if (m_deltaCompressionEnabled && m_currentSnapshot.timestamp > 0) {
            snapshot.worldData = CreateDeltaCompressedData(snapshot.worldData, m_currentSnapshot.worldData);
            snapshot.playerData = CreateDeltaCompressedData(snapshot.playerData, m_currentSnapshot.playerData);
        } else {
            snapshot.worldData = CompressData(snapshot.worldData);
            snapshot.playerData = CompressData(snapshot.playerData);
        }
    }

    // Update current snapshot
    m_previousSnapshot = m_currentSnapshot;
    m_currentSnapshot = snapshot;

    // Update metrics
    UpdateMetrics(snapshot);
}

void GameStateSync::SendSnapshotToPlayer(uint32_t playerId) {
    if (!m_networkManager) {
        return;
    }

    auto serializedSnapshot = SerializeSnapshot(m_currentSnapshot);

    NetworkPacket packet;
    packet.packetId = 0;
    packet.type = PacketType::WORLD_TIME; // Using existing packet type, would need custom type
    packet.senderId = 0;
    packet.sequenceNumber = m_currentSnapshot.sequenceNumber;
    packet.data = serializedSnapshot;
    packet.reliable = false;

    m_networkManager->SendPacket(playerId, packet);
    m_metrics.totalSnapshotsSent++;
}

void GameStateSync::BroadcastSnapshot() {
    if (!m_networkManager) {
        return;
    }

    auto serializedSnapshot = SerializeSnapshot(m_currentSnapshot);

    NetworkPacket packet;
    packet.packetId = 0;
    packet.type = PacketType::WORLD_TIME; // Using existing packet type
    packet.senderId = 0;
    packet.sequenceNumber = m_currentSnapshot.sequenceNumber;
    packet.data = serializedSnapshot;
    packet.reliable = false;

    m_networkManager->BroadcastPacket(packet);
    m_metrics.totalSnapshotsSent++;
}

void GameStateSync::ReceiveSnapshot(const std::vector<uint8_t>& snapshotData) {
    if (!m_initialized || m_isServer) {
        return;
    }

    try {
        GameStateSnapshot snapshot = DeserializeSnapshot(snapshotData);

        // Decompress if needed
        if (m_compressionEnabled) {
            if (m_deltaCompressionEnabled && !m_snapshotBuffer.empty()) {
                const auto& baseSnapshot = m_snapshotBuffer.back();
                snapshot.worldData = ApplyDeltaDecompression(snapshot.worldData, baseSnapshot.worldData);
                snapshot.playerData = ApplyDeltaDecompression(snapshot.playerData, baseSnapshot.playerData);
            } else {
                snapshot.worldData = DecompressData(snapshot.worldData);
                snapshot.playerData = DecompressData(snapshot.playerData);
            }
        }

        // Add to snapshot buffer
        {
            std::lock_guard<std::mutex> lock(m_snapshotMutex);
            m_snapshotBuffer.push(snapshot);
            m_metrics.totalSnapshotsReceived++;

            // Limit buffer size
            while (m_snapshotBuffer.size() > 30) { // Keep last 30 snapshots
                m_snapshotBuffer.pop();
            }
        }

    } catch (const std::exception& e) {
        VOXELCRAFT_ERROR("Failed to receive snapshot: {}", e.what());
        m_metrics.droppedPackets++;
    }
}

void GameStateSync::ApplySnapshot() {
    std::lock_guard<std::mutex> lock(m_snapshotMutex);

    if (m_snapshotBuffer.empty()) {
        return;
    }

    // Get the most recent snapshot
    const GameStateSnapshot& snapshot = m_snapshotBuffer.back();

    // Apply world state
    // This would update world chunks, entities, etc.

    // Apply player states
    // This would reconcile local player states with server states

    m_metrics.totalStateUpdates++;
}

void GameStateSync::SendPlayerState(const PlayerState& playerState) {
    if (!m_networkManager) {
        return;
    }

    auto serializedState = SerializePlayerState(playerState);

    NetworkPacket packet;
    packet.packetId = 0;
    packet.type = PacketType::PLAYER_POSITION; // Using existing type
    packet.senderId = playerState.playerId;
    packet.sequenceNumber = 0;
    packet.data = serializedState;
    packet.reliable = false;

    m_networkManager->BroadcastPacket(packet);
}

void GameStateSync::ProcessPlayerInput(uint32_t playerId, const std::vector<uint8_t>& inputData) {
    // Store input for processing
    std::lock_guard<std::mutex> lock(m_playerStatesMutex);

    auto it = m_playerStates.find(playerId);
    if (it != m_playerStates.end()) {
        it->second.inputState = inputData;
        it->second.lastUpdateTime = GetCurrentTimestamp();
    }
}

void GameStateSync::ReconcileState(const GameStateSnapshot& serverSnapshot) {
    // Reconcile local state with server state
    // This would correct any prediction errors

    for (const auto& serverPlayerState : serverSnapshot.playerStates) {
        uint32_t playerId = serverPlayerState.first;

        if (!serverPlayerState.second.empty()) {
            PlayerState serverState = DeserializePlayerState(serverPlayerState.second);
            ReconcilePlayerState(playerId, serverState);
        }
    }
}

// Private methods

std::vector<uint8_t> GameStateSync::SerializeSnapshot(const GameStateSnapshot& snapshot) {
    std::vector<uint8_t> data;

    // Serialize timestamp
    uint64_t timestamp = snapshot.timestamp;
    data.insert(data.end(), reinterpret_cast<uint8_t*>(&timestamp),
               reinterpret_cast<uint8_t*>(&timestamp) + sizeof(timestamp));

    // Serialize sequence number
    uint64_t sequence = snapshot.sequenceNumber;
    data.insert(data.end(), reinterpret_cast<uint8_t*>(&sequence),
               reinterpret_cast<uint8_t*>(&sequence) + sizeof(sequence));

    // Serialize world data size and data
    uint32_t worldSize = snapshot.worldData.size();
    data.insert(data.end(), reinterpret_cast<uint8_t*>(&worldSize),
               reinterpret_cast<uint8_t*>(&worldSize) + sizeof(worldSize));
    data.insert(data.end(), snapshot.worldData.begin(), snapshot.worldData.end());

    // Serialize player data size and data
    uint32_t playerSize = snapshot.playerData.size();
    data.insert(data.end(), reinterpret_cast<uint8_t*>(&playerSize),
               reinterpret_cast<uint8_t*>(&playerSize) + sizeof(playerSize));
    data.insert(data.end(), snapshot.playerData.begin(), snapshot.playerData.end());

    return data;
}

GameStateSnapshot GameStateSync::DeserializeSnapshot(const std::vector<uint8_t>& data) {
    GameStateSnapshot snapshot;
    size_t offset = 0;

    if (data.size() < sizeof(uint64_t) * 2 + sizeof(uint32_t) * 2) {
        throw std::runtime_error("Snapshot data too small");
    }

    // Deserialize timestamp
    std::memcpy(&snapshot.timestamp, data.data() + offset, sizeof(uint64_t));
    offset += sizeof(uint64_t);

    // Deserialize sequence number
    std::memcpy(&snapshot.sequenceNumber, data.data() + offset, sizeof(uint64_t));
    offset += sizeof(uint64_t);

    // Deserialize world data
    uint32_t worldSize;
    std::memcpy(&worldSize, data.data() + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    if (offset + worldSize > data.size()) {
        throw std::runtime_error("World data size exceeds available data");
    }

    snapshot.worldData.assign(data.begin() + offset, data.begin() + offset + worldSize);
    offset += worldSize;

    // Deserialize player data
    uint32_t playerSize;
    std::memcpy(&playerSize, data.data() + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    if (offset + playerSize > data.size()) {
        throw std::runtime_error("Player data size exceeds available data");
    }

    snapshot.playerData.assign(data.begin() + offset, data.begin() + offset + playerSize);

    return snapshot;
}

std::vector<uint8_t> GameStateSync::SerializePlayerState(const PlayerState& state) {
    std::vector<uint8_t> data;

    // Serialize player ID
    data.insert(data.end(), reinterpret_cast<const uint8_t*>(&state.playerId),
               reinterpret_cast<const uint8_t*>(&state.playerId) + sizeof(state.playerId));

    // Serialize position
    float pos[3] = {state.position.x, state.position.y, state.position.z};
    data.insert(data.end(), reinterpret_cast<uint8_t*>(pos),
               reinterpret_cast<uint8_t*>(pos) + sizeof(pos));

    // Serialize rotation
    float rot[3] = {state.rotation.x, state.rotation.y, state.rotation.z};
    data.insert(data.end(), reinterpret_cast<uint8_t*>(rot),
               reinterpret_cast<uint8_t*>(rot) + sizeof(rot));

    // Serialize velocity
    float vel[3] = {state.velocity.x, state.velocity.y, state.velocity.z};
    data.insert(data.end(), reinterpret_cast<uint8_t*>(vel),
               reinterpret_cast<uint8_t*>(vel) + sizeof(vel));

    // Serialize health
    data.insert(data.end(), reinterpret_cast<const uint8_t*>(&state.health),
               reinterpret_cast<const uint8_t*>(&state.health) + sizeof(state.health));

    // Serialize timestamp
    data.insert(data.end(), reinterpret_cast<const uint8_t*>(&state.lastUpdateTime),
               reinterpret_cast<const uint8_t*>(&state.lastUpdateTime) + sizeof(state.lastUpdateTime));

    return data;
}

PlayerState GameStateSync::DeserializePlayerState(const std::vector<uint8_t>& data) {
    PlayerState state;
    size_t offset = 0;
    size_t expectedSize = sizeof(uint32_t) + sizeof(float) * 9 + sizeof(float) + sizeof(uint64_t);

    if (data.size() < expectedSize) {
        throw std::runtime_error("Player state data too small");
    }

    // Deserialize player ID
    std::memcpy(&state.playerId, data.data() + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Deserialize position
    float pos[3];
    std::memcpy(pos, data.data() + offset, sizeof(float) * 3);
    state.position = Vec3(pos[0], pos[1], pos[2]);
    offset += sizeof(float) * 3;

    // Deserialize rotation
    float rot[3];
    std::memcpy(rot, data.data() + offset, sizeof(float) * 3);
    state.rotation = Vec3(rot[0], rot[1], rot[2]);
    offset += sizeof(float) * 3;

    // Deserialize velocity
    float vel[3];
    std::memcpy(vel, data.data() + offset, sizeof(float) * 3);
    state.velocity = Vec3(vel[0], vel[1], vel[2]);
    offset += sizeof(float) * 3;

    // Deserialize health
    std::memcpy(&state.health, data.data() + offset, sizeof(float));
    offset += sizeof(float);

    // Deserialize timestamp
    std::memcpy(&state.lastUpdateTime, data.data() + offset, sizeof(uint64_t));

    return state;
}

std::vector<uint8_t> GameStateSync::CompressData(const std::vector<uint8_t>& data) {
    // Simple compression placeholder
    // In a real implementation, this would use zlib or similar
    return data;
}

std::vector<uint8_t> GameStateSync::DecompressData(const std::vector<uint8_t>& data) {
    // Simple decompression placeholder
    return data;
}

std::vector<uint8_t> GameStateSync::CreateDeltaCompressedData(const std::vector<uint8_t>& current,
                                                            const std::vector<uint8_t>& previous) {
    // Simple delta compression placeholder
    // In a real implementation, this would compute differences
    return current;
}

std::vector<uint8_t> GameStateSync::ApplyDeltaDecompression(const std::vector<uint8_t>& delta,
                                                          const std::vector<uint8_t>& base) {
    // Simple delta decompression placeholder
    return delta;
}

void GameStateSync::ReconcilePlayerState(uint32_t playerId, const PlayerState& serverState) {
    std::lock_guard<std::mutex> lock(m_playerStatesMutex);

    auto it = m_playerStates.find(playerId);
    if (it != m_playerStates.end()) {
        PlayerState& localState = it->second;

        // Check if server state is newer
        if (serverState.lastUpdateTime > localState.lastUpdateTime) {
            // Check for significant difference
            Vec3 positionDiff = {
                serverState.position.x - localState.position.x,
                serverState.position.y - localState.position.y,
                serverState.position.z - localState.position.z
            };

            float diffMagnitude = std::sqrt(
                positionDiff.x * positionDiff.x +
                positionDiff.y * positionDiff.y +
                positionDiff.z * positionDiff.z
            );

            // If difference is significant, correct the state
            if (diffMagnitude > 0.1f) {
                VOXELCRAFT_DEBUG("Reconciling player {} state (diff: {})", playerId, diffMagnitude);
                localState = serverState;
            }
        }
    } else {
        // New player state
        m_playerStates[playerId] = serverState;
    }
}

void GameStateSync::ReconcileEntityState(uint32_t entityId, const EntityState& serverState) {
    std::lock_guard<std::mutex> lock(m_entityStatesMutex);

    auto it = m_entityStates.find(entityId);
    if (it != m_entityStates.end()) {
        EntityState& localState = it->second;

        if (serverState.lastUpdateTime > localState.lastUpdateTime) {
            localState = serverState;
        }
    } else {
        m_entityStates[entityId] = serverState;
    }
}

void GameStateSync::PredictPlayerState(uint32_t playerId, double deltaTime) {
    std::lock_guard<std::mutex> lock(m_playerStatesMutex);

    auto it = m_playerStates.find(playerId);
    if (it != m_playerStates.end()) {
        PlayerState& state = it->second;

        // Simple prediction based on velocity
        state.position.x += state.velocity.x * deltaTime;
        state.position.y += state.velocity.y * deltaTime;
        state.position.z += state.velocity.z * deltaTime;
    }
}

uint64_t GameStateSync::GetCurrentTimestamp() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

double GameStateSync::GetTimeSinceLastSnapshot() const {
    auto now = std::chrono::steady_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::duration<double>>(
        now - m_lastSnapshotTime
    );
    return diff.count();
}

bool GameStateSync::ShouldCreateSnapshot() const {
    return GetTimeSinceLastSnapshot() >= m_snapshotInterval;
}

void GameStateSync::CleanupOldSnapshots() {
    std::lock_guard<std::mutex> lock(m_snapshotMutex);

    auto cutoffTime = GetCurrentTimestamp() - static_cast<uint64_t>(m_interpolationTime * 2000);

    while (!m_snapshotBuffer.empty() && m_snapshotBuffer.front().timestamp < cutoffTime) {
        m_snapshotBuffer.pop();
    }
}

void GameStateSync::UpdateMetrics(const GameStateSnapshot& snapshot) {
    std::lock_guard<std::mutex> lock(m_metricsMutex);

    m_metrics.totalSnapshotsSent++;
    UpdateCompressionStats(1024, snapshot.worldData.size()); // Placeholder sizes
}

void GameStateSync::CalculateLatency() {
    // Calculate average latency between sent and received snapshots
    // This is a simplified implementation
}

void GameStateSync::UpdateCompressionStats(size_t originalSize, size_t compressedSize) {
    if (originalSize > 0) {
        m_metrics.compressionRatio = static_cast<double>(compressedSize) / originalSize;
    }
}

} // namespace VoxelCraft
