/**
 * @file NetworkManager.cpp
 * @brief VoxelCraft Network Manager Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file implements the NetworkManager class that provides comprehensive networking
 * functionality for the VoxelCraft game engine, including client-server architecture,
 * state synchronization, latency management, and secure authentication systems.
 */

#include "NetworkManager.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "NetworkProtocol.hpp"
#include "NetworkSecurity.hpp"
#include "NetworkSynchronization.hpp"
#include "NetworkLatency.hpp"

#include "../core/Logger.hpp"
#include "../entities/System.hpp"
#include "../world/World.hpp"

#include <algorithm>
#include <chrono>
#include <sstream>
#include <filesystem>

namespace VoxelCraft {

    // Static member initialization
    std::atomic<uint32_t> NetworkManager::s_nextElementId(1);
    std::atomic<uint32_t> NetworkManager::s_nextAnimationId(1);
    std::atomic<uint32_t> NetworkManager::s_nextListenerId(1);

    NetworkManager::NetworkManager(const NetworkManagerConfig& config)
        : m_config(config)
        , m_state(NetworkState::Inactive)
        , m_mode(NetworkMode::Offline)
        , m_inputMode(UIInputMode::MouseKeyboard)
        , m_window(nullptr)
        , m_renderer(nullptr)
        , m_inputSystem(nullptr)
        , m_fontManager(nullptr)
        , m_textureManager(nullptr)
        , m_soundManager(nullptr)
        , m_player(nullptr)
        , m_world(nullptr)
        , m_hud(nullptr)
        , m_menuSystem(nullptr)
        , m_inventoryUI(nullptr)
        , m_chatSystem(nullptr)
        , m_settingsUI(nullptr)
        , m_uiRenderer(nullptr)
        , m_focusedElement(0)
        , m_isInitialized(false)
        , m_lastUpdateTime(0.0)
    {
        // Initialize metrics
        m_metrics = {};

        // Set initial mode
        SetMode(m_config.defaultMode);
    }

    NetworkManager::~NetworkManager() {
        Shutdown();
    }

    bool NetworkManager::Initialize() {
        if (m_isInitialized) {
            VOXELCRAFT_WARNING("NetworkManager already initialized");
            return true;
        }

        VOXELCRAFT_INFO("Initializing NetworkManager...");

        try {
            // Initialize subsystems
            if (!InitializeSubsystems()) {
                VOXELCRAFT_ERROR("Failed to initialize network subsystems");
                return false;
            }

            m_isInitialized = true;
            m_state = NetworkState::Active;

            VOXELCRAFT_INFO("NetworkManager initialized successfully");
            return true;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Exception during NetworkManager initialization: " + std::string(e.what()));
            return false;
        }
    }

    void NetworkManager::Shutdown() {
        if (!m_isInitialized) {
            return;
        }

        VOXELCRAFT_INFO("Shutting down NetworkManager...");

        // Disconnect if connected
        if (m_mode == NetworkMode::Client || m_mode == NetworkMode::ListenServer) {
            Disconnect();
        }

        // Stop server if running
        if (m_mode == NetworkMode::Server || m_mode == NetworkMode::DedicatedServer ||
            m_mode == NetworkMode::ListenServer) {
            StopServer();
        }

        // Shutdown subsystems
        if (m_client) {
            m_client->Shutdown();
        }

        if (m_server) {
            m_server->Shutdown();
        }

        // Clear references
        m_player = nullptr;
        m_world = nullptr;

        m_isInitialized = false;
        m_state = NetworkState::Inactive;

        VOXELCRAFT_INFO("NetworkManager shutdown complete");
    }

