/**
 * @file NetworkMessages.hpp
 * @brief VoxelCraft Network Messages - Message Definitions
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines all network message types and structures used in the
 * VoxelCraft multiplayer system, including serialization and deserialization
 * functions for each message type.
 */

#ifndef VOXELCRAFT_MULTIPLAYER_NETWORK_MESSAGES_HPP
#define VOXELCRAFT_MULTIPLAYER_NETWORK_MESSAGES_HPP

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <any>
#include <optional>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"

namespace VoxelCraft {

    /**
     * @struct NetworkMessages
     * @brief Collection of all network message types
     */
    struct NetworkMessages {

        // =============================================================================
        // CONNECTION MESSAGES
        // =============================================================================

        /**
         * @struct ConnectionRequestMessage
         * @brief Client connection request
         */
        struct ConnectionRequestMessage {
            std::string clientVersion;             ///< Client version string
            std::string username;                  ///< Requested username
            std::string password;                  ///< Password (if required)
            std::unordered_map<std::string, std::any> clientInfo; ///< Client information

            static std::string GetMessageType() { return "ConnectionRequest"; }

            template<typename Serializer>
            static std::vector<uint8_t> Serialize(const ConnectionRequestMessage& msg) {
                return Serializer::Serialize(msg);
            }

            template<typename Serializer>
            static std::optional<ConnectionRequestMessage> Deserialize(const std::vector<uint8_t>& data) {
                return Serializer::Deserialize<ConnectionRequestMessage>(data);
            }
        };

        /**
         * @struct ConnectionAcceptMessage
         * @brief Server connection acceptance
         */
        struct ConnectionAcceptMessage {
            uint32_t clientId;                     ///< Assigned client ID
            uint32_t playerId;                     ///< Assigned player ID
            std::string serverName;                ///< Server name
            std::string serverVersion;             ///< Server version
            std::unordered_map<std::string, std::any> serverInfo; ///< Server information

            static std::string GetMessageType() { return "ConnectionAccept"; }

            template<typename Serializer>
            static std::vector<uint8_t> Serialize(const ConnectionAcceptMessage& msg) {
                return Serializer::Serialize(msg);
            }

            template<typename Serializer>
            static std::optional<ConnectionAcceptMessage> Deserialize(const std::vector<uint8_t>& data) {
                return Serializer::Deserialize<ConnectionAcceptMessage>(data);
            }
        };

        /**
         * @struct ConnectionRejectMessage
         * @brief Server connection rejection
         */
        struct ConnectionRejectMessage {
            std::string reason;                    ///< Rejection reason
            std::string suggestion;                ///< Suggestion for user
            std::unordered_map<std::string, std::any> additionalInfo; ///< Additional information

            static std::string GetMessageType() { return "ConnectionReject"; }

            template<typename Serializer>
            static std::vector<uint8_t> Serialize(const ConnectionRejectMessage& msg) {
                return Serializer::Serialize(msg);
            }

            template<typename Serializer>
            static std::optional<ConnectionRejectMessage> Deserialize(const std::vector<uint8_t>& data) {
                return Serializer::Deserialize<ConnectionRejectMessage>(data);
            }
        };

        /**
         * @struct ConnectionCloseMessage
         * @brief Connection closure notification
         */
        struct ConnectionCloseMessage {
            std::string reason;                    ///< Closure reason
            bool canReconnect;                     ///< Whether client can reconnect
            std::unordered_map<std::string, std::any> additionalInfo; ///< Additional information

            static std::string GetMessageType() { return "ConnectionClose"; }

            template<typename Serializer>
            static std::vector<uint8_t> Serialize(const ConnectionCloseMessage& msg) {
                return Serializer::Serialize(msg);
            }

            template<typename Serializer>
            static std::optional<ConnectionCloseMessage> Deserialize(const std::vector<uint8_t>& data) {
                return Serializer::Deserialize<ConnectionCloseMessage>(data);
            }
        };

        // =============================================================================
        // AUTHENTICATION MESSAGES
        // =============================================================================

        /**
         * @struct AuthenticationRequestMessage
         * @brief Authentication request
         */
        struct AuthenticationRequestMessage {
            std::string username;                  ///< Username
            std::string passwordHash;              ///< Password hash
            std::string authenticationToken;       ///< Authentication token (if available)
            std::unordered_map<std::string, std::any> authData; ///< Additional auth data

            static std::string GetMessageType() { return "AuthenticationRequest"; }

