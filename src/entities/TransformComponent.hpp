/**
 * @file TransformComponent.hpp
 * @brief VoxelCraft ECS - Transform Component
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * Transform component for position, rotation, and scale in 3D space.
 */

#ifndef VOXELCRAFT_ENTITIES_TRANSFORM_COMPONENT_HPP
#define VOXELCRAFT_ENTITIES_TRANSFORM_COMPONENT_HPP

#include "Component.hpp"
#include <array>
#include <cmath>
#include <algorithm>

// Simple vector and quaternion implementations for when GLM is not available
namespace VoxelCraft {

    struct Vec3 {
        float x, y, z;

        Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
        Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

        Vec3 operator+(const Vec3& other) const { return Vec3(x + other.x, y + other.y, z + other.z); }
        Vec3 operator-(const Vec3& other) const { return Vec3(x - other.x, y - other.y, z - other.z); }
        Vec3 operator*(float scalar) const { return Vec3(x * scalar, y * scalar, z * scalar); }
        Vec3 operator/(float scalar) const { return Vec3(x / scalar, y / scalar, z / scalar); }

        Vec3& operator+=(const Vec3& other) { x += other.x; y += other.y; z += other.z; return *this; }
        Vec3& operator-=(const Vec3& other) { x -= other.x; y -= other.y; z -= other.z; return *this; }
        Vec3& operator*=(float scalar) { x *= scalar; y *= scalar; z *= scalar; return *this; }
        Vec3& operator/=(float scalar) { x /= scalar; y /= scalar; z /= scalar; return *this; }

        float length() const { return std::sqrt(x * x + y * y + z * z); }
        Vec3 normalize() const { float len = length(); return len > 0.0001f ? *this / len : Vec3(); }
        float dot(const Vec3& other) const { return x * other.x + y * other.y + z * other.z; }
        Vec3 cross(const Vec3& other) const {
            return Vec3(
                y * other.z - z * other.y,
                z * other.x - x * other.z,
                x * other.y - y * other.x
            );
        }

        static Vec3 zero() { return Vec3(0.0f, 0.0f, 0.0f); }
        static Vec3 up() { return Vec3(0.0f, 1.0f, 0.0f); }
        static Vec3 forward() { return Vec3(0.0f, 0.0f, -1.0f); }
        static Vec3 right() { return Vec3(1.0f, 0.0f, 0.0f); }
    };

    struct Quat {
        float w, x, y, z;

        Quat() : w(1.0f), x(0.0f), y(0.0f), z(0.0f) {}
        Quat(float w_, float x_, float y_, float z_) : w(w_), x(x_), y(y_), z(z_) {}

        // Create quaternion from Euler angles (degrees)
        static Quat fromEuler(float pitch, float yaw, float roll) {
            float cosPitch = std::cos(pitch * 0.0174532925f * 0.5f);
            float sinPitch = std::sin(pitch * 0.0174532925f * 0.5f);
            float cosYaw = std::cos(yaw * 0.0174532925f * 0.5f);
            float sinYaw = std::sin(yaw * 0.0174532925f * 0.5f);
            float cosRoll = std::cos(roll * 0.0174532925f * 0.5f);
            float sinRoll = std::sin(roll * 0.0174532925f * 0.5f);

            return Quat(
                cosRoll * cosPitch * cosYaw + sinRoll * sinPitch * sinYaw,
                sinRoll * cosPitch * cosYaw - cosRoll * sinPitch * sinYaw,
                cosRoll * sinPitch * cosYaw + sinRoll * cosPitch * sinYaw,
                cosRoll * cosPitch * sinYaw - sinRoll * sinPitch * cosYaw
            );
        }

        Quat operator*(const Quat& other) const {
            return Quat(
                w * other.w - x * other.x - y * other.y - z * other.z,
                w * other.x + x * other.w + y * other.z - z * other.y,
                w * other.y - x * other.z + y * other.w + z * other.x,
                w * other.z + x * other.y - y * other.x + z * other.w
            );
        }

        Quat& operator*=(const Quat& other) { *this = *this * other; return *this; }

        Vec3 operator*(const Vec3& v) const {
            Vec3 qVec(x, y, z);
            Vec3 uv = qVec.cross(v);
            Vec3 uuv = qVec.cross(uv);
            return v + uv * (2.0f * w) + uuv * 2.0f;
        }

        Quat normalize() const {
            float len = std::sqrt(w * w + x * x + y * y + z * z);
            if (len > 0.0001f) {
                return Quat(w / len, x / len, y / len, z / len);
            }
            return Quat(1.0f, 0.0f, 0.0f, 0.0f);
        }

        Quat inverse() const {
            float len2 = w * w + x * x + y * y + z * z;
            return Quat(w / len2, -x / len2, -y / len2, -z / len2);
        }
    };

