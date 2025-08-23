#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <chrono>
#include <cstdint>

namespace VoxelCraft {

    /**
     * @enum PacketType
     * @brief Tipos de paquetes de red
     */
    enum class PacketType : uint16_t {
        // Conexión y autenticación
        HANDSHAKE = 0,
        LOGIN_START,
        LOGIN_SUCCESS,
        LOGIN_FAILURE,
        DISCONNECT,

        // Estado del juego
        KEEP_ALIVE,
        PING,
        PONG,

        // Jugador
        PLAYER_POSITION,
        PLAYER_ROTATION,
        PLAYER_POSITION_AND_ROTATION,
        PLAYER_ANIMATION,
        PLAYER_ACTION,

        // Entidades
        SPAWN_ENTITY,
        DESTROY_ENTITY,
        ENTITY_POSITION,
        ENTITY_ROTATION,
        ENTITY_POSITION_AND_ROTATION,
        ENTITY_METADATA,
        ENTITY_ANIMATION,
        ENTITY_EFFECT,

        // Mundo
        CHUNK_DATA,
        CHUNK_UNLOAD,
        BLOCK_CHANGE,
        MULTI_BLOCK_CHANGE,
        WORLD_TIME,
        WEATHER_UPDATE,

        // Inventario
        INVENTORY_OPEN,
        INVENTORY_CLOSE,
        INVENTORY_CONTENT,
        INVENTORY_SLOT_UPDATE,
        INVENTORY_TRANSACTION,

        // Chat
        CHAT_MESSAGE,
        CHAT_COMMAND,

        // Interacciones
        PLAYER_DIG,
        PLAYER_PLACE_BLOCK,
        PLAYER_USE_ITEM,
        ENTITY_INTERACTION,

        // Partículas
        SPAWN_PARTICLE,

        // Sonido
        SOUND_EFFECT,
        NAMED_SOUND,

        // Sistema
        PLUGIN_MESSAGE,
        CUSTOM_PAYLOAD,

        // Estados del servidor
        SERVER_STATUS,
        SERVER_LIST_PING,
        SERVER_DIFFICULTY,
        SERVER_GAMEMODE,

        // Debug y diagnóstico
        DEBUG_PACKET,

        // Total de tipos
        MAX_PACKET_TYPES
    };

    /**
     * @enum PacketPriority
     * @brief Prioridades de paquetes
     */
    enum class PacketPriority {
        LOW = 0,        ///< Paquetes de baja prioridad (stats, etc.)
        NORMAL,         ///< Prioridad normal
        HIGH,           ///< Alta prioridad (movimiento, acciones)
        CRITICAL        ///< Crítica (conexión, autenticación)
    };

    /**
     * @struct PacketHeader
     * @brief Cabecera de paquete
     */
    struct PacketHeader {
        uint32_t magic = 0xDEADBEEF;           ///< Número mágico para validación
        uint16_t packetType;                   ///< Tipo de paquete
        uint32_t packetSize;                   ///< Tamaño total del paquete
        uint32_t sequenceNumber;               ///< Número de secuencia
        uint32_t ackNumber;                    ///< Número de ACK
        uint16_t flags;                        ///< Flags del paquete
        uint32_t connectionId;                 ///< ID de conexión
        std::chrono::steady_clock::time_point timestamp; ///< Timestamp

        // Flags
        static const uint16_t FLAG_COMPRESSED = 0x0001;
        static const uint16_t FLAG_ENCRYPTED = 0x0002;
        static const uint16_t FLAG_RELIABLE = 0x0004;
        static const uint16_t FLAG_ORDERED = 0x0008;
        static const uint16_t FLAG_FRAGMENTED = 0x0010;
        static const uint16_t FLAG_ACK_REQUESTED = 0x0020;

        bool IsCompressed() const { return flags & FLAG_COMPRESSED; }
        bool IsEncrypted() const { return flags & FLAG_ENCRYPTED; }
        bool IsReliable() const { return flags & FLAG_RELIABLE; }
        bool IsOrdered() const { return flags & FLAG_ORDERED; }
        bool IsFragmented() const { return flags & FLAG_FRAGMENTED; }
        bool AckRequested() const { return flags & FLAG_ACK_REQUESTED; }

        void SetCompressed(bool value) {
            if (value) flags |= FLAG_COMPRESSED;
            else flags &= ~FLAG_COMPRESSED;
        }

        void SetEncrypted(bool value) {
            if (value) flags |= FLAG_ENCRYPTED;
            else flags &= ~FLAG_ENCRYPTED;
        }

        void SetReliable(bool value) {
            if (value) flags |= FLAG_RELIABLE;
            else flags &= ~FLAG_RELIABLE;
        }

