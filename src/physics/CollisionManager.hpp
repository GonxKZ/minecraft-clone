#pragma once
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <deque>
#include <mutex>
#include <atomic>
#include <functional>
#include <cstdint>
#include "core/Logger.hpp"

namespace VoxelCraft {

	class Collider;
	class RigidBody;
	class Contact;
	class ContactManifold;
	class BroadPhase;
	class NarrowPhase;
	class CollisionShape;

	/**
	 * @brief Ray cast hit result
	 */
	struct RayCastHit
	{
		std::shared_ptr<Collider> collider;
		std::shared_ptr<RigidBody> rigidBody;
		glm::vec3 point;
		glm::vec3 normal;
		float distance;
		uint32_t layerMask;

		RayCastHit()
			: distance(std::numeric_limits<float>::max())
			, layerMask(0xFFFFFFFF)
		{}
	};

	/**
	 * @brief Overlap result
	 */
	struct OverlapResult
	{
		std::shared_ptr<Collider> collider;
		std::shared_ptr<RigidBody> rigidBody;
		uint32_t layerMask;
	};

	/**
	 * @brief Sweep test result
	 */
	struct SweepResult
	{
		std::shared_ptr<Collider> collider;
		std::shared_ptr<RigidBody> rigidBody;
		glm::vec3 point;
		glm::vec3 normal;
		float distance;
		bool hit;

		SweepResult() : distance(std::numeric_limits<float>::max()), hit(false) {}
	};

	/**
	 * @brief Contact information
	 */
	struct Contact
	{
		std::shared_ptr<Collider> colliderA;
		std::shared_ptr<Collider> colliderB;
		glm::vec3 point;
		glm::vec3 normal;
		glm::vec3 tangent1;
		glm::vec3 tangent2;
		float penetration;
		float friction;
		float restitution;
		bool persistent;

		Contact()
			: penetration(0.0f)
			, friction(0.5f)
			, restitution(0.1f)
			, persistent(false)
		{}
	};

	/**
	 * @brief Contact manifold for persistent contacts
	 */
	struct ContactManifold
	{
		std::shared_ptr<Collider> colliderA;
		std::shared_ptr<Collider> colliderB;
		std::vector<Contact> contacts;
		glm::vec3 normal;
		float friction;
		float restitution;
		bool persistent;

		ContactManifold()
			: friction(0.5f)
			, restitution(0.1f)
			, persistent(false)
		{}
	};

	/**
	 * @brief Collision manager configuration
	 */
	struct CollisionConfig
	{
		// Broad phase settings
		bool enableBroadPhase = true;
		bool enableNarrowPhase = true;
		bool enablePersistentContacts = true;

		// Performance settings
		int maxContacts = 1000;
		int maxManifolds = 100;
		float contactThreshold = 0.001f;
		float penetrationThreshold = 0.01f;
		float contactPersistenceThreshold = 0.1f;

		// Spatial partitioning
		bool enableSpatialPartitioning = true;
		int spatialPartitionSize = 1000; // World units per partition
		int maxObjectsPerPartition = 100;

		// Collision detection
		int collisionIterations = 10;
		float collisionMargin = 0.001f;
		float timeOfImpactThreshold = 0.001f;

		// Debug settings
		bool enableDebugDrawing = false;
		bool enableContactDrawing = false;
		bool enableBroadPhaseDrawing = false;
	};

	/**
	 * @brief Collision manager statistics
	 */
	struct CollisionStats
	{
		int broadPhaseTests;
		int narrowPhaseTests;
		int collisionCount;
		int contactCount;
		int manifoldCount;
		float broadPhaseTime;
		float narrowPhaseTime;
		float collisionResolutionTime;
		int rayCastCount;
		int sphereCastCount;
		int boxCastCount;
		int overlapSphereCount;
		int overlapBoxCount;
		int sweepTestCount;
	};

