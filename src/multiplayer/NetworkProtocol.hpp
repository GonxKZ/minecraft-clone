/**
 * @file NetworkProtocol.hpp
 * @brief VoxelCraft Network Protocol - Communication Protocols
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the NetworkProtocol class that handles network communication
 * protocols, message serialization/deserialization, packet formatting, and
 * protocol versioning for the VoxelCraft multiplayer system.
 */

#ifndef VOXELCRAFT_MULTIPLAYER_NETWORK_PROTOCOL_HPP
#define VOXELCRAFT_MULTIPLAYER_NETWORK_PROTOCOL_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <chrono>
#include <queue>
#include <optional>
#include <variant>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"

namespace VoxelCraft {

    // Forward declarations
    struct NetworkMessage;
    class NetworkManager;

    /**
     * @enum ProtocolVersion
     * @brief Network protocol versions
     */
    enum class ProtocolVersion {
        V1_0_0,                 ///< Initial protocol version
        V1_1_0,                 ///< Added compression support
        V1_2_0,                 ///< Added encryption support
        V1_3_0,                 ///< Added authentication
        V1_4_0,                 ///< Added entity synchronization
        V1_5_0,                 ///< Added world streaming
        V1_6_0,                 ///< Added voice chat support
        V1_7_0,                 ///< Added mod support
        V1_8_0,                 ///< Added anti-cheat measures
        Latest = V1_8_0        ///< Latest protocol version
    };

    /**
     * @enum PacketType
     * @brief Network packet types
     */
    enum class PacketType {
        Handshake,              ///< Connection handshake
        Authentication,         ///< Authentication packet
        GameData,               ///< Game state data
        EntityData,             ///< Entity state data
        WorldData,              ///< World state data
        PlayerData,             ///< Player state data
        ChatData,               ///< Chat message data
        CommandData,            ///< Command data
        Heartbeat,              ///< Keep-alive heartbeat
        Acknowledgment,         ///< Message acknowledgment
        Error,                  ///< Error packet
        Custom                  ///< Custom packet type
    };

    /**
     * @enum SerializationFormat
     * @brief Data serialization formats
     */
    enum class SerializationFormat {
        Binary,                 ///< Binary serialization
        JSON,                   ///< JSON serialization
        MessagePack,            ///< MessagePack serialization
        ProtocolBuffers,        ///< Protocol Buffers
        Custom                  ///< Custom format
    };

    /**
     * @enum CompressionAlgorithm
     * @brief Data compression algorithms
     */
    enum class CompressionAlgorithm {
        None,                   ///< No compression
        Zlib,                   ///< Zlib compression
        Gzip,                   ///< Gzip compression
        LZ4,                    ///< LZ4 compression
        Snappy,                 ///< Snappy compression
        Zstandard               ///< Zstandard compression
    };

    /**
     * @enum EncryptionAlgorithm
     * @brief Data encryption algorithms
     */
    enum class EncryptionAlgorithm {
        None,                   ///< No encryption
        AES128,                 ///< AES-128 encryption
        AES256,                 ///< AES-256 encryption
        ChaCha20,               ///< ChaCha20 encryption
        RSA,                    ///< RSA encryption
        ECC                     ///< Elliptic Curve Cryptography
    };

    /**
     * @struct ProtocolHeader
     * @brief Network protocol header
     */
    struct ProtocolHeader {
        uint32_t magicNumber;                      ///< Protocol magic number (0xVC0100)
        ProtocolVersion version;                   ///< Protocol version
        PacketType packetType;                     ///< Packet type
        uint32_t packetSize;                       ///< Packet size (excluding header)
        uint32_t sequenceNumber;                   ///< Sequence number
        uint32_t acknowledgmentNumber;             ///< Acknowledgment number
        uint16_t checksum;                         ///< Packet checksum
        uint8_t flags;                             ///< Protocol flags
        uint8_t reserved;                          ///< Reserved for future use

