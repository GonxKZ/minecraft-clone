/**
 * @file AIController.cpp
 * @brief VoxelCraft AI Controller Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "AIController.hpp"
#include "../entities/Entity.hpp"
#include "../player/Player.hpp"
#include "../world/World.hpp"
#include <algorithm>
#include <cmath>

namespace VoxelCraft {

    AIController::AIController(Entity* entity, const AIControllerConfig& config)
        : m_entity(entity), m_config(config), m_stats() {
        // Set home position to current position
        if (entity) {
            m_homePosition = entity->GetPosition();
        }

        // Initialize stats based on config
        m_stats.movementSpeed = config.movementSpeed;
        m_stats.perceptionRange = config.perceptionRange;
    }

    AIController::~AIController() {
        // Cleanup systems
    }

    bool AIController::Initialize() {
        try {
            InitializeBehaviorTree();
            InitializePerceptionSystem();
            InitializeMemorySystem();

            // Initialize pathfinding
            if (m_entity && m_entity->GetWorld()) {
                m_pathfinding = std::make_unique<Pathfinding>(m_entity->GetWorld());
                m_pathfinding->SetMaxSearchDistance(m_config.maxPathfindingDistance);
            }

            // Register with global managers
            if (m_perceptionSystem) {
                PerceptionManager::GetInstance().RegisterEntity(m_entity, std::move(m_perceptionSystem));
                m_perceptionSystem = nullptr; // Manager took ownership
            }

            if (m_memorySystem) {
                MemoryManager::GetInstance().RegisterEntity(m_entity, std::move(m_memorySystem));
                m_memorySystem = nullptr; // Manager took ownership
            }

            return true;
        } catch (const std::exception& e) {
            // Log error
            return false;
        }
    }

    void AIController::Update(float deltaTime) {
        if (!m_entity) return;

        m_updateTimer += deltaTime;
        if (m_updateTimer < m_config.updateInterval) return;
        m_updateTimer = 0.0f;

        // Update AI systems
        UpdateStateLogic(deltaTime);
        UpdateEmotion(deltaTime);
        UpdateStats(deltaTime);
        UpdateMovement(deltaTime);

        // Execute behavior tree
        if (m_behaviorTree) {
            BehaviorContext context;
            context.entity = m_entity;
            context.world = m_entity->GetWorld();
            context.targetPlayer = dynamic_cast<Player*>(m_target);
            context.deltaTime = deltaTime;
            context.currentTime = std::chrono::steady_clock::now();

            m_behaviorTree->Execute(context);
        }

        // Handle state transitions
        HandleStateTransitions();

        // Execute current behavior
        ExecuteCurrentBehavior();
    }

    void AIController::SetState(AIState state) {
        if (m_currentState != state) {
            m_currentState = state;
            m_stateTimer = 0.0f;
            TriggerEvent("state_changed");
        }
    }

    void AIController::SetGoal(AIGoal goal) {
        if (m_currentGoal != goal) {
            m_currentGoal = goal;
            TriggerEvent("goal_changed");
        }
    }

    void AIController::SetEmotion(AIEmotion emotion) {
        if (m_currentEmotion != emotion) {
            m_currentEmotion = emotion;
            m_emotionTimer = 0.0f;
            TriggerEvent("emotion_changed");
        }
    }

    bool AIController::MoveToPosition(const glm::vec3& position) {
        if (!m_pathfinding) return false;

        auto path = m_pathfinding->FindPath(m_entity->GetPosition(), position, m_entity);
        if (!path.empty()) {
            m_currentPath = Path(path);
            m_moveTarget = position;
            return true;
        }

        return false;
    }

    bool AIController::Attack(Entity* target) {
        if (!target || m_attackCooldown > 0.0f) return false;

        // Calculate attack
        float distance = GetDistanceToTarget(target);
        if (distance <= m_config.attackRange) {
            // Perform attack
            float damage = m_stats.attackDamage;
            target->TakeDamage(damage);

            m_attackCooldown = 1.0f / m_stats.attackSpeed;
            TriggerEvent("attack");
            return true;
        }

        return false;
    }

    bool AIController::FleeFrom(const glm::vec3& threatPosition) {
        if (!m_entity) return false;

        // Calculate flee direction (opposite to threat)
        glm::vec3 fleeDirection = glm::normalize(m_entity->GetPosition() - threatPosition);
        glm::vec3 fleeTarget = m_entity->GetPosition() + fleeDirection * 10.0f;

        // Try to find safe position
        if (MoveToPosition(fleeTarget)) {
            SetState(AIState::FLEEING);
            return true;
        }

        return false;
    }

    bool AIController::CanSee(Entity* target) const {
        if (!target) return false;

        auto perceptionSystem = PerceptionManager::GetInstance().GetPerceptionSystem(m_entity);
        if (!perceptionSystem) return false;

        PerceptionStimulus stimulus(StimulusType::PLAYER_SIGHT, target->GetPosition());
        auto result = perceptionSystem->CanPerceive(stimulus);

        return result.detected;
    }

    bool AIController::CanHear(Entity* target) const {
        if (!target) return false;

        auto perceptionSystem = PerceptionManager::GetInstance().GetPerceptionSystem(m_entity);
        if (!perceptionSystem) return false;

        PerceptionStimulus stimulus(StimulusType::PLAYER_SOUND, target->GetPosition());
        auto result = perceptionSystem->CanPerceive(stimulus);

        return result.detected;
    }

    float AIController::GetDistanceToTarget(Entity* target) const {
        if (!target || !m_entity) return -1.0f;
        return glm::distance(m_entity->GetPosition(), target->GetPosition());
    }

    void AIController::RegisterEventCallback(std::function<void(const std::string&, Entity*)> callback) {
        if (callback) {
            m_eventCallbacks.push_back(callback);
        }
    }

    void AIController::InitializeBehaviorTree() {
        m_behaviorTree = std::make_unique<BehaviorTree>("MainBehaviorTree");

        // Create root selector
        auto rootSelector = std::make_shared<SelectorNode>("RootSelector");

        // Survival behaviors (highest priority)
        auto survivalSequence = std::make_shared<SequenceNode>("Survival");

        // Health check
        auto healthCheck = std::make_shared<ConditionNode>("LowHealth",
            [this](BehaviorContext& context) {
                return m_stats.health < m_stats.maxHealth * 0.3f;
            });

        // Find healing action
        auto findHealing = std::make_shared<ActionNode>("FindHealing",
            [this](BehaviorContext& context) {
                // Look for food or healing items
                return NodeStatus::SUCCESS;
            });

        survivalSequence->AddChild(healthCheck);
        survivalSequence->AddChild(findHealing);

        // Combat behaviors
        auto combatSelector = std::make_shared<SelectorNode>("Combat");

        // Attack sequence
        auto attackSequence = std::make_shared<SequenceNode>("Attack");
        auto hasTarget = std::make_shared<ConditionNode>("HasTarget",
            [this](BehaviorContext& context) { return m_target != nullptr; });

        auto inRange = std::make_shared<ConditionNode>("InAttackRange",
            [this](BehaviorContext& context) {
                if (!m_target) return false;
                return GetDistanceToTarget(m_target) <= m_config.attackRange;
            });

        auto attackAction = std::make_shared<ActionNode>("Attack",
            [this](BehaviorContext& context) {
                if (Attack(m_target)) {
                    return NodeStatus::SUCCESS;
                }
                return NodeStatus::FAILURE;
            });

        attackSequence->AddChild(hasTarget);
        attackSequence->AddChild(inRange);
        attackSequence->AddChild(attackAction);

        // Chase sequence
        auto chaseSequence = std::make_shared<SequenceNode>("Chase");
        auto chaseAction = std::make_shared<ActionNode>("Chase",
            [this](BehaviorContext& context) {
                if (m_target && MoveToPosition(m_target->GetPosition())) {
                    SetState(AIState::CHASING);
                    return NodeStatus::RUNNING;
                }
                return NodeStatus::FAILURE;
            });

        chaseSequence->AddChild(hasTarget);
        chaseSequence->AddChild(chaseAction);

        combatSelector->AddChild(attackSequence);
        combatSelector->AddChild(chaseSequence);

        // Exploration behaviors
        auto exploreAction = std::make_shared<ActionNode>("Explore",
            [this](BehaviorContext& context) {
                // Generate random exploration target
                glm::vec3 randomPos = m_entity->GetPosition() +
                    glm::vec3((rand() % 21 - 10), 0, (rand() % 21 - 10));

                if (MoveToPosition(randomPos)) {
                    SetState(AIState::WANDERING);
                    return NodeStatus::RUNNING;
                }
                return NodeStatus::SUCCESS;
            });

        // Add all behaviors to root (priority order)
        rootSelector->AddChild(survivalSequence);
        rootSelector->AddChild(combatSelector);
        rootSelector->AddChild(exploreAction);

        m_behaviorTree->SetRoot(rootSelector);
    }

    void AIController::InitializePerceptionSystem() {
        m_perceptionSystem = std::make_unique<PerceptionSystem>(m_entity);

        // Configure perception based on AI type
        if (m_config.aiType == "hostile") {
            m_stats.aggression = 0.8f;
            m_stats.fear = 0.2f;
        } else if (m_config.aiType == "passive") {
            m_stats.aggression = 0.1f;
            m_stats.fear = 0.8f;
        } else if (m_config.aiType == "neutral") {
            m_stats.aggression = 0.3f;
            m_stats.fear = 0.3f;
        }
    }

    void AIController::InitializeMemorySystem() {
        m_memorySystem = std::make_unique<MemorySystem>(m_entity, static_cast<int>(m_stats.memoryCapacity));
    }

    void AIController::UpdateStateLogic(float deltaTime) {
        m_stateTimer += deltaTime;

        // Find best target
        Entity* bestTarget = FindBestTarget();
        if (bestTarget != m_target) {
            m_target = bestTarget;
            if (m_target) {
                TriggerEvent("target_acquired");
            } else {
                TriggerEvent("target_lost");
            }
        }

        // Update attack cooldown
        if (m_attackCooldown > 0.0f) {
            m_attackCooldown -= deltaTime;
        }
    }

    void AIController::UpdateEmotion(float deltaTime) {
        m_emotionTimer += deltaTime;

        // Update emotion based on current situation
        if (m_emotionTimer >= 1.0f) { // Check every second
            m_emotionTimer = 0.0f;

            if (m_stats.health < m_stats.maxHealth * 0.2f) {
                SetEmotion(AIEmotion::FEARFUL);
            } else if (m_target && m_config.isHostile) {
                SetEmotion(AIEmotion::ANGRY);
            } else if (m_stats.hunger < m_stats.maxHunger * 0.3f) {
                SetEmotion(AIEmotion::HUNGRY);
            } else if (m_stats.energy < m_stats.maxEnergy * 0.3f) {
                SetEmotion(AIEmotion::TIRED);
            } else {
                SetEmotion(AIEmotion::NEUTRAL);
            }
        }
    }

    void AIController::UpdateStats(float deltaTime) {
        // Update hunger
        m_stats.hunger -= deltaTime * 0.1f;
        m_stats.hunger = std::max(0.0f, m_stats.hunger);

        // Update energy
        if (m_currentState == AIState::IDLE || m_currentState == AIState::RESTING) {
            m_stats.energy += deltaTime * 0.5f;
        } else {
            m_stats.energy -= deltaTime * 0.2f;
        }
        m_stats.energy = std::clamp(m_stats.energy, 0.0f, m_stats.maxEnergy);

        // Update fear and aggression based on situation
        if (m_target) {
            float threatLevel = CalculateThreatLevel(m_target);
            m_stats.fear = std::min(m_stats.fear + threatLevel * 0.1f, 1.0f);
            m_stats.aggression = std::min(m_stats.aggression + (1.0f - threatLevel) * 0.1f, 1.0f);
        } else {
            m_stats.fear = std::max(m_stats.fear - 0.05f, 0.0f);
            m_stats.aggression = std::max(m_stats.aggression - 0.02f, 0.1f);
        }
    }

    void AIController::HandleStateTransitions() {
        switch (m_currentState) {
            case AIState::IDLE:
                if (m_target && m_config.isHostile) {
                    SetState(AIState::CHASING);
                } else if (m_stats.hunger < m_stats.maxHunger * 0.5f) {
                    SetState(AIState::HUNTING);
                } else if (m_stateTimer > 5.0f) { // Been idle for 5 seconds
                    SetState(AIState::WANDERING);
                }
                break;

            case AIState::CHASING:
                if (!m_target) {
                    SetState(AIState::IDLE);
                } else if (GetDistanceToTarget(m_target) <= m_config.attackRange) {
                    SetState(AIState::ATTACKING);
                }
                break;

            case AIState::ATTACKING:
                if (!m_target || GetDistanceToTarget(m_target) > m_config.attackRange) {
                    SetState(AIState::CHASING);
                }
                break;

            case AIState::FLEEING:
                if (!m_target || m_stateTimer > 10.0f) {
                    SetState(AIState::IDLE);
                }
                break;

            default:
                break;
        }
    }

    void AIController::ExecuteCurrentBehavior() {
        switch (m_currentState) {
            case AIState::WANDERING:
                // Continue wandering behavior
                break;

            case AIState::CHASING:
                if (m_target) {
                    MoveToPosition(m_target->GetPosition());
                }
                break;

            case AIState::ATTACKING:
                if (m_target) {
                    Attack(m_target);
                }
                break;

            case AIState::FLEEING:
                if (m_target) {
                    FleeFrom(m_target->GetPosition());
                }
                break;

            default:
                break;
        }
    }

    Entity* AIController::FindBestTarget() {
        if (!m_entity || !m_entity->GetWorld()) return nullptr;

        // Simple target finding - look for closest player
        // In a real implementation, this would be more sophisticated
        auto players = m_entity->GetWorld()->GetPlayers();
        Entity* bestTarget = nullptr;
        float bestScore = 0.0f;

        for (auto player : players) {
            if (!player) continue;

            float distance = GetDistanceToTarget(player);
            if (distance > m_config.perceptionRange) continue;

            float score = 0.0f;

            // Distance factor (closer is better)
            score += (m_config.perceptionRange - distance) / m_config.perceptionRange;

            // Visibility factor
            if (CanSee(player)) score += 0.5f;

            // Aggression factor
            score += m_stats.aggression * 0.3f;

            if (score > bestScore) {
                bestScore = score;
                bestTarget = player;
            }
        }

        return bestTarget;
    }

    float AIController::CalculateThreatLevel(Entity* entity) const {
        if (!entity) return 0.0f;

        float threat = 0.0f;

        // Distance factor
        float distance = GetDistanceToTarget(entity);
        if (distance > 0) {
            threat += 1.0f - std::min(distance / m_config.perceptionRange, 1.0f);
        }

        // Health difference
        if (entity->GetHealth() > m_stats.health) {
            threat += 0.3f;
        }

        return std::min(threat, 1.0f);
    }

    float AIController::CalculateInterestLevel(Entity* entity) const {
        if (!entity) return 0.0f;

        float interest = 0.0f;

        // Curiosity factor
        interest += m_stats.curiosity * 0.4f;

        // Social factor
        interest += m_stats.social * 0.3f;

        // Distance factor (moderate distance is more interesting)
        float distance = GetDistanceToTarget(entity);
        if (distance > 0) {
            float normalizedDistance = distance / m_config.perceptionRange;
            interest += (1.0f - std::abs(normalizedDistance - 0.5f) * 2.0f) * 0.3f;
        }

        return std::min(interest, 1.0f);
    }

    void AIController::TriggerEvent(const std::string& event) {
        for (auto& callback : m_eventCallbacks) {
            if (callback) {
                callback(event, m_entity);
            }
        }
    }

    void AIController::UpdateMovement(float deltaTime) {
        if (!m_entity || m_currentPath.IsFinished()) return;

        // Move along current path
        glm::vec3 nextWaypoint = m_currentPath.GetNextWaypoint();
        glm::vec3 currentPos = m_entity->GetPosition();
        glm::vec3 direction = glm::normalize(nextWaypoint - currentPos);

        // Simple movement (in real implementation would use physics)
        glm::vec3 newPos = currentPos + direction * m_stats.movementSpeed * deltaTime;

        // Check if reached waypoint
        if (glm::distance(newPos, nextWaypoint) < 0.5f) {
            m_currentPath.MoveToNextWaypoint();
        } else {
            m_entity->SetPosition(newPos);
        }
    }

    // AIControllerFactory implementation
    std::unordered_map<std::string, std::function<std::unique_ptr<AIController>(Entity*)>>
        AIControllerFactory::s_creators;

    std::unique_ptr<AIController> AIControllerFactory::CreateController(Entity* entity, const std::string& aiType) {
        auto it = s_creators.find(aiType);
        if (it != s_creators.end()) {
            return it->second(entity);
        }

        // Default generic AI
        AIControllerConfig config;
        config.aiType = aiType;
        return std::make_unique<AIController>(entity, config);
    }

    void AIControllerFactory::RegisterAIType(const std::string& type,
                                           std::function<std::unique_ptr<AIController>(Entity*)> creator) {
        s_creators[type] = creator;
    }

} // namespace VoxelCraft
