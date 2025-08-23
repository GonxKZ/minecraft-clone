#include "CollisionManager.hpp"
#include "Collider.hpp"
#include "RigidBody.hpp"
#include "BroadPhase.hpp"
#include "NarrowPhase.hpp"
#include "SpatialPartition.hpp"
#include <algorithm>
#include <cmath>
#include <queue>

namespace VoxelCraft {

	CollisionManager::CollisionManager()
		: m_initialized(false)
		, m_detectingCollisions(false)
	{
		// Initialize statistics
		std::memset(&m_stats, 0, sizeof(CollisionStats));

		VOXELCRAFT_LOG_INFO("CollisionManager initialized");
	}

	CollisionManager::~CollisionManager()
	{
		Shutdown();
	}

	bool CollisionManager::Initialize(const CollisionConfig& config)
	{
		if (m_initialized) {
			VOXELCRAFT_LOG_WARN("CollisionManager already initialized");
			return true;
		}

		m_config = config;

		try {
			// Initialize collision detection phases
			InitializeCollisionPhases();

			// Initialize spatial partitioning
			InitializeSpatialPartition();

			// Initialize timing
			m_lastStatsUpdate = std::chrono::steady_clock::now();

			m_initialized = true;
			VOXELCRAFT_LOG_INFO("CollisionManager initialized successfully");

			return true;
		}
		catch (const std::exception& e) {
			VOXELCRAFT_LOG_ERROR("Failed to initialize CollisionManager: {}", e.what());
			Shutdown();
			return false;
		}
	}

	void CollisionManager::Shutdown()
	{
		if (!m_initialized) {
			return;
		}

		VOXELCRAFT_LOG_INFO("Shutting down CollisionManager...");

		// Stop collision threads
		m_detectingCollisions = false;

		for (auto& thread : m_collisionThreads) {
			if (thread.joinable()) {
				thread.join();
			}
		}

		m_collisionThreads.clear();

		// Clear all collision data
		Clear();

		// Shutdown collision phases
		m_broadPhase.reset();
		m_narrowPhase.reset();
		m_spatialPartition.reset();

		m_initialized = false;

		VOXELCRAFT_LOG_INFO("CollisionManager shutdown complete");
	}

