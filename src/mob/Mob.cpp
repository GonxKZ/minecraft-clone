/**
 * @file Mob.cpp
 * @brief VoxelCraft Mob System Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "Mob.hpp"
#include "../world/World.hpp"
#include "../player/Player.hpp"
#include <algorithm>
#include <random>
#include <cmath>

namespace VoxelCraft {

// Mob base implementation
Mob::Mob(MobType type, const glm::vec3& position, World* world)
    : Entity(position, world)
    , m_type(type)
    , m_state(MobState::IDLE)
    , m_health(20.0f)
    , m_target(nullptr)
    , m_velocity(0.0f)
    , m_gravity(0.08f)
    , m_onGround(false)
    , m_inWater(false)
    , m_onFire(false)
    , m_isFlying(false)
    , m_isClimbing(false)
    , m_wanderTimer(0.0f)
    , m_attackTimer(0.0f)
    , m_healTimer(0.0f)
    , m_teleportTimer(0.0f)
    , m_animationTimer(0.0f)
    , m_glowIntensity(0.0f)
    , m_sizeMultiplier(1.0f)
{
    InitializeAttributes();
    InitializeBehaviors();
    InitializeSounds();

    m_health = m_attributes.health;
    m_homePosition = position;
    m_homeRadius = 16.0f;

    // Initialize pathfinding
    m_pathfinding = std::make_unique<Pathfinding>(world);
}

void Mob::Update(float deltaTime) {
    if (!IsAlive()) return;

    UpdateAI(deltaTime);
    UpdateMovement(deltaTime);
    UpdateAnimations(deltaTime);
    UpdateEffects(deltaTime);

    // Update timers
    m_wanderTimer -= deltaTime;
    m_attackTimer -= deltaTime;
    m_healTimer -= deltaTime;
    m_teleportTimer -= deltaTime;

    // Update memory timestamp
    m_memory.lastUpdate = std::chrono::steady_clock::now();
}

void Mob::FixedUpdate(float deltaTime) {
    // Apply gravity
    if (!m_onGround && !m_isFlying) {
        m_velocity.y -= m_gravity;
    }

    // Apply velocity
    glm::vec3 newPosition = GetPosition() + m_velocity;

    // Check collisions and adjust position
    // This would typically use the world's collision system
    SetPosition(newPosition);

    // Reset velocity for next frame
    m_velocity *= 0.91f; // Air friction
    if (m_onGround) {
        m_velocity.x *= 0.8f; // Ground friction
        m_velocity.z *= 0.8f;
    }
}

void Mob::TakeDamage(float damage, Entity* source) {
    if (!IsAlive()) return;

    // Apply damage reduction from armor
    float actualDamage = damage * (1.0f - m_attributes.armor * 0.04f);
    actualDamage = std::max(0.0f, actualDamage);

    m_health -= actualDamage;

    if (source && HasBehavior(MobBehavior::HOSTILE)) {
        SetTarget(source);
        SetState(MobState::ATTACKING);
    }

    OnDamage(source);

    if (!IsAlive()) {
        OnDeath();
        Kill();
    }
}

void Mob::Kill() {
    SetState(MobState::DYING);

    // Drop items
    if (m_attributes.dropsLoot) {
        for (size_t i = 0; i < m_attributes.dropItems.size(); ++i) {
            if (i < m_attributes.dropChances.size()) {
                float chance = m_attributes.dropChances[i];
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_real_distribution<float> dis(0.0f, 1.0f);

                if (dis(gen) <= chance) {
                    // Drop item at position
                    // This would typically create an item entity in the world
                }
            }
        }
    }

    // Award experience
    if (m_attributes.experienceValue > 0) {
        // This would typically create experience orbs
    }
}

bool Mob::HasBehavior(MobBehavior behavior) const {
    return std::find(m_behaviors.begin(), m_behaviors.end(), behavior) != m_behaviors.end();
}

void Mob::AddBehavior(MobBehavior behavior) {
    if (!HasBehavior(behavior)) {
        m_behaviors.push_back(behavior);
    }
}

void Mob::RemoveBehavior(MobBehavior behavior) {
    auto it = std::find(m_behaviors.begin(), m_behaviors.end(), behavior);
    if (it != m_behaviors.end()) {
        m_behaviors.erase(it);
    }
}

void Mob::SetPosition(const glm::vec3& position) {
    Entity::SetPosition(position);

    // Check if in water
    m_inWater = false; // This would check world water blocks

    // Check if on ground
    m_onGround = false; // This would check world collision

    // Check if on fire
    m_onFire = false; // This would check world fire blocks
}

void Mob::Move(const glm::vec3& direction, float speed) {
    if (!IsAlive()) return;

    glm::vec3 normalizedDirection = glm::normalize(direction);
    glm::vec3 movement = normalizedDirection * speed * m_attributes.speed;

    m_velocity += movement;

    // Limit velocity
    float maxSpeed = m_attributes.speed * 2.0f;
    if (glm::length(m_velocity) > maxSpeed) {
        m_velocity = glm::normalize(m_velocity) * maxSpeed;
    }
}

void Mob::Jump() {
    if (!IsAlive() || !m_onGround) return;

    m_velocity.y = m_attributes.jumpHeight;
    m_onGround = false;
}

bool Mob::Attack(Entity* target) {
    if (!target || !IsAlive() || m_attackTimer > 0.0f) return false;

    if (DistanceToTarget(target) <= m_attributes.attackRange) {
        float damage = m_attributes.attackDamage;
        target->TakeDamage(damage, this);
        m_attackTimer = 1.0f / m_attributes.attackSpeed;

        OnAttack(target);
        return true;
    }

    return false;
}

bool Mob::RangedAttack(Entity* target) {
    if (!target || !IsAlive() || !HasBehavior(MobBehavior::RANGED)) return false;

    // This would typically create projectile entities
    // For now, just deal damage directly
    float damage = m_attributes.attackDamage * 0.8f;
    target->TakeDamage(damage, this);

    return true;
}

bool Mob::Teleport(const glm::vec3& position) {
    if (!HasBehavior(MobBehavior::TELEPORTING)) return false;

    // Check if position is valid
    if (CanSpawnAt(position)) {
        SetPosition(position);
        m_teleportTimer = 5.0f; // Cooldown
        return true;
    }

    return false;
}

void Mob::PlaySound(const std::string& sound) {
    // This would typically play sound effects
    // For now, just log or handle the sound
}

bool Mob::Spawn(const glm::vec3& position) {
    if (!CanSpawnAt(position)) return false;

    SetPosition(position);
    SetState(MobState::IDLE);
    SetHealth(m_attributes.health);

    OnSpawn();
    return true;
}

void Mob::Despawn() {
    // This would typically remove the mob from the world
    // and clean up resources
}

bool Mob::CanSpawnAt(const glm::vec3& position) const {
    if (!m_world) return false;

    // Check light level
    int lightLevel = 15; // This would get actual light level from world
    if (m_attributes.spawnsInDarkness) {
        if (lightLevel > m_attributes.spawnLightLevel) return false;
    } else {
        if (lightLevel < m_attributes.spawnLightLevel) return false;
    }

    // Check if position is solid
    // This would check world blocks

    return true;
}

std::string Mob::GetName() const {
    switch (m_type) {
        case MobType::CREEPER: return "creeper";
        case MobType::ZOMBIE: return "zombie";
        case MobType::SKELETON: return "skeleton";
        case MobType::SPIDER: return "spider";
        case MobType::ENDERMAN: return "enderman";
        case MobType::WOLF: return "wolf";
        case MobType::CHICKEN: return "chicken";
        case MobType::COW: return "cow";
        case MobType::PIG: return "pig";
        case MobType::SHEEP: return "sheep";
        case MobType::VILLAGER: return "villager";
        default: return "unknown_mob";
    }
}

std::string Mob::GetDisplayName() const {
    switch (m_type) {
        case MobType::CREEPER: return "Creeper";
        case MobType::ZOMBIE: return "Zombie";
        case MobType::SKELETON: return "Skeleton";
        case MobType::SPIDER: return "Spider";
        case MobType::ENDERMAN: return "Enderman";
        case MobType::WOLF: return "Wolf";
        case MobType::CHICKEN: return "Chicken";
        case MobType::COW: return "Cow";
        case MobType::PIG: return "Pig";
        case MobType::SHEEP: return "Sheep";
        case MobType::VILLAGER: return "Villager";
        default: return "Unknown Mob";
    }
}

std::string Mob::GetDescription() const {
    switch (m_type) {
        case MobType::CREEPER: return "A green, stealthy creature that explodes when close to players";
        case MobType::ZOMBIE: return "An undead mob that attacks by melee";
        case MobType::SKELETON: return "An undead archer that attacks from range";
        case MobType::SPIDER: return "An arachnid that can climb walls and is hostile at night";
        case MobType::ENDERMAN: return "A tall, teleporting mob that hates being looked at";
        case MobType::WOLF: return "A canine that can be tamed and will fight for its owner";
        case MobType::CHICKEN: return "A passive bird that lays eggs";
        case MobType::COW: return "A passive bovine that can be milked";
        case MobType::PIG: return "A passive porcine that can be ridden";
        case MobType::SHEEP: return "A passive ovine that grows wool";
        case MobType::VILLAGER: return "A passive NPC that trades with players";
        default: return "An unknown mob";
    }
}

void Mob::OnDeath() {
    SetState(MobState::DEAD);
    PlaySound("death");
}

void Mob::OnSpawn() {
    SetState(MobState::IDLE);
    PlaySound("spawn");
}

void Mob::OnAttack(Entity* target) {
    PlaySound("attack");
}

void Mob::OnDamage(Entity* source) {
    PlaySound("hurt");
}

void Mob::OnHeal(float amount) {
    PlaySound("heal");
}

void Mob::UpdateAI(float deltaTime) {
    // Base AI logic - can be overridden by specific mob types

    // Find target if hostile
    if (HasBehavior(MobBehavior::HOSTILE) && !m_target) {
        m_target = FindNearestTarget(m_attributes.followRange);
        if (m_target) {
            SetState(MobState::ATTACKING);
        }
    }

    // Flee if passive and threatened
    if (HasBehavior(MobBehavior::PASSIVE) && m_target) {
        if (DistanceToTarget(m_target) < 8.0f) {
            SetState(MobState::FLEEING);
        }
    }

    // Wander if idle
    if (m_state == MobState::IDLE && m_wanderTimer <= 0.0f) {
        SetState(MobState::WANDERING);
        m_wanderTimer = 10.0f + (rand() % 20); // 10-30 seconds

        // Pick random wander target
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(-10.0f, 10.0f);
        m_wanderTarget = GetPosition() + glm::vec3(dis(gen), 0.0f, dis(gen));
    }

    // Attack if has target and in range
    if (m_target && m_state == MobState::ATTACKING) {
        if (DistanceToTarget(m_target) <= m_attributes.followRange) {
            if (DistanceToTarget(m_target) <= m_attributes.attackRange) {
                if (HasBehavior(MobBehavior::RANGED)) {
                    RangedAttack(m_target);
                } else {
                    Attack(m_target);
                }
            } else {
                // Move towards target
                glm::vec3 direction = glm::normalize(m_target->GetPosition() - GetPosition());
                Move(direction, 1.0f);
            }
        } else {
            m_target = nullptr;
            SetState(MobState::IDLE);
        }
    }

    // Handle wandering
    if (m_state == MobState::WANDERING) {
        glm::vec3 direction = glm::normalize(m_wanderTarget - GetPosition());
        Move(direction, 0.5f);

        if (glm::distance(GetPosition(), m_wanderTarget) < 1.0f) {
            SetState(MobState::IDLE);
        }
    }

    // Handle fleeing
    if (m_state == MobState::FLEEING && m_target) {
        glm::vec3 direction = glm::normalize(GetPosition() - m_target->GetPosition());
        Move(direction, 1.2f);

        if (DistanceToTarget(m_target) > 12.0f) {
            SetState(MobState::IDLE);
            m_target = nullptr;
        }
    }
}

void Mob::UpdateMovement(float deltaTime) {
    // Base movement logic
    // Specific mob types can override this
}

void Mob::UpdateAnimations(float deltaTime) {
    m_animationTimer += deltaTime;

    // Simple animation based on state
    switch (m_state) {
        case MobState::IDLE:
            // Idle animation
            break;
        case MobState::WANDERING:
        case MobState::FOLLOWING:
            // Walking animation
            break;
        case MobState::ATTACKING:
            // Attack animation
            break;
        case MobState::FLEEING:
            // Running animation
            break;
        default:
            break;
    }
}

void Mob::UpdateEffects(float deltaTime) {
    // Update visual effects like glow, size changes, etc.

    if (m_onFire) {
        m_glowIntensity = std::min(1.0f, m_glowIntensity + deltaTime);
    } else {
        m_glowIntensity = std::max(0.0f, m_glowIntensity - deltaTime);
    }

    // Handle burning damage
    if (m_onFire && !m_attributes.immuneToFire) {
        static float burnTimer = 0.0f;
        burnTimer += deltaTime;
        if (burnTimer >= 1.0f) {
            TakeDamage(1.0f);
            burnTimer = 0.0f;
        }
    }

    // Handle sunlight damage
    if (m_attributes.burnsInSunlight) {
        // Check if in sunlight
        bool inSunlight = true; // This would check world light level
        if (inSunlight) {
            static float sunTimer = 0.0f;
            sunTimer += deltaTime;
            if (sunTimer >= 1.0f) {
                TakeDamage(1.0f);
                sunTimer = 0.0f;
            }
        }
    }
}

bool Mob::IsValidTarget(Entity* target) const {
    if (!target || !target->IsAlive()) return false;

    // Check distance
    if (DistanceToTarget(target) > m_attributes.viewRange) return false;

    // Check line of sight
    if (!HasLineOfSight(target)) return false;

    return true;
}

Entity* Mob::FindNearestTarget(float range) const {
    // This would search for nearby players or other entities
    // For now, return nullptr as we don't have a proper entity system
    return nullptr;
}

float Mob::DistanceToTarget(Entity* target) const {
    if (!target) return 9999.0f;
    return glm::distance(GetPosition(), target->GetPosition());
}

bool Mob::IsTargetInRange(Entity* target, float range) const {
    return DistanceToTarget(target) <= range;
}

bool Mob::HasLineOfSight(Entity* target) const {
    if (!target) return false;

    // This would typically do raycasting through the world
    // For now, assume line of sight if within range
    return DistanceToTarget(target) <= m_attributes.viewRange;
}

void Mob::InitializeAttributes() {
    // Base attributes - specific mobs will override these
    m_attributes.health = 20.0f;
    m_attributes.speed = 0.2f;
    m_attributes.attackDamage = 2.0f;
    m_attributes.attackSpeed = 1.0f;
    m_attributes.attackRange = 2.0f;
    m_attributes.followRange = 16.0f;
    m_attributes.viewRange = 16.0f;
    m_attributes.jumpHeight = 1.0f;
    m_attributes.size = 1.0f;
    m_attributes.experienceValue = 5;
    m_attributes.canDespawn = true;
    m_attributes.spawnsInDarkness = true;
    m_attributes.spawnLightLevel = 7;
    m_attributes.minSpawnGroup = 1;
    m_attributes.maxSpawnGroup = 1;
}

void Mob::InitializeBehaviors() {
    // Base behaviors - specific mobs will override these
    m_behaviors.clear();
    m_behaviors.push_back(MobBehavior::PASSIVE);
}

void Mob::InitializeSounds() {
    // Base sounds - specific mobs will override these
    m_attributes.sounds = {"idle", "hurt", "death"};
}

// HostileMob implementation
HostileMob::HostileMob(MobType type, const glm::vec3& position, World* world)
    : Mob(type, position, world) {
}

void HostileMob::InitializeAttributes() {
    Mob::InitializeAttributes();
    m_attributes.spawnsInDarkness = true;
    m_attributes.canDespawn = true;
}

void HostileMob::InitializeBehaviors() {
    Mob::InitializeBehaviors();
    RemoveBehavior(MobBehavior::PASSIVE);
    AddBehavior(MobBehavior::HOSTILE);
    AddBehavior(MobBehavior::MELEE);
}

// PassiveMob implementation
PassiveMob::PassiveMob(MobType type, const glm::vec3& position, World* world)
    : Mob(type, position, world) {
}

void PassiveMob::InitializeAttributes() {
    Mob::InitializeAttributes();
    m_attributes.spawnsInDarkness = false;
    m_attributes.canDespawn = false;
}

void PassiveMob::InitializeBehaviors() {
    Mob::InitializeBehaviors();
    AddBehavior(MobBehavior::PASSIVE);
}

// TamableMob implementation
TamableMob::TamableMob(MobType type, const glm::vec3& position, World* world)
    : PassiveMob(type, position, world)
    , m_tamed(false)
    , m_owner(nullptr)
    , m_tamingProgress(0.0f) {
}

void TamableMob::InitializeAttributes() {
    PassiveMob::InitializeAttributes();
    m_attributes.canDespawn = false;
}

void TamableMob::InitializeBehaviors() {
    PassiveMob::InitializeBehaviors();
    AddBehavior(MobBehavior::TAMABLE);
}

bool TamableMob::Tame(Entity* owner) {
    if (m_tamed) return false;

    m_owner = owner;
    m_tamed = true;
    m_tamingProgress = 1.0f;

    // Play taming sound
    PlaySound("tame");

    return true;
}

void TamableMob::Untame() {
    m_owner = nullptr;
    m_tamed = false;
    m_tamingProgress = 0.0f;
}

// FlyingMob implementation
FlyingMob::FlyingMob(MobType type, const glm::vec3& position, World* world)
    : Mob(type, position, world) {
}

void FlyingMob::InitializeAttributes() {
    Mob::InitializeAttributes();
    m_attributes.canFly = true;
    m_attributes.flySpeed = 0.3f;
    m_attributes.spawnsInDarkness = true;
}

void FlyingMob::InitializeBehaviors() {
    Mob::InitializeBehaviors();
    AddBehavior(MobBehavior::FLYING);
}

void FlyingMob::UpdateMovement(float deltaTime) {
    // Flying movement logic
    m_isFlying = true;
    m_onGround = false;
    m_velocity.y = 0.0f; // Neutral buoyancy
}

// AquaticMob implementation
AquaticMob::AquaticMob(MobType type, const glm::vec3& position, World* world)
    : Mob(type, position, world) {
}

void AquaticMob::InitializeAttributes() {
    Mob::InitializeAttributes();
    m_attributes.canBreatheUnderwater = true;
    m_attributes.canSwim = true;
    m_attributes.swimSpeed = 0.1f;
    m_attributes.spawnsInDarkness = false;
}

void AquaticMob::InitializeBehaviors() {
    Mob::InitializeBehaviors();
    AddBehavior(MobBehavior::SWIMMING);
}

void AquaticMob::UpdateMovement(float deltaTime) {
    if (m_inWater) {
        // Swimming movement
        m_velocity.y *= 0.8f; // Water friction
    }
}

// Creeper implementation
Creeper::Creeper(const glm::vec3& position, World* world)
    : HostileMob(MobType::CREEPER, position, world)
    , m_explosionTimer(0.0f)
    , m_explosionRadius(3.0f)
    , m_charged(false)
    , m_ignited(false) {
}

void Creeper::UpdateAI(float deltaTime) {
    HostileMob::UpdateAI(deltaTime);

    if (m_target) {
        float distance = DistanceToTarget(m_target);

        // Start hissing when close
        if (distance <= 3.0f && !m_ignited) {
            m_ignited = true;
            m_explosionTimer = 1.5f;
            PlaySound("hiss");
        }

        // Explode when timer runs out
        if (m_ignited) {
            m_explosionTimer -= deltaTime;
            if (m_explosionTimer <= 0.0f) {
                Explode();
            }
        }
    } else {
        m_ignited = false;
        m_explosionTimer = 0.0f;
    }
}

void Creeper::OnDeath() {
    Mob::OnDeath();

    // Creepers explode on death if ignited
    if (m_ignited) {
        Explode();
    }
}

void Creeper::Explode() {
    // Create explosion
    float radius = m_charged ? m_explosionRadius * 2.0f : m_explosionRadius;

    // This would typically damage entities and destroy blocks in radius
    // For now, just create a visual effect

    PlaySound("explode");

    // Remove the creeper
    Kill();
}

// Zombie implementation
Zombie::Zombie(const glm::vec3& position, World* world)
    : HostileMob(MobType::ZOMBIE, position, world)
    , m_canBreakDoors(false)
    , m_doorBreakTimer(0.0f)
    , m_doorBreakProgress(0) {
}

void Zombie::UpdateAI(float deltaTime) {
    HostileMob::UpdateAI(deltaTime);

    // Door breaking logic
    if (m_canBreakDoors && m_target && DistanceToTarget(m_target) <= 2.0f) {
        // Check if there's a door nearby
        // This would check world blocks for doors
        bool doorNearby = false;

        if (doorNearby) {
            m_doorBreakTimer += deltaTime;
            if (m_doorBreakTimer >= 2.0f) {
                m_doorBreakTimer = 0.0f;
                m_doorBreakProgress++;

                if (m_doorBreakProgress >= 10) {
                    // Break the door
                    // This would destroy the door block
                    m_doorBreakProgress = 0;
                }
            }
        } else {
            m_doorBreakTimer = 0.0f;
            m_doorBreakProgress = 0;
        }
    }
}

void Zombie::OnAttack(Entity* target) {
    HostileMob::OnAttack(target);

    // 10% chance to apply hunger effect in Hard difficulty
    // This would typically check game difficulty and apply effects
}

// Enderman implementation
Enderman::Enderman(const glm::vec3& position, World* world)
    : HostileMob(MobType::ENDERMAN, position, world)
    , m_heldBlockID(0)
    , m_stareTimer(0.0f)
    , m_isStaring(false)
    , m_isProvoked(false) {
}

void Enderman::UpdateAI(float deltaTime) {
    HostileMob::UpdateAI(deltaTime);

    // Enderman-specific AI
    if (m_target) {
        // Teleport when damaged
        if (m_isProvoked && m_teleportTimer <= 0.0f) {
            glm::vec3 teleportPos = GetPosition() + glm::vec3(
                (rand() % 32) - 16,
                0.0f,
                (rand() % 32) - 16
            );

            if (Teleport(teleportPos)) {
                m_isProvoked = false;
            }
        }

        // Stare at target
        if (HasLineOfSight(m_target)) {
            m_isStaring = true;
            m_stareTimer += deltaTime;

            if (m_stareTimer >= 5.0f) {
                // Provoke and attack
                m_isProvoked = true;
                SetState(MobState::ATTACKING);
                m_stareTimer = 0.0f;
            }
        } else {
            m_isStaring = false;
            m_stareTimer = 0.0f;
        }
    } else {
        m_isStaring = false;
        m_stareTimer = 0.0f;
        m_isProvoked = false;
    }
}

void Enderman::TakeDamage(float damage, Entity* source) {
    HostileMob::TakeDamage(damage, source);

    // Endermen teleport when damaged
    if (source && m_teleportTimer <= 0.0f) {
        m_isProvoked = true;
        SetTarget(source);
    }
}

bool Enderman::Teleport(const glm::vec3& position) {
    if (Mob::Teleport(position)) {
        PlaySound("teleport");
        // Create teleport particles
        return true;
    }
    return false;
}

// Wolf implementation
Wolf::Wolf(const glm::vec3& position, World* world)
    : TamableMob(MobType::WOLF, position, world)
    , m_isAngry(false)
    , m_hungerLevel(1.0f)
    , m_packLeader(nullptr) {
}

void Wolf::UpdateAI(float deltaTime) {
    TamableMob::UpdateAI(deltaTime);

    if (m_tamed && m_owner) {
        // Follow owner
        if (DistanceToTarget(m_owner) > 3.0f) {
            glm::vec3 direction = glm::normalize(m_owner->GetPosition() - GetPosition());
            Move(direction, 1.0f);
        }

        // Protect owner
        if (m_owner->GetHealth() < m_owner->GetMaxHealth() * 0.5f) {
            // Look for enemies near owner
            // This would search for hostile mobs near the owner
        }
    } else {
        // Wild wolf behavior
        if (m_hungerLevel < 0.3f) {
            // Hunt for food
            Hunt();
        }
    }
}

void Wolf::OnAttack(Entity* target) {
    TamableMob::OnAttack(target);

    // Wolves can howl when attacking
    if (rand() % 10 == 0) {
        Howl();
    }
}

void Wolf::Howl() {
    PlaySound("howl");
}

void Wolf::Hunt() {
    // Look for nearby animals to hunt
    // This would search for passive mobs like sheep, chicken, etc.
    SetState(MobState::FOLLOWING);
}

void Wolf::BegForFood() {
    // Play begging animation
    // This would typically show the wolf begging for food
}

// Villager implementation
Villager::Villager(const glm::vec3& position, World* world)
    : PassiveMob(MobType::VILLAGER, position, world)
    , m_profession("none")
    , m_level(1)
    , m_hasWorkstation(false)
    , m_hasBed(false)
    , m_workTimer(0.0f)
    , m_socialTimer(0.0f) {
}

void Villager::UpdateAI(float deltaTime) {
    PassiveMob::UpdateAI(deltaTime);

    m_workTimer += deltaTime;
    m_socialTimer += deltaTime;

    // Work at workstation
    if (m_hasWorkstation && m_workTimer >= 10.0f) {
        Work();
        m_workTimer = 0.0f;
    }

    // Socialize with other villagers
    if (m_socialTimer >= 30.0f) {
        Socialize();
        m_socialTimer = 0.0f;
    }

    // Sleep at night
    // This would check world time and go to bed if it's night
    bool isNight = false;
    if (isNight && m_hasBed) {
        Sleep();
    }
}

void Villager::Work() {
    // Perform job-specific work
    if (m_profession == "farmer") {
        // Farm crops
    } else if (m_profession == "librarian") {
        // Read books
    } else if (m_profession == "armorer") {
        // Craft armor
    }
    // Add more professions as needed
}

void Villager::Socialize() {
    // Look for other villagers to talk to
    // This would search for nearby villagers and interact
}

void Villager::Sleep() {
    // Go to bed
    SetState(MobState::SLEEPING);
}

void Villager::Trade() {
    // Open trading interface with player
    // This would typically open a trading GUI
}

} // namespace VoxelCraft
