/**
 * @file CollisionSystem.hpp
 * @brief VoxelCraft Advanced Collision Detection System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the CollisionSystem class that provides advanced collision
 * detection, contact generation, and collision response for rigid bodies and
 * voxel-based worlds.
 */

#ifndef VOXELCRAFT_PHYSICS_COLLISION_SYSTEM_HPP
#define VOXELCRAFT_PHYSICS_COLLISION_SYSTEM_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <algorithm>

#include "../core/Config.hpp"
#include "PhysicsTypes.hpp"
#include "RigidBody.hpp"
#include "Collider.hpp"

namespace VoxelCraft {

    // Forward declarations
    class RigidBody;
    class Collider;
    class BroadPhase;
    class NarrowPhase;
    class PhysicsMaterial;

    /**
     * @enum CollisionPhase
     * @brief Phases of collision detection
     */
    enum class CollisionPhase {
        BroadPhase,         ///< Broad phase (AABB tests)
        NarrowPhase,        ///< Narrow phase (detailed tests)
        ContactGeneration,  ///< Contact point generation
        ContactProcessing,  ///< Contact processing and response
        PostProcessing      ///< Post-processing and cleanup
    };

    /**
     * @enum CollisionType
     * @brief Types of collisions
     */
    enum class CollisionType {
        Discrete,           ///< Discrete collision detection
        Continuous,         ///< Continuous collision detection (CCD)
        Hybrid,             ///< Hybrid discrete/continuous
        Voxel,              ///< Voxel-based collision
        Terrain,            ///< Terrain collision
        Custom              ///< Custom collision type
    };

    /**
     * @struct ContactPoint
     * @brief Contact point between two colliders
     */
    struct ContactPoint {
        glm::vec3 position;                 ///< Contact position in world space
        glm::vec3 normal;                   ///< Contact normal (from body A to B)
        float penetration;                  ///< Penetration depth
        float lambda;                       ///< Lagrange multiplier for constraint
        float friction;                     ///< Friction coefficient at contact
        float restitution;                  ///< Restitution coefficient at contact
        bool persistent;                    ///< Persistent contact (for warm starting)
        uint32_t featureId;                 ///< Feature ID for persistent contacts
    };

    /**
     * @struct ContactManifold
     * @brief Contact manifold between two bodies
     */
    struct ContactManifold {
        RigidBody* bodyA;                   ///< First body in contact
        RigidBody* bodyB;                   ///< Second body in contact
        Collider* colliderA;                ///< Collider from body A
        Collider* colliderB;                ///< Collider from body B
        std::vector<ContactPoint> points;   ///< Contact points
        glm::vec3 center;                   ///< Manifold center
        float radius;                       ///< Manifold radius
        bool isNew;                        ///< New manifold (not persistent)
        bool isColliding;                   ///< Currently colliding
        uint32_t frameCount;                ///< Frames since first contact
        double timestamp;                   ///< Last update timestamp
    };

    /**
     * @struct CollisionPair
     * @brief Potential collision pair from broad phase
     */
    struct CollisionPair {
        RigidBody* bodyA;                   ///< First body
        RigidBody* bodyB;                   ///< Second body
        Collider* colliderA;                ///< Collider from body A
        Collider* colliderB;                ///< Collider from body B
        float distance;                     ///< Separation distance
        bool isVoxelCollision;              ///< Voxel-based collision
        uint32_t voxelX, voxelY, voxelZ;    ///< Voxel coordinates (if voxel collision)
        bool processed;                     ///< Pair processed flag
    };

    /**
     * @struct RaycastHit
     * @brief Raycast hit result
     */
    struct RaycastHit {
        RigidBody* body;                    ///< Hit body
        Collider* collider;                 ///< Hit collider
        glm::vec3 position;                 ///< Hit position
        glm::vec3 normal;                   ///< Hit normal
        float distance;                     ///< Hit distance
        float fraction;                     ///< Hit fraction along ray
        uint32_t triangleIndex;             ///< Triangle index (for mesh colliders)
        bool isVoxelHit;                    ///< Voxel hit flag
        uint32_t voxelX, voxelY, voxelZ;    ///< Voxel coordinates (if voxel hit)
    };

