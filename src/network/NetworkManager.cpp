#include "NetworkManager.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "GameStateSync.hpp"
#include "EncryptionManager.hpp"
#include "AuthenticationManager.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <chrono>

namespace VoxelCraft {

    NetworkManager& NetworkManager::GetInstance() {
        static NetworkManager instance;
        return instance;
    }

    NetworkManager::~NetworkManager() {
        Shutdown();
    }

    bool NetworkManager::Initialize(const NetworkConfig& config) {
        std::lock_guard<std::mutex> lock(m_networkMutex);

        if (m_running) {
            Shutdown();
        }

        m_config = config;
        m_connectionState = ConnectionState::DISCONNECTED;
        m_running = true;
        m_stats = NetworkStats();
        m_stats.lastUpdate = std::chrono::steady_clock::now();

        // Inicializar componentes según el modo
        try {
            if (m_config.mode == NetworkMode::SERVER || m_config.mode == NetworkMode::HOST) {
                m_server = std::make_unique<Server>();
                if (!m_server->Initialize(m_config)) {
                    std::cerr << "Failed to initialize server" << std::endl;
                    return false;
                }
            }

            if (m_config.mode == NetworkMode::CLIENT || m_config.mode == NetworkMode::HOST) {
                m_client = std::make_unique<Client>();
                if (!m_client->Initialize(m_config)) {
                    std::cerr << "Failed to initialize client" << std::endl;
                    return false;
                }
            }

            // Inicializar componentes comunes
            m_gameStateSync = std::make_unique<GameStateSync>();

            if (m_config.enableEncryption) {
                m_encryptionManager = std::make_unique<EncryptionManager>();
            }

            if (m_config.enableAuthentication) {
                m_authManager = std::make_unique<AuthenticationManager>();
            }

            // Iniciar hilo de red
            m_networkThread = std::thread(&NetworkManager::NetworkThreadFunc, this);

            LogNetworkEvent("NetworkManager initialized successfully");
            return true;

        } catch (const std::exception& e) {
            std::cerr << "Exception during NetworkManager initialization: " << e.what() << std::endl;
            return false;
        }
    }

    void NetworkManager::Shutdown() {
        std::lock_guard<std::mutex> lock(m_networkMutex);

        if (!m_running) {
            return;
        }

        m_running = false;
        m_networkCV.notify_all();

        // Detener hilo de red
        if (m_networkThread.joinable()) {
            m_networkThread.join();
        }

        // Desconectar y limpiar componentes
        if (m_server) {
            m_server->Shutdown();
            m_server.reset();
        }

        if (m_client) {
            m_client->Disconnect();
            m_client.reset();
        }

        m_gameStateSync.reset();
        m_encryptionManager.reset();
        m_authManager.reset();

        // Limpiar colas
        {
            std::lock_guard<std::mutex> sendLock(m_sendQueueMutex);
            std::queue<std::shared_ptr<Packet>>().swap(m_sendQueue);
        }

        {
            std::lock_guard<std::mutex> receiveLock(m_receiveQueueMutex);
            std::queue<std::pair<std::shared_ptr<Packet>, uint32_t>>().swap(m_receiveQueue);
        }

        m_connectionState = ConnectionState::DISCONNECTED;
        LogNetworkEvent("NetworkManager shutdown complete");
    }

    void NetworkManager::Update() {
        if (!m_running) {
            return;
        }

        // Procesar paquetes recibidos
        ProcessReceiveQueue();

        // Actualizar estadísticas
        UpdateStats();

        // Manejar reconexión si es necesario
        if (m_connectionState == ConnectionState::RECONNECTING) {
            HandleReconnection();
        }

        // Actualizar componentes
        if (m_server) {
            m_server->Update();
        }

        if (m_client) {
            m_client->Update();
        }

        if (m_gameStateSync) {
            m_gameStateSync->Update();
        }
    }

