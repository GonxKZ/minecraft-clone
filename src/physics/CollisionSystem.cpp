/**
 * @file CollisionSystem.cpp
 * @brief VoxelCraft Collision Detection and Physics System Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "CollisionSystem.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>

#include "../math/MathUtils.hpp"
#include "../entities/Player.hpp"
#include "../world/World.hpp"
#include "../utils/Logger.hpp"
#include "../utils/Random.hpp"

namespace VoxelCraft {

    // Static instance
    static CollisionSystem* s_instance = nullptr;

    // Broadphase data structure
    struct BroadphaseData {
        std::vector<CollisionObject*> objects;
        std::vector<AABB> aabbs;
        std::vector<int> indices;

        // For spatial hashing
        std::unordered_map<uint64_t, std::vector<CollisionObject*>> spatialHash;
        float cellSize = 10.0f;

        // For BVH
        struct BVHNode {
            AABB bounds;
            int left = -1;
            int right = -1;
            int objectIndex = -1;
            bool isLeaf = false;
        };
        std::vector<BVHNode> bvhNodes;
        int rootNode = -1;

        // For sweep and prune
        std::vector<CollisionObject*> sortedX;
        std::vector<CollisionObject*> sortedY;
        std::vector<CollisionObject*> sortedZ;
    };

    // CollisionObject implementation
    CollisionObject::CollisionObject() = default;
    CollisionObject::~CollisionObject() = default;

    // AABBCollisionObject implementation
    AABBCollisionObject::AABBCollisionObject(const AABB& localAABB)
        : m_localAABB(localAABB) {
        m_shape = CollisionShape::AABB;
        UpdateBounds();
    }

    void AABBCollisionObject::UpdateBounds() {
        // Transform local AABB to world space
        Vec3 worldMin = m_position + m_localAABB.min * m_scale;
        Vec3 worldMax = m_position + m_localAABB.max * m_scale;

        m_worldAABB = AABB::FromMinMax(worldMin, worldMax);
    }

    // OBBCollisionObject implementation
    OBBCollisionObject::OBBCollisionObject(const OBB& localOBB)
        : m_localOBB(localOBB) {
        m_shape = CollisionShape::OBB;
        UpdateBounds();
    }

    void OBBCollisionObject::UpdateBounds() {
        OBB worldOBB = GetWorldOBB();
        std::vector<Vec3> corners;
        worldOBB.GetCorners(corners);

        if (!corners.empty()) {
            m_worldAABB = AABB::FromMinMax(corners[0], corners[0]);
            for (size_t i = 1; i < corners.size(); ++i) {
                m_worldAABB.Expand(corners[i]);
            }
        }
    }

    OBB OBBCollisionObject::GetWorldOBB() const {
        OBB worldOBB = m_localOBB;
        worldOBB.center = m_position + m_rotation.Rotate(m_localOBB.center * m_scale);
        worldOBB.extents = m_localOBB.extents * m_scale;
        worldOBB.rotation = m_rotation * m_localOBB.rotation;
        return worldOBB;
    }

    // SphereCollisionObject implementation
    SphereCollisionObject::SphereCollisionObject(const Sphere& localSphere)
        : m_localSphere(localSphere) {
        m_shape = CollisionShape::SPHERE;
        UpdateBounds();
    }

    void SphereCollisionObject::UpdateBounds() {
        Sphere worldSphere = GetWorldSphere();
        Vec3 radiusVec = Vec3(worldSphere.radius);
        m_worldAABB = AABB::FromCenterExtents(worldSphere.center, radiusVec);
    }

    Sphere SphereCollisionObject::GetWorldSphere() const {
        Sphere worldSphere;
        worldSphere.center = m_position + m_rotation.Rotate(m_localSphere.center * m_scale);
        worldSphere.radius = m_localSphere.radius * std::max(std::max(m_scale.x, m_scale.y), m_scale.z);
        return worldSphere;
    }

    // CapsuleCollisionObject implementation
    CapsuleCollisionObject::CapsuleCollisionObject(const Capsule& localCapsule)
        : m_localCapsule(localCapsule) {
        m_shape = CollisionShape::CAPSULE;
        UpdateBounds();
    }

    void CapsuleCollisionObject::UpdateBounds() {
        Capsule worldCapsule = GetWorldCapsule();

        // Calculate AABB for capsule
        Vec3 radiusVec = Vec3(worldCapsule.radius);
        AABB startAABB = AABB::FromCenterExtents(worldCapsule.start, radiusVec);
        AABB endAABB = AABB::FromCenterExtents(worldCapsule.end, radiusVec);

        m_worldAABB = startAABB;
        m_worldAABB.Expand(endAABB);
    }

    Capsule CapsuleCollisionObject::GetWorldCapsule() const {
        Capsule worldCapsule;
        worldCapsule.start = m_position + m_rotation.Rotate(m_localCapsule.start * m_scale);
        worldCapsule.end = m_position + m_rotation.Rotate(m_localCapsule.end * m_scale);
        worldCapsule.radius = m_localCapsule.radius * std::max(std::max(m_scale.x, m_scale.y), m_scale.z);
        return worldCapsule;
    }

    CollisionSystem& CollisionSystem::GetInstance() {
        if (!s_instance) {
            s_instance = new CollisionSystem();
        }
        return *s_instance;
    }

    bool CollisionSystem::Initialize(const CollisionConfig& config) {
        if (m_initialized) {
            Logger::Warning("CollisionSystem already initialized");
            return true;
        }

        m_config = config;
        m_initialized = true;
        m_debugMode = false;

        // Initialize broadphase data
        m_broadphaseData = std::make_unique<BroadphaseData>();
        m_broadphaseData->cellSize = config.cellSize;

        Logger::Info("CollisionSystem initialized with {} collision objects",
                    m_objects.size());

        return true;
    }

    void CollisionSystem::Shutdown() {
        if (!m_initialized) {
            return;
        }

        // Clear all collision objects
        m_objects.clear();
        m_staticObjects.clear();
        m_dynamicObjects.clear();

        // Clear broadphase data
        if (m_broadphaseData) {
            m_broadphaseData->objects.clear();
            m_broadphaseData->aabbs.clear();
            m_broadphaseData->indices.clear();
            m_broadphaseData->spatialHash.clear();
            m_broadphaseData->bvhNodes.clear();
            m_broadphaseData->sortedX.clear();
            m_broadphaseData->sortedY.clear();
            m_broadphaseData->sortedZ.clear();
        }

        // Clear manifolds and pairs
        m_manifolds.clear();
        m_broadphasePairs.clear();

        // Clear cache
        m_aabbCache.clear();
        m_cacheVersion.clear();

        m_initialized = false;
        Logger::Info("CollisionSystem shutdown");
    }

    void CollisionSystem::Update(float deltaTime) {
        if (!m_initialized || !m_config.enableCollision) {
            return;
        }

        auto startTime = std::chrono::high_resolution_clock::now();

        // Update broadphase
        if (m_config.enableBroadphase) {
            UpdateBroadphase();
        }

        // Detect collisions
        int collisions = DetectCollisions(deltaTime);

        // Resolve collisions
        if (!m_manifolds.empty()) {
            ResolveCollisions(m_manifolds, deltaTime);
        }

        // Integrate physics
        IntegratePhysics(deltaTime);

        // Update statistics
        auto endTime = std::chrono::high_resolution_clock::now();
        m_stats.collisionTime = std::chrono::duration<float>(endTime - startTime).count();

        // Clear manifolds for next frame
        m_manifolds.clear();
    }

    bool CollisionSystem::RegisterObject(CollisionObject* object) {
        if (!object || !m_initialized) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_collisionMutex);

        // Check if already registered
        if (std::find(m_objects.begin(), m_objects.end(), object) != m_objects.end()) {
            return false;
        }

        // Add to appropriate list
        m_objects.push_back(object);
        if (object->IsStatic()) {
            m_staticObjects.push_back(object);
        } else {
            m_dynamicObjects.push_back(object);
        }

        // Add to broadphase
        if (m_broadphaseData) {
            m_broadphaseData->objects.push_back(object);
            m_broadphaseData->aabbs.push_back(object->GetWorldAABB());
            m_broadphaseData->indices.push_back(static_cast<int>(m_broadphaseData->objects.size() - 1));
        }

        m_stats.totalObjects++;
        Logger::Debug("Collision object registered: {}", static_cast<void*>(object));

        return true;
    }

    bool CollisionSystem::UnregisterObject(CollisionObject* object) {
        if (!object) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_collisionMutex);

        // Remove from main list
        auto it = std::find(m_objects.begin(), m_objects.end(), object);
        if (it == m_objects.end()) {
            return false;
        }

        m_objects.erase(it);

        // Remove from static/dynamic lists
        if (object->IsStatic()) {
            auto staticIt = std::find(m_staticObjects.begin(), m_staticObjects.end(), object);
            if (staticIt != m_staticObjects.end()) {
                m_staticObjects.erase(staticIt);
            }
        } else {
            auto dynamicIt = std::find(m_dynamicObjects.begin(), m_dynamicObjects.end(), object);
            if (dynamicIt != m_dynamicObjects.end()) {
                m_dynamicObjects.erase(dynamicIt);
            }
        }

        // Remove from broadphase
        if (m_broadphaseData) {
            auto broadphaseIt = std::find(m_broadphaseData->objects.begin(),
                                        m_broadphaseData->objects.end(), object);
            if (broadphaseIt != m_broadphaseData->objects.end()) {
                size_t index = broadphaseIt - m_broadphaseData->objects.begin();
                m_broadphaseData->objects.erase(broadphaseIt);
                m_broadphaseData->aabbs.erase(m_broadphaseData->aabbs.begin() + index);
                m_broadphaseData->indices.erase(m_broadphaseData->indices.begin() + index);
            }
        }

        // Remove from cache
        m_aabbCache.erase(object);
        m_cacheVersion.erase(object);

        m_stats.totalObjects--;
        Logger::Debug("Collision object unregistered: {}", static_cast<void*>(object));

        return true;
    }

    bool CollisionSystem::UpdateObjectPosition(CollisionObject* object, const Vec3& position) {
        if (!object) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_collisionMutex);

        object->SetPosition(position);

        // Update cache
        UpdateObjectCache(object);

        return true;
    }

    bool CollisionSystem::UpdateObjectTransform(CollisionObject* object, const Vec3& position, const Quaternion& rotation) {
        if (!object) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_collisionMutex);

        object->SetPosition(position);
        object->SetRotation(rotation);

        // Update cache
        UpdateObjectCache(object);

        return true;
    }

    int CollisionSystem::DetectCollisions(float deltaTime) {
        if (!m_config.enableCollision) {
            return 0;
        }

        auto broadphaseStart = std::chrono::high_resolution_clock::now();

        // Find potential collision pairs using broadphase
        std::vector<std::pair<CollisionObject*, CollisionObject*>> pairs;
        FindBroadphasePairs(pairs);

        auto broadphaseEnd = std::chrono::high_resolution_clock::now();
        m_stats.broadphaseTime = std::chrono::duration<float>(broadphaseEnd - broadphaseStart).count();
        m_stats.broadphasePairs = static_cast<int>(pairs.size());

        auto narrowphaseStart = std::chrono::high_resolution_clock::now();

        // Perform narrowphase collision detection
        m_manifolds.clear();
        int actualCollisions = 0;

        for (const auto& pair : pairs) {
            CollisionManifold manifold;
            if (CheckCollision(pair.first, pair.second, manifold)) {
                m_manifolds.push_back(manifold);
                actualCollisions++;

                // Debug visualization
                if (m_debugMode) {
                    m_debugManifolds.push_back(manifold);
                }
            }
        }

        auto narrowphaseEnd = std::chrono::high_resolution_clock::now();
        m_stats.narrowphaseTime = std::chrono::duration<float>(narrowphaseEnd - narrowphaseStart).count();
        m_stats.narrowphaseTests = static_cast<int>(pairs.size());
        m_stats.actualCollisions = actualCollisions;

        return actualCollisions;
    }

    bool CollisionSystem::CheckCollision(CollisionObject* objectA, CollisionObject* objectB, CollisionManifold& manifold) {
        if (!objectA || !objectB) {
            return false;
        }

        // Check collision masks
        if (!CheckCollisionMask(objectA, objectB)) {
            return false;
        }

        // Quick AABB test first
        if (!objectA->GetWorldAABB().Intersects(objectB->GetWorldAABB())) {
            return false;
        }

        // Perform shape-specific collision detection
        switch (objectA->GetShape()) {
            case CollisionShape::AABB:
                return TestAABBCollision(objectA, objectB, manifold);
            case CollisionShape::OBB:
                return TestOBBCollision(objectA, objectB, manifold);
            case CollisionShape::SPHERE:
                return TestSphereCollision(objectA, objectB, manifold);
            case CollisionShape::CAPSULE:
                return TestCapsuleCollision(objectA, objectB, manifold);
            default:
                return false;
        }
    }

    bool CollisionSystem::Raycast(const Ray& ray, RaycastHit& hit, uint32_t collisionMask) {
        if (!m_initialized) {
            return false;
        }

        std::shared_lock<std::shared_mutex> lock(m_collisionMutex);

        hit.hit = false;
        hit.distance = ray.maxDistance;
        float closestDistance = ray.maxDistance;

        // Check all objects
        for (CollisionObject* object : m_objects) {
            if (!IsObjectInLayer(object, collisionMask)) {
                continue;
            }

            RaycastHit objectHit;
            if (RaycastObject(ray, object, objectHit)) {
                if (objectHit.distance < closestDistance) {
                    hit = objectHit;
                    closestDistance = objectHit.distance;
                    hit.hit = true;
                }
            }
        }

        m_stats.raycastsPerformed++;
        return hit.hit;
    }

    bool CollisionSystem::RaycastObject(const Ray& ray, CollisionObject* object, RaycastHit& hit) {
        if (!object) {
            return false;
        }

        hit.hit = false;
        hit.entity = object->GetOwner();
        hit.userData = object->GetUserData();

        // Quick AABB test
        float aabbDistance;
        Vec3 aabbNormal;
        if (!RaycastAABB(ray, object->GetWorldAABB(), aabbDistance, aabbNormal)) {
            return false;
        }

        if (aabbDistance > ray.maxDistance) {
            return false;
        }

        // Shape-specific raycast
        switch (object->GetShape()) {
            case CollisionShape::AABB: {
                AABBCollisionObject* aabbObj = static_cast<AABBCollisionObject*>(object);
                hit.hit = RaycastAABB(ray, aabbObj->GetLocalAABB(), hit.distance, hit.normal);
                break;
            }
            case CollisionShape::OBB: {
                OBBCollisionObject* obbObj = static_cast<OBBCollisionObject*>(object);
                OBB worldOBB = obbObj->GetWorldOBB();
                hit.hit = RaycastOBB(ray, worldOBB, hit.distance, hit.normal);
                break;
            }
            case CollisionShape::SPHERE: {
                SphereCollisionObject* sphereObj = static_cast<SphereCollisionObject*>(object);
                Sphere worldSphere = sphereObj->GetWorldSphere();
                hit.hit = RaycastSphere(ray, worldSphere, hit.distance, hit.normal);
                break;
            }
            case CollisionShape::CAPSULE: {
                CapsuleCollisionObject* capsuleObj = static_cast<CapsuleCollisionObject*>(object);
                Capsule worldCapsule = capsuleObj->GetWorldCapsule();
                hit.hit = RaycastCapsule(ray, worldCapsule, hit.distance, hit.normal);
                break;
            }
            default:
                return false;
        }

        if (hit.hit) {
            hit.point = ray.GetPoint(hit.distance);
        }

        return hit.hit;
    }

    bool CollisionSystem::SweepTest(const Vec3& start, const Vec3& end, CollisionObject* shape, SweepTestResult& result, uint32_t collisionMask) {
        if (!shape || !m_initialized) {
            return false;
        }

        std::shared_lock<std::shared_mutex> lock(m_collisionMutex);

        result.hit = false;
        result.hitTime = 1.0f;

        // Create a temporary object for the sweep test
        CollisionObject tempObject = *shape;
        Vec3 direction = end - start;
        float distance = direction.Length();

        if (distance < 0.001f) {
            return false;
        }

        direction = direction / distance;

        // Check against all objects
        for (CollisionObject* object : m_objects) {
            if (object == shape || !IsObjectInLayer(object, collisionMask)) {
                continue;
            }

            // Perform sweep test based on shape types
            bool hit = false;
            float hitTime = 1.0f;
            Vec3 hitNormal;

            // Simple sweep implementation - check discrete points
            const int steps = 10;
            for (int i = 0; i <= steps; ++i) {
                float t = static_cast<float>(i) / steps;
                Vec3 testPos = start + direction * (distance * t);
                tempObject.SetPosition(testPos);

                CollisionManifold manifold;
                if (CheckCollision(&tempObject, object, manifold)) {
                    hit = true;
                    hitTime = t;
                    hitNormal = manifold.normal;
                    break;
                }
            }

            if (hit && hitTime < result.hitTime) {
                result.hit = true;
                result.hitTime = hitTime;
                result.hitPoint = start + direction * (distance * hitTime);
                result.hitNormal = hitNormal;
                result.hitEntity = object->GetOwner();
                result.userData = object->GetUserData();
            }
        }

        m_stats.sweepTestsPerformed++;
        return result.hit;
    }

    int CollisionSystem::QueryAABB(const AABB& aabb, std::vector<CollisionObject*>& objects, uint32_t collisionMask) {
        objects.clear();

        std::shared_lock<std::shared_mutex> lock(m_collisionMutex);

        for (CollisionObject* object : m_objects) {
            if (IsObjectInLayer(object, collisionMask) &&
                object->GetWorldAABB().Intersects(aabb)) {
                objects.push_back(object);
            }
        }

        return static_cast<int>(objects.size());
    }

    int CollisionSystem::QuerySphere(const Vec3& center, float radius, std::vector<CollisionObject*>& objects, uint32_t collisionMask) {
        objects.clear();
        Sphere querySphere = {center, radius};

        std::shared_lock<std::shared_mutex> lock(m_collisionMutex);

        for (CollisionObject* object : m_objects) {
            if (IsObjectInLayer(object, collisionMask)) {
                // Simple sphere-AABB intersection test
                AABB objectAABB = object->GetWorldAABB();
                Sphere objectSphere = Sphere::FromAABB(objectAABB);

                if (querySphere.Intersects(objectSphere)) {
                    objects.push_back(object);
                }
            }
        }

        return static_cast<int>(objects.size());
    }

    int CollisionSystem::QueryRay(const Ray& ray, std::vector<CollisionObject*>& objects, uint32_t collisionMask) {
        objects.clear();

        std::shared_lock<std::shared_mutex> lock(m_collisionMutex);

        for (CollisionObject* object : m_objects) {
            if (IsObjectInLayer(object, collisionMask)) {
                RaycastHit hit;
                if (RaycastObject(ray, object, hit)) {
                    objects.push_back(object);
                }
            }
        }

        return static_cast<int>(objects.size());
    }

    CollisionObject* CollisionSystem::FindNearestObject(const Vec3& position, uint32_t collisionMask) {
        std::shared_lock<std::shared_mutex> lock(m_collisionMutex);

        CollisionObject* nearest = nullptr;
        float minDistance = std::numeric_limits<float>::max();

        for (CollisionObject* object : m_objects) {
            if (IsObjectInLayer(object, collisionMask)) {
                float distance = (position - object->GetPosition()).LengthSquared();
                if (distance < minDistance) {
                    minDistance = distance;
                    nearest = object;
                }
            }
        }

        return nearest;
    }

    bool CollisionSystem::ResolveCollision(const CollisionManifold& manifold, float deltaTime) {
        if (!manifold.entityA || !manifold.entityB) {
            return false;
        }

        // Get collision objects
        CollisionObject* objectA = static_cast<CollisionObject*>(manifold.entityA->GetCollisionObject());
        CollisionObject* objectB = static_cast<CollisionObject*>(manifold.entityB->GetCollisionObject());

        if (!objectA || !objectB) {
            return false;
        }

        // Handle different collision responses
        switch (manifold.entityA->GetCollisionResponse()) {
            case CollisionResponse::BLOCK:
                ResolveAABBManifold(manifold);
                break;
            case CollisionResponse::SLIDE:
                // Implement sliding response
                break;
            case CollisionResponse::BOUNCE:
                // Implement bouncing response
                break;
            case CollisionResponse::IGNORE:
                return false;
            case CollisionResponse::TRIGGER:
                // Call trigger callback
                break;
            default:
                break;
        }

        return true;
    }

    int CollisionSystem::ResolveCollisions(const std::vector<CollisionManifold>& manifolds, float deltaTime) {
        int resolved = 0;

        for (const auto& manifold : manifolds) {
            if (ResolveCollision(manifold, deltaTime)) {
                resolved++;
            }
        }

        return resolved;
    }

    void CollisionSystem::IntegratePhysics(float deltaTime) {
        if (!m_config.enableCollision) {
            return;
        }

        std::unique_lock<std::shared_mutex> lock(m_collisionMutex);

        // Apply forces to dynamic objects
        for (CollisionObject* object : m_dynamicObjects) {
            if (object->GetOwner()) {
                // Apply gravity
                ApplyGravity(object, deltaTime);

                // Apply damping
                ApplyDamping(object, deltaTime);
            }
        }
    }

    void CollisionSystem::ApplyGravity(CollisionObject* object, float deltaTime) {
        if (!object->GetOwner()) {
            return;
        }

        // Simple gravity implementation
        Vec3 velocity = object->GetOwner()->GetVelocity();
        velocity.y -= m_config.gravity * deltaTime;
        object->GetOwner()->SetVelocity(velocity);
    }

    void CollisionSystem::ApplyDamping(CollisionObject* object, float deltaTime) {
        if (!object->GetOwner()) {
            return;
        }

        // Apply linear and angular damping
        Vec3 velocity = object->GetOwner()->GetVelocity();
        velocity *= (1.0f - m_config.linearDamping * deltaTime);
        object->GetOwner()->SetVelocity(velocity);

        Vec3 angularVelocity = object->GetOwner()->GetAngularVelocity();
        angularVelocity *= (1.0f - m_config.angularDamping * deltaTime);
        object->GetOwner()->SetAngularVelocity(angularVelocity);
    }

    // Broadphase implementations
    void CollisionSystem::UpdateBroadphase() {
        if (!m_broadphaseData) {
            return;
        }

        switch (m_config.broadphaseType) {
            case BroadphaseType::BRUTE_FORCE:
                // No update needed
                break;
            case BroadphaseType::SPATIAL_HASH:
                UpdateSpatialHash();
                break;
            case BroadphaseType::SWEEP_AND_PRUNE:
                PerformSweepAndPrune();
                break;
            case BroadphaseType::DYNAMIC_BVH:
                BuildDynamicBVH();
                break;
            default:
                break;
        }
    }

    void CollisionSystem::FindBroadphasePairs(std::vector<std::pair<CollisionObject*, CollisionObject*>>& pairs) {
        if (!m_broadphaseData) {
            return;
        }

        pairs.clear();

        switch (m_config.broadphaseType) {
            case BroadphaseType::BRUTE_FORCE:
                // O(n²) brute force check
                for (size_t i = 0; i < m_broadphaseData->objects.size(); ++i) {
                    for (size_t j = i + 1; j < m_broadphaseData->objects.size(); ++j) {
                        CollisionObject* a = m_broadphaseData->objects[i];
                        CollisionObject* b = m_broadphaseData->objects[j];

                        if (CheckCollisionMask(a, b) &&
                            a->GetWorldAABB().Intersects(b->GetWorldAABB())) {
                            pairs.emplace_back(a, b);
                        }
                    }
                }
                break;

            case BroadphaseType::SPATIAL_HASH:
                // Use spatial hash to find potential pairs
                FindSpatialHashPairs(pairs);
                break;

            case BroadphaseType::SWEEP_AND_PRUNE:
                // Use sweep and prune
                FindSweepAndPrunePairs(pairs);
                break;

            case BroadphaseType::DYNAMIC_BVH:
                // Use dynamic BVH
                FindBVHPairs(pairs);
                break;

            default:
                break;
        }
    }

    void CollisionSystem::UpdateSpatialHash() {
        if (!m_broadphaseData) {
            return;
        }

        m_broadphaseData->spatialHash.clear();

        for (CollisionObject* object : m_broadphaseData->objects) {
            AABB aabb = object->GetWorldAABB();
            Vec3 minCell = (aabb.min / m_broadphaseData->cellSize).Floor();
            Vec3 maxCell = (aabb.max / m_broadphaseData->cellSize).Floor();

            for (int x = static_cast<int>(minCell.x); x <= static_cast<int>(maxCell.x); ++x) {
                for (int y = static_cast<int>(minCell.y); y <= static_cast<int>(maxCell.y); ++y) {
                    for (int z = static_cast<int>(minCell.z); z <= static_cast<int>(maxCell.z); ++z) {
                        uint64_t key = GetSpatialHashKey(x, y, z);
                        m_broadphaseData->spatialHash[key].push_back(object);
                    }
                }
            }
        }
    }

    void CollisionSystem::PerformSweepAndPrune() {
        if (!m_broadphaseData) {
            return;
        }

        // Sort objects by x, y, z coordinates
        m_broadphaseData->sortedX = m_broadphaseData->objects;
        m_broadphaseData->sortedY = m_broadphaseData->objects;
        m_broadphaseData->sortedZ = m_broadphaseData->objects;

        std::sort(m_broadphaseData->sortedX.begin(), m_broadphaseData->sortedX.end(),
                 [](CollisionObject* a, CollisionObject* b) {
                     return a->GetWorldAABB().min.x < b->GetWorldAABB().min.x;
                 });

        std::sort(m_broadphaseData->sortedY.begin(), m_broadphaseData->sortedY.end(),
                 [](CollisionObject* a, CollisionObject* b) {
                     return a->GetWorldAABB().min.y < b->GetWorldAABB().min.y;
                 });

        std::sort(m_broadphaseData->sortedZ.begin(), m_broadphaseData->sortedZ.end(),
                 [](CollisionObject* a, CollisionObject* b) {
                     return a->GetWorldAABB().min.z < b->GetWorldAABB().min.z;
                 });
    }

    void CollisionSystem::BuildDynamicBVH() {
        if (!m_broadphaseData) {
            return;
        }

        // Simple BVH construction
        m_broadphaseData->bvhNodes.clear();
        if (m_broadphaseData->objects.empty()) {
            return;
        }

        // Create leaf nodes
        for (size_t i = 0; i < m_broadphaseData->objects.size(); ++i) {
            BroadphaseData::BVHNode node;
            node.bounds = m_broadphaseData->objects[i]->GetWorldAABB();
            node.objectIndex = static_cast<int>(i);
            node.isLeaf = true;
            m_broadphaseData->bvhNodes.push_back(node);
        }

        // Build hierarchy (simplified)
        if (m_broadphaseData->bvhNodes.size() > 1) {
            // Create root node
            BroadphaseData::BVHNode root;
            root.left = 0;
            root.right = 1;
            root.isLeaf = false;

            // Calculate bounds
            root.bounds = m_broadphaseData->bvhNodes[0].bounds;
            root.bounds.Expand(m_broadphaseData->bvhNodes[1].bounds);

            m_broadphaseData->bvhNodes.push_back(root);
            m_broadphaseData->rootNode = static_cast<int>(m_broadphaseData->bvhNodes.size() - 1);
        } else if (!m_broadphaseData->bvhNodes.empty()) {
            m_broadphaseData->rootNode = 0;
        }
    }

    // Broadphase pair finding methods
    void CollisionSystem::FindSpatialHashPairs(std::vector<std::pair<CollisionObject*, CollisionObject*>>& pairs) {
        std::unordered_set<uint64_t> processed;

        for (const auto& bucket : m_broadphaseData->spatialHash) {
            const auto& objects = bucket.second;
            if (objects.size() < 2) {
                continue;
            }

            // Check all pairs in this bucket
            for (size_t i = 0; i < objects.size(); ++i) {
                for (size_t j = i + 1; j < objects.size(); ++j) {
                    CollisionObject* a = objects[i];
                    CollisionObject* b = objects[j];

                    if (CheckCollisionMask(a, b) &&
                        a->GetWorldAABB().Intersects(b->GetWorldAABB())) {
                        pairs.emplace_back(a, b);
                    }
                }
            }
        }
    }

    void CollisionSystem::FindSweepAndPrunePairs(std::vector<std::pair<CollisionObject*, CollisionObject*>>& pairs) {
        // Simple sweep and prune implementation
        for (size_t i = 0; i < m_broadphaseData->sortedX.size(); ++i) {
            for (size_t j = i + 1; j < m_broadphaseData->sortedX.size(); ++j) {
                CollisionObject* a = m_broadphaseData->sortedX[i];
                CollisionObject* b = m_broadphaseData->sortedX[j];

                if (CheckCollisionMask(a, b) &&
                    a->GetWorldAABB().Intersects(b->GetWorldAABB())) {
                    pairs.emplace_back(a, b);
                }
            }
        }
    }

    void CollisionSystem::FindBVHPairs(std::vector<std::pair<CollisionObject*, CollisionObject*>>& pairs) {
        if (m_broadphaseData->rootNode == -1) {
            return;
        }

        // Simple BVH traversal (simplified implementation)
        std::vector<int> nodesToCheck;
        nodesToCheck.push_back(m_broadphaseData->rootNode);

        while (!nodesToCheck.empty()) {
            int nodeIndex = nodesToCheck.back();
            nodesToCheck.pop_back();

            if (nodeIndex < 0 || nodeIndex >= static_cast<int>(m_broadphaseData->bvhNodes.size())) {
                continue;
            }

            const BroadphaseData::BVHNode& node = m_broadphaseData->bvhNodes[nodeIndex];

            if (node.isLeaf) {
                // Check leaf against all other leaves
                for (size_t i = 0; i < m_broadphaseData->objects.size(); ++i) {
                    if (static_cast<int>(i) != node.objectIndex) {
                        CollisionObject* a = m_broadphaseData->objects[node.objectIndex];
                        CollisionObject* b = m_broadphaseData->objects[i];

                        if (CheckCollisionMask(a, b) &&
                            a->GetWorldAABB().Intersects(b->GetWorldAABB())) {
                            pairs.emplace_back(a, b);
                        }
                    }
                }
            } else {
                // Add children to check
                if (node.left != -1) {
                    nodesToCheck.push_back(node.left);
                }
                if (node.right != -1) {
                    nodesToCheck.push_back(node.right);
                }
            }
        }
    }

    // Raycast implementations
    bool CollisionSystem::RaycastAABB(const Ray& ray, const AABB& aabb, float& distance, Vec3& normal) {
        // Slab method for AABB raycast
        float tmin = 0.0f;
        float tmax = ray.maxDistance;
        int normalIndex = -1;

        // Check each axis
        for (int i = 0; i < 3; ++i) {
            float origin = (i == 0) ? ray.origin.x : (i == 1) ? ray.origin.y : ray.origin.z;
            float direction = (i == 0) ? ray.direction.x : (i == 1) ? ray.direction.y : ray.direction.z;
            float min = (i == 0) ? aabb.min.x : (i == 1) ? aabb.min.y : aabb.min.z;
            float max = (i == 0) ? aabb.max.x : (i == 1) ? aabb.max.y : aabb.max.z;

            if (std::abs(direction) < 0.001f) {
                // Parallel to axis
                if (origin < min || origin > max) {
                    return false;
                }
            } else {
                float t1 = (min - origin) / direction;
                float t2 = (max - origin) / direction;

                if (t1 > t2) {
                    std::swap(t1, t2);
                }

                tmin = std::max(tmin, t1);
                tmax = std::min(tmax, t2);

                if (tmin > tmax) {
                    return false;
                }

                // Update normal
                if (t1 > tmin + 0.001f) {
                    normalIndex = i;
                }
            }
        }

        distance = tmin;

        // Calculate normal
        normal = Vec3(0.0f);
        if (normalIndex >= 0) {
            if (normalIndex == 0) {
                normal.x = (ray.direction.x > 0) ? -1.0f : 1.0f;
            } else if (normalIndex == 1) {
                normal.y = (ray.direction.y > 0) ? -1.0f : 1.0f;
            } else {
                normal.z = (ray.direction.z > 0) ? -1.0f : 1.0f;
            }
        }

        return distance <= ray.maxDistance;
    }

    bool CollisionSystem::RaycastSphere(const Ray& ray, const Sphere& sphere, float& distance, Vec3& normal) {
        Vec3 toCenter = sphere.center - ray.origin;
        float toCenterLengthSquared = toCenter.LengthSquared();
        float sphereRadiusSquared = sphere.radius * sphere.radius;

        // Check if ray starts inside sphere
        if (toCenterLengthSquared < sphereRadiusSquared) {
            distance = 0.0f;
            normal = -ray.direction;
            return true;
        }

        float projection = toCenter.Dot(ray.direction);

        if (projection < 0) {
            return false;
        }

        float discriminant = sphereRadiusSquared - toCenterLengthSquared + projection * projection;

        if (discriminant < 0) {
            return false;
        }

        distance = projection - std::sqrt(discriminant);

        if (distance > ray.maxDistance) {
            return false;
        }

        Vec3 hitPoint = ray.GetPoint(distance);
        normal = (hitPoint - sphere.center).Normalized();

        return true;
    }

    bool CollisionSystem::RaycastOBB(const Ray& ray, const OBB& obb, float& distance, Vec3& normal) {
        // Transform ray to OBB local space
        Vec3 localOrigin = obb.center - ray.origin;
        Vec3 localDirection = ray.direction;

        // Get OBB axes
        Vec3 axes[3];
        obb.GetAxes(axes[0], axes[1], axes[2]);

        // Transform ray direction to local space
        localDirection = Vec3(
            localDirection.Dot(axes[0]),
            localDirection.Dot(axes[1]),
            localDirection.Dot(axes[2])
        );

        // Create local space AABB
        AABB localAABB = AABB::FromMinMax(-obb.extents, obb.extents);

        // Create local ray
        Ray localRay(localOrigin, localDirection, ray.maxDistance);

        // Raycast against local AABB
        float localDistance;
        Vec3 localNormal;
        if (!RaycastAABB(localRay, localAABB, localDistance, localNormal)) {
            return false;
        }

        // Transform normal back to world space
        normal = axes[0] * localNormal.x + axes[1] * localNormal.y + axes[2] * localNormal.z;
        normal = normal.Normalized();

        distance = localDistance;
        return true;
    }

    bool CollisionSystem::RaycastCapsule(const Ray& ray, const Capsule& capsule, float& distance, Vec3& normal) {
        // Simple capsule raycast - check against cylinder and spheres
        Vec3 ab = capsule.end - capsule.start;
        Vec3 ao = ray.origin - capsule.start;

        float abLengthSquared = ab.LengthSquared();
        float abDotAo = ab.Dot(ao);
        float abDotDirection = ab.Dot(ray.direction);

        float m = abDotAo / abLengthSquared;
        float n = abDotDirection / abLengthSquared;

        Vec3 q = ray.direction - ab * n;
        float qLengthSquared = q.LengthSquared();

        if (qLengthSquared == 0) {
            // Parallel to capsule axis
            Vec3 point = capsule.start + ab * m;
            float distanceToPoint = (point - ray.origin).Length();
            if (distanceToPoint <= capsule.radius) {
                distance = distanceToPoint;
                normal = -ray.direction;
                return true;
            }
            return false;
        }

        float s = ao.Dot(q) / qLengthSquared;
        float t = m + n * s;

        // Clamp to capsule segment
        t = std::clamp(t, 0.0f, 1.0f);

        Vec3 point = capsule.start + ab * t;
        Vec3 toPoint = point - ray.origin;
        float distanceToPoint = toPoint.Length();

        if (distanceToPoint > capsule.radius) {
            return false;
        }

        float distanceAlongRay = toPoint.Dot(ray.direction);

        if (distanceAlongRay < 0 || distanceAlongRay > ray.maxDistance) {
            return false;
        }

        distance = distanceAlongRay;
        Vec3 hitPoint = ray.GetPoint(distance);
        normal = (hitPoint - point).Normalized();

        return true;
    }

    // Utility methods
    uint64_t CollisionSystem::GetSpatialHashKey(int x, int y, int z) const {
        // Simple hash function for spatial hashing
        uint64_t key = 0;
        key = static_cast<uint64_t>(x) << 42;
        key |= static_cast<uint64_t>(y) << 21;
        key |= static_cast<uint64_t>(z);
        return key;
    }

    bool CollisionSystem::CheckCollisionMask(CollisionObject* a, CollisionObject* b) const {
        return (a->GetCollisionMask() & b->GetCollisionMask()) != 0;
    }

    bool CollisionSystem::IsObjectInLayer(CollisionObject* object, uint32_t layer) const {
        return (object->GetCollisionMask() & layer) != 0;
    }

    void CollisionSystem::UpdateObjectCache(CollisionObject* object) {
        if (!object) {
            return;
        }

        m_aabbCache[object] = object->GetWorldAABB();
        m_cacheVersion[object] = m_cacheVersion[object] + 1;
    }

    bool CollisionSystem::IsCacheValid(CollisionObject* object) const {
        if (!object) {
            return false;
        }

        auto it = m_cacheVersion.find(object);
        return it != m_cacheVersion.end() && it->second == m_cacheVersion.at(object);
    }

    void CollisionSystem::ResetStats() {
        m_stats = CollisionStats();
        m_stats.totalObjects = static_cast<int>(m_objects.size());
    }

    std::string CollisionSystem::GetDebugInfo() const {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(3);

        ss << "CollisionSystem Debug Info:\n";
        ss << "Total objects: " << m_stats.totalObjects << "\n";
        ss << "Broadphase pairs: " << m_stats.broadphasePairs << "\n";
        ss << "Narrowphase tests: " << m_stats.narrowphaseTests << "\n";
        ss << "Actual collisions: " << m_stats.actualCollisions << "\n";
        ss << "Raycasts performed: " << m_stats.raycastsPerformed << "\n";
        ss << "Sweep tests performed: " << m_stats.sweepTestsPerformed << "\n";
        ss << "Broadphase time: " << m_stats.broadphaseTime << "s\n";
        ss << "Narrowphase time: " << m_stats.narrowphaseTime << "s\n";
        ss << "Total collision time: " << m_stats.collisionTime << "s\n";
        ss << "Cache hits: " << m_stats.cacheHits << "\n";
        ss << "Cache misses: " << m_stats.cacheMisses << "\n";

        return ss.str();
    }

    bool CollisionSystem::Validate() const {
        // Basic validation checks
        bool valid = true;

        if (m_objects.size() != m_staticObjects.size() + m_dynamicObjects.size()) {
            Logger::Error("CollisionSystem validation failed: object count mismatch");
            valid = false;
        }

        if (m_broadphaseData && m_broadphaseData->objects.size() != m_objects.size()) {
            Logger::Error("CollisionSystem validation failed: broadphase data mismatch");
            valid = false;
        }

        return valid;
    }

    // Stub implementations for missing methods
    bool CollisionSystem::TestAABBCollision(CollisionObject* a, CollisionObject* b, CollisionManifold& manifold) {
        // AABB-AABB collision detection
        AABBCollisionObject* aabbA = static_cast<AABBCollisionObject*>(a);
        AABB aabb1 = aabbA->GetWorldAABB();

        AABB aabb2 = b->GetWorldAABB();

        if (!aabb1.Intersects(aabb2)) {
            return false;
        }

        // Calculate manifold
        manifold.entityA = a->GetOwner();
        manifold.entityB = b->GetOwner();
        manifold.penetration = CalculateAABBPenetration(aabb1, aabb2);
        manifold.point = CalculateAABBContactPoint(aabb1, aabb2);
        manifold.normal = CalculateAABBCollisionNormal(aabb1, aabb2);
        manifold.restitution = CalculateRestitution(a, b);
        manifold.friction = CalculateFriction(a, b);

        return true;
    }

    bool CollisionSystem::TestOBBCollision(CollisionObject* a, CollisionObject* b, CollisionManifold& manifold) {
        // OBB-OBB collision detection (simplified)
        OBBCollisionObject* obbA = static_cast<OBBCollisionObject*>(a);
        OBB worldOBB1 = obbA->GetWorldOBB();

        OBB worldOBB2 = (b->GetShape() == CollisionShape::OBB) ?
            static_cast<OBBCollisionObject*>(b)->GetWorldOBB() :
            OBB::FromAABB(b->GetWorldAABB());

        // Simple AABB intersection test for now
        AABB aabb1 = a->GetWorldAABB();
        AABB aabb2 = b->GetWorldAABB();

        if (!aabb1.Intersects(aabb2)) {
            return false;
        }

        // Calculate manifold (simplified)
        manifold.entityA = a->GetOwner();
        manifold.entityB = b->GetOwner();
        manifold.penetration = CalculateAABBPenetration(aabb1, aabb2);
        manifold.point = CalculateAABBContactPoint(aabb1, aabb2);
        manifold.normal = CalculateAABBCollisionNormal(aabb1, aabb2);
        manifold.restitution = CalculateRestitution(a, b);
        manifold.friction = CalculateFriction(a, b);

        return true;
    }

    bool CollisionSystem::TestSphereCollision(CollisionObject* a, CollisionObject* b, CollisionManifold& manifold) {
        // Sphere-sphere or sphere-other collision detection
        SphereCollisionObject* sphereA = static_cast<SphereCollisionObject*>(a);
        Sphere worldSphere1 = sphereA->GetWorldSphere();

        Sphere worldSphere2 = (b->GetShape() == CollisionShape::SPHERE) ?
            static_cast<SphereCollisionObject*>(b)->GetWorldSphere() :
            Sphere::FromAABB(b->GetWorldAABB());

        if (!worldSphere1.Intersects(worldSphere2)) {
            return false;
        }

        // Calculate manifold
        manifold.entityA = a->GetOwner();
        manifold.entityB = b->GetOwner();
        manifold.penetration = worldSphere1.radius + worldSphere2.radius -
                              (worldSphere1.center - worldSphere2.center).Length();
        manifold.point = worldSphere1.center + (worldSphere2.center - worldSphere1.center).Normalized() *
                        (worldSphere1.radius - manifold.penetration * 0.5f);
        manifold.normal = (worldSphere2.center - worldSphere1.center).Normalized();
        manifold.restitution = CalculateRestitution(a, b);
        manifold.friction = CalculateFriction(a, b);

        return true;
    }

    bool CollisionSystem::TestCapsuleCollision(CollisionObject* a, CollisionObject* b, CollisionManifold& manifold) {
        // Capsule-capsule or capsule-other collision detection (simplified)
        CapsuleCollisionObject* capsuleA = static_cast<CapsuleCollisionObject*>(a);
        Capsule worldCapsule1 = capsuleA->GetWorldCapsule();

        // Convert other shapes to spheres for simplicity
        Sphere worldSphere2 = (b->GetShape() == CollisionShape::SPHERE) ?
            static_cast<SphereCollisionObject*>(b)->GetWorldSphere() :
            Sphere::FromAABB(b->GetWorldAABB());

        // Simple distance check
        float distance = (worldCapsule1.start - worldSphere2.center).Length();
        float minDistance = worldCapsule1.radius + worldSphere2.radius;

        if (distance > minDistance) {
            return false;
        }

        // Calculate manifold
        manifold.entityA = a->GetOwner();
        manifold.entityB = b->GetOwner();
        manifold.penetration = minDistance - distance;
        manifold.point = worldSphere2.center +
                        (worldCapsule1.start - worldSphere2.center).Normalized() * worldSphere2.radius;
        manifold.normal = (worldSphere2.center - worldCapsule1.start).Normalized();
        manifold.restitution = CalculateRestitution(a, b);
        manifold.friction = CalculateFriction(a, b);

        return true;
    }

    void CollisionSystem::ResolveAABBManifold(const CollisionManifold& manifold) {
        // Simple AABB collision resolution
        if (!manifold.entityA || !manifold.entityB) {
            return;
        }

        // Get positions
        Vec3 posA = manifold.entityA->GetPosition();
        Vec3 posB = manifold.entityB->GetPosition();

        // Separate objects along normal
        float separation = manifold.penetration * 0.5f;
        posA -= manifold.normal * separation;
        posB += manifold.normal * separation;

        // Update positions
        manifold.entityA->SetPosition(posA);
        manifold.entityB->SetPosition(posB);
    }

    float CollisionSystem::CalculateAABBPenetration(const AABB& a, const AABB& b) const {
        Vec3 centerA = a.GetCenter();
        Vec3 centerB = b.GetCenter();
        Vec3 extentsA = a.GetExtents();
        Vec3 extentsB = b.GetExtents();

        Vec3 diff = centerB - centerA;
        Vec3 minOverlap = extentsA + extentsB - Vec3(std::abs(diff.x), std::abs(diff.y), std::abs(diff.z));

        return std::min(std::min(minOverlap.x, minOverlap.y), minOverlap.z);
    }

    Vec3 CollisionSystem::CalculateAABBContactPoint(const AABB& a, const AABB& b) const {
        Vec3 centerA = a.GetCenter();
        Vec3 centerB = b.GetCenter();

        Vec3 halfSizeA = a.GetExtents();
        Vec3 halfSizeB = b.GetExtents();

        Vec3 diff = centerB - centerA;

        // Find the closest point on AABB A to center of AABB B
        Vec3 contactPoint = centerA;
        contactPoint.x = std::clamp(centerB.x, centerA.x - halfSizeA.x, centerA.x + halfSizeA.x);
        contactPoint.y = std::clamp(centerB.y, centerA.y - halfSizeA.y, centerA.y + halfSizeA.y);
        contactPoint.z = std::clamp(centerB.z, centerA.z - halfSizeA.z, centerA.z + halfSizeA.z);

        return contactPoint;
    }

    Vec3 CollisionSystem::CalculateAABBCollisionNormal(const AABB& a, const AABB& b) const {
        Vec3 centerA = a.GetCenter();
        Vec3 centerB = b.GetCenter();

        Vec3 diff = centerB - centerA;
        Vec3 absDiff = Vec3(std::abs(diff.x), std::abs(diff.y), std::abs(diff.z));

        // Find the axis with the smallest penetration
        if (absDiff.x < absDiff.y && absDiff.x < absDiff.z) {
            return Vec3(diff.x > 0 ? 1.0f : -1.0f, 0.0f, 0.0f);
        } else if (absDiff.y < absDiff.z) {
            return Vec3(0.0f, diff.y > 0 ? 1.0f : -1.0f, 0.0f);
        } else {
            return Vec3(0.0f, 0.0f, diff.z > 0 ? 1.0f : -1.0f);
        }
    }

    float CollisionSystem::CalculateRestitution(CollisionObject* a, CollisionObject* b) const {
        return std::max(m_config.restitution, 0.0f);
    }

    float CollisionSystem::CalculateFriction(CollisionObject* a, CollisionObject* b) const {
        return std::max(m_config.friction, 0.0f);
    }

} // namespace VoxelCraft
