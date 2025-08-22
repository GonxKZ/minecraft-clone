/**
 * @file Client.hpp
 * @brief VoxelCraft Client - Game Client Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the Client class that handles the client-side networking,
 * server connection, state synchronization, input prediction, and local
 * simulation for the VoxelCraft multiplayer system.
 */

#ifndef VOXELCRAFT_MULTIPLAYER_CLIENT_HPP
#define VOXELCRAFT_MULTIPLAYER_CLIENT_HPP

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
#include <deque>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"

namespace VoxelCraft {

    // Forward declarations
    class NetworkManager;
    class NetworkProtocol;
    struct NetworkMessage;
    struct NetworkPeer;
    class Player;
    class World;
    class EntityManager;
    class ClientConfig;

    /**
     * @enum ClientState
     * @brief Client operational states
     */
    enum class ClientState {
        Disconnected,            ///< Client is disconnected
        Connecting,              ///< Client is connecting to server
        Connected,               ///< Client is connected to server
        Loading,                 ///< Client is loading the world
        Playing,                 ///< Client is actively playing
        Disconnecting            ///< Client is disconnecting
    };

    /**
     * @enum ConnectionStatus
     * @brief Connection status details
     */
    enum class ConnectionStatus {
        Success,                ///< Connection successful
        Timeout,                ///< Connection timed out
        Rejected,               ///< Connection rejected by server
        ServerFull,             ///< Server is full
        VersionMismatch,        ///< Protocol version mismatch
        AuthenticationFailed,   ///< Authentication failed
        NetworkError,           ///< Network error
        UnknownError            ///< Unknown error
    };

    /**
     * @enum SynchronizationState
     * @brief Client synchronization states
     */
    enum class SynchronizationState {
        NotSynchronized,        ///< Not synchronized
        Synchronizing,          ///< Synchronizing with server
        Synchronized,           ///< Synchronized with server
        Desynchronized          ///< Desynchronized from server
    };

    /**
     * @struct ClientConfig
     * @brief Client configuration settings
     */
    struct ClientConfig {
        // Basic settings
        std::string serverAddress;              ///< Server address to connect to
        uint16_t serverPort;                    ///< Server port
        std::string username;                   ///< Client username
        std::string password;                   ///< Client password (if required)

        // Connection settings
        float connectionTimeout;                ///< Connection timeout (seconds)
        float reconnectionDelay;                ///< Reconnection delay (seconds)
        uint32_t maxReconnectionAttempts;       ///< Maximum reconnection attempts
        bool enableAutoReconnect;               ///< Enable automatic reconnection

        // Performance settings
        float updateInterval;                   ///< Client update interval (seconds)
        uint32_t maxPacketSize;                 ///< Maximum packet size
        bool enableCompression;                 ///< Enable data compression
        bool enableEncryption;                  ///< Enable data encryption

        // Synchronization settings
        float interpolationDelay;               ///< Interpolation delay (seconds)
        float extrapolationLimit;               ///< Extrapolation limit (seconds)
        bool enableClientSidePrediction;        ///< Enable client-side prediction
        bool enableServerReconciliation;        ///< Enable server reconciliation
        float predictionErrorThreshold;         ///< Prediction error threshold

        // Network settings
        uint32_t sendRate;                      ///< Send rate (packets per second)
        uint32_t receiveRate;                   ///< Receive rate (packets per second)
        float latencySmoothing;                 ///< Latency smoothing factor
        bool enableLatencyCompensation;         ///< Enable latency compensation

        // Local simulation settings
        bool enableLocalSimulation;             ///< Enable local simulation
        float simulationRate;                   ///< Local simulation rate
        uint32_t maxSimulationSteps;            ///< Maximum simulation steps
        bool enableInputBuffering;              ///< Enable input buffering

        // Debug settings
        bool enableDebugLogging;                ///< Enable debug logging
        bool enableNetworkStats;                ///< Enable network statistics
        bool enableSyncDebugging;               ///< Enable synchronization debugging
        std::string logFile;                    ///< Client log file

