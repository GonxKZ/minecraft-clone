#include "PhysicsEngine.hpp"
#include "RigidBody.hpp"
#include "Collider.hpp"
#include "CollisionManager.hpp"
#include "ForceManager.hpp"
#include "ConstraintManager.hpp"
#include <algorithm>
#include <cmath>

namespace VoxelCraft {

	PhysicsEngine::PhysicsEngine()
		: m_initialized(false)
		, m_enabled(true)
		, m_paused(false)
		, m_timeScale(1.0f)
		, m_accumulatedTime(0.0f)
		, m_gravity(0.0f, m_config.gravity, 0.0f)
		, m_simulating(false)
	{
		// Initialize statistics
		std::memset(&m_stats, 0, sizeof(PhysicsStats));

		VOXELCRAFT_LOG_INFO("PhysicsEngine initialized");
	}

	PhysicsEngine::~PhysicsEngine()
	{
		Shutdown();
	}

	bool PhysicsEngine::Initialize(const PhysicsConfig& config)
	{
		if (m_initialized) {
			VOXELCRAFT_LOG_WARN("PhysicsEngine already initialized");
			return true;
		}

		m_config = config;
		m_gravity = glm::vec3(0.0f, m_config.gravity, 0.0f);
		m_accumulatedTime = 0.0f;

		try {
			// Initialize core systems
			InitializeSystems();
			InitializeMaterials();
			InitializeLayers();

			// Start physics threads if multithreading is enabled
			if (m_config.enableMultithreading) {
				StartPhysicsThreads();
			}

			// Initialize timing
			m_lastStatsUpdate = std::chrono::steady_clock::now();
			m_frameStartTime = std::chrono::steady_clock::now();

			m_initialized = true;
			VOXELCRAFT_LOG_INFO("PhysicsEngine initialized successfully with {} threads", m_config.numPhysicsThreads);

			return true;
		}
		catch (const std::exception& e) {
			VOXELCRAFT_LOG_ERROR("Failed to initialize PhysicsEngine: {}", e.what());
			Shutdown();
			return false;
		}
	}

	void PhysicsEngine::Shutdown()
	{
		if (!m_initialized) {
			return;
		}

		VOXELCRAFT_LOG_INFO("Shutting down PhysicsEngine...");

		// Stop physics threads
		StopPhysicsThreads();

		// Clear all physics objects
		m_rigidBodies.clear();
		m_colliders.clear();
		m_constraints.clear();
		m_characterControllers.clear();
		m_vehicles.clear();
		m_softBodies.clear();

		// Clear materials and layers
		m_materials.clear();
		m_layerMatrix.clear();

		// Shutdown core systems
		m_physicsWorld.reset();
		m_collisionManager.reset();
		m_forceManager.reset();
		m_constraintManager.reset();

		m_initialized = false;

		VOXELCRAFT_LOG_INFO("PhysicsEngine shutdown complete");
	}

