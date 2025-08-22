/**
 * @file AsyncCompute.hpp
 * @brief VoxelCraft Asynchronous Compute System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the AsyncCompute class that provides asynchronous compute
 * functionality for the VoxelCraft game engine, including GPU compute shaders,
 * async command buffers, compute pipelines, and parallel compute operations
 * with proper synchronization and resource management for maximum performance.
 */

#ifndef VOXELCRAFT_PARALLEL_ASYNC_COMPUTE_HPP
#define VOXELCRAFT_PARALLEL_ASYNC_COMPUTE_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <condition_variable>
#include <future>
#include <chrono>
#include <optional>
#include <queue>
#include <deque>
#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"
#include "../core/System.hpp"

namespace VoxelCraft {

    // Forward declarations
    class ThreadPool;
    class ParallelTaskSystem;

    /**
     * @enum ComputeAPI
     * @brief Supported compute APIs
     */
    enum class ComputeAPI {
        None,                   ///< No compute API
        OpenCL,                 ///< OpenCL (cross-platform)
        CUDA,                   ///< CUDA (NVIDIA)
        VulkanCompute,          ///< Vulkan Compute Shaders
        DirectCompute,          ///< DirectX 12 Compute
        MetalCompute,           ///< Metal Compute
        CPU,                    ///< CPU-based compute
        Custom                  ///< Custom compute API
    };

    /**
     * @enum ComputeWorkloadType
     * @brief Types of compute workloads
     */
    enum class ComputeWorkloadType {
        Physics,                ///< Physics simulation
        AI,                     ///< AI calculations
        Procedural,             ///< Procedural generation
        Rendering,              ///< Rendering computations
        Audio,                  ///< Audio processing
        Simulation,             ///< General simulation
        Custom                  ///< Custom workload
    };

    /**
     * @enum ComputePriority
     * @brief Compute task priorities
     */
    enum class ComputePriority {
        Low,                    ///< Low priority
        Normal,                 ///< Normal priority
        High,                   ///< High priority
        Critical,               ///< Critical priority
        RealTime                ///< Real-time priority
    };

    /**
     * @enum ComputeResourceType
     * @brief Types of compute resources
     */
    enum class ComputeResourceType {
        Buffer,                 ///< Data buffer
        Texture,                ///< Texture resource
        Program,                ///< Compute program/shader
        Kernel,                 ///< Compute kernel
        Event,                  ///< Synchronization event
        Custom                  ///< Custom resource type
    };

    /**
     * @struct ComputeResource
     * @brief Compute resource descriptor
     */
    struct ComputeResource {
        uint32_t resourceId;                        ///< Unique resource identifier
        ComputeResourceType type;                   ///< Resource type
        std::string name;                           ///< Resource name
        size_t size;                                ///< Resource size in bytes
        void* nativeHandle;                         ///< Native API handle
        bool isPersistent;                          ///< Whether resource persists

        // Memory properties
        bool isHostVisible;                         ///< Can be mapped to host memory
        bool isDeviceLocal;                         ///< Located in device memory
        bool isCoherent;                            ///< Cache coherent

        // Usage tracking
        double lastUsed;                            ///< Last usage timestamp
        uint32_t usageCount;                        ///< Number of times used
        size_t memoryUsed;                          ///< Memory currently used

        ComputeResource()
            : resourceId(0)
            , type(ComputeResourceType::Buffer)
            , size(0)
            , nativeHandle(nullptr)
            , isPersistent(false)
            , isHostVisible(false)
            , isDeviceLocal(false)
            , isCoherent(false)
            , lastUsed(0.0)
            , usageCount(0)
            , memoryUsed(0)
        {}
    };

    /**
     * @struct ComputeTask
     * @brief Compute task descriptor
     */
    struct ComputeTask {
        uint32_t taskId;                            ///< Unique task identifier
        std::string name;                           ///< Task name
        ComputeWorkloadType workloadType;           ///< Workload type
        ComputePriority priority;                   ///< Task priority

        // Execution parameters
        uint32_t programId;                         ///< Compute program ID
        uint32_t kernelId;                          ///< Kernel ID
        std::array<uint32_t, 3> globalWorkSize;     ///< Global work size
        std::array<uint32_t, 3> localWorkSize;      ///< Local work size
        uint32_t workDimensions;                    ///< Work dimensions (1D, 2D, 3D)

