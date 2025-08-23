#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <queue>
#include <condition_variable>
#include <chrono>
#include <functional>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#endif

namespace VoxelCraft {

    class Packet;
    class PlayerEntity;

    /**
     * @enum ServerState
     * @brief Estados del servidor
     */
    enum class ServerState {
        STOPPED = 0,    ///< Servidor detenido
        STARTING,       ///< Iniciando servidor
        RUNNING,        ///< Servidor ejecutándose
        STOPPING,       ///< Deteniendo servidor
        ERROR           ///< Error del servidor
    };

    /**
     * @struct ClientConnection
     * @brief Información de conexión de cliente
     */
    struct ClientConnection {
        uint32_t connectionId;                    ///< ID único de conexión
        std::string address;                       ///< Dirección IP del cliente
        uint16_t port;                            ///< Puerto del cliente
#ifdef _WIN32
        SOCKET socket;                            ///< Socket de Windows
#else
        int socket;                               ///< Socket POSIX
#endif
        std::chrono::steady_clock::time_point lastActivity; ///< Última actividad
        std::chrono::steady_clock::time_point connectedTime; ///< Tiempo de conexión
        std::chrono::milliseconds ping;           ///< Ping del cliente
        bool authenticated;                       ///< Si está autenticado
        std::string playerName;                   ///< Nombre del jugador
        std::shared_ptr<PlayerEntity> playerEntity; ///< Entidad del jugador

        // Buffers de red
        std::vector<uint8_t> receiveBuffer;
        std::vector<uint8_t> sendBuffer;

        // Estadísticas
        size_t bytesSent;
        size_t bytesReceived;
        size_t packetsSent;
        size_t packetsReceived;

        ClientConnection(uint32_t id = 0, const std::string& addr = "", uint16_t p = 0,
#ifdef _WIN32
                        SOCKET sock = INVALID_SOCKET
#else
                        int sock = -1
#endif
                        );
    };

    /**
     * @struct ServerConfig
     * @brief Configuración del servidor
     */
    struct ServerConfig {
        uint16_t port = 25565;                    ///< Puerto del servidor
        size_t maxConnections = 100;              ///< Máximo de conexiones
        size_t maxPlayers = 20;                   ///< Máximo de jugadores
        std::chrono::milliseconds connectionTimeout = std::chrono::milliseconds(30000);
        std::chrono::milliseconds heartbeatInterval = std::chrono::milliseconds(1000);
        std::chrono::milliseconds tickRate = std::chrono::milliseconds(50); // 20 TPS
        size_t receiveBufferSize = 8192;          ///< Tamaño del buffer de recepción
        size_t sendBufferSize = 8192;             ///< Tamaño del buffer de envío
        bool enableWhitelist = false;             ///< Habilitar whitelist
        bool enableAuthentication = true;         ///< Habilitar autenticación
        std::string serverName = "VoxelCraft Server"; ///< Nombre del servidor
        std::string serverMotd = "A VoxelCraft Server"; ///< MOTD del servidor
        int maxViewDistance = 10;                 ///< Distancia máxima de vista
        bool enableCompression = true;            ///< Habilitar compresión
        int compressionThreshold = 256;           ///< Umbral de compresión
        std::string encryptionKey = "";           ///< Clave de encriptación
    };

    /**
     * @struct ServerStats
     * @brief Estadísticas del servidor
     */
    struct ServerStats {
        size_t totalConnections;                  ///< Total de conexiones
        size_t activeConnections;                 ///< Conexiones activas
        size_t totalPlayers;                      ///< Total de jugadores
        size_t activePlayers;                     ///< Jugadores activos
        std::chrono::milliseconds uptime;         ///< Tiempo de actividad
        std::chrono::milliseconds averagePing;    ///< Ping promedio
        size_t bytesSentPerSecond;                ///< Bytes enviados por segundo
        size_t bytesReceivedPerSecond;            ///< Bytes recibidos por segundo
        size_t packetsSentPerSecond;              ///< Paquetes enviados por segundo
        size_t packetsReceivedPerSecond;          ///< Paquetes recibidos por segundo
        std::chrono::steady_clock::time_point startTime; ///< Tiempo de inicio
    };

    /**
     * @class Server
     * @brief Servidor dedicado para multijugador
     */
    class Server {
    public:
        /**
         * @brief Constructor
         */
        Server();

        /**
         * @brief Destructor
         */
        ~Server();

        /**
         * @brief Inicializar servidor
         * @param config Configuración del servidor
         * @return true si la inicialización fue exitosa
         */
        bool Initialize(const ServerConfig& config);

        /**
         * @brief Iniciar servidor
         * @param port Puerto del servidor
         * @return true si el inicio fue exitoso
         */
        bool Start(uint16_t port);

        /**
         * @brief Detener servidor
         * @return true si la detención fue exitosa
         */
        bool Stop();

        /**
         * @brief Actualizar servidor (llamar cada frame)
         */
        void Update();

        // Estado del servidor
        ServerState GetState() const { return m_state; }
        bool IsRunning() const { return m_state == ServerState::RUNNING; }
        const ServerConfig& GetConfig() const { return m_config; }
        const ServerStats& GetStats() const { return m_stats; }