            template<typename Serializer>
            static std::vector<uint8_t> Serialize(const AuthenticationRequestMessage& msg) {
                return Serializer::Serialize(msg);
            }

            template<typename Serializer>
            static std::optional<AuthenticationRequestMessage> Deserialize(const std::vector<uint8_t>& data) {
                return Serializer::Deserialize<AuthenticationRequestMessage>(data);
            }
        };

        /**
         * @struct AuthenticationResponseMessage
         * @brief Authentication response
         */
        struct AuthenticationResponseMessage {
            bool success;                          ///< Authentication success
            std::string message;                   ///< Response message
            uint32_t userId;                       ///< User ID (if successful)
            std::string sessionToken;              ///< Session token (if successful)
            std::unordered_map<std::string, std::any> userData; ///< User data (if successful)

            static std::string GetMessageType() { return "AuthenticationResponse"; }

            template<typename Serializer>
            static std::vector<uint8_t> Serialize(const AuthenticationResponseMessage& msg) {
                return Serializer::Serialize(msg);
            }

            template<typename Serializer>
            static std::optional<AuthenticationResponseMessage> Deserialize(const std::vector<uint8_t>& data) {
                return Serializer::Deserialize<AuthenticationResponseMessage>(data);
            }
        };

        // =============================================================================
        // GAME STATE MESSAGES
        // =============================================================================

        /**
         * @struct PlayerJoinMessage
         * @brief Player joining the game
         */
        struct PlayerJoinMessage {
            uint32_t playerId;                     ///< Player ID
            std::string username;                  ///< Player username
            glm::vec3 position;                    ///< Initial position
            glm::quat rotation;                    ///< Initial rotation
            std::unordered_map<std::string, std::any> playerData; ///< Player data

            static std::string GetMessageType() { return "PlayerJoin"; }

            template<typename Serializer>
            static std::vector<uint8_t> Serialize(const PlayerJoinMessage& msg) {
                return Serializer::Serialize(msg);
            }

            template<typename Serializer>
            static std::optional<PlayerJoinMessage> Deserialize(const std::vector<uint8_t>& data) {
                return Serializer::Deserialize<PlayerJoinMessage>(data);
            }
        };

        /**
         * @struct PlayerLeaveMessage
         * @brief Player leaving the game
         */
        struct PlayerLeaveMessage {
            uint32_t playerId;                     ///< Player ID
            std::string reason;                    ///< Leave reason
            std::unordered_map<std::string, std::any> additionalData; ///< Additional data

            static std::string GetMessageType() { return "PlayerLeave"; }

            template<typename Serializer>
            static std::vector<uint8_t> Serialize(const PlayerLeaveMessage& msg) {
                return Serializer::Serialize(msg);
            }

            template<typename Serializer>
            static std::optional<PlayerLeaveMessage> Deserialize(const std::vector<uint8_t>& data) {
                return Serializer::Deserialize<PlayerLeaveMessage>(data);
            }
        };

        /**
         * @struct PlayerUpdateMessage
         * @brief Player state update
         */
        struct PlayerUpdateMessage {
            uint32_t playerId;                     ///< Player ID
            uint32_t sequenceNumber;               ///< Update sequence number
            double timestamp;                      ///< Update timestamp
            glm::vec3 position;                    ///< Player position
            glm::quat rotation;                    ///< Player rotation
            glm::vec3 velocity;                    ///< Player velocity
            uint32_t inputFlags;                   ///< Input flags
            std::unordered_map<std::string, std::any> playerState; ///< Player state data

            static std::string GetMessageType() { return "PlayerUpdate"; }

            template<typename Serializer>
            static std::vector<uint8_t> Serialize(const PlayerUpdateMessage& msg) {
                return Serializer::Serialize(msg);
            }

            template<typename Serializer>
            static std::optional<PlayerUpdateMessage> Deserialize(const std::vector<uint8_t>& data) {
                return Serializer::Deserialize<PlayerUpdateMessage>(data);
            }
        };

        /**
         * @struct EntityCreateMessage
         * @brief Entity creation
         */
        struct EntityCreateMessage {
            uint32_t entityId;                     ///< Entity ID
            std::string entityType;                ///< Entity type
            glm::vec3 position;                    ///< Entity position
            glm::quat rotation;                    ///< Entity rotation
            glm::vec3 scale;                       ///< Entity scale
            std::unordered_map<std::string, std::any> entityData; ///< Entity data