        // Resource bindings
        std::unordered_map<uint32_t, uint32_t> resourceBindings; ///< Resource bindings (slot -> resourceId)
        std::unordered_map<std::string, std::any> uniformValues; ///< Uniform values

        // Dependencies and execution
        std::vector<uint32_t> dependencies;         ///< Task dependencies
        std::vector<uint32_t> dependents;           ///< Tasks that depend on this
        bool canExecute;                            ///< Can be executed
        bool isExecuting;                           ///< Currently executing
        bool isCompleted;                           ///< Execution completed

        // Timing and performance
        double submitTime;                          ///< Submission timestamp
        double startTime;                           ///< Start timestamp
        double endTime;                             ///< End timestamp
        double duration;                            ///< Execution duration
        double estimatedDuration;                   ///< Estimated duration

        // Callbacks
        std::function<void(uint32_t)> onComplete;   ///< Completion callback
        std::function<void(uint32_t, const std::string&)> onError; ///< Error callback

        // Error handling
        bool hasError;                              ///< Has error occurred
        std::string errorMessage;                   ///< Error message
        int retryCount;                             ///< Number of retries

        ComputeTask()
            : taskId(0)
            , workloadType(ComputeWorkloadType::Custom)
            , priority(ComputePriority::Normal)
            , programId(0)
            , kernelId(0)
            , workDimensions(1)
            , canExecute(false)
            , isExecuting(false)
            , isCompleted(false)
            , submitTime(0.0)
            , startTime(0.0)
            , endTime(0.0)
            , duration(0.0)
            , estimatedDuration(0.0)
            , hasError(false)
            , retryCount(0)
        {
            globalWorkSize.fill(1);
            localWorkSize.fill(1);
        }
    };

    /**
     * @struct ComputeProgram
     * @brief Compute program descriptor
     */
    struct ComputeProgram {
        uint32_t programId;                         ///< Unique program identifier
        std::string name;                           ///< Program name
        std::string source;                         ///< Program source code
        ComputeAPI api;                             ///< Compute API used
        bool isCompiled;                            ///< Compilation status
        void* nativeProgram;                        ///< Native API program handle

        // Kernels in this program
        std::unordered_map<std::string, uint32_t> kernels; ///< Kernel name to ID mapping
        std::vector<uint32_t> kernelIds;            ///< List of kernel IDs

        // Compilation info
        double compileTime;                         ///< Compilation time
        std::string compileLog;                     ///< Compilation log
        bool hasCompilationError;                   ///< Compilation error flag

        // Resource requirements
        size_t localMemoryRequired;                 ///< Local memory required
        size_t privateMemoryRequired;               ///< Private memory required
        std::vector<ComputeResourceType> resourceTypes; ///< Required resource types

        ComputeProgram()
            : programId(0)
            , api(ComputeAPI::None)
            , isCompiled(false)
            , nativeProgram(nullptr)
            , compileTime(0.0)
            , hasCompilationError(false)
            , localMemoryRequired(0)
            , privateMemoryRequired(0)
        {}
    };

    /**
     * @struct AsyncComputeStats
     * @brief Asynchronous compute performance statistics
     */
    struct AsyncComputeStats {
        // Task statistics
        uint64_t totalTasksSubmitted;               ///< Total tasks submitted
        uint64_t totalTasksCompleted;               ///< Total tasks completed
        uint64_t totalTasksFailed;                  ///< Total tasks failed
        uint64_t activeTasks;                       ///< Currently active tasks

        // Performance metrics
        double averageTaskTime;                     ///< Average task execution time (ms)
        double maxTaskTime;                         ///< Maximum task execution time (ms)
        double totalComputeTime;                    ///< Total compute time (ms)
        double utilization;                         ///< Compute unit utilization (0-1)

        // Resource statistics
        uint64_t totalResourcesCreated;             ///< Total resources created
        uint64_t activeResources;                   ///< Currently active resources
        size_t totalMemoryUsed;                     ///< Total memory used
        size_t peakMemoryUsed;                      ///< Peak memory usage

        // API usage statistics
        std::unordered_map<ComputeAPI, uint64_t> apiUsage; ///< Usage by compute API
        std::unordered_map<ComputeWorkloadType, uint64_t> workloadUsage; ///< Usage by workload type

        // Error statistics
        uint64_t compilationErrors;                 ///< Compilation errors
        uint64_t executionErrors;                   ///< Execution errors
        uint64_t timeoutErrors;                     ///< Timeout errors
        uint64_t resourceErrors;                    ///< Resource-related errors

