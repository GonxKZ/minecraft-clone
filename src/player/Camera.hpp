/**
 * @file Camera.hpp
 * @brief VoxelCraft Camera System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the Camera class that provides advanced camera
 * functionality for the player including third-person, first-person,
 * collision detection, and smooth camera movement.
 */

#ifndef VOXELCRAFT_PLAYER_CAMERA_HPP
#define VOXELCRAFT_PLAYER_CAMERA_HPP

#include <memory>
#include <vector>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>

#include "../core/Config.hpp"
#include "../physics/PhysicsTypes.hpp"
#include "../world/World.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Player;
    class World;
    class Entity;

    /**
     * @enum CameraMode
     * @brief Camera operational modes
     */
    enum class CameraMode {
        FirstPerson,            ///< First person camera
        ThirdPerson,            ///< Third person camera
        Free,                   ///< Free camera (no target)
        Orbital,                ///< Orbital camera around target
        Cinematic,              ///< Cinematic camera with path following
        Spectator,              ///< Spectator camera with collision
        Custom                  ///< Custom camera mode
    };

    /**
     * @enum CameraBehavior
     * @brief Camera behavior flags
     */
    enum class CameraBehavior : uint32_t {
        NONE                    = 0x0000,
        COLLISION_DETECTION     = 0x0001,      ///< Enable collision detection
        SMOOTH_FOLLOW           = 0x0002,      ///< Smooth following of target
        ZOOM_ON_COLLISION       = 0x0004,      ///< Zoom in when colliding
        AUTO_ADJUST_DISTANCE    = 0x0008,      ///< Auto-adjust distance
        LOOK_AT_TARGET          = 0x0010,      ///< Always look at target
        ORBIT_AROUND_TARGET     = 0x0020,      ///< Orbit around target
        ENABLE_SHAKE            = 0x0040,      ///< Enable camera shake
        ENABLE_DOF              = 0x0080,      ///< Enable depth of field
        ENABLE_MOTION_BLUR      = 0x0100,      ///< Enable motion blur
        ENABLE_VIGNETTE         = 0x0200,      ///< Enable vignette effect
        DEBUG_DRAW              = 0x1000,      ///< Enable debug drawing
        CUSTOM_BEHAVIOR_1       = 0x2000,      ///< Custom behavior 1
        CUSTOM_BEHAVIOR_2       = 0x4000,      ///< Custom behavior 2
        CUSTOM_BEHAVIOR_3       = 0x8000       ///< Custom behavior 3
    };

    /**
     * @struct CameraConfig
     * @brief Camera configuration settings
     */
    struct CameraConfig {
        // Basic settings
        CameraMode mode;                        ///< Camera mode
        uint32_t behaviorFlags;                 ///< Camera behavior flags

        // Position and orientation
        glm::vec3 position;                     ///< Camera world position
        glm::quat orientation;                  ///< Camera world orientation
        float distance;                         ///< Distance from target
        float minDistance;                      ///< Minimum camera distance
        float maxDistance;                      ///< Maximum camera distance

        // Field of view and perspective
        float fieldOfView;                      ///< Field of view (degrees)
        float nearPlane;                        ///< Near clipping plane
        float farPlane;                         ///< Far clipping plane
        float aspectRatio;                      ///< Aspect ratio

        // Movement and rotation
        float movementSpeed;                    ///< Camera movement speed
        float rotationSpeed;                    ///< Camera rotation speed
        float zoomSpeed;                        ///< Camera zoom speed
        bool invertY;                           ///< Invert Y axis rotation
        bool invertX;                           ///< Invert X axis rotation

        // Smoothing and interpolation
        float positionSmoothing;                ///< Position smoothing factor (0.0 - 1.0)
        float rotationSmoothing;                ///< Rotation smoothing factor (0.0 - 1.0)
        float zoomSmoothing;                    ///< Zoom smoothing factor (0.0 - 1.0)
        float followSmoothing;                  ///< Target following smoothing

        // Collision settings
        float collisionRadius;                  ///< Camera collision sphere radius
        float collisionStepDistance;            ///< Collision raycast step distance
        int maxCollisionIterations;             ///< Maximum collision resolution iterations
        bool enableTerrainCollision;            ///< Enable terrain collision
        bool enableEntityCollision;             ///< Enable entity collision

        // Orbital settings (for orbital mode)
        float orbitalHeight;                    ///< Orbital camera height
        float orbitalRadius;                    ///< Orbital camera radius
        float orbitalSpeed;                     ///< Orbital rotation speed
        bool orbitalAutoRotate;                 ///< Auto-rotate orbital camera

        // Effects settings
        float shakeIntensity;                   ///< Camera shake intensity
        float shakeFrequency;                   ///< Camera shake frequency
        float shakeDuration;                    ///< Camera shake duration
        float motionBlurStrength;               ///< Motion blur strength
        float vignetteStrength;                 ///< Vignette effect strength

        // Advanced settings
        bool enableFrustumCulling;              ///< Enable frustum culling
        bool enableOcclusionCulling;            ///< Enable occlusion culling
        bool enableLevelOfDetail;               ///< Enable LOD system
        float lodDistance;                      ///< LOD switch distance

        // Debug settings
        bool showDebugInfo;                     ///< Show debug information
        bool showCollisionGeometry;             ///< Show collision geometry
        bool showFrustum;                       ///< Show camera frustum
        bool showTargetInfo;                    ///< Show target information
    };

    /**
     * @struct CameraState
     * @brief Current state of the camera
     */
    struct CameraState {
        // Transform
        glm::vec3 position;                     ///< Current world position
        glm::quat orientation;                  ///< Current world orientation
        glm::vec3 targetPosition;               ///< Target world position
        glm::quat targetOrientation;            ///< Target world orientation

        // Movement
        glm::vec3 velocity;                     ///< Current velocity
        glm::vec3 angularVelocity;              ///< Current angular velocity
        float currentDistance;                  ///< Current distance from target
        float targetDistance;                   ///< Target distance from target

        // View parameters
        float currentFOV;                       ///< Current field of view
        float targetFOV;                        ///< Target field of view
        glm::mat4 viewMatrix;                   ///< Current view matrix
        glm::mat4 projectionMatrix;             ///< Current projection matrix
        glm::mat4 viewProjectionMatrix;         ///< Combined view-projection matrix

        // Collision and visibility
        bool isColliding;                       ///< Camera is colliding with geometry
        float collisionDistance;                ///< Distance to collision point
        bool targetVisible;                     ///< Target is visible from camera
        bool isInWater;                         ///< Camera is underwater
        bool isInLava;                          ///< Camera is in lava

        // Effects
        float shakeTimeRemaining;               ///< Remaining shake time
        glm::vec3 shakeOffset;                  ///< Current shake offset
        float motionBlurAmount;                 ///< Current motion blur amount
        float vignetteAmount;                   ///< Current vignette amount

        // Timing
        double lastUpdateTime;                  ///< Last update timestamp
        float deltaTime;                        ///< Last frame delta time
    };

    /**
     * @struct CameraMetrics
     * @brief Performance metrics for the camera system
     */
    struct CameraMetrics {
        // Performance metrics
        uint64_t updateCount;                   ///< Number of updates performed
        double totalUpdateTime;                 ///< Total update time (ms)
        double averageUpdateTime;               ///< Average update time (ms)
        double maxUpdateTime;                   ///< Maximum update time (ms)

        // Collision metrics
        uint64_t collisionTests;                ///< Collision tests performed
        uint64_t collisionHits;                 ///< Collision hits detected
        double averageCollisionTime;            ///< Average collision test time
        uint64_t collisionAvoided;              ///< Times collision was avoided

        // Visibility metrics
        uint64_t visibilityTests;               ///< Visibility tests performed
        uint64_t occlusionTests;                ///< Occlusion tests performed
        uint64_t objectsCulled;                 ///< Objects culled by frustum
        uint64_t objectsOccluded;               ///< Objects culled by occlusion

        // Movement metrics
        float totalDistanceTraveled;            ///< Total distance camera moved
        float averageSpeed;                     ///< Average camera speed
        float maxSpeed;                         ///< Maximum camera speed
        uint64_t targetLossEvents;              ///< Times target was lost

        // Effect metrics
        uint64_t shakeEvents;                   ///< Camera shake events
        uint64_t zoomEvents;                    ///< Camera zoom events
        uint64_t modeChanges;                   ///< Camera mode changes
        uint64_t behaviorChanges;               ///< Behavior flag changes

        // Render metrics
        uint32_t trianglesRendered;             ///< Triangles rendered this frame
        uint32_t drawCalls;                     ///< Draw calls this frame
        float renderTime;                       ///< Render time this frame
        float gpuTime;                          ///< GPU time this frame
    };

    /**
     * @class Camera
     * @brief Advanced camera system with collision detection and smooth movement
     *
     * The Camera class provides a comprehensive camera system for VoxelCraft
     * supporting multiple camera modes, collision detection, smooth following,
     * and various visual effects. It integrates with the physics system and
     * provides collision-aware camera positioning.
     *
     * Key Features:
     * - Multiple camera modes (first-person, third-person, orbital, free)
     * - Collision detection with world geometry and entities
     * - Smooth camera following and interpolation
     * - Camera shake and visual effects
     * - Frustum culling and occlusion culling
     * - Level of detail (LOD) system
     * - Debug visualization and profiling
     * - Integration with player character system
     *
     * Camera Modes:
     * - **First Person**: Camera positioned at player eye level
     * - **Third Person**: Camera follows player from behind with collision detection
     * - **Free**: Camera can be moved independently (no target following)
     * - **Orbital**: Camera orbits around a target point
     * - **Cinematic**: Camera follows predefined paths for cutscenes
     * - **Spectator**: Camera with collision but no target constraints
     */
    class Camera {
    public:
        /**
         * @brief Constructor
         * @param config Camera configuration
         */
        explicit Camera(const CameraConfig& config);

        /**
         * @brief Destructor
         */
        ~Camera();

        /**
         * @brief Deleted copy constructor
         */
        Camera(const Camera&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        Camera& operator=(const Camera&) = delete;

        // Camera lifecycle

        /**
         * @brief Initialize camera
         * @param world Parent world
         * @return true if successful, false otherwise
         */
        bool Initialize(World* world);

        /**
         * @brief Shutdown camera
         */
        void Shutdown();

        /**
         * @brief Update camera
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Render camera debug information
         */
        void RenderDebug();

        // Camera configuration and state

        /**
         * @brief Get camera configuration
         * @return Current configuration
         */
        const CameraConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set camera configuration
         * @param config New configuration
         */
        void SetConfig(const CameraConfig& config);

        /**
         * @brief Get camera state
         * @return Current state
         */
        const CameraState& GetState() const { return m_state; }

        /**
         * @brief Get camera mode
         * @return Current camera mode
         */
        CameraMode GetMode() const { return m_config.mode; }

        /**
         * @brief Set camera mode
         * @param mode New camera mode
         */
        void SetMode(CameraMode mode);

        // Target and following

        /**
         * @brief Get camera target
         * @return Target entity or nullptr
         */
        Entity* GetTarget() const { return m_target; }

        /**
         * @brief Set camera target
         * @param target New target entity
         */
        void SetTarget(Entity* target);

        /**
         * @brief Clear camera target
         */
        void ClearTarget();

        /**
         * @brief Check if camera has target
         * @return true if has target, false otherwise
         */
        bool HasTarget() const { return m_target != nullptr; }

        /**
         * @brief Get target position
         * @return Target world position
         */
        glm::vec3 GetTargetPosition() const;

        /**
         * @brief Get target forward direction
         * @return Target forward vector
         */
        glm::vec3 GetTargetForward() const;

        /**
         * @brief Get target up direction
         * @return Target up vector
         */
        glm::vec3 GetTargetUp() const;

        // Position and orientation

        /**
         * @brief Get camera position
         * @return World position
         */
        const glm::vec3& GetPosition() const { return m_state.position; }

        /**
         * @brief Set camera position
         * @param position New world position
         */
        void SetPosition(const glm::vec3& position);

        /**
         * @brief Get camera orientation
         * @return World orientation quaternion
         */
        const glm::quat& GetOrientation() const { return m_state.orientation; }

        /**
         * @brief Set camera orientation
         * @param orientation New world orientation quaternion
         */
        void SetOrientation(const glm::quat& orientation);

        /**
         * @brief Get forward direction
         * @return Camera forward vector
         */
        glm::vec3 GetForward() const;

        /**
         * @brief Get up direction
         * @return Camera up vector
         */
        glm::vec3 GetUp() const;

        /**
         * @brief Get right direction
         * @return Camera right vector
         */
        glm::vec3 GetRight() const;

        // Distance and zoom

        /**
         * @brief Get current distance from target
         * @return Current distance
         */
        float GetDistance() const { return m_state.currentDistance; }

        /**
         * @brief Set target distance from target
         * @param distance New target distance
         */
        void SetTargetDistance(float distance);

        /**
         * @brief Get target distance
         * @return Target distance
         */
        float GetTargetDistance() const { return m_state.targetDistance; }

        /**
         * @brief Zoom camera in
         * @param amount Zoom amount
         */
        void ZoomIn(float amount = 1.0f);

        /**
         * @brief Zoom camera out
         * @param amount Zoom amount
         */
        void ZoomOut(float amount = 1.0f);

        /**
         * @brief Reset zoom to default distance
         */
        void ResetZoom();

        // Movement and rotation

        /**
         * @brief Move camera
         * @param direction Movement direction
         * @param speed Movement speed multiplier
         */
        void Move(const glm::vec3& direction, float speed = 1.0f);

        /**
         * @brief Rotate camera
         * @param yaw Yaw rotation in degrees
         * @param pitch Pitch rotation in degrees
         */
        void Rotate(float yaw, float pitch);

        /**
         * @brief Add yaw rotation
         * @param yaw Yaw rotation in degrees
         */
        void AddYaw(float yaw);

        /**
         * @brief Add pitch rotation
         * @param pitch Pitch rotation in degrees
         */
        void AddPitch(float pitch);

        /**
         * @brief Look at position
         * @param position World position to look at
         */
        void LookAt(const glm::vec3& position);

        /**
         * @brief Look at entity
         * @param entity Entity to look at
         */
        void LookAt(Entity* entity);

        // View matrices and projection

        /**
         * @brief Get view matrix
         * @return Current view matrix
         */
        const glm::mat4& GetViewMatrix() const { return m_state.viewMatrix; }

        /**
         * @brief Get projection matrix
         * @return Current projection matrix
         */
        const glm::mat4& GetProjectionMatrix() const { return m_state.projectionMatrix; }

        /**
         * @brief Get view-projection matrix
         * @return Combined view-projection matrix
         */
        const glm::mat4& GetViewProjectionMatrix() const { return m_state.viewProjectionMatrix; }

        /**
         * @brief Get field of view
         * @return Current FOV in degrees
         */
        float GetFieldOfView() const { return m_state.currentFOV; }

        /**
         * @brief Set field of view
         * @param fov New FOV in degrees
         */
        void SetFieldOfView(float fov);

        /**
         * @brief Get aspect ratio
         * @return Current aspect ratio
         */
        float GetAspectRatio() const { return m_config.aspectRatio; }

        /**
         * @brief Set aspect ratio
         * @param ratio New aspect ratio
         */
        void SetAspectRatio(float ratio);

        // Collision and visibility

        /**
         * @brief Check if camera is colliding
         * @return true if colliding, false otherwise
         */
        bool IsColliding() const { return m_state.isColliding; }

        /**
         * @brief Get collision distance
         * @return Distance to collision point
         */
        float GetCollisionDistance() const { return m_state.collisionDistance; }

        /**
         * @brief Check if target is visible
         * @return true if target is visible, false otherwise
         */
        bool IsTargetVisible() const { return m_state.targetVisible; }

        /**
         * @brief Perform visibility test to target
         * @return true if target is visible, false otherwise
         */
        bool TestTargetVisibility() const;

        /**
         * @brief Perform collision detection
         * @return true if collision detected, false otherwise
         */
        bool DetectCollisions();

        /**
         * @brief Resolve camera collision
         * @return true if collision resolved, false otherwise
         */
        bool ResolveCollision();

        // Effects and shake

        /**
         * @brief Apply camera shake
         * @param intensity Shake intensity
         * @param frequency Shake frequency
         * @param duration Shake duration in seconds
         */
        void ApplyShake(float intensity, float frequency, float duration);

        /**
         * @brief Stop camera shake
         */
        void StopShake();

        /**
         * @brief Check if camera is shaking
         * @return true if shaking, false otherwise
         */
        bool IsShaking() const { return m_state.shakeTimeRemaining > 0.0f; }

        /**
         * @brief Apply motion blur effect
         * @param amount Motion blur amount (0.0 - 1.0)
         */
        void SetMotionBlur(float amount);

        /**
         * @brief Apply vignette effect
         * @param amount Vignette amount (0.0 - 1.0)
         */
        void SetVignette(float amount);

        // Frustum operations

        /**
         * @brief Get camera frustum planes
         * @return Array of 6 frustum planes
         */
        const std::array<glm::vec4, 6>& GetFrustumPlanes() const { return m_frustumPlanes; }

        /**
         * @brief Update frustum planes
         */
        void UpdateFrustumPlanes();

        /**
         * @brief Check if point is in frustum
         * @param point World space point
         * @return true if in frustum, false otherwise
         */
        bool IsPointInFrustum(const glm::vec3& point) const;

        /**
         * @brief Check if sphere is in frustum
         * @param center Sphere center
         * @param radius Sphere radius
         * @return true if in frustum, false otherwise
         */
        bool IsSphereInFrustum(const glm::vec3& center, float radius) const;

        /**
         * @brief Check if AABB is in frustum
         * @param aabb Axis-aligned bounding box
         * @return true if in frustum, false otherwise
         */
        bool IsAABBInFrustum(const AABB& aabb) const;

        // Behavior flags

        /**
         * @brief Check if behavior flag is set
         * @param flag Behavior flag to check
         * @return true if set, false otherwise
         */
        bool HasBehaviorFlag(CameraBehavior flag) const {
            return (m_config.behaviorFlags & static_cast<uint32_t>(flag)) != 0;
        }

        /**
         * @brief Set behavior flag
         * @param flag Behavior flag to set
         */
        void SetBehaviorFlag(CameraBehavior flag);

        /**
         * @brief Clear behavior flag
         * @param flag Behavior flag to clear
         */
        void ClearBehaviorFlag(CameraBehavior flag);

        /**
         * @brief Toggle behavior flag
         * @param flag Behavior flag to toggle
         */
        void ToggleBehaviorFlag(CameraBehavior flag);

        /**
         * @brief Get behavior flags
         * @return Current behavior flags
         */
        uint32_t GetBehaviorFlags() const { return m_config.behaviorFlags; }

        /**
         * @brief Set behavior flags
         * @param flags New behavior flags
         */
        void SetBehaviorFlags(uint32_t flags);

        // World integration

        /**
         * @brief Get camera world
         * @return Camera world
         */
        World* GetWorld() const { return m_world; }

        /**
         * @brief Set camera world
         * @param world New world
         */
        void SetWorld(World* world);

        // Ray casting from camera

        /**
         * @brief Cast ray from camera
         * @param screenX Screen X coordinate (0.0 - 1.0)
         * @param screenY Screen Y coordinate (0.0 - 1.0)
         * @param maxDistance Maximum ray distance
         * @param hitInfo Output hit information
         * @return true if hit something, false otherwise
         */
        bool CastRay(float screenX, float screenY, float maxDistance, RaycastHit& hitInfo) const;

        /**
         * @brief Cast ray from camera center
         * @param maxDistance Maximum ray distance
         * @param hitInfo Output hit information
         * @return true if hit something, false otherwise
         */
        bool CastCenterRay(float maxDistance, RaycastHit& hitInfo) const;

        /**
         * @brief Get world position from screen coordinates
         * @param screenX Screen X coordinate (0.0 - 1.0)
         * @param screenY Screen Y coordinate (0.0 - 1.0)
         * @param distance Distance from camera
         * @return World position
         */
        glm::vec3 ScreenToWorld(float screenX, float screenY, float distance) const;

        /**
         * @brief Get screen coordinates from world position
         * @param worldPos World position
         * @param screenX Output screen X coordinate
         * @param screenY Output screen Y coordinate
         * @return true if position is on screen, false otherwise
         */
        bool WorldToScreen(const glm::vec3& worldPos, float& screenX, float& screenY) const;

        // Metrics and debugging

        /**
         * @brief Get camera metrics
         * @return Performance metrics
         */
        const CameraMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

        /**
         * @brief Get debug information
         * @return Debug information string
         */
        std::string GetDebugInfo() const;

        /**
         * @brief Validate camera state
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Get debug render data
         * @return Debug render data
         */
        const std::vector<DebugRenderData>& GetDebugRenderData() const;

        /**
         * @brief Clear debug render data
         */
        void ClearDebugRenderData();

    private:
        /**
         * @brief Initialize camera subsystems
         * @return true if successful, false otherwise
         */
        bool InitializeSubsystems();

        /**
         * @brief Update camera position and orientation
         * @param deltaTime Time elapsed
         */
        void UpdateTransform(double deltaTime);

        /**
         * @brief Update camera matrices
         */
        void UpdateMatrices();

        /**
         * @brief Update target following
         * @param deltaTime Time elapsed
         */
        void UpdateTargetFollowing(double deltaTime);

        /**
         * @brief Update collision detection and resolution
         * @param deltaTime Time elapsed
         */
        void UpdateCollision(double deltaTime);

        /**
         * @brief Update camera effects (shake, motion blur, etc.)
         * @param deltaTime Time elapsed
         */
        void UpdateEffects(double deltaTime);

        /**
         * @brief Update frustum culling
         * @param deltaTime Time elapsed
         */
        void UpdateFrustumCulling(double deltaTime);

        /**
         * @brief Update orbital camera movement
         * @param deltaTime Time elapsed
         */
        void UpdateOrbitalMovement(double deltaTime);

        /**
         * @brief Update cinematic camera movement
         * @param deltaTime Time elapsed
         */
        void UpdateCinematicMovement(double deltaTime);

        /**
         * @brief Apply smoothing to camera movement
         * @param deltaTime Time elapsed
         */
        void ApplySmoothing(double deltaTime);

        /**
         * @brief Clamp camera distance to limits
         */
        void ClampDistance();

        /**
         * @brief Clamp camera rotation angles
         */
        void ClampRotation();

        /**
         * @brief Update camera metrics
         * @param deltaTime Time elapsed
         */
        void UpdateMetrics(double deltaTime);

        /**
         * @brief Collect debug render data
         */
        void CollectDebugRenderData();

        /**
         * @brief Handle camera errors
         * @param error Error message
         */
        void HandleCameraError(const std::string& error);

        // Camera data
        CameraConfig m_config;                          ///< Camera configuration
        CameraState m_state;                            ///< Current camera state
        CameraMetrics m_metrics;                        ///< Performance metrics

        // World integration
        World* m_world;                                 ///< Parent world
        Entity* m_target;                               ///< Camera target entity

        // Frustum data
        std::array<glm::vec4, 6> m_frustumPlanes;       ///< Frustum planes
        bool m_frustumValid;                            ///< Frustum is valid

        // Effect state
        double m_shakeStartTime;                        ///< Shake start time
        glm::vec3 m_shakeBaseOffset;                    ///< Base shake offset

        // Debug data
        std::vector<DebugRenderData> m_debugRenderData;
        mutable std::shared_mutex m_debugMutex;         ///< Debug synchronization

        // State flags
        bool m_isInitialized;                           ///< Camera is initialized
        bool m_needsMatrixUpdate;                       ///< Matrices need update
        bool m_needsFrustumUpdate;                      ///< Frustum needs update

        // Timing
        double m_lastUpdateTime;                        ///< Last update timestamp
        double m_creationTime;                          ///< Camera creation timestamp
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_PLAYER_CAMERA_HPP
