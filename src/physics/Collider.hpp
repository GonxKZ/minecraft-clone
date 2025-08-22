/**
 * @file Collider.hpp
 * @brief VoxelCraft Collision Shapes System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the Collider class and various collision shapes
 * used in the physics system for collision detection and response.
 */

#ifndef VOXELCRAFT_PHYSICS_COLLIDER_HPP
#define VOXELCRAFT_PHYSICS_COLLIDER_HPP

#include <memory>
#include <vector>
#include <array>
#include <unordered_map>
#include <string>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <optional>
#include <any>

#include "../core/Config.hpp"
#include "PhysicsTypes.hpp"

namespace VoxelCraft {

    // Forward declarations
    class RigidBody;
    class PhysicsMaterial;

    /**
     * @enum ColliderType
     * @brief Type of collision shape
     */
    enum class ColliderType {
        Sphere,             ///< Sphere collider
        Box,                ///< Axis-aligned box collider
        Capsule,            ///< Capsule collider
        Cylinder,           ///< Cylinder collider
        ConvexHull,         ///< Convex hull collider
        TriangleMesh,       ///< Triangle mesh collider
        HeightField,        ///< Height field collider
        Compound,           ///< Compound collider (multiple shapes)
        Voxel,              ///< Voxel-based collider
        Custom              ///< Custom collider shape
    };

    /**
     * @enum ColliderFlags
     * @brief Collider behavior flags
     */
    enum ColliderFlags : uint32_t {
        FLAG_NONE           = 0x0000,
        FLAG_IS_TRIGGER     = 0x0001,      ///< Collider is a trigger (no collision response)
        FLAG_DISABLE_RAYCAST = 0x0002,     ///< Disable raycast against this collider
        FLAG_ENABLE_CCD     = 0x0004,      ///< Enable continuous collision detection
        FLAG_DEBUG_DRAW     = 0x0008,      ///< Enable debug drawing
        FLAG_CONVEX_ONLY    = 0x0010,      ///< Convex collision only
        FLAG_DOUBLE_SIDED   = 0x0020,      ///< Double-sided collision
        FLAG_USE_SHARED_SHAPE = 0x0040,    ///< Use shared shape data
        FLAG_DYNAMIC_AABB   = 0x0080,      ///< Dynamic AABB computation
        FLAG_USER_FLAG_0    = 0x1000,      ///< User-defined flag 0
        FLAG_USER_FLAG_1    = 0x2000,      ///< User-defined flag 1
        FLAG_USER_FLAG_2    = 0x4000,      ///< User-defined flag 2
        FLAG_USER_FLAG_3    = 0x8000       ///< User-defined flag 3
    };

    /**
     * @struct AABB
     * @brief Axis-Aligned Bounding Box
     */
    struct AABB {
        glm::vec3 min;                     ///< Minimum corner
        glm::vec3 max;                     ///< Maximum corner

        AABB() = default;
        AABB(const glm::vec3& min, const glm::vec3& max) : min(min), max(max) {}

        /**
         * @brief Get center of AABB
         * @return Center point
         */
        glm::vec3 GetCenter() const { return (min + max) * 0.5f; }

        /**
         * @brief Get size of AABB
         * @return Size vector
         */
        glm::vec3 GetSize() const { return max - min; }

        /**
         * @brief Get extents of AABB (half size)
         * @return Extents vector
         */
        glm::vec3 GetExtents() const { return GetSize() * 0.5f; }

        /**
         * @brief Check if AABB is valid
         * @return true if valid, false otherwise
         */
        bool IsValid() const { return min.x <= max.x && min.y <= max.y && min.z <= max.z; }

        /**
         * @brief Check if point is inside AABB
         * @param point Point to test
         * @return true if inside, false otherwise
         */
        bool Contains(const glm::vec3& point) const {
            return point.x >= min.x && point.x <= max.x &&
                   point.y >= min.y && point.y <= max.y &&
                   point.z >= min.z && point.z <= max.z;
        }

        /**
         * @brief Check if AABB intersects with another AABB
         * @param other Other AABB
         * @return true if intersects, false otherwise
         */
        bool Intersects(const AABB& other) const {
            return min.x <= other.max.x && max.x >= other.min.x &&
                   min.y <= other.max.y && max.y >= other.min.y &&
                   min.z <= other.max.z && max.z >= other.min.z;
        }