            static std::string GetMessageType() { return "EntityCreate"; }

            template<typename Serializer>
            static std::vector<uint8_t> Serialize(const EntityCreateMessage& msg) {
                return Serializer::Serialize(msg);
            }

            template<typename Serializer>
            static std::optional<EntityCreateMessage> Deserialize(const std::vector<uint8_t>& data) {
                return Serializer::Deserialize<EntityCreateMessage>(data);
            }
        };

        /**
         * @struct EntityUpdateMessage
         * @brief Entity state update
         */
        struct EntityUpdateMessage {
            uint32_t entityId;                     ///< Entity ID
            uint32_t sequenceNumber;               ///< Update sequence number
            double timestamp;                      ///< Update timestamp
            glm::vec3 position;                    ///< Entity position
            glm::quat rotation;                    ///< Entity rotation
            glm::vec3 velocity;                    ///< Entity velocity
            std::unordered_map<std::string, std::any> entityState; ///< Entity state data

            static std::string GetMessageType() { return "EntityUpdate"; }

            template<typename Serializer>
            static std::vector<uint8_t> Serialize(const EntityUpdateMessage& msg) {
                return Serializer::Serialize(msg);
            }

            template<typename Serializer>
            static std::optional<EntityUpdateMessage> Deserialize(const std::vector<uint8_t>& data) {
                return Serializer::Deserialize<EntityUpdateMessage>(data);
            }
        };

        /**
         * @struct EntityDestroyMessage
         * @brief Entity destruction
         */
        struct EntityDestroyMessage {
            uint32_t entityId;                     ///< Entity ID
            std::string reason;                    ///< Destruction reason
            std::unordered_map<std::string, std::any> additionalData; ///< Additional data

            static std::string GetMessageType() { return "EntityDestroy"; }

            template<typename Serializer>
            static std::vector<uint8_t> Serialize(const EntityDestroyMessage& msg) {
                return Serializer::Serialize(msg);
            }

            template<typename Serializer>
            static std::optional<EntityDestroyMessage> Deserialize(const std::vector<uint8_t>& data) {
                return Serializer::Deserialize<EntityDestroyMessage>(data);
            }
        };

        // =============================================================================
        // WORLD MESSAGES
        // =============================================================================

        /**
         * @struct WorldUpdateMessage
         * @brief World state update
         */
        struct WorldUpdateMessage {
            uint32_t sequenceNumber;               ///< Update sequence number
            double timestamp;                      ///< Update timestamp
            std::string worldName;                 ///< World name
            uint32_t chunkCount;                   ///< Number of chunks
            std::vector<std::pair<glm::ivec3, std::vector<uint8_t>>> chunkData; ///< Chunk data
            std::unordered_map<std::string, std::any> worldState; ///< World state data

            static std::string GetMessageType() { return "WorldUpdate"; }

            template<typename Serializer>
            static std::vector<uint8_t> Serialize(const WorldUpdateMessage& msg) {
                return Serializer::Serialize(msg);
            }

            template<typename Serializer>
            static std::optional<WorldUpdateMessage> Deserialize(const std::vector<uint8_t>& data) {
                return Serializer::Deserialize<WorldUpdateMessage>(data);
            }
        };

        /**
         * @struct ChunkUpdateMessage
         * @brief Individual chunk update
         */
        struct ChunkUpdateMessage {
            glm::ivec3 chunkPosition;              ///< Chunk position
            uint32_t chunkVersion;                 ///< Chunk version
            std::vector<uint8_t> blockData;        ///< Block data
            std::vector<uint8_t> lightData;        ///< Light data
            bool isCompressed;                     ///< Data is compressed
            std::unordered_map<std::string, std::any> chunkMetadata; ///< Chunk metadata

            static std::string GetMessageType() { return "ChunkUpdate"; }

            template<typename Serializer>
            static std::vector<uint8_t> Serialize(const ChunkUpdateMessage& msg) {
                return Serializer::Serialize(msg);
            }

            template<typename Serializer>
            static std::optional<ChunkUpdateMessage> Deserialize(const std::vector<uint8_t>& data) {
                return Serializer::Deserialize<ChunkUpdateMessage>(data);
            }
        };

