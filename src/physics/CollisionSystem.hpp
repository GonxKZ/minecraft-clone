/**
 * @file CollisionSystem.hpp
 * @brief VoxelCraft Collision Detection and Physics System
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_PHYSICS_COLLISION_SYSTEM_HPP
#define VOXELCRAFT_PHYSICS_COLLISION_SYSTEM_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <functional>

#include "../math/Vec3.hpp"
#include "../math/Vec2.hpp"
#include "../math/Mat4.hpp"
#include "../math/Quaternion.hpp"
#include "../entities/Entity.hpp"
#include "../world/World.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Player;
    class PhysicsBody;
    class RigidBody;
    struct CollisionConfig;
    struct CollisionData;
    struct CollisionManifold;
    struct RaycastHit;
    struct SweepTestResult;
    struct CollisionStats;
    struct BroadphaseData;

    /**
     * @enum CollisionShape
     * @brief Types of collision shapes
     */
    enum class CollisionShape {
        AABB,                    ///< Axis-Aligned Bounding Box
        OBB,                     ///< Oriented Bounding Box
        SPHERE,                  ///< Sphere
        CAPSULE,                 ///< Capsule
        CYLINDER,                ///< Cylinder
        CONVEX_HULL,            ///< Convex hull
        TRIANGLE_MESH,          ///< Triangle mesh
        HEIGHTFIELD,            ///< Heightfield
        COMPOUND,               ///< Compound shape
        CUSTOM                  ///< Custom shape
    };

    /**
     * @enum CollisionType
     * @brief Types of collision detection
     */
    enum class CollisionType {
        DISCRETE,               ///< Discrete collision detection
        CONTINUOUS,             ///< Continuous collision detection
        MULTI_PHASE,            ///< Multi-phase collision detection
        HYBRID                  ///< Hybrid approach
    };

    /**
     * @enum CollisionResponse
     * @brief How collisions are resolved
     */
    enum class CollisionResponse {
        BLOCK,                  ///< Block movement (default)
        SLIDE,                  ///< Slide along surface
        BOUNCE,                 ///< Bounce off surface
        IGNORE,                 ///< Ignore collision
        TRIGGER,                ///< Trigger event only
        CUSTOM                  ///< Custom response
    };

    /**
     * @enum BroadphaseType
     * @brief Broadphase collision detection algorithms
     */
    enum class BroadphaseType {
        BRUTE_FORCE,            ///< Simple O(n²) check
        SPATIAL_HASH,           ///< Spatial hashing
        SWEEP_AND_PRUNE,        ///< Sweep and prune
        DYNAMIC_BVH,            ///< Dynamic Bounding Volume Hierarchy
        OCTREE,                 ///< Octree
        QUADTREE,               ///< Quadtree (2D)
        CUSTOM                  ///< Custom broadphase
    };

    /**
     * @struct AABB
     * @brief Axis-Aligned Bounding Box
     */
    struct AABB {
        Vec3 min;               ///< Minimum corner
        Vec3 max;               ///< Maximum corner

        /**
         * @brief Create AABB from center and extents
         */
        static AABB FromCenterExtents(const Vec3& center, const Vec3& extents) {
            return AABB{
                center - extents,
                center + extents
            };
        }

        /**
         * @brief Create AABB from min and max corners
         */
        static AABB FromMinMax(const Vec3& min, const Vec3& max) {
            return AABB{min, max};
        }

        /**
         * @brief Get center of AABB
         */
        Vec3 GetCenter() const {
            return (min + max) * 0.5f;
        }

        /**
         * @brief Get extents of AABB
         */
        Vec3 GetExtents() const {
            return (max - min) * 0.5f;
        }

        /**
         * @brief Get size of AABB
         */
        Vec3 GetSize() const {
            return max - min;
        }

        /**
         * @brief Check if point is inside AABB
         */
        bool Contains(const Vec3& point) const {
            return point.x >= min.x && point.x <= max.x &&
                   point.y >= min.y && point.y <= max.y &&
                   point.z >= min.z && point.z <= max.z;
        }

        /**
         * @brief Check if AABB intersects with another AABB
         */
        bool Intersects(const AABB& other) const {
            return min.x <= other.max.x && max.x >= other.min.x &&
                   min.y <= other.max.y && max.y >= other.min.y &&
                   min.z <= other.max.z && max.z >= other.min.z;
        }

        /**
         * @brief Expand AABB to include point
         */
        void Expand(const Vec3& point) {
            min = Vec3::Min(min, point);
            max = Vec3::Max(max, point);
        }

        /**
         * @brief Expand AABB to include another AABB
         */
        void Expand(const AABB& other) {
            min = Vec3::Min(min, other.min);
            max = Vec3::Max(max, other.max);
        }

        /**
         * @brief Get volume of AABB
         */
        float GetVolume() const {
            Vec3 size = GetSize();
            return size.x * size.y * size.z;
        }

        /**
         * @brief Get surface area of AABB
         */
        float GetSurfaceArea() const {
            Vec3 size = GetSize();
            return 2.0f * (size.x * size.y + size.x * size.z + size.y * size.z);
        }
    };

    /**
     * @struct OBB
     * @brief Oriented Bounding Box
     */
    struct OBB {
        Vec3 center;            ///< Center position
        Vec3 extents;           ///< Half-extents
        Quaternion rotation;    ///< Rotation quaternion

        /**
         * @brief Get world space axes
         */
        void GetAxes(Vec3& xAxis, Vec3& yAxis, Vec3& zAxis) const {
            Mat4 rotationMatrix = rotation.ToMatrix();
            xAxis = Vec3(rotationMatrix[0], rotationMatrix[1], rotationMatrix[2]);
            yAxis = Vec3(rotationMatrix[4], rotationMatrix[5], rotationMatrix[6]);
            zAxis = Vec3(rotationMatrix[8], rotationMatrix[9], rotationMatrix[10]);
        }

        /**
         * @brief Get corner points
         */
        void GetCorners(std::vector<Vec3>& corners) const {
            Vec3 axes[3];
            GetAxes(axes[0], axes[1], axes[2]);

            corners.clear();
            for (int i = 0; i < 8; ++i) {
                Vec3 corner = center;
                corner += axes[0] * extents.x * ((i & 1) ? 1.0f : -1.0f);
                corner += axes[1] * extents.y * ((i & 2) ? 1.0f : -1.0f);
                corner += axes[2] * extents.z * ((i & 4) ? 1.0f : -1.0f);
                corners.push_back(corner);
            }
        }

        /**
         * @brief Transform OBB by matrix
         */
        OBB Transform(const Mat4& transform) const {
            OBB result;
            result.center = transform.TransformPoint(center);
            result.extents = extents; // Assuming uniform scaling
            result.rotation = Quaternion(transform) * rotation;
            return result;
        }
    };

    /**
     * @struct Sphere
     * @brief Sphere collision shape
     */
    struct Sphere {
        Vec3 center;            ///< Center position
        float radius;           ///< Sphere radius

        /**
         * @brief Create sphere from AABB
         */
        static Sphere FromAABB(const AABB& aabb) {
            Vec3 center = aabb.GetCenter();
            Vec3 extents = aabb.GetExtents();
            float radius = extents.Length();
            return Sphere{center, radius};
        }

        /**
         * @brief Check if point is inside sphere
         */
        bool Contains(const Vec3& point) const {
            return (point - center).LengthSquared() <= radius * radius;
        }

        /**
         * @brief Check if sphere intersects with another sphere
         */
        bool Intersects(const Sphere& other) const {
            float distanceSquared = (center - other.center).LengthSquared();
            float radiusSum = radius + other.radius;
            return distanceSquared <= radiusSum * radiusSum;
        }
    };

    /**
     * @struct Capsule
     * @brief Capsule collision shape
     */
    struct Capsule {
        Vec3 start;             ///< Start position
        Vec3 end;               ///< End position
        float radius;           ///< Capsule radius

        /**
         * @brief Get height of capsule
         */
        float GetHeight() const {
            return (end - start).Length();
        }

        /**
         * @brief Get direction of capsule
         */
        Vec3 GetDirection() const {
            return (end - start).Normalized();
        }
    };

    /**
     * @struct Ray
     * @brief Ray for raycasting
     */
    struct Ray {
        Vec3 origin;            ///< Ray origin
        Vec3 direction;         ///< Ray direction (normalized)
        float maxDistance;      ///< Maximum ray distance

        /**
         * @brief Get point at distance along ray
         */
        Vec3 GetPoint(float distance) const {
            return origin + direction * distance;
        }

        /**
         * @brief Create ray from origin and direction
         */
        static Ray Create(const Vec3& origin, const Vec3& direction, float maxDistance = 1000.0f) {
            return Ray{origin, direction.Normalized(), maxDistance};
        }
    };

    /**
     * @struct CollisionManifold
     * @brief Collision contact information
     */
    struct CollisionManifold {
        Vec3 normal;            ///< Collision normal
        Vec3 point;             ///< Contact point
        float penetration;      ///< Penetration depth
        float restitution;      ///< Restitution coefficient
        float friction;         ///< Friction coefficient
        Entity* entityA;        ///< First entity
        Entity* entityB;        ///< Second entity
        int triangleIndex;      ///< Triangle index (for mesh collisions)
    };

    /**
     * @struct RaycastHit
     * @brief Raycast hit information
     */
    struct RaycastHit {
        bool hit;               ///< Whether ray hit something
        Vec3 point;             ///< Hit point
        Vec3 normal;            ///< Surface normal at hit point
        float distance;         ///< Distance from ray origin to hit point
        Entity* entity;         ///< Hit entity
        int triangleIndex;      ///< Triangle index (for mesh collisions)
        Vec3 barycentric;       ///< Barycentric coordinates (for triangle hits)
        void* userData;         ///< User-defined data
    };

    /**
     * @struct SweepTestResult
     * @brief Sweep test result
     */
    struct SweepTestResult {
        bool hit;               ///< Whether sweep hit something
        Vec3 hitPoint;          ///< Hit point
        Vec3 hitNormal;         ///< Hit normal
        float hitTime;          ///< Time of hit (0-1)
        Entity* hitEntity;      ///< Hit entity
        void* userData;         ///< User-defined data
    };

    /**
     * @struct CollisionConfig
     * @brief Configuration for collision system
     */
    struct CollisionConfig {
        bool enableCollision = true;
        bool enableBroadphase = true;
        bool enableNarrowphase = true;
        bool enableCCD = false;  ///< Continuous Collision Detection
        bool enableMultiThreading = false;

        CollisionType collisionType = CollisionType::DISCRETE;
        BroadphaseType broadphaseType = BroadphaseType::DYNAMIC_BVH;
        float fixedTimeStep = 1.0f / 60.0f;

        // Collision detection settings
        float aabbMargin = 0.01f;    ///< AABB margin for floating point precision
        float contactThreshold = 0.001f;  ///< Contact threshold
        int maxContacts = 4;         ///< Maximum contacts per collision
        float maxPenetration = 0.1f; ///< Maximum penetration allowed

        // Performance settings
        int maxObjectsPerNode = 16;  ///< Max objects per BVH node
        float worldSize = 10000.0f;  ///< World size for spatial hashing
        float cellSize = 10.0f;      ///< Cell size for spatial hashing

        // Physics settings
        float gravity = -9.81f;      ///< Gravity acceleration
        float restitution = 0.3f;    ///< Default restitution
        float friction = 0.5f;       ///< Default friction
        float linearDamping = 0.1f;  ///< Linear damping
        float angularDamping = 0.1f; ///< Angular damping

        // Collision layers
        uint32_t collisionMask = 0xFFFFFFFF;  ///< Default collision mask
    };

    /**
     * @struct CollisionStats
     * @brief Performance statistics for collision system
     */
    struct CollisionStats {
        int totalObjects = 0;           ///< Total collision objects
        int broadphasePairs = 0;        ///< Broadphase collision pairs
        int narrowphaseTests = 0;       ///< Narrowphase collision tests
        int actualCollisions = 0;       ///< Actual collisions detected
        int raycastsPerformed = 0;      ///< Raycasts performed
        int sweepTestsPerformed = 0;    ///< Sweep tests performed

        float broadphaseTime = 0.0f;    ///< Broadphase processing time
        float narrowphaseTime = 0.0f;   ///< Narrowphase processing time
        float collisionTime = 0.0f;     ///< Total collision processing time

        int falsePositives = 0;         ///< Broadphase false positives
        int cacheHits = 0;              ///< Cache hits
        int cacheMisses = 0;            ///< Cache misses
    };

    /**
     * @class CollisionObject
     * @brief Base collision object
     */
    class CollisionObject {
    public:
        CollisionObject();
        virtual ~CollisionObject();

        /**
         * @brief Set collision shape
         */
        void SetShape(CollisionShape shape) { m_shape = shape; }

        /**
         * @brief Get collision shape
         */
        CollisionShape GetShape() const { return m_shape; }

        /**
         * @brief Set position
         */
        void SetPosition(const Vec3& position) { m_position = position; UpdateBounds(); }

        /**
         * @brief Get position
         */
        const Vec3& GetPosition() const { return m_position; }

        /**
         * @brief Set rotation
         */
        void SetRotation(const Quaternion& rotation) { m_rotation = rotation; UpdateBounds(); }

        /**
         * @brief Get rotation
         */
        const Quaternion& GetRotation() const { return m_rotation; }

        /**
         * @brief Set scale
         */
        void SetScale(const Vec3& scale) { m_scale = scale; UpdateBounds(); }

        /**
         * @brief Get scale
         */
        const Vec3& GetScale() const { return m_scale; }

        /**
         * @brief Set collision response
         */
        void SetCollisionResponse(CollisionResponse response) { m_response = response; }

        /**
         * @brief Get collision response
         */
        CollisionResponse GetCollisionResponse() const { return m_response; }

        /**
         * @brief Set collision mask
         */
        void SetCollisionMask(uint32_t mask) { m_collisionMask = mask; }

        /**
         * @brief Get collision mask
         */
        uint32_t GetCollisionMask() const { return m_collisionMask; }

        /**
         * @brief Set user data
         */
        void SetUserData(void* data) { m_userData = data; }

        /**
         * @brief Get user data
         */
        void* GetUserData() const { return m_userData; }

        /**
         * @brief Get world space AABB
         */
        const AABB& GetWorldAABB() const { return m_worldAABB; }

        /**
         * @brief Check if object is static
         */
        bool IsStatic() const { return m_isStatic; }

        /**
         * @brief Set static state
         */
        void SetStatic(bool isStatic) { m_isStatic = isStatic; }

        /**
         * @brief Get owner entity
         */
        Entity* GetOwner() const { return m_owner; }

        /**
         * @brief Set owner entity
         */
        void SetOwner(Entity* owner) { m_owner = owner; }

    protected:
        /**
         * @brief Update world bounds
         */
        virtual void UpdateBounds() = 0;

        CollisionShape m_shape = CollisionShape::AABB;
        Vec3 m_position = Vec3(0.0f);
        Quaternion m_rotation = Quaternion::Identity();
        Vec3 m_scale = Vec3(1.0f);
        CollisionResponse m_response = CollisionResponse::BLOCK;
        uint32_t m_collisionMask = 0xFFFFFFFF;
        void* m_userData = nullptr;
        bool m_isStatic = false;
        Entity* m_owner = nullptr;

        AABB m_worldAABB;
    };

    /**
     * @class AABBCollisionObject
     * @brief AABB collision object
     */
    class AABBCollisionObject : public CollisionObject {
    public:
        AABBCollisionObject(const AABB& localAABB);

        /**
         * @brief Set local AABB
         */
        void SetLocalAABB(const AABB& aabb) { m_localAABB = aabb; UpdateBounds(); }

        /**
         * @brief Get local AABB
         */
        const AABB& GetLocalAABB() const { return m_localAABB; }

    protected:
        void UpdateBounds() override;

    private:
        AABB m_localAABB;
    };

    /**
     * @class OBBCollisionObject
     * @brief OBB collision object
     */
    class OBBCollisionObject : public CollisionObject {
    public:
        OBBCollisionObject(const OBB& localOBB);

        /**
         * @brief Set local OBB
         */
        void SetLocalOBB(const OBB& obb) { m_localOBB = obb; UpdateBounds(); }

        /**
         * @brief Get local OBB
         */
        const OBB& GetLocalOBB() const { return m_localOBB; }

        /**
         * @brief Get world space OBB
         */
        OBB GetWorldOBB() const;

    protected:
        void UpdateBounds() override;

    private:
        OBB m_localOBB;
    };

    /**
     * @class SphereCollisionObject
     * @brief Sphere collision object
     */
    class SphereCollisionObject : public CollisionObject {
    public:
        SphereCollisionObject(const Sphere& localSphere);

        /**
         * @brief Set local sphere
         */
        void SetLocalSphere(const Sphere& sphere) { m_localSphere = sphere; UpdateBounds(); }

        /**
         * @brief Get local sphere
         */
        const Sphere& GetLocalSphere() const { return m_localSphere; }

        /**
         * @brief Get world space sphere
         */
        Sphere GetWorldSphere() const;

    protected:
        void UpdateBounds() override;

    private:
        Sphere m_localSphere;
    };

    /**
     * @class CapsuleCollisionObject
     * @brief Capsule collision object
     */
    class CapsuleCollisionObject : public CollisionObject {
    public:
        CapsuleCollisionObject(const Capsule& localCapsule);

        /**
         * @brief Set local capsule
         */
        void SetLocalCapsule(const Capsule& capsule) { m_localCapsule = capsule; UpdateBounds(); }

        /**
         * @brief Get local capsule
         */
        const Capsule& GetLocalCapsule() const { return m_localCapsule; }

        /**
         * @brief Get world space capsule
         */
        Capsule GetWorldCapsule() const;

    protected:
        void UpdateBounds() override;

    private:
        Capsule m_localCapsule;
    };

    /**
     * @class CollisionSystem
     * @brief Advanced collision detection and physics system
     *
     * Features:
     * - Multiple collision shapes (AABB, OBB, Sphere, Capsule)
     * - Broadphase and narrowphase collision detection
     * - Raycasting and sweep testing
     * - Continuous Collision Detection (CCD)
     * - Collision layers and masks
     * - Multi-threading support
     * - Performance monitoring and optimization
     * - Dynamic bounding volume hierarchies
     * - Spatial hashing for fast queries
     * - Collision response and resolution
     * - Trigger volumes and events
     * - Mesh and heightfield collision
     * - Character controller collision
     * - Vehicle and rigid body physics
     * - Soft body and cloth simulation
     * - Destructible objects
     * - Fluid and particle collision
     */
    class CollisionSystem {
    public:
        static CollisionSystem& GetInstance();

        /**
         * @brief Initialize the collision system
         * @param config Collision configuration
         * @return true if successful
         */
        bool Initialize(const CollisionConfig& config);

        /**
         * @brief Shutdown the collision system
         */
        void Shutdown();

        /**
         * @brief Update collision system (call every frame)
         * @param deltaTime Time since last update
         */
        void Update(float deltaTime);

        // Collision Object Management
        /**
         * @brief Register collision object
         * @param object Collision object to register
         * @return true if successful
         */
        bool RegisterObject(CollisionObject* object);

        /**
         * @brief Unregister collision object
         * @param object Collision object to unregister
         * @return true if successful
         */
        bool UnregisterObject(CollisionObject* object);

        /**
         * @brief Update collision object position
         * @param object Collision object
         * @param position New position
         * @return true if successful
         */
        bool UpdateObjectPosition(CollisionObject* object, const Vec3& position);

        /**
         * @brief Update collision object transform
         * @param object Collision object
         * @param position New position
         * @param rotation New rotation
         * @return true if successful
         */
        bool UpdateObjectTransform(CollisionObject* object, const Vec3& position, const Quaternion& rotation);

        // Collision Detection
        /**
         * @brief Perform collision detection
         * @param deltaTime Time step
         * @return Number of collisions detected
         */
        int DetectCollisions(float deltaTime);

        /**
         * @brief Check collision between two objects
         * @param objectA First object
         * @param objectB Second object
         * @param manifold Collision manifold (output)
         * @return true if colliding
         */
        bool CheckCollision(CollisionObject* objectA, CollisionObject* objectB, CollisionManifold& manifold);

        /**
         * @brief Check collision between object and world
         * @param object Collision object
         * @param world World
         * @param manifolds Collision manifolds (output)
         * @return Number of collisions
         */
        int CheckWorldCollision(CollisionObject* object, World* world, std::vector<CollisionManifold>& manifolds);

        // Raycasting
        /**
         * @brief Perform raycast
         * @param ray Ray to cast
         * @param hit Hit information (output)
         * @param collisionMask Collision mask filter
         * @return true if hit something
         */
        bool Raycast(const Ray& ray, RaycastHit& hit, uint32_t collisionMask = 0xFFFFFFFF);

        /**
         * @brief Perform raycast against specific object
         * @param ray Ray to cast
         * @param object Target object
         * @param hit Hit information (output)
         * @return true if hit
         */
        bool RaycastObject(const Ray& ray, CollisionObject* object, RaycastHit& hit);

        /**
         * @brief Perform multi-raycast
         * @param ray Ray to cast
         * @param hits Hit information array (output)
         * @param maxHits Maximum number of hits
         * @param collisionMask Collision mask filter
         * @return Number of hits
         */
        int MultiRaycast(const Ray& ray, std::vector<RaycastHit>& hits, int maxHits = 10, uint32_t collisionMask = 0xFFFFFFFF);

        // Sweep Testing
        /**
         * @brief Perform sweep test
         * @param start Start position
         * @param end End position
         * @param shape Collision shape
         * @param result Sweep result (output)
         * @param collisionMask Collision mask filter
         * @return true if hit something
         */
        bool SweepTest(const Vec3& start, const Vec3& end, CollisionObject* shape, SweepTestResult& result, uint32_t collisionMask = 0xFFFFFFFF);

        /**
         * @brief Perform shape cast
         * @param start Start transform
         * @param end End transform
         * @param shape Collision shape
         * @param result Sweep result (output)
         * @param collisionMask Collision mask filter
         * @return true if hit something
         */
        bool ShapeCast(const Mat4& start, const Mat4& end, CollisionObject* shape, SweepTestResult& result, uint32_t collisionMask = 0xFFFFFFFF);

        // Collision Queries
        /**
         * @brief Find objects in AABB
         * @param aabb Query AABB
         * @param objects Objects in AABB (output)
         * @param collisionMask Collision mask filter
         * @return Number of objects found
         */
        int QueryAABB(const AABB& aabb, std::vector<CollisionObject*>& objects, uint32_t collisionMask = 0xFFFFFFFF);

        /**
         * @brief Find objects in sphere
         * @param center Sphere center
         * @param radius Sphere radius
         * @param objects Objects in sphere (output)
         * @param collisionMask Collision mask filter
         * @return Number of objects found
         */
        int QuerySphere(const Vec3& center, float radius, std::vector<CollisionObject*>& objects, uint32_t collisionMask = 0xFFFFFFFF);

        /**
         * @brief Find objects along ray
         * @param ray Query ray
         * @param objects Objects along ray (output)
         * @param collisionMask Collision mask filter
         * @return Number of objects found
         */
        int QueryRay(const Ray& ray, std::vector<CollisionObject*>& objects, uint32_t collisionMask = 0xFFFFFFFF);

        /**
         * @brief Find nearest object
         * @param position Query position
         * @param collisionMask Collision mask filter
         * @return Nearest object or nullptr
         */
        CollisionObject* FindNearestObject(const Vec3& position, uint32_t collisionMask = 0xFFFFFFFF);

        // Collision Response
        /**
         * @brief Resolve collision
         * @param manifold Collision manifold
         * @param deltaTime Time step
         * @return true if resolved
         */
        bool ResolveCollision(const CollisionManifold& manifold, float deltaTime);

        /**
         * @brief Resolve multiple collisions
         * @param manifolds Collision manifolds
         * @param deltaTime Time step
         * @return Number of resolved collisions
         */
        int ResolveCollisions(const std::vector<CollisionManifold>& manifolds, float deltaTime);

        // Physics Integration
        /**
         * @brief Integrate physics
         * @param deltaTime Time step
         */
        void IntegratePhysics(float deltaTime);

        /**
         * @brief Apply gravity to object
         * @param object Collision object
         * @param deltaTime Time step
         */
        void ApplyGravity(CollisionObject* object, float deltaTime);

        /**
         * @brief Apply damping to object
         * @param object Collision object
         * @param deltaTime Time step
         */
        void ApplyDamping(CollisionObject* object, float deltaTime);

        // Configuration
        /**
         * @brief Set collision configuration
         * @param config New configuration
         */
        void SetConfig(const CollisionConfig& config) { m_config = config; }

        /**
         * @brief Get current configuration
         * @return Current configuration
         */
        const CollisionConfig& GetConfig() const { return m_config; }

        // Statistics
        /**
         * @brief Get collision system statistics
         * @return Current statistics
         */
        const CollisionStats& GetStats() const { return m_stats; }

        /**
         * @brief Reset statistics
         */
        void ResetStats();

        // Debug
        /**
         * @brief Enable debug mode
         * @param enable Enable state
         */
        void EnableDebugMode(bool enable) { m_debugMode = enable; }

        /**
         * @brief Render debug information
         */
        void RenderDebugInfo();

        /**
         * @brief Get debug information
         * @return Debug info string
         */
        std::string GetDebugInfo() const;

        /**
         * @brief Validate collision system
         * @return true if valid
         */
        bool Validate() const;

    private:
        CollisionSystem() = default;
        ~CollisionSystem();

        // Prevent copying
        CollisionSystem(const CollisionSystem&) = delete;
        CollisionSystem& operator=(const CollisionSystem&) = delete;

        // Broadphase collision detection
        void UpdateBroadphase();
        void BuildDynamicBVH();
        void UpdateSpatialHash();
        void PerformSweepAndPrune();
        void FindBroadphasePairs(std::vector<std::pair<CollisionObject*, CollisionObject*>>& pairs);

        // Narrowphase collision detection
        bool TestAABBCollision(CollisionObject* a, CollisionObject* b, CollisionManifold& manifold);
        bool TestOBBCollision(CollisionObject* a, CollisionObject* b, CollisionManifold& manifold);
        bool TestSphereCollision(CollisionObject* a, CollisionObject* b, CollisionManifold& manifold);
        bool TestCapsuleCollision(CollisionObject* a, CollisionObject* b, CollisionManifold& manifold);
        bool TestAABBSphereCollision(const AABB& aabb, const Sphere& sphere, CollisionManifold& manifold);
        bool TestAABBPlaneCollision(const AABB& aabb, const Vec3& planeNormal, float planeDistance, CollisionManifold& manifold);

        // Raycast implementations
        bool RaycastAABB(const Ray& ray, const AABB& aabb, float& distance, Vec3& normal);
        bool RaycastOBB(const Ray& ray, const OBB& obb, float& distance, Vec3& normal);
        bool RaycastSphere(const Ray& ray, const Sphere& sphere, float& distance, Vec3& normal);
        bool RaycastCapsule(const Ray& ray, const Capsule& capsule, float& distance, Vec3& normal);
        bool RaycastTriangle(const Ray& ray, const Vec3& v0, const Vec3& v1, const Vec3& v2, float& distance, Vec3& barycentric);

        // Sweep test implementations
        bool SweepAABB(const AABB& aabb, const Vec3& start, const Vec3& end, SweepTestResult& result);
        bool SweepSphere(const Sphere& sphere, const Vec3& start, const Vec3& end, SweepTestResult& result);
        bool SweepCapsule(const Capsule& capsule, const Vec3& start, const Vec3& end, SweepTestResult& result);

        // Collision resolution
        void ResolveAABBManifold(const CollisionManifold& manifold);
        void ResolveOBBManifold(const CollisionManifold& manifold);
        void ResolveSphereManifold(const CollisionManifold& manifold);
        void CalculateContactPoints(const CollisionManifold& manifold, std::vector<Vec3>& points);

        // Utility functions
        AABB CalculateWorldAABB(CollisionObject* object) const;
        bool CheckCollisionMask(CollisionObject* a, CollisionObject* b) const;
        bool IsObjectInLayer(CollisionObject* object, uint32_t layer) const;
        float CalculateRestitution(CollisionObject* a, CollisionObject* b) const;
        float CalculateFriction(CollisionObject* a, CollisionObject* b) const;

        // Performance optimization
        void UpdateObjectCache(CollisionObject* object);
        bool IsCacheValid(CollisionObject* object) const;
        void OptimizeBroadphase();

        // Member variables
        CollisionConfig m_config;
        bool m_initialized = false;
        bool m_debugMode = false;

        // Collision objects
        std::vector<CollisionObject*> m_objects;
        std::vector<CollisionObject*> m_staticObjects;
        std::vector<CollisionObject*> m_dynamicObjects;

        // Broadphase data
        BroadphaseType m_broadphaseType = BroadphaseType::DYNAMIC_BVH;
        std::unique_ptr<BroadphaseData> m_broadphaseData;

        // Collision data
        std::vector<CollisionManifold> m_manifolds;
        std::vector<std::pair<CollisionObject*, CollisionObject*>> m_broadphasePairs;

        // Cache for performance
        std::unordered_map<CollisionObject*, AABB> m_aabbCache;
        std::unordered_map<CollisionObject*, uint32_t> m_cacheVersion;

        // Statistics
        CollisionStats m_stats;

        // Thread safety
        mutable std::shared_mutex m_collisionMutex;

        // Debug data
        std::vector<Ray> m_debugRays;
        std::vector<AABB> m_debugAABBs;
        std::vector<CollisionManifold> m_debugManifolds;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_PHYSICS_COLLISION_SYSTEM_HPP
