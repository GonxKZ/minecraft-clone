/**
 * @file PhysicsUtils.hpp
 * @brief VoxelCraft Physics Utilities - Minecraft-like Physics
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_CORE_PHYSICS_UTILS_HPP
#define VOXELCRAFT_CORE_PHYSICS_UTILS_HPP

#include <cmath>
#include <algorithm>
#include "GameConstants.hpp"

namespace VoxelCraft {

    /**
     * @struct Vec3
     * @brief 3D Vector structure
     */
    struct Vec3 {
        float x, y, z;

        Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}

        // Vector operations
        Vec3 operator+(const Vec3& other) const {
            return Vec3(x + other.x, y + other.y, z + other.z);
        }

        Vec3 operator-(const Vec3& other) const {
            return Vec3(x - other.x, y - other.y, z - other.z);
        }

        Vec3 operator*(float scalar) const {
            return Vec3(x * scalar, y * scalar, z * scalar);
        }

        Vec3 operator/(float scalar) const {
            return Vec3(x / scalar, y / scalar, z / scalar);
        }

        Vec3& operator+=(const Vec3& other) {
            x += other.x; y += other.y; z += other.z;
            return *this;
        }

        Vec3& operator-=(const Vec3& other) {
            x -= other.x; y -= other.y; z -= other.z;
            return *this;
        }

        Vec3& operator*=(float scalar) {
            x *= scalar; y *= scalar; z *= scalar;
            return *this;
        }

        Vec3& operator/=(float scalar) {
            x /= scalar; y /= scalar; z /= scalar;
            return *this;
        }

        // Vector properties
        float length() const {
            return std::sqrt(x * x + y * y + z * z);
        }

        float lengthSquared() const {
            return x * x + y * y + z * z;
        }

        Vec3 normalized() const {
            float len = length();
            return len > 0 ? *this / len : Vec3(0, 0, 0);
        }

        // Dot product
        float dot(const Vec3& other) const {
            return x * other.x + y * other.y + z * other.z;
        }

        // Cross product
        Vec3 cross(const Vec3& other) const {
            return Vec3(
                y * other.z - z * other.y,
                z * other.x - x * other.z,
                x * other.y - y * other.x
            );
        }
    };

    /**
     * @struct Vec2
     * @brief 2D Vector structure
     */
    struct Vec2 {
        float x, y;

        Vec2(float x = 0, float y = 0) : x(x), y(y) {}

        Vec2 operator+(const Vec2& other) const {
            return Vec2(x + other.x, y + other.y);
        }

        Vec2 operator-(const Vec2& other) const {
            return Vec2(x - other.x, y - other.y);
        }

        Vec2 operator*(float scalar) const {
            return Vec2(x * scalar, y * scalar);
        }

        float length() const {
            return std::sqrt(x * x + y * y);
        }

        Vec2 normalized() const {
            float len = length();
            return len > 0 ? *this / len : Vec2(0, 0);
        }
    };

    /**
     * @struct AABB
     * @brief Axis-Aligned Bounding Box
     */
    struct AABB {
        Vec3 min;
        Vec3 max;

        AABB(const Vec3& min = Vec3(), const Vec3& max = Vec3())
            : min(min), max(max) {}

        /**
         * @brief Check if point is inside AABB
         */
        bool contains(const Vec3& point) const {
            return point.x >= min.x && point.x <= max.x &&
                   point.y >= min.y && point.y <= max.y &&
                   point.z >= min.z && point.z <= max.z;
        }

        /**
         * @brief Check if two AABBs intersect
         */
        bool intersects(const AABB& other) const {
            return min.x <= other.max.x && max.x >= other.min.x &&
                   min.y <= other.max.y && max.y >= other.min.y &&
                   min.z <= other.max.z && max.z >= other.min.z;
        }

        /**
         * @brief Expand AABB to include point
         */
        void expand(const Vec3& point) {
            min.x = std::min(min.x, point.x);
            min.y = std::min(min.y, point.y);
            min.z = std::min(min.z, point.z);
            max.x = std::max(max.x, point.x);
            max.y = std::max(max.y, point.y);
            max.z = std::max(max.z, point.z);
        }

        /**
         * @brief Get AABB center
         */
        Vec3 center() const {
            return Vec3(
                (min.x + max.x) * 0.5f,
                (min.y + max.y) * 0.5f,
                (min.z + max.z) * 0.5f
            );
        }

        /**
         * @brief Get AABB size
         */
        Vec3 size() const {
            return Vec3(
                max.x - min.x,
                max.y - min.y,
                max.z - min.z
            );
        }
    };

    /**
     * @namespace PhysicsUtils
     * @brief Physics utility functions matching Minecraft mechanics
     */
    namespace PhysicsUtils {

        /**
         * @brief Apply gravity to velocity
         * @param velocity Current velocity
         * @param deltaTime Time step
         * @return New velocity with gravity applied
         */
        inline Vec3 applyGravity(const Vec3& velocity, float deltaTime) {
            return Vec3(
                velocity.x,
                velocity.y - GameConstants::GRAVITY * deltaTime,
                velocity.z
            );
        }

        /**
         * @brief Apply terminal velocity limit
         * @param velocity Current velocity
         * @return Clamped velocity
         */
        inline Vec3 applyTerminalVelocity(const Vec3& velocity) {
            Vec3 result = velocity;
            if (result.y < -GameConstants::TERMINAL_VELOCITY) {
                result.y = -GameConstants::TERMINAL_VELOCITY;
            }
            return result;
        }

        /**
         * @brief Apply friction to velocity
         * @param velocity Current velocity
         * @param friction Friction coefficient
         * @return New velocity with friction applied
         */
        inline Vec3 applyFriction(const Vec3& velocity, float friction) {
            return Vec3(
                velocity.x * friction,
                velocity.y, // Don't apply friction to Y axis (gravity)
                velocity.z * friction
            );
        }

        /**
         * @brief Apply ground friction (higher friction when on ground)
         * @param velocity Current velocity
         * @param onGround Whether entity is on ground
         * @return New velocity with ground friction applied
         */
        inline Vec3 applyGroundFriction(const Vec3& velocity, bool onGround) {
            if (onGround) {
                return applyFriction(velocity, GameConstants::FRICTION);
            } else {
                return applyFriction(velocity, GameConstants::AIR_FRICTION);
            }
        }

        /**
         * @brief Apply fluid friction (water/lava)
         * @param velocity Current velocity
         * @param fluidType Type of fluid (0=air, 1=water, 2=lava)
         * @return New velocity with fluid friction applied
         */
        inline Vec3 applyFluidFriction(const Vec3& velocity, int fluidType) {
            switch (fluidType) {
                case 1: // Water
                    return applyFriction(velocity, GameConstants::WATER_FRICTION);
                case 2: // Lava
                    return applyFriction(velocity, GameConstants::LAVA_FRICTION);
                default: // Air
                    return applyFriction(velocity, GameConstants::AIR_FRICTION);
            }
        }

        /**
         * @brief Calculate jump velocity
         * @param jumpHeight Desired jump height in blocks
         * @return Initial jump velocity
         */
        inline float calculateJumpVelocity(float jumpHeight = GameConstants::PLAYER_JUMP_VELOCITY) {
            // Using kinematic equation: v = sqrt(2gh)
            return std::sqrt(2.0f * GameConstants::GRAVITY * jumpHeight);
        }

        /**
         * @brief Calculate fall damage
         * @param fallDistance Distance fallen (in blocks)
         * @return Damage amount (0 if no damage)
         */
        inline float calculateFallDamage(float fallDistance) {
            if (fallDistance <= GameConstants::PLAYER_MAX_FALL_DISTANCE) {
                return 0.0f; // No damage for small falls
            }
            // Minecraft formula: damage = (fallDistance - 3) * 0.5
            return (fallDistance - GameConstants::PLAYER_MAX_FALL_DISTANCE) * 0.5f;
        }

        /**
         * @brief Check if entity can jump
         * @param onGround Whether entity is on ground
         * @param inWater Whether entity is in water
         * @return true if can jump
         */
        inline bool canJump(bool onGround, bool inWater) {
            return onGround || inWater;
        }

        /**
         * @brief Calculate horizontal movement speed
         * @param baseSpeed Base movement speed
         * @param sprinting Whether sprinting
         * @param sneaking Whether sneaking
         * @param inWater Whether in water
         * @return Final movement speed
         */
        inline float calculateMovementSpeed(float baseSpeed, bool sprinting,
                                          bool sneaking, bool inWater) {
            float speed = baseSpeed;

            if (sprinting) {
                speed = GameConstants::PLAYER_SPRINT_SPEED;
            } else if (sneaking) {
                speed = GameConstants::PLAYER_SNEAK_SPEED;
            }

            if (inWater) {
                speed *= 0.5f; // Swimming is slower
            }

            return speed;
        }

        /**
         * @brief Check collision between two AABBs
         * @param a First AABB
         * @param b Second AABB
         * @return true if colliding
         */
        inline bool checkCollision(const AABB& a, const AABB& b) {
            return a.intersects(b);
        }

        /**
         * @brief Create AABB for block at position
         * @param position Block position
         * @return Block AABB
         */
        inline AABB createBlockAABB(const Vec3& position) {
            return AABB(
                position,
                Vec3(position.x + 1.0f, position.y + 1.0f, position.z + 1.0f)
            );
        }

        /**
         * @brief Create AABB for entity
         * @param position Entity position
         * @param width Entity width
         * @param height Entity height
         * @return Entity AABB
         */
        inline AABB createEntityAABB(const Vec3& position, float width, float height) {
            return AABB(
                Vec3(position.x - width * 0.5f, position.y, position.z - width * 0.5f),
                Vec3(position.x + width * 0.5f, position.y + height, position.z + width * 0.5f)
            );
        }

        /**
         * @brief Calculate distance between two points
         * @param a First point
         * @param b Second point
         * @return Distance
         */
        inline float distance(const Vec3& a, const Vec3& b) {
            Vec3 diff = b - a;
            return diff.length();
        }

        /**
         * @brief Calculate squared distance between two points (faster)
         * @param a First point
         * @param b Second point
         * @return Squared distance
         */
        inline float distanceSquared(const Vec3& a, const Vec3& b) {
            Vec3 diff = b - a;
            return diff.lengthSquared();
        }

        /**
         * @brief Linear interpolation between two vectors
         * @param a Start vector
         * @param b End vector
         * @param t Interpolation factor (0-1)
         * @return Interpolated vector
         */
        inline Vec3 lerp(const Vec3& a, const Vec3& b, float t) {
            t = std::clamp(t, 0.0f, 1.0f);
            return a + (b - a) * t;
        }

        /**
         * @brief Smooth step interpolation
         * @param edge0 Start edge
         * @param edge1 End edge
         * @param x Input value
         * @return Smoothed value
         */
        inline float smoothstep(float edge0, float edge1, float x) {
            x = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
            return x * x * (3.0f - 2.0f * x);
        }

        /**
         * @brief Clamp value between min and max
         * @param value Input value
         * @param min Minimum value
         * @param max Maximum value
         * @return Clamped value
         */
        inline float clamp(float value, float min, float max) {
            return std::clamp(value, min, max);
        }

        /**
         * @brief Clamp vector components between min and max
         * @param value Input vector
         * @param min Minimum values
         * @param max Maximum values
         * @return Clamped vector
         */
        inline Vec3 clamp(const Vec3& value, const Vec3& min, const Vec3& max) {
            return Vec3(
                std::clamp(value.x, min.x, max.x),
                std::clamp(value.y, min.y, max.y),
                std::clamp(value.z, min.z, max.z)
            );
        }

    } // namespace PhysicsUtils

} // namespace VoxelCraft

#endif // VOXELCRAFT_CORE_PHYSICS_UTILS_HPP