        ProtocolHeader()
            : magicNumber(0x56433031)              // "VC01" in ASCII
            , version(ProtocolVersion::Latest)
            , packetType(PacketType::GameData)
            , packetSize(0)
            , sequenceNumber(0)
            , acknowledgmentNumber(0)
            , checksum(0)
            , flags(0)
            , reserved(0)
        {}
    };

    /**
     * @struct ProtocolConfig
     * @brief Network protocol configuration
     */
    struct ProtocolConfig {
        // Basic settings
        ProtocolVersion protocolVersion;           ///< Protocol version
        bool enableVersionCheck;                   ///< Enable version checking
        bool strictVersionMatching;                ///< Require exact version match

        // Serialization settings
        SerializationFormat serializationFormat;   ///< Data serialization format
        bool enableCompression;                    ///< Enable data compression
        CompressionAlgorithm compressionAlgorithm; ///< Compression algorithm
        int compressionLevel;                      ///< Compression level (0-9)

        // Security settings
        bool enableEncryption;                     ///< Enable data encryption
        EncryptionAlgorithm encryptionAlgorithm;   ///< Encryption algorithm
        std::string encryptionKey;                 ///< Encryption key
        bool enableIntegrityCheck;                 ///< Enable integrity checking

        // Performance settings
        uint32_t maxPacketSize;                    ///< Maximum packet size
        uint32_t maxFragmentSize;                  ///< Maximum fragment size
        bool enableFragmentation;                  ///< Enable packet fragmentation
        bool enableBatching;                       ///< Enable message batching
        uint32_t batchTimeout;                     ///< Batch timeout (ms)

        // Reliability settings
        bool enableReliability;                    ///< Enable reliable delivery
        uint32_t maxRetries;                       ///< Maximum retry attempts
        uint32_t retryTimeout;                     ///< Retry timeout (ms)
        uint32_t connectionTimeout;                ///< Connection timeout (ms)

        // Flow control settings
        bool enableFlowControl;                    ///< Enable flow control
        uint32_t windowSize;                       ///< Sliding window size
        uint32_t maxOutstandingPackets;            ///< Max outstanding packets

        ProtocolConfig()
            : protocolVersion(ProtocolVersion::Latest)
            , enableVersionCheck(true)
            , strictVersionMatching(true)
            , serializationFormat(SerializationFormat::Binary)
            , enableCompression(true)
            , compressionAlgorithm(CompressionAlgorithm::LZ4)
            , compressionLevel(6)
            , enableEncryption(false)
            , encryptionAlgorithm(EncryptionAlgorithm::AES256)
            , enableIntegrityCheck(true)
            , maxPacketSize(4096)
            , maxFragmentSize(1024)
            , enableFragmentation(true)
            , enableBatching(true)
            , batchTimeout(10)
            , enableReliability(true)
            , maxRetries(5)
            , retryTimeout(1000)
            , connectionTimeout(30000)
            , enableFlowControl(true)
            , windowSize(64)
            , maxOutstandingPackets(32)
        {}
    };

    /**
     * @struct ProtocolMetrics
     * @brief Performance metrics for network protocol
     */
    struct ProtocolMetrics {
        // Performance metrics
        uint64_t packetsSent;                      ///< Total packets sent
        uint64_t packetsReceived;                  ///< Total packets received
        uint64_t bytesSent;                        ///< Total bytes sent
        uint64_t bytesReceived;                    ///< Total bytes received

        // Compression metrics
        uint64_t uncompressedBytes;                ///< Total uncompressed bytes
        uint64_t compressedBytes;                  ///< Total compressed bytes
        float averageCompressionRatio;             ///< Average compression ratio

        // Reliability metrics
        uint64_t packetsRetransmitted;             ///< Packets retransmitted
        uint64_t packetsDropped;                   ///< Packets dropped
        uint64_t packetsCorrupted;                 ///< Packets corrupted
        float packetLossRate;                      ///< Packet loss rate (0.0 - 1.0)