        /**
         * @brief Expand AABB to include point
         * @param point Point to include
         */
        void Expand(const glm::vec3& point) {
            min = glm::min(min, point);
            max = glm::max(max, point);
        }

        /**
         * @brief Expand AABB to include another AABB
         * @param other Other AABB
         */
        void Expand(const AABB& other) {
            min = glm::min(min, other.min);
            max = glm::max(max, other.max);
        }

        /**
         * @brief Get volume of AABB
         * @return Volume
         */
        float GetVolume() const {
            glm::vec3 size = GetSize();
            return size.x * size.y * size.z;
        }

        /**
         * @brief Get surface area of AABB
         * @return Surface area
         */
        float GetSurfaceArea() const {
            glm::vec3 size = GetSize();
            return 2.0f * (size.x * size.y + size.x * size.z + size.y * size.z);
        }
    };

    /**
     * @struct BoundingSphere
     * @brief Bounding sphere for collision detection
     */
    struct BoundingSphere {
        glm::vec3 center;                  ///< Sphere center
        float radius;                      ///< Sphere radius

        BoundingSphere() = default;
        BoundingSphere(const glm::vec3& center, float radius) : center(center), radius(radius) {}

        /**
         * @brief Check if sphere is valid
         * @return true if valid, false otherwise
         */
        bool IsValid() const { return radius > 0.0f; }

        /**
         * @brief Check if point is inside sphere
         * @param point Point to test
         * @return true if inside, false otherwise
         */
        bool Contains(const glm::vec3& point) const {
            return glm::distance(center, point) <= radius;
        }

        /**
         * @brief Check if sphere intersects with another sphere
         * @param other Other sphere
         * @return true if intersects, false otherwise
         */
        bool Intersects(const BoundingSphere& other) const {
            float distance = glm::distance(center, other.center);
            return distance <= (radius + other.radius);
        }

        /**
         * @brief Check if sphere intersects with AABB
         * @param aabb AABB to test
         * @return true if intersects, false otherwise
         */
        bool Intersects(const AABB& aabb) const;
    };

    /**
     * @struct Ray
     * @brief Ray for raycasting
     */
    struct Ray {
        glm::vec3 origin;                  ///< Ray origin
        glm::vec3 direction;               ///< Ray direction (normalized)
        float maxDistance;                 ///< Maximum ray distance

        Ray() = default;
        Ray(const glm::vec3& origin, const glm::vec3& direction, float maxDistance = 1000.0f)
            : origin(origin), direction(glm::normalize(direction)), maxDistance(maxDistance) {}

        /**
         * @brief Get point at distance along ray
         * @param distance Distance along ray
         * @return Point at distance
         */
        glm::vec3 GetPoint(float distance) const {
            return origin + direction * distance;
        }

        /**
         * @brief Check if ray is valid
         * @return true if valid, false otherwise
         */
        bool IsValid() const {
            return glm::length(direction) > 0.0f && maxDistance > 0.0f;
        }
    };

    /**
     * @struct Triangle
     * @brief Triangle for mesh collision
     */
    struct Triangle {
        glm::vec3 vertices[3];             ///< Triangle vertices
        glm::vec3 normal;                  ///< Triangle normal
        uint32_t materialIndex;            ///< Material index

        Triangle() = default;
        Triangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2)
            : vertices{v0, v1, v2} {
            normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        }

        /**
         * @brief Get triangle center
         * @return Center point
         */
        glm::vec3 GetCenter() const {
            return (vertices[0] + vertices[1] + vertices[2]) / 3.0f;
        }

        /**
         * @brief Get triangle area
         * @return Area
         */
        float GetArea() const {
            return 0.5f * glm::length(glm::cross(vertices[1] - vertices[0], vertices[2] - vertices[0]));
        }

