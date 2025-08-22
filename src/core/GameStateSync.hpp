/**
 * @file GameStateSync.hpp
 * @brief VoxelCraft Game State Synchronization System
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_CORE_GAME_STATE_SYNC_HPP
#define VOXELCRAFT_CORE_GAME_STATE_SYNC_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <atomic>
#include <chrono>
#include <functional>

#include "../core/NetworkManager.hpp"

namespace VoxelCraft {

// Forward declarations
class World;
class Player;
struct Vec3;

struct GameStateSnapshot {
    uint64_t timestamp;                    ///< Snapshot timestamp
    uint64_t sequenceNumber;               ///< Snapshot sequence number
    std::vector<uint8_t> worldData;        ///< World state data
    std::vector<uint8_t> playerData;       ///< Player state data
    std::vector<uint8_t> entityData;       ///< Entity state data
    std::unordered_map<uint32_t, std::vector<uint8_t>> playerStates; ///< Individual player states
};

struct PlayerState {
    uint32_t playerId;                     ///< Player ID
    Vec3 position;                         ///< Player position
    Vec3 rotation;                         ///< Player rotation
    Vec3 velocity;                         ///< Player velocity
    float health;                          ///< Player health
    uint64_t lastUpdateTime;               ///< Last update timestamp
    std::vector<uint8_t> inputState;       ///< Current input state
};

struct EntityState {
    uint32_t entityId;                     ///< Entity ID
    Vec3 position;                         ///< Entity position
    Vec3 rotation;                         ///< Entity rotation
    Vec3 velocity;                         ///< Entity velocity
    uint32_t entityType;                   ///< Entity type
    uint64_t lastUpdateTime;               ///< Last update timestamp
};

struct SyncMetrics {
    uint64_t totalSnapshotsSent;           ///< Total snapshots sent
    uint64_t totalSnapshotsReceived;       ///< Total snapshots received
    uint64_t totalStateUpdates;            ///< Total state updates
    double averageLatency;                 ///< Average synchronization latency
    double averageUpdateInterval;          ///< Average update interval
    uint32_t outOfOrderPackets;            ///< Number of out-of-order packets
    uint32_t droppedPackets;               ///< Number of dropped packets
    double compressionRatio;               ///< Data compression ratio
};

class GameStateSync {
public:
    GameStateSync();
    ~GameStateSync();

    bool Initialize(const Config& config);
    void Shutdown();

    void Update(double deltaTime);

    // Server-side methods
    void CreateSnapshot(World* world, const std::vector<Player*>& players);
    void SendSnapshotToPlayer(uint32_t playerId);
    void BroadcastSnapshot();
    void ProcessPlayerInput(uint32_t playerId, const std::vector<uint8_t>& inputData);

    // Client-side methods
    void ReceiveSnapshot(const std::vector<uint8_t>& snapshotData);
    void ApplySnapshot();
    void SendPlayerState(const PlayerState& playerState);
    void ReconcileState(const GameStateSnapshot& serverSnapshot);

    // General methods
    const SyncMetrics& GetMetrics() const { return m_metrics; }
    void SetNetworkManager(NetworkManager* networkManager) { m_networkManager = networkManager; }

    // Configuration
    void SetSnapshotInterval(double interval) { m_snapshotInterval = interval; }
    void SetInterpolationTime(double time) { m_interpolationTime = time; }
    void SetExtrapolationTime(double time) { m_extrapolationTime = time; }
    void SetCompressionEnabled(bool enabled) { m_compressionEnabled = enabled; }
    void SetDeltaCompressionEnabled(bool enabled) { m_deltaCompressionEnabled = enabled; }

private:
    // Synchronization state
    std::atomic<bool> m_initialized;
    NetworkManager* m_networkManager;

    // Snapshot management
    GameStateSnapshot m_currentSnapshot;
    GameStateSnapshot m_previousSnapshot;
    std::queue<GameStateSnapshot> m_snapshotBuffer;
    mutable std::mutex m_snapshotMutex;

    // Player state management
    std::unordered_map<uint32_t, PlayerState> m_playerStates;
    mutable std::mutex m_playerStatesMutex;

    // Entity state management
    std::unordered_map<uint32_t, EntityState> m_entityStates;
    mutable std::mutex m_entityStatesMutex;

    // Timing
    double m_snapshotInterval;             ///< Time between snapshots (seconds)
    double m_interpolationTime;            ///< Client-side interpolation time
    double m_extrapolationTime;            ///< Client-side extrapolation time
    std::chrono::steady_clock::time_point m_lastSnapshotTime;

    // Configuration
    bool m_compressionEnabled;             ///< Enable data compression
    bool m_deltaCompressionEnabled;        ///< Enable delta compression
    bool m_isServer;                       ///< Is this instance running as server

    // Metrics
    SyncMetrics m_metrics;
    mutable std::mutex m_metricsMutex;

    // Client-side prediction
    struct PredictedState {
        PlayerState state;
        uint64_t predictionTime;
        std::vector<uint8_t> inputs;
    };
    std::vector<PredictedState> m_predictionHistory;

    // Serialization methods
    std::vector<uint8_t> SerializeSnapshot(const GameStateSnapshot& snapshot);
    GameStateSnapshot DeserializeSnapshot(const std::vector<uint8_t>& data);
    std::vector<uint8_t> SerializePlayerState(const PlayerState& state);
    PlayerState DeserializePlayerState(const std::vector<uint8_t>& data);
    std::vector<uint8_t> SerializeEntityState(const EntityState& state);
    EntityState DeserializeEntityState(const std::vector<uint8_t>& data);

    // Compression methods
    std::vector<uint8_t> CompressData(const std::vector<uint8_t>& data);
    std::vector<uint8_t> DecompressData(const std::vector<uint8_t>& data);
    std::vector<uint8_t> CreateDeltaCompressedData(const std::vector<uint8_t>& current,
                                                  const std::vector<uint8_t>& previous);
    std::vector<uint8_t> ApplyDeltaDecompression(const std::vector<uint8_t>& delta,
                                                const std::vector<uint8_t>& base);

    // State reconciliation
    void ReconcilePlayerState(uint32_t playerId, const PlayerState& serverState);
    void ReconcileEntityState(uint32_t entityId, const EntityState& serverState);
    void HandleStateConflict(const PlayerState& localState, const PlayerState& serverState);

    // Prediction and correction
    void PredictPlayerState(uint32_t playerId, double deltaTime);
    void CorrectPrediction(uint32_t playerId, const PlayerState& correctedState);
    void StorePrediction(uint32_t playerId, const PredictedState& prediction);

    // Network packet handlers
    void HandleSnapshotPacket(const NetworkPacket& packet);
    void HandlePlayerStatePacket(const NetworkPacket& packet);
    void HandleInputPacket(const NetworkPacket& packet);

    // Metrics update
    void UpdateMetrics(const GameStateSnapshot& snapshot);
    void CalculateLatency();
    void UpdateCompressionStats(size_t originalSize, size_t compressedSize);

    // Utility methods
    uint64_t GetCurrentTimestamp() const;
    double GetTimeSinceLastSnapshot() const;
    bool ShouldCreateSnapshot() const;
    void CleanupOldSnapshots();
    void ValidateSnapshot(const GameStateSnapshot& snapshot);
};

} // namespace VoxelCraft

#endif // VOXELCRAFT_CORE_GAME_STATE_SYNC_HPP