    bool NetworkManager::Connect(const std::string& address, uint16_t port) {
        if (!m_client || !m_running) {
            return false;
        }

        std::lock_guard<std::mutex> lock(m_networkMutex);

        m_connectionState = ConnectionState::CONNECTING;
        LogNetworkEvent("Attempting to connect to " + address + ":" + std::to_string(port));

        bool result = m_client->Connect(address, port);
        if (result) {
            m_connectionState = ConnectionState::AUTHENTICATING;
        } else {
            m_connectionState = ConnectionState::ERROR;
        }

        return result;
    }

    bool NetworkManager::Disconnect() {
        std::lock_guard<std::mutex> lock(m_networkMutex);

        bool clientDisconnected = true;
        bool serverStopped = true;

        if (m_client) {
            clientDisconnected = m_client->Disconnect();
        }

        if (m_server) {
            serverStopped = m_server->Stop();
        }

        if (clientDisconnected && serverStopped) {
            m_connectionState = ConnectionState::DISCONNECTED;
            LogNetworkEvent("Disconnected successfully");
            return true;
        }

        return false;
    }

    bool NetworkManager::StartServer(uint16_t port) {
        if (!m_server || !m_running) {
            return false;
        }

        std::lock_guard<std::mutex> lock(m_networkMutex);

        LogNetworkEvent("Starting server on port " + std::to_string(port));
        bool result = m_server->Start(port);

        if (result) {
            m_connectionState = ConnectionState::CONNECTED;
        } else {
            m_connectionState = ConnectionState::ERROR;
        }

        return result;
    }

    bool NetworkManager::StopServer() {
        if (!m_server) {
            return true;
        }

        std::lock_guard<std::mutex> lock(m_networkMutex);

        LogNetworkEvent("Stopping server");
        bool result = m_server->Stop();

        if (result) {
            m_connectionState = ConnectionState::DISCONNECTED;
        }

        return result;
    }

    bool NetworkManager::SendPacket(std::shared_ptr<Packet> packet) {
        if (!m_running || !packet) {
            return false;
        }

        std::lock_guard<std::mutex> lock(m_sendQueueMutex);
        m_sendQueue.push(packet);
        m_sendQueueCV.notify_one();

        return true;
    }

    bool NetworkManager::SendPacketTo(std::shared_ptr<Packet> packet, uint32_t connectionId) {
        if (!m_running || !packet || !ValidateConnection(connectionId)) {
            return false;
        }

        std::lock_guard<std::mutex> lock(m_sendQueueMutex);
        m_sendQueue.push(packet);
        m_sendQueueCV.notify_one();

        return true;
    }

    bool NetworkManager::BroadcastPacket(std::shared_ptr<Packet> packet) {
        if (!m_running || !packet) {
            return false;
        }

        std::lock_guard<std::mutex> lock(m_sendQueueMutex);
        m_sendQueue.push(packet);
        m_sendQueueCV.notify_one();

        return true;
    }

    size_t NetworkManager::GetActiveConnectionCount() const {
        if (m_server) {
            return m_server->GetActiveConnectionCount();
        }
        return m_client && m_client->IsConnected() ? 1 : 0;
    }

    std::vector<uint32_t> NetworkManager::GetActiveConnections() const {
        if (m_server) {
            return m_server->GetActiveConnections();
        }
        if (m_client && m_client->IsConnected()) {
            return {0}; // Connection ID 0 for client
        }
        return {};
    }

    bool NetworkManager::IsConnectionActive(uint32_t connectionId) const {
        if (m_server) {
            return m_server->IsConnectionActive(connectionId);
        }
        return m_client && m_client->IsConnected() && connectionId == 0;
    }

    void NetworkManager::DisconnectConnection(uint32_t connectionId) {
        if (m_server) {
            m_server->DisconnectConnection(connectionId);
        } else if (m_client && connectionId == 0) {
            m_client->Disconnect();
        }
    }