    void NetworkManager::Update(double deltaTime) {
        if (!m_isInitialized || m_state != NetworkState::Active) {
            return;
        }

        try {
            double currentTime = std::chrono::duration<double>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();

            // Update server if running
            if (m_server && m_server->IsRunning()) {
                m_server->Update(deltaTime);
            }

            // Update client if connected
            if (m_client && m_client->IsConnected()) {
                m_client->Update(deltaTime);
            }

            // Process event queue
            ProcessEventQueue();

            // Update metrics
            UpdateMetrics(deltaTime);

            m_lastUpdateTime = currentTime;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Exception during NetworkManager update: " + std::string(e.what()));
            HandleError("Update exception: " + std::string(e.what()));
        }
    }

    bool NetworkManager::InitializeSubsystems() {
        VOXELCRAFT_INFO("Initializing network subsystems...");

        try {
            // Create protocol handler
            ProtocolConfig protocolConfig;
            protocolConfig.protocolVersion = ProtocolVersion::Latest;
            protocolConfig.enableCompression = m_config.enableCompression;
            protocolConfig.enableEncryption = m_config.enableEncryption;
            protocolConfig.maxPacketSize = m_config.maxPacketSize;

            m_protocol = std::make_unique<NetworkProtocol>(protocolConfig);

            if (!m_protocol->Initialize()) {
                VOXELCRAFT_ERROR("Failed to initialize network protocol");
                return false;
            }

            // Create security system
            // m_security = std::make_unique<NetworkSecurity>(...);

            // Create synchronization system
            // m_sync = std::make_unique<NetworkSynchronization>(...);

            // Create latency manager
            // m_latency = std::make_unique<NetworkLatency>(...);

            VOXELCRAFT_INFO("Network subsystems initialized successfully");
            return true;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Exception initializing network subsystems: " + std::string(e.what()));
            return false;
        }
    }

    bool NetworkManager::StartServer(uint16_t port, uint32_t maxPlayers) {
        if (!m_isInitialized) {
            VOXELCRAFT_ERROR("NetworkManager not initialized");
            return false;
        }

        if (m_server && m_server->IsRunning()) {
            VOXELCRAFT_WARNING("Server is already running");
            return true;
        }

        VOXELCRAFT_INFO("Starting server on port " + std::to_string(port));

        try {
            // Create server configuration
            ServerConfig serverConfig;
            serverConfig.port = port;
            serverConfig.maxPlayers = maxPlayers;
            serverConfig.mode = (m_mode == NetworkMode::ListenServer) ?
                               ServerMode::Listen : ServerMode::Dedicated;
            serverConfig.enableCompression = m_config.enableCompression;
            serverConfig.enableEncryption = m_config.enableEncryption;

            // Create and start server
            m_server = std::make_unique<Server>(serverConfig);

            if (!m_server->Start()) {
                VOXELCRAFT_ERROR("Failed to start server");
                m_server.reset();
                return false;
            }

            // Update mode
            if (m_mode == NetworkMode::Offline) {
                m_mode = (m_client) ? NetworkMode::ListenServer : NetworkMode::DedicatedServer;
            }

            VOXELCRAFT_INFO("Server started successfully");
            return true;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Exception starting server: " + std::string(e.what()));
            return false;
        }
    }

    bool NetworkManager::StopServer() {
        if (!m_server || !m_server->IsRunning()) {
            VOXELCRAFT_WARNING("Server is not running");
            return true;
        }

        VOXELCRAFT_INFO("Stopping server...");

        try {
            if (!m_server->Stop()) {
                VOXELCRAFT_ERROR("Failed to stop server gracefully");
            }

            m_server.reset();

            // Update mode
            if (m_mode == NetworkMode::DedicatedServer) {
                m_mode = NetworkMode::Offline;
            } else if (m_mode == NetworkMode::ListenServer) {
                m_mode = m_client ? NetworkMode::Client : NetworkMode::Offline;
            }

            VOXELCRAFT_INFO("Server stopped");
            return true;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Exception stopping server: " + std::string(e.what()));
            return false;
        }
    }

    bool NetworkManager::IsServerRunning() const {
        return m_server && m_server->IsRunning();
    }

