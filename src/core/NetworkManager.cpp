/**
 * @file NetworkManager.cpp
 * @brief VoxelCraft Network Manager Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "NetworkManager.hpp"
#include "Config.hpp"
#include "Logger.hpp"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <sstream>
#include <iomanip>

namespace VoxelCraft {

    NetworkManager::NetworkManager()
        : m_mode(NetworkMode::OFFLINE)
        , m_state(NetworkState::DISCONNECTED)
        , m_serverPort(25565)
        , m_maxPlayers(10)
        , m_localPlayerId(0)
        , m_nextPacketId(1)
        , m_running(false)
        , m_networkImpl(nullptr)
    {
        // Initialize metrics
        std::memset(&m_metrics, 0, sizeof(NetworkMetrics));
    }

    NetworkManager::~NetworkManager() {
        Shutdown();
    }

    bool NetworkManager::Initialize(const Config& config) {
        VOXELCRAFT_INFO("Initializing Network Manager");

        try {
            // Load network configuration
            m_serverPort = config.Get("network.server_port", 25565);
            m_maxPlayers = config.Get("network.max_players", 10);
            std::string networkMode = config.Get("network.mode", "offline");

            // Set network mode
            if (networkMode == "client") {
                m_mode = NetworkMode::CLIENT;
            } else if (networkMode == "server") {
                m_mode = NetworkMode::SERVER;
            } else if (networkMode == "dedicated_server") {
                m_mode = NetworkMode::DEDICATED_SERVER;
            } else {
                m_mode = NetworkMode::OFFLINE;
            }

            // Initialize network implementation (stub for now)
            m_networkImpl = reinterpret_cast<void*>(0x1); // Placeholder

            VOXELCRAFT_INFO("Network Manager initialized successfully - Mode: {}",
                          networkMode);

            return true;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Failed to initialize Network Manager: {}", e.what());
            return false;
        }
    }

    void NetworkManager::Shutdown() {
        VOXELCRAFT_INFO("Shutting down Network Manager");

        // Stop network thread
        m_running = false;
        if (m_networkThread && m_networkThread->joinable()) {
            m_networkThread->join();
        }

        // Disconnect all players
        Disconnect();

        // Clean up
        m_players.clear();
        while (!m_incomingPackets.empty()) m_incomingPackets.pop();
        while (!m_outgoingPackets.empty()) m_outgoingPackets.pop();

        m_networkImpl = nullptr;

        VOXELCRAFT_INFO("Network Manager shutdown complete");
    }

    void NetworkManager::Update(double deltaTime) {
        if (m_mode == NetworkMode::OFFLINE) {
            return;
        }

        // Process network events
        ProcessEvents();

        // Update metrics
        UpdateMetrics();

        // Clean up disconnected players
        CleanupDisconnectedPlayers();
    }

    void NetworkManager::ProcessEvents() {
        std::lock_guard<std::mutex> lock(m_packetsMutex);

        // Process incoming packets
        while (!m_incomingPackets.empty()) {
            NetworkPacket packet = m_incomingPackets.front();
            m_incomingPackets.pop();

            // Handle packet based on type
            switch (packet.type) {
                case PacketType::HANDSHAKE:
                    HandlePlayerConnected(packet.senderId);
                    break;
                case PacketType::LOGOUT:
                    HandlePlayerDisconnected(packet.senderId);
                    break;
                default:
                    // Call packet received callback
                    if (m_packetReceivedCallback) {
                        m_packetReceivedCallback(packet);
                    }
                    break;
            }

            m_metrics.packetsReceived++;
            m_metrics.bytesReceived += packet.data.size();
        }

        // Process outgoing packets
        while (!m_outgoingPackets.empty()) {
            NetworkPacket packet = m_outgoingPackets.front();
            m_outgoingPackets.pop();

            if (SendNetworkPacket(packet)) {
                m_metrics.packetsSent++;
                m_metrics.bytesSent += packet.data.size();
            } else {
                m_metrics.packetsLost++;
            }
        }
    }

    bool NetworkManager::Connect(const std::string& address) {
        if (m_mode != NetworkMode::CLIENT) {
            VOXELCRAFT_ERROR("Cannot connect: Network manager not in client mode");
            return false;
        }

        if (m_state != NetworkState::DISCONNECTED) {
            VOXELCRAFT_ERROR("Cannot connect: Already connected or connecting");
            return false;
        }

        VOXELCRAFT_INFO("Connecting to server: {}", address);

        m_serverAddress = address;
        m_state = NetworkState::CONNECTING;

        // Start network thread
        m_running = true;
        m_networkThread = std::make_unique<std::thread>(&NetworkManager::NetworkThread, this);

        return true;
    }

    bool NetworkManager::StartServer(uint16_t port, uint32_t maxPlayers) {
        if (m_mode != NetworkMode::SERVER && m_mode != NetworkMode::DEDICATED_SERVER) {
            VOXELCRAFT_ERROR("Cannot start server: Network manager not in server mode");
            return false;
        }

        if (m_state != NetworkState::DISCONNECTED) {
            VOXELCRAFT_ERROR("Cannot start server: Already running");
            return false;
        }

        VOXELCRAFT_INFO("Starting server on port {} with max {} players", port, maxPlayers);

        m_serverPort = port;
        m_maxPlayers = maxPlayers;
        m_state = NetworkState::CONNECTED;

        // Start network thread
        m_running = true;
        m_networkThread = std::make_unique<std::thread>(&NetworkManager::NetworkThread, this);

        return true;
    }

    void NetworkManager::Disconnect() {
        VOXELCRAFT_INFO("Disconnecting from network");

        m_running = false;
        m_state = NetworkState::DISCONNECTING;

        // Stop network thread
        if (m_networkThread && m_networkThread->joinable()) {
            m_networkThread->join();
        }

        // Disconnect all players
        std::lock_guard<std::mutex> lock(m_playersMutex);
        for (auto& pair : m_players) {
            if (m_playerDisconnectedCallback) {
                m_playerDisconnectedCallback(pair.first);
            }
        }
        m_players.clear();

        m_state = NetworkState::DISCONNECTED;
        VOXELCRAFT_INFO("Network disconnection complete");
    }

    bool NetworkManager::SendPacket(uint32_t playerId, const NetworkPacket& packet) {
        if (m_mode == NetworkMode::OFFLINE) {
            return false;
        }

        std::lock_guard<std::mutex> lock(m_packetsMutex);
        NetworkPacket sendPacket = packet;
        sendPacket.packetId = m_nextPacketId++;

        m_outgoingPackets.push(sendPacket);
        return true;
    }

    bool NetworkManager::BroadcastPacket(const NetworkPacket& packet) {
        if (m_mode == NetworkMode::OFFLINE) {
            return false;
        }

        std::lock_guard<std::mutex> playersLock(m_playersMutex);
        std::lock_guard<std::mutex> packetsLock(m_packetsMutex);

        NetworkPacket broadcastPacket = packet;
        broadcastPacket.packetId = m_nextPacketId++;

        // Send to all connected players
        for (const auto& pair : m_players) {
            if (pair.second.state == NetworkState::CONNECTED) {
                m_outgoingPackets.push(broadcastPacket);
            }
        }

        return true;
    }

    const PlayerConnection* NetworkManager::GetPlayerConnection(uint32_t playerId) const {
        std::lock_guard<std::mutex> lock(m_playersMutex);
        auto it = m_players.find(playerId);
        return it != m_players.end() ? &it->second : nullptr;
    }

    std::vector<PlayerConnection> NetworkManager::GetConnectedPlayers() const {
        std::lock_guard<std::mutex> lock(m_playersMutex);
        std::vector<PlayerConnection> connectedPlayers;

        for (const auto& pair : m_players) {
            if (pair.second.state == NetworkState::CONNECTED) {
                connectedPlayers.push_back(pair.second);
            }
        }

        return connectedPlayers;
    }

    bool NetworkManager::KickPlayer(uint32_t playerId, const std::string& reason) {
        auto player = GetPlayerConnection(playerId);
        if (!player) {
            return false;
        }

        VOXELCRAFT_INFO("Kicking player {} ({}) - Reason: {}", playerId, player->playerName, reason);

        std::lock_guard<std::mutex> lock(m_playersMutex);
        m_players.erase(playerId);

        if (m_playerDisconnectedCallback) {
            m_playerDisconnectedCallback(playerId);
        }

        return true;
    }

    bool NetworkManager::BanPlayer(uint32_t playerId, const std::string& reason) {
        auto player = GetPlayerConnection(playerId);
        if (!player) {
            return false;
        }

        VOXELCRAFT_INFO("Banning player {} ({}) - Reason: {}", playerId, player->playerName, reason);

        // Add to ban list
        std::lock_guard<std::mutex> banLock(m_banListMutex);
        m_bannedAddresses.push_back(player->address);

        // Kick the player
        return KickPlayer(playerId, "Banned: " + reason);
    }

    bool NetworkManager::IsPlayerBanned(const std::string& address) const {
        std::lock_guard<std::mutex> lock(m_banListMutex);
        return std::find(m_bannedAddresses.begin(), m_bannedAddresses.end(), address) != m_bannedAddresses.end();
    }

    // Private methods

    void NetworkManager::NetworkThread() {
        VOXELCRAFT_INFO("Network thread started");

        while (m_running) {
            try {
                // Receive packets
                auto packets = ReceiveNetworkPackets();

                // Add to incoming queue
                {
                    std::lock_guard<std::mutex> lock(m_packetsMutex);
                    for (const auto& packet : packets) {
                        m_incomingPackets.push(packet);
                    }
                }

                // Process outgoing packets
                ProcessOutgoingPackets();

                // Sleep to prevent busy waiting
                std::this_thread::sleep_for(std::chrono::milliseconds(10));

            } catch (const std::exception& e) {
                VOXELCRAFT_ERROR("Network thread error: {}", e.what());
                m_state = NetworkState::ERROR;
                break;
            }
        }

        VOXELCRAFT_INFO("Network thread stopped");
    }

    void NetworkManager::ProcessIncomingPackets() {
        // Already handled in ProcessEvents()
    }

    void NetworkManager::ProcessOutgoingPackets() {
        std::lock_guard<std::mutex> lock(m_packetsMutex);

        while (!m_outgoingPackets.empty()) {
            NetworkPacket packet = m_outgoingPackets.front();
            m_outgoingPackets.pop();

            if (SendNetworkPacket(packet)) {
                m_metrics.packetsSent++;
                m_metrics.bytesSent += packet.data.size();
            } else {
                m_metrics.packetsLost++;
            }
        }
    }

    void NetworkManager::HandlePlayerConnected(uint32_t playerId) {
        std::lock_guard<std::mutex> lock(m_playersMutex);

        // Create new player connection
        PlayerConnection connection;
        connection.playerId = playerId;
        connection.playerName = "Player_" + std::to_string(playerId);
        connection.address = "127.0.0.1"; // Placeholder
        connection.state = NetworkState::CONNECTED;
        connection.lastActivity = std::chrono::steady_clock::now();
        connection.ping = 0;
        connection.authenticated = true;

        m_players[playerId] = connection;
        m_metrics.connectedPlayers = m_players.size();

        VOXELCRAFT_INFO("Player {} connected", playerId);

        if (m_playerConnectedCallback) {
            m_playerConnectedCallback(playerId);
        }
    }

    void NetworkManager::HandlePlayerDisconnected(uint32_t playerId) {
        std::lock_guard<std::mutex> lock(m_playersMutex);
        auto it = m_players.find(playerId);

        if (it != m_players.end()) {
            VOXELCRAFT_INFO("Player {} disconnected", playerId);
            m_players.erase(it);
            m_metrics.connectedPlayers = m_players.size();

            if (m_playerDisconnectedCallback) {
                m_playerDisconnectedCallback(playerId);
            }
        }
    }

    uint32_t NetworkManager::AuthenticatePlayer(const std::string& address, const std::string& credentials) {
        // Simple authentication - just assign an ID
        static uint32_t nextId = 1;
        return nextId++;
    }

    bool NetworkManager::SendNetworkPacket(const NetworkPacket& packet) {
        // Stub implementation - in real implementation would use sockets
        return m_state == NetworkState::CONNECTED;
    }

    std::vector<NetworkPacket> NetworkManager::ReceiveNetworkPackets() {
        // Stub implementation - in real implementation would receive from sockets
        return std::vector<NetworkPacket>();
    }

    void NetworkManager::UpdateMetrics() {
        std::lock_guard<std::mutex> lock(m_metricsMutex);

        // Calculate bandwidth usage (simplified)
        static auto lastUpdate = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastUpdate).count();

        if (elapsed > 0) {
            m_metrics.bandwidthUsed = (m_metrics.bytesSent + m_metrics.bytesReceived) / elapsed;
            lastUpdate = now;
        }

        // Update connected players count
        {
            std::lock_guard<std::mutex> playersLock(m_playersMutex);
            m_metrics.connectedPlayers = m_players.size();
        }
    }

    void NetworkManager::CleanupDisconnectedPlayers() {
        std::lock_guard<std::mutex> lock(m_playersMutex);
        auto now = std::chrono::steady_clock::now();
        auto timeout = std::chrono::seconds(30); // 30 second timeout

        for (auto it = m_players.begin(); it != m_players.end();) {
            auto elapsed = now - it->second.lastActivity;
            if (elapsed > timeout && it->second.state != NetworkState::CONNECTED) {
                VOXELCRAFT_INFO("Removing timed out player {}", it->first);
                it = m_players.erase(it);
            } else {
                ++it;
            }
        }
    }

    // Static helper methods

    std::vector<uint8_t> NetworkManager::SerializeVec3(const Vec3& vec) {
        std::vector<uint8_t> data(sizeof(float) * 3);
        float* floats = reinterpret_cast<float*>(data.data());
        floats[0] = vec.x;
        floats[1] = vec.y;
        floats[2] = vec.z;
        return data;
    }

    Vec3 NetworkManager::DeserializeVec3(const std::vector<uint8_t>& data) {
        if (data.size() < sizeof(float) * 3) {
            return Vec3(0, 0, 0);
        }

        const float* floats = reinterpret_cast<const float*>(data.data());
        return Vec3(floats[0], floats[1], floats[2]);
    }

    NetworkPacket NetworkManager::CreatePlayerPositionPacket(uint32_t playerId, const Vec3& position) {
        NetworkPacket packet;
        packet.packetId = 0; // Will be set by sender
        packet.type = PacketType::PLAYER_POSITION;
        packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
        packet.senderId = playerId;
        packet.sequenceNumber = 0;
        packet.data = SerializeVec3(position);
        packet.reliable = false; // Position updates don't need to be 100% reliable

        return packet;
    }

    NetworkPacket NetworkManager::CreateChatMessagePacket(uint32_t playerId, const std::string& message) {
        NetworkPacket packet;
        packet.packetId = 0; // Will be set by sender
        packet.type = PacketType::CHAT_MESSAGE;
        packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
        packet.senderId = playerId;
        packet.sequenceNumber = 0;
        packet.data = std::vector<uint8_t>(message.begin(), message.end());
        packet.reliable = true; // Chat messages must be reliable

        return packet;
    }

} // namespace VoxelCraft
