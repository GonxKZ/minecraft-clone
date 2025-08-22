/**
 * @file Client.cpp
 * @brief VoxelCraft Multiplayer Client Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "Client.hpp"
#include "Logger.hpp"
#include "../player/Player.hpp"

#include <chrono>

namespace VoxelCraft {

Client::Client()
    : m_connected(false)
    , m_initialized(false)
    , m_localPlayerId(0)
    , m_ping(0)
    , m_networkThreadRunning(false)
{
}

Client::~Client() {
    Shutdown();
}

bool Client::Initialize(const Config& config) {
    if (m_initialized) {
        return true;
    }

    VOXELCRAFT_INFO("Initializing VoxelCraft Multiplayer Client v1.0.0");

    try {
        // Create network manager
        m_networkManager = std::make_unique<NetworkManager>();
        if (!m_networkManager->Initialize(config)) {
            VOXELCRAFT_ERROR("Failed to initialize network manager");
            return false;
        }

        // Set up network callbacks
        m_networkManager->SetPlayerConnectedCallback(
            [this](uint32_t playerId) { OnConnected(); }
        );

        m_networkManager->SetPlayerDisconnectedCallback(
            [this](uint32_t playerId) { OnDisconnected(); }
        );

        m_networkManager->SetPacketReceivedCallback(
            [this](const NetworkPacket& packet) {
                std::lock_guard<std::mutex> lock(m_incomingMutex);
                m_incomingPackets.push(packet);
            }
        );

        m_networkManager->SetConnectionFailedCallback(
            [this](const std::string& reason) { OnConnectionFailed(reason); }
        );

        m_initialized = true;
        VOXELCRAFT_INFO("Multiplayer client initialized successfully");

        return true;

    } catch (const std::exception& e) {
        VOXELCRAFT_ERROR("Failed to initialize client: {}", e.what());
        return false;
    }
}

void Client::Shutdown() {
    if (!m_initialized) {
        return;
    }

    VOXELCRAFT_INFO("Shutting down VoxelCraft Multiplayer Client");

    // Disconnect
    Disconnect();

    // Clean up
    m_remotePlayers.clear();
    m_networkManager.reset();

    m_initialized = false;
    VOXELCRAFT_INFO("Multiplayer client shutdown complete");
}

void Client::Update(double deltaTime) {
    if (!m_initialized || !m_connected) {
        return;
    }

    // Process incoming packets
    ProcessIncomingPackets();

    // Update network manager
    if (m_networkManager) {
        m_networkManager->Update(deltaTime);
    }

    // Update ping
    UpdatePing();
}

bool Client::Connect(const std::string& serverAddress) {
    if (!m_initialized) {
        VOXELCRAFT_ERROR("Cannot connect: Client not initialized");
        return false;
    }

    if (m_connected) {
        VOXELCRAFT_WARNING("Already connected to server");
        return true;
    }

    VOXELCRAFT_INFO("Connecting to server: {}", serverAddress);

    m_serverAddress = serverAddress;
    m_connected = true;

    // Start network thread
    m_networkThreadRunning = true;
    m_networkThread = std::make_unique<std::thread>(&Client::NetworkThread, this);

    // Attempt connection
    if (m_networkManager && m_networkManager->Connect(serverAddress)) {
        VOXELCRAFT_INFO("Connection attempt started");
        return true;
    } else {
        VOXELCRAFT_ERROR("Failed to start connection");
        m_connected = false;
        return false;
    }
}

void Client::Disconnect() {
    if (!m_connected) {
        return;
    }

    VOXELCRAFT_INFO("Disconnecting from server");

    m_connected = false;
    m_networkThreadRunning = false;

    // Stop network thread
    if (m_networkThread && m_networkThread->joinable()) {
        m_networkThread->join();
    }

    // Disconnect network manager
    if (m_networkManager) {
        m_networkManager->Disconnect();
    }

    // Clear remote players
    std::lock_guard<std::mutex> lock(m_remotePlayersMutex);
    m_remotePlayers.clear();

    // Clear packet queues
    std::lock_guard<std::mutex> inLock(m_incomingMutex);
    while (!m_incomingPackets.empty()) m_incomingPackets.pop();

    std::lock_guard<std::mutex> outLock(m_outgoingMutex);
    while (!m_outgoingPackets.empty()) m_outgoingPackets.pop();

    VOXELCRAFT_INFO("Disconnected from server");
}

void Client::SendPlayerPosition(const Vec3& position) {
    if (!m_connected) return;

    auto packet = NetworkManager::CreatePlayerPositionPacket(m_localPlayerId, position);
    SendPacket(packet);
}

void Client::SendPlayerRotation(const Vec3& rotation) {
    if (!m_connected) return;

    // Create rotation packet (similar to position packet)
    NetworkPacket packet;
    packet.packetId = 0;
    packet.type = PacketType::PLAYER_ROTATION;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    packet.senderId = m_localPlayerId;
    packet.sequenceNumber = 0;
    packet.data = NetworkManager::SerializeVec3(rotation);
    packet.reliable = false;

    SendPacket(packet);
}

void Client::SendPlayerInput(const std::vector<uint8_t>& inputData) {
    if (!m_connected) return;

    NetworkPacket packet;
    packet.packetId = 0;
    packet.type = PacketType::PLAYER_INPUT;
    packet.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    packet.senderId = m_localPlayerId;
    packet.sequenceNumber = 0;
    packet.data = inputData;
    packet.reliable = true;

    SendPacket(packet);
}

void Client::SendChatMessage(const std::string& message) {
    if (!m_connected) return;

    auto packet = NetworkManager::CreateChatMessagePacket(m_localPlayerId, message);
    SendPacket(packet);
}

std::vector<std::shared_ptr<Player>> Client::GetRemotePlayers() {
    std::lock_guard<std::mutex> lock(m_remotePlayersMutex);
    std::vector<std::shared_ptr<Player>> players;

    for (const auto& pair : m_remotePlayers) {
        players.push_back(pair.second);
    }

    return players;
}

std::shared_ptr<Player> Client::GetRemotePlayer(uint32_t playerId) {
    std::lock_guard<std::mutex> lock(m_remotePlayersMutex);
    auto it = m_remotePlayers.find(playerId);
    return it != m_remotePlayers.end() ? it->second : nullptr;
}

void Client::NetworkThread() {
    VOXELCRAFT_INFO("Network thread started");

    while (m_networkThreadRunning) {
        try {
            // Process outgoing packets
            ProcessOutgoingPackets();

            // Update network manager
            if (m_networkManager) {
                m_networkManager->Update(0.05); // 20 TPS
            }

            // Sleep to prevent busy waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Network thread error: {}", e.what());
            break;
        }
    }

    VOXELCRAFT_INFO("Network thread stopped");
}

void Client::ProcessIncomingPackets() {
    std::lock_guard<std::mutex> lock(m_incomingMutex);

    while (!m_incomingPackets.empty()) {
        NetworkPacket packet = m_incomingPackets.front();
        m_incomingPackets.pop();

        // Handle packet based on type
        switch (packet.type) {
            case PacketType::HANDSHAKE:
                HandleHandshakePacket(packet);
                break;
            case PacketType::PLAYER_POSITION:
                HandlePlayerPositionPacket(packet);
                break;
            case PacketType::PLAYER_CONNECTED:
                HandlePlayerConnectedPacket(packet);
                break;
            case PacketType::PLAYER_DISCONNECTED:
                HandlePlayerDisconnectedPacket(packet);
                break;
            case PacketType::CHAT_MESSAGE:
                HandleChatMessagePacket(packet);
                break;
            case PacketType::WORLD_TIME:
                HandleWorldStatePacket(packet);
                break;
            default:
                VOXELCRAFT_TRACE("Unhandled packet type: {}", static_cast<int>(packet.type));
                break;
        }
    }
}

void Client::ProcessOutgoingPackets() {
    if (!m_networkManager) return;

    std::lock_guard<std::mutex> lock(m_outgoingMutex);

    while (!m_outgoingPackets.empty()) {
        NetworkPacket packet = m_outgoingPackets.front();
        m_outgoingPackets.pop();

        m_networkManager->BroadcastPacket(packet); // Send to server
    }
}

void Client::HandleHandshakePacket(const NetworkPacket& packet) {
    VOXELCRAFT_INFO("Received handshake from server");
    m_localPlayerId = packet.senderId;
}

void Client::HandlePlayerPositionPacket(const NetworkPacket& packet) {
    if (packet.data.size() >= sizeof(float) * 3) {
        Vec3 position = NetworkManager::DeserializeVec3(packet.data);
        UpdateRemotePlayerPosition(packet.senderId, position);
    }
}

void Client::HandlePlayerConnectedPacket(const NetworkPacket& packet) {
    // Extract player name from packet data
    std::string playerName;
    if (!packet.data.empty()) {
        playerName = std::string(packet.data.begin(), packet.data.end());
    }

    AddRemotePlayer(packet.senderId, playerName);
}

void Client::HandlePlayerDisconnectedPacket(const NetworkPacket& packet) {
    RemoveRemotePlayer(packet.senderId);
}

void Client::HandleChatMessagePacket(const NetworkPacket& packet) {
    if (!packet.data.empty()) {
        std::string message(packet.data.begin(), packet.data.end());
        VOXELCRAFT_INFO("[CHAT] {}", message);
    }
}

void Client::HandleWorldStatePacket(const NetworkPacket& packet) {
    // Handle world state updates (chunks, entities, etc.)
    VOXELCRAFT_TRACE("Received world state update");
}

void Client::OnConnected() {
    VOXELCRAFT_INFO("Connected to server successfully");
    m_connected = true;
}

void Client::OnDisconnected() {
    VOXELCRAFT_INFO("Disconnected from server");
    m_connected = false;
}

void Client::OnConnectionFailed(const std::string& reason) {
    VOXELCRAFT_ERROR("Connection failed: {}", reason);
    m_connected = false;
}

void Client::AddRemotePlayer(uint32_t playerId, const std::string& playerName) {
    std::lock_guard<std::mutex> lock(m_remotePlayersMutex);

    if (m_remotePlayers.find(playerId) == m_remotePlayers.end()) {
        auto player = std::make_shared<Player>(playerName);
        m_remotePlayers[playerId] = player;
        VOXELCRAFT_INFO("Remote player {} joined: {}", playerId, playerName);
    }
}

void Client::RemoveRemotePlayer(uint32_t playerId) {
    std::lock_guard<std::mutex> lock(m_remotePlayersMutex);

    auto it = m_remotePlayers.find(playerId);
    if (it != m_remotePlayers.end()) {
        VOXELCRAFT_INFO("Remote player {} left", playerId);
        m_remotePlayers.erase(it);
    }
}

void Client::UpdateRemotePlayerPosition(uint32_t playerId, const Vec3& position) {
    std::lock_guard<std::mutex> lock(m_remotePlayersMutex);

    auto it = m_remotePlayers.find(playerId);
    if (it != m_remotePlayers.end()) {
        it->second->Teleport(position);
    }
}

void Client::UpdateRemotePlayerRotation(uint32_t playerId, const Vec3& rotation) {
    std::lock_guard<std::mutex> lock(m_remotePlayersMutex);

    auto it = m_remotePlayers.find(playerId);
    if (it != m_remotePlayers.end()) {
        // Update player rotation (would need to add rotation to Player class)
        VOXELCRAFT_TRACE("Updated rotation for player {}", playerId);
    }
}

void Client::SendPacket(const NetworkPacket& packet) {
    std::lock_guard<std::mutex> lock(m_outgoingMutex);
    m_outgoingPackets.push(packet);
}

void Client::UpdatePing() {
    static auto lastPingTime = std::chrono::steady_clock::now();
    auto currentTime = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        currentTime - lastPingTime
    );

    // Send ping every 30 seconds
    if (elapsed.count() >= 30000) {
        NetworkPacket pingPacket;
        pingPacket.type = PacketType::PING;
        pingPacket.senderId = m_localPlayerId;
        SendPacket(pingPacket);
        lastPingTime = currentTime;
    }

    // Update ping based on pong responses (simplified)
    if (m_networkManager) {
        auto connections = m_networkManager->GetConnectedPlayers();
        if (!connections.empty()) {
            m_ping = connections[0].ping; // Simplified - take first connection's ping
        }
    }
}

} // namespace VoxelCraft