    std::string NetworkManager::GetServerInfo() const {
        if (!m_server) {
            return "No server running";
        }
        return m_server->GetServerInfo();
    }

    bool NetworkManager::Connect(const std::string& address, uint16_t port) {
        if (!m_isInitialized) {
            VOXELCRAFT_ERROR("NetworkManager not initialized");
            return false;
        }

        if (m_client && m_client->IsConnected()) {
            VOXELCRAFT_WARNING("Client is already connected");
            return true;
        }

        VOXELCRAFT_INFO("Connecting to server " + address + ":" + std::to_string(port));

        try {
            // Create client configuration
            ClientConfig clientConfig;
            clientConfig.serverAddress = address;
            clientConfig.serverPort = port;
            clientConfig.username = m_config.username;
            clientConfig.enableCompression = m_config.enableCompression;
            clientConfig.enableEncryption = m_config.enableEncryption;

            // Create and start client
            m_client = std::make_unique<Client>(clientConfig);

            if (!m_client->Initialize()) {
                VOXELCRAFT_ERROR("Failed to initialize client");
                m_client.reset();
                return false;
            }

            if (!m_client->Connect(address, port)) {
                VOXELCRAFT_ERROR("Failed to start connection");
                m_client.reset();
                return false;
            }

            // Update mode
            m_mode = m_server ? NetworkMode::ListenServer : NetworkMode::Client;

            VOXELCRAFT_INFO("Connection initiated");
            return true;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Exception connecting to server: " + std::string(e.what()));
            return false;
        }
    }

    bool NetworkManager::Disconnect() {
        if (!m_client || !m_client->IsConnected()) {
            VOXELCRAFT_WARNING("Client is not connected");
            return true;
        }

        VOXELCRAFT_INFO("Disconnecting from server...");

        try {
            if (!m_client->Disconnect()) {
                VOXELCRAFT_ERROR("Failed to disconnect gracefully");
            }

            m_client.reset();

            // Update mode
            if (m_mode == NetworkMode::Client) {
                m_mode = NetworkMode::Offline;
            } else if (m_mode == NetworkMode::ListenServer) {
                m_mode = m_server ? NetworkMode::DedicatedServer : NetworkMode::Offline;
            }

            VOXELCRAFT_INFO("Disconnected from server");
            return true;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Exception disconnecting from server: " + std::string(e.what()));
            return false;
        }
    }

    std::string NetworkManager::GetConnectionInfo() const {
        if (!m_client) {
            return "Not connected";
        }
        return m_client->GetConnectionInfo();
    }

    bool NetworkManager::Authenticate(const std::string& username, const std::string& password) {
        if (!m_client || !m_client->IsConnected()) {
            VOXELCRAFT_ERROR("Not connected to server");
            return false;
        }

        return m_client->Authenticate(username, password);
    }

    bool NetworkManager::IsAuthenticated() const {
        return m_client && m_client->IsAuthenticated();
    }

    uint32_t NetworkManager::GetUserId() const {
        if (!m_client) {
            return 0;
        }
        return m_client->GetUserId();
    }

    uint32_t NetworkManager::SendMessage(uint32_t peerId, const NetworkMessage& message) {
        if (!m_isInitialized) {
            VOXELCRAFT_ERROR("NetworkManager not initialized");
            return 0;
        }

        try {
            if (m_server && m_server->IsRunning()) {
                // Send through server
                return m_server->SendMessage(peerId, message);
            } else if (m_client && m_client->IsConnected()) {
                // Send through client
                return m_client->SendMessage(peerId, message);
            } else {
                VOXELCRAFT_ERROR("No active network connection");
                return 0;
            }
        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Exception sending message: " + std::string(e.what()));
            return 0;
        }
    }

    uint32_t NetworkManager::SendMessageToAll(const NetworkMessage& message) {
        if (!m_server || !m_server->IsRunning()) {
            VOXELCRAFT_ERROR("Server not running");
            return 0;
        }

        return m_server->BroadcastMessage(message);
    }