        // Fragmentation metrics
        uint64_t packetsFragmented;                ///< Packets fragmented
        uint64_t fragmentsSent;                    ///< Fragments sent
        uint64_t fragmentsReceived;                ///< Fragments received
        uint64_t fragmentsReassembled;             ///< Fragments reassembled

        // Batching metrics
        uint64_t batchesCreated;                   ///< Batches created
        uint64_t batchesSent;                      ///< Batches sent
        uint64_t messagesBatched;                  ///< Messages batched
        float averageBatchSize;                    ///< Average batch size

        // Security metrics
        uint64_t packetsEncrypted;                 ///< Packets encrypted
        uint64_t packetsDecrypted;                 ///< Packets decrypted
        uint64_t integrityChecksPassed;            ///< Integrity checks passed
        uint64_t integrityChecksFailed;            ///< Integrity checks failed

        // Timing metrics
        double averageSerializationTime;           ///< Average serialization time (ms)
        double averageDeserializationTime;         ///< Average deserialization time (ms)
        double averageCompressionTime;             ///< Average compression time (ms)
        double averageDecompressionTime;           ///< Average decompression time (ms)

        // Error metrics
        uint64_t serializationErrors;              ///< Serialization errors
        uint64_t deserializationErrors;            ///< Deserialization errors
        uint64_t compressionErrors;                ///< Compression errors
        uint64_t decompressionErrors;              ///< Decompression errors
        uint64_t protocolErrors;                   ///< Protocol errors
    };

    /**
     * @class MessageSerializer
     * @brief Network message serialization/deserialization
     */
    class MessageSerializer {
    public:
        /**
         * @brief Serialize network message
         * @param message Message to serialize
         * @param format Serialization format
         * @return Serialized data
         */
        static std::vector<uint8_t> Serialize(const NetworkMessage& message,
                                            SerializationFormat format = SerializationFormat::Binary);

        /**
         * @brief Deserialize network message
         * @param data Serialized data
         * @param format Serialization format
         * @return Deserialized message
         */
        static std::optional<NetworkMessage> Deserialize(const std::vector<uint8_t>& data,
                                                        SerializationFormat format = SerializationFormat::Binary);

        /**
         * @brief Calculate serialized size
         * @param message Message to calculate
         * @param format Serialization format
         * @return Size in bytes
         */
        static size_t CalculateSize(const NetworkMessage& message,
                                  SerializationFormat format = SerializationFormat::Binary);

        /**
         * @brief Validate serialized data
         * @param data Data to validate
         * @param format Serialization format
         * @return true if valid, false otherwise
         */
        static bool ValidateData(const std::vector<uint8_t>& data,
                               SerializationFormat format = SerializationFormat::Binary);
    };

    /**
     * @class PacketCompressor
     * @brief Network packet compression/decompression
     */
    class PacketCompressor {
    public:
        /**
         * @brief Compress data
         * @param input Input data
         * @param algorithm Compression algorithm
         * @param level Compression level
         * @return Compressed data
         */
        static std::vector<uint8_t> Compress(const std::vector<uint8_t>& input,
                                           CompressionAlgorithm algorithm = CompressionAlgorithm::LZ4,
                                           int level = 6);

        /**
         * @brief Decompress data
         * @param input Compressed data
         * @param algorithm Compression algorithm
         * @param expectedSize Expected uncompressed size
         * @return Decompressed data
         */
        static std::optional<std::vector<uint8_t>> Decompress(const std::vector<uint8_t>& input,
                                                             CompressionAlgorithm algorithm = CompressionAlgorithm::LZ4,
                                                             size_t expectedSize = 0);

        /**
         * @brief Get compression ratio
         * @param originalSize Original size
         * @param compressedSize Compressed size
         * @return Compression ratio
         */
        static float GetCompressionRatio(size_t originalSize, size_t compressedSize);

        /**
         * @brief Get recommended algorithm
         * @param dataSize Data size
         * @return Recommended algorithm
         */
        static CompressionAlgorithm GetRecommendedAlgorithm(size_t dataSize);
    };

