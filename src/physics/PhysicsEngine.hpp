#pragma once
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <atomic>
#include <mutex>
#include <cstdint>
#include "core/Logger.hpp"

namespace VoxelCraft {

	class Entity;
	class Chunk;
	class World;
	class Collider;
	class RigidBody;
	class PhysicsWorld;
	class CollisionManager;
	class ForceManager;
	class ConstraintManager;

	/**
	 * @brief Physics engine configuration
	 */
	struct PhysicsConfig
	{
		// Simulation settings
		float fixedTimeStep = 1.0f / 60.0f;    // 60Hz physics update
		float maxTimeStep = 1.0f / 30.0f;      // Minimum 30Hz
		int maxSubSteps = 10;                   // Maximum substeps
		float gravity = -9.81f;                // Standard gravity
		float terminalVelocity = -53.0f;       // Minecraft terminal velocity

		// Collision settings
		float collisionMargin = 0.001f;        // Collision detection margin
		int maxContacts = 1000;                // Maximum contacts per frame
		float contactThreshold = 0.001f;       // Contact resolution threshold

		// Performance settings
		int maxRigidBodies = 10000;            // Maximum rigid bodies
		int maxColliders = 50000;              // Maximum colliders
		int solverIterations = 10;             // Constraint solver iterations
		bool enableMultithreading = true;      // Enable parallel physics
		int numPhysicsThreads = 4;             // Number of physics threads

		// Features
		bool enableGravity = true;             // Enable gravity
		bool enableCollisions = true;          // Enable collision detection
		bool enableSleeping = true;            // Enable body sleeping
		bool enableContinuousCollision = false; // Enable CCD
		bool enableDebugDrawing = false;       // Enable debug visualization

		// Advanced settings
		float linearDamping = 0.01f;           // Linear velocity damping
		float angularDamping = 0.01f;          // Angular velocity damping
		float friction = 0.5f;                 // Default friction
		float restitution = 0.1f;              // Default restitution
		float sleepThreshold = 0.1f;           // Sleep threshold
	};

	/**
	 * @brief Physics simulation statistics
	 */
	struct PhysicsStats
	{
		// Performance metrics
		float simulationTime;
		float collisionTime;
		float solverTime;
		float broadPhaseTime;
		float narrowPhaseTime;
		int rigidBodyCount;
		int activeBodyCount;
		int sleepingBodyCount;
		int collisionCount;
		int contactCount;
		int constraintCount;

		// Memory usage
		size_t memoryUsed;
		size_t peakMemoryUsed;

		// Simulation quality
		float averageFPS;
		float minFPS;
		float maxFPS;
		int frameCount;
	};

	/**
	 * @brief Physics material properties
	 */
	struct PhysicsMaterial
	{
		std::string name;
		float friction;
		float restitution;
		float density;
		float linearDamping;
		float angularDamping;

		PhysicsMaterial() : friction(0.5f), restitution(0.1f), density(1.0f),
			linearDamping(0.01f), angularDamping(0.01f) {}

		PhysicsMaterial(float f, float r, float d = 1.0f)
			: friction(f), restitution(r), density(d),
			linearDamping(0.01f), angularDamping(0.01f) {}
	};

	/**
	 * @brief Force types
	 */
	enum class ForceType
	{
		GRAVITY,
		BUOYANCY,
		DRAG,
		THRUST,
		EXPLOSION,
		WIND,
		CUSTOM
	};

	/**
	 * @brief Collision shapes
	 */
	enum class ShapeType
	{
		BOX,
		SPHERE,
		CAPSULE,
		CYLINDER,
		CONVEX_HULL,
		MESH,
		TERRAIN,
		COMPOUND
	};

	/**
	 * @brief Collision detection modes
	 */
	enum class CollisionMode
	{
		DISCRETE,
		CONTINUOUS,
		BOTH
	};

	/**
	 * @brief Main Physics Engine class
	 *
	 * The PhysicsEngine provides:
	 * - Rigid body dynamics
	 * - Collision detection and resolution
	 * - Constraint solving
	 * - Force management
	 * - Ray casting and shape queries
	 * - Character controller
	 * - Vehicle physics
	 * - Soft body simulation
	 * - Fluid dynamics
	 * - Ragdoll physics
	 * - Destruction system
	 * - Physics debugging
	 */
	class PhysicsEngine
	{
	public:
		/**
		 * @brief Constructor
		 */
		PhysicsEngine();

		/**
		 * @brief Destructor
		 */
		~PhysicsEngine();

		/**
		 * @brief Initialize physics engine
		 */
		bool Initialize(const PhysicsConfig& config = PhysicsConfig());