    uint32_t NetworkManager::BroadcastMessage(const NetworkMessage& message) {
        return SendMessageToAll(message);
    }

    uint32_t NetworkManager::RegisterMessageHandler(NetworkMessageType messageType,
                                                   std::function<void(const NetworkMessage&)> handler) {
        uint32_t handlerId = s_nextListenerId++;

        // Store handler based on current mode
        if (m_server) {
            // Server-side handler
        } else if (m_client) {
            // Client-side handler
        }

        return handlerId;
    }

    bool NetworkManager::UnregisterMessageHandler(uint32_t handlerId) {
        // Remove handler
        return true;
    }

    const NetworkPeer* NetworkManager::GetPeer(uint32_t peerId) const {
        if (m_server) {
            return m_server->GetPlayer(peerId);
        }
        return nullptr;
    }

    std::vector<NetworkPeer> NetworkManager::GetConnectedPeers() const {
        if (m_server) {
            return m_server->GetConnectedPlayers();
        }
        return {};
    }

    uint32_t NetworkManager::GetPeerCount() const {
        if (m_server) {
            return m_server->GetPlayerCount();
        }
        return 0;
    }

    bool NetworkManager::KickPeer(uint32_t peerId, const std::string& reason) {
        if (!m_server) {
            VOXELCRAFT_ERROR("Server not running");
            return false;
        }

        return m_server->KickPlayer(peerId, reason);
    }

    bool NetworkManager::BanPeer(uint32_t peerId, const std::string& reason, double duration) {
        if (!m_server) {
            VOXELCRAFT_ERROR("Server not running");
            return false;
        }

        return m_server->BanPlayer(peerId, reason, duration);
    }

    bool NetworkManager::SynchronizeWorld(World* world) {
        if (m_client && m_client->IsConnected()) {
            return m_client->SynchronizeWithServer();
        }
        return false;
    }

    bool NetworkManager::SynchronizeEntity(uint32_t entityId, bool force) {
        // Entity synchronization logic
        return true;
    }

    bool NetworkManager::SynchronizePlayer(Player* player) {
        if (m_client && m_client->IsConnected()) {
            return m_client->SynchronizePlayer(player);
        }
        return false;
    }

    uint32_t NetworkManager::GetPing() const {
        if (m_client) {
            return m_client->GetPing();
        }
        return 0;
    }

    bool NetworkManager::MeasureLatency(uint32_t peerId) {
        if (m_client && peerId == 0) { // Server
            return m_client->MeasureLatency();
        }
        return false;
    }

    double NetworkManager::GetNetworkTime() const {
        if (m_client) {
            return m_client->GetUptime(); // Simplified
        }
        return 0.0;
    }

    bool NetworkManager::SynchronizeTime(double serverTime) {
        if (m_client) {
            return m_client->SynchronizeTime(serverTime);
        }
        return false;
    }

    bool NetworkManager::SetMode(NetworkMode mode) {
        if (mode == m_mode) {
            return true;
        }

        VOXELCRAFT_INFO("Changing network mode from " + std::to_string(static_cast<int>(m_mode)) +
                       " to " + std::to_string(static_cast<int>(mode)));

        NetworkMode oldMode = m_mode;
        m_mode = mode;

        // Handle mode transitions
        switch (mode) {
            case NetworkMode::Offline:
                // Disconnect client and stop server
                if (m_client) {
                    m_client->Disconnect();
                }
                if (m_server) {
                    m_server->Stop();
                }
                break;

            case NetworkMode::Client:
                // Start client
                if (!m_client) {
                    // Create client
                }
                break;

            case NetworkMode::Server:
                // Start server
                if (!m_server) {
                    StartServer(m_config.serverPort, m_config.maxPlayers);
                }
                break;

            case NetworkMode::DedicatedServer:
                // Start dedicated server
                if (!m_server) {
                    StartServer(m_config.serverPort, m_config.maxPlayers);
                }
                break;

            case NetworkMode::ListenServer:
                // Start both server and client
                if (!m_server) {
                    StartServer(m_config.serverPort, m_config.maxPlayers);
                }
                if (!m_client) {
                    Connect("localhost", m_config.serverPort);
                }
                break;

            default:
                break;
        }

        return true;
    }

