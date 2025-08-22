/**
 * @file PhysicsTypes.hpp
 * @brief VoxelCraft Physics Common Types and Structures
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines common types, structures, and enums used throughout
 * the physics system for collision detection, raycasting, and debugging.
 */

#ifndef VOXELCRAFT_PHYSICS_PHYSICS_TYPES_HPP
#define VOXELCRAFT_PHYSICS_PHYSICS_TYPES_HPP

#include <vector>
#include <string>
#include <array>
#include <glm/glm.hpp>

namespace VoxelCraft {

    // Forward declarations
    class RigidBody;
    class Collider;

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
     * @struct VoxelCollisionData
     * @brief Data for voxel-based collision
     */
    struct VoxelCollisionData {
        glm::vec3 position;                ///< Voxel position
        glm::vec3 size;                    ///< Voxel size
        bool isSolid;                      ///< Whether voxel is solid
        bool isDestructible;               ///< Whether voxel can be destroyed
        float hardness;                    ///< Voxel hardness
        float friction;                    ///< Voxel friction
        float restitution;                 ///< Voxel restitution
        std::string materialName;          ///< Voxel material name
        uint32_t materialIndex;            ///< Voxel material index
    };

    /**
     * @struct DebugRenderData
     * @brief Debug rendering data for physics visualization
     */
    struct DebugRenderData {
        enum class Type {
            Line,                          ///< Line segment
            Sphere,                        ///< Sphere
            Box,                           ///< Axis-aligned box
            Capsule,                       ///< Capsule
            Arrow,                         ///< Arrow
            Text,                          ///< Text
            ContactPoint,                  ///< Contact point
            AABB,                          ///< Axis-aligned bounding box
            OBB                            ///< Oriented bounding box
        };

        Type type;                         ///< Debug data type
        glm::vec3 position;                ///< Position
        glm::vec3 size;                    ///< Size (for boxes, spheres, etc.)
        glm::vec3 direction;               ///< Direction (for arrows, capsules)
        glm::vec4 color;                   ///< Color (RGBA)
        float radius;                      ///< Radius (for spheres, capsules)
        float length;                      ///< Length (for capsules, arrows)
        std::string text;                  ///< Text content
        float duration;                    ///< Display duration (seconds)
        bool depthTest;                    ///< Enable depth testing
    };

    /**
     * @struct ConstraintState
     * @brief State information for physics constraints
     */
    struct ConstraintState {
        glm::vec3 impulse;                 ///< Accumulated impulse
        glm::vec3 angularImpulse;          ///< Accumulated angular impulse
        float lambda;                      ///< Lagrange multiplier
        float error;                       ///< Constraint error
        bool isActive;                     ///< Constraint is active
        double lastUpdate;                 ///< Last update timestamp
    };

    /**
     * @struct RigidBodyState
     * @brief Complete state of a rigid body
     */
    struct RigidBodyState {
        // Position and orientation
        glm::vec3 position;                 ///< World space position
        glm::quat orientation;              ///< World space orientation

        // Linear motion
        glm::vec3 linearVelocity;           ///< Linear velocity
        glm::vec3 linearAcceleration;       ///< Linear acceleration

        // Angular motion
        glm::vec3 angularVelocity;          ///< Angular velocity (radians/sec)
        glm::vec3 angularAcceleration;      ///< Angular acceleration

        // Forces and torques
        glm::vec3 totalForce;               ///< Total force applied
        glm::vec3 totalTorque;              ///< Total torque applied

        // Timestamps
        double lastUpdateTime;              ///< Last update timestamp
        double creationTime;                ///< Body creation timestamp
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
        bool isNew;                         ///< New manifold (not persistent)
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

} // namespace VoxelCraft

#endif // VOXELCRAFT_PHYSICS_PHYSICS_TYPES_HPP