	void PhysicsEngine::Update(float deltaTime)
	{
		if (!m_initialized || !m_enabled || m_paused) {
			return;
		}

		auto startTime = std::chrono::steady_clock::now();

		// Apply time scale
		deltaTime *= m_timeScale;

		// Accumulate time for fixed timestep
		m_accumulatedTime += deltaTime;

		// Update physics in fixed timesteps
		int steps = 0;
		while (m_accumulatedTime >= m_config.fixedTimeStep && steps < m_config.maxSubSteps) {
			FixedUpdate(m_config.fixedTimeStep);
			m_accumulatedTime -= m_config.fixedTimeStep;
			steps++;
		}

		// If too many substeps, prevent spiral of death
		if (m_accumulatedTime >= m_config.fixedTimeStep) {
			m_accumulatedTime = 0.0f;
		}

		auto endTime = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

		// Update frame statistics
		m_stats.frameCount++;
		m_stats.averageFPS = 1.0f / deltaTime;
		if (m_stats.frameCount == 1 || deltaTime < 1.0f / m_stats.minFPS) {
			m_stats.minFPS = 1.0f / deltaTime;
		}
		if (m_stats.frameCount == 1 || deltaTime > 1.0f / m_stats.maxFPS) {
			m_stats.maxFPS = 1.0f / deltaTime;
		}

		// Update stats every second
		auto now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::seconds>(now - m_lastStatsUpdate).count() >= 1) {
			UpdateStats();
			m_lastStatsUpdate = now;
		}
	}

	void PhysicsEngine::FixedUpdate(float fixedDeltaTime)
	{
		if (!m_initialized || !m_enabled) {
			return;
		}

		if (m_config.enableMultithreading) {
			// Multithreaded physics simulation
			SimulatePhysics(fixedDeltaTime);
		} else {
			// Single-threaded physics simulation
			auto collisionStart = std::chrono::steady_clock::now();
			DetectCollisions();
			auto collisionEnd = std::chrono::steady_clock::now();

			auto solverStart = std::chrono::steady_clock::now();
			ResolveCollisions();
			SolveConstraints();
			auto solverEnd = std::chrono::steady_clock::now();

			IntegrateRigidBodies(fixedDeltaTime);
			UpdateSleepingBodies();

			// Update timing statistics
			auto collisionTime = std::chrono::duration_cast<std::chrono::microseconds>(collisionEnd - collisionStart);
			auto solverTime = std::chrono::duration_cast<std::chrono::microseconds>(solverEnd - solverStart);

			m_stats.collisionTime = collisionTime.count() / 1000000.0f;
			m_stats.solverTime = solverTime.count() / 1000000.0f;
		}

		// Validate physics state periodically
		if (m_stats.frameCount % 1000 == 0) {
			if (!ValidatePhysicsState()) {
				VOXELCRAFT_LOG_WARN("Physics state validation failed");
			}
		}
	}

	void PhysicsEngine::DebugRender()
	{
		if (!m_config.enableDebugDrawing) {
			return;
		}

		// Debug render rigid bodies
		for (auto& body : m_rigidBodies) {
			if (body && body->IsActive()) {
				body->DebugRender();
			}
		}

		// Debug render colliders
		for (auto& collider : m_colliders) {
			if (collider) {
				collider->DebugRender();
			}
		}

		// Debug render constraints
		for (auto& constraint : m_constraints) {
			if (constraint) {
				constraint->DebugRender();
			}
		}

		// Debug render contact points
		if (m_collisionManager) {
			m_collisionManager->DebugRender();
		}
	}

	std::shared_ptr<RigidBody> PhysicsEngine::CreateRigidBody(const std::string& name)
	{
		if (!m_initialized) {
			return nullptr;
		}

		std::unique_lock<std::mutex> lock(m_physicsMutex);

		if (m_rigidBodies.size() >= static_cast<size_t>(m_config.maxRigidBodies)) {
			VOXELCRAFT_LOG_WARN("Maximum rigid body count reached ({})", m_config.maxRigidBodies);
			return nullptr;
		}

		auto body = std::make_shared<RigidBody>(name);
		body->SetPhysicsEngine(shared_from_this());

		m_rigidBodies.push_back(body);
		m_stats.rigidBodyCount++;

		return body;
	}

	void PhysicsEngine::DestroyRigidBody(std::shared_ptr<RigidBody> body)
	{
		if (!body) {
			return;
		}

		std::unique_lock<std::mutex> lock(m_physicsMutex);

		auto it = std::find(m_rigidBodies.begin(), m_rigidBodies.end(), body);
		if (it != m_rigidBodies.end()) {
			// Remove from physics world
			if (m_physicsWorld) {
				m_physicsWorld->RemoveRigidBody(body);
			}

			// Remove associated colliders
			for (auto& collider : body->GetColliders()) {
				DestroyCollider(collider);
			}

			// Remove associated constraints
			std::vector<std::shared_ptr<Constraint>> constraintsToRemove;
			for (auto& constraint : m_constraints) {
				if (constraint->GetBodyA() == body || constraint->GetBodyB() == body) {
					constraintsToRemove.push_back(constraint);
				}
			}

			for (auto& constraint : constraintsToRemove) {
				DestroyConstraint(constraint);
			}

			m_rigidBodies.erase(it);
			m_stats.rigidBodyCount--;
		}
	}

	std::shared_ptr<Collider> PhysicsEngine::CreateCollider(ShapeType type)
	{
		if (!m_initialized) {
			return nullptr;
		}

		std::unique_lock<std::mutex> lock(m_physicsMutex);

		if (m_colliders.size() >= static_cast<size_t>(m_config.maxColliders)) {
			VOXELCRAFT_LOG_WARN("Maximum collider count reached ({})", m_config.maxColliders);
			return nullptr;
		}

		auto collider = std::make_shared<Collider>(type);
		collider->SetPhysicsEngine(shared_from_this());

		m_colliders.push_back(collider);

		return collider;
	}

	void PhysicsEngine::DestroyCollider(std::shared_ptr<Collider> collider)
	{
		if (!collider) {
			return;
		}

		std::unique_lock<std::mutex> lock(m_physicsMutex);

		auto it = std::find(m_colliders.begin(), m_colliders.end(), collider);
		if (it != m_colliders.end()) {
			// Remove from collision manager
			if (m_collisionManager) {
				m_collisionManager->RemoveCollider(collider);
			}

			// Detach from rigid body
			if (auto body = collider->GetRigidBody()) {
				body->RemoveCollider(collider);
			}

			m_colliders.erase(it);
		}
	}

	void PhysicsEngine::AddForce(std::shared_ptr<RigidBody> body, const glm::vec3& force, ForceType type)
	{
		if (!body || !m_forceManager) {
			return;
		}

		m_forceManager->AddForce(body, force, type);
	}

	void PhysicsEngine::AddTorque(std::shared_ptr<RigidBody> body, const glm::vec3& torque)
	{
		if (!body || !m_forceManager) {
			return;
		}

		m_forceManager->AddTorque(body, torque);
	}

	void PhysicsEngine::ApplyImpulse(std::shared_ptr<RigidBody> body, const glm::vec3& impulse, const glm::vec3& point)
	{
		if (!body) {
			return;
		}

		body->ApplyImpulse(impulse, point);
	}

	void PhysicsEngine::ApplyTorqueImpulse(std::shared_ptr<RigidBody> body, const glm::vec3& torqueImpulse)
	{
		if (!body) {
			return;
		}

		body->ApplyTorqueImpulse(torqueImpulse);
	}

	bool PhysicsEngine::RayCast(const glm::vec3& from, const glm::vec3& to, RayCastHit& hit, uint32_t layerMask)
	{
		if (!m_initialized || !m_collisionManager) {
			return false;
		}

		return m_collisionManager->RayCast(from, to, hit, layerMask);
	}

	bool PhysicsEngine::SphereCast(const glm::vec3& from, const glm::vec3& to, float radius, RayCastHit& hit, uint32_t layerMask)
	{
		if (!m_initialized || !m_collisionManager) {
			return false;
		}

		return m_collisionManager->SphereCast(from, to, radius, hit, layerMask);
	}

	bool PhysicsEngine::BoxCast(const glm::vec3& from, const glm::vec3& to, const glm::vec3& halfExtents, RayCastHit& hit, uint32_t layerMask)
	{
		if (!m_initialized || !m_collisionManager) {
			return false;
		}

		return m_collisionManager->BoxCast(from, to, halfExtents, hit, layerMask);
	}

	bool PhysicsEngine::OverlapSphere(const glm::vec3& center, float radius, std::vector<OverlapResult>& results, uint32_t layerMask)
	{
		if (!m_initialized || !m_collisionManager) {
			return false;
		}

		return m_collisionManager->OverlapSphere(center, radius, results, layerMask);
	}

	bool PhysicsEngine::OverlapBox(const glm::vec3& center, const glm::vec3& halfExtents, std::vector<OverlapResult>& results, uint32_t layerMask)
	{
		if (!m_initialized || !m_collisionManager) {
			return false;
		}

		return m_collisionManager->OverlapBox(center, halfExtents, results, layerMask);
	}

	bool PhysicsEngine::SweepTest(std::shared_ptr<Collider> collider, const glm::vec3& direction, float distance, SweepResult& result)
	{
		if (!m_initialized || !m_collisionManager || !collider) {
			return false;
		}

		return m_collisionManager->SweepTest(collider, direction, distance, result);
	}

	void PhysicsEngine::SetConfig(const PhysicsConfig& config)
	{
		m_config = config;

		// Update gravity
		m_gravity = glm::vec3(0.0f, m_config.gravity, 0.0f);

		// Restart physics threads if multithreading setting changed
		if (m_config.enableMultithreading && !m_simulating) {
			StartPhysicsThreads();
		} else if (!m_config.enableMultithreading && m_simulating) {
			StopPhysicsThreads();
		}

		VOXELCRAFT_LOG_INFO("PhysicsEngine configuration updated");
	}

	void PhysicsEngine::SetGravity(const glm::vec3& gravity)
	{
		m_gravity = gravity;
	}

	std::vector<std::shared_ptr<RigidBody>> PhysicsEngine::GetRigidBodies() const
	{
		std::unique_lock<std::mutex> lock(m_physicsMutex);
		return m_rigidBodies;
	}

	std::vector<std::shared_ptr<RigidBody>> PhysicsEngine::GetActiveRigidBodies() const
	{
		std::unique_lock<std::mutex> lock(m_physicsMutex);

		std::vector<std::shared_ptr<RigidBody>> activeBodies;
		for (auto& body : m_rigidBodies) {
			if (body && body->IsActive()) {
				activeBodies.push_back(body);
			}
		}

		return activeBodies;
	}

	std::vector<std::shared_ptr<RigidBody>> PhysicsEngine::GetSleepingRigidBodies() const
	{
		std::unique_lock<std::mutex> lock(m_physicsMutex);

		std::vector<std::shared_ptr<RigidBody>> sleepingBodies;
		for (auto& body : m_rigidBodies) {
			if (body && body->IsSleeping()) {
				sleepingBodies.push_back(body);
			}
		}

		return sleepingBodies;
	}

	std::shared_ptr<CharacterController> PhysicsEngine::CreateCharacterController()
	{
		if (!m_initialized) {
			return nullptr;
		}

		auto controller = std::make_shared<CharacterController>();
		controller->SetPhysicsEngine(shared_from_this());

		m_characterControllers.push_back(controller);

		return controller;
	}

	std::shared_ptr<Vehicle> PhysicsEngine::CreateVehicle()
	{
		if (!m_initialized) {
			return nullptr;
		}

		auto vehicle = std::make_shared<Vehicle>();
		vehicle->SetPhysicsEngine(shared_from_this());

		m_vehicles.push_back(vehicle);

		return vehicle;
	}

	std::shared_ptr<SoftBody> PhysicsEngine::CreateSoftBody()
	{
		if (!m_initialized) {
			return nullptr;
		}

		auto softBody = std::make_shared<SoftBody>();
		softBody->SetPhysicsEngine(shared_from_this());

		m_softBodies.push_back(softBody);

		return softBody;
	}

	std::shared_ptr<Constraint> PhysicsEngine::CreateConstraint(ConstraintType type, std::shared_ptr<RigidBody> bodyA, std::shared_ptr<RigidBody> bodyB)
	{
		if (!m_initialized || !m_constraintManager) {
			return nullptr;
		}

		auto constraint = m_constraintManager->CreateConstraint(type, bodyA, bodyB);
		if (constraint) {
			m_constraints.push_back(constraint);
		}

		return constraint;
	}

	void PhysicsEngine::DestroyConstraint(std::shared_ptr<Constraint> constraint)
	{
		if (!constraint || !m_constraintManager) {
			return;
		}

		std::unique_lock<std::mutex> lock(m_physicsMutex);

		auto it = std::find(m_constraints.begin(), m_constraints.end(), constraint);
		if (it != m_constraints.end()) {
			m_constraintManager->DestroyConstraint(constraint);
			m_constraints.erase(it);
		}
	}

	void PhysicsEngine::AddMaterial(const std::string& name, const PhysicsMaterial& material)
	{
		std::unique_lock<std::mutex> lock(m_physicsMutex);
		m_materials[name] = material;
	}

	const PhysicsMaterial& PhysicsEngine::GetMaterial(const std::string& name) const
	{
		auto it = m_materials.find(name);
		if (it != m_materials.end()) {
			return it->second;
		}

		// Return default material
		static PhysicsMaterial defaultMaterial;
		return defaultMaterial;
	}

	void PhysicsEngine::SetLayerCollision(uint32_t layer1, uint32_t layer2, bool collide)
	{
		if (layer1 >= m_layerMatrix.size() || layer2 >= m_layerMatrix.size()) {
			return;
		}

		m_layerMatrix[layer1][layer2] = collide;
		m_layerMatrix[layer2][layer1] = collide;
	}

	bool PhysicsEngine::GetLayerCollision(uint32_t layer1, uint32_t layer2) const
	{
		if (layer1 >= m_layerMatrix.size() || layer2 >= m_layerMatrix.size()) {
			return true; // Default to colliding
		}

		return m_layerMatrix[layer1][layer2];
	}

	void PhysicsEngine::InitializeSystems()
	{
		// Create physics world
		m_physicsWorld = std::make_unique<PhysicsWorld>();

		// Create collision manager
		m_collisionManager = std::make_unique<CollisionManager>();

		// Create force manager
		m_forceManager = std::make_unique<ForceManager>();

		// Create constraint manager
		m_constraintManager = std::make_unique<ConstraintManager>();

		VOXELCRAFT_LOG_INFO("Physics systems initialized");
	}

	void PhysicsEngine::InitializeMaterials()
	{
		// Add default materials
		AddMaterial("default", PhysicsMaterial(0.5f, 0.1f, 1.0f));
		AddMaterial("wood", PhysicsMaterial(0.6f, 0.2f, 0.8f));
		AddMaterial("metal", PhysicsMaterial(0.3f, 0.5f, 2.7f));
		AddMaterial("stone", PhysicsMaterial(0.8f, 0.1f, 2.5f));
		AddMaterial("rubber", PhysicsMaterial(0.9f, 0.8f, 1.1f));
		AddMaterial("ice", PhysicsMaterial(0.1f, 0.05f, 0.9f));
		AddMaterial("glass", PhysicsMaterial(0.4f, 0.2f, 2.4f));
		AddMaterial("dirt", PhysicsMaterial(0.7f, 0.1f, 1.5f));
		AddMaterial("sand", PhysicsMaterial(0.6f, 0.1f, 1.6f));
		AddMaterial("water", PhysicsMaterial(0.1f, 0.0f, 1.0f));

		VOXELCRAFT_LOG_INFO("Physics materials initialized: {} materials", m_materials.size());
	}

	void PhysicsEngine::InitializeLayers()
	{
		// Initialize 32 collision layers
		const uint32_t numLayers = 32;
		m_layerMatrix.resize(numLayers);
		for (uint32_t i = 0; i < numLayers; ++i) {
			m_layerMatrix[i].resize(numLayers, true); // Default to all layers colliding
		}

		// Set up common layer interactions
		// Layer 0: Default
		// Layer 1: Player
		// Layer 2: Enemy
		// Layer 3: Projectile
		// Layer 4: Terrain
		// Layer 5: Water
		// Layer 6: Trigger
		// Layer 7: UI

		// Player doesn't collide with enemies (for now)
		SetLayerCollision(1, 2, false);

		// Projectiles don't collide with their owners
		SetLayerCollision(3, 1, false);
		SetLayerCollision(3, 2, false);

		// Water doesn't collide with most things
		SetLayerCollision(5, 1, false);
		SetLayerCollision(5, 2, false);
		SetLayerCollision(5, 3, false);

		// Triggers don't collide with anything physical
		for (uint32_t i = 0; i < numLayers; ++i) {
			if (i != 6) { // Except other triggers
				SetLayerCollision(6, i, false);
			}
		}

		VOXELCRAFT_LOG_INFO("Physics collision layers initialized");
	}

	void PhysicsEngine::StartPhysicsThreads()
	{
		if (m_simulating) {
			return;
		}

		m_simulating = true;

		for (int i = 0; i < m_config.numPhysicsThreads; ++i) {
			m_physicsThreads.emplace_back(&PhysicsEngine::PhysicsThreadFunction, this, i);
		}

		VOXELCRAFT_LOG_INFO("Physics threads started: {}", m_config.numPhysicsThreads);
	}

	void PhysicsEngine::StopPhysicsThreads()
	{
		if (!m_simulating) {
			return;
		}

		m_simulating = false;

		for (auto& thread : m_physicsThreads) {
			if (thread.joinable()) {
				thread.join();
			}
		}

		m_physicsThreads.clear();

		VOXELCRAFT_LOG_INFO("Physics threads stopped");
	}

	void PhysicsEngine::SimulatePhysics(float deltaTime)
	{
		if (!m_physicsWorld || !m_collisionManager || !m_constraintManager) {
			return;
		}

		// Update forces
		if (m_forceManager) {
			m_forceManager->Update(deltaTime);
		}

		// Detect collisions
		auto broadPhaseStart = std::chrono::steady_clock::now();
		m_collisionManager->BroadPhase();
		auto broadPhaseEnd = std::chrono::steady_clock::now();

		auto narrowPhaseStart = std::chrono::steady_clock::now();
		m_collisionManager->NarrowPhase();
		auto narrowPhaseEnd = std::chrono::steady_clock::now();

		// Resolve collisions and solve constraints
		auto solverStart = std::chrono::steady_clock::now();
		m_collisionManager->ResolveCollisions();
		m_constraintManager->SolveConstraints();
		auto solverEnd = std::chrono::steady_clock::now();

		// Integrate rigid bodies
		m_physicsWorld->Integrate(deltaTime);

		// Update sleeping bodies
		UpdateSleepingBodies();

		// Update timing statistics
		auto broadPhaseTime = std::chrono::duration_cast<std::chrono::microseconds>(broadPhaseEnd - broadPhaseStart);
		auto narrowPhaseTime = std::chrono::duration_cast<std::chrono::microseconds>(narrowPhaseEnd - narrowPhaseStart);
		auto solverTime = std::chrono::duration_cast<std::chrono::microseconds>(solverEnd - solverStart);

		m_stats.broadPhaseTime = broadPhaseTime.count() / 1000000.0f;
		m_stats.narrowPhaseTime = narrowPhaseTime.count() / 1000000.0f;
		m_stats.solverTime = solverTime.count() / 1000000.0f;
	}

	void PhysicsEngine::DetectCollisions()
	{
		if (m_collisionManager) {
			m_collisionManager->DetectCollisions();
		}
	}

	void PhysicsEngine::ResolveCollisions()
	{
		if (m_collisionManager) {
			m_collisionManager->ResolveCollisions();
		}
	}

	void PhysicsEngine::SolveConstraints()
	{
		if (m_constraintManager) {
			m_constraintManager->SolveConstraints();
		}
	}

	void PhysicsEngine::IntegrateRigidBodies(float deltaTime)
	{
		for (auto& body : m_rigidBodies) {
			if (body && body->IsActive()) {
				body->Integrate(deltaTime);
			}
		}
	}

	void PhysicsEngine::UpdateSleepingBodies()
	{
		if (!m_config.enableSleeping) {
			return;
		}

		std::unique_lock<std::mutex> lock(m_physicsMutex);

		int sleepingCount = 0;
		int activeCount = 0;

		for (auto& body : m_rigidBodies) {
			if (body) {
				if (ShouldSleep(body)) {
					body->SetSleeping(true);
					sleepingCount++;
				} else {
					body->SetSleeping(false);
					activeCount++;
				}
			}
		}

		m_stats.activeBodyCount = activeCount;
		m_stats.sleepingBodyCount = sleepingCount;
	}

	void PhysicsEngine::UpdateStats()
	{
		std::unique_lock<std::mutex> lock(m_physicsMutex);

		m_stats.rigidBodyCount = m_rigidBodies.size();
		m_stats.collisionCount = m_collisionManager ? m_collisionManager->GetCollisionCount() : 0;
		m_stats.contactCount = m_collisionManager ? m_collisionManager->GetContactCount() : 0;
		m_stats.constraintCount = m_constraints.size();

		// Calculate memory usage
		size_t memory = 0;
		memory += m_rigidBodies.size() * sizeof(RigidBody);
		memory += m_colliders.size() * sizeof(Collider);
		memory += m_constraints.size() * sizeof(Constraint);
		m_stats.memoryUsed = memory;

		if (memory > m_stats.peakMemoryUsed) {
			m_stats.peakMemoryUsed = memory;
		}

		// Calculate simulation time
		auto now = std::chrono::steady_clock::now();
		auto frameTime = std::chrono::duration_cast<std::chrono::microseconds>(now - m_frameStartTime);
		m_stats.simulationTime = frameTime.count() / 1000000.0f;
		m_frameStartTime = now;
	}

	void PhysicsEngine::PhysicsThreadFunction(int threadIndex)
	{
		VOXELCRAFT_LOG_INFO("Physics thread {} started", threadIndex);

		while (m_simulating) {
			// Wait for work to do
			std::this_thread::sleep_for(std::chrono::milliseconds(1));

			// Process physics work for this thread
			// In a full implementation, this would process a subset of rigid bodies
			// and colliders assigned to this thread
		}

		VOXELCRAFT_LOG_INFO("Physics thread {} stopped", threadIndex);
	}

	bool PhysicsEngine::ShouldSleep(std::shared_ptr<RigidBody> body) const
	{
		if (!body || !m_config.enableSleeping) {
			return false;
		}

		// Check velocity threshold
		float velocity = glm::length(body->GetLinearVelocity());
		float angularVelocity = glm::length(body->GetAngularVelocity());

		if (velocity > m_config.sleepThreshold || angularVelocity > m_config.sleepThreshold) {
			return false;
		}

		// Check if body has been below threshold for enough time
		// In a full implementation, this would track time below threshold

		return true;
	}

	void PhysicsEngine::WakeUpNearbyBodies(std::shared_ptr<RigidBody> body)
	{
		if (!body) {
			return;
		}

		// Find bodies within wake radius
		float wakeRadius = 5.0f; // Configurable
		auto position = body->GetPosition();

		for (auto& otherBody : m_rigidBodies) {
			if (otherBody && otherBody != body && otherBody->IsSleeping()) {
				float distance = glm::distance(position, otherBody->GetPosition());
				if (distance <= wakeRadius) {
					otherBody->SetSleeping(false);
				}
			}
		}
	}

	bool PhysicsEngine::ValidatePhysicsState() const
	{
		bool valid = true;

		for (auto& body : m_rigidBodies) {
			if (body) {
				// Check for NaN or infinite values
				auto position = body->GetPosition();
				auto velocity = body->GetLinearVelocity();
				auto rotation = body->GetRotation();

				if (std::isnan(position.x) || std::isnan(position.y) || std::isnan(position.z) ||
					std::isinf(position.x) || std::isinf(position.y) || std::isinf(position.z)) {
					valid = false;
				}

				if (std::isnan(velocity.x) || std::isnan(velocity.y) || std::isnan(velocity.z) ||
					std::isinf(velocity.x) || std::isinf(velocity.y) || std::isinf(velocity.z)) {
					valid = false;
				}

				if (std::isnan(rotation.x) || std::isnan(rotation.y) || std::isnan(rotation.z) || std::isnan(rotation.w) ||
					std::isinf(rotation.x) || std::isinf(rotation.y) || std::isinf(rotation.z) || std::isinf(rotation.w)) {
					valid = false;
				}
			}
		}

		return valid;
	}

	void PhysicsEngine::HandlePhysicsError(const std::string& error)
	{
		VOXELCRAFT_LOG_ERROR("Physics error: {}", error);

		// In a full implementation, this could:
		// - Reset problematic physics objects
		// - Disable physics temporarily
		// - Send error reports
		// - Attempt recovery procedures
	}

} // namespace VoxelCraft
