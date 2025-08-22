/**
 * @file Server.hpp
 * @brief VoxelCraft Server - Dedicated Game Server
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the Server class that handles the server-side networking,
 * game state management, player handling, world synchronization, and server
 * administration for the VoxelCraft multiplayer system.
 */

#ifndef VOXELCRAFT_MULTIPLAYER_SERVER_HPP
#define VOXELCRAFT_MULTIPLAYER_SERVER_HPP

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
#include <thread>
#include <condition_variable>

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
    class World;
    class EntityManager;
    class Player;
    class ServerConfig;

    /**
     * @enum ServerState
     * @brief Server operational states
     */
    enum class ServerState {
        Starting,               ///< Server is starting
        Running,                ///< Server is running
        Stopping,               ///< Server is stopping
        Stopped,                ///< Server is stopped
        Error                   ///< Server is in error state
    };

    /**
     * @enum ServerMode
     * @brief Server operation modes
     */
    enum class ServerMode {
        Dedicated,              ///< Dedicated server
        Listen,                 ///< Listen server (with local client)
        Private,                ///< Private server (password protected)
        LAN,                    ///< LAN server (local network only)
        Custom                  ///< Custom server mode
    };

    /**
     * @enum PlayerState
     * @brief Server-side player states
     */
    enum class PlayerState {
        Connecting,             ///< Player is connecting
        Connected,              ///< Player is connected
        Loading,                ///< Player is loading the world
        Playing,                ///< Player is actively playing
        Idle,                   ///< Player is idle
        Disconnecting           ///< Player is disconnecting
    };

    /**
     * @struct ServerPlayer
     * @brief Server-side player information
     */
    struct ServerPlayer {
        uint32_t playerId;                         ///< Unique player ID
        uint32_t peerId;                           ///< Network peer ID
        std::string username;                      ///< Player username
        PlayerState state;                         ///< Player state
        double connectionTime;                     ///< Connection timestamp
        double lastActivity;                       ///< Last activity timestamp
        glm::vec3 position;                        ///< Current position
        glm::quat rotation;                        ///< Current rotation
        uint32_t ping;                             ///< Current ping
        bool isAdmin;                              ///< Player has admin privileges
        std::unordered_map<std::string, std::any> properties; ///< Custom properties

        ServerPlayer()
            : playerId(0)
            , peerId(0)
            , state(PlayerState::Connecting)
            , connectionTime(0.0)
            , lastActivity(0.0)
            , position(0.0f)
            , rotation(1.0f, 0.0f, 0.0f, 0.0f)
            , ping(0)
            , isAdmin(false)
        {}
    };

    /**
     * @struct ServerConfig
     * @brief Server configuration settings
     */
    struct ServerConfig {
        // Basic settings
        ServerMode mode;                           ///< Server operation mode
        std::string serverName;                    ///< Server name
        std::string serverDescription;             ///< Server description
        std::string serverVersion;                 ///< Server version
        uint16_t port;                             ///< Server port
        uint32_t maxPlayers;                       ///< Maximum number of players
        bool enablePassword;                       ///< Enable password protection
        std::string password;                      ///< Server password

        // Performance settings
        uint32_t maxConnections;                   ///< Maximum connections
        float updateInterval;                      ///< Server update interval (seconds)
        uint32_t maxEntities;                      ///< Maximum entities
        uint32_t maxChunks;                        ///< Maximum loaded chunks
        bool enableMultithreading;                 ///< Enable multithreading
        uint32_t threadPoolSize;                   ///< Thread pool size

        // Game settings
        std::string gameMode;                      ///< Game mode
        std::string worldName;                     ///< World name
        std::string worldSeed;                     ///< World seed
        bool enablePVP;                            ///< Enable player vs player
        bool enableAnimals;                        ///< Enable animals
        bool enableMobs;                           ///< Enable hostile mobs
        float difficulty;                          ///< Game difficulty (0.0 - 1.0)

        // Network settings
        uint32_t maxPacketSize;                    ///< Maximum packet size
        float connectionTimeout;                   ///< Connection timeout (seconds)
        float heartbeatInterval;                   ///< Heartbeat interval (seconds)
        bool enableCompression;                    ///< Enable data compression
        bool enableEncryption;                     ///< Enable data encryption

        // Security settings
        bool enableWhitelist;                      ///< Enable whitelist
        std::vector<std::string> whitelist;        ///< Whitelist usernames
        bool enableBlacklist;                      ///< Enable blacklist
        std::vector<std::string> blacklist;        ///< Blacklist usernames
        bool enableAntiCheat;                      ///< Enable anti-cheat system
        uint32_t maxWarnings;                      ///< Maximum warnings before kick

        // Administration settings
        std::vector<std::string> adminUsers;       ///< Administrator usernames
        bool enableRemoteAdmin;                    ///< Enable remote administration
        uint16_t adminPort;                        ///< Admin port
        bool enableConsole;                        ///< Enable server console
        bool enableLogging;                        ///< Enable server logging
        std::string logFile;                       ///< Log file path

        // World settings
        float viewDistance;                        ///< Player view distance
        bool enableWeather;                        ///< Enable weather system
        bool enableDayNightCycle;                  ///< Enable day/night cycle
        uint32_t timeScale;                        ///< Time scale factor

        // Backup settings
        bool enableAutoBackup;                     ///< Enable automatic backups
        uint32_t backupInterval;                   ///< Backup interval (minutes)
        std::string backupDirectory;               ///< Backup directory
        uint32_t maxBackups;                       ///< Maximum backup count

        ServerConfig()
            : mode(ServerMode::Dedicated)
            , serverName("VoxelCraft Server")
            , serverDescription("A VoxelCraft multiplayer server")
            , serverVersion("1.0.0")
            , port(25565)
            , maxPlayers(10)
            , enablePassword(false)
            , maxConnections(16)
            , updateInterval(1.0f / 20.0f)
            , maxEntities(1000)
            , maxChunks(10000)
            , enableMultithreading(true)
            , threadPoolSize(4)
            , gameMode("survival")
            , worldName("world")
            , enablePVP(true)
            , enableAnimals(true)
            , enableMobs(true)
            , difficulty(0.5f)
            , maxPacketSize(4096)
            , connectionTimeout(30.0f)
            , heartbeatInterval(10.0f)
            , enableCompression(true)
            , enableEncryption(false)
            , enableWhitelist(false)
            , enableBlacklist(false)
            , enableAntiCheat(true)
            , maxWarnings(3)
            , enableRemoteAdmin(false)
            , adminPort(25566)
            , enableConsole(true)
            , enableLogging(true)
            , logFile("server.log")
            , viewDistance(16.0f)
            , enableWeather(true)
            , enableDayNightCycle(true)
            , timeScale(1)
            , enableAutoBackup(true)
            , backupInterval(60)
            , backupDirectory("backups")
            , maxBackups(10)
        {}
    };

    /**
     * @struct ServerMetrics
     * @brief Performance metrics for server
     */
    struct ServerMetrics {
        // Performance metrics
        uint64_t updateCount;                      ///< Number of updates performed
        double totalUpdateTime;                    ///< Total update time (ms)
        double averageUpdateTime;                  ///< Average update time (ms)
        double maxUpdateTime;                      ///< Maximum update time (ms)

        // Connection metrics
        uint32_t currentPlayers;                   ///< Current number of players
        uint32_t maxPlayers;                       ///< Maximum players reached
        uint32_t totalConnections;                 ///< Total connections established
        uint32_t failedConnections;                ///< Number of failed connections
        uint32_t kickedPlayers;                    ///< Number of kicked players
        uint32_t bannedPlayers;                    ///< Number of banned players

        // Network metrics
        uint64_t bytesSent;                        ///< Total bytes sent
        uint64_t bytesReceived;                    ///< Total bytes received
        uint32_t packetsPerSecond;                 ///< Packets per second
        uint32_t averagePing;                      ///< Average player ping
        uint32_t maxPing;                          ///< Maximum player ping

        // Game metrics
        uint32_t activeEntities;                   ///< Number of active entities
        uint32_t loadedChunks;                     ///< Number of loaded chunks
        uint32_t activeChunks;                     ///< Number of active chunks
        float tps;                                 ///< Ticks per second
        float averageTps;                          ///< Average TPS

        // Memory metrics
        size_t memoryUsage;                        ///< Current memory usage
        size_t peakMemoryUsage;                    ///< Peak memory usage
        uint32_t activeThreads;                    ///< Number of active threads
        uint32_t threadPoolUsage;                  ///< Thread pool usage

        // Uptime metrics
        double serverUptime;                       ///< Server uptime (seconds)
        double startTime;                          ///< Server start time
        double lastRestart;                        ///< Last restart time
        uint32_t restartCount;                     ///< Number of restarts

        // Error metrics
        uint32_t networkErrors;                    ///< Network errors
        uint32_t gameErrors;                       ///< Game logic errors
        uint32_t systemErrors;                     ///< System errors
        uint32_t criticalErrors;                   ///< Critical errors
    };

    /**
     * @class Server
     * @brief Dedicated game server for VoxelCraft
     *
     * The Server class handles all server-side operations for VoxelCraft multiplayer,
     * including player management, world synchronization, entity handling, security,
     * administration, and performance monitoring.
     *
     * Key Features:
     * - Dedicated server architecture with high performance
     * - Advanced player management and authentication
     * - Real-time world synchronization with compression
     * - Entity synchronization with prediction and reconciliation
     * - Security features including anti-cheat and DDoS protection
     * - Administration tools and remote management
     * - Performance monitoring and optimization
     * - Automatic backups and crash recovery
     * - Plugin/mod support architecture
     * - Multi-threading for optimal performance
     *
     * The server supports multiple operation modes:
     * - Dedicated server for large-scale hosting
     * - Listen server for small groups
     * - LAN server for local network play
     * - Private server with password protection
     *
     * The server is designed to handle hundreds of concurrent players
     * while maintaining low latency and high stability.
     */
    class Server {
    public:
        /**
         * @brief Constructor
         * @param config Server configuration
         */
        explicit Server(const ServerConfig& config);

        /**
         * @brief Destructor
         */
        ~Server();

        /**
         * @brief Deleted copy constructor
         */
        Server(const Server&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        Server& operator=(const Server&) = delete;

        // Server lifecycle

        /**
         * @brief Start server
         * @return true if successful, false otherwise
         */
        bool Start();

        /**
         * @brief Stop server
         * @return true if successful, false otherwise
         */
        bool Stop();

        /**
         * @brief Restart server
         * @return true if successful, false otherwise
         */
        bool Restart();

        /**
         * @brief Update server
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Get server state
         * @return Current server state
         */
        ServerState GetState() const { return m_state; }

        /**
         * @brief Get server mode
         * @return Current server mode
         */
        ServerMode GetMode() const { return m_mode; }

        /**
         * @brief Check if server is running
         * @return true if running, false otherwise
         */
        bool IsRunning() const { return m_state == ServerState::Running; }

        // Configuration

        /**
         * @brief Get server configuration
         * @return Current configuration
         */
        const ServerConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set server configuration
         * @param config New configuration
         * @return true if successful, false otherwise
         */
        bool SetConfig(const ServerConfig& config);

        /**
         * @brief Reload configuration from file
         * @return true if successful, false otherwise
         */
        bool ReloadConfig();

        // Player management

        /**
         * @brief Get connected players
         * @return Vector of connected players
         */
        std::vector<ServerPlayer> GetConnectedPlayers() const;

        /**
         * @brief Get player by ID
         * @param playerId Player ID
         * @return Player information or nullptr if not found
         */
        const ServerPlayer* GetPlayer(uint32_t playerId) const;

        /**
         * @brief Get player by username
         * @param username Player username
         * @return Player information or nullptr if not found
         */
        const ServerPlayer* GetPlayer(const std::string& username) const;

        /**
         * @brief Get player count
         * @return Number of connected players
         */
        uint32_t GetPlayerCount() const;

        /**
         * @brief Kick player
         * @param playerId Player ID
         * @param reason Kick reason
         * @return true if successful, false otherwise
         */
        bool KickPlayer(uint32_t playerId, const std::string& reason = "");

        /**
         * @brief Ban player
         * @param playerId Player ID
         * @param reason Ban reason
         * @param duration Ban duration (0 for permanent)
         * @return true if successful, false otherwise
         */
        bool BanPlayer(uint32_t playerId, const std::string& reason = "", double duration = 0.0);

        /**
         * @brief Unban player
         * @param username Player username
         * @return true if successful, false otherwise
         */
        bool UnbanPlayer(const std::string& username);

        /**
         * @brief Send message to player
         * @param playerId Player ID
         * @param message Message to send
         * @return true if successful, false otherwise
         */
        bool SendMessage(uint32_t playerId, const std::string& message);

        /**
         * @brief Broadcast message to all players
         * @param message Message to broadcast
         * @return true if successful, false otherwise
         */
        bool BroadcastMessage(const std::string& message);

        // World management

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

        /**
         * @brief Load world
         * @param worldName World name
         * @return true if successful, false otherwise
         */
        bool LoadWorld(const std::string& worldName);

        /**
         * @brief Save world
         * @return true if successful, false otherwise
         */
        bool SaveWorld();

        /**
         * @brief Unload world
         * @return true if successful, false otherwise
         */
        bool UnloadWorld();

        // Entity management

        /**
         * @brief Create entity on server
         * @param entityType Entity type
         * @param position Entity position
         * @param properties Entity properties
         * @return Entity ID or 0 if failed
         */
        uint32_t CreateEntity(const std::string& entityType, const glm::vec3& position,
                            const std::unordered_map<std::string, std::any>& properties = {});

        /**
         * @brief Destroy entity
         * @param entityId Entity ID
         * @return true if successful, false otherwise
         */
        bool DestroyEntity(uint32_t entityId);

        /**
         * @brief Update entity
         * @param entityId Entity ID
         * @param properties Entity properties
         * @return true if successful, false otherwise
         */
        bool UpdateEntity(uint32_t entityId, const std::unordered_map<std::string, std::any>& properties);

        // Command system

        /**
         * @brief Execute command
         * @param command Command to execute
         * @param senderId Command sender ID (0 for server)
         * @return true if successful, false otherwise
         */
        bool ExecuteCommand(const std::string& command, uint32_t senderId = 0);

        /**
         * @brief Register command handler
         * @param command Command name
         * @param handler Command handler function
         * @return true if successful, false otherwise
         */
        bool RegisterCommand(const std::string& command,
                           std::function<bool(const std::vector<std::string>&, uint32_t)> handler);

        /**
         * @brief Unregister command handler
         * @param command Command name
         * @return true if successful, false otherwise
         */
        bool UnregisterCommand(const std::string& command);

        // Administration

        /**
         * @brief Add administrator
         * @param username Administrator username
         * @return true if successful, false otherwise
         */
        bool AddAdmin(const std::string& username);

        /**
         * @brief Remove administrator
         * @param username Administrator username
         * @return true if successful, false otherwise
         */
        bool RemoveAdmin(const std::string& username);

        /**
         * @brief Check if user is administrator
         * @param username Username to check
         * @return true if administrator, false otherwise
         */
        bool IsAdmin(const std::string& username) const;

        /**
         * @brief Get administrator list
         * @return Vector of administrator usernames
         */
        std::vector<std::string> GetAdminList() const;

        // Security

        /**
         * @brief Add user to whitelist
         * @param username Username to add
         * @return true if successful, false otherwise
         */
        bool AddToWhitelist(const std::string& username);

        /**
         * @brief Remove user from whitelist
         * @param username Username to remove
         * @return true if successful, false otherwise
         */
        bool RemoveFromWhitelist(const std::string& username);

        /**
         * @brief Check if user is whitelisted
         * @param username Username to check
         * @return true if whitelisted, false otherwise
         */
        bool IsWhitelisted(const std::string& username) const;

        /**
         * @brief Get whitelist
         * @return Vector of whitelisted usernames
         */
        std::vector<std::string> GetWhitelist() const;

        // Backup system

        /**
         * @brief Create backup
         * @param name Backup name (optional)
         * @return true if successful, false otherwise
         */
        bool CreateBackup(const std::string& name = "");

        /**
         * @brief Restore backup
         * @param name Backup name
         * @return true if successful, false otherwise
         */
        bool RestoreBackup(const std::string& name);

        /**
         * @brief List backups
         * @return Vector of backup names
         */
        std::vector<std::string> ListBackups() const;

        /**
         * @brief Delete backup
         * @param name Backup name
         * @return true if successful, false otherwise
         */
        bool DeleteBackup(const std::string& name);

        // Metrics and monitoring

        /**
         * @brief Get server metrics
         * @return Performance metrics
         */
        const ServerMetrics& GetMetrics() const { return m_metrics; }

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
         * @brief Get server information
         * @return Server information as string
         */
        std::string GetServerInfo() const;

        // Utility functions

        /**
         * @brief Get server uptime
         * @return Uptime in seconds
         */
        double GetUptime() const;

        /**
         * @brief Validate server state
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Get status report
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

        /**
         * @brief Optimize server performance
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> Optimize();

    private:
        /**
         * @brief Initialize server components
         * @return true if successful, false otherwise
         */
        bool InitializeComponents();

        /**
         * @brief Start network thread
         */
        void StartNetworkThread();

        /**
         * @brief Stop network thread
         */
        void StopNetworkThread();

        /**
         * @brief Network thread function
         */
        void NetworkThreadFunction();

        /**
         * @brief Start game thread
         */
        void StartGameThread();

        /**
         * @brief Stop game thread
         */
        void StopGameThread();

        /**
         * @brief Game thread function
         */
        void GameThreadFunction();

        /**
         * @brief Handle incoming messages
         */
        void HandleIncomingMessages();

        /**
         * @brief Handle player connections
         */
        void HandlePlayerConnections();

        /**
         * @brief Handle player disconnections
         */
        void HandlePlayerDisconnections();

        /**
         * @brief Handle player updates
         */
        void HandlePlayerUpdates();

        /**
         * @brief Update world state
         */
        void UpdateWorldState();

        /**
         * @brief Update entity states
         */
        void UpdateEntityStates();

        /**
         * @brief Process commands
         */
        void ProcessCommands();

        /**
         * @brief Perform security checks
         */
        void PerformSecurityChecks();

        /**
         * @brief Clean up disconnected players
         */
        void CleanupDisconnectedPlayers();

        /**
         * @brief Update performance metrics
         * @param deltaTime Time elapsed
         */
        void UpdateMetrics(double deltaTime);

        /**
         * @brief Handle server errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        /**
         * @brief Create default command handlers
         */
        void CreateDefaultCommandHandlers();

        /**
         * @brief Save server configuration
         * @return true if successful, false otherwise
         */
        bool SaveConfig();

        /**
         * @brief Load server configuration
         * @return true if successful, false otherwise
         */
        bool LoadConfig();

        // Server data
        ServerConfig m_config;                      ///< Server configuration
        ServerMetrics m_metrics;                    ///< Performance metrics
        ServerState m_state;                        ///< Current server state
        ServerMode m_mode;                          ///< Current server mode

        // Core systems
        World* m_world;                             ///< Game world
        EntityManager* m_entityManager;             ///< Entity manager
        NetworkManager* m_networkManager;           ///< Network manager

        // Player management
        std::unordered_map<uint32_t, ServerPlayer> m_players; ///< Connected players
        std::unordered_map<std::string, uint32_t> m_playerNames; ///< Player name mapping
        mutable std::shared_mutex m_playersMutex;   ///< Players synchronization

        // Entity management
        std::unordered_map<uint32_t, std::unordered_map<std::string, std::any>> m_entities; ///< Server entities
        mutable std::shared_mutex m_entitiesMutex;  ///< Entities synchronization

        // Command system
        std::unordered_map<std::string, std::function<bool(const std::vector<std::string>&, uint32_t)>> m_commandHandlers; ///< Command handlers
        mutable std::shared_mutex m_commandsMutex;  ///< Commands synchronization

        // Security
        std::unordered_set<std::string> m_whitelist; ///< Player whitelist
        std::unordered_set<std::string> m_blacklist; ///< Player blacklist
        std::unordered_map<std::string, double> m_bannedPlayers; ///< Banned players (username -> ban end time)
        mutable std::shared_mutex m_securityMutex;  ///< Security synchronization

        // Threading
        std::unique_ptr<std::thread> m_networkThread; ///< Network thread
        std::unique_ptr<std::thread> m_gameThread;   ///< Game thread
        std::atomic<bool> m_networkRunning;          ///< Network thread running flag
        std::atomic<bool> m_gameRunning;             ///< Game thread running flag
        std::condition_variable m_networkCondition;  ///< Network thread condition
        std::condition_variable m_gameCondition;     ///< Game thread condition
        mutable std::mutex m_networkMutex;           ///< Network synchronization
        mutable std::mutex m_gameMutex;              ///< Game synchronization

        // Timing
        double m_startTime;                          ///< Server start time
        double m_lastUpdateTime;                     ///< Last update time
        double m_lastBackupTime;                     ///< Last backup time

        // State flags
        bool m_isInitialized;                        ///< Server is initialized
        std::string m_lastError;                     ///< Last error message

        static std::atomic<uint32_t> s_nextPlayerId; ///< Next player ID counter
        static std::atomic<uint32_t> s_nextEntityId; ///< Next entity ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_MULTIPLAYER_SERVER_HPP