        // Queue statistics
        uint64_t queueSize;                         ///< Current queue size
        uint64_t maxQueueSize;                      ///< Maximum queue size
        double averageQueueTime;                    ///< Average queue time (ms)
        uint64_t queueOverflows;                    ///< Queue overflow events
    };

    /**
     * @class AsyncCompute
     * @brief Advanced asynchronous compute system
     *
     * The AsyncCompute class provides asynchronous compute functionality for the
     * VoxelCraft game engine, including GPU compute shaders, async command buffers,
     * compute pipelines, and parallel compute operations with proper synchronization
     * and resource management for maximum performance in game computations.
     *
     * Key Features:
     * - Multiple compute APIs (OpenCL, CUDA, Vulkan, DirectX, Metal)
     * - GPU compute shaders and kernel execution
     * - Asynchronous command buffers and queues
     * - Resource management and memory optimization
     * - Task dependencies and execution graphs
     * - Performance monitoring and profiling
     * - Multi-GPU support and load balancing
     * - Integration with game engine systems
     * - Real-time performance analysis
     *
     * The async compute system is designed to leverage GPU and CPU compute
     * resources efficiently for game-related computations while maintaining
     * proper synchronization with the main game loop.
     */
    class AsyncCompute : public System {
    public:
        /**
         * @brief Constructor
         * @param threadPool Thread pool instance
         */
        explicit AsyncCompute(ThreadPool* threadPool);

        /**
         * @brief Destructor
         */
        ~AsyncCompute();

