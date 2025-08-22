/**
 * @file SIMDMath.hpp
 * @brief VoxelCraft SIMD Mathematics Library
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the SIMDMath class that provides SIMD-optimized mathematical
 * operations for the VoxelCraft game engine, including vector operations, matrix
 * operations, trigonometric functions, and physics calculations with CPU SIMD
 * instruction sets (SSE, AVX, NEON) for maximum performance.
 */

#ifndef VOXELCRAFT_PARALLEL_SIMD_MATH_HPP
#define VOXELCRAFT_PARALLEL_SIMD_MATH_HPP

#include <memory>
#include <vector>
#include <array>
#include <functional>
#include <atomic>
#include <optional>
#include <type_traits>
#include <algorithm>
#include <numeric>

#ifdef __x86_64__
#include <immintrin.h>    // AVX, AVX2
#include <smmintrin.h>    // SSE4.1, SSE4.2
#include <tmmintrin.h>    // SSSE3
#include <pmmintrin.h>    // SSE3
#include <xmmintrin.h>    // SSE
#elif defined(__aarch64__)
#include <arm_neon.h>     // ARM NEON
#endif

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"

namespace VoxelCraft {

    /**
     * @enum SIMDInstructionSet
     * @brief Supported SIMD instruction sets
     */
    enum class SIMDInstructionSet {
        None,                   ///< No SIMD support
        SSE,                    ///< SSE (Streaming SIMD Extensions)
        SSE2,                   ///< SSE2
        SSE3,                   ///< SSE3
        SSSE3,                  ///< Supplemental SSE3
        SSE4_1,                 ///< SSE4.1
        SSE4_2,                 ///< SSE4.2
        AVX,                    ///< AVX (Advanced Vector Extensions)
        AVX2,                   ///< AVX2
        AVX512,                 ///< AVX-512
        NEON,                   ///< ARM NEON
        Custom                  ///< Custom instruction set
    };

    /**
     * @enum SIMDDataType
     * @brief SIMD data types
     */
    enum class SIMDDataType {
        Float32,                ///< 32-bit float
        Float64,                ///< 64-bit float
        Int32,                  ///< 32-bit integer
        Int64,                  ///< 64-bit integer
        UInt32,                 ///< 32-bit unsigned integer
        UInt64,                 ///< 64-bit unsigned integer
        Custom                  ///< Custom data type
    };

    /**
     * @enum SIMDVectorSize
     * @brief SIMD vector sizes
     */
    enum class SIMDVectorSize {
        Scalar,                 ///< Scalar (1 element)
        Vec2,                   ///< 2-element vector
        Vec4,                   ///< 4-element vector
        Vec8,                   ///< 8-element vector
        Vec16,                  ///< 16-element vector
        Custom                  ///< Custom size
    };

    /**
     * @struct SIMDVector4f
     * @brief SIMD 4-element float vector
     */
    struct SIMDVector4f {
#ifdef __x86_64__
        __m128 data;            ///< SSE/AVX register data
#elif defined(__aarch64__)
        float32x4_t data;       ///< NEON register data
#else
        std::array<float, 4> data; ///< Fallback array
#endif

        // Constructors
        SIMDVector4f() = default;
        SIMDVector4f(float x, float y, float z, float w);
        SIMDVector4f(const glm::vec4& v);
        SIMDVector4f(const float* ptr);

        // Conversion operators
        operator glm::vec4() const;
        operator const float*() const;

        // Element access
        float& operator[](size_t index);
        const float& operator[](size_t index) const;
    };

    /**
     * @struct SIMDVector8f
     * @brief SIMD 8-element float vector
     */
    struct SIMDVector8f {
#ifdef __x86_64__
        __m256 data;            ///< AVX register data
#elif defined(__aarch64__)
        float32x4x2_t data;     ///< NEON register data (2x4)
#else
        std::array<float, 8> data; ///< Fallback array
#endif

        // Constructors
        SIMDVector8f() = default;
        SIMDVector8f(const float* ptr);
        SIMDVector8f(const SIMDVector4f& low, const SIMDVector4f& high);

        // Conversion operators
        operator const float*() const;