    // Simple 4x4 matrix implementation
    struct Mat4 {
        std::array<float, 16> data;

        Mat4() {
            data.fill(0.0f);
            data[0] = data[5] = data[10] = data[15] = 1.0f; // Identity matrix
        }

        static Mat4 identity() { return Mat4(); }

        static Mat4 translate(const Vec3& v) {
            Mat4 m;
            m.data[12] = v.x;
            m.data[13] = v.y;
            m.data[14] = v.z;
            return m;
        }

        static Mat4 rotate(const Quat& q) {
            float x2 = q.x + q.x, y2 = q.y + q.y, z2 = q.z + q.z;
            float xx = q.x * x2, xy = q.x * y2, xz = q.x * z2;
            float yy = q.y * y2, yz = q.y * z2, zz = q.z * z2;
            float wx = q.w * x2, wy = q.w * y2, wz = q.w * z2;

            Mat4 m;
            m.data[0] = 1.0f - (yy + zz);
            m.data[1] = xy + wz;
            m.data[2] = xz - wy;
            m.data[3] = 0.0f;

            m.data[4] = xy - wz;
            m.data[5] = 1.0f - (xx + zz);
            m.data[6] = yz + wx;
            m.data[7] = 0.0f;

            m.data[8] = xz + wy;
            m.data[9] = yz - wx;
            m.data[10] = 1.0f - (xx + yy);
            m.data[11] = 0.0f;

            m.data[12] = 0.0f;
            m.data[13] = 0.0f;
            m.data[14] = 0.0f;
            m.data[15] = 1.0f;

            return m;
        }

        static Mat4 scale(const Vec3& v) {
            Mat4 m;
            m.data[0] = v.x;
            m.data[5] = v.y;
            m.data[10] = v.z;
            return m;
        }

        Mat4 operator*(const Mat4& other) const {
            Mat4 result;
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j) {
                    result.data[i * 4 + j] = 0.0f;
                    for (int k = 0; k < 4; ++k) {
                        result.data[i * 4 + j] += data[i * 4 + k] * other.data[k * 4 + j];
                    }
                }
            }
            return result;
        }

        Vec3 operator*(const Vec3& v) const {
            Vec3 result;
            result.x = data[0] * v.x + data[4] * v.y + data[8] * v.z + data[12];
            result.y = data[1] * v.x + data[5] * v.y + data[9] * v.z + data[13];
            result.z = data[2] * v.x + data[6] * v.y + data[10] * v.z + data[14];
            return result;
        }
    };

} // namespace VoxelCraft

namespace VoxelCraft {

    class TransformComponent : public Component {
    public:
        /**
         * @brief Constructor
         * @param owner Entity that owns this component
         */
        TransformComponent(Entity* owner);

        /**
         * @brief Constructor with initial position
         * @param owner Entity that owns this component
         * @param position Initial position
         */
        TransformComponent(Entity* owner, const Vec3& position);

        /**
         * @brief Constructor with full transform
         * @param owner Entity that owns this component
         * @param position Initial position
         * @param rotation Initial rotation
         * @param scale Initial scale
         */
        TransformComponent(Entity* owner,
                          const Vec3& position,
                          const Quat& rotation = Quat(),
                          const Vec3& scale = Vec3(1.0f, 1.0f, 1.0f));

        /**
         * @brief Destructor
         */
        ~TransformComponent() override;

        /**
         * @brief Get component type name
         * @return Component type name
         */
        std::string GetTypeName() const override { return "TransformComponent"; }

        /**
         * @brief Initialize the component
         * @return true if successful, false otherwise
         */
        bool Initialize() override;

        /**
         * @brief Update the component
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime) override;

        // Position operations

        /**
         * @brief Set position
         * @param position New position
         */
        void SetPosition(const Vec3& position);

        /**
         * @brief Set position with individual coordinates
         * @param x X coordinate
         * @param y Y coordinate
         * @param z Z coordinate
         */
        void SetPosition(float x, float y, float z);

        /**
         * @brief Get current position
         * @return Current position
         */
        const Vec3& GetPosition() const { return m_position; }

        /**
         * @brief Translate by vector
         * @param translation Translation vector
         */
        void Translate(const Vec3& translation);

        /**
         * @brief Translate by individual coordinates
         * @param x X translation
         * @param y Y translation
         * @param z Z translation
         */
        void Translate(float x, float y, float z);

        // Rotation operations

        /**
         * @brief Set rotation using quaternion
         * @param rotation New rotation quaternion
         */
        void SetRotation(const Quat& rotation);

        /**
         * @brief Set rotation using Euler angles (degrees)
         * @param pitch Pitch angle
         * @param yaw Yaw angle
         * @param roll Roll angle
         */
        void SetRotation(float pitch, float yaw, float roll);

