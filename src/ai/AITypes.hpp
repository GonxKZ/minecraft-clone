/**
 * @file AITypes.hpp
 * @brief VoxelCraft AI System Common Types and Definitions
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines common types, structures, enums, and constants used
 * throughout the AI system in VoxelCraft.
 */

#ifndef VOXELCRAFT_AI_AI_TYPES_HPP
#define VOXELCRAFT_AI_AI_TYPES_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <functional>
#include <array>
#include <chrono>
#include <any>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"
#include "../physics/PhysicsTypes.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Mob;
    class Entity;
    class World;
    class Blackboard;
    class BehaviorTree;
    class PathfindingSystem;
    class NavigationSystem;
    class ProceduralGenerator;
    class VillageSystem;
    class SensorySystem;
    class DecisionSystem;
    class LearningSystem;
    class BlackboardSystem;

    /**
     * @struct AIVector3
     * @brief AI-specific 3D vector with additional functionality
     */
    struct AIVector3 : public glm::vec3 {
        using glm::vec3::vec3; // Inherit constructors

        AIVector3() = default;
        AIVector3(const glm::vec3& v) : glm::vec3(v) {}
        AIVector3(float x, float y, float z) : glm::vec3(x, y, z) {}

        /**
         * @brief Check if vector is valid (no NaN or infinite values)
         * @return true if valid, false otherwise
         */
        bool IsValid() const {
            return !glm::any(glm::isnan(*this)) && !glm::any(glm::isinf(*this));
        }

        /**
         * @brief Get distance to another vector
         * @param other Other vector
         * @return Distance
         */
        float DistanceTo(const AIVector3& other) const {
            return glm::distance(static_cast<glm::vec3>(*this), static_cast<glm::vec3>(other));
        }

        /**
         * @brief Check if point is within range
         * @param center Center point
         * @param radius Range radius
         * @return true if within range, false otherwise
         */
        bool IsWithinRange(const AIVector3& center, float radius) const {
            return DistanceTo(center) <= radius;
        }

        /**
         * @brief Get direction to another vector
         * @param target Target vector
         * @return Normalized direction vector
         */
        AIVector3 DirectionTo(const AIVector3& target) const {
            AIVector3 dir = target - *this;
            float length = glm::length(static_cast<glm::vec3>(dir));
            return length > 0.0f ? dir / length : AIVector3(0.0f, 0.0f, 0.0f);
        }

        /**
         * @brief Clamp vector to bounds
         * @param min Minimum bounds
         * @param max Maximum bounds
         * @return Clamped vector
         */
        AIVector3 Clamp(const AIVector3& min, const AIVector3& max) const {
            return AIVector3(
                glm::clamp(x, min.x, max.x),
                glm::clamp(y, min.y, max.y),
                glm::clamp(z, min.z, max.z)
            );
        }

        /**
         * @brief Convert to string representation
         * @return String representation
         */
        std::string ToString() const {
            return "(" + std::to_string(x) + ", " +
                   std::to_string(y) + ", " +
                   std::to_string(z) + ")";
        }
    };

    /**
     * @struct AITransform
     * @brief AI-specific transform with position, rotation, and scale
     */
    struct AITransform {
        AIVector3 position;                     ///< Position in world space
        glm::quat rotation;                     ///< Rotation quaternion
        AIVector3 scale;                        ///< Scale factors

        AITransform()
            : position(0.0f, 0.0f, 0.0f)
            , rotation(1.0f, 0.0f, 0.0f, 0.0f)
            , scale(1.0f, 1.0f, 1.0f)
        {}

        AITransform(const AIVector3& pos, const glm::quat& rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
                   const AIVector3& scl = AIVector3(1.0f, 1.0f, 1.0f))
            : position(pos), rotation(rot), scale(scl)
        {}

        /**
         * @brief Get forward direction
         * @return Forward vector
         */
        AIVector3 GetForward() const {
            return AIVector3(glm::rotate(rotation, AIVector3(0.0f, 0.0f, 1.0f)));
        }

        /**
         * @brief Get up direction
         * @return Up vector
         */
        AIVector3 GetUp() const {
            return AIVector3(glm::rotate(rotation, AIVector3(0.0f, 1.0f, 0.0f)));
        }

        /**
         * @brief Get right direction
         * @return Right vector
         */
        AIVector3 GetRight() const {
            return AIVector3(glm::rotate(rotation, AIVector3(1.0f, 0.0f, 0.0f)));
        }

        /**
         * @brief Transform point by this transform
         * @param point Point to transform
         * @return Transformed point
         */
        AIVector3 TransformPoint(const AIVector3& point) const {
            AIVector3 rotated = AIVector3(glm::rotate(rotation, static_cast<glm::vec3>(point)));
            return position + rotated * scale;
        }

        /**
         * @brief Transform direction by this transform
         * @param direction Direction to transform
         * @return Transformed direction
         */
        AIVector3 TransformDirection(const AIVector3& direction) const {
            return AIVector3(glm::rotate(rotation, static_cast<glm::vec3>(direction)));
        }

        /**
         * @brief Check if transform is valid
         * @return true if valid, false otherwise
         */
        bool IsValid() const {
            return position.IsValid() && scale.IsValid() &&
                   !glm::any(glm::isnan(rotation)) && !glm::any(glm::isinf(rotation));
        }

        /**
         * @brief Convert to string representation
         * @return String representation
         */
        std::string ToString() const {
            return "Position: " + position.ToString() +
                   ", Scale: " + scale.ToString();
        }
    };

    /**
     * @struct AIBounds
     * @brief AI-specific bounding volume
     */
    struct AIBounds {
        enum class Type {
            Sphere,                             ///< Spherical bounds
            AABB,                               ///< Axis-aligned bounding box
            OBB                                 ///< Oriented bounding box
        };

        Type type;                              ///< Bounds type
        AIVector3 center;                       ///< Bounds center
        union {
            float sphereRadius;                 ///< Sphere radius
            struct {
                AIVector3 min;                 ///< AABB minimum
                AIVector3 max;                 ///< AABB maximum
            } aabb;
            struct {
                AIVector3 extents;             ///< OBB extents
                AITransform transform;         ///< OBB transform
            } obb;
        };

        AIBounds() : type(Type::Sphere), sphereRadius(1.0f) {}

        /**
         * @brief Create sphere bounds
         * @param center Sphere center
         * @param radius Sphere radius
         * @return Sphere bounds
         */
        static AIBounds Sphere(const AIVector3& center, float radius) {
            AIBounds bounds;
            bounds.type = Type::Sphere;
            bounds.center = center;
            bounds.sphereRadius = radius;
            return bounds;
        }

        /**
         * @brief Create AABB bounds
         * @param min Minimum corner
         * @param max Maximum corner
         * @return AABB bounds
         */
        static AIBounds AABB(const AIVector3& min, const AIVector3& max) {
            AIBounds bounds;
            bounds.type = Type::AABB;
            bounds.center = (min + max) * 0.5f;
            bounds.aabb.min = min;
            bounds.aabb.max = max;
            return bounds;
        }

        /**
         * @brief Check if point is inside bounds
         * @param point Point to check
         * @return true if inside, false otherwise
         */
        bool Contains(const AIVector3& point) const {
            switch (type) {
                case Type::Sphere:
                    return glm::distance(static_cast<glm::vec3>(center), static_cast<glm::vec3>(point)) <= sphereRadius;

                case Type::AABB:
                    return point.x >= aabb.min.x && point.x <= aabb.max.x &&
                           point.y >= aabb.min.y && point.y <= aabb.max.y &&
                           point.z >= aabb.min.z && point.z <= aabb.max.z;

                case Type::OBB:
                    // Transform point to OBB local space
                    AIVector3 localPoint = obb.transform.TransformPoint(point - obb.transform.position);
                    return glm::abs(localPoint.x) <= obb.extents.x &&
                           glm::abs(localPoint.y) <= obb.extents.y &&
                           glm::abs(localPoint.z) <= obb.extents.z;

                default:
                    return false;
            }
        }

        /**
         * @brief Check if bounds intersect with another bounds
         * @param other Other bounds
         * @return true if intersect, false otherwise
         */
        bool Intersects(const AIBounds& other) const {
            // Simplified intersection test - can be expanded for better accuracy
            return Contains(other.center) || other.Contains(center);
        }

        /**
         * @brief Get bounds size
         * @return Size vector
         */
        AIVector3 GetSize() const {
            switch (type) {
                case Type::Sphere:
                    return AIVector3(sphereRadius * 2.0f);

                case Type::AABB:
                    return aabb.max - aabb.min;

                case Type::OBB:
                    return obb.extents * 2.0f;

                default:
                    return AIVector3(0.0f);
            }
        }

        /**
         * @brief Get bounds volume
         * @return Volume
         */
        float GetVolume() const {
            switch (type) {
                case Type::Sphere:
                    return (4.0f / 3.0f) * glm::pi<float>() * sphereRadius * sphereRadius * sphereRadius;

                case Type::AABB:
                    AIVector3 size = GetSize();
                    return size.x * size.y * size.z;

                case Type::OBB:
                    return obb.extents.x * obb.extents.y * obb.extents.z * 8.0f;

                default:
                    return 0.0f;
            }
        }
    };

    /**
     * @struct AITimer
     * @brief AI-specific timer with various timing modes
     */
    struct AITimer {
        enum class Mode {
            Once,                               ///< Timer fires once
            Repeat,                             ///< Timer repeats indefinitely
            PingPong                            ///< Timer goes back and forth
        };

        Mode mode;                              ///< Timer mode
        double duration;                        ///< Timer duration (seconds)
        double currentTime;                     ///< Current time
        double startTime;                       ///< Start time
        bool isRunning;                         ///< Timer is running
        bool isPaused;                          ///< Timer is paused
        int repeatCount;                        ///< Number of times repeated
        int maxRepeats;                         ///< Maximum repetitions (-1 for infinite)

        AITimer()
            : mode(Mode::Once)
            , duration(1.0)
            , currentTime(0.0)
            , startTime(0.0)
            , isRunning(false)
            , isPaused(false)
            , repeatCount(0)
            , maxRepeats(-1)
        {}

        AITimer(double dur, Mode m = Mode::Once)
            : mode(m)
            , duration(dur)
            , currentTime(0.0)
            , startTime(0.0)
            , isRunning(false)
            , isPaused(false)
            , repeatCount(0)
            , maxRepeats(-1)
        {}

        /**
         * @brief Start the timer
         */
        void Start() {
            if (!isRunning) {
                startTime = std::chrono::duration<double>(
                    std::chrono::high_resolution_clock::now().time_since_epoch()).count();
                currentTime = 0.0;
                isRunning = true;
                isPaused = false;
            }
        }

        /**
         * @brief Stop the timer
         */
        void Stop() {
            isRunning = false;
            isPaused = false;
            currentTime = 0.0;
            repeatCount = 0;
        }

        /**
         * @brief Pause the timer
         */
        void Pause() {
            if (isRunning && !isPaused) {
                isPaused = true;
            }
        }

        /**
         * @brief Resume the timer
         */
        void Resume() {
            if (isRunning && isPaused) {
                isPaused = false;
            }
        }

        /**
         * @brief Update the timer
         * @param deltaTime Time elapsed
         * @return true if timer completed, false otherwise
         */
        bool Update(double deltaTime) {
            if (!isRunning || isPaused) {
                return false;
            }

            currentTime += deltaTime;

            if (currentTime >= duration) {
                repeatCount++;

                switch (mode) {
                    case Mode::Once:
                        Stop();
                        return true;

                    case Mode::Repeat:
                        if (maxRepeats >= 0 && repeatCount >= maxRepeats) {
                            Stop();
                            return true;
                        } else {
                            currentTime -= duration;
                            return true;
                        }

                    case Mode::PingPong:
                        currentTime = duration - (currentTime - duration);
                        return true;

                    default:
                        return false;
                }
            }

            return false;
        }

        /**
         * @brief Get normalized progress (0.0 - 1.0)
         * @return Progress
         */
        float GetProgress() const {
            if (duration <= 0.0) return 1.0f;
            return static_cast<float>(glm::clamp(currentTime / duration, 0.0, 1.0));
        }

        /**
         * @brief Get remaining time
         * @return Remaining time
         */
        double GetRemainingTime() const {
            return std::max(0.0, duration - currentTime);
        }

        /**
         * @brief Check if timer is completed
         * @return true if completed, false otherwise
         */
        bool IsCompleted() const {
            return !isRunning && repeatCount > 0;
        }

        /**
         * @brief Reset the timer
         */
        void Reset() {
            currentTime = 0.0;
            repeatCount = 0;
            isRunning = false;
            isPaused = false;
        }
    };

    /**
     * @struct AIKnowledgeBase
     * @brief Knowledge base for AI decision making
     */
    struct AIKnowledgeBase {
        std::unordered_map<std::string, std::any> facts;      ///< Known facts
        std::unordered_map<std::string, std::vector<std::string>> rules; ///< Inference rules
        std::unordered_map<std::string, float> beliefs;      ///< Belief values (0.0 - 1.0)
        std::unordered_map<std::string, std::vector<std::string>> relationships; ///< Entity relationships

        /**
         * @brief Add fact to knowledge base
         * @param key Fact key
         * @param value Fact value
         */
        template<typename T>
        void AddFact(const std::string& key, const T& value) {
            facts[key] = value;
        }

        /**
         * @brief Get fact from knowledge base
         * @tparam T Expected type
         * @param key Fact key
         * @param defaultValue Default value
         * @return Fact value or default
         */
        template<typename T>
        T GetFact(const std::string& key, const T& defaultValue = T{}) const {
            auto it = facts.find(key);
            if (it != facts.end()) {
                try {
                    return std::any_cast<T>(it->second);
                } catch (const std::bad_any_cast&) {
                    return defaultValue;
                }
            }
            return defaultValue;
        }

        /**
         * @brief Add belief to knowledge base
         * @param fact Fact to believe in
         * @param confidence Confidence level (0.0 - 1.0)
         */
        void AddBelief(const std::string& fact, float confidence) {
            beliefs[fact] = glm::clamp(confidence, 0.0f, 1.0f);
        }

        /**
         * @brief Get belief confidence
         * @param fact Fact to check
         * @return Confidence level or 0.0 if unknown
         */
        float GetBelief(const std::string& fact) const {
            auto it = beliefs.find(fact);
            return (it != beliefs.end()) ? it->second : 0.0f;
        }

        /**
         * @brief Add relationship between entities
         * @param entity1 First entity
         * @param entity2 Second entity
         * @param relationship Relationship type
         */
        void AddRelationship(const std::string& entity1, const std::string& entity2,
                           const std::string& relationship) {
            relationships[entity1].push_back(entity2 + ":" + relationship);
            relationships[entity2].push_back(entity1 + ":" + relationship);
        }

        /**
         * @brief Get relationships for entity
         * @param entity Entity to check
         * @return Vector of relationships
         */
        const std::vector<std::string>& GetRelationships(const std::string& entity) const {
            static const std::vector<std::string> empty;
            auto it = relationships.find(entity);
            return (it != relationships.end()) ? it->second : empty;
        }

        /**
         * @brief Clear all knowledge
         */
        void Clear() {
            facts.clear();
            rules.clear();
            beliefs.clear();
            relationships.clear();
        }

        /**
         * @brief Get knowledge summary
         * @return Summary string
         */
        std::string GetSummary() const {
            return "Facts: " + std::to_string(facts.size()) +
                   ", Beliefs: " + std::to_string(beliefs.size()) +
                   ", Relationships: " + std::to_string(relationships.size());
        }
    };

    /**
     * @struct AIBehaviorProfile
     * @brief Behavior profile defining AI personality and tendencies
     */
    struct AIBehaviorProfile {
        std::string name;                       ///< Profile name
        std::string description;                ///< Profile description

        // Personality traits (0.0 - 1.0)
        float aggression;                       ///< Tendency towards aggressive behavior
        float curiosity;                        ///< Tendency to explore and investigate
        float fearfulness;                      ///< Tendency to be afraid
        float greediness;                       ///< Tendency to seek resources
        float socialness;                       ///< Tendency to be social
        float patience;                         ///< Patience level
        float intelligence;                     ///< Problem-solving capability
        float adaptability;                     ///< Ability to adapt to changes

        // Behavioral preferences
        std::vector<std::string> preferredActions; ///< Preferred action types
        std::vector<std::string> avoidedActions; ///< Actions to avoid
        std::vector<std::string> preferredTargets; ///< Preferred target types
        std::vector<std::string> fearedStimuli;  ///< Feared stimulus types

        // Response curves
        std::unordered_map<std::string, std::function<float(float)>> responseCurves; ///< Stimulus response curves

        AIBehaviorProfile()
            : aggression(0.5f), curiosity(0.5f), fearfulness(0.5f), greediness(0.5f)
            , socialness(0.5f), patience(0.5f), intelligence(0.5f), adaptability(0.5f)
        {}

        /**
         * @brief Get response to stimulus
         * @param stimulusType Type of stimulus
         * @param intensity Stimulus intensity (0.0 - 1.0)
         * @return Response value (0.0 - 1.0)
         */
        float GetResponse(const std::string& stimulusType, float intensity) const {
            auto it = responseCurves.find(stimulusType);
            if (it != responseCurves.end()) {
                return glm::clamp(it->second(intensity), 0.0f, 1.0f);
            }

            // Default response based on personality traits
            if (stimulusType == "threat") {
                return glm::mix(aggression, fearfulness, 0.5f) * intensity;
            } else if (stimulusType == "opportunity") {
                return greediness * intensity;
            } else if (stimulusType == "social") {
                return socialness * intensity;
            }

            return 0.5f * intensity; // Neutral response
        }

        /**
         * @brief Check if action is preferred
         * @param actionType Type of action
         * @return true if preferred, false otherwise
         */
        bool IsActionPreferred(const std::string& actionType) const {
            return std::find(preferredActions.begin(), preferredActions.end(), actionType) != preferredActions.end();
        }

        /**
         * @brief Check if action is avoided
         * @param actionType Type of action
         * @return true if avoided, false otherwise
         */
        bool IsActionAvoided(const std::string& actionType) const {
            return std::find(avoidedActions.begin(), avoidedActions.end(), actionType) != avoidedActions.end();
        }

        /**
         * @brief Get profile summary
         * @return Summary string
         */
        std::string GetSummary() const {
            return name + " (A:" + std::to_string(aggression) +
                   " C:" + std::to_string(curiosity) +
                   " F:" + std::to_string(fearfulness) + ")";
        }
    };

    /**
     * @struct AIDecisionContext
     * @brief Context information for AI decision making
     */
    struct AIDecisionContext {
        Mob* decisionMaker;                      ///< Entity making the decision
        Blackboard* blackboard;                  ///< Shared knowledge base
        double currentTime;                      ///< Current game time
        double deltaTime;                        ///< Time since last decision

        // Environmental context
        AIVector3 position;                      ///< Current position
        AIVector3 forward;                       ///< Forward direction
        std::vector<Entity*> nearbyEntities;     ///< Nearby entities
        std::unordered_map<std::string, float> environmentalFactors; ///< Environmental factors

        // Internal state
        float health;                            ///< Current health
        float hunger;                            ///< Current hunger
        float stamina;                           ///< Current stamina
        std::vector<std::string> activeGoals;    ///< Currently active goals
        std::vector<std::string> completedGoals; ///< Recently completed goals

        // Recent history
        std::vector<std::string> recentActions;  ///< Recently performed actions
        std::vector<std::string> recentStimuli;  ///< Recently detected stimuli
        std::unordered_map<std::string, double> actionCooldowns; ///< Action cooldowns

        AIDecisionContext()
            : decisionMaker(nullptr)
            , blackboard(nullptr)
            , currentTime(0.0)
            , deltaTime(0.0)
            , health(1.0f)
            , hunger(0.0f)
            , stamina(1.0f)
        {}

        /**
         * @brief Check if action is on cooldown
         * @param actionType Type of action
         * @return true if on cooldown, false otherwise
         */
        bool IsActionOnCooldown(const std::string& actionType) const {
            auto it = actionCooldowns.find(actionType);
            return it != actionCooldowns.end() && it->second > currentTime;
        }

        /**
         * @brief Get remaining cooldown for action
         * @param actionType Type of action
         * @return Remaining cooldown time
         */
        double GetActionCooldown(const std::string& actionType) const {
            auto it = actionCooldowns.find(actionType);
            return (it != actionCooldowns.end()) ? std::max(0.0, it->second - currentTime) : 0.0;
        }

        /**
         * @brief Check if goal is active
         * @param goalType Type of goal
         * @return true if active, false otherwise
         */
        bool IsGoalActive(const std::string& goalType) const {
            return std::find(activeGoals.begin(), activeGoals.end(), goalType) != activeGoals.end();
        }

        /**
         * @brief Check if goal was recently completed
         * @param goalType Type of goal
         * @return true if recently completed, false otherwise
         */
        bool WasGoalRecentlyCompleted(const std::string& goalType) const {
            return std::find(completedGoals.begin(), completedGoals.end(), goalType) != completedGoals.end();
        }
    };

    /**
     * @struct AIAction
     * @brief Represents an action that an AI can perform
     */
    struct AIAction {
        std::string type;                        ///< Action type
        std::string name;                        ///< Action name
        std::string description;                 ///< Action description
        std::function<bool(Mob*, const AIDecisionContext&)> precondition; ///< Action precondition
        std::function<bool(Mob*, const AIDecisionContext&)> effect; ///< Action effect
        std::function<void(Mob*, const AIDecisionContext&)> execution; ///< Action execution

        // Action properties
        float cost;                              ///< Action cost
        float utility;                           ///< Action utility value
        double cooldown;                         ///< Action cooldown
        int priority;                            ///< Action priority
        std::vector<std::string> requiredTags;   ///< Required tags
        std::vector<std::string> effects;        ///< Action effects

        AIAction()
            : cost(1.0f)
            , utility(0.0f)
            , cooldown(0.0)
            , priority(0)
        {}

        /**
         * @brief Check if action can be executed
         * @param mob Mob performing action
         * @param context Decision context
         * @return true if executable, false otherwise
         */
        bool CanExecute(Mob* mob, const AIDecisionContext& context) const {
            if (precondition) {
                return precondition(mob, context);
            }
            return true;
        }

        /**
         * @brief Execute the action
         * @param mob Mob performing action
         * @param context Decision context
         * @return true if successful, false otherwise
         */
        bool Execute(Mob* mob, const AIDecisionContext& context) {
            if (execution) {
                execution(mob, context);
                return true;
            }
            return false;
        }

        /**
         * @brief Get action utility value
         * @param mob Mob performing action
         * @param context Decision context
         * @return Utility value
         */
        float GetUtility(Mob* mob, const AIDecisionContext& context) const {
            return utility;
        }
    };

    /**
     * @struct AIGoal
     * @brief Represents a goal that an AI wants to achieve
     */
    struct AIGoal {
        std::string type;                        ///< Goal type
        std::string name;                        ///< Goal name
        std::string description;                 ///< Goal description
        std::function<bool(Mob*, const AIDecisionContext&)> condition; ///< Goal completion condition
        std::function<float(Mob*, const AIDecisionContext&)> priorityFunction; ///< Goal priority function

        // Goal properties
        float basePriority;                      ///< Base priority value
        float currentPriority;                   ///< Current priority value
        bool isActive;                           ///< Goal is currently active
        double activationTime;                   ///< Time when goal was activated
        double completionTime;                   ///< Time when goal was completed
        std::vector<std::string> requiredActions; ///< Required actions to complete
        std::unordered_map<std::string, std::any> parameters; ///< Goal parameters

        AIGoal()
            : basePriority(0.0f)
            , currentPriority(0.0f)
            , isActive(false)
            , activationTime(0.0)
            , completionTime(0.0)
        {}

        /**
         * @brief Check if goal is completed
         * @param mob Mob with goal
         * @param context Decision context
         * @return true if completed, false otherwise
         */
        bool IsCompleted(Mob* mob, const AIDecisionContext& context) const {
            if (condition) {
                return condition(mob, context);
            }
            return false;
        }

        /**
         * @brief Calculate goal priority
         * @param mob Mob with goal
         * @param context Decision context
         * @return Priority value
         */
        float CalculatePriority(Mob* mob, const AIDecisionContext& context) {
            if (priorityFunction) {
                currentPriority = priorityFunction(mob, context);
            } else {
                currentPriority = basePriority;
            }
            return currentPriority;
        }

        /**
         * @brief Activate the goal
         * @param currentTime Current game time
         */
        void Activate(double currentTime) {
            isActive = true;
            activationTime = currentTime;
        }

        /**
         * @brief Complete the goal
         * @param currentTime Current game time
         */
        void Complete(double currentTime) {
            isActive = false;
            completionTime = currentTime;
        }
    };

    /**
     * @struct AIWorldState
     * @brief Representation of the current world state for AI planning
     */
    struct AIWorldState {
        std::unordered_map<std::string, bool> booleanStates;    ///< Boolean world states
        std::unordered_map<std::string, int> integerStates;     ///< Integer world states
        std::unordered_map<std::string, float> floatStates;     ///< Float world states
        std::unordered_map<std::string, std::string> stringStates; ///< String world states

        /**
         * @brief Set boolean state
         * @param key State key
         * @param value State value
         */
        void SetBool(const std::string& key, bool value) {
            booleanStates[key] = value;
        }

        /**
         * @brief Get boolean state
         * @param key State key
         * @param defaultValue Default value
         * @return State value or default
         */
        bool GetBool(const std::string& key, bool defaultValue = false) const {
            auto it = booleanStates.find(key);
            return (it != booleanStates.end()) ? it->second : defaultValue;
        }

        /**
         * @brief Set integer state
         * @param key State key
         * @param value State value
         */
        void SetInt(const std::string& key, int value) {
            integerStates[key] = value;
        }

        /**
         * @brief Get integer state
         * @param key State key
         * @param defaultValue Default value
         * @return State value or default
         */
        int GetInt(const std::string& key, int defaultValue = 0) const {
            auto it = integerStates.find(key);
            return (it != integerStates.end()) ? it->second : defaultValue;
        }

        /**
         * @brief Calculate distance to another world state
         * @param other Other world state
         * @return Distance measure
         */
        float DistanceTo(const AIWorldState& other) const {
            float distance = 0.0f;
            // Simplified distance calculation - can be expanded
            for (const auto& [key, value] : booleanStates) {
                if (other.booleanStates.count(key) && other.booleanStates.at(key) != value) {
                    distance += 1.0f;
                }
            }
            return distance;
        }

        /**
         * @brief Check if world state satisfies conditions
         * @param conditions Required conditions
         * @return true if satisfied, false otherwise
         */
        bool Satisfies(const std::unordered_map<std::string, bool>& conditions) const {
            for (const auto& [key, requiredValue] : conditions) {
                if (GetBool(key) != requiredValue) {
                    return false;
                }
            }
            return true;
        }

        /**
         * @brief Clear all states
         */
        void Clear() {
            booleanStates.clear();
            integerStates.clear();
            floatStates.clear();
            stringStates.clear();
        }

        /**
         * @brief Get state summary
         * @return Summary string
         */
        std::string GetSummary() const {
            return "Bool: " + std::to_string(booleanStates.size()) +
                   ", Int: " + std::to_string(integerStates.size()) +
                   ", Float: " + std::to_string(floatStates.size()) +
                   ", String: " + std::to_string(stringStates.size());
        }
    };

    // Type aliases for convenience
    using AIVector3Array = std::vector<AIVector3>;
    using AITransformArray = std::vector<AITransform>;
    using AIBoundsArray = std::vector<AIBounds>;
    using AITimerArray = std::vector<AITimer>;
    using AIActionArray = std::vector<AIAction>;
    using AIGoalArray = std::vector<AIGoal>;

    // Constants
    constexpr float AI_DEFAULT_UPDATE_INTERVAL = 1.0f / 30.0f;  // 30 FPS
    constexpr float AI_DEFAULT_SIGHT_RANGE = 16.0f;
    constexpr float AI_DEFAULT_HEARING_RANGE = 8.0f;
    constexpr float AI_DEFAULT_SMELL_RANGE = 4.0f;
    constexpr float AI_DEFAULT_MEMORY_DURATION = 300.0f;        // 5 minutes
    constexpr int AI_DEFAULT_MAX_MEMORIES = 100;
    constexpr float AI_DEFAULT_AGENT_RADIUS = 0.5f;
    constexpr float AI_DEFAULT_AGENT_HEIGHT = 1.8f;

    // Default paths and directories
    constexpr const char* AI_DEFAULT_BEHAVIOR_PATH = "assets/ai/behaviors/";
    constexpr const char* AI_DEFAULT_CONFIG_PATH = "assets/ai/config/";
    constexpr const char* AI_DEFAULT_DATA_PATH = "assets/ai/data/";

} // namespace VoxelCraft

#endif // VOXELCRAFT_AI_AI_TYPES_HPP