        // Element access
        float& operator[](size_t index);
        const float& operator[](size_t index) const;
    };

    /**
     * @struct SIMDMatrix4f
     * @brief SIMD 4x4 float matrix
     */
    struct SIMDMatrix4f {
#ifdef __x86_64__
        __m128 rows[4];         ///< SSE/AVX register data (4 rows)
#elif defined(__aarch64__)
        float32x4_t rows[4];    ///< NEON register data
#else
        std::array<std::array<float, 4>, 4> data; ///< Fallback array
#endif

        // Constructors
        SIMDMatrix4f() = default;
        SIMDMatrix4f(const glm::mat4& m);
        SIMDMatrix4f(const float* ptr);

        // Conversion operators
        operator glm::mat4() const;
        operator const float*() const;
    };

    /**
     * @struct SIMDMathStats
     * @brief SIMD math performance statistics
     */
    struct SIMDMathStats {
        // Operation statistics
        uint64_t totalOperations;                   ///< Total SIMD operations
        uint64_t vectorOperations;                  ///< Vector operations
        uint64_t matrixOperations;                  ///< Matrix operations
        uint64_t physicsOperations;                 ///< Physics calculations

        // Performance metrics
        double totalExecutionTime;                  ///< Total execution time (ms)
        double averageOperationTime;                ///< Average operation time (ns)
        double peakPerformance;                     ///< Peak performance (GFLOPS)
        double utilization;                         ///< SIMD unit utilization (0-1)

        // Instruction set usage
        std::unordered_map<SIMDInstructionSet, uint64_t> instructionSetUsage; ///< Usage by instruction set
        std::unordered_map<std::string, uint64_t> operationUsage; ///< Usage by operation type

        // Memory statistics
        size_t memoryUsed;                          ///< Memory used by SIMD operations
        size_t cacheMisses;                         ///< Cache misses
        size_t cacheHits;                           ///< Cache hits
        double cacheHitRate;                        ///< Cache hit rate (0-1)

        // Error statistics
        uint32_t alignmentErrors;                   ///< Memory alignment errors
        uint32_t overflowErrors;                    ///< Overflow/underflow errors
        uint32_t precisionErrors;                   ///< Precision-related errors
    };

    /**
     * @class SIMDMath
     * @brief SIMD-optimized mathematics library
     *
     * The SIMDMath class provides SIMD-optimized mathematical operations for the
     * VoxelCraft game engine, including vector operations, matrix operations,
     * trigonometric functions, and physics calculations with CPU SIMD instruction
     * sets (SSE, AVX, NEON) for maximum performance in parallel computations.
     *
     * Key Features:
     * - SIMD vector and matrix operations
     * - Automatic instruction set detection and selection
     * - Memory-aligned data structures
     * - Fallback implementations for unsupported instruction sets
     * - Performance monitoring and optimization
     * - Integration with GLM for seamless usage
     * - Support for 2D, 3D, and 4D vector operations
     * - Matrix transformations and operations
     * - Physics calculations (collision, intersection)
     * - Trigonometric and transcendental functions
     *
     * The SIMD math library is designed to provide maximum performance
     * for mathematical operations while maintaining compatibility
     * across different CPU architectures and instruction sets.
     */
    class SIMDMath {
    public:
        /**
         * @brief Constructor
         */
        SIMDMath();

        /**
         * @brief Destructor
         */
        ~SIMDMath();