	void CollisionManager::Update(float deltaTime)
	{
		if (!m_initialized) {
			return;
		}

		// Update spatial partitioning
		UpdateSpatialPartition();

		// Clean up old contacts
		CleanUpContacts();

		// Update statistics periodically
		auto now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::seconds>(now - m_lastStatsUpdate).count() >= 1) {
			UpdateStats();
			m_lastStatsUpdate = now;
		}
	}

	void CollisionManager::AddCollider(std::shared_ptr<Collider> collider)
	{
		if (!collider || !m_initialized) {
			return;
		}

		std::unique_lock<std::mutex> lock(m_collisionMutex);

		// Check if collider is already added
		auto it = std::find(m_colliders.begin(), m_colliders.end(), collider);
		if (it == m_colliders.end()) {
			m_colliders.push_back(collider);

			// Add to spatial partition
			if (m_spatialPartition) {
				m_spatialPartition->AddCollider(collider);
			}

			VOXELCRAFT_LOG_DEBUG("Added collider to collision manager");
		}
	}

	void CollisionManager::RemoveCollider(std::shared_ptr<Collider> collider)
	{
		if (!collider || !m_initialized) {
			return;
		}

		std::unique_lock<std::mutex> lock(m_collisionMutex);

		// Remove from colliders list
		auto it = std::find(m_colliders.begin(), m_colliders.end(), collider);
		if (it != m_colliders.end()) {
			m_colliders.erase(it);

			// Remove from spatial partition
			if (m_spatialPartition) {
				m_spatialPartition->RemoveCollider(collider);
			}

			// Remove associated contacts
			std::vector<size_t> contactsToRemove;
			for (size_t i = 0; i < m_contacts.size(); ++i) {
				if (m_contacts[i].colliderA == collider || m_contacts[i].colliderB == collider) {
					contactsToRemove.push_back(i);
				}
			}

			// Remove contacts in reverse order to maintain indices
			std::sort(contactsToRemove.rbegin(), contactsToRemove.rend());
			for (size_t index : contactsToRemove) {
				RemoveContact(index);
			}

			VOXELCRAFT_LOG_DEBUG("Removed collider from collision manager");
		}
	}

	void CollisionManager::UpdateCollider(std::shared_ptr<Collider> collider)
	{
		if (!collider || !m_initialized || !m_spatialPartition) {
			return;
		}

		m_spatialPartition->UpdateCollider(collider);
	}

	void CollisionManager::BroadPhase()
	{
		if (!m_initialized || !m_broadPhase || !m_config.enableBroadPhase) {
			return;
		}

		auto startTime = std::chrono::steady_clock::now();

		// Perform broad phase collision detection
		m_collisionPairs.clear();
		m_broadPhase->Execute(m_colliders, m_collisionPairs);

		auto endTime = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

		m_stats.broadPhaseTime = duration.count() / 1000000.0f;
		m_stats.broadPhaseTests = m_collisionPairs.size();
	}

	void CollisionManager::NarrowPhase()
	{
		if (!m_initialized || !m_narrowPhase || !m_config.enableNarrowPhase) {
			return;
		}

		auto startTime = std::chrono::steady_clock::now();

		// Process collision pairs from broad phase
		ProcessCollisionPairs();

		auto endTime = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

		m_stats.narrowPhaseTime = duration.count() / 1000000.0f;
	}

	void CollisionManager::DetectCollisions()
	{
		if (!m_initialized) {
			return;
		}

		// Perform broad phase
		BroadPhase();

		// Perform narrow phase
		NarrowPhase();

		// Generate contacts
		GenerateContacts(m_collisionPairs);
	}

	void CollisionManager::ResolveCollisions()
	{
		if (!m_initialized) {
			return;
		}

		auto startTime = std::chrono::steady_clock::now();

		// Solve contact constraints
		for (auto& manifold : m_manifolds) {
			SolveManifoldConstraints(manifold, 1.0f / 60.0f); // Fixed timestep
		}

		// Solve individual contacts
		for (auto& contact : m_contacts) {
			SolveContactConstraint(contact, 1.0f / 60.0f);
		}

		auto endTime = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

		m_stats.collisionResolutionTime = duration.count() / 1000000.0f;
	}

	void CollisionManager::GenerateContacts(const std::vector<std::pair<std::shared_ptr<Collider>, std::shared_ptr<Collider>>>& pairs)
	{
		std::unique_lock<std::mutex> lock(m_collisionMutex);

		int newContacts = 0;

		for (const auto& pair : pairs) {
			auto colliderA = pair.first;
			auto colliderB = pair.second;

			if (!CanCollide(colliderA, colliderB)) {
				continue;
			}

			// Check for existing manifold
			bool foundManifold = false;
			for (auto& manifold : m_manifolds) {
				if ((manifold.colliderA == colliderA && manifold.colliderB == colliderB) ||
					(manifold.colliderA == colliderB && manifold.colliderB == colliderA)) {

					// Update existing manifold
					UpdateManifold(manifold);
					foundManifold = true;
					break;
				}
			}

			if (!foundManifold && m_config.enablePersistentContacts) {
				// Create new manifold
				ContactManifold manifold = GenerateManifold(colliderA, colliderB);
				if (!manifold.contacts.empty()) {
					m_manifolds.push_back(manifold);
					newContacts += manifold.contacts.size();
				}
			} else if (!m_config.enablePersistentContacts) {
				// Generate temporary contact
				Contact contact;
				if (CheckOverlap(colliderA, colliderB, contact)) {
					AddContact(contact);
					newContacts++;
				}
			}
		}

		m_stats.contactCount = m_contacts.size();
		m_stats.manifoldCount = m_manifolds.size();
		m_stats.collisionCount = newContacts;
	}

	void CollisionManager::SolveContacts(float deltaTime)
	{
		// This is called by ResolveCollisions
		// Individual contact solving is handled there
	}

	void CollisionManager::CleanUpContacts()
	{
		std::unique_lock<std::mutex> lock(m_collisionMutex);

		// Remove old contacts that are no longer valid
		std::vector<Contact> validContacts;
		for (const auto& contact : m_contacts) {
			if (contact.persistent && contact.penetration > m_config.contactThreshold) {
				validContacts.push_back(contact);
			}
		}
		m_contacts = validContacts;

		// Remove old manifolds
		std::vector<ContactManifold> validManifolds;
		for (const auto& manifold : m_manifolds) {
			if (!manifold.contacts.empty()) {
				validManifolds.push_back(manifold);
			}
		}
		m_manifolds = validManifolds;
	}

	bool CollisionManager::RayCast(const glm::vec3& from, const glm::vec3& to, RayCastHit& hit, uint32_t layerMask)
	{
		if (!m_initialized) {
			return false;
		}

		m_stats.rayCastCount++;

		bool hitFound = false;
		float closestDistance = std::numeric_limits<float>::max();

		std::unique_lock<std::mutex> lock(m_collisionMutex);

		for (auto& collider : m_colliders) {
			if (!collider || !collider->IsEnabled()) {
				continue;
			}

			// Check layer mask
			if ((collider->GetLayerMask() & layerMask) == 0) {
				continue;
			}

			RayCastHit currentHit;
			if (RayCastAgainstCollider(from, to, collider, currentHit)) {
				if (currentHit.distance < closestDistance) {
					hit = currentHit;
					closestDistance = currentHit.distance;
					hitFound = true;
				}
			}
		}

		return hitFound;
	}

	bool CollisionManager::SphereCast(const glm::vec3& from, const glm::vec3& to, float radius, RayCastHit& hit, uint32_t layerMask)
	{
		if (!m_initialized) {
			return false;
		}

		m_stats.sphereCastCount++;

		bool hitFound = false;
		float closestDistance = std::numeric_limits<float>::max();

		std::unique_lock<std::mutex> lock(m_collisionMutex);

		for (auto& collider : m_colliders) {
			if (!collider || !collider->IsEnabled()) {
				continue;
			}

			// Check layer mask
			if ((collider->GetLayerMask() & layerMask) == 0) {
				continue;
			}

			RayCastHit currentHit;
			if (SphereCastAgainstCollider(from, to, radius, collider, currentHit)) {
				if (currentHit.distance < closestDistance) {
					hit = currentHit;
					closestDistance = currentHit.distance;
					hitFound = true;
				}
			}
		}

		return hitFound;
	}

	bool CollisionManager::BoxCast(const glm::vec3& from, const glm::vec3& to, const glm::vec3& halfExtents, RayCastHit& hit, uint32_t layerMask)
	{
		if (!m_initialized) {
			return false;
		}

		m_stats.boxCastCount++;

		bool hitFound = false;
		float closestDistance = std::numeric_limits<float>::max();

		std::unique_lock<std::mutex> lock(m_collisionMutex);

		for (auto& collider : m_colliders) {
			if (!collider || !collider->IsEnabled()) {
				continue;
			}

			// Check layer mask
			if ((collider->GetLayerMask() & layerMask) == 0) {
				continue;
			}

			RayCastHit currentHit;
			if (BoxCastAgainstCollider(from, to, halfExtents, collider, currentHit)) {
				if (currentHit.distance < closestDistance) {
					hit = currentHit;
					closestDistance = currentHit.distance;
					hitFound = true;
				}
			}
		}

		return hitFound;
	}

	bool CollisionManager::OverlapSphere(const glm::vec3& center, float radius, std::vector<OverlapResult>& results, uint32_t layerMask)
	{
		if (!m_initialized) {
			return false;
		}

		m_stats.overlapSphereCount++;

		results.clear();

		std::unique_lock<std::mutex> lock(m_collisionMutex);

		for (auto& collider : m_colliders) {
			if (!collider || !collider->IsEnabled()) {
				continue;
			}

			// Check layer mask
			if ((collider->GetLayerMask() & layerMask) == 0) {
				continue;
			}

			// Simple sphere overlap check
			auto colliderPos = collider->GetWorldPosition();
			auto colliderBounds = collider->GetWorldBounds();

			// Check if sphere intersects with collider bounds
			glm::vec3 closestPoint = glm::clamp(center, colliderBounds.min, colliderBounds.max);
			float distanceSquared = glm::distance2(center, closestPoint);

			if (distanceSquared <= radius * radius) {
				OverlapResult result;
				result.collider = collider;
				result.rigidBody = collider->GetRigidBody();
				result.layerMask = collider->GetLayerMask();
				results.push_back(result);
			}
		}

		return !results.empty();
	}

	bool CollisionManager::OverlapBox(const glm::vec3& center, const glm::vec3& halfExtents, std::vector<OverlapResult>& results, uint32_t layerMask)
	{
		if (!m_initialized) {
			return false;
		}

		m_stats.overlapBoxCount++;

		results.clear();

		std::unique_lock<std::mutex> lock(m_collisionMutex);

		for (auto& collider : m_colliders) {
			if (!collider || !collider->IsEnabled()) {
				continue;
			}

			// Check layer mask
			if ((collider->GetLayerMask() & layerMask) == 0) {
				continue;
			}

			// Simple AABB overlap check
			auto colliderBounds = collider->GetWorldBounds();

			bool overlap = (center.x - halfExtents.x <= colliderBounds.max.x && center.x + halfExtents.x >= colliderBounds.min.x) &&
						  (center.y - halfExtents.y <= colliderBounds.max.y && center.y + halfExtents.y >= colliderBounds.min.y) &&
						  (center.z - halfExtents.z <= colliderBounds.max.z && center.z + halfExtents.z >= colliderBounds.min.z);

			if (overlap) {
				OverlapResult result;
				result.collider = collider;
				result.rigidBody = collider->GetRigidBody();
				result.layerMask = collider->GetLayerMask();
				results.push_back(result);
			}
		}

		return !results.empty();
	}

	bool CollisionManager::SweepTest(std::shared_ptr<Collider> collider, const glm::vec3& direction, float distance, SweepResult& result)
	{
		if (!m_initialized || !collider) {
			return false;
		}

		m_stats.sweepTestCount++;

		// Simple sweep test implementation
		auto startPos = collider->GetWorldPosition();
		auto endPos = startPos + direction * distance;

		RayCastHit hit;
		if (RayCast(startPos, endPos, hit)) {
			result.collider = hit.collider;
			result.rigidBody = hit.rigidBody;
			result.point = hit.point;
			result.normal = hit.normal;
			result.distance = hit.distance;
			result.hit = true;
			return true;
		}

		return false;
	}

	bool CollisionManager::CanCollide(std::shared_ptr<Collider> colliderA, std::shared_ptr<Collider> colliderB) const
	{
		if (!colliderA || !colliderB) {
			return false;
		}

		// Check if colliders are the same
		if (colliderA == colliderB) {
			return false;
		}

		// Check if both colliders are enabled
		if (!colliderA->IsEnabled() || !colliderB->IsEnabled()) {
			return false;
		}

		// Check if colliders are attached to the same rigid body
		if (colliderA->GetRigidBody() && colliderB->GetRigidBody() &&
			colliderA->GetRigidBody() == colliderB->GetRigidBody()) {
			return false;
		}

		// Check layer compatibility
		return CheckLayerCompatibility(colliderA->GetLayerMask(), colliderB->GetLayerMask());
	}

	void CollisionManager::DebugRender()
	{
		if (!m_config.enableDebugDrawing) {
			return;
		}

		if (m_config.enableContactDrawing) {
			DebugRenderContacts();
		}

		if (m_config.enableBroadPhaseDrawing && m_spatialPartition) {
			DebugRenderBroadPhase();
		}
	}

	void CollisionManager::SetConfig(const CollisionConfig& config)
	{
		m_config = config;

		// Reinitialize collision phases if needed
		if (m_config.enableBroadPhase != (m_broadPhase != nullptr)) {
			InitializeCollisionPhases();
		}

		VOXELCRAFT_LOG_INFO("CollisionManager configuration updated");
	}

	std::vector<std::shared_ptr<Collider>> CollisionManager::GetColliders() const
	{
		std::unique_lock<std::mutex> lock(m_collisionMutex);
		return m_colliders;
	}

	std::vector<Contact> CollisionManager::GetContacts() const
	{
		std::unique_lock<std::mutex> lock(m_collisionMutex);
		return m_contacts;
	}

	std::vector<ContactManifold> CollisionManager::GetManifolds() const
	{
		std::unique_lock<std::mutex> lock(m_collisionMutex);
		return m_manifolds;
	}

	void CollisionManager::Clear()
	{
		std::unique_lock<std::mutex> lock(m_collisionMutex);

		m_contacts.clear();
		m_manifolds.clear();
		m_collisionPairs.clear();

		std::memset(&m_stats, 0, sizeof(CollisionStats));
	}

	void CollisionManager::InitializeCollisionPhases()
	{
		// Initialize broad phase
		if (m_config.enableBroadPhase) {
			m_broadPhase = std::make_unique<BroadPhase>();
			m_broadPhase->Initialize();
		}

		// Initialize narrow phase
		if (m_config.enableNarrowPhase) {
			m_narrowPhase = std::make_unique<NarrowPhase>();
			m_narrowPhase->Initialize();
		}

		VOXELCRAFT_LOG_INFO("Collision phases initialized");
	}

	void CollisionManager::InitializeSpatialPartition()
	{
		if (m_config.enableSpatialPartitioning) {
			m_spatialPartition = std::make_unique<SpatialPartition>();
			m_spatialPartition->Initialize(m_config.spatialPartitionSize, m_config.maxObjectsPerPartition);
		}

		VOXELCRAFT_LOG_INFO("Spatial partitioning initialized");
	}

	void CollisionManager::UpdateSpatialPartition()
	{
		if (m_spatialPartition) {
			for (auto& collider : m_colliders) {
				if (collider) {
					m_spatialPartition->UpdateCollider(collider);
				}
			}
		}
	}

	void CollisionManager::CollectCollisionPairs()
	{
		if (m_broadPhase) {
			m_collisionPairs.clear();
			m_broadPhase->Execute(m_colliders, m_collisionPairs);
		}
	}

	void CollisionManager::ProcessCollisionPairs()
	{
		if (m_narrowPhase) {
			// Narrow phase processes pairs and generates contacts
			// This is handled in GenerateContacts
		}
	}

	ContactManifold CollisionManager::GenerateManifold(std::shared_ptr<Collider> colliderA, std::shared_ptr<Collider> colliderB)
	{
		ContactManifold manifold;
		manifold.colliderA = colliderA;
		manifold.colliderB = colliderB;

		// Generate contacts for this pair
		Contact contact;
		if (CheckOverlap(colliderA, colliderB, contact)) {
			manifold.contacts.push_back(contact);
			manifold.normal = contact.normal;
			manifold.friction = CalculateContactFriction(colliderA, colliderB);
			manifold.restitution = CalculateContactRestitution(colliderA, colliderB);
			manifold.persistent = true;
		}

		return manifold;
	}

	void CollisionManager::AddContact(const Contact& contact)
	{
		if (m_contacts.size() < static_cast<size_t>(m_config.maxContacts)) {
			m_contacts.push_back(contact);
		}
	}

	void CollisionManager::RemoveContact(size_t index)
	{
		if (index < m_contacts.size()) {
			m_contacts.erase(m_contacts.begin() + index);
		}
	}

	void CollisionManager::UpdateManifold(ContactManifold& manifold)
	{
		// Update existing manifold with new contact data
		std::vector<Contact> newContacts;

		Contact contact;
		if (CheckOverlap(manifold.colliderA, manifold.colliderB, contact)) {
			newContacts.push_back(contact);
		}

		// Merge new contacts with existing ones
		MergeContacts(manifold, newContacts);
	}

	bool CollisionManager::IsPersistentContact(const Contact& contact) const
	{
		return contact.penetration > m_config.contactPersistenceThreshold;
	}

	void CollisionManager::MergeContacts(ContactManifold& manifold, const std::vector<Contact>& newContacts)
	{
		// Simple contact merging - in a full implementation this would be more sophisticated
		manifold.contacts = newContacts;

		if (!manifold.contacts.empty()) {
			manifold.normal = manifold.contacts[0].normal;
		}
	}

	void CollisionManager::SolveContactConstraint(const Contact& contact, float deltaTime)
	{
		// Basic contact constraint solving
		auto bodyA = contact.colliderA ? contact.colliderA->GetRigidBody() : nullptr;
		auto bodyB = contact.colliderB ? contact.colliderB->GetRigidBody() : nullptr;

		if (!bodyA && !bodyB) {
			return;
		}

		// Calculate relative velocity
		glm::vec3 velA = bodyA ? bodyA->GetVelocityAtPoint(contact.point) : glm::vec3(0.0f);
		glm::vec3 velB = bodyB ? bodyB->GetVelocityAtPoint(contact.point) : glm::vec3(0.0f);
		glm::vec3 relativeVel = velA - velB;

		// Calculate separating velocity
		float separatingVel = glm::dot(relativeVel, contact.normal);

		// Check if bodies are separating
		if (separatingVel > 0.0f) {
			return;
		}

		// Calculate restitution and friction coefficients
		float restitution = contact.restitution;
		float friction = contact.friction;

		// Calculate impulse magnitude
		float impulseMagnitude = -(1.0f + restitution) * separatingVel;

		// Apply mass factors
		float massA = bodyA ? bodyA->GetMass() : 0.0f;
		float massB = bodyB ? bodyB->GetMass() : 0.0f;

		if (massA > 0.0f && massB > 0.0f) {
			impulseMagnitude /= (1.0f / massA + 1.0f / massB);
		} else if (massA > 0.0f) {
			impulseMagnitude *= massA;
		} else if (massB > 0.0f) {
			impulseMagnitude *= massB;
		}

		// Calculate impulse vector
		glm::vec3 impulse = impulseMagnitude * contact.normal;

		// Apply impulses
		if (bodyA) {
			bodyA->ApplyImpulse(-impulse, contact.point);
		}
		if (bodyB) {
			bodyB->ApplyImpulse(impulse, contact.point);
		}

		// Apply friction
		if (friction > 0.0f) {
			// Calculate tangential velocity
			glm::vec3 tangentVel = relativeVel - separatingVel * contact.normal;

			float tangentVelMagnitude = glm::length(tangentVel);
			if (tangentVelMagnitude > 0.001f) {
				glm::vec3 tangentDir = tangentVel / tangentVelMagnitude;

				// Calculate friction impulse
				float frictionImpulseMagnitude = -tangentVelMagnitude * friction;
				frictionImpulseMagnitude = std::min(frictionImpulseMagnitude, impulseMagnitude * friction);

				glm::vec3 frictionImpulse = frictionImpulseMagnitude * tangentDir;

				// Apply friction impulses
				if (bodyA) {
					bodyA->ApplyImpulse(-frictionImpulse, contact.point);
				}
				if (bodyB) {
					bodyB->ApplyImpulse(frictionImpulse, contact.point);
				}
			}
		}
	}

	void CollisionManager::SolveManifoldConstraints(const ContactManifold& manifold, float deltaTime)
	{
		for (const auto& contact : manifold.contacts) {
			SolveContactConstraint(contact, deltaTime);
		}
	}

	float CollisionManager::CalculateContactFriction(std::shared_ptr<Collider> colliderA, std::shared_ptr<Collider> colliderB) const
	{
		// Simple friction calculation based on collider materials
		float frictionA = colliderA ? colliderA->GetFriction() : 0.5f;
		float frictionB = colliderB ? colliderB->GetFriction() : 0.5f;

		return std::sqrt(frictionA * frictionB);
	}

	float CollisionManager::CalculateContactRestitution(std::shared_ptr<Collider> colliderA, std::shared_ptr<Collider> colliderB) const
	{
		// Simple restitution calculation based on collider materials
		float restitutionA = colliderA ? colliderA->GetRestitution() : 0.1f;
		float restitutionB = colliderB ? colliderB->GetRestitution() : 0.1f;

		return std::max(restitutionA, restitutionB);
	}

	bool CollisionManager::RayCastAgainstCollider(const glm::vec3& from, const glm::vec3& to, std::shared_ptr<Collider> collider, RayCastHit& hit)
	{
		// Simple ray cast implementation
		// In a full implementation, this would use proper ray-shape intersection tests

		auto bounds = collider->GetWorldBounds();

		// Simple AABB ray intersection
		glm::vec3 dir = to - from;
		float length = glm::length(dir);

		if (length < 0.001f) {
			return false;
		}

		dir = glm::normalize(dir);

		// Calculate intersection with AABB
		float tmin = 0.0f;
		float tmax = length;

		for (int i = 0; i < 3; ++i) {
			float invD = 1.0f / dir[i];
			float t0 = (bounds.min[i] - from[i]) * invD;
			float t1 = (bounds.max[i] - from[i]) * invD;

			if (invD < 0.0f) {
				std::swap(t0, t1);
			}

			tmin = std::max(tmin, t0);
			tmax = std::min(tmax, t1);

			if (tmax <= tmin) {
				return false;
			}
		}

		if (tmin > length) {
			return false;
		}

		// We have a hit
		hit.collider = collider;
		hit.rigidBody = collider->GetRigidBody();
		hit.point = from + dir * tmin;
		hit.normal = -dir; // Simple normal calculation
		hit.distance = tmin;
		hit.layerMask = collider->GetLayerMask();

		return true;
	}

	bool CollisionManager::SphereCastAgainstCollider(const glm::vec3& from, const glm::vec3& to, float radius, std::shared_ptr<Collider> collider, RayCastHit& hit)
	{
		// Sphere cast implementation
		auto bounds = collider->GetWorldBounds();

		// Expand bounds by sphere radius
		bounds.min -= glm::vec3(radius);
		bounds.max += glm::vec3(radius);

		// Use ray cast against expanded bounds
		return RayCastAgainstCollider(from, to, collider, hit);
	}

	bool CollisionManager::BoxCastAgainstCollider(const glm::vec3& from, const glm::vec3& to, const glm::vec3& halfExtents, std::shared_ptr<Collider> collider, RayCastHit& hit)
	{
		// Box cast implementation
		auto bounds = collider->GetWorldBounds();

		// Expand bounds by box half extents
		bounds.min -= halfExtents;
		bounds.max += halfExtents;

		// Use ray cast against expanded bounds
		return RayCastAgainstCollider(from, to, collider, hit);
	}

	bool CollisionManager::CheckOverlap(std::shared_ptr<Collider> colliderA, std::shared_ptr<Collider> colliderB, Contact& contact)
	{
		// Simple overlap check using AABB
		auto boundsA = colliderA->GetWorldBounds();
		auto boundsB = colliderB->GetWorldBounds();

		// Check for AABB overlap
		bool overlap = (boundsA.min.x <= boundsB.max.x && boundsA.max.x >= boundsB.min.x) &&
					  (boundsA.min.y <= boundsB.max.y && boundsA.max.y >= boundsB.min.y) &&
					  (boundsA.min.z <= boundsB.max.z && boundsA.max.z >= boundsB.min.z);

		if (overlap) {
			// Generate contact information
			contact.colliderA = colliderA;
			contact.colliderB = colliderB;

			// Calculate penetration and normal (simplified)
			glm::vec3 centerA = (boundsA.min + boundsA.max) * 0.5f;
			glm::vec3 centerB = (boundsB.min + boundsB.max) * 0.5f;
			glm::vec3 diff = centerA - centerB;

			// Calculate overlap on each axis
			glm::vec3 overlap;
			overlap.x = std::min(boundsA.max.x - boundsB.min.x, boundsB.max.x - boundsA.min.x);
			overlap.y = std::min(boundsA.max.y - boundsB.min.y, boundsB.max.y - boundsA.min.y);
			overlap.z = std::min(boundsA.max.z - boundsB.min.z, boundsB.max.z - boundsA.min.z);

			// Find axis with minimum overlap
			if (overlap.x < overlap.y && overlap.x < overlap.z) {
				contact.normal = glm::vec3(diff.x > 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f);
				contact.penetration = overlap.x;
			} else if (overlap.y < overlap.z) {
				contact.normal = glm::vec3(0.0f, diff.y > 0.0f ? 1.0f : -1.0f, 0.0f);
				contact.penetration = overlap.y;
			} else {
				contact.normal = glm::vec3(0.0f, 0.0f, diff.z > 0.0f ? 1.0f : -1.0f);
				contact.penetration = overlap.z;
			}

			// Calculate contact point (simplified)
			contact.point = (boundsA.min + boundsA.max + boundsB.min + boundsB.max) * 0.25f;

			// Calculate friction and restitution
			contact.friction = CalculateContactFriction(colliderA, colliderB);
			contact.restitution = CalculateContactRestitution(colliderA, colliderB);

			return true;
		}

		return false;
	}

	bool CollisionManager::CheckLayerCompatibility(uint32_t layerA, uint32_t layerB) const
	{
		// Simple layer compatibility check
		// In a full implementation, this would use a layer matrix
		return (layerA & layerB) != 0;
	}

	void CollisionManager::UpdateStats()
	{
		// Stats are updated in real-time during collision detection
	}

	bool CollisionManager::ValidateCollisionData() const
	{
		// Validate collision data integrity
		for (const auto& contact : m_contacts) {
			if (!contact.colliderA || !contact.colliderB) {
				return false;
			}

			if (std::isnan(contact.penetration) || contact.penetration < 0.0f) {
				return false;
			}
		}

		for (const auto& manifold : m_manifolds) {
			if (!manifold.colliderA || !manifold.colliderB) {
				return false;
			}

			if (manifold.contacts.empty()) {
				return false;
			}
		}

		return true;
	}

	void CollisionManager::HandleCollisionError(const std::string& error)
	{
		VOXELCRAFT_LOG_ERROR("Collision error: {}", error);

		// Clear problematic collision data
		Clear();
	}

	void CollisionManager::DebugRenderContacts()
	{
		// Render contact points and normals
		for (const auto& contact : m_contacts) {
			// Render contact point
			// DebugRenderer::DrawPoint(contact.point, glm::vec3(1.0f, 0.0f, 0.0f));

			// Render contact normal
			// DebugRenderer::DrawLine(contact.point, contact.point + contact.normal * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
		}
	}

	void CollisionManager::DebugRenderManifolds()
	{
		// Render contact manifolds
		for (const auto& manifold : m_manifolds) {
			for (const auto& contact : manifold.contacts) {
				// Render manifold contacts
				// DebugRenderer::DrawPoint(contact.point, glm::vec3(0.0f, 0.0f, 1.0f));
			}
		}
	}

	void CollisionManager::DebugRenderBroadPhase()
	{
		if (m_spatialPartition) {
			// m_spatialPartition->DebugRender();
		}
	}

} // namespace VoxelCraft
