#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <mutex>
#include <atomic>
#include <thread>
#include <queue>
#include <condition_variable>
#include <chrono>

namespace VoxelCraft {

    class Server;
    class Client;
    class Packet;
    class GameStateSync;
    class EncryptionManager;
    class AuthenticationManager;

    /**
     * @enum NetworkMode
     * @brief Modos de operación de red
     */
    enum class NetworkMode {
        NONE = 0,           ///< Sin conexión de red
        CLIENT,             ///< Modo cliente
        SERVER,             ///< Modo servidor
        HOST                ///< Modo host (cliente + servidor)
    };

    /**
     * @enum ConnectionState
     * @brief Estados de conexión de red
     */
    enum class ConnectionState {
        DISCONNECTED = 0,   ///< Desconectado
        CONNECTING,         ///< Conectando
        AUTHENTICATING,    ///< Autenticando
        CONNECTED,          ///< Conectado
        RECONNECTING,       ///< Reconectando
        ERROR               ///< Error de conexión
    };

    /**
     * @enum NetworkProtocol
     * @brief Protocolos de red soportados
     */
    enum class NetworkProtocol {
        TCP = 0,            ///< TCP para comunicación confiable
        UDP,                ///< UDP para comunicación rápida
        WEBSOCKET,          ///< WebSocket para comunicación web
        QUIC                ///< QUIC para comunicación moderna
    };

    /**
     * @struct NetworkConfig
     * @brief Configuración de red
     */
    struct NetworkConfig {
        NetworkMode mode = NetworkMode::NONE;
        NetworkProtocol protocol = NetworkProtocol::TCP;
        std::string serverAddress = "127.0.0.1";
        uint16_t serverPort = 25565; // Puerto por defecto de Minecraft
        uint16_t localPort = 0; // Puerto local (0 = automático)
        size_t maxConnections = 100; // Máximo de conexiones para servidor
        size_t maxPacketSize = 2097152; // 2MB máximo por paquete
        size_t sendBufferSize = 1048576; // 1MB buffer de envío
        size_t receiveBufferSize = 1048576; // 1MB buffer de recepción
        std::chrono::milliseconds connectionTimeout = std::chrono::milliseconds(5000);
        std::chrono::milliseconds heartbeatInterval = std::chrono::milliseconds(30000);
        bool enableEncryption = true;
        bool enableCompression = true;
        bool enableAuthentication = true;
        int maxReconnectionAttempts = 3;
        std::chrono::milliseconds reconnectionDelay = std::chrono::milliseconds(1000);
    };

    /**
     * @struct NetworkStats
     * @brief Estadísticas de red
     */
    struct NetworkStats {
        size_t bytesSent = 0;
        size_t bytesReceived = 0;
        size_t packetsSent = 0;
        size_t packetsReceived = 0;
        size_t packetsLost = 0;
        std::chrono::milliseconds ping = std::chrono::milliseconds(0);
        std::chrono::milliseconds latency = std::chrono::milliseconds(0);
        float packetLossRate = 0.0f;
        size_t activeConnections = 0;
        size_t totalConnections = 0;
        std::chrono::steady_clock::time_point lastUpdate;
    };

    /**
     * @class NetworkManager
     * @brief Gestor central de toda la comunicación de red
     */
    class NetworkManager {
    public:
        static NetworkManager& GetInstance();

        /**
         * @brief Inicializar el sistema de red
         * @param config Configuración de red
         * @return true si la inicialización fue exitosa
         */
        bool Initialize(const NetworkConfig& config);

        /**
         * @brief Apagar el sistema de red
         */
        void Shutdown();

        /**
         * @brief Actualizar el estado de red (llamar cada frame)
         */
        void Update();

        // Configuración
        void SetConfig(const NetworkConfig& config) { m_config = config; }
        const NetworkConfig& GetConfig() const { return m_config; }

        // Estado de conexión
        ConnectionState GetConnectionState() const { return m_connectionState; }
        NetworkMode GetNetworkMode() const { return m_config.mode; }
        bool IsConnected() const { return m_connectionState == ConnectionState::CONNECTED; }
        bool IsServer() const { return m_config.mode == NetworkMode::SERVER || m_config.mode == NetworkMode::HOST; }
        bool IsClient() const { return m_config.mode == NetworkMode::CLIENT || m_config.mode == NetworkMode::HOST; }