	/**
	 * @brief Collision manager for physics engine
	 *
	 * The CollisionManager handles:
	 * - Broad phase collision detection
	 * - Narrow phase collision detection
	 * - Contact generation and management
	 * - Ray casting and shape queries
	 * - Collision resolution
	 * - Spatial partitioning
	 * - Collision layer filtering
	 * - Persistent contact manifolds
	 */
	class CollisionManager
	{
	public:
		/**
		 * @brief Constructor
		 */
		CollisionManager();

		/**
		 * @brief Destructor
		 */
		~CollisionManager();

		/**
		 * @brief Initialize collision manager
		 */
		bool Initialize(const CollisionConfig& config = CollisionConfig());

		/**
		 * @brief Shutdown collision manager
		 */
		void Shutdown();

		/**
		 * @brief Update collision manager
		 */
		void Update(float deltaTime);

		/**
		 * @brief Add collider to collision detection
		 */
		void AddCollider(std::shared_ptr<Collider> collider);

		/**
		 * @brief Remove collider from collision detection
		 */
		void RemoveCollider(std::shared_ptr<Collider> collider);

		/**
		 * @brief Update collider position/orientation
		 */
		void UpdateCollider(std::shared_ptr<Collider> collider);

		/**
		 * @brief Perform broad phase collision detection
		 */
		void BroadPhase();

		/**
		 * @brief Perform narrow phase collision detection
		 */
		void NarrowPhase();

		/**
		 * @brief Detect all collisions
		 */
		void DetectCollisions();

		/**
		 * @brief Resolve all collisions
		 */
		void ResolveCollisions();

		/**
		 * @brief Generate contacts from collision pairs
		 */
		void GenerateContacts(const std::vector<std::pair<std::shared_ptr<Collider>, std::shared_ptr<Collider>>>& pairs);

		/**
		 * @brief Solve contact constraints
		 */
		void SolveContacts(float deltaTime);

		/**
		 * @brief Clean up old contacts
		 */
		void CleanUpContacts();

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
		 * @brief Check if two colliders can collide
		 */
		bool CanCollide(std::shared_ptr<Collider> colliderA, std::shared_ptr<Collider> colliderB) const;

		/**
		 * @brief Debug render collision information
		 */
		void DebugRender();

		/**
		 * @brief Get collision configuration
		 */
		const CollisionConfig& GetConfig() const { return m_config; }

		/**
		 * @brief Set collision configuration
		 */
		void SetConfig(const CollisionConfig& config);

		/**
		 * @brief Get collision statistics
		 */
		const CollisionStats& GetStats() const { return m_stats; }

		/**
		 * @brief Get collision count
		 */
		int GetCollisionCount() const { return m_stats.collisionCount; }

		/**
		 * @brief Get contact count
		 */
		int GetContactCount() const { return m_stats.contactCount; }

		/**
		 * @brief Get manifold count
		 */
		int GetManifoldCount() const { return m_stats.manifoldCount; }

		/**
		 * @brief Check if initialized
		 */
		bool IsInitialized() const { return m_initialized; }

		/**
		 * @brief Get all colliders
		 */
		std::vector<std::shared_ptr<Collider>> GetColliders() const;

		/**
		 * @brief Get all contacts
		 */
		std::vector<Contact> GetContacts() const;

		/**
		 * @brief Get all manifolds
		 */
		std::vector<ContactManifold> GetManifolds() const;

		/**
		 * @brief Clear all collision data
		 */
		void Clear();

	private:
		CollisionConfig m_config;
		CollisionStats m_stats;
		bool m_initialized;

		// Collision objects
		std::vector<std::shared_ptr<Collider>> m_colliders;
		std::vector<Contact> m_contacts;
		std::vector<ContactManifold> m_manifolds;

		// Spatial partitioning
		std::unique_ptr<SpatialPartition> m_spatialPartition;

		// Collision detection phases
		std::unique_ptr<BroadPhase> m_broadPhase;
		std::unique_ptr<NarrowPhase> m_narrowPhase;