		/**
		 * @brief Shutdown physics engine
		 */
		void Shutdown();

		/**
		 * @brief Update physics simulation
		 */
		void Update(float deltaTime);

		/**
		 * @brief Fixed update for physics simulation
		 */
		void FixedUpdate(float fixedDeltaTime);

		/**
		 * @brief Render debug information
		 */
		void DebugRender();

		/**
		 * @brief Create rigid body
		 */
		std::shared_ptr<RigidBody> CreateRigidBody(const std::string& name = "");

		/**
		 * @brief Destroy rigid body
		 */
		void DestroyRigidBody(std::shared_ptr<RigidBody> body);

		/**
		 * @brief Create collider
		 */
		std::shared_ptr<Collider> CreateCollider(ShapeType type);

		/**
		 * @brief Add force to rigid body
		 */
		void AddForce(std::shared_ptr<RigidBody> body, const glm::vec3& force, ForceType type = ForceType::CUSTOM);

		/**
		 * @brief Add torque to rigid body
		 */
		void AddTorque(std::shared_ptr<RigidBody> body, const glm::vec3& torque);

		/**
		 * @brief Apply impulse to rigid body
		 */
		void ApplyImpulse(std::shared_ptr<RigidBody> body, const glm::vec3& impulse, const glm::vec3& point = glm::vec3(0.0f));

		/**
		 * @brief Apply torque impulse to rigid body
		 */
		void ApplyTorqueImpulse(std::shared_ptr<RigidBody> body, const glm::vec3& torqueImpulse);

		/**
		 * @brief Ray cast
		 */
		bool RayCast(const glm::vec3& from, const glm::vec3& to, RayCastHit& hit, uint32_t layerMask = 0xFFFFFFFF);

		/**
		 * @brief Sphere cast
		 */
		bool SphereCast(const glm::vec3& from, const glm::vec3& to, float radius, RayCastHit& hit, uint32_t layerMask = 0xFFFFFFFF);

		/**
		 * @brief Box cast
		 */
		bool BoxCast(const glm::vec3& from, const glm::vec3& to, const glm::vec3& halfExtents, RayCastHit& hit, uint32_t layerMask = 0xFFFFFFFF);

		/**
		 * @brief Overlap sphere
		 */
		bool OverlapSphere(const glm::vec3& center, float radius, std::vector<OverlapResult>& results, uint32_t layerMask = 0xFFFFFFFF);

		/**
		 * @brief Overlap box
		 */
		bool OverlapBox(const glm::vec3& center, const glm::vec3& halfExtents, std::vector<OverlapResult>& results, uint32_t layerMask = 0xFFFFFFFF);

		/**
		 * @brief Sweep test
		 */
		bool SweepTest(std::shared_ptr<Collider> collider, const glm::vec3& direction, float distance, SweepResult& result);

		/**
		 * @brief Get physics configuration
		 */
		const PhysicsConfig& GetConfig() const { return m_config; }

		/**
		 * @brief Set physics configuration
		 */
		void SetConfig(const PhysicsConfig& config);

		/**
		 * @brief Get physics statistics
		 */
		const PhysicsStats& GetStats() const { return m_stats; }

		/**
		 * @brief Check if physics engine is initialized
		 */
		bool IsInitialized() const { return m_initialized; }

		/**
		 * @brief Set gravity
		 */
		void SetGravity(const glm::vec3& gravity);

		/**
		 * @brief Get gravity
		 */
		const glm::vec3& GetGravity() const { return m_gravity; }

		/**
		 * @brief Enable/disable physics simulation
		 */
		void SetEnabled(bool enabled) { m_enabled = enabled; }

		/**
		 * @brief Check if physics simulation is enabled
		 */
		bool IsEnabled() const { return m_enabled; }

		/**
		 * @brief Pause/unpause physics simulation
		 */
		void SetPaused(bool paused) { m_paused = paused; }

		/**
		 * @brief Check if physics simulation is paused
		 */
		bool IsPaused() const { return m_paused; }

		/**
		 * @brief Set simulation speed
		 */
		void SetTimeScale(float timeScale) { m_timeScale = timeScale; }

		/**
		 * @brief Get simulation speed
		 */
		float GetTimeScale() const { return m_timeScale; }

		/**
		 * @brief Get all rigid bodies
		 */
		std::vector<std::shared_ptr<RigidBody>> GetRigidBodies() const;

		/**
		 * @brief Get active rigid bodies
		 */
		std::vector<std::shared_ptr<RigidBody>> GetActiveRigidBodies() const;

