/**
 * @file NetworkManager.hpp
 * @brief VoxelCraft Network Manager - Multiplayer Support
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_CORE_NETWORK_MANAGER_HPP
#define VOXELCRAFT_CORE_NETWORK_MANAGER_HPP

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <atomic>
#include <thread>
#include <functional>
#include <chrono>

namespace VoxelCraft {

// Forward declarations
class Config;
struct Vec3;

/**
 * @enum NetworkMode
 * @brief Network operation mode
 */
enum class NetworkMode {
    OFFLINE = 0,      ///< Single player, no networking
    CLIENT,           ///< Client mode, connecting to server
    SERVER,           ///< Server mode, hosting game
    DEDICATED_SERVER  ///< Dedicated server mode
};

/**
 * @enum NetworkState
 * @brief Current network connection state
 */
enum class NetworkState {
    DISCONNECTED = 0, ///< Not connected
    CONNECTING,       ///< Attempting to connect
    CONNECTED,        ///< Successfully connected
    DISCONNECTING,    ///< Disconnecting
    ERROR             ///< Connection error
};

/**
 * @enum PacketType
 * @brief Types of network packets
 */
enum class PacketType {
    // Connection packets
    HANDSHAKE = 0,
    LOGIN,
    LOGOUT,
    PING,
    PONG,

    // Game state packets
    PLAYER_POSITION,
    PLAYER_ROTATION,
    PLAYER_INPUT,
    PLAYER_ANIMATION,
    PLAYER_DAMAGE,
    PLAYER_DEATH,

    // World packets
    CHUNK_DATA,
    BLOCK_UPDATE,
    ENTITY_SPAWN,
    ENTITY_UPDATE,
    ENTITY_DESPAWN,
    WORLD_TIME,

    // Chat packets
    CHAT_MESSAGE,

    // Custom packets
    CUSTOM_START = 1000
};

/**
 * @struct NetworkPacket
 * @brief Network packet structure
 */
struct NetworkPacket {
    uint32_t packetId;                    ///< Unique packet ID
    PacketType type;                      ///< Packet type
    uint64_t timestamp;                   ///< Packet timestamp
    uint32_t senderId;                    ///< Sender player ID
    uint32_t sequenceNumber;              ///< Sequence number for ordering
    std::vector<uint8_t> data;           ///< Packet data
    bool reliable;                        ///< Whether packet requires reliable delivery
};

/**
 * @struct PlayerConnection
 * @brief Player connection information
 */
struct PlayerConnection {
    uint32_t playerId;                    ///< Unique player ID
    std::string playerName;               ///< Player display name
    std::string address;                  ///< Network address
    NetworkState state;                   ///< Connection state
    std::chrono::steady_clock::time_point lastActivity; ///< Last activity timestamp
    uint32_t ping;                        ///< Current ping in ms
    bool authenticated;                   ///< Authentication status
};

/**
 * @struct NetworkMetrics
 * @brief Network performance metrics
 */
struct NetworkMetrics {
    uint64_t bytesSent;                   ///< Total bytes sent
    uint64_t bytesReceived;               ///< Total bytes received
    uint64_t packetsSent;                 ///< Total packets sent
    uint64_t packetsReceived;             ///< Total packets received
    uint64_t packetsLost;                 ///< Total packets lost
    double averageLatency;                ///< Average round-trip time
    uint32_t connectedPlayers;            ///< Number of connected players
    double bandwidthUsed;                 ///< Current bandwidth usage
};

/**
 * @class NetworkManager
 * @brief Main network manager for multiplayer functionality
 *
 * The NetworkManager handles all networking operations including:
 * - Client-server connections
 * - Packet sending/receiving
 * - Player management
 * - World state synchronization
 * - Security and authentication
 * - Performance monitoring
 */
class NetworkManager {
public:
    /**
     * @brief Constructor
     */
    NetworkManager();

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

    /**
     * @brief Initialize the network manager
     * @param config Network configuration
     * @return true if successful, false otherwise
     */
    bool Initialize(const Config& config);

    /**
     * @brief Shutdown the network manager
     */
    void Shutdown();

    /**
     * @brief Update network state
     * @param deltaTime Time elapsed since last update
     */
    void Update(double deltaTime);

    /**
     * @brief Process network events
     */
    void ProcessEvents();

    /**
     * @brief Get current network mode
     * @return Current network mode
     */
    NetworkMode GetMode() const { return m_mode; }

    /**
     * @brief Get current network state
     * @return Current network state
     */
    NetworkState GetState() const { return m_state; }

    /**
     * @brief Get network metrics
     * @return Network performance metrics
     */
    const NetworkMetrics& GetMetrics() const { return m_metrics; }

    /**
     * @brief Connect to server (client mode)
     * @param address Server address (IP:port)
     * @return true if connection started successfully
     */
    bool Connect(const std::string& address);

    /**
     * @brief Start server (server mode)
     * @param port Server port
     * @param maxPlayers Maximum number of players
     * @return true if server started successfully
     */
    bool StartServer(uint16_t port, uint32_t maxPlayers = 10);

    /**
     * @brief Disconnect from server or stop server
     */
    void Disconnect();

    /**
     * @brief Send packet to specific player
     * @param playerId Target player ID
     * @param packet Packet to send
     * @return true if sent successfully
     */
    bool SendPacket(uint32_t playerId, const NetworkPacket& packet);

    /**
     * @brief Send packet to all players
     * @param packet Packet to send
     * @return true if sent successfully
     */
    bool BroadcastPacket(const NetworkPacket& packet);

    /**
     * @brief Get player connection info
     * @param playerId Player ID
     * @return Player connection info, nullptr if not found
     */
    const PlayerConnection* GetPlayerConnection(uint32_t playerId) const;

