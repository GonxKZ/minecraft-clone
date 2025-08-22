/**
 * @file Server.hpp
 * @brief VoxelCraft Dedicated Server
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_NETWORK_SERVER_HPP
#define VOXELCRAFT_NETWORK_SERVER_HPP

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <atomic>
#include <queue>
#include <chrono>

#include "../core/Config.hpp"
#include "../core/World.hpp"
#include "../core/NetworkManager.hpp"

namespace VoxelCraft {

class Server {
public:
    Server();
    ~Server();

    bool Initialize(const Config& config);
    void Shutdown();
    void Run();

    bool Start(uint16_t port, uint32_t maxPlayers = 10);
    void Stop();

    bool IsRunning() const { return m_running; }
    uint32_t GetConnectedPlayerCount() const;
    const std::vector<std::string> GetConnectedPlayerNames() const;

private:
    // Server configuration
    uint16_t m_port;
    uint32_t m_maxPlayers;
    std::string m_serverName;
    std::string m_serverMotd;

    // Server state
    std::atomic<bool> m_running;
    std::atomic<bool> m_initialized;

    // Game world
    std::unique_ptr<World> m_world;
    std::unique_ptr<NetworkManager> m_networkManager;

    // Threading
    std::unique_ptr<std::thread> m_serverThread;
    std::unique_ptr<std::thread> m_worldUpdateThread;

    // Synchronization
    mutable std::mutex m_worldMutex;

    // Server metrics
    struct ServerMetrics {
        uint64_t totalConnections;
        uint64_t totalDisconnections;
        double uptime;
        uint32_t peakPlayers;
        uint64_t totalPacketsSent;
        uint64_t totalPacketsReceived;
        double averageTickTime;
    } m_metrics;

    // Server methods
    void ServerLoop();
    void WorldUpdateLoop();
    void ProcessNetworkEvents();
    void UpdateServerMetrics();

    // Event handlers
    void OnPlayerConnected(uint32_t playerId);
    void OnPlayerDisconnected(uint32_t playerId);
    void OnPlayerInput(uint32_t playerId, const std::vector<uint8_t>& inputData);

    // World synchronization
    void BroadcastWorldState();
    void SendWorldStateToPlayer(uint32_t playerId);
    void ProcessPlayerActions();

    // Server commands
    void ProcessServerCommand(const std::string& command);

    // Logging
    void LogServerEvent(const std::string& event);
};

} // namespace VoxelCraft

#endif // VOXELCRAFT_NETWORK_SERVER_HPP