        /**
         * @brief Rotate by quaternion
         * @param rotation Rotation quaternion to apply
         */
        void Rotate(const Quat& rotation);

        /**
         * @brief Rotate around axis by angle
         * @param axis Rotation axis
         * @param angle Rotation angle in degrees
         */
        void Rotate(const Vec3& axis, float angle);

        /**
         * @brief Get current rotation
         * @return Current rotation quaternion
         */
        const Quat& GetRotation() const { return m_rotation; }

        /**
         * @brief Get forward direction vector
         * @return Forward direction
         */
        Vec3 GetForward() const;

        /**
         * @brief Get right direction vector
         * @return Right direction
         */
        Vec3 GetRight() const;

        /**
         * @brief Get up direction vector
         * @return Up direction
         */
        Vec3 GetUp() const;

        /**
         * @brief Look at target position
         * @param target Target position to look at
         * @param up Up vector for orientation
         */
        void LookAt(const Vec3& target, const Vec3& up = Vec3::up());

        // Scale operations

        /**
         * @brief Set scale
         * @param scale New scale vector
         */
        void SetScale(const Vec3& scale);

        /**
         * @brief Set uniform scale
         * @param scale Uniform scale value
         */
        void SetScale(float scale);

        /**
         * @brief Set scale with individual coordinates
         * @param x X scale
         * @param y Y scale
         * @param z Z scale
         */
        void SetScale(float x, float y, float z);

        /**
         * @brief Get current scale
         * @return Current scale vector
         */
        const Vec3& GetScale() const { return m_scale; }

        /**
         * @brief Scale by vector
         * @param scale Scale vector to apply
         */
        void Scale(const Vec3& scale);

        /**
         * @brief Scale uniformly
         * @param scale Scale factor to apply
         */
        void Scale(float scale);

        // Matrix operations

        /**
         * @brief Get world transformation matrix
         * @return 4x4 transformation matrix
         */
        Mat4 GetWorldMatrix() const;

        /**
         * @brief Get local transformation matrix
         * @return 4x4 transformation matrix
         */
        Mat4 GetLocalMatrix() const;

        /**
         * @brief Mark transform as dirty (forces matrix recalculation)
         */
        void MarkDirty();

        /**
         * @brief Check if transform is dirty
         * @return true if dirty, false otherwise
         */
        bool IsDirty() const { return m_isDirty; }

        // Hierarchy operations (if needed)

        /**
         * @brief Set parent transform
         * @param parent Parent transform component
         */
        void SetParent(TransformComponent* parent);

        /**
         * @brief Get parent transform
         * @return Parent transform component or nullptr
         */
        TransformComponent* GetParent() const { return m_parent; }

        /**
         * @brief Add child transform
         * @param child Child transform component
         */
        void AddChild(TransformComponent* child);

        /**
         * @brief Remove child transform
         * @param child Child transform component to remove
         */
        void RemoveChild(TransformComponent* child);

        /**
         * @brief Get children transforms
         * @return Vector of child transform components
         */
        const std::vector<TransformComponent*>& GetChildren() const { return m_children; }

        /**
         * @brief Get world position (including parent transforms)
         * @return World position
         */
        Vec3 GetWorldPosition() const;

        /**
         * @brief Get world rotation (including parent transforms)
         * @return World rotation quaternion
         */
        Quat GetWorldRotation() const;

        /**
         * @brief Get world scale (including parent transforms)
         * @return World scale
         */
        Vec3 GetWorldScale() const;

        /**
         * @brief Get world transformation matrix (including parent transforms)
         * @return World 4x4 transformation matrix
         */
        Mat4 GetWorldMatrixHierarchy() const;

    private:
        /**
         * @brief Update transformation matrices if dirty
         */
        void UpdateMatrices() const;

        /**
         * @brief Recalculate world matrix recursively
         * @param parentWorldMatrix Parent world matrix
         * @return Updated world matrix
         */
        Mat4 CalculateWorldMatrix(const Mat4& parentWorldMatrix) const;

        Vec3 m_position;                         ///< Local position
        Quat m_rotation;                         ///< Local rotation (quaternion)
        Vec3 m_scale;                            ///< Local scale

        mutable Mat4 m_localMatrix;              ///< Cached local transformation matrix
        mutable Mat4 m_worldMatrix;              ///< Cached world transformation matrix
        mutable bool m_isDirty;                  ///< Dirty flag for matrix recalculation

        // Hierarchy support
        TransformComponent* m_parent;            ///< Parent transform
        std::vector<TransformComponent*> m_children; ///< Child transforms
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_ENTITIES_TRANSFORM_COMPONENT_HPP
