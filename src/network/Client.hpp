/**
 * @file Client.hpp
 * @brief VoxelCraft Multiplayer Client
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_NETWORK_CLIENT_HPP
#define VOXELCRAFT_NETWORK_CLIENT_HPP

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>

#include "../core/Config.hpp"
#include "../core/NetworkManager.hpp"

namespace VoxelCraft {

// Forward declarations
class Player;
struct Vec3;

class Client {
public:
    Client();
    ~Client();

    bool Initialize(const Config& config);
    void Shutdown();
    void Update(double deltaTime);

    bool Connect(const std::string& serverAddress);
    void Disconnect();
    bool IsConnected() const { return m_connected; }

    // Player synchronization
    void SendPlayerPosition(const Vec3& position);
    void SendPlayerRotation(const Vec3& rotation);
    void SendPlayerInput(const std::vector<uint8_t>& inputData);
    void SendChatMessage(const std::string& message);

    // Remote player management
    std::vector<std::shared_ptr<Player>> GetRemotePlayers();
    std::shared_ptr<Player> GetRemotePlayer(uint32_t playerId);

    // Server info
    std::string GetServerName() const { return m_serverName; }
    std::string GetServerMotd() const { return m_serverMotd; }
    uint32_t GetPing() const { return m_ping; }

private:
    // Connection state
    std::atomic<bool> m_connected;
    std::atomic<bool> m_initialized;
    std::string m_serverAddress;
    std::string m_serverName;
    std::string m_serverMotd;
    uint32_t m_localPlayerId;
    uint32_t m_ping;

    // Network manager
    std::unique_ptr<NetworkManager> m_networkManager;

    // Remote players
    std::unordered_map<uint32_t, std::shared_ptr<Player>> m_remotePlayers;
    mutable std::mutex m_remotePlayersMutex;

    // Network threading
    std::unique_ptr<std::thread> m_networkThread;
    std::atomic<bool> m_networkThreadRunning;

    // Packet queues
    std::queue<NetworkPacket> m_incomingPackets;
    mutable std::mutex m_incomingMutex;
    std::queue<NetworkPacket> m_outgoingPackets;
    mutable std::mutex m_outgoingMutex;

    // Client methods
    void NetworkThread();
    void ProcessIncomingPackets();
    void ProcessOutgoingPackets();

    // Packet handlers
    void HandleHandshakePacket(const NetworkPacket& packet);
    void HandlePlayerPositionPacket(const NetworkPacket& packet);
    void HandlePlayerConnectedPacket(const NetworkPacket& packet);
    void HandlePlayerDisconnectedPacket(const NetworkPacket& packet);
    void HandleChatMessagePacket(const NetworkPacket& packet);
    void HandleWorldStatePacket(const NetworkPacket& packet);

    // Event callbacks
    void OnConnected();
    void OnDisconnected();
    void OnConnectionFailed(const std::string& reason);

    // Player management
    void AddRemotePlayer(uint32_t playerId, const std::string& playerName);
    void RemoveRemotePlayer(uint32_t playerId);
    void UpdateRemotePlayerPosition(uint32_t playerId, const Vec3& position);
    void UpdateRemotePlayerRotation(uint32_t playerId, const Vec3& rotation);

    // Network utilities
    void SendPacket(const NetworkPacket& packet);
    void UpdatePing();
};

} // namespace VoxelCraft

#endif // VOXELCRAFT_NETWORK_CLIENT_HPP