    /**
     * @struct OverlapResult
     * @brief Overlap test result
     */
    struct OverlapResult {
        RigidBody* body;                    ///< Overlapping body
        Collider* collider;                 ///< Overlapping collider
        float penetration;                  ///< Penetration depth
        glm::vec3 separation;               ///< Separation vector
        bool isVoxelOverlap;                ///< Voxel overlap flag
    };

    /**
     * @struct CollisionConfig
     * @brief Collision system configuration
     */
    struct CollisionConfig {
        // Basic settings
        CollisionType collisionType;        ///< Collision detection type
        bool enableCCD;                     ///< Enable continuous collision detection
        bool enablePersistentContacts;      ///< Enable persistent contact manifolds
        bool enableWarmStarting;            ///< Enable constraint warm starting

        // Performance settings
        int maxContactsPerManifold;         ///< Maximum contacts per manifold
        int maxManifoldsPerFrame;           ///< Maximum manifolds per frame
        float contactBreakThreshold;        ///< Contact breaking threshold
        float contactMergeThreshold;        ///< Contact merging threshold

        // Quality settings
        float collisionMargin;              ///< Collision margin for stability
        float minContactSeparation;         ///< Minimum contact separation
        float maxPenetrationDepth;          ///< Maximum allowed penetration
        bool enableContactCaching;          ///< Enable contact point caching

        // Voxel settings
        bool enableVoxelCollisions;         ///< Enable voxel-based collisions
        float voxelCollisionMargin;         ///< Voxel collision margin
        bool enableVoxelCaching;            ///< Enable voxel collision caching
        int voxelCacheSize;                 ///< Voxel collision cache size

        // Threading settings
        bool enableMultithreading;          ///< Enable multithreaded collision detection
        int workerThreads;                  ///< Number of collision worker threads
        int batchSize;                      ///< Batch size for parallel processing

        // Debug settings
        bool enableDebugDrawing;            ///< Enable collision debug drawing
        bool enableCollisionLogging;        ///< Enable collision logging
        bool enablePerformanceProfiling;    ///< Enable performance profiling
    };

    /**
     * @struct CollisionMetrics
     * @brief Performance metrics for collision system
     */
    struct CollisionMetrics {
        // Broad phase metrics
        uint64_t broadPhaseTests;           ///< Broad phase tests performed
        uint64_t broadPhaseHits;            ///< Broad phase hits
        double broadPhaseTime;              ///< Broad phase time (ms)
        float broadPhaseEfficiency;         ///< Broad phase efficiency (0.0 - 1.0)

        // Narrow phase metrics
        uint64_t narrowPhaseTests;          ///< Narrow phase tests performed
        uint64_t narrowPhaseHits;           ///< Narrow phase hits
        double narrowPhaseTime;             ///< Narrow phase time (ms)
        uint64_t contactsGenerated;         ///< Contact points generated
        uint64_t manifoldsCreated;          ///< Contact manifolds created

        // Contact processing metrics
        uint64_t contactsProcessed;         ///< Contacts processed
        uint64_t contactsRejected;          ///< Contacts rejected
        double contactProcessingTime;       ///< Contact processing time (ms)
        uint64_t persistentContactsUsed;    ///< Persistent contacts reused

        // Voxel collision metrics
        uint64_t voxelCollisions;           ///< Voxel collision tests
        uint64_t voxelHits;                 ///< Voxel collision hits
        double voxelCollisionTime;          ///< Voxel collision time (ms)
        uint64_t voxelCacheHits;            ///< Voxel cache hits
        uint64_t voxelCacheMisses;          ///< Voxel cache misses