        /**
         * @brief Deleted copy constructor
         */
        AsyncCompute(const AsyncCompute&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        AsyncCompute& operator=(const AsyncCompute&) = delete;

        // Async compute lifecycle

        /**
         * @brief Initialize async compute system
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Shutdown async compute system
         */
        void Shutdown();

        /**
         * @brief Update async compute system
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Get async compute statistics
         * @return Current statistics
         */
        const AsyncComputeStats& GetStats() const { return m_stats; }

        // Compute API management

        /**
         * @brief Get available compute APIs
         * @return Vector of available APIs
         */
        std::vector<ComputeAPI> GetAvailableAPIs() const;

        /**
         * @brief Set active compute API
         * @param api Compute API to use
         * @return true if successful, false otherwise
         */
        bool SetActiveAPI(ComputeAPI api);

        /**
         * @brief Get active compute API
         * @return Current active API
         */
        ComputeAPI GetActiveAPI() const { return m_activeAPI; }

        /**
         * @brief Check if compute API is available
         * @param api API to check
         * @return true if available, false otherwise
         */
        bool IsAPIAvailable(ComputeAPI api) const;

        // Compute program management

        /**
         * @brief Create compute program
         * @param name Program name
         * @param source Source code
         * @param api Compute API to use
         * @return Program ID or 0 if failed
         */
        uint32_t CreateProgram(const std::string& name, const std::string& source, ComputeAPI api = ComputeAPI::None);

        /**
         * @brief Compile compute program
         * @param programId Program ID
         * @param options Compilation options
         * @return true if successful, false otherwise
         */
        bool CompileProgram(uint32_t programId, const std::string& options = "");

        /**
         * @brief Get program compilation status
         * @param programId Program ID
         * @return Compilation status and log
         */
        std::pair<bool, std::string> GetProgramStatus(uint32_t programId) const;

        /**
         * @brief Create kernel from program
         * @param programId Program ID
         * @param kernelName Kernel function name
         * @return Kernel ID or 0 if failed
         */
        uint32_t CreateKernel(uint32_t programId, const std::string& kernelName);

        /**
         * @brief Destroy compute program
         * @param programId Program ID
         * @return true if successful, false otherwise
         */
        bool DestroyProgram(uint32_t programId);

        // Compute resource management

        /**
         * @brief Create compute resource
         * @param type Resource type
         * @param name Resource name
         * @param size Resource size in bytes
         * @param persistent Whether resource should persist
         * @return Resource ID or 0 if failed
         */
        uint32_t CreateResource(ComputeResourceType type, const std::string& name, size_t size, bool persistent = false);

        /**
         * @brief Write data to compute resource
         * @param resourceId Resource ID
         * @param data Data to write
         * @param offset Offset in resource
         * @return true if successful, false otherwise
         */
        bool WriteResource(uint32_t resourceId, const void* data, size_t offset = 0);

        /**
         * @brief Read data from compute resource
         * @param resourceId Resource ID
         * @param data Output data buffer
         * @param size Number of bytes to read
         * @param offset Offset in resource
         * @return true if successful, false otherwise
         */
        bool ReadResource(uint32_t resourceId, void* data, size_t size, size_t offset = 0);

        /**
         * @brief Copy data between compute resources
         * @param srcId Source resource ID
         * @param dstId Destination resource ID
         * @param size Number of bytes to copy
         * @param srcOffset Source offset
         * @param dstOffset Destination offset
         * @return true if successful, false otherwise
         */
        bool CopyResource(uint32_t srcId, uint32_t dstId, size_t size, size_t srcOffset = 0, size_t dstOffset = 0);

        /**
         * @brief Destroy compute resource
         * @param resourceId Resource ID
         * @return true if successful, false otherwise
         */
        bool DestroyResource(uint32_t resourceId);

        /**
         * @brief Get compute resource info
         * @param resourceId Resource ID
         * @return Resource information
         */
        std::optional<ComputeResource> GetResourceInfo(uint32_t resourceId) const;

        // Compute task management

        /**
         * @brief Submit compute task
         * @param task Task to submit
         * @return Task ID or 0 if failed
         */
        uint32_t SubmitTask(const ComputeTask& task);

        /**
         * @brief Submit compute task with dependencies
         * @param task Task to submit
         * @param dependencies Task dependencies
         * @return Task ID or 0 if failed
         */
        uint32_t SubmitTaskWithDependencies(const ComputeTask& task, const std::vector<uint32_t>& dependencies);

        /**
         * @brief Wait for compute task completion
         * @param taskId Task ID
         * @param timeout Timeout in seconds (0 = infinite)
         * @return true if completed, false if timeout or failed
         */
        bool WaitForTask(uint32_t taskId, float timeout = 0.0f);

        /**
         * @brief Cancel compute task
         * @param taskId Task ID
         * @return true if cancelled, false otherwise
         */
        bool CancelTask(uint32_t taskId);

        /**
         * @brief Get compute task status
         * @param taskId Task ID
         * @return Task status information
         */
        std::optional<ComputeTask> GetTaskStatus(uint32_t taskId) const;

        // High-level compute operations

        /**
         * @brief Execute physics simulation on GPU
         * @param positions Position data
         * @param velocities Velocity data
         * @param deltaTime Time step
         * @param numEntities Number of entities
         * @return std::future for completion
         */
        std::future<void> ExecutePhysicsSimulation(std::vector<glm::vec3>& positions,
                                                  std::vector<glm::vec3>& velocities,
                                                  float deltaTime, size_t numEntities);

        /**
         * @brief Execute procedural terrain generation
         * @param heightmap Output heightmap
         * @param width Heightmap width
         * @param height Heightmap height
         * @param seed Random seed
         * @return std::future for completion
         */
        std::future<void> ExecuteTerrainGeneration(std::vector<float>& heightmap,
                                                  int width, int height, uint32_t seed);

        /**
         * @brief Execute AI pathfinding on GPU
         * @param startPositions Start positions
         * @param endPositions End positions
         * @param paths Output paths
         * @param numAgents Number of agents
         * @return std::future for completion
         */
        std::future<void> ExecutePathfinding(std::vector<glm::vec3>& startPositions,
                                           std::vector<glm::vec3>& endPositions,
                                           std::vector<std::vector<glm::vec3>>& paths,
                                           size_t numAgents);

        /**
         * @brief Execute particle simulation
         * @param positions Particle positions
         * @param velocities Particle velocities
         * @param deltaTime Time step
         * @param numParticles Number of particles
         * @return std::future for completion
         */
        std::future<void> ExecuteParticleSimulation(std::vector<glm::vec4>& positions,
                                                   std::vector<glm::vec4>& velocities,
                                                   float deltaTime, size_t numParticles);

        // Resource optimization

        /**
         * @brief Optimize resource usage
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> OptimizeResources();

        /**
         * @brief Defragment compute memory
         * @return true if successful, false otherwise
         */
        bool DefragmentMemory();

        /**
         * @brief Get memory usage statistics
         * @return Memory usage by category
         */
        std::unordered_map<std::string, size_t> GetMemoryUsage() const;

        // Performance monitoring

        /**
         * @brief Enable performance monitoring
         * @param enabled Enable state
         * @return true if successful, false otherwise
         */
        bool EnablePerformanceMonitoring(bool enabled);

        /**
         * @brief Get compute performance report
         * @return Detailed performance report
         */
        std::string GetPerformanceReport() const;

        /**
         * @brief Get API-specific performance data
         * @param api Compute API
         * @return API performance data
         */
        std::unordered_map<std::string, double> GetAPIPerformanceData(ComputeAPI api) const;

        // Utility functions

        /**
         * @brief Get compute device information
         * @return Device information string
         */
        std::string GetDeviceInfo() const;

        /**
         * @brief Get supported compute features
         * @return Vector of supported features
         */
        std::vector<std::string> GetSupportedFeatures() const;

        /**
         * @brief Check if feature is supported
         * @param feature Feature name
         * @return true if supported, false otherwise
         */
        bool IsFeatureSupported(const std::string& feature) const;

        /**
         * @brief Get recommended compute settings
         * @param workloadType Workload type
         * @return Recommended settings
         */
        std::unordered_map<std::string, std::any> GetRecommendedSettings(ComputeWorkloadType workloadType) const;

        /**
         * @brief Validate async compute system
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Get status report
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

        /**
         * @brief Optimize async compute system
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> Optimize();

    private:
        /**
         * @brief Initialize compute APIs
         * @return true if successful, false otherwise
         */
        bool InitializeAPIs();

        /**
         * @brief Initialize compute API
         * @param api API to initialize
         * @return true if successful, false otherwise
         */
        bool InitializeAPI(ComputeAPI api);

        /**
         * @brief Execute compute task
         * @param task Task to execute
         * @return true if successful, false otherwise
         */
        bool ExecuteComputeTask(ComputeTask& task);

        /**
         * @brief Process completed compute task
         * @param taskId Task ID
         */
        void ProcessCompletedTask(uint32_t taskId);

        /**
         * @brief Check task dependencies
         * @param taskId Task ID
         * @return true if dependencies satisfied, false otherwise
         */
        bool CheckTaskDependencies(uint32_t taskId) const;

        /**
         * @brief Update task execution queue
         */
        void UpdateTaskQueue();

        /**
         * @brief Handle compute API-specific operations
         * @param api Compute API
         * @param operation Operation to perform
         * @return true if successful, false otherwise
         */
        bool HandleAPIOperation(ComputeAPI api, const std::string& operation);

        /**
         * @brief Update performance statistics
         * @param deltaTime Time elapsed
         */
        void UpdateStats(double deltaTime);

        /**
         * @brief Handle async compute errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        // Async compute data
        ThreadPool* m_threadPool;                    ///< Thread pool instance
        AsyncComputeStats m_stats;                   ///< Performance statistics

        // Compute API management
        ComputeAPI m_activeAPI;                      ///< Active compute API
        std::vector<ComputeAPI> m_availableAPIs;     ///< Available compute APIs
        std::unordered_map<ComputeAPI, bool> m_apiInitialized; ///< API initialization status

        // Program and kernel management
        std::unordered_map<uint32_t, ComputeProgram> m_programs; ///< Compute programs
        std::unordered_map<uint32_t, ComputeTask> m_tasks; ///< Compute tasks
        mutable std::shared_mutex m_programMutex;    ///< Program synchronization

        // Resource management
        std::unordered_map<uint32_t, ComputeResource> m_resources; ///< Compute resources
        mutable std::shared_mutex m_resourceMutex;   ///< Resource synchronization

        // Task execution
        std::queue<uint32_t> m_taskQueue;            ///< Task execution queue
        std::vector<uint32_t> m_executingTasks;      ///< Currently executing tasks
        mutable std::shared_mutex m_taskMutex;       ///< Task synchronization
        std::condition_variable_any m_taskAvailable;  ///< Task available condition

        // Performance tracking
        std::deque<double> m_executionTimes;         ///< Recent execution times
        std::chrono::steady_clock::time_point m_startTime; ///< System start time

        // State tracking
        bool m_isInitialized;                        ///< System is initialized
        double m_lastUpdateTime;                     ///< Last update time
        std::string m_lastError;                     ///< Last error message

        static std::atomic<uint32_t> s_nextProgramId; ///< Next program ID counter
        static std::atomic<uint32_t> s_nextKernelId;  ///< Next kernel ID counter
        static std::atomic<uint32_t> s_nextResourceId; ///< Next resource ID counter
        static std::atomic<uint32_t> s_nextTaskId;    ///< Next task ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_PARALLEL_ASYNC_COMPUTE_HPP