        // Conexión
        bool Connect(const std::string& address, uint16_t port);
        bool Disconnect();
        bool StartServer(uint16_t port);
        bool StopServer();

        // Comunicación
        bool SendPacket(std::shared_ptr<Packet> packet);
        bool SendPacketTo(std::shared_ptr<Packet> packet, uint32_t connectionId);
        bool BroadcastPacket(std::shared_ptr<Packet> packet);

        // Callbacks de eventos
        using ConnectionCallback = std::function<void(uint32_t connectionId, bool connected)>;
        using PacketCallback = std::function<void(std::shared_ptr<Packet> packet, uint32_t connectionId)>;
        using ErrorCallback = std::function<void(const std::string& error, uint32_t connectionId)>;

        void SetConnectionCallback(const ConnectionCallback& callback) { m_connectionCallback = callback; }
        void SetPacketCallback(const PacketCallback& callback) { m_packetCallback = callback; }
        void SetErrorCallback(const ErrorCallback& callback) { m_errorCallback = callback; }

        // Estadísticas
        const NetworkStats& GetNetworkStats() const { return m_stats; }
        void ResetStats();

        // Gestión de conexiones
        size_t GetActiveConnectionCount() const;
        std::vector<uint32_t> GetActiveConnections() const;
        bool IsConnectionActive(uint32_t connectionId) const;
        void DisconnectConnection(uint32_t connectionId);

        // Debug y diagnóstico
        std::string GetNetworkInfo() const;
        void EnableDebugMode(bool enable) { m_debugMode = enable; }
        bool IsDebugModeEnabled() const { return m_debugMode; }

        // Seguridad
        bool EnableEncryption(bool enable);
        bool EnableAuthentication(bool enable);
        bool SetEncryptionKey(const std::string& key);
        bool AuthenticateConnection(uint32_t connectionId, const std::string& token);

        // Compresión
        bool EnableCompression(bool enable);
        bool SetCompressionLevel(int level);

        // Latencia y ping
        std::chrono::milliseconds GetPing(uint32_t connectionId = 0) const;
        std::chrono::milliseconds GetAverageLatency() const;
        bool SendPing(uint32_t connectionId = 0);
        bool SendPong(uint32_t connectionId = 0);

    private:
        NetworkManager() = default;
        ~NetworkManager();

        // Prevent copying
        NetworkManager(const NetworkManager&) = delete;
        NetworkManager& operator=(const NetworkManager&) = delete;

        // Componentes del sistema de red
        std::unique_ptr<Server> m_server;
        std::unique_ptr<Client> m_client;
        std::unique_ptr<GameStateSync> m_gameStateSync;
        std::unique_ptr<EncryptionManager> m_encryptionManager;
        std::unique_ptr<AuthenticationManager> m_authManager;

        // Configuración y estado
        NetworkConfig m_config;
        std::atomic<ConnectionState> m_connectionState;
        NetworkStats m_stats;

        // Hilos y sincronización
        std::thread m_networkThread;
        std::atomic<bool> m_running;
        std::mutex m_networkMutex;
        std::condition_variable m_networkCV;

        // Colas de mensajes
        std::queue<std::shared_ptr<Packet>> m_sendQueue;
        std::queue<std::pair<std::shared_ptr<Packet>, uint32_t>> m_receiveQueue;
        std::mutex m_sendQueueMutex;
        std::mutex m_receiveQueueMutex;
        std::condition_variable m_sendQueueCV;
        std::condition_variable m_receiveQueueCV;

        // Callbacks
        ConnectionCallback m_connectionCallback;
        PacketCallback m_packetCallback;
        ErrorCallback m_errorCallback;

        // Debug
        bool m_debugMode = false;

        // Métodos privados
        void NetworkThreadFunc();
        void ProcessSendQueue();
        void ProcessReceiveQueue();
        void UpdateStats();
        void HandleReconnection();
        bool ValidateConnection(uint32_t connectionId);
        void LogNetworkEvent(const std::string& event, uint32_t connectionId = 0);
    };

} // namespace VoxelCraft