        // Performance metrics
        uint64_t totalCollisions;           ///< Total collisions detected
        uint64_t totalManifolds;            ///< Total active manifolds
        double averageCollisionTime;        ///< Average collision detection time
        double maxCollisionTime;            ///< Maximum collision detection time
        uint64_t threadTasks;               ///< Multithreaded tasks
        double threadUtilization;           ///< Thread utilization (0.0 - 1.0)

        // Error metrics
        uint64_t penetrationErrors;         ///< Penetration detection errors
        uint64_t manifoldErrors;            ///< Manifold generation errors
        uint64_t cacheErrors;               ///< Cache-related errors
        uint64_t numericalErrors;           ///< Numerical computation errors
    };

    /**
     * @class CollisionSystem
     * @brief Advanced collision detection and contact generation system
     *
     * The CollisionSystem provides comprehensive collision detection with:
     * - Broad-phase and narrow-phase collision detection
     * - Multiple collision primitives (sphere, box, capsule, mesh, voxel)
     * - Contact manifold generation and management
     * - Persistent contact handling for stability
     * - Voxel-based collision detection
     * - Continuous collision detection (CCD)
     * - Multithreaded collision processing
     * - Debug visualization and profiling
     *
     * Key Features:
     * - Hierarchical collision detection (broad → narrow → contact)
     * - Support for rigid body collisions
     * - Voxel-block collision integration
     * - Contact point generation and caching
     * - Persistent manifolds for stability
     * - Warm starting for constraints
     * - Thread-safe operations
     */
    class CollisionSystem {
    public:
        /**
         * @brief Constructor
         * @param config Collision configuration
         */
        explicit CollisionSystem(const CollisionConfig& config);

        /**
         * @brief Destructor
         */
        ~CollisionSystem();