    /**
     * @class PacketEncryptor
     * @brief Network packet encryption/decryption
     */
    class PacketEncryptor {
    public:
        /**
         * @brief Encrypt data
         * @param input Input data
         * @param algorithm Encryption algorithm
         * @param key Encryption key
         * @return Encrypted data
         */
        static std::vector<uint8_t> Encrypt(const std::vector<uint8_t>& input,
                                          EncryptionAlgorithm algorithm = EncryptionAlgorithm::AES256,
                                          const std::string& key = "");

        /**
         * @brief Decrypt data
         * @param input Encrypted data
         * @param algorithm Encryption algorithm
         * @param key Encryption key
         * @return Decrypted data
         */
        static std::optional<std::vector<uint8_t>> Decrypt(const std::vector<uint8_t>& input,
                                                          EncryptionAlgorithm algorithm = EncryptionAlgorithm::AES256,
                                                          const std::string& key = "");

        /**
         * @brief Generate encryption key
         * @param algorithm Encryption algorithm
         * @return Generated key
         */
        static std::string GenerateKey(EncryptionAlgorithm algorithm = EncryptionAlgorithm::AES256);

        /**
         * @brief Validate key
         * @param key Key to validate
         * @param algorithm Encryption algorithm
         * @return true if valid, false otherwise
         */
        static bool ValidateKey(const std::string& key, EncryptionAlgorithm algorithm);
    };

    /**
     * @class PacketFragmenter
     * @brief Network packet fragmentation/reassembly
     */
    class PacketFragmenter {
    public:
        /**
         * @brief Fragment packet
         * @param packetData Packet data
         * @param maxFragmentSize Maximum fragment size
         * @return Vector of fragments
         */
        static std::vector<std::vector<uint8_t>> Fragment(const std::vector<uint8_t>& packetData,
                                                         size_t maxFragmentSize = 1024);

        /**
         * @brief Reassemble fragments
         * @param fragments Packet fragments
         * @return Reassembled packet data
         */
        static std::optional<std::vector<uint8_t>> Reassemble(const std::vector<std::vector<uint8_t>>& fragments);

        /**
         * @brief Check if data is fragmented
         * @param data Data to check
         * @return true if fragmented, false otherwise
         */
        static bool IsFragmented(const std::vector<uint8_t>& data);

        /**
         * @brief Get fragment count
         * @param data Fragmented data
         * @return Fragment count
         */
        static uint32_t GetFragmentCount(const std::vector<uint8_t>& data);
    };

    /**
     * @class NetworkProtocol
     * @brief Main network protocol handler
     *
     * The NetworkProtocol class handles all aspects of network communication
     * including message serialization, packet formatting, compression, encryption,
     * fragmentation, and protocol versioning for the VoxelCraft multiplayer system.
     *
     * Key Features:
     * - Multiple serialization formats (Binary, JSON, MessagePack)
     * - Advanced compression algorithms (LZ4, Zstandard, etc.)
     * - Strong encryption support (AES256, ChaCha20, RSA)
     * - Packet fragmentation and reassembly for large messages
     * - Message batching for performance optimization
     * - Reliable and unreliable delivery channels
     * - Protocol versioning and backward compatibility
     * - Integrity checking and error detection
     * - Performance monitoring and optimization
     *
     * The protocol system is designed to be extensible and efficient,
     * supporting both small-scale LAN games and large-scale dedicated servers.
     */
    class NetworkProtocol {
    public:
        /**
         * @brief Constructor
         * @param config Protocol configuration
         */
        explicit NetworkProtocol(const ProtocolConfig& config);

        /**
         * @brief Destructor
         */
        ~NetworkProtocol();

