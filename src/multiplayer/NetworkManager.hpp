/**
 * @file NetworkManager.hpp
 * @brief VoxelCraft Network Manager - Main Networking Controller
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the NetworkManager class that provides comprehensive networking
 * functionality for the VoxelCraft game engine, including client-server architecture,
 * state synchronization, latency management, and secure authentication systems.
 */

#ifndef VOXELCRAFT_MULTIPLAYER_NETWORK_MANAGER_HPP
#define VOXELCRAFT_MULTIPLAYER_NETWORK_MANAGER_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <chrono>
#include <queue>
#include <optional>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"
#include "../core/System.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Server;
    class Client;
    class NetworkProtocol;
    class NetworkSecurity;
    class NetworkSynchronization;
    class NetworkLatencyManager;
    class Player;
    class World;
    class EntityManager;

    /**
     * @enum NetworkMode
     * @brief Network operation modes
     */
    enum class NetworkMode {
        Offline,                 ///< Single player mode
        Client,                  ///< Client mode
        Server,                  ///< Server mode
        DedicatedServer,         ///< Dedicated server mode
        ListenServer             ///< Listen server (client + server)
    };

    /**
     * @enum NetworkState
     * @brief Network system states
     */
    enum class NetworkState {
        Inactive,                ///< Network system inactive
        Starting,                ///< Network system starting
        Active,                  ///< Network system active
        Stopping,                ///< Network system stopping
        Error                    ///< Network system in error state
    };

    /**
     * @enum ConnectionState
     * @brief Client connection states
     */
    enum class ConnectionState {
        Disconnected,            ///< Not connected
        Connecting,              ///< Attempting connection
        Connected,               ///< Connected to server
        Disconnecting,           ///< Disconnecting from server
        Reconnecting,            ///< Attempting reconnection
        ConnectionLost           ///< Connection lost
    };

    /**
     * @enum NetworkMessageType
     * @brief Types of network messages
     */
    enum class NetworkMessageType {
        // Connection messages
        ConnectionRequest,       ///< Client requesting connection
        ConnectionAccept,        ///< Server accepting connection
        ConnectionReject,        ///< Server rejecting connection
        ConnectionClose,         ///< Connection closed
        Heartbeat,               ///< Keep-alive heartbeat

        // Authentication messages
        AuthenticationRequest,   ///< Authentication request
        AuthenticationResponse,  ///< Authentication response
        AuthenticationSuccess,   ///< Authentication successful
        AuthenticationFailure,   ///< Authentication failed

        // Game state messages
        PlayerJoin,              ///< Player joining the game
        PlayerLeave,             ///< Player leaving the game
        PlayerUpdate,            ///< Player state update
        EntityCreate,            ///< Entity creation
        EntityUpdate,            ///< Entity state update
        EntityDestroy,           ///< Entity destruction
        WorldUpdate,             ///< World state update
        ChatMessage,             ///< Chat message
        Command,                 ///< Console command

        // Synchronization messages
        StateSynchronization,    ///< State synchronization
        TimeSynchronization,     ///< Time synchronization
        LatencyUpdate,           ///< Latency measurement update

        // Error messages
        Error,                   ///< General error message
        Warning,                 ///< Warning message

        // Custom messages
        Custom                   ///< Custom message type
    };

    /**
     * @enum NetworkChannel
     * @brief Network communication channels
     */
    enum class NetworkChannel {
        ReliableOrdered,         ///< Reliable ordered delivery
        ReliableUnordered,       ///< Reliable unordered delivery
        UnreliableOrdered,       ///< Unreliable ordered delivery
        UnreliableUnordered      ///< Unreliable unordered delivery
    };

    /**
     * @struct NetworkMessage
     * @brief Network message structure
     */
    struct NetworkMessage {
        uint32_t messageId;                      ///< Unique message ID
        NetworkMessageType type;                 ///< Message type
        uint32_t senderId;                       ///< Sender ID
        uint32_t receiverId;                     ///< Receiver ID (0 for broadcast)
        double timestamp;                        ///< Message timestamp
        uint32_t sequenceNumber;                 ///< Sequence number for ordering
        NetworkChannel channel;                  ///< Communication channel
        std::vector<uint8_t> data;              ///< Message data
        bool requiresAck;                       ///< Requires acknowledgment
        std::unordered_map<std::string, std::any> metadata; ///< Additional metadata

        NetworkMessage()
            : messageId(0)
            , type(NetworkMessageType::Custom)
            , senderId(0)
            , receiverId(0)
            , timestamp(0.0)
            , sequenceNumber(0)
            , channel(NetworkChannel::ReliableOrdered)
            , requiresAck(true)
        {}
    };

    /**
     * @struct NetworkPeer
     * @brief Network peer information
     */
    struct NetworkPeer {
        uint32_t peerId;                        ///< Unique peer ID
        std::string address;                     ///< Network address
        uint16_t port;                          ///< Network port
        ConnectionState state;                  ///< Connection state
        double lastActivity;                    ///< Last activity timestamp
        double connectionTime;                  ///< Connection established time
        uint32_t ping;                          ///< Current ping (ms)
        uint32_t packetLoss;                    ///< Packet loss percentage
        std::string playerName;                 ///< Associated player name
        std::unordered_map<std::string, std::any> properties; ///< Custom properties

        NetworkPeer()
            : peerId(0)
            , port(0)
            , state(ConnectionState::Disconnected)
            , lastActivity(0.0)
            , connectionTime(0.0)
            , ping(0)
            , packetLoss(0)
        {}
    };

    /**
     * @struct NetworkManagerConfig
     * @brief Network manager configuration
     */
    struct NetworkManagerConfig {
        // Basic settings
        NetworkMode mode;                       ///< Network operation mode
        std::string serverAddress;              ///< Server address to connect to
        uint16_t serverPort;                    ///< Server port
        uint16_t localPort;                     ///< Local port for listening
        bool enableUPnP;                        ///< Enable UPnP for port forwarding
        bool enableIPv6;                        ///< Enable IPv6 support

        // Performance settings
        uint32_t maxConnections;                ///< Maximum number of connections
        uint32_t maxPlayers;                    ///< Maximum number of players
        float updateInterval;                   ///< Network update interval (seconds)
        uint32_t maxPacketSize;                 ///< Maximum packet size (bytes)
        uint32_t sendRate;                      ///< Send rate (packets per second)
        uint32_t receiveRate;                   ///< Receive rate (packets per second)

        // Timeout settings
        float connectionTimeout;                ///< Connection timeout (seconds)
        float heartbeatInterval;                ///< Heartbeat interval (seconds)
        float reconnectionDelay;                ///< Reconnection delay (seconds)
        uint32_t maxReconnectionAttempts;       ///< Maximum reconnection attempts

        // Security settings
        bool enableEncryption;                  ///< Enable encryption
        bool enableCompression;                 ///< Enable compression
        std::string encryptionKey;              ///< Encryption key
        uint32_t compressionLevel;              ///< Compression level (0-9)

        // Synchronization settings
        float interpolationDelay;               ///< Interpolation delay (seconds)
        float extrapolationLimit;               ///< Extrapolation limit (seconds)
        bool enableClientSidePrediction;        ///< Enable client-side prediction
        bool enableServerReconciliation;        ///< Enable server reconciliation

        // Debug settings
        bool enableDebugLogging;                ///< Enable debug logging
        bool enableNetworkStats;                ///< Enable network statistics
        bool enablePacketLogging;               ///< Enable packet logging
        std::string logFile;                    ///< Network log file

        // Advanced settings
        uint32_t threadPoolSize;                ///< Network thread pool size
        uint32_t sendBufferSize;                ///< Send buffer size (bytes)
        uint32_t receiveBufferSize;             ///< Receive buffer size (bytes)
        bool enableBandwidthThrottling;         ///< Enable bandwidth throttling
        uint32_t maxBandwidthUp;                ///< Max upload bandwidth (KB/s)
        uint32_t maxBandwidthDown;              ///< Max download bandwidth (KB/s)

        NetworkManagerConfig()
            : mode(NetworkMode::Offline)
            , serverPort(25565)
            , localPort(0)
            , enableUPnP(false)
            , enableIPv6(false)
            , maxConnections(16)
            , maxPlayers(10)
            , updateInterval(1.0f / 30.0f)
            , maxPacketSize(4096)
            , sendRate(30)
            , receiveRate(30)
            , connectionTimeout(30.0f)
            , heartbeatInterval(10.0f)
            , reconnectionDelay(5.0f)
            , maxReconnectionAttempts(5)
            , enableEncryption(false)
            , enableCompression(true)
            , compressionLevel(6)
            , interpolationDelay(0.1f)
            , extrapolationLimit(0.5f)
            , enableClientSidePrediction(true)
            , enableServerReconciliation(true)
            , enableDebugLogging(false)
            , enableNetworkStats(false)
            , enablePacketLogging(false)
            , threadPoolSize(4)
            , sendBufferSize(8192)
            , receiveBufferSize(8192)
            , enableBandwidthThrottling(false)
            , maxBandwidthUp(1024)
            , maxBandwidthDown(1024)
        {}
    };

    /**
     * @struct NetworkManagerMetrics
     * @brief Performance metrics for network manager
     */
    struct NetworkManagerMetrics {
        // Performance metrics
        uint64_t updateCount;                  ///< Number of updates performed
        double totalUpdateTime;                ///< Total update time (ms)
        double averageUpdateTime;              ///< Average update time (ms)
        double maxUpdateTime;                  ///< Maximum update time (ms)

        // Connection metrics
        uint32_t activeConnections;            ///< Number of active connections
        uint32_t pendingConnections;           ///< Number of pending connections
        uint32_t totalConnections;             ///< Total connections established
        uint32_t failedConnections;            ///< Number of failed connections

        // Message metrics
        uint64_t messagesSent;                 ///< Total messages sent
        uint64_t messagesReceived;             ///< Total messages received
        uint64_t bytesSent;                    ///< Total bytes sent
        uint64_t bytesReceived;                ///< Total bytes received

        // Latency metrics
        uint32_t averagePing;                  ///< Average ping (ms)
        uint32_t minPing;                      ///< Minimum ping (ms)
        uint32_t maxPing;                      ///< Maximum ping (ms)
        uint32_t packetLoss;                   ///< Packet loss percentage

        // Bandwidth metrics
        uint32_t currentBandwidthUp;           ///< Current upload bandwidth (KB/s)
        uint32_t currentBandwidthDown;         ///< Current download bandwidth (KB/s)
        uint32_t peakBandwidthUp;              ///< Peak upload bandwidth (KB/s)
        uint32_t peakBandwidthDown;            ///< Peak download bandwidth (KB/s)

        // Error metrics
        uint32_t connectionErrors;             ///< Connection errors
        uint32_t timeoutErrors;                ///< Timeout errors
        uint32_t protocolErrors;               ///< Protocol errors
        uint32_t securityErrors;               ///< Security errors

        // Synchronization metrics
        uint32_t interpolationErrors;          ///< Interpolation errors
        uint32_t extrapolationErrors;          ///< Extrapolation errors
        uint32_t predictionErrors;             ///< Prediction errors
        uint32_t reconciliationCount;          ///< Server reconciliation count
    };

    /**
     * @class NetworkManager
     * @brief Main network manager for VoxelCraft
     *
     * The NetworkManager class provides comprehensive networking functionality
     * for the VoxelCraft game engine, including client-server architecture,
     * state synchronization, latency management, and secure authentication.
     *
     * Key Features:
     * - Client-server architecture with dedicated server support
     * - Advanced state synchronization with interpolation and prediction
     * - Latency management with automatic measurement and compensation
     * - Secure authentication and encryption systems
     * - Bandwidth optimization and throttling
     * - Connection management with automatic reconnection
     * - Network statistics and debugging tools
     * - Cross-platform networking with IPv4/IPv6 support
     * - Thread-safe operations with performance optimization
     *
     * The network system supports multiple operation modes:
     * - Single player (offline mode)
     * - Client mode for connecting to servers
     * - Server mode for hosting games
     * - Dedicated server mode for large-scale hosting
     * - Listen server mode (client + server combined)
     */
    class NetworkManager : public System {
    public:
        /**
         * @brief Constructor
         * @param config Network manager configuration
         */
        explicit NetworkManager(const NetworkManagerConfig& config);

        /**
         * @brief Destructor
         */
        ~NetworkManager();

        /**
         * @brief Deleted copy constructor
         */
        NetworkManager(const NetworkManager&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        NetworkManager& operator=(const NetworkManager&) = delete;

        // System lifecycle

        /**
         * @brief Initialize network manager
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Shutdown network manager
         */
        void Shutdown();

        /**
         * @brief Update network manager
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Get network state
         * @return Current network state
         */
        NetworkState GetState() const { return m_state; }

        /**
         * @brief Get network mode
         * @return Current network mode
         */
        NetworkMode GetMode() const { return m_mode; }

        /**
         * @brief Get connection state
         * @return Current connection state
         */
        ConnectionState GetConnectionState() const { return m_connectionState; }

        // Server operations

        /**
         * @brief Start server
         * @param port Server port
         * @param maxPlayers Maximum number of players
         * @return true if successful, false otherwise
         */
        bool StartServer(uint16_t port, uint32_t maxPlayers);

        /**
         * @brief Stop server
         * @return true if successful, false otherwise
         */
        bool StopServer();

        /**
         * @brief Check if server is running
         * @return true if running, false otherwise
         */
        bool IsServerRunning() const;

        /**
         * @brief Get server information
         * @return Server info as string
         */
        std::string GetServerInfo() const;

        // Client operations

        /**
         * @brief Connect to server
         * @param address Server address
         * @param port Server port
         * @return true if connection started, false otherwise
         */
        bool Connect(const std::string& address, uint16_t port);

        /**
         * @brief Disconnect from server
         * @return true if disconnection started, false otherwise
         */
        bool Disconnect();

        /**
         * @brief Check if connected to server
         * @return true if connected, false otherwise
         */
        bool IsConnected() const { return m_connectionState == ConnectionState::Connected; }

        /**
         * @brief Get connection information
         * @return Connection info as string
         */
        std::string GetConnectionInfo() const;

        // Message operations

        /**
         * @brief Send message to peer
         * @param peerId Target peer ID (0 for server/all clients)
         * @param message Message to send
         * @return Message ID or 0 if failed
         */
        uint32_t SendMessage(uint32_t peerId, const NetworkMessage& message);

        /**
         * @brief Send message to all peers
         * @param message Message to send
         * @return Message ID or 0 if failed
         */
        uint32_t SendMessageToAll(const NetworkMessage& message);

        /**
         * @brief Broadcast message to all connected peers
         * @param message Message to broadcast
         * @return Message ID or 0 if failed
         */
        uint32_t BroadcastMessage(const NetworkMessage& message);

        /**
         * @brief Register message handler
         * @param messageType Message type to handle
         * @param handler Handler function
         * @return Handler ID
         */
        uint32_t RegisterMessageHandler(NetworkMessageType messageType,
                                       std::function<void(const NetworkMessage&)> handler);

        /**
         * @brief Unregister message handler
         * @param handlerId Handler ID
         * @return true if removed, false otherwise
         */
        bool UnregisterMessageHandler(uint32_t handlerId);

        // Peer management

        /**
         * @brief Get peer information
         * @param peerId Peer ID
         * @return Peer information or nullptr if not found
         */
        const NetworkPeer* GetPeer(uint32_t peerId) const;

        /**
         * @brief Get all connected peers
         * @return Vector of connected peers
         */
        std::vector<NetworkPeer> GetConnectedPeers() const;

        /**
         * @brief Get peer count
         * @return Number of connected peers
         */
        uint32_t GetPeerCount() const;

        /**
         * @brief Kick peer from server
         * @param peerId Peer ID to kick
         * @param reason Kick reason
         * @return true if successful, false otherwise
         */
        bool KickPeer(uint32_t peerId, const std::string& reason = "");

        /**
         * @brief Ban peer from server
         * @param peerId Peer ID to ban
         * @param reason Ban reason
         * @return true if successful, false otherwise
         */
        bool BanPeer(uint32_t peerId, const std::string& reason = "");

        // Authentication

        /**
         * @brief Authenticate with server
         * @param username Username
         * @param password Password
         * @return true if authentication started, false otherwise
         */
        bool Authenticate(const std::string& username, const std::string& password);

        /**
         * @brief Check if authenticated
         * @return true if authenticated, false otherwise
         */
        bool IsAuthenticated() const { return m_isAuthenticated; }

        /**
         * @brief Get authenticated user ID
         * @return User ID or 0 if not authenticated
         */
        uint32_t GetUserId() const { return m_userId; }

        // World and entity synchronization

        /**
         * @brief Synchronize world state
         * @param world World to synchronize
         * @return true if synchronization started, false otherwise
         */
        bool SynchronizeWorld(World* world);

        /**
         * @brief Synchronize entity
         * @param entityId Entity ID
         * @param force Force synchronization
         * @return true if synchronization started, false otherwise
         */
        bool SynchronizeEntity(uint32_t entityId, bool force = false);

        /**
         * @brief Synchronize player
         * @param player Player to synchronize
         * @return true if synchronization started, false otherwise
         */
        bool SynchronizePlayer(Player* player);

        // Latency and timing

        /**
         * @brief Get current ping to server
         * @return Ping in milliseconds
         */
        uint32_t GetPing() const { return m_currentPing; }

        /**
         * @brief Get average ping
         * @return Average ping in milliseconds
         */
        uint32_t GetAveragePing() const { return m_averagePing; }

        /**
         * @brief Measure latency to peer
         * @param peerId Peer ID
         * @return true if measurement started, false otherwise
         */
        bool MeasureLatency(uint32_t peerId);

        /**
         * @brief Get network time
         * @return Network time in seconds
         */
        double GetNetworkTime() const { return m_networkTime; }

        /**
         * @brief Synchronize network time
         * @param serverTime Server time
         * @return true if synchronization started, false otherwise
         */
        bool SynchronizeTime(double serverTime);

        // Configuration

        /**
         * @brief Get network manager configuration
         * @return Current configuration
         */
        const NetworkManagerConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set network manager configuration
         * @param config New configuration
         */
        void SetConfig(const NetworkManagerConfig& config);

        // Metrics and monitoring

        /**
         * @brief Get network manager metrics
         * @return Performance metrics
         */
        const NetworkManagerMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

        /**
         * @brief Get performance report
         * @return Detailed performance report
         */
        std::string GetPerformanceReport() const;

        /**
         * @brief Get network statistics
         * @return Network statistics as string
         */
        std::string GetNetworkStats() const;

        // Event system

        /**
         * @brief Add network event listener
         * @param eventType Event type
         * @param listener Event listener function
         * @return Listener ID
         */
        uint32_t AddEventListener(const std::string& eventType,
                                 std::function<void(const std::unordered_map<std::string, std::any>&)> listener);

        /**
         * @brief Remove network event listener
         * @param listenerId Listener ID
         * @return true if removed, false otherwise
         */
        bool RemoveEventListener(uint32_t listenerId);

        /**
         * @brief Trigger network event
         * @param eventType Event type
         * @param eventData Event data
         */
        void TriggerEvent(const std::string& eventType,
                         const std::unordered_map<std::string, std::any>& eventData);

        // Utility functions

        /**
         * @brief Check if running as server
         * @return true if server, false otherwise
         */
        bool IsServer() const {
            return m_mode == NetworkMode::Server ||
                   m_mode == NetworkMode::DedicatedServer ||
                   m_mode == NetworkMode::ListenServer;
        }

        /**
         * @brief Check if running as client
         * @return true if client, false otherwise
         */
        bool IsClient() const {
            return m_mode == NetworkMode::Client ||
                   m_mode == NetworkMode::ListenServer;
        }

        /**
         * @brief Get local peer ID
         * @return Local peer ID
         */
        uint32_t GetLocalPeerId() const { return m_localPeerId; }

        /**
         * @brief Validate network manager state
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Get status report
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

        /**
         * @brief Optimize network performance
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> Optimize();

    private:
        /**
         * @brief Initialize network subsystems
         * @return true if successful, false otherwise
         */
        bool InitializeSubsystems();

        /**
         * @brief Update network state
         * @param deltaTime Time elapsed
         */
        void UpdateNetworkState(double deltaTime);

        /**
         * @brief Process incoming messages
         */
        void ProcessIncomingMessages();

        /**
         * @brief Process outgoing messages
         */
        void ProcessOutgoingMessages();

        /**
         * @brief Handle connection events
         */
        void HandleConnectionEvents();

        /**
         * @brief Update peer states
         * @param deltaTime Time elapsed
         */
        void UpdatePeerStates(double deltaTime);

        /**
         * @brief Send heartbeat messages
         */
        void SendHeartbeat();

        /**
         * @brief Check for timeouts
         */
        void CheckTimeouts();

        /**
         * @brief Handle network errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        /**
         * @brief Update performance metrics
         * @param deltaTime Time elapsed
         */
        void UpdateMetrics(double deltaTime);

        /**
         * @brief Create default message handlers
         */
        void CreateDefaultMessageHandlers();

        // Network manager data
        NetworkManagerConfig m_config;                 ///< Configuration
        NetworkManagerMetrics m_metrics;               ///< Performance metrics
        NetworkState m_state;                          ///< Current state
        NetworkMode m_mode;                            ///< Current mode
        ConnectionState m_connectionState;             ///< Connection state

        // Core systems
        std::unique_ptr<Server> m_server;              ///< Server instance
        std::unique_ptr<Client> m_client;              ///< Client instance
        std::unique_ptr<NetworkProtocol> m_protocol;   ///< Network protocol
        std::unique_ptr<NetworkSecurity> m_security;   ///< Security system
        std::unique_ptr<NetworkSynchronization> m_sync; ///< Synchronization system
        std::unique_ptr<NetworkLatencyManager> m_latency; ///< Latency manager

        // Peer management
        std::unordered_map<uint32_t, NetworkPeer> m_peers; ///< Connected peers
        mutable std::shared_mutex m_peersMutex;         ///< Peers synchronization

        // Message handling
        std::queue<NetworkMessage> m_incomingMessages;  ///< Incoming message queue
        std::queue<NetworkMessage> m_outgoingMessages;  ///< Outgoing message queue
        std::unordered_map<uint32_t, std::function<void(const NetworkMessage&)>> m_messageHandlers; ///< Message handlers
        mutable std::shared_mutex m_messagesMutex;      ///< Messages synchronization

        // Event system
        std::unordered_map<uint32_t, std::function<void(const std::unordered_map<std::string, std::any>&)>> m_eventListeners; ///< Event listeners
        mutable std::shared_mutex m_eventsMutex;        ///< Events synchronization

        // Authentication
        bool m_isAuthenticated;                         ///< Authentication status
        uint32_t m_userId;                              ///< Authenticated user ID
        std::string m_authToken;                        ///< Authentication token

        // Network state
        uint32_t m_localPeerId;                         ///< Local peer ID
        uint32_t m_serverPeerId;                        ///< Server peer ID
        std::string m_serverAddress;                    ///< Connected server address
        uint16_t m_serverPort;                          ///< Connected server port

        // Timing and latency
        double m_networkTime;                           ///< Current network time
        uint32_t m_currentPing;                         ///< Current ping
        uint32_t m_averagePing;                         ///< Average ping
        double m_lastHeartbeat;                         ///< Last heartbeat time
        double m_lastLatencyUpdate;                     ///< Last latency update

        // Message sequencing
        std::atomic<uint32_t> m_nextMessageId;          ///< Next message ID
        std::atomic<uint32_t> m_nextSequenceNumber;     ///< Next sequence number

        // State flags
        bool m_isInitialized;                           ///< System is initialized
        double m_lastUpdateTime;                        ///< Last update time

        static std::atomic<uint32_t> s_nextPeerId;      ///< Next peer ID counter
        static std::atomic<uint32_t> s_nextListenerId;  ///< Next listener ID counter
        static std::atomic<uint32_t> s_nextHandlerId;   ///< Next handler ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_MULTIPLAYER_NETWORK_MANAGER_HPP