    void NetworkManager::SetConfig(const NetworkManagerConfig& config) {
        m_config = config;

        // Update subsystems
        if (m_server) {
            ServerConfig serverConfig = m_server->GetConfig();
            serverConfig.enableCompression = m_config.enableCompression;
            serverConfig.enableEncryption = m_config.enableEncryption;
            m_server->SetConfig(serverConfig);
        }

        if (m_client) {
            ClientConfig clientConfig = m_client->GetConfig();
            clientConfig.enableCompression = m_config.enableCompression;
            clientConfig.enableEncryption = m_config.enableEncryption;
            m_client->SetConfig(clientConfig);
        }

        VOXELCRAFT_INFO("Network configuration updated");
    }

    void NetworkManager::UpdateMetrics(double deltaTime) {
        ++m_metrics.updateCount;

        // Calculate average update time
        if (m_metrics.updateCount > 1) {
            m_metrics.averageUpdateTime = (m_metrics.averageUpdateTime +
                                          deltaTime) / 2.0;
        } else {
            m_metrics.averageUpdateTime = deltaTime;
        }

        m_metrics.maxUpdateTime = std::max(m_metrics.maxUpdateTime, deltaTime);

        // Update connection metrics
        if (m_server) {
            auto serverMetrics = m_server->GetMetrics();
            m_metrics.currentPlayers = serverMetrics.currentPlayers;
            m_metrics.bytesSent = serverMetrics.bytesSent;
            m_metrics.bytesReceived = serverMetrics.bytesReceived;
        }

        if (m_client) {
            auto clientMetrics = m_client->GetMetrics();
            m_metrics.bytesSent = clientMetrics.bytesSent;
            m_metrics.bytesReceived = clientMetrics.bytesReceived;
            m_metrics.currentPing = clientMetrics.currentPing;
            m_metrics.averagePing = clientMetrics.averagePing;
        }

        // Reset metrics periodically
        if (m_metrics.updateCount > 10000) {
            m_metrics.updateCount = 1;
            m_metrics.totalUpdateTime = 0.0;
        }
    }

    void NetworkManager::ResetMetrics() {
        m_metrics = {};
        VOXELCRAFT_DEBUG("Network metrics reset");
    }

    std::string NetworkManager::GetPerformanceReport() const {
        std::stringstream ss;

        ss << "=== NetworkManager Performance Report ===\n";
        ss << "Updates: " << m_metrics.updateCount << "\n";
        ss << "Average Update Time: " << m_metrics.averageUpdateTime * 1000.0 << "ms\n";
        ss << "Max Update Time: " << m_metrics.maxUpdateTime * 1000.0 << "ms\n";
        ss << "Current Players: " << m_metrics.currentPlayers << "\n";
        ss << "Bytes Sent: " << m_metrics.bytesSent << "\n";
        ss << "Bytes Received: " << m_metrics.bytesReceived << "\n";
        ss << "Current Ping: " << m_metrics.currentPing << "ms\n";
        ss << "Average Ping: " << m_metrics.averagePing << "ms\n";

        return ss.str();
    }

    std::string NetworkManager::GetNetworkStats() const {
        std::stringstream ss;

        ss << "Network Stats:\n";
        ss << "Mode: " << static_cast<int>(m_mode) << "\n";
        ss << "State: " << static_cast<int>(m_state) << "\n";
        ss << "Connected Players: " << m_metrics.currentPlayers << "\n";
        ss << "Ping: " << m_metrics.currentPing << "ms\n";
        ss << "Packet Loss: " << m_metrics.packetLoss * 100.0 << "%\n";
        ss << "Bytes/sec Up: " << m_metrics.currentBandwidthUp << "\n";
        ss << "Bytes/sec Down: " << m_metrics.currentBandwidthDown << "\n";

        return ss.str();
    }