        /**
         * @brief Deleted copy constructor
         */
        CollisionSystem(const CollisionSystem&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        CollisionSystem& operator=(const CollisionSystem&) = delete;

        // System lifecycle

        /**
         * @brief Initialize collision system
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Shutdown collision system
         */
        void Shutdown();

        /**
         * @brief Update collision system
         * @param deltaTime Time elapsed
         */
        void Update(double deltaTime);

        /**
         * @brief Perform collision detection for all bodies
         * @param bodies Vector of rigid bodies
         * @param manifolds Output collision manifolds
         * @return Number of collision manifolds generated
         */
        size_t DetectCollisions(const std::vector<RigidBody*>& bodies,
                               std::vector<ContactManifold>& manifolds);

        // Collision queries

        /**
         * @brief Perform raycast
         * @param ray Ray to cast
         * @param maxDistance Maximum distance
         * @param bodies Bodies to test against
         * @param hitInfo Output hit information
         * @return true if hit something, false otherwise
         */
        bool Raycast(const Ray& ray, float maxDistance,
                    const std::vector<RigidBody*>& bodies, RaycastHit& hitInfo) const;

        /**
         * @brief Perform multiple raycasts
         * @param rays Rays to cast
         * @param maxDistance Maximum distance
         * @param bodies Bodies to test against
         * @param hitInfos Output hit information
         * @return Number of hits
         */
        size_t MultiRaycast(const std::vector<Ray>& rays, float maxDistance,
                           const std::vector<RigidBody*>& bodies,
                           std::vector<RaycastHit>& hitInfos) const;

        /**
         * @brief Perform overlap test with sphere
         * @param center Sphere center
         * @param radius Sphere radius
         * @param bodies Bodies to test against
         * @param results Output overlap results
         * @return Number of overlaps
         */
        size_t OverlapSphere(const glm::vec3& center, float radius,
                            const std::vector<RigidBody*>& bodies,
                            std::vector<OverlapResult>& results) const;

        /**
         * @brief Perform overlap test with AABB
         * @param aabb AABB to test
         * @param bodies Bodies to test against
         * @param results Output overlap results
         * @return Number of overlaps
         */
        size_t OverlapAABB(const AABB& aabb,
                          const std::vector<RigidBody*>& bodies,
                          std::vector<OverlapResult>& results) const;

        // Voxel collision management

        /**
         * @brief Add voxel collision
         * @param position Voxel position
         * @param size Voxel size
         * @return true if added, false otherwise
         */
        bool AddVoxelCollision(const glm::vec3& position, const glm::vec3& size);

        /**
         * @brief Remove voxel collision
         * @param position Voxel position
         * @return true if removed, false otherwise
         */
        bool RemoveVoxelCollision(const glm::vec3& position);

        /**
         * @brief Update voxel collisions for area
         * @param minPos Minimum position
         * @param maxPos Maximum position
         * @return Number of voxel collisions updated
         */
        size_t UpdateVoxelCollisions(const glm::vec3& minPos, const glm::vec3& maxPos);

        /**
         * @brief Check voxel collision at position
         * @param position Position to check
         * @return true if collision exists, false otherwise
         */
        bool HasVoxelCollision(const glm::vec3& position) const;

        /**
         * @brief Get voxel collision data
         * @param position Voxel position
         * @return Voxel collision data or empty optional
         */
        std::optional<VoxelCollisionData> GetVoxelCollision(const glm::vec3& position) const;

        // Manifold management

        /**
         * @brief Get active contact manifolds
         * @return Vector of active manifolds
         */
        std::vector<ContactManifold> GetActiveManifolds() const;

        /**
         * @brief Get manifold between two bodies
         * @param bodyA First body
         * @param bodyB Second body
         * @return Manifold or empty optional if not found
         */
        std::optional<ContactManifold> GetManifold(RigidBody* bodyA, RigidBody* bodyB) const;

        /**
         * @brief Clear all contact manifolds
         */
        void ClearManifolds();

        /**
         * @brief Update persistent manifolds
         * @param manifolds Current manifolds
         * @return Number of manifolds updated
         */
        size_t UpdatePersistentManifolds(std::vector<ContactManifold>& manifolds);

        // Configuration

        /**
         * @brief Get collision configuration
         * @return Current configuration
         */
        const CollisionConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set collision configuration
         * @param config New configuration
         */
        void SetConfig(const CollisionConfig& config);

        /**
         * @brief Get collision metrics
         * @return Performance metrics
         */
        const CollisionMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

        // Debug and visualization

        /**
         * @brief Enable/disable debug drawing
         * @param enabled Enable state
         * @return Previous state
         */
        bool SetDebugDrawingEnabled(bool enabled);

        /**
         * @brief Check if debug drawing is enabled
         * @return true if enabled, false otherwise
         */
        bool IsDebugDrawingEnabled() const { return m_config.enableDebugDrawing; }

        /**
         * @brief Get debug render data
         * @return Debug render data
         */
        const std::vector<DebugRenderData>& GetDebugRenderData() const;

        /**
         * @brief Clear debug render data
         */
        void ClearDebugRenderData();

        // Profiling

        /**
         * @brief Get profiling information
         * @return Profiling data
         */
        std::string GetProfilingData() const;

    private:
        /**
         * @brief Initialize collision subsystems
         * @return true if successful, false otherwise
         */
        bool InitializeSubsystems();

        /**
         * @brief Initialize worker threads
         * @return true if successful, false otherwise
         */
        bool InitializeWorkerThreads();

        /**
         * @brief Worker thread function
         * @param threadId Thread ID
         */
        void WorkerThreadFunction(int threadId);

        /**
         * @brief Perform broad phase collision detection
         * @param bodies Bodies to test
         * @param pairs Output collision pairs
         * @return Number of potential collisions
         */
        size_t PerformBroadPhase(const std::vector<RigidBody*>& bodies,
                                std::vector<CollisionPair>& pairs);

        /**
         * @brief Perform narrow phase collision detection
         * @param pairs Input collision pairs
         * @param manifolds Output contact manifolds
         * @return Number of collisions detected
         */
        size_t PerformNarrowPhase(const std::vector<CollisionPair>& pairs,
                                 std::vector<ContactManifold>& manifolds);

        /**
         * @brief Generate contact manifold for collision pair
         * @param pair Collision pair
         * @param manifold Output manifold
         * @return true if manifold generated, false otherwise
         */
        bool GenerateContactManifold(const CollisionPair& pair, ContactManifold& manifold);

        /**
         * @brief Process contact points for manifold
         * @param manifold Manifold to process
         * @return Number of contacts processed
         */
        size_t ProcessContactPoints(ContactManifold& manifold);

        /**
         * @brief Merge contact manifolds
         * @param manifolds Manifolds to merge
         * @return Number of manifolds after merging
         */
        size_t MergeContactManifolds(std::vector<ContactManifold>& manifolds);

        /**
         * @brief Perform voxel collision detection
         * @param body Body to test
         * @param manifolds Output manifolds
         * @return Number of voxel collisions detected
         */
        size_t DetectVoxelCollisions(RigidBody* body, std::vector<ContactManifold>& manifolds);

        /**
         * @brief Perform continuous collision detection
         * @param bodyA First body
         * @param bodyB Second body
         * @param manifold Output manifold
         * @return true if collision detected, false otherwise
         */
        bool PerformCCD(RigidBody* bodyA, RigidBody* bodyB, ContactManifold& manifold);

        /**
         * @brief Update collision metrics
         * @param deltaTime Time elapsed
         */
        void UpdateMetrics(double deltaTime);

        /**
         * @brief Collect debug render data
         */
        void CollectDebugRenderData();

        /**
         * @brief Validate collision state
         * @return true if valid, false otherwise
         */
        bool ValidateCollisionState() const;

        /**
         * @brief Handle collision errors
         * @param error Error message
         */
        void HandleCollisionError(const std::string& error);

        // Configuration and state
        CollisionConfig m_config;                   ///< Collision configuration
        CollisionMetrics m_metrics;                 ///< Performance metrics

        // Core systems
        std::unique_ptr<BroadPhase> m_broadPhase;   ///< Broad phase collision detection
        std::unique_ptr<NarrowPhase> m_narrowPhase; ///< Narrow phase collision detection

        // Manifold management
        std::unordered_map<uint64_t, ContactManifold> m_manifolds;
        mutable std::shared_mutex m_manifoldsMutex; ///< Manifold synchronization

        // Voxel collisions
        std::unordered_map<glm::vec3, VoxelCollisionData, glm::vec3::hash> m_voxelCollisions;
        mutable std::shared_mutex m_voxelMutex;     ///< Voxel synchronization
        std::unordered_map<uint64_t, VoxelCollisionData> m_voxelCache;
        mutable std::shared_mutex m_cacheMutex;     ///< Cache synchronization

        // Threading
        std::vector<std::thread> m_workerThreads;  ///< Worker threads
        std::atomic<bool> m_workersRunning;        ///< Workers running flag
        std::deque<std::function<void()>> m_taskQueue; ///< Task queue
        mutable std::mutex m_taskMutex;            ///< Task synchronization
        std::condition_variable m_taskCondition;   ///< Task condition variable

        // Debug and profiling
        std::vector<DebugRenderData> m_debugRenderData;
        mutable std::shared_mutex m_debugMutex;    ///< Debug synchronization
        std::unordered_map<std::string, double> m_profileSections;
        mutable std::mutex m_profileMutex;         ///< Profile synchronization

        // Statistics
        uint64_t m_totalBroadPhaseTests;           ///< Total broad phase tests
        uint64_t m_totalNarrowPhaseTests;          ///< Total narrow phase tests
        uint64_t m_totalContactsGenerated;         ///< Total contacts generated
        uint64_t m_totalManifoldsCreated;          ///< Total manifolds created
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_PHYSICS_COLLISION_SYSTEM_HPP
