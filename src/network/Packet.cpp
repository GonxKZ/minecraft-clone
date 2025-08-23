#include "Packet.hpp"
#include <cstring>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace VoxelCraft {

    // Packet implementation
    Packet::Packet(PacketType type, PacketPriority priority)
        : m_priority(priority) {
        m_header.packetType = static_cast<uint16_t>(type);
        m_header.timestamp = std::chrono::steady_clock::now();
        m_header.flags = 0;
    }

    bool Packet::Serialize() {
        m_data.clear();
        m_readPosition = 0;

        WriteData();
        UpdateHeader();

        return true;
    }

    bool Packet::Deserialize(const std::vector<uint8_t>& data) {
        m_data = data;
        m_originalData = data;
        m_readPosition = sizeof(PacketHeader);

        if (!ValidateHeader()) {
            return false;
        }

        ReadData();
        return true;
    }

    std::vector<uint8_t> Packet::ToBytes() const {
        std::vector<uint8_t> bytes;

        // Serializar header
        const uint8_t* headerBytes = reinterpret_cast<const uint8_t*>(&m_header);
        bytes.insert(bytes.end(), headerBytes, headerBytes + sizeof(PacketHeader));

        // Serializar data
        bytes.insert(bytes.end(), m_data.begin(), m_data.end());

        return bytes;
    }

    std::shared_ptr<Packet> Packet::FromBytes(const std::vector<uint8_t>& data) {
        if (data.size() < sizeof(PacketHeader)) {
            return nullptr;
        }

        // Deserializar header
        PacketHeader header;
        std::memcpy(&header, data.data(), sizeof(PacketHeader));

        if (!header.magic == 0xDEADBEEF) {
            return nullptr;
        }

        // Crear paquete del tipo correcto
        auto packet = PacketFactory::CreatePacket(static_cast<PacketType>(header.packetType));
        if (!packet) {
            return nullptr;
        }

        // Deserializar
        if (!packet->Deserialize(data)) {
            return nullptr;
        }

        return packet;
    }

    std::string Packet::GetName() const {
        switch (GetType()) {
            case PacketType::HANDSHAKE: return "Handshake";
            case PacketType::LOGIN_START: return "Login Start";
            case PacketType::LOGIN_SUCCESS: return "Login Success";
            case PacketType::LOGIN_FAILURE: return "Login Failure";
            case PacketType::DISCONNECT: return "Disconnect";
            case PacketType::KEEP_ALIVE: return "Keep Alive";
            case PacketType::PING: return "Ping";
            case PacketType::PONG: return "Pong";
            case PacketType::PLAYER_POSITION: return "Player Position";
            case PacketType::PLAYER_ROTATION: return "Player Rotation";
            case PacketType::PLAYER_POSITION_AND_ROTATION: return "Player Position and Rotation";
            case PacketType::PLAYER_ANIMATION: return "Player Animation";
            case PacketType::PLAYER_ACTION: return "Player Action";
            case PacketType::SPAWN_ENTITY: return "Spawn Entity";
            case PacketType::DESTROY_ENTITY: return "Destroy Entity";
            case PacketType::ENTITY_POSITION: return "Entity Position";
            case PacketType::ENTITY_ROTATION: return "Entity Rotation";
            case PacketType::ENTITY_POSITION_AND_ROTATION: return "Entity Position and Rotation";
            case PacketType::ENTITY_METADATA: return "Entity Metadata";
            case PacketType::ENTITY_ANIMATION: return "Entity Animation";
            case PacketType::ENTITY_EFFECT: return "Entity Effect";
            case PacketType::CHUNK_DATA: return "Chunk Data";
            case PacketType::CHUNK_UNLOAD: return "Chunk Unload";
            case PacketType::BLOCK_CHANGE: return "Block Change";
            case PacketType::MULTI_BLOCK_CHANGE: return "Multi Block Change";
            case PacketType::WORLD_TIME: return "World Time";
            case PacketType::WEATHER_UPDATE: return "Weather Update";
            case PacketType::INVENTORY_OPEN: return "Inventory Open";
            case PacketType::INVENTORY_CLOSE: return "Inventory Close";
            case PacketType::INVENTORY_CONTENT: return "Inventory Content";
            case PacketType::INVENTORY_SLOT_UPDATE: return "Inventory Slot Update";
            case PacketType::INVENTORY_TRANSACTION: return "Inventory Transaction";
            case PacketType::CHAT_MESSAGE: return "Chat Message";
            case PacketType::CHAT_COMMAND: return "Chat Command";
            case PacketType::PLAYER_DIG: return "Player Dig";
            case PacketType::PLAYER_PLACE_BLOCK: return "Player Place Block";
            case PacketType::PLAYER_USE_ITEM: return "Player Use Item";
            case PacketType::ENTITY_INTERACTION: return "Entity Interaction";
            case PacketType::SPAWN_PARTICLE: return "Spawn Particle";
            case PacketType::SOUND_EFFECT: return "Sound Effect";
            case PacketType::NAMED_SOUND: return "Named Sound";
            case PacketType::PLUGIN_MESSAGE: return "Plugin Message";
            case PacketType::CUSTOM_PAYLOAD: return "Custom Payload";
            case PacketType::SERVER_STATUS: return "Server Status";
            case PacketType::SERVER_LIST_PING: return "Server List Ping";
            case PacketType::SERVER_DIFFICULTY: return "Server Difficulty";
            case PacketType::SERVER_GAMEMODE: return "Server Gamemode";
            case PacketType::DEBUG_PACKET: return "Debug Packet";
            default: return "Unknown Packet";
        }
    }

    std::string Packet::ToString() const {
        std::stringstream ss;
        ss << GetName() << " [Type: " << static_cast<int>(GetType())
           << ", Size: " << GetSize() << " bytes, Seq: " << GetSequenceNumber()
           << ", ConnID: " << GetConnectionId() << "]";
        return ss.str();
    }

    bool Packet::IsValid() const {
        return ValidateHeader();
    }

    bool Packet::Compress() {
        if (m_header.IsCompressed()) {
            return true; // Already compressed
        }

        // Simple RLE compression for demo purposes
        // In real implementation, use zlib or similar
        std::vector<uint8_t> compressedData;
        size_t i = 0;

        while (i < m_data.size()) {
            uint8_t current = m_data[i];
            size_t count = 1;

            while (i + count < m_data.size() && m_data[i + count] == current && count < 255) {
                count++;
            }

            compressedData.push_back(current);
            compressedData.push_back(static_cast<uint8_t>(count));
            i += count;
        }

        if (compressedData.size() < m_data.size()) {
            m_originalData = m_data;
            m_data = compressedData;
            m_header.SetCompressed(true);
            UpdateHeader();
            return true;
        }

        return false;
    }

    bool Packet::Decompress() {
        if (!m_header.IsCompressed()) {
            return true; // Not compressed
        }

        // Simple RLE decompression
        std::vector<uint8_t> decompressedData;

        for (size_t i = 0; i < m_data.size(); i += 2) {
            if (i + 1 < m_data.size()) {
                uint8_t value = m_data[i];
                uint8_t count = m_data[i + 1];

                for (uint8_t j = 0; j < count; ++j) {
                    decompressedData.push_back(value);
                }
            }
        }

        m_data = decompressedData;
        m_header.SetCompressed(false);
        UpdateHeader();
        return true;
    }

    bool Packet::Encrypt(const std::string& key) {
        if (m_header.IsEncrypted()) {
            return true; // Already encrypted
        }

        // Simple XOR encryption for demo purposes
        // In real implementation, use AES or similar
        if (!key.empty()) {
            for (size_t i = 0; i < m_data.size(); ++i) {
                m_data[i] ^= key[i % key.size()];
            }
            m_header.SetEncrypted(true);
            UpdateHeader();
            return true;
        }

        return false;
    }

    bool Packet::Decrypt(const std::string& key) {
        if (!m_header.IsEncrypted()) {
            return true; // Not encrypted
        }

        // XOR decryption (same as encryption)
        if (!key.empty()) {
            for (size_t i = 0; i < m_data.size(); ++i) {
                m_data[i] ^= key[i % key.size()];
            }
            m_header.SetEncrypted(false);
            UpdateHeader();
            return true;
        }

        return false;
    }

    // Write methods
    void Packet::WriteInt8(int8_t value) {
        m_data.push_back(static_cast<uint8_t>(value));
    }

    void Packet::WriteInt16(int16_t value) {
        uint16_t networkValue = htons(static_cast<uint16_t>(value));
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&networkValue);
        m_data.insert(m_data.end(), bytes, bytes + sizeof(uint16_t));
    }

    void Packet::WriteInt32(int32_t value) {
        uint32_t networkValue = htonl(static_cast<uint32_t>(value));
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&networkValue);
        m_data.insert(m_data.end(), bytes, bytes + sizeof(uint32_t));
    }

    void Packet::WriteInt64(int64_t value) {
        uint64_t networkValue = htobe64(static_cast<uint64_t>(value));
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&networkValue);
        m_data.insert(m_data.end(), bytes, bytes + sizeof(uint64_t));
    }

    void Packet::WriteUInt8(uint8_t value) {
        m_data.push_back(value);
    }

    void Packet::WriteUInt16(uint16_t value) {
        uint16_t networkValue = htons(value);
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&networkValue);
        m_data.insert(m_data.end(), bytes, bytes + sizeof(uint16_t));
    }

    void Packet::WriteUInt32(uint32_t value) {
        uint32_t networkValue = htonl(value);
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&networkValue);
        m_data.insert(m_data.end(), bytes, bytes + sizeof(uint32_t));
    }

    void Packet::WriteUInt64(uint64_t value) {
        uint64_t networkValue = htobe64(value);
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&networkValue);
        m_data.insert(m_data.end(), bytes, bytes + sizeof(uint64_t));
    }

    void Packet::WriteFloat(float value) {
        uint32_t intValue;
        std::memcpy(&intValue, &value, sizeof(float));
        WriteUInt32(intValue);
    }

    void Packet::WriteDouble(double value) {
        uint64_t intValue;
        std::memcpy(&intValue, &value, sizeof(double));
        WriteUInt64(intValue);
    }

    void Packet::WriteString(const std::string& value) {
        WriteUInt16(static_cast<uint16_t>(value.length()));
        m_data.insert(m_data.end(), value.begin(), value.end());
    }

    void Packet::WriteBytes(const std::vector<uint8_t>& value) {
        WriteUInt32(static_cast<uint32_t>(value.size()));
        m_data.insert(m_data.end(), value.begin(), value.end());
    }

    void Packet::WriteBool(bool value) {
        m_data.push_back(value ? 1 : 0);
    }

    // Read methods
    int8_t Packet::ReadInt8() {
        if (m_readPosition >= m_data.size()) return 0;
        return static_cast<int8_t>(m_data[m_readPosition++]);
    }

    int16_t Packet::ReadInt16() {
        if (m_readPosition + sizeof(int16_t) > m_data.size()) return 0;
        uint16_t networkValue;
        std::memcpy(&networkValue, &m_data[m_readPosition], sizeof(uint16_t));
        m_readPosition += sizeof(uint16_t);
        return static_cast<int16_t>(ntohs(networkValue));
    }

    int32_t Packet::ReadInt32() {
        if (m_readPosition + sizeof(int32_t) > m_data.size()) return 0;
        uint32_t networkValue;
        std::memcpy(&networkValue, &m_data[m_readPosition], sizeof(uint32_t));
        m_readPosition += sizeof(uint32_t);
        return static_cast<int32_t>(ntohl(networkValue));
    }

    int64_t Packet::ReadInt64() {
        if (m_readPosition + sizeof(int64_t) > m_data.size()) return 0;
        uint64_t networkValue;
        std::memcpy(&networkValue, &m_data[m_readPosition], sizeof(uint64_t));
        m_readPosition += sizeof(uint64_t);
        return static_cast<int64_t>(be64toh(networkValue));
    }

    uint8_t Packet::ReadUInt8() {
        if (m_readPosition >= m_data.size()) return 0;
        return m_data[m_readPosition++];
    }

    uint16_t Packet::ReadUInt16() {
        if (m_readPosition + sizeof(uint16_t) > m_data.size()) return 0;
        uint16_t networkValue;
        std::memcpy(&networkValue, &m_data[m_readPosition], sizeof(uint16_t));
        m_readPosition += sizeof(uint16_t);
        return ntohs(networkValue);
    }

    uint32_t Packet::ReadUInt32() {
        if (m_readPosition + sizeof(uint32_t) > m_data.size()) return 0;
        uint32_t networkValue;
        std::memcpy(&networkValue, &m_data[m_readPosition], sizeof(uint32_t));
        m_readPosition += sizeof(uint32_t);
        return ntohl(networkValue);
    }

    uint64_t Packet::ReadUInt64() {
        if (m_readPosition + sizeof(uint64_t) > m_data.size()) return 0;
        uint64_t networkValue;
        std::memcpy(&networkValue, &m_data[m_readPosition], sizeof(uint64_t));
        m_readPosition += sizeof(uint64_t);
        return be64toh(networkValue);
    }

    float Packet::ReadFloat() {
        uint32_t intValue = ReadUInt32();
        float floatValue;
        std::memcpy(&floatValue, &intValue, sizeof(float));
        return floatValue;
    }

    double Packet::ReadDouble() {
        uint64_t intValue = ReadUInt64();
        double doubleValue;
        std::memcpy(&doubleValue, &intValue, sizeof(double));
        return doubleValue;
    }

    std::string Packet::ReadString() {
        uint16_t length = ReadUInt16();
        if (m_readPosition + length > m_data.size()) {
            m_readPosition += length;
            return "";
        }

        std::string result(m_data.begin() + m_readPosition,
                          m_data.begin() + m_readPosition + length);
        m_readPosition += length;
        return result;
    }

    std::vector<uint8_t> Packet::ReadBytes(size_t length) {
        if (m_readPosition + length > m_data.size()) {
            length = m_data.size() - m_readPosition;
        }

        std::vector<uint8_t> result(m_data.begin() + m_readPosition,
                                   m_data.begin() + m_readPosition + length);
        m_readPosition += length;
        return result;
    }

    bool Packet::ReadBool() {
        return ReadUInt8() != 0;
    }

    bool Packet::ValidateHeader() const {
        if (m_data.size() < sizeof(PacketHeader)) {
            return false;
        }

        return m_header.magic == 0xDEADBEEF &&
               m_header.packetType < static_cast<uint16_t>(PacketType::MAX_PACKET_TYPES) &&
               m_header.packetSize == m_data.size();
    }

    void Packet::UpdateHeader() {
        m_header.packetSize = static_cast<uint32_t>(m_data.size() + sizeof(PacketHeader));
        m_header.timestamp = std::chrono::steady_clock::now();
    }

    // Specific packet implementations

    // HandshakePacket
    HandshakePacket::HandshakePacket()
        : Packet(PacketType::HANDSHAKE, PacketPriority::CRITICAL) {
    }

    HandshakePacket::HandshakePacket(int32_t protocolVersion, const std::string& serverAddress,
                                   uint16_t serverPort, int32_t nextState)
        : Packet(PacketType::HANDSHAKE, PacketPriority::CRITICAL),
          m_protocolVersion(protocolVersion), m_serverAddress(serverAddress),
          m_serverPort(serverPort), m_nextState(nextState) {
    }

    void HandshakePacket::WriteData() {
        WriteInt32(m_protocolVersion);
        WriteString(m_serverAddress);
        WriteUInt16(m_serverPort);
        WriteInt32(m_nextState);
    }

    void HandshakePacket::ReadData() {
        m_protocolVersion = ReadInt32();
        m_serverAddress = ReadString();
        m_serverPort = ReadUInt16();
        m_nextState = ReadInt32();
    }

    // PlayerPositionPacket
    PlayerPositionPacket::PlayerPositionPacket()
        : Packet(PacketType::PLAYER_POSITION, PacketPriority::HIGH) {
    }

    PlayerPositionPacket::PlayerPositionPacket(double x, double y, double z, bool onGround)
        : Packet(PacketType::PLAYER_POSITION, PacketPriority::HIGH),
          m_x(x), m_y(y), m_z(z), m_onGround(onGround) {
        SetReliable(true);
    }

    void PlayerPositionPacket::WriteData() {
        WriteDouble(m_x);
        WriteDouble(m_y);
        WriteDouble(m_z);
        WriteBool(m_onGround);
    }

    void PlayerPositionPacket::ReadData() {
        m_x = ReadDouble();
        m_y = ReadDouble();
        m_z = ReadDouble();
        m_onGround = ReadBool();
    }

    // PlayerPositionAndRotationPacket
    PlayerPositionAndRotationPacket::PlayerPositionAndRotationPacket()
        : Packet(PacketType::PLAYER_POSITION_AND_ROTATION, PacketPriority::HIGH) {
    }

    PlayerPositionAndRotationPacket::PlayerPositionAndRotationPacket(double x, double y, double z,
                                                                   float yaw, float pitch, bool onGround)
        : Packet(PacketType::PLAYER_POSITION_AND_ROTATION, PacketPriority::HIGH),
          m_x(x), m_y(y), m_z(z), m_yaw(yaw), m_pitch(pitch), m_onGround(onGround) {
        SetReliable(true);
    }

    void PlayerPositionAndRotationPacket::WriteData() {
        WriteDouble(m_x);
        WriteDouble(m_y);
        WriteDouble(m_z);
        WriteFloat(m_yaw);
        WriteFloat(m_pitch);
        WriteBool(m_onGround);
    }

    void PlayerPositionAndRotationPacket::ReadData() {
        m_x = ReadDouble();
        m_y = ReadDouble();
        m_z = ReadDouble();
        m_yaw = ReadFloat();
        m_pitch = ReadFloat();
        m_onGround = ReadBool();
    }

    // ChatMessagePacket
    ChatMessagePacket::ChatMessagePacket()
        : Packet(PacketType::CHAT_MESSAGE, PacketPriority::NORMAL) {
    }

    ChatMessagePacket::ChatMessagePacket(const std::string& message, int8_t position)
        : Packet(PacketType::CHAT_MESSAGE, PacketPriority::NORMAL),
          m_message(message), m_position(position) {
        SetReliable(true);
    }

    void ChatMessagePacket::WriteData() {
        WriteString(m_message);
        WriteInt8(m_position);
    }

    void ChatMessagePacket::ReadData() {
        m_message = ReadString();
        m_position = ReadInt8();
    }

    // BlockChangePacket
    BlockChangePacket::BlockChangePacket()
        : Packet(PacketType::BLOCK_CHANGE, PacketPriority::NORMAL) {
    }

    BlockChangePacket::BlockChangePacket(int32_t x, int32_t y, int32_t z, uint32_t blockId)
        : Packet(PacketType::BLOCK_CHANGE, PacketPriority::NORMAL),
          m_x(x), m_y(y), m_z(z), m_blockId(blockId) {
        SetReliable(true);
    }

    void BlockChangePacket::WriteData() {
        WriteInt32(m_x);
        WriteInt32(m_y);
        WriteInt32(m_z);
        WriteUInt32(m_blockId);
    }

    void BlockChangePacket::ReadData() {
        m_x = ReadInt32();
        m_y = ReadInt32();
        m_z = ReadInt32();
        m_blockId = ReadUInt32();
    }

    // ChunkDataPacket
    ChunkDataPacket::ChunkDataPacket()
        : Packet(PacketType::CHUNK_DATA, PacketPriority::HIGH) {
    }

    ChunkDataPacket::ChunkDataPacket(int32_t chunkX, int32_t chunkZ, bool fullChunk,
                                   uint16_t primaryBitMask, const std::vector<uint8_t>& data)
        : Packet(PacketType::CHUNK_DATA, PacketPriority::HIGH),
          m_chunkX(chunkX), m_chunkZ(chunkZ), m_fullChunk(fullChunk),
          m_primaryBitMask(primaryBitMask), m_data(data) {
        SetReliable(true);
        SetCompressed(true);
    }

    void ChunkDataPacket::WriteData() {
        WriteInt32(m_chunkX);
        WriteInt32(m_chunkZ);
        WriteBool(m_fullChunk);
        WriteUInt16(m_primaryBitMask);
        WriteBytes(m_data);
    }

    void ChunkDataPacket::ReadData() {
        m_chunkX = ReadInt32();
        m_chunkZ = ReadInt32();
        m_fullChunk = ReadBool();
        m_primaryBitMask = ReadUInt16();
        m_data = ReadBytes(ReadUInt32());
    }

    // PacketFactory implementation
    std::unordered_map<PacketType, std::function<std::shared_ptr<Packet>()>> PacketFactory::s_creators;

    std::shared_ptr<Packet> PacketFactory::CreatePacket(PacketType type) {
        auto it = s_creators.find(type);
        if (it != s_creators.end()) {
            return it->second();
        }
        return nullptr;
    }

    std::shared_ptr<Packet> PacketFactory::CreatePacketFromData(const std::vector<uint8_t>& data) {
        return Packet::FromBytes(data);
    }

    void PacketFactory::RegisterPacketType(PacketType type, std::function<std::shared_ptr<Packet>()> creator) {
        s_creators[type] = creator;
    }

    // Register packet types
    REGISTER_PACKET_TYPE(PacketType::HANDSHAKE, HandshakePacket);
    REGISTER_PACKET_TYPE(PacketType::PLAYER_POSITION, PlayerPositionPacket);
    REGISTER_PACKET_TYPE(PacketType::PLAYER_POSITION_AND_ROTATION, PlayerPositionAndRotationPacket);
    REGISTER_PACKET_TYPE(PacketType::CHAT_MESSAGE, ChatMessagePacket);
    REGISTER_PACKET_TYPE(PacketType::BLOCK_CHANGE, BlockChangePacket);
    REGISTER_PACKET_TYPE(PacketType::CHUNK_DATA, ChunkDataPacket);

} // namespace VoxelCraft