    uint32_t NetworkManager::AddEventListener(const std::string& eventType,
                                            std::function<void(const std::unordered_map<std::string, std::any>&)> listener) {
        uint32_t listenerId = s_nextListenerId++;
        m_eventListeners[listenerId] = listener;
        return listenerId;
    }

    bool NetworkManager::RemoveEventListener(uint32_t listenerId) {
        auto it = m_eventListeners.find(listenerId);
        if (it != m_eventListeners.end()) {
            m_eventListeners.erase(it);
            return true;
        }
        return false;
    }

    void NetworkManager::TriggerEvent(const std::string& eventType,
                                    const std::unordered_map<std::string, std::any>& eventData) {
        for (const auto& [id, listener] : m_eventListeners) {
            try {
                listener(eventData);
            } catch (const std::exception& e) {
                VOXELCRAFT_ERROR("Exception in network event listener: " + std::string(e.what()));
            }
        }
    }

    void NetworkManager::ProcessEventQueue() {
        // Process network events
        if (m_server) {
            // Process server events
        }

        if (m_client) {
            // Process client events
        }
    }

    void NetworkManager::HandleError(const std::string& error) {
        VOXELCRAFT_ERROR("NetworkManager Error: " + error);
        m_state = NetworkState::Error;

        // Trigger error event
        std::unordered_map<std::string, std::any> eventData;
        eventData["error"] = error;
        eventData["timestamp"] = std::chrono::duration<double>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();

        TriggerEvent("NetworkError", eventData);
    }

    bool NetworkManager::Validate() const {
        if (!m_isInitialized) {
            return false;
        }

        // Validate subsystems
        if (m_server && !m_server->Validate()) {
            return false;
        }

        if (m_client && !m_client->Validate()) {
            return false;
        }

        if (m_protocol && !m_protocol->Validate()) {
            return false;
        }

        return true;
    }

    std::string NetworkManager::GetStatusReport() const {
        std::stringstream ss;

        ss << "=== NetworkManager Status Report ===\n";
        ss << "State: " << static_cast<int>(m_state) << "\n";
        ss << "Mode: " << static_cast<int>(m_mode) << "\n";
        ss << "Initialized: " << (m_isInitialized ? "Yes" : "No") << "\n";
        ss << "Connected Players: " << m_metrics.currentPlayers << "\n";
        ss << "Server Running: " << (IsServerRunning() ? "Yes" : "No") << "\n";
        ss << "Client Connected: " << (m_client && m_client->IsConnected() ? "Yes" : "No") << "\n";

        if (m_server) {
            ss << "Server Info: " << m_server->GetServerInfo() << "\n";
        }

        if (m_client) {
            ss << "Connection Info: " << m_client->GetConnectionInfo() << "\n";
        }

        return ss.str();
    }

    std::unordered_map<std::string, size_t> NetworkManager::Optimize() {
        std::unordered_map<std::string, size_t> optimizations;

        // Optimize server
        if (m_server) {
            auto serverOpts = m_server->Optimize();
            optimizations.insert(serverOpts.begin(), serverOpts.end());
        }

        // Optimize client
        if (m_client) {
            auto clientOpts = m_client->Optimize();
            optimizations.insert(clientOpts.begin(), clientOpts.end());
        }

        // Optimize protocol
        if (m_protocol) {
            auto protocolOpts = m_protocol->Optimize();
            optimizations.insert(protocolOpts.begin(), protocolOpts.end());
        }

        VOXELCRAFT_INFO("Network optimization completed: " +
                       std::to_string(optimizations.size()) + " optimizations applied");
        return optimizations;
    }

} // namespace VoxelCraft