        /**
         * @struct TimeUpdateMessage
         * @brief Game time update
         */
        struct TimeUpdateMessage {
            double serverTime;                     ///< Server time
            double dayTime;                        ///< Day time (0-24000)
            bool isDay;                            ///< Whether it's day
            bool isRaining;                        ///< Whether it's raining
            float rainStrength;                    ///< Rain strength (0.0 - 1.0)
            bool isThundering;                     ///< Whether it's thundering
            float thunderStrength;                 ///< Thunder strength (0.0 - 1.0)

            static std::string GetMessageType() { return "TimeUpdate"; }

            template<typename Serializer>
            static std::vector<uint8_t> Serialize(const TimeUpdateMessage& msg) {
                return Serializer::Serialize(msg);
            }

            template<typename Serializer>
            static std::optional<TimeUpdateMessage> Deserialize(const std::vector<uint8_t>& data) {
                return Serializer::Deserialize<TimeUpdateMessage>(data);
            }
        };

        // =============================================================================
        // CHAT MESSAGES
        // =============================================================================

        /**
         * @struct ChatMessage
         * @brief Chat message
         */
        struct ChatMessage {
            uint32_t senderId;                     ///< Sender ID (0 for server)
            std::string senderName;                ///< Sender name
            std::string message;                   ///< Message content
            glm::vec4 color;                       ///< Message color
            uint32_t channel;                      ///< Chat channel
            double timestamp;                      ///< Message timestamp
            std::unordered_map<std::string, std::any> messageData; ///< Additional data

            static std::string GetMessageType() { return "ChatMessage"; }

            template<typename Serializer>
            static std::vector<uint8_t> Serialize(const ChatMessage& msg) {
                return Serializer::Serialize(msg);
            }

            template<typename Serializer>
            static std::optional<ChatMessage> Deserialize(const std::vector<uint8_t>& data) {
                return Serializer::Deserialize<ChatMessage>(data);
            }
        };

        /**
         * @struct ChatCommandMessage
         * @brief Chat command
         */
        struct ChatCommandMessage {
            uint32_t senderId;                     ///< Sender ID
            std::string command;                   ///< Command string
            std::vector<std::string> arguments;    ///< Command arguments
            double timestamp;                      ///< Command timestamp
            std::unordered_map<std::string, std::any> commandData; ///< Additional data

            static std::string GetMessageType() { return "ChatCommand"; }

            template<typename Serializer>
            static std::vector<uint8_t> Serialize(const ChatCommandMessage& msg) {
                return Serializer::Serialize(msg);
            }

            template<typename Serializer>
            static std::optional<ChatCommandMessage> Deserialize(const std::vector<uint8_t>& data) {
                return Serializer::Deserialize<ChatCommandMessage>(data);
            }
        };

        // =============================================================================
        // SYNCHRONIZATION MESSAGES
        // =============================================================================

        /**
         * @struct StateSynchronizationMessage
         * @brief State synchronization message
         */
        struct StateSynchronizationMessage {
            uint32_t sequenceNumber;               ///< Sequence number
            double timestamp;                      ///< Timestamp
            std::unordered_map<std::string, std::any> fullState; ///< Full state data
            std::unordered_map<std::string, std::any> deltaState; ///< Delta state data
            bool isFullSync;                       ///< Whether this is a full sync
            std::unordered_map<std::string, std::any> syncMetadata; ///< Sync metadata

            static std::string GetMessageType() { return "StateSynchronization"; }

            template<typename Serializer>
            static std::vector<uint8_t> Serialize(const StateSynchronizationMessage& msg) {
                return Serializer::Serialize(msg);
            }

            template<typename Serializer>
            static std::optional<StateSynchronizationMessage> Deserialize(const std::vector<uint8_t>& data) {
                return Serializer::Deserialize<StateSynchronizationMessage>(data);
            }
        };

        /**
         * @struct TimeSynchronizationMessage
         * @brief Time synchronization message
         */
        struct TimeSynchronizationMessage {
            double serverTime;                     ///< Server time
            double clientSendTime;                 ///< Client send time
            double serverReceiveTime;              ///< Server receive time
            double serverSendTime;                 ///< Server send time
            uint32_t sequenceNumber;               ///< Sequence number

            static std::string GetMessageType() { return "TimeSynchronization"; }

            template<typename Serializer>
            static std::vector<uint8_t> Serialize(const TimeSynchronizationMessage& msg) {
                return Serializer::Serialize(msg);
            }

            template<typename Serializer>
            static std::optional<TimeSynchronizationMessage> Deserialize(const std::vector<uint8_t>& data) {
                return Serializer::Deserialize<TimeSynchronizationMessage>(data);
            }
        };