        // Gestión de conexiones
        bool IsConnectionActive(uint32_t connectionId) const;
        size_t GetActiveConnectionCount() const;
        std::vector<uint32_t> GetActiveConnections() const;
        bool DisconnectConnection(uint32_t connectionId);
        void DisconnectAll();

        // Comunicación
        bool SendPacket(std::shared_ptr<Packet> packet);
        bool SendPacketTo(std::shared_ptr<Packet> packet, uint32_t connectionId);
        bool BroadcastPacket(std::shared_ptr<Packet> packet);
        bool BroadcastPacketExcept(std::shared_ptr<Packet> packet, uint32_t excludeConnectionId);

        // Ping y latencia
        std::chrono::milliseconds GetPing(uint32_t connectionId) const;
        std::chrono::milliseconds GetAverageLatency() const;
        bool SendPing(uint32_t connectionId);
        bool SendPong(uint32_t connectionId);

        // Callbacks de eventos
        using ConnectionCallback = std::function<void(uint32_t connectionId, bool connected)>;
        using PacketCallback = std::function<void(std::shared_ptr<Packet> packet, uint32_t connectionId)>;
        using PlayerCallback = std::function<void(uint32_t connectionId, const std::string& playerName, bool joined)>;

        void SetConnectionCallback(const ConnectionCallback& callback) { m_connectionCallback = callback; }
        void SetPacketCallback(const PacketCallback& callback) { m_packetCallback = callback; }
        void SetPlayerCallback(const PlayerCallback& callback) { m_playerCallback = callback; }

        // Gestión de jugadores
        std::shared_ptr<PlayerEntity> GetPlayerEntity(uint32_t connectionId) const;
        std::string GetPlayerName(uint32_t connectionId) const;
        std::vector<std::string> GetOnlinePlayers() const;
        size_t GetPlayerCount() const;
        bool KickPlayer(uint32_t connectionId, const std::string& reason);
        bool BanPlayer(uint32_t connectionId, const std::string& reason);
        bool BanIP(const std::string& ip, const std::string& reason);

        // Whitelist
        bool AddToWhitelist(const std::string& playerName);
        bool RemoveFromWhitelist(const std::string& playerName);
        bool IsWhitelisted(const std::string& playerName) const;
        std::vector<std::string> GetWhitelist() const;

        // Debug y diagnóstico
        void EnableDebugMode(bool enable) { m_debugMode = enable; }
        bool IsDebugModeEnabled() const { return m_debugMode; }
        std::string GetDebugInfo() const;

    private:
        ServerConfig m_config;
        ServerStats m_stats;
        std::atomic<ServerState> m_state;

#ifdef _WIN32
        SOCKET m_serverSocket;
        WSADATA m_wsaData;
#else
        int m_serverSocket;
#endif

        // Gestión de conexiones
        std::unordered_map<uint32_t, std::unique_ptr<ClientConnection>> m_connections;
        std::unordered_map<std::string, uint32_t> m_playerConnections; // playerName -> connectionId
        std::unordered_map<std::string, uint32_t> m_ipConnections; // ip -> connectionId
        mutable std::shared_mutex m_connectionsMutex;
        std::atomic<uint32_t> m_nextConnectionId;

        // Listas de control
        std::unordered_set<std::string> m_whitelist;
        std::unordered_set<std::string> m_bannedIPs;
        std::unordered_set<std::string> m_bannedPlayers;
        mutable std::shared_mutex m_listsMutex;

        // Hilos y sincronización
        std::thread m_acceptThread;
        std::thread m_updateThread;
        std::atomic<bool> m_running;
        std::mutex m_serverMutex;
        std::condition_variable m_serverCV;

        // Colas de procesamiento
        std::queue<std::pair<std::shared_ptr<Packet>, uint32_t>> m_packetQueue;
        std::queue<uint32_t> m_disconnectQueue;
        std::mutex m_packetQueueMutex;
        std::mutex m_disconnectQueueMutex;
        std::condition_variable m_packetQueueCV;
        std::condition_variable m_disconnectQueueCV;

        // Callbacks
        ConnectionCallback m_connectionCallback;
        PacketCallback m_packetCallback;
        PlayerCallback m_playerCallback;

        // Debug
        bool m_debugMode;

        // Métodos privados
        bool InitializeNetwork();
        void ShutdownNetwork();
        bool CreateSocket();
        bool BindSocket(uint16_t port);
        bool ListenSocket();
        void AcceptThread();
        void UpdateThread();
        void ProcessNetworkEvents();
        void ProcessPacketQueue();
        void ProcessDisconnectQueue();
        void HandleNewConnection();
        void HandleClientData(uint32_t connectionId);
        void HandleClientDisconnect(uint32_t connectionId);
        bool SendDataToClient(uint32_t connectionId, const std::vector<uint8_t>& data);
        bool ReceiveDataFromClient(uint32_t connectionId);
        void UpdateStats();
        bool ValidateConnection(uint32_t connectionId);
        void LogServerEvent(const std::string& event, uint32_t connectionId = 0);
        bool AuthenticateClient(uint32_t connectionId, const std::string& playerName);
        void HandleTimeoutConnections();
        void SendHeartbeat();
    };

} // namespace VoxelCraft