        /**
         * @brief Deleted copy constructor
         */
        SIMDMath(const SIMDMath&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        SIMDMath& operator=(const SIMDMath&) = delete;

        // SIMD math lifecycle

        /**
         * @brief Initialize SIMD math system
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Shutdown SIMD math system
         */
        void Shutdown();

        /**
         * @brief Get SIMD math statistics
         * @return Current statistics
         */
        const SIMDMathStats& GetStats() const { return m_stats; }

        // SIMD capability detection

        /**
         * @brief Get supported instruction sets
         * @return Vector of supported instruction sets
         */
        std::vector<SIMDInstructionSet> GetSupportedInstructionSets() const;

        /**
         * @brief Get best instruction set for current CPU
         * @return Best instruction set
         */
        SIMDInstructionSet GetBestInstructionSet() const;

        /**
         * @brief Check if instruction set is supported
         * @param instructionSet Instruction set to check
         * @return true if supported, false otherwise
         */
        bool IsInstructionSetSupported(SIMDInstructionSet instructionSet) const;

        /**
         * @brief Set active instruction set
         * @param instructionSet Instruction set to use
         * @return true if successful, false otherwise
         */
        bool SetActiveInstructionSet(SIMDInstructionSet instructionSet);

        /**
         * @brief Get active instruction set
         * @return Current active instruction set
         */
        SIMDInstructionSet GetActiveInstructionSet() const { return m_activeInstructionSet; }

        // Vector operations (4-element)

        /**
         * @brief Add two 4D vectors
         * @param a First vector
         * @param b Second vector
         * @return Result vector
         */
        SIMDVector4f Add(const SIMDVector4f& a, const SIMDVector4f& b) const;

        /**
         * @brief Subtract two 4D vectors
         * @param a First vector
         * @param b Second vector
         * @return Result vector
         */
        SIMDVector4f Subtract(const SIMDVector4f& a, const SIMDVector4f& b) const;

        /**
         * @brief Multiply two 4D vectors (component-wise)
         * @param a First vector
         * @param b Second vector
         * @return Result vector
         */
        SIMDVector4f Multiply(const SIMDVector4f& a, const SIMDVector4f& b) const;

        /**
         * @brief Divide two 4D vectors (component-wise)
         * @param a First vector
         * @param b Second vector
         * @return Result vector
         */
        SIMDVector4f Divide(const SIMDVector4f& a, const SIMDVector4f& b) const;

        /**
         * @brief Scale 4D vector by scalar
         * @param v Vector to scale
         * @param s Scalar value
         * @return Scaled vector
         */
        SIMDVector4f Scale(const SIMDVector4f& v, float s) const;

        /**
         * @brief Calculate dot product of two 4D vectors
         * @param a First vector
         * @param b Second vector
         * @return Dot product
         */
        float Dot(const SIMDVector4f& a, const SIMDVector4f& b) const;

        /**
         * @brief Calculate cross product of two 3D vectors (using 4D vectors)
         * @param a First vector
         * @param b Second vector
         * @return Cross product vector
         */
        SIMDVector4f Cross(const SIMDVector4f& a, const SIMDVector4f& b) const;

        /**
         * @brief Calculate magnitude of 4D vector
         * @param v Input vector
         * @return Vector magnitude
         */
        float Magnitude(const SIMDVector4f& v) const;

        /**
         * @brief Normalize 4D vector
         * @param v Vector to normalize
         * @return Normalized vector
         */
        SIMDVector4f Normalize(const SIMDVector4f& v) const;

        // Vector operations (8-element)

        /**
         * @brief Add two 8D vectors
         * @param a First vector
         * @param b Second vector
         * @return Result vector
         */
        SIMDVector8f Add(const SIMDVector8f& a, const SIMDVector8f& b) const;

        /**
         * @brief Multiply 8D vector by scalar
         * @param v Vector to scale
         * @param s Scalar value
         * @return Scaled vector
         */
        SIMDVector8f Scale(const SIMDVector8f& v, float s) const;

        /**
         * @brief Calculate dot product of two 8D vectors
         * @param a First vector
         * @param b Second vector
         * @return Dot product
         */
        float Dot(const SIMDVector8f& a, const SIMDVector8f& b) const;

        // Matrix operations

        /**
         * @brief Multiply two 4x4 matrices
         * @param a First matrix
         * @param b Second matrix
         * @return Result matrix
         */
        SIMDMatrix4f Multiply(const SIMDMatrix4f& a, const SIMDMatrix4f& b) const;

        /**
         * @brief Transform 4D vector by 4x4 matrix
         * @param m Transformation matrix
         * @param v Vector to transform
         * @return Transformed vector
         */
        SIMDVector4f Transform(const SIMDMatrix4f& m, const SIMDVector4f& v) const;

        /**
         * @brief Transform multiple 4D vectors by 4x4 matrix
         * @param m Transformation matrix
         * @param vectors Vector of vectors to transform
         * @param output Output vector for transformed vectors
         */
        void TransformVectors(const SIMDMatrix4f& m, const std::vector<SIMDVector4f>& vectors,
                            std::vector<SIMDVector4f>& output) const;

        /**
         * @brief Transpose 4x4 matrix
         * @param m Matrix to transpose
         * @return Transposed matrix
         */
        SIMDMatrix4f Transpose(const SIMDMatrix4f& m) const;

        /**
         * @brief Calculate inverse of 4x4 matrix
         * @param m Matrix to invert
         * @return Inverse matrix
         */
        SIMDMatrix4f Inverse(const SIMDMatrix4f& m) const;

        // Mathematical functions

        /**
         * @brief Calculate sine of 4D vector components
         * @param v Input vector
         * @return Vector with sine applied to each component
         */
        SIMDVector4f Sin(const SIMDVector4f& v) const;

        /**
         * @brief Calculate cosine of 4D vector components
         * @param v Input vector
         * @return Vector with cosine applied to each component
         */
        SIMDVector4f Cos(const SIMDVector4f& v) const;

        /**
         * @brief Calculate square root of 4D vector components
         * @param v Input vector
         * @return Vector with square root applied to each component
         */
        SIMDVector4f Sqrt(const SIMDVector4f& v) const;

        /**
         * @brief Calculate reciprocal square root of 4D vector components
         * @param v Input vector
         * @return Vector with reciprocal square root applied to each component
         */
        SIMDVector4f ReciprocalSqrt(const SIMDVector4f& v) const;

        /**
         * @brief Calculate exponential of 4D vector components
         * @param v Input vector
         * @return Vector with exponential applied to each component
         */
        SIMDVector4f Exp(const SIMDVector4f& v) const;

        /**
         * @brief Calculate logarithm of 4D vector components
         * @param v Input vector
         * @return Vector with logarithm applied to each component
         */
        SIMDVector4f Log(const SIMDVector4f& v) const;

        // Physics calculations

        /**
         * @brief Calculate distance between two 3D points
         * @param a First point
         * @param b Second point
         * @return Distance
         */
        float Distance(const SIMDVector4f& a, const SIMDVector4f& b) const;

        /**
         * @brief Calculate squared distance between two 3D points
         * @param a First point
         * @param b Second point
         * @return Squared distance
         */
        float DistanceSquared(const SIMDVector4f& a, const SIMDVector4f& b) const;

        /**
         * @brief Check if point is inside sphere
         * @param point Point to check
         * @param center Sphere center
         * @param radius Sphere radius
         * @return true if inside, false otherwise
         */
        bool PointInSphere(const SIMDVector4f& point, const SIMDVector4f& center, float radius) const;

        /**
         * @brief Check if point is inside AABB
         * @param point Point to check
         * @param min Minimum bounds
         * @param max Maximum bounds
         * @return true if inside, false otherwise
         */
        bool PointInAABB(const SIMDVector4f& point, const SIMDVector4f& min, const SIMDVector4f& max) const;

        /**
         * @brief Calculate ray-sphere intersection
         * @param rayOrigin Ray origin
         * @param rayDirection Ray direction (normalized)
         * @param sphereCenter Sphere center
         * @param sphereRadius Sphere radius
         * @return Intersection distance (negative if no intersection)
         */
        float RaySphereIntersection(const SIMDVector4f& rayOrigin, const SIMDVector4f& rayDirection,
                                  const SIMDVector4f& sphereCenter, float sphereRadius) const;

        /**
         * @brief Calculate ray-AABB intersection
         * @param rayOrigin Ray origin
         * @param rayDirection Ray direction (normalized)
         * @param aabbMin AABB minimum bounds
         * @param aabbMax AABB maximum bounds
         * @return Intersection distance (negative if no intersection)
         */
        float RayAABBIntersection(const SIMDVector4f& rayOrigin, const SIMDVector4f& rayDirection,
                                const SIMDVector4f& aabbMin, const SIMDVector4f& aabbMax) const;

        // Batch operations

        /**
         * @brief Process batch of vector additions
         * @param a First vectors
         * @param b Second vectors
         * @param results Output results
         */
        void BatchAdd(const std::vector<SIMDVector4f>& a, const std::vector<SIMDVector4f>& b,
                    std::vector<SIMDVector4f>& results) const;

        /**
         * @brief Process batch of vector transformations
         * @param matrix Transformation matrix
         * @param vectors Input vectors
         * @param results Output transformed vectors
         */
        void BatchTransform(const SIMDMatrix4f& matrix, const std::vector<SIMDVector4f>& vectors,
                          std::vector<SIMDVector4f>& results) const;

        /**
         * @brief Process batch of distance calculations
         * @param points1 First points
         * @param points2 Second points
         * @param results Output distances
         */
        void BatchDistance(const std::vector<SIMDVector4f>& points1, const std::vector<SIMDVector4f>& points2,
                         std::vector<float>& results) const;

        // Memory management

        /**
         * @brief Allocate aligned memory for SIMD operations
         * @param size Memory size in bytes
         * @param alignment Memory alignment (default 64 bytes for cache line)
         * @return Aligned memory pointer
         */
        void* AllocateAlignedMemory(size_t size, size_t alignment = 64);

        /**
         * @brief Deallocate aligned memory
         * @param memory Memory pointer
         */
        void DeallocateAlignedMemory(void* memory);

        /**
         * @brief Check if memory is aligned
         * @param memory Memory pointer
         * @param alignment Alignment requirement
         * @return true if aligned, false otherwise
         */
        bool IsMemoryAligned(const void* memory, size_t alignment = 64) const;

        // Utility functions

        /**
         * @brief Get SIMD vector size for current instruction set
         * @return Vector size in elements
         */
        size_t GetVectorSize() const;

        /**
         * @brief Get SIMD register width in bits
         * @return Register width
         */
        size_t GetRegisterWidth() const;

        /**
         * @brief Get optimal alignment for current instruction set
         * @return Optimal alignment in bytes
         */
        size_t GetOptimalAlignment() const;

        /**
         * @brief Convert GLM vector to SIMD vector
         * @param v GLM vector
         * @return SIMD vector
         */
        SIMDVector4f ToSIMD(const glm::vec4& v) const;

        /**
         * @brief Convert GLM matrix to SIMD matrix
         * @param m GLM matrix
         * @return SIMD matrix
         */
        SIMDMatrix4f ToSIMD(const glm::mat4& m) const;

        /**
         * @brief Convert SIMD vector to GLM vector
         * @param v SIMD vector
         * @return GLM vector
         */
        glm::vec4 FromSIMD(const SIMDVector4f& v) const;

        /**
         * @brief Convert SIMD matrix to GLM matrix
         * @param m SIMD matrix
         * @return GLM matrix
         */
        glm::mat4 FromSIMD(const SIMDMatrix4f& m) const;

        /**
         * @brief Get SIMD performance rating
         * @return Performance rating (0.0 - 1.0)
         */
        float GetPerformanceRating() const;

        /**
         * @brief Validate SIMD math state
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
         * @brief Optimize SIMD math system
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> Optimize();

    private:
        /**
         * @brief Detect supported SIMD instruction sets
         */
        void DetectInstructionSets();

        /**
         * @brief Initialize SIMD instruction set
         * @param instructionSet Instruction set to initialize
         * @return true if successful, false otherwise
         */
        bool InitializeInstructionSet(SIMDInstructionSet instructionSet);

        /**
         * @brief Update performance statistics
         */
        void UpdateStats();

        /**
         * @brief Handle SIMD math errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        // SIMD math data
        SIMDMathStats m_stats;                       ///< Performance statistics
        SIMDInstructionSet m_activeInstructionSet;   ///< Active instruction set

        // Instruction set support
        std::vector<SIMDInstructionSet> m_supportedInstructionSets; ///< Supported instruction sets
        std::unordered_map<SIMDInstructionSet, bool> m_instructionSetInitialized; ///< Initialization status

        // Memory management
        std::vector<void*> m_allocatedMemory;        ///< Allocated memory blocks
        size_t m_totalAllocatedMemory;               ///< Total allocated memory

        // State tracking
        bool m_isInitialized;                        ///< System is initialized
        std::string m_lastError;                     ///< Last error message
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_PARALLEL_SIMD_MATH_HPP