        ClientConfig()
            : serverAddress("localhost")
            , serverPort(25565)
            , connectionTimeout(30.0f)
            , reconnectionDelay(5.0f)
            , maxReconnectionAttempts(5)
            , enableAutoReconnect(true)
            , updateInterval(1.0f / 60.0f)
            , maxPacketSize(4096)
            , enableCompression(true)
            , enableEncryption(false)
            , interpolationDelay(0.1f)
            , extrapolationLimit(0.5f)
            , enableClientSidePrediction(true)
            , enableServerReconciliation(true)
            , predictionErrorThreshold(0.1f)
            , sendRate(30)
            , receiveRate(30)
            , latencySmoothing(0.1f)
            , enableLatencyCompensation(true)
            , enableLocalSimulation(true)
            , simulationRate(1.0f / 60.0f)
            , maxSimulationSteps(10)
            , enableInputBuffering(true)
            , enableDebugLogging(false)
            , enableNetworkStats(false)
            , enableSyncDebugging(false)
            , logFile("client.log")
        {}
    };

    /**
     * @struct ClientMetrics
     * @brief Performance metrics for client
     */
    struct ClientMetrics {
        // Performance metrics
        uint64_t updateCount;                  ///< Number of updates performed
        double totalUpdateTime;                ///< Total update time (ms)
        double averageUpdateTime;              ///< Average update time (ms)
        double maxUpdateTime;                  ///< Maximum update time (ms)

        // Connection metrics
        double connectionTime;                 ///< Connection time (seconds)
        double disconnectionTime;              ///< Disconnection time
        uint32_t reconnectionCount;            ///< Number of reconnections
        uint32_t connectionFailures;           ///< Number of connection failures

        // Network metrics
        uint64_t bytesSent;                    ///< Total bytes sent
        uint64_t bytesReceived;                ///< Total bytes received
        uint32_t packetsSent;                  ///< Total packets sent
        uint32_t packetsReceived;              ///< Total packets received
        uint32_t packetsLost;                  ///< Total packets lost
        uint32_t currentPing;                  ///< Current ping (ms)
        uint32_t averagePing;                  ///< Average ping (ms)
        uint32_t maxPing;                      ///< Maximum ping (ms)

        // Synchronization metrics
        uint32_t syncErrors;                   ///< Synchronization errors
        uint32_t predictionErrors;             ///< Prediction errors
        uint32_t reconciliationCount;          ///< Server reconciliation count
        float averageInterpolationError;       ///< Average interpolation error
        float averagePredictionError;          ///< Average prediction error

        // Simulation metrics
        uint32_t simulationSteps;              ///< Total simulation steps
        double totalSimulationTime;            ///< Total simulation time
        double averageSimulationTime;          ///< Average simulation time
        uint32_t simulationErrors;             ///< Simulation errors

        // Input metrics
        uint32_t inputEvents;                  ///< Total input events
        uint32_t bufferedInputs;               ///< Buffered input events
        double averageInputLatency;            ///< Average input latency

        // Error metrics
        uint32_t networkErrors;                ///< Network errors
        uint32_t protocolErrors;               ///< Protocol errors
        uint32_t synchronizationErrors;        ///< Synchronization errors
        uint32_t predictionErrorsCount;        ///< Prediction errors count
    };

    /**
     * @struct PredictedState
     * @brief Client-side predicted state
     */
    struct PredictedState {
        uint32_t sequenceNumber;               ///< Prediction sequence number
        double timestamp;                      ///< Prediction timestamp
        glm::vec3 position;                    ///< Predicted position
        glm::quat rotation;                    ///< Predicted rotation
        glm::vec3 velocity;                    ///< Predicted velocity
        std::unordered_map<std::string, std::any> properties; ///< Additional properties

        PredictedState()
            : sequenceNumber(0)
            , timestamp(0.0)
            , position(0.0f)
            , rotation(1.0f, 0.0f, 0.0f, 0.0f)
            , velocity(0.0f)
        {}
    };