        void SetOrdered(bool value) {
            if (value) flags |= FLAG_ORDERED;
            else flags &= ~FLAG_ORDERED;
        }

        void SetFragmented(bool value) {
            if (value) flags |= FLAG_FRAGMENTED;
            else flags &= ~FLAG_FRAGMENTED;
        }

        void SetAckRequested(bool value) {
            if (value) flags |= FLAG_ACK_REQUESTED;
            else flags &= ~FLAG_ACK_REQUESTED;
        }
    };

    /**
     * @class Packet
     * @brief Clase base para todos los paquetes de red
     */
    class Packet {
    public:
        /**
         * @brief Constructor
         */
        Packet(PacketType type = PacketType::DEBUG_PACKET, PacketPriority priority = PacketPriority::NORMAL);

        /**
         * @brief Destructor virtual
         */
        virtual ~Packet() = default;

        // Getters
        PacketType GetType() const { return m_header.packetType; }
        PacketPriority GetPriority() const { return m_priority; }
        const PacketHeader& GetHeader() const { return m_header; }
        PacketHeader& GetHeader() { return m_header; }
        size_t GetSize() const { return m_data.size(); }
        const std::vector<uint8_t>& GetData() const { return m_data; }
        bool IsReliable() const { return m_header.IsReliable(); }
        bool IsOrdered() const { return m_header.IsOrdered(); }
        uint32_t GetSequenceNumber() const { return m_header.sequenceNumber; }
        uint32_t GetConnectionId() const { return m_header.connectionId; }

        // Setters
        void SetPriority(PacketPriority priority) { m_priority = priority; }
        void SetConnectionId(uint32_t connectionId) { m_header.connectionId = connectionId; }
        void SetSequenceNumber(uint32_t sequence) { m_header.sequenceNumber = sequence; }
        void SetAckNumber(uint32_t ack) { m_header.ackNumber = ack; }
        void SetReliable(bool reliable) { m_header.SetReliable(reliable); }
        void SetOrdered(bool ordered) { m_header.SetOrdered(ordered); }
        void SetCompressed(bool compressed) { m_header.SetCompressed(compressed); }
        void SetEncrypted(bool encrypted) { m_header.SetEncrypted(encrypted); }
        void SetAckRequested(bool requested) { m_header.SetAckRequested(requested); }

        // Serialización
        virtual bool Serialize();
        virtual bool Deserialize(const std::vector<uint8_t>& data);
        virtual std::vector<uint8_t> ToBytes() const;
        static std::shared_ptr<Packet> FromBytes(const std::vector<uint8_t>& data);

        // Utilidades
        virtual std::string GetName() const;
        virtual std::string ToString() const;
        bool IsValid() const;

        // Compresión y encriptación
        bool Compress();
        bool Decompress();
        bool Encrypt(const std::string& key);
        bool Decrypt(const std::string& key);

    protected:
        PacketHeader m_header;
        PacketPriority m_priority;
        std::vector<uint8_t> m_data;
        std::vector<uint8_t> m_originalData; // Para descompresión/desencriptación

        // Métodos virtuales para subclases
        virtual void WriteData() = 0;
        virtual void ReadData() = 0;

        // Utilidades de serialización
        void WriteInt8(int8_t value);
        void WriteInt16(int16_t value);
        void WriteInt32(int32_t value);
        void WriteInt64(int64_t value);
        void WriteUInt8(uint8_t value);
        void WriteUInt16(uint16_t value);
        void WriteUInt32(uint32_t value);
        void WriteUInt64(uint64_t value);
        void WriteFloat(float value);
        void WriteDouble(double value);
        void WriteString(const std::string& value);
        void WriteBytes(const std::vector<uint8_t>& value);
        void WriteBool(bool value);

        int8_t ReadInt8();
        int16_t ReadInt16();
        int32_t ReadInt32();
        int64_t ReadInt64();
        uint8_t ReadUInt8();
        uint16_t ReadUInt16();
        uint32_t ReadUInt32();
        uint64_t ReadUInt64();
        float ReadFloat();
        double ReadDouble();
        std::string ReadString();
        std::vector<uint8_t> ReadBytes(size_t length);
        bool ReadBool();

    private:
        size_t m_readPosition = 0;

        // Utilidades internas
        bool ValidateHeader() const;
        void UpdateHeader();
        static PacketType GetPacketTypeFromId(uint16_t id);
        static uint16_t GetPacketIdFromType(PacketType type);
    };

    /**
     * @class HandshakePacket
     * @brief Paquete de handshake inicial
     */
    class HandshakePacket : public Packet {
    public:
        HandshakePacket();
        HandshakePacket(int32_t protocolVersion, const std::string& serverAddress, uint16_t serverPort, int32_t nextState);