        /**
         * @struct LatencyUpdateMessage
         * @brief Latency measurement update
         */
        struct LatencyUpdateMessage {
            uint32_t pingId;                       ///< Ping ID
            double sendTime;                       ///< Send timestamp
            double receiveTime;                    ///< Receive timestamp
            uint32_t measuredPing;                 ///< Measured ping (ms)
            std::unordered_map<std::string, std::any> latencyData; ///< Additional latency data

            static std::string GetMessageType() { return "LatencyUpdate"; }

            template<typename Serializer>
            static std::vector<uint8_t> Serialize(const LatencyUpdateMessage& msg) {
                return Serializer::Serialize(msg);
            }

            template<typename Serializer>
            static std::optional<LatencyUpdateMessage> Deserialize(const std::vector<uint8_t>& data) {
                return Serializer::Deserialize<LatencyUpdateMessage>(data);
            }
        };

        // =============================================================================
        // ERROR AND CONTROL MESSAGES
        // =============================================================================

        /**
         * @struct ErrorMessage
         * @brief Error message
         */
        struct ErrorMessage {
            std::string errorCode;                 ///< Error code
            std::string errorMessage;              ///< Error message
            std::string errorDetails;              ///< Error details
            uint32_t errorSeverity;                ///< Error severity (1-5)
            std::unordered_map<std::string, std::any> errorData; ///< Additional error data

            static std::string GetMessageType() { return "Error"; }

            template<typename Serializer>
            static std::vector<uint8_t> Serialize(const ErrorMessage& msg) {
                return Serializer::Serialize(msg);
            }

            template<typename Serializer>
            static std::optional<ErrorMessage> Deserialize(const std::vector<uint8_t>& data) {
                return Serializer::Deserialize<ErrorMessage>(data);
            }
        };

        /**
         * @struct WarningMessage
         * @brief Warning message
         */
        struct WarningMessage {
            std::string warningCode;               ///< Warning code
            std::string warningMessage;            ///< Warning message
            uint32_t warningSeverity;              ///< Warning severity (1-3)
            std::unordered_map<std::string, std::any> warningData; ///< Additional warning data

            static std::string GetMessageType() { return "Warning"; }

            template<typename Serializer>
            static std::vector<uint8_t> Serialize(const WarningMessage& msg) {
                return Serializer::Serialize(msg);
            }

            template<typename Serializer>
            static std::optional<WarningMessage> Deserialize(const std::vector<uint8_t>& data) {
                return Serializer::Deserialize<WarningMessage>(data);
            }
        };

        /**
         * @struct HeartbeatMessage
         * @brief Keep-alive heartbeat
         */
        struct HeartbeatMessage {
            double timestamp;                      ///< Heartbeat timestamp
            uint32_t sequenceNumber;               ///< Sequence number
            std::unordered_map<std::string, std::any> heartbeatData; ///< Additional data

            static std::string GetMessageType() { return "Heartbeat"; }

            template<typename Serializer>
            static std::vector<uint8_t> Serialize(const HeartbeatMessage& msg) {
                return Serializer::Serialize(msg);
            }

            template<typename Serializer>
            static std::optional<HeartbeatMessage> Deserialize(const std::vector<uint8_t>& data) {
                return Serializer::Deserialize<HeartbeatMessage>(data);
            }
        };

        /**
         * @struct AcknowledgmentMessage
         * @brief Message acknowledgment
         */
        struct AcknowledgmentMessage {
            uint32_t acknowledgedMessageId;        ///< ID of acknowledged message
            uint32_t acknowledgmentNumber;         ///< Acknowledgment number
            double acknowledgmentTime;             ///< Acknowledgment timestamp
            std::unordered_map<std::string, std::any> ackData; ///< Additional ack data

            static std::string GetMessageType() { return "Acknowledgment"; }

            template<typename Serializer>
            static std::vector<uint8_t> Serialize(const AcknowledgmentMessage& msg) {
                return Serializer::Serialize(msg);
            }

            template<typename Serializer>
            static std::optional<AcknowledgmentMessage> Deserialize(const std::vector<uint8_t>& data) {
                return Serializer::Deserialize<AcknowledgmentMessage>(data);
            }
        };

        // =============================================================================
        // UTILITY FUNCTIONS
        // =============================================================================