    /**
     * @struct InputCommand
     * @brief Input command for client-side prediction
     */
    struct InputCommand {
        uint32_t sequenceNumber;               ///< Command sequence number
        double timestamp;                      ///< Command timestamp
        uint32_t inputFlags;                   ///< Input flags (movement, actions, etc.)
        glm::vec2 mouseDelta;                  ///< Mouse movement delta
        float deltaTime;                       ///< Time delta for this command
        std::unordered_map<std::string, std::any> properties; ///< Additional properties

        InputCommand()
            : sequenceNumber(0)
            , timestamp(0.0)
            , inputFlags(0)
            , mouseDelta(0.0f)
            , deltaTime(0.0f)
        {}
    };

    /**
     * @class Client
     * @brief Game client for VoxelCraft multiplayer
     *
     * The Client class handles all client-side operations for VoxelCraft multiplayer,
     * including server connection, state synchronization, client-side prediction,
     * input buffering, and local simulation with server reconciliation.
     *
     * Key Features:
     * - Automatic server connection and reconnection
     * - Client-side prediction with server reconciliation
     * - State synchronization with interpolation and extrapolation
     * - Input buffering and latency compensation
     * - Local simulation with rollback on server correction
     * - Network statistics and debugging tools
     * - Authentication and secure connection handling
     * - Performance monitoring and optimization
     * - Cross-platform networking support
     *
     * The client implements advanced networking techniques:
     * - Client-side prediction for responsive gameplay
     * - Entity interpolation for smooth movement
     * - Input buffering for consistent timing
     * - Latency compensation for fair gameplay
     * - Automatic reconnection with state recovery
     * - Bandwidth optimization with delta compression
     *
     * The client is designed to provide a seamless multiplayer experience
     * with minimal perceived latency while maintaining game state consistency.
     */
    class Client {
    public:
        /**
         * @brief Constructor
         * @param config Client configuration
         */
        explicit Client(const ClientConfig& config);

        /**
         * @brief Destructor
         */
        ~Client();

