/**
 * @file Serializer.hpp
 * @brief VoxelCraft Advanced Binary Serialization System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the Serializer class that provides comprehensive binary
 * serialization and deserialization for the VoxelCraft game engine, including
 * high-performance binary formats, schema evolution, data validation, and
 * memory-efficient serialization for complex game objects and large datasets.
 */

#ifndef VOXELCRAFT_SAVE_SERIALIZER_HPP
#define VOXELCRAFT_SAVE_SERIALIZER_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <chrono>
#include <optional>
#include <type_traits>
#include <array>
#include <deque>
#include <list>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <variant>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"

namespace VoxelCraft {

    // Forward declarations
    class SaveManager;

    /**
     * @enum SerializationMode
     * @brief Serialization operation modes
     */
    enum class SerializationMode {
        Save,                   ///< Saving data
        Load,                   ///< Loading data
        Validate,               ///< Validation only
        CalculateSize           ///< Calculate serialized size
    };

    /**
     * @enum DataType
     * @brief Supported data types for serialization
     */
    enum class DataType {
        // Primitive types
        Bool,
        Int8, Int16, Int32, Int64,
        UInt8, UInt16, UInt32, UInt64,
        Float32, Float64,
        String, Bytes,

        // GLM types
        Vec2, Vec3, Vec4,
        Mat2, Mat3, Mat4,
        Quat,

        // Complex types
        Array, Object,
        Custom
    };

    /**
     * @enum CompressionMethod
     * @brief Data compression methods for serialization
     */
    enum class CompressionMethod {
        None,                   ///< No compression
        RunLengthEncoding,      ///< RLE for repetitive data
        DeltaEncoding,          ///< Delta encoding for sequential data
        Quantization,           ///< Quantization for floating point
        Dictionary,             ///< Dictionary-based compression
        Hybrid                  ///< Hybrid compression method
    };

    /**
     * @struct SerializationContext
     * @brief Context information for serialization operations
     */
    struct SerializationContext {
        SerializationMode mode;                     ///< Operation mode
        uint32_t version;                           ///< Data format version
        std::string objectType;                     ///< Object type being serialized
        std::unordered_map<std::string, std::any> metadata; ///< Additional metadata

        // Performance tracking
        size_t bytesProcessed;                      ///< Bytes processed so far
        size_t totalBytes;                          ///< Total bytes to process
        double startTime;                           ///< Operation start time

        // Error handling
        std::vector<std::string> warnings;          ///< Warning messages
        std::vector<std::string> errors;            ///< Error messages

        SerializationContext()
            : mode(SerializationMode::Save)
            , version(1)
            , bytesProcessed(0)
            , totalBytes(0)
            , startTime(0.0)
        {}
    };

    /**
     * @struct SerializationSchema
     * @brief Schema definition for serialized objects
     */
    struct SerializationSchema {
        uint32_t schemaId;                          ///< Unique schema ID
        std::string objectType;                     ///< Object type name
        uint32_t version;                           ///< Schema version
        std::vector<std::pair<std::string, DataType>> fields; ///< Field definitions
        std::unordered_map<std::string, std::any> defaultValues; ///< Default field values

        // Schema evolution
        std::vector<SchemaMigration> migrations;    ///< Version migration rules

        // Validation rules
        std::unordered_map<std::string, ValidationRule> validationRules; ///< Field validation

        SerializationSchema()
            : schemaId(0)
            , version(1)
        {}
    };

    /**
     * @struct SchemaMigration
     * @brief Schema migration rule for version evolution
     */
    struct SchemaMigration {
        uint32_t fromVersion;                       ///< Source version
        uint32_t toVersion;                         ///< Target version
        std::string description;                    ///< Migration description
        std::function<bool(const std::vector<uint8_t>&, std::vector<uint8_t>&)> migrationFunction; ///< Migration function

        SchemaMigration()
            : fromVersion(0)
            , toVersion(0)
        {}
    };