    std::chrono::milliseconds NetworkManager::GetPing(uint32_t connectionId) const {
        if (m_server) {
            return m_server->GetPing(connectionId);
        }
        if (m_client && connectionId == 0) {
            return m_client->GetPing();
        }
        return std::chrono::milliseconds(0);
    }

    std::chrono::milliseconds NetworkManager::GetAverageLatency() const {
        if (m_server) {
            return m_server->GetAverageLatency();
        }
        if (m_client) {
            return m_client->GetLatency();
        }
        return std::chrono::milliseconds(0);
    }

    bool NetworkManager::SendPing(uint32_t connectionId) {
        if (m_server) {
            return m_server->SendPing(connectionId);
        }
        if (m_client && connectionId == 0) {
            return m_client->SendPing();
        }
        return false;
    }

    bool NetworkManager::SendPong(uint32_t connectionId) {
        if (m_server) {
            return m_server->SendPong(connectionId);
        }
        if (m_client && connectionId == 0) {
            return m_client->SendPong();
        }
        return false;
    }

    bool NetworkManager::EnableEncryption(bool enable) {
        if (enable && !m_encryptionManager) {
            m_encryptionManager = std::make_unique<EncryptionManager>();
        }

        m_config.enableEncryption = enable;
        return true;
    }

    bool NetworkManager::EnableAuthentication(bool enable) {
        if (enable && !m_authManager) {
            m_authManager = std::make_unique<AuthenticationManager>();
        }

        m_config.enableAuthentication = enable;
        return true;
    }

    bool NetworkManager::EnableCompression(bool enable) {
        m_config.enableCompression = enable;
        return true;
    }

    bool NetworkManager::SetEncryptionKey(const std::string& key) {
        if (m_encryptionManager) {
            return m_encryptionManager->SetEncryptionKey(key);
        }
        return false;
    }

    bool NetworkManager::AuthenticateConnection(uint32_t connectionId, const std::string& token) {
        if (m_authManager) {
            return m_authManager->AuthenticateConnection(connectionId, token);
        }
        return false;
    }

    bool NetworkManager::SetCompressionLevel(int level) {
        if (level < 0 || level > 9) {
            return false;
        }
        // Implement compression level setting
        return true;
    }

    void NetworkManager::ResetStats() {
        m_stats = NetworkStats();
        m_stats.lastUpdate = std::chrono::steady_clock::now();
    }

    std::string NetworkManager::GetNetworkInfo() const {
        std::stringstream ss;
        ss << "NetworkManager Info:\n";
        ss << "  Mode: " << static_cast<int>(m_config.mode) << "\n";
        ss << "  Protocol: " << static_cast<int>(m_config.protocol) << "\n";
        ss << "  State: " << static_cast<int>(m_connectionState) << "\n";
        ss << "  Server: " << (m_server ? "Active" : "Inactive") << "\n";
        ss << "  Client: " << (m_client ? "Active" : "Inactive") << "\n";
        ss << "  Active Connections: " << GetActiveConnectionCount() << "\n";
        ss << "  Encryption: " << (m_config.enableEncryption ? "Enabled" : "Disabled") << "\n";
        ss << "  Authentication: " << (m_config.enableAuthentication ? "Enabled" : "Disabled") << "\n";
        ss << "  Compression: " << (m_config.enableCompression ? "Enabled" : "Disabled") << "\n";
        return ss.str();
    }

    void NetworkManager::NetworkThreadFunc() {
        while (m_running) {
            try {
                // Procesar cola de envío
                ProcessSendQueue();

                // Esperar por trabajo o timeout
                std::unique_lock<std::mutex> lock(m_networkMutex);
                m_networkCV.wait_for(lock, std::chrono::milliseconds(100));

            } catch (const std::exception& e) {
                std::cerr << "Exception in network thread: " << e.what() << std::endl;
                if (m_errorCallback) {
                    m_errorCallback("Network thread exception: " + std::string(e.what()), 0);
                }
            }
        }
    }