    /**
     * @brief Get all connected players
     * @return Vector of player connections
     */
    std::vector<PlayerConnection> GetConnectedPlayers() const;

    /**
     * @brief Kick player from server
     * @param playerId Player ID to kick
     * @param reason Kick reason
     * @return true if kicked successfully
     */
    bool KickPlayer(uint32_t playerId, const std::string& reason = "");

    /**
     * @brief Ban player from server
     * @param playerId Player ID to ban
     * @param reason Ban reason
     * @return true if banned successfully
     */
    bool BanPlayer(uint32_t playerId, const std::string& reason = "");

    /**
     * @brief Check if player is banned
     * @param address Player network address
     * @return true if banned
     */
    bool IsPlayerBanned(const std::string& address) const;

    // Event callbacks

    /**
     * @brief Set player connected callback
     * @param callback Callback function
     */
    void SetPlayerConnectedCallback(std::function<void(uint32_t)> callback) {
        m_playerConnectedCallback = callback;
    }

    /**
     * @brief Set player disconnected callback
     * @param callback Callback function
     */
    void SetPlayerDisconnectedCallback(std::function<void(uint32_t)> callback) {
        m_playerDisconnectedCallback = callback;
    }

    /**
     * @brief Set packet received callback
     * @param callback Callback function
     */
    void SetPacketReceivedCallback(std::function<void(const NetworkPacket&)> callback) {
        m_packetReceivedCallback = callback;
    }

    /**
     * @brief Set connection failed callback
     * @param callback Callback function
     */
    void SetConnectionFailedCallback(std::function<void(const std::string&)> callback) {
        m_connectionFailedCallback = callback;
    }

    // Serialization helpers

    /**
     * @brief Serialize Vec3 to bytes
     * @param vec Vector to serialize
     * @return Serialized data
     */
    static std::vector<uint8_t> SerializeVec3(const Vec3& vec);

    /**
     * @brief Deserialize Vec3 from bytes
     * @param data Serialized data
     * @return Deserialized vector
     */
    static Vec3 DeserializeVec3(const std::vector<uint8_t>& data);

    /**
     * @brief Create player position packet
     * @param playerId Player ID
     * @param position Player position
     * @return Network packet
     */
    static NetworkPacket CreatePlayerPositionPacket(uint32_t playerId, const Vec3& position);

    /**
     * @brief Create chat message packet
     * @param playerId Sender player ID
     * @param message Chat message
     * @return Network packet
     */
    static NetworkPacket CreateChatMessagePacket(uint32_t playerId, const std::string& message);

private:
    // Network configuration
    NetworkMode m_mode;                    ///< Current network mode
    std::atomic<NetworkState> m_state;    ///< Current network state
    std::string m_serverAddress;          ///< Server address (client mode)
    uint16_t m_serverPort;                ///< Server port
    uint32_t m_maxPlayers;                ///< Maximum players allowed
    uint32_t m_localPlayerId;             ///< Local player ID

    // Player management
    std::unordered_map<uint32_t, PlayerConnection> m_players; ///< Connected players
    mutable std::mutex m_playersMutex;     ///< Players synchronization

    // Packet handling
    std::queue<NetworkPacket> m_incomingPackets; ///< Incoming packet queue
    std::queue<NetworkPacket> m_outgoingPackets; ///< Outgoing packet queue
    mutable std::mutex m_packetsMutex;     ///< Packets synchronization
    uint32_t m_nextPacketId;              ///< Next packet ID counter

    // Threading
    std::unique_ptr<std::thread> m_networkThread; ///< Network processing thread
    std::atomic<bool> m_running;          ///< Network thread running flag

    // Metrics
    NetworkMetrics m_metrics;             ///< Performance metrics
    mutable std::mutex m_metricsMutex;    ///< Metrics synchronization

    // Security
    std::vector<std::string> m_bannedAddresses; ///< Banned IP addresses
    mutable std::mutex m_banListMutex;    ///< Ban list synchronization

    // Event callbacks
    std::function<void(uint32_t)> m_playerConnectedCallback;
    std::function<void(uint32_t)> m_playerDisconnectedCallback;
    std::function<void(const NetworkPacket&)> m_packetReceivedCallback;
    std::function<void(const std::string&)> m_connectionFailedCallback;

    // Network implementation details (would use sockets, etc.)
    void* m_networkImpl;                  ///< Platform-specific network implementation

    /**
     * @brief Network processing thread function
     */
    void NetworkThread();

    /**
     * @brief Process incoming packets
     */
    void ProcessIncomingPackets();

    /**
     * @brief Process outgoing packets
     */
    void ProcessOutgoingPackets();

    /**
     * @brief Handle player connection
     * @param playerId Connected player ID
     */
    void HandlePlayerConnected(uint32_t playerId);

    /**
     * @brief Handle player disconnection
     * @param playerId Disconnected player ID
     */
    void HandlePlayerDisconnected(uint32_t playerId);

    /**
     * @brief Authenticate player connection
     * @param address Player address
     * @param credentials Authentication credentials
     * @return Player ID if authenticated, 0 otherwise
     */
    uint32_t AuthenticatePlayer(const std::string& address, const std::string& credentials);

    /**
     * @brief Send packet over network
     * @param packet Packet to send
     * @return true if sent successfully
     */
    bool SendNetworkPacket(const NetworkPacket& packet);

    /**
     * @brief Receive packets from network
     * @return Vector of received packets
     */
    std::vector<NetworkPacket> ReceiveNetworkPackets();

    /**
     * @brief Update network metrics
     */
    void UpdateMetrics();

    /**
     * @brief Clean up disconnected players
     */
    void CleanupDisconnectedPlayers();
};

} // namespace VoxelCraft

#endif // VOXELCRAFT_CORE_NETWORK_MANAGER_HPP