		/**
		 * @brief Get sleeping rigid bodies
		 */
		std::vector<std::shared_ptr<RigidBody>> GetSleepingRigidBodies() const;

		/**
		 * @brief Create character controller
		 */
		std::shared_ptr<CharacterController> CreateCharacterController();

		/**
		 * @brief Create vehicle
		 */
		std::shared_ptr<Vehicle> CreateVehicle();

		/**
		 * @brief Create soft body
		 */
		std::shared_ptr<SoftBody> CreateSoftBody();

		/**
		 * @brief Create constraint
		 */
		std::shared_ptr<Constraint> CreateConstraint(ConstraintType type, std::shared_ptr<RigidBody> bodyA, std::shared_ptr<RigidBody> bodyB = nullptr);

		/**
		 * @brief Destroy constraint
		 */
		void DestroyConstraint(std::shared_ptr<Constraint> constraint);

		/**
		 * @brief Add physics material
		 */
		void AddMaterial(const std::string& name, const PhysicsMaterial& material);

		/**
		 * @brief Get physics material
		 */
		const PhysicsMaterial& GetMaterial(const std::string& name) const;

		/**
		 * @brief Set collision layer mask
		 */
		void SetLayerCollision(uint32_t layer1, uint32_t layer2, bool collide);

		/**
		 * @brief Get collision layer mask
		 */
		bool GetLayerCollision(uint32_t layer1, uint32_t layer2) const;

	private:
		PhysicsConfig m_config;
		PhysicsStats m_stats;
		bool m_initialized;
		bool m_enabled;
		bool m_paused;
		float m_timeScale;
		float m_accumulatedTime;
		glm::vec3 m_gravity;

		// Core systems
		std::unique_ptr<PhysicsWorld> m_physicsWorld;
		std::unique_ptr<CollisionManager> m_collisionManager;
		std::unique_ptr<ForceManager> m_forceManager;
		std::unique_ptr<ConstraintManager> m_constraintManager;

		// Physics objects
		std::vector<std::shared_ptr<RigidBody>> m_rigidBodies;
		std::vector<std::shared_ptr<Collider>> m_colliders;
		std::vector<std::shared_ptr<Constraint>> m_constraints;
		std::vector<std::shared_ptr<CharacterController>> m_characterControllers;
		std::vector<std::shared_ptr<Vehicle>> m_vehicles;
		std::vector<std::shared_ptr<SoftBody>> m_softBodies;

		// Materials and layers
		std::unordered_map<std::string, PhysicsMaterial> m_materials;
		std::vector<std::vector<bool>> m_layerMatrix;

		// Threading
		std::mutex m_physicsMutex;
		std::vector<std::thread> m_physicsThreads;
		std::atomic<bool> m_simulating;

		// Performance tracking
		std::chrono::steady_clock::time_point m_lastStatsUpdate;
		std::chrono::steady_clock::time_point m_frameStartTime;

		/**
		 * @brief Initialize physics systems
		 */
		void InitializeSystems();

		/**
		 * @brief Initialize default materials
		 */
		void InitializeMaterials();

		/**
		 * @brief Initialize collision layers
		 */
		void InitializeLayers();

		/**
		 * @brief Start physics threads
		 */
		void StartPhysicsThreads();

		/**
		 * @brief Stop physics threads
		 */
		void StopPhysicsThreads();

		/**
		 * @brief Update physics simulation
		 */
		void SimulatePhysics(float deltaTime);

		/**
		 * @brief Detect collisions
		 */
		void DetectCollisions();

		/**
		 * @brief Resolve collisions
		 */
		void ResolveCollisions();

		/**
		 * @brief Solve constraints
		 */
		void SolveConstraints();

		/**
		 * @brief Integrate rigid bodies
		 */
		void IntegrateRigidBodies(float deltaTime);

		/**
		 * @brief Update sleeping bodies
		 */
		void UpdateSleepingBodies();

		/**
		 * @brief Update statistics
		 */
		void UpdateStats();

		/**
		 * @brief Physics thread function
		 */
		void PhysicsThreadFunction(int threadIndex);

		/**
		 * @brief Check if body should sleep
		 */
		bool ShouldSleep(std::shared_ptr<RigidBody> body) const;

		/**
		 * @brief Wake up nearby bodies
		 */
		void WakeUpNearbyBodies(std::shared_ptr<RigidBody> body);

		/**
		 * @brief Validate physics state
		 */
		bool ValidatePhysicsState() const;

		/**
		 * @brief Handle physics errors
		 */
		void HandlePhysicsError(const std::string& error);
	};

} // namespace VoxelCraft