    void NetworkManager::ProcessSendQueue() {
        std::unique_lock<std::mutex> lock(m_sendQueueMutex);

        while (!m_sendQueue.empty()) {
            auto packet = m_sendQueue.front();
            m_sendQueue.pop();
            lock.unlock();

            try {
                // Enviar paquete según el modo
                bool sent = false;

                if (m_server && m_config.mode == NetworkMode::SERVER) {
                    sent = m_server->SendPacket(packet);
                } else if (m_client && m_config.mode == NetworkMode::CLIENT) {
                    sent = m_client->SendPacket(packet);
                } else if (m_config.mode == NetworkMode::HOST) {
                    // En modo host, enviar tanto al servidor como al cliente
                    if (m_server) {
                        sent = m_server->SendPacket(packet) || sent;
                    }
                    if (m_client) {
                        sent = m_client->SendPacket(packet) || sent;
                    }
                }

                if (sent) {
                    m_stats.packetsSent++;
                    m_stats.bytesSent += packet->GetSize();
                } else {
                    m_stats.packetsLost++;
                }

            } catch (const std::exception& e) {
                std::cerr << "Exception sending packet: " << e.what() << std::endl;
                m_stats.packetsLost++;
            }

            lock.lock();
        }
    }

    void NetworkManager::ProcessReceiveQueue() {
        std::unique_lock<std::mutex> lock(m_receiveQueueMutex);

        while (!m_receiveQueue.empty()) {
            auto [packet, connectionId] = m_receiveQueue.front();
            m_receiveQueue.pop();
            lock.unlock();

            try {
                m_stats.packetsReceived++;
                m_stats.bytesReceived += packet->GetSize();

                // Llamar callback de paquete
                if (m_packetCallback) {
                    m_packetCallback(packet, connectionId);
                }

            } catch (const std::exception& e) {
                std::cerr << "Exception processing received packet: " << e.what() << std::endl;
            }

            lock.lock();
        }
    }

    void NetworkManager::UpdateStats() {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_stats.lastUpdate);

        if (elapsed.count() >= 1000) { // Update every second
            m_stats.packetLossRate = m_stats.packetsLost / static_cast<float>(m_stats.packetsSent + m_stats.packetsLost);
            m_stats.activeConnections = GetActiveConnectionCount();
            m_stats.lastUpdate = now;
        }
    }

    void NetworkManager::HandleReconnection() {
        static int reconnectionAttempts = 0;
        static auto lastAttempt = std::chrono::steady_clock::now();

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastAttempt);

        if (elapsed >= m_config.reconnectionDelay && reconnectionAttempts < m_config.maxReconnectionAttempts) {
            LogNetworkEvent("Attempting reconnection " + std::to_string(reconnectionAttempts + 1));
            reconnectionAttempts++;
            lastAttempt = now;

            // Intentar reconectar
            if (m_client) {
                if (m_client->Connect(m_config.serverAddress, m_config.serverPort)) {
                    m_connectionState = ConnectionState::AUTHENTICATING;
                    reconnectionAttempts = 0;
                }
            }
        } else if (reconnectionAttempts >= m_config.maxReconnectionAttempts) {
            m_connectionState = ConnectionState::ERROR;
            LogNetworkEvent("Max reconnection attempts reached");
            reconnectionAttempts = 0;
        }
    }

    bool NetworkManager::ValidateConnection(uint32_t connectionId) {
        return IsConnectionActive(connectionId);
    }

    void NetworkManager::LogNetworkEvent(const std::string& event, uint32_t connectionId) {
        if (m_debugMode) {
            std::cout << "[Network] " << event;
            if (connectionId != 0) {
                std::cout << " (Connection: " << connectionId << ")";
            }
            std::cout << std::endl;
        }
    }

} // namespace VoxelCraft