		// Collision pairs from broad phase
		std::vector<std::pair<std::shared_ptr<Collider>, std::shared_ptr<Collider>>> m_collisionPairs;

		// Threading
		std::mutex m_collisionMutex;
		std::vector<std::thread> m_collisionThreads;
		std::atomic<bool> m_detectingCollisions;

		// Performance tracking
		std::chrono::steady_clock::time_point m_lastStatsUpdate;

		/**
		 * @brief Initialize collision detection phases
		 */
		void InitializeCollisionPhases();

		/**
		 * @brief Initialize spatial partitioning
		 */
		void InitializeSpatialPartition();

		/**
		 * @brief Update spatial partitioning
		 */
		void UpdateSpatialPartition();

		/**
		 * @brief Collect collision pairs from broad phase
		 */
		void CollectCollisionPairs();

		/**
		 * @brief Process collision pairs in narrow phase
		 */
		void ProcessCollisionPairs();

		/**
		 * @brief Generate contact manifold from collision
		 */
		ContactManifold GenerateManifold(std::shared_ptr<Collider> colliderA, std::shared_ptr<Collider> colliderB);

		/**
		 * @brief Add contact to system
		 */
		void AddContact(const Contact& contact);

		/**
		 * @brief Remove contact from system
		 */
		void RemoveContact(size_t index);

		/**
		 * @brief Update contact manifold
		 */
		void UpdateManifold(ContactManifold& manifold);

		/**
		 * @brief Check if contact is persistent
		 */
		bool IsPersistentContact(const Contact& contact) const;

		/**
		 * @brief Merge contacts into manifold
		 */
		void MergeContacts(ContactManifold& manifold, const std::vector<Contact>& newContacts);

		/**
		 * @brief Solve contact constraint
		 */
		void SolveContactConstraint(const Contact& contact, float deltaTime);

		/**
		 * @brief Solve manifold constraints
		 */
		void SolveManifoldConstraints(const ContactManifold& manifold, float deltaTime);

		/**
		 * @brief Calculate contact friction
		 */
		float CalculateContactFriction(std::shared_ptr<Collider> colliderA, std::shared_ptr<Collider> colliderB) const;

		/**
		 * @brief Calculate contact restitution
		 */
		float CalculateContactRestitution(std::shared_ptr<Collider> colliderA, std::shared_ptr<Collider> colliderB) const;

		/**
		 * @brief Perform ray cast against single collider
		 */
		bool RayCastAgainstCollider(const glm::vec3& from, const glm::vec3& to, std::shared_ptr<Collider> collider, RayCastHit& hit);

		/**
		 * @brief Perform sphere cast against single collider
		 */
		bool SphereCastAgainstCollider(const glm::vec3& from, const glm::vec3& to, float radius, std::shared_ptr<Collider> collider, RayCastHit& hit);

		/**
		 * @brief Perform box cast against single collider
		 */
		bool BoxCastAgainstCollider(const glm::vec3& from, const glm::vec3& to, const glm::vec3& halfExtents, std::shared_ptr<Collider> collider, RayCastHit& hit);

		/**
		 * @brief Check overlap between two colliders
		 */
		bool CheckOverlap(std::shared_ptr<Collider> colliderA, std::shared_ptr<Collider> colliderB, Contact& contact);

		/**
		 * @brief Check collision layer compatibility
		 */
		bool CheckLayerCompatibility(uint32_t layerA, uint32_t layerB) const;

		/**
		 * @brief Update collision statistics
		 */
		void UpdateStats();

		/**
		 * @brief Validate collision data
		 */
		bool ValidateCollisionData() const;

		/**
		 * @brief Handle collision error
		 */
		void HandleCollisionError(const std::string& error);

		/**
		 * @brief Debug render contacts
		 */
		void DebugRenderContacts();

		/**
		 * @brief Debug render manifolds
		 */
		void DebugRenderManifolds();

		/**
		 * @brief Debug render broad phase
		 */
		void DebugRenderBroadPhase();
	};

} // namespace VoxelCraft