        /**
         * @brief Deleted copy constructor
         */
        Client(const Client&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        Client& operator=(const Client&) = delete;

        // Client lifecycle

        /**
         * @brief Initialize client
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Shutdown client
         */
        void Shutdown();

        /**
         * @brief Update client
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Get client state
         * @return Current client state
         */
        ClientState GetState() const { return m_state; }

        /**
         * @brief Get connection status
         * @return Current connection status
         */
        ConnectionStatus GetConnectionStatus() const { return m_connectionStatus; }

        /**
         * @brief Get synchronization state
         * @return Current synchronization state
         */
        SynchronizationState GetSynchronizationState() const { return m_syncState; }

        /**
         * @brief Check if connected to server
         * @return true if connected, false otherwise
         */
        bool IsConnected() const { return m_state == ClientState::Connected || m_state == ClientState::Playing; }

        // Connection management

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
         * @brief Reconnect to server
         * @return true if reconnection started, false otherwise
         */
        bool Reconnect();

        /**
         * @brief Get connection information
         * @return Connection info as string
         */
        std::string GetConnectionInfo() const;

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

        // Player and entity management

        /**
         * @brief Set local player
         * @param player Player instance
         */
        void SetLocalPlayer(Player* player);

        /**
         * @brief Get local player
         * @return Local player instance
         */
        Player* GetLocalPlayer() const { return m_localPlayer; }

        /**
         * @brief Set world instance
         * @param world World instance
         */
        void SetWorld(World* world);

        /**
         * @brief Get world instance
         * @return World instance
         */
        World* GetWorld() const { return m_world; }

        // Input handling

        /**
         * @brief Queue input command
         * @param command Input command
         * @return true if queued, false otherwise
         */
        bool QueueInputCommand(const InputCommand& command);

        /**
         * @brief Process pending input commands
         * @param deltaTime Time elapsed
         */
        void ProcessInputCommands(double deltaTime);

        /**
         * @brief Get input command buffer size
         * @return Number of buffered input commands
         */
        size_t GetInputBufferSize() const { return m_inputBuffer.size(); }

        // Prediction and reconciliation

        /**
         * @brief Predict local player movement
         * @param deltaTime Time elapsed
         * @param inputCommand Current input command
         */
        void PredictPlayerMovement(double deltaTime, const InputCommand& inputCommand);

        /**
         * @brief Reconcile with server state
         * @param serverState Server state
         * @param sequenceNumber Server sequence number
         */
        void ReconcileWithServer(const std::unordered_map<std::string, std::any>& serverState,
                                uint32_t sequenceNumber);

        /**
         * @brief Get prediction error
         * @return Current prediction error
         */
        float GetPredictionError() const { return m_predictionError; }

        /**
         * @brief Get reconciliation count
         * @return Number of server reconciliations
         */
        uint32_t GetReconciliationCount() const { return m_reconciliationCount; }

        // State synchronization

        /**
         * @brief Synchronize with server
         * @return true if synchronization started, false otherwise
         */
        bool SynchronizeWithServer();

        /**
         * @brief Check if synchronized
         * @return true if synchronized, false otherwise
         */
        bool IsSynchronized() const { return m_syncState == SynchronizationState::Synchronized; }

        /**
         * @brief Get synchronization progress
         * @return Progress (0.0 - 1.0)
         */
        float GetSynchronizationProgress() const { return m_syncProgress; }

        // Network information

        /**
         * @brief Get current ping
         * @return Ping in milliseconds
         */
        uint32_t GetPing() const { return m_currentPing; }

        /**
         * @brief Get average ping
         * @return Average ping in milliseconds
         */
        uint32_t GetAveragePing() const { return m_averagePing; }

        /**
         * @brief Get packet loss
         * @return Packet loss percentage (0.0 - 1.0)
         */
        float GetPacketLoss() const { return m_packetLoss; }

        /**
         * @brief Measure latency to server
         * @return true if measurement started, false otherwise
         */
        bool MeasureLatency();

        // Configuration

        /**
         * @brief Get client configuration
         * @return Current configuration
         */
        const ClientConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set client configuration
         * @param config New configuration
         */
        void SetConfig(const ClientConfig& config);

        // Metrics and monitoring

        /**
         * @brief Get client metrics
         * @return Performance metrics
         */
        const ClientMetrics& GetMetrics() const { return m_metrics; }

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
         * @brief Add client event listener
         * @param eventType Event type
         * @param listener Event listener function
         * @return Listener ID
         */
        uint32_t AddEventListener(const std::string& eventType,
                                 std::function<void(const std::unordered_map<std::string, std::any>&)> listener);

        /**
         * @brief Remove client event listener
         * @param listenerId Listener ID
         * @return true if removed, false otherwise
         */
        bool RemoveEventListener(uint32_t listenerId);

        /**
         * @brief Trigger client event
         * @param eventType Event type
         * @param eventData Event data
         */
        void TriggerEvent(const std::string& eventType,
                         const std::unordered_map<std::string, std::any>& eventData);

        // Utility functions

        /**
         * @brief Get client uptime
         * @return Uptime in seconds
         */
        double GetUptime() const;

        /**
         * @brief Validate client state
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Get status report
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

        /**
         * @brief Optimize client performance
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> Optimize();

    private:
        /**
         * @brief Initialize client components
         * @return true if successful, false otherwise
         */
        bool InitializeComponents();

        /**
         * @brief Handle connection process
         */
        void HandleConnection();

        /**
         * @brief Handle disconnection process
         */
        void HandleDisconnection();

        /**
         * @brief Handle authentication process
         */
        void HandleAuthentication();

        /**
         * @brief Handle world loading
         */
        void HandleWorldLoading();

        /**
         * @brief Process incoming messages
         */
        void ProcessIncomingMessages();

        /**
         * @brief Process outgoing messages
         */
        void ProcessOutgoingMessages();

        /**
         * @brief Update local simulation
         * @param deltaTime Time elapsed
         */
        void UpdateLocalSimulation(double deltaTime);

        /**
         * @brief Update entity interpolation
         * @param deltaTime Time elapsed
         */
        void UpdateEntityInterpolation(double deltaTime);

        /**
         * @brief Update network statistics
         */
        void UpdateNetworkStats();

        /**
         * @brief Clean up old prediction states
         */
        void CleanupPredictionStates();

        /**
         * @brief Apply server correction
         * @param serverState Server state
         * @param sequenceNumber Server sequence number
         */
        void ApplyServerCorrection(const std::unordered_map<std::string, std::any>& serverState,
                                  uint32_t sequenceNumber);

        /**
         * @brief Update performance metrics
         * @param deltaTime Time elapsed
         */
        void UpdateMetrics(double deltaTime);

        /**
         * @brief Handle client errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        /**
         * @brief Create default event handlers
         */
        void CreateDefaultEventHandlers();

        // Client data
        ClientConfig m_config;                     ///< Client configuration
        ClientMetrics m_metrics;                   ///< Performance metrics
        ClientState m_state;                       ///< Current client state
        ConnectionStatus m_connectionStatus;       ///< Connection status
        SynchronizationState m_syncState;          ///< Synchronization state

        // Core systems
        Player* m_localPlayer;                     ///< Local player instance
        World* m_world;                            ///< Game world instance
        NetworkManager* m_networkManager;          ///< Network manager

        // Network state
        uint32_t m_clientId;                       ///< Client ID assigned by server
        uint32_t m_userId;                         ///< Authenticated user ID
        bool m_isAuthenticated;                    ///< Authentication status
        std::string m_serverAddress;               ///< Connected server address
        uint16_t m_serverPort;                     ///< Connected server port
        double m_connectionStartTime;              ///< Connection start time
        double m_lastMessageTime;                  ///< Last message timestamp

        // Prediction and reconciliation
        std::deque<InputCommand> m_inputBuffer;    ///< Input command buffer
        std::unordered_map<uint32_t, PredictedState> m_predictionHistory; ///< Prediction history
        uint32_t m_lastReconciledSequence;         ///< Last reconciled sequence number
        float m_predictionError;                   ///< Current prediction error
        uint32_t m_reconciliationCount;            ///< Server reconciliation count
        mutable std::shared_mutex m_predictionMutex; ///< Prediction synchronization

        // Synchronization
        float m_syncProgress;                      ///< Synchronization progress (0.0 - 1.0)
        double m_syncStartTime;                    ///< Synchronization start time
        std::unordered_map<std::string, std::any> m_pendingSyncState; ///< Pending sync state

        // Network statistics
        uint32_t m_currentPing;                    ///< Current ping
        uint32_t m_averagePing;                    ///< Average ping
        float m_packetLoss;                        ///< Packet loss rate
        uint32_t m_packetsSent;                    ///< Packets sent counter
        uint32_t m_packetsReceived;                ///< Packets received counter
        double m_lastLatencyUpdate;                ///< Last latency update time

        // Event system
        std::unordered_map<uint32_t, std::function<void(const std::unordered_map<std::string, std::any>&)>> m_eventListeners; ///< Event listeners
        mutable std::shared_mutex m_eventsMutex;   ///< Events synchronization

        // Input command sequencing
        std::atomic<uint32_t> m_nextCommandSequence; ///< Next command sequence number

        // State flags
        bool m_isInitialized;                       ///< Client is initialized
        double m_startTime;                         ///< Client start time
        double m_lastUpdateTime;                    ///< Last update time
        std::string m_lastError;                    ///< Last error message

        static std::atomic<uint32_t> s_nextClientId; ///< Next client ID counter
        static std::atomic<uint32_t> s_nextListenerId; ///< Next listener ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_MULTIPLAYER_CLIENT_HPP