    /**
     * @struct ValidationRule
     * @brief Data validation rule
     */
    struct ValidationRule {
        std::string fieldName;                      ///< Field name
        std::function<bool(const std::any&)> validator; ///< Validation function
        std::string errorMessage;                   ///< Error message if validation fails

        ValidationRule() = default;
    };

    /**
     * @struct SerializationStats
     * @brief Serialization performance statistics
     */
    struct SerializationStats {
        // Operation statistics
        uint64_t objectsSerialized;                 ///< Total objects serialized
        uint64_t objectsDeserialized;               ///< Total objects deserialized
        uint64_t bytesSerialized;                   ///< Total bytes serialized
        uint64_t bytesDeserialized;                 ///< Total bytes deserialized

        // Performance metrics
        double averageSerializationTime;            ///< Average serialization time (ms)
        double averageDeserializationTime;          ///< Average deserialization time (ms)
        double minSerializationTime;                ///< Minimum serialization time (ms)
        double maxSerializationTime;                ///< Maximum serialization time (ms)

        // Compression statistics
        uint64_t uncompressedBytes;                 ///< Total uncompressed bytes
        uint64_t compressedBytes;                   ///< Total compressed bytes
        float averageCompressionRatio;              ///< Average compression ratio

        // Schema statistics
        uint32_t schemasRegistered;                 ///< Number of registered schemas
        uint32_t schemaMigrations;                  ///< Number of schema migrations performed
        uint32_t validationErrors;                  ///< Number of validation errors

        // Memory statistics
        size_t memoryUsed;                          ///< Current memory usage
        size_t peakMemoryUsed;                      ///< Peak memory usage
        uint32_t memoryAllocations;                 ///< Number of memory allocations
    };

    /**
     * @class Serializer
     * @brief Advanced binary serialization system
     *
     * The Serializer class provides comprehensive binary serialization and
     * deserialization for the VoxelCraft game engine, featuring high-performance
     * binary formats, schema evolution, data validation, memory-efficient
     * serialization, and support for complex game objects and large datasets
     * with enterprise-grade reliability and performance.
     *
     * Key Features:
     * - High-performance binary serialization
     * - Schema-based object serialization
     * - Version migration and compatibility
     * - Data validation and integrity checking
     * - Memory-efficient streaming serialization
     * - Compression and optimization
     * - Support for complex data types (GLM, STL containers)
     * - Custom serialization for game objects
     * - Performance monitoring and optimization
     * - Endianness handling and portability
     * - Memory alignment and padding optimization
     *
     * The serializer is designed to handle massive game worlds efficiently
     * while maintaining data integrity and providing fast save/load times.
     */
    class Serializer {
    public:
        /**
         * @brief Constructor
         * @param context Serialization context
         */
        explicit Serializer(const SerializationContext& context = SerializationContext());

        /**
         * @brief Destructor
         */
        ~Serializer();