        /**
         * @brief Check if point is inside triangle
         * @param point Point to test
         * @return true if inside, false otherwise
         */
        bool ContainsPoint(const glm::vec3& point) const;
    };

    /**
     * @struct ColliderProperties
     * @brief Properties of a collider
     */
    struct ColliderProperties {
        std::string name;                  ///< Collider name
        glm::vec3 localPosition;           ///< Local position offset
        glm::quat localRotation;           ///< Local rotation
        glm::vec3 localScale;              ///< Local scale
        PhysicsMaterial* material;         ///< Physics material
        uint32_t collisionGroup;           ///< Collision group
        uint32_t collisionMask;            ///< Collision mask
        uint32_t flags;                    ///< Collider flags
        float density;                     ///< Density for mass calculation
        bool isTrigger;                    ///< Trigger collider flag
        std::unordered_map<std::string, std::any> customProperties; ///< Custom properties
    };

    /**
     * @struct ColliderMetrics
     * @brief Performance metrics for a collider
     */
    struct ColliderMetrics {
        uint64_t collisionTests;           ///< Number of collision tests
        uint64_t collisionHits;            ///< Number of collision hits
        uint64_t raycastTests;             ///< Number of raycast tests
        uint64_t raycastHits;              ///< Number of raycast hits
        double totalCollisionTime;         ///< Total time in collision detection
        double totalRaycastTime;           ///< Total time in raycasting
        size_t memoryUsage;                ///< Memory usage estimate
        uint32_t triangleCount;            ///< Number of triangles (for meshes)
        uint32_t vertexCount;              ///< Number of vertices (for meshes)
    };

    /**
     * @class Collider
     * @brief Base class for collision shapes
     *
     * Collider is the base class for all collision shapes in the physics system.
     * It provides the interface for collision detection, raycasting, and
     * geometric queries. Different collider types implement specific collision
     * detection algorithms optimized for their shape.
     *
     * Key Features:
     * - Multiple collision shapes (sphere, box, capsule, mesh, etc.)
     * - Efficient collision detection algorithms
     * - Raycasting support
     * - Bounding volume calculation
     * - Material properties
     * - Transform hierarchy support
     * - Debug visualization
     */
    class Collider {
    public:
        /**
         * @brief Constructor
         * @param type Collider type
         */
        explicit Collider(ColliderType type);

        /**
         * @brief Destructor
         */
        virtual ~Collider() = default;

        /**
         * @brief Deleted copy constructor
         */
        Collider(const Collider&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        Collider& operator=(const Collider&) = delete;

        // Collider identification

        /**
         * @brief Get collider ID
         * @return Unique collider ID
         */
        uint32_t GetID() const { return m_id; }

        /**
         * @brief Get collider type
         * @return Collider type
         */
        ColliderType GetType() const { return m_type; }

        /**
         * @brief Get collider name
         * @return Collider name
         */
        const std::string& GetName() const { return m_properties.name; }

        /**
         * @brief Set collider name
         * @param name New name
         */
        void SetName(const std::string& name) { m_properties.name = name; }

        // Transform access

        /**
         * @brief Get local position
         * @return Local position offset
         */
        const glm::vec3& GetLocalPosition() const { return m_properties.localPosition; }

        /**
         * @brief Set local position
         * @param position New local position
         */
        void SetLocalPosition(const glm::vec3& position);

        /**
         * @brief Get local rotation
         * @return Local rotation quaternion
         */
        const glm::quat& GetLocalRotation() const { return m_properties.localRotation; }

        /**
         * @brief Set local rotation
         * @param rotation New local rotation
         */
        void SetLocalRotation(const glm::quat& rotation);

        /**
         * @brief Get local scale
         * @return Local scale
         */
        const glm::vec3& GetLocalScale() const { return m_properties.localScale; }

        /**
         * @brief Set local scale
         * @param scale New local scale
         */
        void SetLocalScale(const glm::vec3& scale);

        /**
         * @brief Get world transform matrix
         * @return World transform matrix
         */
        glm::mat4 GetWorldTransform() const;

        /**
         * @brief Get local transform matrix
         * @return Local transform matrix
         */
        glm::mat4 GetLocalTransform() const;

        // Rigid body association

        /**
         * @brief Get associated rigid body
         * @return Associated rigid body
         */
        RigidBody* GetRigidBody() const { return m_rigidBody; }

        /**
         * @brief Set associated rigid body
         * @param body Rigid body to associate
         */
        void SetRigidBody(RigidBody* body) { m_rigidBody = body; }

        // Material access

        /**
         * @brief Get physics material
         * @return Physics material
         */
        PhysicsMaterial* GetMaterial() const { return m_properties.material; }

        /**
         * @brief Set physics material
         * @param material New physics material
         */
        void SetMaterial(PhysicsMaterial* material) { m_properties.material = material; }

        // Bounding volumes

        /**
         * @brief Get local space AABB
         * @return Local AABB
         */
        virtual AABB GetLocalAABB() const = 0;

        /**
         * @brief Get world space AABB
         * @return World AABB
         */
        virtual AABB GetWorldAABB() const;

        /**
         * @brief Get local space bounding sphere
         * @return Local bounding sphere
         */
        virtual BoundingSphere GetLocalBoundingSphere() const = 0;

        /**
         * @brief Get world space bounding sphere
         * @return World bounding sphere
         */
        virtual BoundingSphere GetWorldBoundingSphere() const;

        // Collision detection

        /**
         * @brief Check collision with another collider
         * @param other Other collider
         * @param contactPoint Output contact point (if collision)
         * @return true if colliding, false otherwise
         */
        virtual bool CheckCollision(const Collider* other, glm::vec3& contactPoint) const = 0;

        /**
         * @brief Get collision penetration depth
         * @param other Other collider
         * @return Penetration depth (0 if no collision)
         */
        virtual float GetPenetrationDepth(const Collider* other) const = 0;

        /**
         * @brief Get collision normal
         * @param other Other collider
         * @return Collision normal vector
         */
        virtual glm::vec3 GetCollisionNormal(const Collider* other) const = 0;

        // Raycasting

        /**
         * @brief Perform raycast against collider
         * @param ray Ray to cast
         * @param hitInfo Output hit information
         * @return true if hit, false otherwise
         */
        virtual bool Raycast(const Ray& ray, RaycastHit& hitInfo) const = 0;

        /**
         * @brief Get closest point on collider surface
         * @param point World space point
         * @return Closest point on surface
         */
        virtual glm::vec3 GetClosestPoint(const glm::vec3& point) const = 0;

        // Geometric queries

        /**
         * @brief Check if point is inside collider
         * @param point World space point
         * @return true if inside, false otherwise
         */
        virtual bool ContainsPoint(const glm::vec3& point) const = 0;

        /**
         * @brief Get volume of collider
         * @return Volume
         */
        virtual float GetVolume() const = 0;

        /**
         * @brief Get surface area of collider
         * @return Surface area
         */
        virtual float GetSurfaceArea() const = 0;

        /**
         * @brief Get inertia tensor for collider
         * @param mass Mass of object
         * @return Inertia tensor
         */
        virtual glm::mat3 GetInertiaTensor(float mass) const = 0;

        // Properties

        /**
         * @brief Get collider properties
         * @return Collider properties
         */
        const ColliderProperties& GetProperties() const { return m_properties; }

        /**
         * @brief Set collider properties
         * @param properties New properties
         */
        void SetProperties(const ColliderProperties& properties);

        /**
         * @brief Check if collider is a trigger
         * @return true if trigger, false otherwise
         */
        bool IsTrigger() const { return m_properties.isTrigger; }

        /**
         * @brief Set trigger state
         * @param isTrigger New trigger state
         */
        void SetTrigger(bool isTrigger) { m_properties.isTrigger = isTrigger; }

        /**
         * @brief Get density for mass calculation
         * @return Density
         */
        float GetDensity() const { return m_properties.density; }

        /**
         * @brief Set density for mass calculation
         * @param density New density
         */
        void SetDensity(float density) { m_properties.density = density; }

        // Flags

        /**
         * @brief Check if flag is set
         * @param flag Flag to check
         * @return true if set, false otherwise
         */
        bool HasFlag(ColliderFlags flag) const { return (m_properties.flags & flag) != 0; }

        /**
         * @brief Set flag
         * @param flag Flag to set
         */
        void SetFlag(ColliderFlags flag);

        /**
         * @brief Clear flag
         * @param flag Flag to clear
         */
        void ClearFlag(ColliderFlags flag);

        // Custom properties

        /**
         * @brief Set custom property
         * @tparam T Property type
         * @param key Property key
         * @param value Property value
         */
        template<typename T>
        void SetProperty(const std::string& key, const T& value);

        /**
         * @brief Get custom property
         * @tparam T Property type
         * @param key Property key
         * @param defaultValue Default value
         * @return Property value or default
         */
        template<typename T>
        T GetProperty(const std::string& key, const T& defaultValue = T{}) const;

        /**
         * @brief Check if property exists
         * @param key Property key
         * @return true if exists, false otherwise
         */
        bool HasProperty(const std::string& key) const;

        // Metrics and debugging

        /**
         * @brief Get collider metrics
         * @return Performance metrics
         */
        const ColliderMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

        /**
         * @brief Get debug information
         * @return Debug information string
         */
        virtual std::string GetDebugInfo() const;

        /**
         * @brief Validate collider state
         * @return true if valid, false otherwise
         */
        virtual bool Validate() const;

    protected:
        /**
         * @brief Update world space bounding volumes
         */
        virtual void UpdateWorldBounds();

        /**
         * @brief Update metrics
         */
        void UpdateMetrics();

        uint32_t m_id;                      ///< Unique collider ID
        ColliderType m_type;                ///< Collider type
        ColliderProperties m_properties;    ///< Collider properties
        RigidBody* m_rigidBody;             ///< Associated rigid body
        ColliderMetrics m_metrics;          ///< Performance metrics

        // Cached world space bounds
        mutable AABB m_worldAABB;           ///< World space AABB
        mutable BoundingSphere m_worldBoundingSphere; ///< World space bounding sphere
        mutable bool m_boundsDirty;         ///< Bounds need update flag

        static std::atomic<uint32_t> s_nextColliderId; ///< Next collider ID
    };

    // Template implementations

    template<typename T>
    void Collider::SetProperty(const std::string& key, const T& value) {
        m_properties.customProperties[key] = value;
    }

    template<typename T>
    T Collider::GetProperty(const std::string& key, const T& defaultValue) const {
        auto it = m_properties.customProperties.find(key);
        if (it != m_properties.customProperties.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast&) {
                return defaultValue;
            }
        }
        return defaultValue;
    }

    // Specific collider implementations

    /**
     * @class SphereCollider
     * @brief Spherical collision shape
     */
    class SphereCollider : public Collider {
    public:
        explicit SphereCollider(float radius = 0.5f);

        /**
         * @brief Get sphere radius
         * @return Radius
         */
        float GetRadius() const { return m_radius; }

        /**
         * @brief Set sphere radius
         * @param radius New radius
         */
        void SetRadius(float radius);

    protected:
        AABB GetLocalAABB() const override;
        BoundingSphere GetLocalBoundingSphere() const override;
        bool CheckCollision(const Collider* other, glm::vec3& contactPoint) const override;
        float GetPenetrationDepth(const Collider* other) const override;
        glm::vec3 GetCollisionNormal(const Collider* other) const override;
        bool Raycast(const Ray& ray, RaycastHit& hitInfo) const override;
        glm::vec3 GetClosestPoint(const glm::vec3& point) const override;
        bool ContainsPoint(const glm::vec3& point) const override;
        float GetVolume() const override;
        float GetSurfaceArea() const override;
        glm::mat3 GetInertiaTensor(float mass) const override;

    private:
        float m_radius;                    ///< Sphere radius
    };

    /**
     * @class BoxCollider
     * @brief Axis-aligned box collision shape
     */
    class BoxCollider : public Collider {
    public:
        explicit BoxCollider(const glm::vec3& halfExtents = glm::vec3(0.5f));

        /**
         * @brief Get box half extents
         * @return Half extents
         */
        const glm::vec3& GetHalfExtents() const { return m_halfExtents; }

        /**
         * @brief Set box half extents
         * @param halfExtents New half extents
         */
        void SetHalfExtents(const glm::vec3& halfExtents);

    protected:
        AABB GetLocalAABB() const override;
        BoundingSphere GetLocalBoundingSphere() const override;
        bool CheckCollision(const Collider* other, glm::vec3& contactPoint) const override;
        float GetPenetrationDepth(const Collider* other) const override;
        glm::vec3 GetCollisionNormal(const Collider* other) const override;
        bool Raycast(const Ray& ray, RaycastHit& hitInfo) const override;
        glm::vec3 GetClosestPoint(const glm::vec3& point) const override;
        bool ContainsPoint(const glm::vec3& point) const override;
        float GetVolume() const override;
        float GetSurfaceArea() const override;
        glm::mat3 GetInertiaTensor(float mass) const override;

    private:
        glm::vec3 m_halfExtents;           ///< Box half extents
    };

    /**
     * @class CapsuleCollider
     * @brief Capsule collision shape
     */
    class CapsuleCollider : public Collider {
    public:
        CapsuleCollider(float radius = 0.5f, float height = 1.0f);

        /**
         * @brief Get capsule radius
         * @return Radius
         */
        float GetRadius() const { return m_radius; }

        /**
         * @brief Set capsule radius
         * @param radius New radius
         */
        void SetRadius(float radius);

        /**
         * @brief Get capsule height
         * @return Height
         */
        float GetHeight() const { return m_height; }

        /**
         * @brief Set capsule height
         * @param height New height
         */
        void SetHeight(float height);

    protected:
        AABB GetLocalAABB() const override;
        BoundingSphere GetLocalBoundingSphere() const override;
        bool CheckCollision(const Collider* other, glm::vec3& contactPoint) const override;
        float GetPenetrationDepth(const Collider* other) const override;
        glm::vec3 GetCollisionNormal(const Collider* other) const override;
        bool Raycast(const Ray& ray, RaycastHit& hitInfo) const override;
        glm::vec3 GetClosestPoint(const glm::vec3& point) const override;
        bool ContainsPoint(const glm::vec3& point) const override;
        float GetVolume() const override;
        float GetSurfaceArea() const override;
        glm::mat3 GetInertiaTensor(float mass) const override;

    private:
        float m_radius;                    ///< Capsule radius
        float m_height;                    ///< Capsule height
    };

    /**
     * @class MeshCollider
     * @brief Triangle mesh collision shape
     */
    class MeshCollider : public Collider {
    public:
        MeshCollider(const std::vector<glm::vec3>& vertices = {},
                    const std::vector<uint32_t>& indices = {});

        /**
         * @brief Set mesh data
         * @param vertices Vertex array
         * @param indices Index array
         */
        void SetMeshData(const std::vector<glm::vec3>& vertices,
                        const std::vector<uint32_t>& indices);

        /**
         * @brief Get mesh vertices
         * @return Vertex array
         */
        const std::vector<glm::vec3>& GetVertices() const { return m_vertices; }

        /**
         * @brief Get mesh indices
         * @return Index array
         */
        const std::vector<uint32_t>& GetIndices() const { return m_indices; }

        /**
         * @brief Get mesh triangles
         * @return Triangle array
         */
        const std::vector<Triangle>& GetTriangles() const { return m_triangles; }

    protected:
        AABB GetLocalAABB() const override;
        BoundingSphere GetLocalBoundingSphere() const override;
        bool CheckCollision(const Collider* other, glm::vec3& contactPoint) const override;
        float GetPenetrationDepth(const Collider* other) const override;
        glm::vec3 GetCollisionNormal(const Collider* other) const override;
        bool Raycast(const Ray& ray, RaycastHit& hitInfo) const override;
        glm::vec3 GetClosestPoint(const glm::vec3& point) const override;
        bool ContainsPoint(const glm::vec3& point) const override;
        float GetVolume() const override;
        float GetSurfaceArea() const override;
        glm::mat3 GetInertiaTensor(float mass) const override;

    private:
        /**
         * @brief Build triangle list from mesh data
         */
        void BuildTriangles();

        std::vector<glm::vec3> m_vertices;  ///< Mesh vertices
        std::vector<uint32_t> m_indices;    ///< Mesh indices
        std::vector<Triangle> m_triangles;  ///< Mesh triangles
    };

    /**
     * @class VoxelCollider
     * @brief Voxel-based collision shape
     */
    class VoxelCollider : public Collider {
    public:
        VoxelCollider(const glm::vec3& size = glm::vec3(1.0f));

        /**
         * @brief Set voxel size
         * @param size Voxel size
         */
        void SetVoxelSize(const glm::vec3& size) { m_voxelSize = size; }

        /**
         * @brief Get voxel size
         * @return Voxel size
         */
        const glm::vec3& GetVoxelSize() const { return m_voxelSize; }

    protected:
        AABB GetLocalAABB() const override;
        BoundingSphere GetLocalBoundingSphere() const override;
        bool CheckCollision(const Collider* other, glm::vec3& contactPoint) const override;
        float GetPenetrationDepth(const Collider* other) const override;
        glm::vec3 GetCollisionNormal(const Collider* other) const override;
        bool Raycast(const Ray& ray, RaycastHit& hitInfo) const override;
        glm::vec3 GetClosestPoint(const glm::vec3& point) const override;
        bool ContainsPoint(const glm::vec3& point) const override;
        float GetVolume() const override;
        float GetSurfaceArea() const override;
        glm::mat3 GetInertiaTensor(float mass) const override;

    private:
        glm::vec3 m_voxelSize;             ///< Size of voxel
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_PHYSICS_COLLIDER_HPP