        /**
         * @brief Get message type from string
         * @param typeString Type string
         * @return Message type enum or nullopt
         */
        static std::optional<NetworkMessageType> GetMessageTypeFromString(const std::string& typeString);

        /**
         * @brief Get message type string
         * @param messageType Message type enum
         * @return Type string
         */
        static std::string GetMessageTypeString(NetworkMessageType messageType);

        /**
         * @brief Validate message data
         * @param messageType Message type
         * @param data Message data
         * @return true if valid, false otherwise
         */
        static bool ValidateMessageData(NetworkMessageType messageType, const std::vector<uint8_t>& data);

        /**
         * @brief Calculate message checksum
         * @param data Message data
         * @return Checksum value
         */
        static uint32_t CalculateMessageChecksum(const std::vector<uint8_t>& data);

        /**
         * @brief Get message priority
         * @param messageType Message type
         * @return Priority level (0-10, higher is more important)
         */
        static uint32_t GetMessagePriority(NetworkMessageType messageType);

        /**
         * @brief Check if message requires acknowledgment
         * @param messageType Message type
         * @return true if acknowledgment required, false otherwise
         */
        static bool MessageRequiresAcknowledgment(NetworkMessageType messageType);

        /**
         * @brief Get message channel
         * @param messageType Message type
         * @return Recommended network channel
         */
        static NetworkChannel GetMessageChannel(NetworkMessageType messageType);

        /**
         * @brief Get message size estimate
         * @param messageType Message type
         * @return Estimated size in bytes
         */
        static size_t GetMessageSizeEstimate(NetworkMessageType messageType);

    };

    // =============================================================================
    // MESSAGE TYPE TRAITS
    // =============================================================================

    /**
     * @struct MessageTraits
     * @brief Message type traits for compile-time information
     */
    template<typename T>
    struct MessageTraits;

    #define DEFINE_MESSAGE_TRAITS(Type) \
    template<> \
    struct MessageTraits<NetworkMessages::Type> { \
        static constexpr NetworkMessageType messageType = NetworkMessageType::Type; \
        static constexpr bool requiresAck = NetworkMessages::MessageRequiresAcknowledgment(NetworkMessageType::Type); \
        static constexpr NetworkChannel channel = NetworkMessages::GetMessageChannel(NetworkMessageType::Type); \
        static constexpr uint32_t priority = NetworkMessages::GetMessagePriority(NetworkMessageType::Type); \
        static constexpr size_t estimatedSize = NetworkMessages::GetMessageSizeEstimate(NetworkMessageType::Type); \
    };

    // Define traits for all message types
    DEFINE_MESSAGE_TRAITS(ConnectionRequestMessage)
    DEFINE_MESSAGE_TRAITS(ConnectionAcceptMessage)
    DEFINE_MESSAGE_TRAITS(ConnectionRejectMessage)
    DEFINE_MESSAGE_TRAITS(ConnectionCloseMessage)
    DEFINE_MESSAGE_TRAITS(AuthenticationRequestMessage)
    DEFINE_MESSAGE_TRAITS(AuthenticationResponseMessage)
    DEFINE_MESSAGE_TRAITS(PlayerJoinMessage)
    DEFINE_MESSAGE_TRAITS(PlayerLeaveMessage)
    DEFINE_MESSAGE_TRAITS(PlayerUpdateMessage)
    DEFINE_MESSAGE_TRAITS(EntityCreateMessage)
    DEFINE_MESSAGE_TRAITS(EntityUpdateMessage)
    DEFINE_MESSAGE_TRAITS(EntityDestroyMessage)
    DEFINE_MESSAGE_TRAITS(WorldUpdateMessage)
    DEFINE_MESSAGE_TRAITS(ChunkUpdateMessage)
    DEFINE_MESSAGE_TRAITS(TimeUpdateMessage)
    DEFINE_MESSAGE_TRAITS(ChatMessage)
    DEFINE_MESSAGE_TRAITS(ChatCommandMessage)
    DEFINE_MESSAGE_TRAITS(StateSynchronizationMessage)
    DEFINE_MESSAGE_TRAITS(TimeSynchronizationMessage)
    DEFINE_MESSAGE_TRAITS(LatencyUpdateMessage)
    DEFINE_MESSAGE_TRAITS(ErrorMessage)
    DEFINE_MESSAGE_TRAITS(WarningMessage)
    DEFINE_MESSAGE_TRAITS(HeartbeatMessage)
    DEFINE_MESSAGE_TRAITS(AcknowledgmentMessage)