        /**
         * @brief Deleted copy constructor
         */
        Serializer(const Serializer&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        Serializer& operator=(const Serializer&) = delete;

        /**
         * @brief Get serialization context
         * @return Current context
         */
        const SerializationContext& GetContext() const { return m_context; }

        /**
         * @brief Set serialization context
         * @param context New context
         */
        void SetContext(const SerializationContext& context);

        /**
         * @brief Get serialization statistics
         * @return Current statistics
         */
        const SerializationStats& GetStats() const { return m_stats; }

        // Schema management

        /**
         * @brief Register serialization schema
         * @param schema Schema definition
         * @return Schema ID or 0 if failed
         */
        uint32_t RegisterSchema(const SerializationSchema& schema);

        /**
         * @brief Get schema by ID
         * @param schemaId Schema ID
         * @return Schema or nullopt if not found
         */
        std::optional<SerializationSchema> GetSchema(uint32_t schemaId) const;

        /**
         * @brief Get schema by object type
         * @param objectType Object type name
         * @return Schema or nullopt if not found
         */
        std::optional<SerializationSchema> GetSchemaByType(const std::string& objectType) const;

        /**
         * @brief Add schema migration
         * @param schemaId Schema ID
         * @param migration Migration rule
         * @return true if successful, false otherwise
         */
        bool AddSchemaMigration(uint32_t schemaId, const SchemaMigration& migration);

        // Serialization interface

        /**
         * @brief Begin serialization
         * @param objectType Object type
         * @param version Data version
         * @return true if successful, false otherwise
         */
        bool BeginSerialization(const std::string& objectType, uint32_t version = 1);

        /**
         * @brief End serialization
         * @return Serialized data
         */
        std::vector<uint8_t> EndSerialization();

        /**
         * @brief Begin deserialization
         * @param data Serialized data
         * @return true if successful, false otherwise
         */
        bool BeginDeserialization(const std::vector<uint8_t>& data);

        /**
         * @brief End deserialization
         * @return true if successful, false otherwise
         */
        bool EndDeserialization();

        // Primitive type serialization

        /**
         * @brief Serialize boolean
         * @param value Boolean value
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool Serialize(bool value, const std::string& name = "");

        /**
         * @brief Serialize integer
         * @param value Integer value
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool Serialize(int32_t value, const std::string& name = "");

        /**
         * @brief Serialize unsigned integer
         * @param value Unsigned integer value
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool Serialize(uint32_t value, const std::string& name = "");

        /**
         * @brief Serialize 64-bit integer
         * @param value 64-bit integer value
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool Serialize(int64_t value, const std::string& name = "");

        /**
         * @brief Serialize 64-bit unsigned integer
         * @param value 64-bit unsigned integer value
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool Serialize(uint64_t value, const std::string& name = "");

        /**
         * @brief Serialize float
         * @param value Float value
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool Serialize(float value, const std::string& name = "");

        /**
         * @brief Serialize double
         * @param value Double value
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool Serialize(double value, const std::string& name = "");

        /**
         * @brief Serialize string
         * @param value String value
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool Serialize(const std::string& value, const std::string& name = "");

        /**
         * @brief Serialize byte array
         * @param data Byte array
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool Serialize(const std::vector<uint8_t>& data, const std::string& name = "");

        // GLM type serialization

        /**
         * @brief Serialize GLM vector2
         * @param value GLM vector2
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool Serialize(const glm::vec2& value, const std::string& name = "");

        /**
         * @brief Serialize GLM vector3
         * @param value GLM vector3
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool Serialize(const glm::vec3& value, const std::string& name = "");

        /**
         * @brief Serialize GLM vector4
         * @param value GLM vector4
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool Serialize(const glm::vec4& value, const std::string& name = "");

        /**
         * @brief Serialize GLM quaternion
         * @param value GLM quaternion
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool Serialize(const glm::quat& value, const std::string& name = "");

        /**
         * @brief Serialize GLM matrix3
         * @param value GLM matrix3
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool Serialize(const glm::mat3& value, const std::string& name = "");

        /**
         * @brief Serialize GLM matrix4
         * @param value GLM matrix4
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool Serialize(const glm::mat4& value, const std::string& name = "");

        // STL container serialization

        /**
         * @brief Serialize vector
         * @param container Vector container
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        template<typename T>
        bool Serialize(const std::vector<T>& container, const std::string& name = "");

        /**
         * @brief Serialize array
         * @param container Array container
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        template<typename T, size_t N>
        bool Serialize(const std::array<T, N>& container, const std::string& name = "");

        /**
         * @brief Serialize set
         * @param container Set container
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        template<typename T>
        bool Serialize(const std::set<T>& container, const std::string& name = "");

        /**
         * @brief Serialize unordered_set
         * @param container Unordered set container
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        template<typename T>
        bool Serialize(const std::unordered_set<T>& container, const std::string& name = "");

        /**
         * @brief Serialize map
         * @param container Map container
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        template<typename K, typename V>
        bool Serialize(const std::map<K, V>& container, const std::string& name = "");

        /**
         * @brief Serialize unordered_map
         * @param container Unordered map container
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        template<typename K, typename V>
        bool Serialize(const std::unordered_map<K, V>& container, const std::string& name = "");

        // Deserialization interface

        /**
         * @brief Deserialize boolean
         * @param value Output boolean value
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool Deserialize(bool& value, const std::string& name = "");

        /**
         * @brief Deserialize integer
         * @param value Output integer value
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool Deserialize(int32_t& value, const std::string& name = "");

        /**
         * @brief Deserialize unsigned integer
         * @param value Output unsigned integer value
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool Deserialize(uint32_t& value, const std::string& name = "");

        /**
         * @brief Deserialize 64-bit integer
         * @param value Output 64-bit integer value
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool Deserialize(int64_t& value, const std::string& name = "");

        /**
         * @brief Deserialize 64-bit unsigned integer
         * @param value Output 64-bit unsigned integer value
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool Deserialize(uint64_t& value, const std::string& name = "");

        /**
         * @brief Deserialize float
         * @param value Output float value
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool Deserialize(float& value, const std::string& name = "");

        /**
         * @brief Deserialize double
         * @param value Output double value
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool Deserialize(double& value, const std::string& name = "");

        /**
         * @brief Deserialize string
         * @param value Output string value
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool Deserialize(std::string& value, const std::string& name = "");

        /**
         * @brief Deserialize byte array
         * @param data Output byte array
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool Deserialize(std::vector<uint8_t>& data, const std::string& name = "");

        // GLM type deserialization

        /**
         * @brief Deserialize GLM vector2
         * @param value Output GLM vector2
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool Deserialize(glm::vec2& value, const std::string& name = "");

        /**
         * @brief Deserialize GLM vector3
         * @param value Output GLM vector3
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool Deserialize(glm::vec3& value, const std::string& name = "");

        /**
         * @brief Deserialize GLM vector4
         * @param value Output GLM vector4
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool Deserialize(glm::vec4& value, const std::string& name = "");

        /**
         * @brief Deserialize GLM quaternion
         * @param value Output GLM quaternion
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool Deserialize(glm::quat& value, const std::string& name = "");

        /**
         * @brief Deserialize GLM matrix3
         * @param value Output GLM matrix3
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool Deserialize(glm::mat3& value, const std::string& name = "");

        /**
         * @brief Deserialize GLM matrix4
         * @param value Output GLM matrix4
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool Deserialize(glm::mat4& value, const std::string& name = "");

        // STL container deserialization

        /**
         * @brief Deserialize vector
         * @param container Output vector container
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        template<typename T>
        bool Deserialize(std::vector<T>& container, const std::string& name = "");

        /**
         * @brief Deserialize array
         * @param container Output array container
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        template<typename T, size_t N>
        bool Deserialize(std::array<T, N>& container, const std::string& name = "");

        /**
         * @brief Deserialize set
         * @param container Output set container
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        template<typename T>
        bool Deserialize(std::set<T>& container, const std::string& name = "");

        /**
         * @brief Deserialize unordered_set
         * @param container Output unordered set container
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        template<typename T>
        bool Deserialize(std::unordered_set<T>& container, const std::string& name = "");

        /**
         * @brief Deserialize map
         * @param container Output map container
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        template<typename K, typename V>
        bool Deserialize(std::map<K, V>& container, const std::string& name = "");

        /**
         * @brief Deserialize unordered_map
         * @param container Output unordered map container
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        template<typename K, typename V>
        bool Deserialize(std::unordered_map<K, V>& container, const std::string& name = "");

        // Advanced serialization features

        /**
         * @brief Serialize object with custom serializer
         * @param object Object to serialize
         * @param serializer Custom serializer function
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool SerializeCustom(const std::any& object,
                           std::function<bool(Serializer&)> serializer,
                           const std::string& name = "");

        /**
         * @brief Deserialize object with custom deserializer
         * @param object Output object
         * @param deserializer Custom deserializer function
         * @param name Field name (optional)
         * @return true if successful, false otherwise
         */
        bool DeserializeCustom(std::any& object,
                             std::function<bool(Serializer&)> deserializer,
                             const std::string& name = "");

        /**
         * @brief Add compression to current serialization
         * @param method Compression method
         * @return true if successful, false otherwise
         */
        bool AddCompression(CompressionMethod method);

        /**
         * @brief Add validation to current serialization
         * @param validator Validation function
         * @return true if successful, false otherwise
         */
        bool AddValidation(std::function<bool(const std::vector<uint8_t>&)> validator);

        // Utility functions

        /**
         * @brief Calculate serialized size
         * @param objectType Object type
         * @return Size in bytes or 0 if unknown
         */
        size_t CalculateSerializedSize(const std::string& objectType) const;

        /**
         * @brief Get current serialization position
         * @return Current position in bytes
         */
        size_t GetCurrentPosition() const;

        /**
         * @brief Seek to position
         * @param position Position in bytes
         * @return true if successful, false otherwise
         */
        bool Seek(size_t position);

        /**
         * @brief Get serialization errors
         * @return Vector of error messages
         */
        const std::vector<std::string>& GetErrors() const { return m_context.errors; }

        /**
         * @brief Get serialization warnings
         * @return Vector of warning messages
         */
        const std::vector<std::string>& GetWarnings() const { return m_context.warnings; }

        /**
         * @brief Clear errors and warnings
         */
        void ClearMessages();

        /**
         * @brief Validate serializer state
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Get status report
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

        /**
         * @brief Get performance report
         * @return Detailed performance report
         */
        std::string GetPerformanceReport() const;

        /**
         * @brief Optimize serializer
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> Optimize();

    private:
        /**
         * @brief Initialize serializer
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Write data to serialization buffer
         * @param data Data to write
         * @param size Data size in bytes
         * @return true if successful, false otherwise
         */
        bool WriteData(const void* data, size_t size);

        /**
         * @brief Read data from deserialization buffer
         * @param data Output data buffer
         * @param size Data size in bytes
         * @return true if successful, false otherwise
         */
        bool ReadData(void* data, size_t size);

        /**
         * @brief Write primitive value with compression
         * @param value Value to write
         * @return true if successful, false otherwise
         */
        template<typename T>
        bool WritePrimitive(T value);

        /**
         * @brief Read primitive value with decompression
         * @param value Output value
         * @return true if successful, false otherwise
         */
        template<typename T>
        bool ReadPrimitive(T& value);

        /**
         * @brief Compress data using specified method
         * @param input Input data
         * @param output Output compressed data
         * @param method Compression method
         * @return true if successful, false otherwise
         */
        bool CompressData(const std::vector<uint8_t>& input,
                         std::vector<uint8_t>& output,
                         CompressionMethod method);

        /**
         * @brief Decompress data using specified method
         * @param input Input compressed data
         * @param output Output decompressed data
         * @param method Compression method
         * @return true if successful, false otherwise
         */
        bool DecompressData(const std::vector<uint8_t>& input,
                          std::vector<uint8_t>& output,
                          CompressionMethod method);

        /**
         * @brief Apply schema migration
         * @param data Input data
         * @param migratedData Output migrated data
         * @param schema Target schema
         * @return true if successful, false otherwise
         */
        bool ApplySchemaMigration(const std::vector<uint8_t>& data,
                                std::vector<uint8_t>& migratedData,
                                const SerializationSchema& schema);

        /**
         * @brief Validate serialized data
         * @param data Data to validate
         * @param schema Schema to validate against
         * @return true if valid, false otherwise
         */
        bool ValidateData(const std::vector<uint8_t>& data, const SerializationSchema& schema);

        /**
         * @brief Update performance statistics
         * @param operationTime Operation time in milliseconds
         * @param dataSize Data size in bytes
         */
        void UpdateStats(double operationTime, size_t dataSize);

        /**
         * @brief Handle serialization errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        // Serializer data
        SerializationContext m_context;              ///< Current serialization context
        SerializationStats m_stats;                  ///< Performance statistics

        // Data buffers
        std::vector<uint8_t> m_buffer;               ///< Serialization buffer
        size_t m_bufferPosition;                     ///< Current buffer position
        std::vector<uint8_t> m_readBuffer;           ///< Read buffer for deserialization
        size_t m_readPosition;                       ///< Current read position

        // Schema management
        std::unordered_map<uint32_t, SerializationSchema> m_schemas; ///< Registered schemas
        std::unordered_map<std::string, uint32_t> m_schemaTypes; ///< Schema type mapping
        mutable std::shared_mutex m_schemasMutex;    ///< Schema synchronization

        // Compression and validation
        std::vector<CompressionMethod> m_compressionStack; ///< Compression methods stack
        std::vector<std::function<bool(const std::vector<uint8_t>&)>> m_validators; ///< Validation functions

        // State tracking
        bool m_isInitialized;                        ///< Serializer is initialized
        bool m_isSerializing;                        ///< Currently serializing
        bool m_isDeserializing;                      ///< Currently deserializing
        std::string m_lastError;                     ///< Last error message

        static std::atomic<uint32_t> s_nextSchemaId; ///< Next schema ID counter
    };

    // Template implementations for container serialization

    template<typename T>
    bool Serializer::Serialize(const std::vector<T>& container, const std::string& name) {
        if (!BeginSerialization("vector", 1)) return false;

        uint32_t size = static_cast<uint32_t>(container.size());
        if (!Serialize(size, "size")) return false;

        for (uint32_t i = 0; i < size; ++i) {
            if (!Serialize(container[i], "item_" + std::to_string(i))) return false;
        }

        return EndSerialization().size() > 0;
    }

    template<typename T, size_t N>
    bool Serializer::Serialize(const std::array<T, N>& container, const std::string& name) {
        if (!BeginSerialization("array", 1)) return false;

        for (size_t i = 0; i < N; ++i) {
            if (!Serialize(container[i], "item_" + std::to_string(i))) return false;
        }

        return EndSerialization().size() > 0;
    }

    template<typename T>
    bool Serializer::Serialize(const std::set<T>& container, const std::string& name) {
        if (!BeginSerialization("set", 1)) return false;

        uint32_t size = static_cast<uint32_t>(container.size());
        if (!Serialize(size, "size")) return false;

        uint32_t index = 0;
        for (const auto& item : container) {
            if (!Serialize(item, "item_" + std::to_string(index++))) return false;
        }

        return EndSerialization().size() > 0;
    }

    template<typename T>
    bool Serializer::Serialize(const std::unordered_set<T>& container, const std::string& name) {
        if (!BeginSerialization("unordered_set", 1)) return false;

        uint32_t size = static_cast<uint32_t>(container.size());
        if (!Serialize(size, "size")) return false;

        uint32_t index = 0;
        for (const auto& item : container) {
            if (!Serialize(item, "item_" + std::to_string(index++))) return false;
        }

        return EndSerialization().size() > 0;
    }

    template<typename K, typename V>
    bool Serializer::Serialize(const std::map<K, V>& container, const std::string& name) {
        if (!BeginSerialization("map", 1)) return false;

        uint32_t size = static_cast<uint32_t>(container.size());
        if (!Serialize(size, "size")) return false;

        uint32_t index = 0;
        for (const auto& pair : container) {
            if (!Serialize(pair.first, "key_" + std::to_string(index))) return false;
            if (!Serialize(pair.second, "value_" + std::to_string(index))) return false;
            ++index;
        }

        return EndSerialization().size() > 0;
    }

    template<typename K, typename V>
    bool Serializer::Serialize(const std::unordered_map<K, V>& container, const std::string& name) {
        if (!BeginSerialization("unordered_map", 1)) return false;

        uint32_t size = static_cast<uint32_t>(container.size());
        if (!Serialize(size, "size")) return false;

        uint32_t index = 0;
        for (const auto& pair : container) {
            if (!Serialize(pair.first, "key_" + std::to_string(index))) return false;
            if (!Serialize(pair.second, "value_" + std::to_string(index))) return false;
            ++index;
        }

        return EndSerialization().size() > 0;
    }

} // namespace VoxelCraft

#endif // VOXELCRAFT_SAVE_SERIALIZER_HPP