        /**
         * @brief Deleted copy constructor
         */
        NetworkProtocol(const NetworkProtocol&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        NetworkProtocol& operator=(const NetworkProtocol&) = delete;

        // Protocol lifecycle

        /**
         * @brief Initialize protocol
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Shutdown protocol
         */
        void Shutdown();

        /**
         * @brief Update protocol state
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Get protocol configuration
         * @return Current configuration
         */
        const ProtocolConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set protocol configuration
         * @param config New configuration
         */
        void SetConfig(const ProtocolConfig& config);

        // Message processing

        /**
         * @brief Encode message to packet
         * @param message Message to encode
         * @return Encoded packet data
         */
        std::vector<uint8_t> EncodeMessage(const NetworkMessage& message);

        /**
         * @brief Decode packet to message
         * @param packetData Packet data
         * @return Decoded message or nullptr if failed
         */
        std::optional<NetworkMessage> DecodePacket(const std::vector<uint8_t>& packetData);

        /**
         * @brief Process incoming packet
         * @param packetData Packet data
         * @param senderId Sender ID
         * @return Processed messages
         */
        std::vector<NetworkMessage> ProcessIncomingPacket(const std::vector<uint8_t>& packetData,
                                                         uint32_t senderId);

        /**
         * @brief Process outgoing message
         * @param message Message to process
         * @return Processed packet data
         */
        std::vector<uint8_t> ProcessOutgoingMessage(const NetworkMessage& message);

        // Batch processing

        /**
         * @brief Add message to batch
         * @param message Message to batch
         * @return true if added, false otherwise
         */
        bool AddToBatch(const NetworkMessage& message);

        /**
         * @brief Process message batch
         * @return Batched packet data
         */
        std::optional<std::vector<uint8_t>> ProcessBatch();

        /**
         * @brief Clear current batch
         */
        void ClearBatch();

        /**
         * @brief Get batch size
         * @return Number of messages in batch
         */
        size_t GetBatchSize() const { return m_messageBatch.size(); }

        // Fragmentation handling

        /**
         * @brief Handle incoming fragment
         * @param fragmentData Fragment data
         * @param senderId Sender ID
         * @return Complete packet data if all fragments received, empty optional otherwise
         */
        std::optional<std::vector<uint8_t>> HandleFragment(const std::vector<uint8_t>& fragmentData,
                                                          uint32_t senderId);

        /**
         * @brief Clean up old fragments
         * @param maxAge Maximum fragment age (seconds)
         */
        void CleanupFragments(double maxAge = 30.0);

        // Protocol utilities

        /**
         * @brief Check protocol compatibility
         * @param version Protocol version to check
         * @return true if compatible, false otherwise
         */
        bool IsCompatible(ProtocolVersion version) const;

        /**
         * @brief Get protocol version string
         * @param version Protocol version
         * @return Version string
         */
        static std::string GetVersionString(ProtocolVersion version);

        /**
         * @brief Parse protocol version from string
         * @param versionString Version string
         * @return Protocol version or nullopt if invalid
         */
        static std::optional<ProtocolVersion> ParseVersionString(const std::string& versionString);

        /**
         * @brief Calculate packet checksum
         * @param data Packet data
         * @return Checksum value
         */
        static uint16_t CalculateChecksum(const std::vector<uint8_t>& data);

        /**
         * @brief Validate packet checksum
         * @param data Packet data
         * @param expectedChecksum Expected checksum
         * @return true if valid, false otherwise
         */
        static bool ValidateChecksum(const std::vector<uint8_t>& data, uint16_t expectedChecksum);

        // Metrics and monitoring

        /**
         * @brief Get protocol metrics
         * @return Performance metrics
         */
        const ProtocolMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

        /**
         * @brief Get performance report
         * @return Detailed performance report
         */
        std::string GetPerformanceReport() const;

        // Error handling

        /**
         * @brief Get last error message
         * @return Error message
         */
        const std::string& GetLastError() const { return m_lastError; }

        /**
         * @brief Clear last error
         */
        void ClearLastError() { m_lastError.clear(); }

        // Utility functions

        /**
         * @brief Validate protocol state
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Get protocol status report
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

        /**
         * @brief Optimize protocol performance
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> Optimize();

    private:
        /**
         * @brief Initialize protocol components
         * @return true if successful, false otherwise
         */
        bool InitializeComponents();

        /**
         * @brief Create protocol header
         * @param message Message to create header for
         * @return Protocol header
         */
        ProtocolHeader CreateHeader(const NetworkMessage& message) const;

        /**
         * @brief Parse protocol header
         * @param data Data to parse
         * @return Parsed header or nullopt if invalid
         */
        std::optional<ProtocolHeader> ParseHeader(const std::vector<uint8_t>& data) const;

        /**
         * @brief Serialize protocol header
         * @param header Header to serialize
         * @return Serialized header
         */
        std::vector<uint8_t> SerializeHeader(const ProtocolHeader& header) const;

        /**
         * @brief Deserialize protocol header
         * @param data Data to deserialize
         * @return Deserialized header
         */
        ProtocolHeader DeserializeHeader(const std::vector<uint8_t>& data) const;

        /**
         * @brief Compress packet data
         * @param data Data to compress
         * @return Compressed data
         */
        std::vector<uint8_t> CompressData(const std::vector<uint8_t>& data);

        /**
         * @brief Decompress packet data
         * @param data Data to decompress
         * @param expectedSize Expected uncompressed size
         * @return Decompressed data
         */
        std::optional<std::vector<uint8_t>> DecompressData(const std::vector<uint8_t>& data,
                                                          size_t expectedSize);

        /**
         * @brief Encrypt packet data
         * @param data Data to encrypt
         * @return Encrypted data
         */
        std::vector<uint8_t> EncryptData(const std::vector<uint8_t>& data);

        /**
         * @brief Decrypt packet data
         * @param data Data to decrypt
         * @return Decrypted data
         */
        std::optional<std::vector<uint8_t>> DecryptData(const std::vector<uint8_t>& data);

        /**
         * @brief Fragment packet if needed
         * @param packetData Packet data
         * @return Vector of fragments
         */
        std::vector<std::vector<uint8_t>> FragmentPacket(const std::vector<uint8_t>& packetData);

        /**
         * @brief Update performance metrics
         * @param deltaTime Time elapsed
         */
        void UpdateMetrics(double deltaTime);

        /**
         * @brief Handle protocol errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        // Protocol data
        ProtocolConfig m_config;                     ///< Protocol configuration
        ProtocolMetrics m_metrics;                   ///< Performance metrics

        // Message batching
        std::vector<NetworkMessage> m_messageBatch;  ///< Current message batch
        double m_batchStartTime;                     ///< Batch start time
        mutable std::shared_mutex m_batchMutex;      ///< Batch synchronization

        // Fragmentation handling
        struct FragmentData {
            uint32_t packetId;                       ///< Packet ID
            uint32_t totalFragments;                 ///< Total fragments
            std::vector<std::vector<uint8_t>> fragments; ///< Received fragments
            double lastUpdateTime;                   ///< Last update time
        };

        std::unordered_map<uint32_t, FragmentData> m_fragmentedPackets; ///< Fragmented packets
        mutable std::shared_mutex m_fragmentsMutex;  ///< Fragments synchronization

        // Sequence tracking
        std::atomic<uint32_t> m_nextSequenceNumber;  ///< Next sequence number
        std::atomic<uint32_t> m_nextAcknowledgmentNumber; ///< Next acknowledgment number

        // Protocol state
        bool m_isInitialized;                         ///< Protocol is initialized
        std::string m_lastError;                      ///< Last error message
        double m_lastUpdateTime;                      ///< Last update time

        // Constants
        static constexpr uint32_t PROTOCOL_MAGIC = 0x56433031; ///< "VC01" magic number
        static constexpr size_t HEADER_SIZE = sizeof(ProtocolHeader); ///< Header size
        static constexpr size_t MAX_MESSAGE_SIZE = 1024 * 1024; ///< Max message size (1MB)
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_MULTIPLAYER_NETWORK_PROTOCOL_HPP