        int32_t GetProtocolVersion() const { return m_protocolVersion; }
        std::string GetServerAddress() const { return m_serverAddress; }
        uint16_t GetServerPort() const { return m_serverPort; }
        int32_t GetNextState() const { return m_nextState; }

    protected:
        void WriteData() override;
        void ReadData() override;

    private:
        int32_t m_protocolVersion;
        std::string m_serverAddress;
        uint16_t m_serverPort;
        int32_t m_nextState;
    };

    /**
     * @class PlayerPositionPacket
     * @brief Paquete de posición del jugador
     */
    class PlayerPositionPacket : public Packet {
    public:
        PlayerPositionPacket();
        PlayerPositionPacket(double x, double y, double z, bool onGround);

        double GetX() const { return m_x; }
        double GetY() const { return m_y; }
        double GetZ() const { return m_z; }
        bool IsOnGround() const { return m_onGround; }

    protected:
        void WriteData() override;
        void ReadData() override;

    private:
        double m_x, m_y, m_z;
        bool m_onGround;
    };

    /**
     * @class PlayerPositionAndRotationPacket
     * @brief Paquete de posición y rotación del jugador
     */
    class PlayerPositionAndRotationPacket : public Packet {
    public:
        PlayerPositionAndRotationPacket();
        PlayerPositionAndRotationPacket(double x, double y, double z, float yaw, float pitch, bool onGround);

        double GetX() const { return m_x; }
        double GetY() const { return m_y; }
        double GetZ() const { return m_z; }
        float GetYaw() const { return m_yaw; }
        float GetPitch() const { return m_pitch; }
        bool IsOnGround() const { return m_onGround; }

    protected:
        void WriteData() override;
        void ReadData() override;

    private:
        double m_x, m_y, m_z;
        float m_yaw, m_pitch;
        bool m_onGround;
    };

    /**
     * @class ChatMessagePacket
     * @brief Paquete de mensaje de chat
     */
    class ChatMessagePacket : public Packet {
    public:
        ChatMessagePacket();
        ChatMessagePacket(const std::string& message, int8_t position = 0);

        std::string GetMessage() const { return m_message; }
        int8_t GetPosition() const { return m_position; }

    protected:
        void WriteData() override;
        void ReadData() override;

    private:
        std::string m_message;
        int8_t m_position;
    };

    /**
     * @class BlockChangePacket
     * @brief Paquete de cambio de bloque
     */
    class BlockChangePacket : public Packet {
    public:
        BlockChangePacket();
        BlockChangePacket(int32_t x, int32_t y, int32_t z, uint32_t blockId);

        int32_t GetX() const { return m_x; }
        int32_t GetY() const { return m_y; }
        int32_t GetZ() const { return m_z; }
        uint32_t GetBlockId() const { return m_blockId; }

    protected:
        void WriteData() override;
        void ReadData() override;

    private:
        int32_t m_x, m_y, m_z;
        uint32_t m_blockId;
    };

    /**
     * @class ChunkDataPacket
     * @brief Paquete de datos de chunk
     */
    class ChunkDataPacket : public Packet {
    public:
        ChunkDataPacket();
        ChunkDataPacket(int32_t chunkX, int32_t chunkZ, bool fullChunk, uint16_t primaryBitMask,
                       const std::vector<uint8_t>& data);

        int32_t GetChunkX() const { return m_chunkX; }
        int32_t GetChunkZ() const { return m_chunkZ; }
        bool IsFullChunk() const { return m_fullChunk; }
        uint16_t GetPrimaryBitMask() const { return m_primaryBitMask; }
        const std::vector<uint8_t>& GetData() const { return m_data; }

    protected:
        void WriteData() override;
        void ReadData() override;

    private:
        int32_t m_chunkX, m_chunkZ;
        bool m_fullChunk;
        uint16_t m_primaryBitMask;
        std::vector<uint8_t> m_data;
    };

    /**
     * @class PacketFactory
     * @brief Factory para crear paquetes
     */
    class PacketFactory {
    public:
        static std::shared_ptr<Packet> CreatePacket(PacketType type);
        static std::shared_ptr<Packet> CreatePacketFromData(const std::vector<uint8_t>& data);
        static void RegisterPacketType(PacketType type, std::function<std::shared_ptr<Packet>()> creator);

    private:
        static std::unordered_map<PacketType, std::function<std::shared_ptr<Packet>()>> s_creators;
    };

    // Macros para registrar tipos de paquetes
    #define REGISTER_PACKET_TYPE(TYPE, CLASS) \
        static bool _registered_##CLASS = []() { \
            PacketFactory::RegisterPacketType(TYPE, []() { return std::make_shared<CLASS>(); }); \
            return true; \
        }();

} // namespace VoxelCraft