    // =============================================================================
    // MESSAGE HANDLER INTERFACE
    // =============================================================================

    /**
     * @class IMessageHandler
     * @brief Interface for message handlers
     */
    class IMessageHandler {
    public:
        virtual ~IMessageHandler() = default;

        /**
         * @brief Handle incoming message
         * @param message Message to handle
         * @param senderId Sender ID
         * @return true if handled, false otherwise
         */
        virtual bool HandleMessage(const NetworkMessage& message, uint32_t senderId) = 0;

        /**
         * @brief Get supported message types
         * @return Vector of supported message types
         */
        virtual std::vector<NetworkMessageType> GetSupportedMessageTypes() const = 0;
    };

    /**
     * @class MessageHandler
     * @brief Template message handler base class
     */
    template<typename MessageType>
    class MessageHandler : public IMessageHandler {
    public:
        /**
         * @brief Handle message of specific type
         * @param message Message to handle
         * @param senderId Sender ID
         * @return true if handled, false otherwise
         */
        virtual bool HandleMessage(const MessageType& message, uint32_t senderId) = 0;

        /**
         * @brief Handle incoming message (implementation)
         * @param message Message to handle
         * @param senderId Sender ID
         * @return true if handled, false otherwise
         */
        bool HandleMessage(const NetworkMessage& message, uint32_t senderId) override {
            if (auto typedMessage = MessageType::Deserialize<MessageSerializer>(message.data)) {
                return HandleMessage(*typedMessage, senderId);
            }
            return false;
        }

        /**
         * @brief Get supported message types
         * @return Vector containing the supported message type
         */
        std::vector<NetworkMessageType> GetSupportedMessageTypes() const override {
            return { MessageTraits<MessageType>::messageType };
        }
    };

    // =============================================================================
    // MESSAGE FACTORY
    // =============================================================================

    /**
     * @class MessageFactory
     * @brief Factory for creating network messages
     */
    class MessageFactory {
    public:
        /**
         * @brief Create message
         * @tparam MessageType Type of message to create
         * @param message Message data
         * @param senderId Sender ID
         * @param receiverId Receiver ID
         * @return Network message
         */
        template<typename MessageType>
        static NetworkMessage CreateMessage(const MessageType& message,
                                          uint32_t senderId = 0,
                                          uint32_t receiverId = 0) {
            NetworkMessage netMessage;
            netMessage.messageId = GenerateMessageId();
            netMessage.type = MessageTraits<MessageType>::messageType;
            netMessage.senderId = senderId;
            netMessage.receiverId = receiverId;
            netMessage.timestamp = GetCurrentTime();
            netMessage.sequenceNumber = GenerateSequenceNumber();
            netMessage.channel = MessageTraits<MessageType>::channel;
            netMessage.data = MessageType::Serialize<MessageSerializer>(message);
            netMessage.requiresAck = MessageTraits<MessageType>::requiresAck;

            return netMessage;
        }

        /**
         * @brief Parse message
         * @param netMessage Network message
         * @return Parsed message data or nullopt
         */
        template<typename MessageType>
        static std::optional<MessageType> ParseMessage(const NetworkMessage& netMessage) {
            if (netMessage.type == MessageTraits<MessageType>::messageType) {
                return MessageType::Deserialize<MessageSerializer>(netMessage.data);
            }
            return std::nullopt;
        }

        /**
         * @brief Validate message
         * @tparam MessageType Type of message to validate
         * @param message Message to validate
         * @return true if valid, false otherwise
         */
        template<typename MessageType>
        static bool ValidateMessage(const NetworkMessage& message) {
            if (message.type != MessageTraits<MessageType>::messageType) {
                return false;
            }

            // Check if message data can be deserialized
            return MessageType::Deserialize<MessageSerializer>(message.data).has_value();
        }

    private:
        /**
         * @brief Generate unique message ID
         * @return Message ID
         */
        static uint32_t GenerateMessageId();

        /**
         * @brief Generate sequence number
         * @return Sequence number
         */
        static uint32_t GenerateSequenceNumber();

        /**
         * @brief Get current time
         * @return Current time
         */
        static double GetCurrentTime();

        static std::atomic<uint32_t> s_messageIdCounter;
        static std::atomic<uint32_t> s_sequenceNumberCounter;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_MULTIPLAYER_NETWORK_MESSAGES_HPP
