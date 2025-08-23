/**
 * @file PerceptionSystem.cpp
 * @brief VoxelCraft AI Perception System Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "PerceptionSystem.hpp"
#include "../entities/Entity.hpp"
#include "../player/Player.hpp"
#include "../world/World.hpp"
#include <algorithm>
#include <cmath>

namespace VoxelCraft {

    // PerceptionSensor implementation
    PerceptionSensor::PerceptionSensor(PerceptionType type, float range, Entity* entity)
        : m_type(type), m_range(range), m_entity(entity) {
    }

    // SightSensor implementation
    SightSensor::SightSensor(float range, Entity* entity)
        : PerceptionSensor(PerceptionType::SIGHT, range, entity) {
    }

    void SightSensor::Update(float deltaTime) {
        if (!m_enabled) return;

        m_updateTimer += deltaTime;
        if (m_updateTimer >= m_updateInterval) {
            m_updateTimer = 0.0f;
            // Update sight processing
        }
    }

    PerceptionResult SightSensor::CanPerceive(const PerceptionStimulus& stimulus) {
        if (!m_enabled || !m_entity) {
            return PerceptionResult(false, 0.0f, 0.0f, glm::vec3(0.0f));
        }

        PerceptionResult result;

        // Calculate distance
        glm::vec3 entityPos = m_entity->GetPosition();
        result.distance = glm::distance(entityPos, stimulus.position);

        // Check if within range
        if (result.distance > m_sightDistance) {
            return result;
        }

        // Check if in field of view
        if (!IsInFieldOfView(stimulus.position)) {
            return result;
        }

        // Check line of sight
        if (!HasLineOfSight(stimulus.position)) {
            result.confidence = 0.1f; // Partial detection through obstacles
            return result;
        }

        // Calculate direction
        result.direction = glm::normalize(stimulus.position - entityPos);

        // Calculate confidence based on distance and angle
        float distanceFactor = 1.0f - (result.distance / m_sightDistance);
        float angleFactor = 1.0f; // Could be improved with actual angle calculation

        result.confidence = (distanceFactor + angleFactor) * 0.5f;
        result.detected = result.confidence > 0.3f; // Detection threshold

        if (result.detected) {
            result.stimulus = stimulus;
        }

        return result;
    }

    bool SightSensor::HasLineOfSight(const glm::vec3& targetPos) const {
        if (!m_entity) return false;

        glm::vec3 startPos = m_entity->GetPosition() + glm::vec3(0.0f, m_eyeHeight, 0.0f);
        glm::vec3 direction = targetPos - startPos;
        float distance = glm::length(direction);

        if (distance < 0.1f) return true;

        direction = glm::normalize(direction);
        const float stepSize = 0.5f;
        int steps = static_cast<int>(distance / stepSize);

        // Raycast through blocks
        for (int i = 1; i < steps; ++i) {
            glm::vec3 checkPos = startPos + direction * (i * stepSize);

            // Check if block at position is solid (simplified)
            // In a real implementation, this would query the world for block solidity
            if (false) { // Placeholder for actual block check
                return false;
            }
        }

        return true;
    }

    bool SightSensor::IsInFieldOfView(const glm::vec3& targetPos) const {
        if (!m_entity) return false;

        glm::vec3 viewDir = GetViewDirection();
        glm::vec3 toTarget = glm::normalize(targetPos - (m_entity->GetPosition() + glm::vec3(0.0f, m_eyeHeight, 0.0f)));

        float angle = glm::degrees(glm::acos(glm::dot(viewDir, toTarget)));
        return angle <= (m_fieldOfView * 0.5f);
    }

    glm::vec3 SightSensor::GetViewDirection() const {
        if (!m_entity) return glm::vec3(0.0f, 0.0f, 1.0f);

        // Get entity rotation and convert to direction vector
        glm::vec3 rotation = m_entity->GetRotation();
        float yaw = glm::radians(rotation.y);
        float pitch = glm::radians(rotation.x);

        return glm::normalize(glm::vec3(
            -glm::sin(yaw) * glm::cos(pitch),
            -glm::sin(pitch),
            glm::cos(yaw) * glm::cos(pitch)
        ));
    }

    // HearingSensor implementation
    HearingSensor::HearingSensor(float range, Entity* entity)
        : PerceptionSensor(PerceptionType::HEARING, range, entity) {
    }

    void HearingSensor::Update(float deltaTime) {
        if (!m_enabled) return;

        m_updateTimer += deltaTime;
        if (m_updateTimer >= m_updateInterval) {
            m_updateTimer = 0.0f;
            // Update hearing processing
        }
    }

    PerceptionResult HearingSensor::CanPerceive(const PerceptionStimulus& stimulus) {
        if (!m_enabled || !m_entity) {
            return PerceptionResult(false, 0.0f, 0.0f, glm::vec3(0.0f));
        }

        PerceptionResult result;

        // Calculate distance
        glm::vec3 entityPos = m_entity->GetPosition();
        result.distance = glm::distance(entityPos, stimulus.position);

        // Check if within range
        if (result.distance > m_range) {
            return result;
        }

        // Calculate direction
        result.direction = glm::normalize(stimulus.position - entityPos);

        // Calculate attenuation
        float attenuation = CalculateAttenuation(result.distance, stimulus.type);

        // Calculate confidence
        result.confidence = stimulus.intensity * attenuation * m_sensitivity;
        result.detected = result.confidence > 0.2f; // Hearing threshold

        if (result.detected) {
            result.stimulus = stimulus;
        }

        return result;
    }

    float HearingSensor::CalculateAttenuation(float distance, StimulusType stimulusType) const {
        // Base attenuation with distance
        float attenuation = 1.0f / (1.0f + distance * 0.1f);

        // Different stimulus types have different attenuation characteristics
        switch (stimulusType) {
            case StimulusType::PLAYER_SOUND:
                attenuation *= 1.0f; // Normal sound
                break;
            case StimulusType::BLOCK_BREAK:
                attenuation *= 1.2f; // Loud sound
                break;
            case StimulusType::EXPLOSION:
                attenuation *= 2.0f; // Very loud sound
                break;
            case StimulusType::PROJECTILE:
                attenuation *= 0.8f; // Quiet sound
                break;
            default:
                attenuation *= 1.0f;
                break;
        }

        return attenuation;
    }

    // PerceptionSystem implementation
    PerceptionSystem::PerceptionSystem(Entity* entity)
        : m_entity(entity) {
        // Add default sensors
        AddSensor(std::make_unique<SightSensor>(16.0f, entity));
        AddSensor(std::make_unique<HearingSensor>(24.0f, entity));
    }

    PerceptionSystem::~PerceptionSystem() {
        // Cleanup sensors
    }

    void PerceptionSystem::Update(float deltaTime) {
        if (!m_enabled) return;

        // Update all sensors
        for (auto& pair : m_sensors) {
            if (pair.second) {
                pair.second->Update(deltaTime);
            }
        }

        // Update memory
        UpdateMemory(deltaTime);
    }

    void PerceptionSystem::AddSensor(std::unique_ptr<PerceptionSensor> sensor) {
        if (sensor) {
            m_sensors[sensor->GetType()] = std::move(sensor);
        }
    }

    void PerceptionSystem::RemoveSensor(PerceptionType type) {
        m_sensors.erase(type);
    }

    PerceptionSensor* PerceptionSystem::GetSensor(PerceptionType type) const {
        auto it = m_sensors.find(type);
        return (it != m_sensors.end()) ? it->second.get() : nullptr;
    }

    PerceptionResult PerceptionSystem::CanPerceive(const PerceptionStimulus& stimulus) {
        PerceptionResult bestResult;

        for (auto& pair : m_sensors) {
            if (pair.second && pair.second->IsEnabled()) {
                PerceptionResult result = pair.second->CanPerceive(stimulus);

                if (result.detected && result.confidence > bestResult.confidence) {
                    bestResult = result;
                }
            }
        }

        // Update statistics
        m_stats.totalStimuli++;
        if (bestResult.detected) {
            m_stats.detectedStimuli++;
            m_stats.detectionsByType[PerceptionType::SIGHT]++; // Simplified
        } else {
            m_stats.missedStimuli++;
        }

        // Remember stimulus if detected
        if (bestResult.detected) {
            RememberStimulus(stimulus);
        }

        return bestResult;
    }

    std::vector<PerceptionResult> PerceptionSystem::GetAllPerceptionResults(const PerceptionStimulus& stimulus) {
        std::vector<PerceptionResult> results;

        for (auto& pair : m_sensors) {
            if (pair.second && pair.second->IsEnabled()) {
                results.push_back(pair.second->CanPerceive(stimulus));
            }
        }

        return results;
    }

    void PerceptionSystem::SetEnabled(bool enabled) {
        m_enabled = enabled;
    }

    void PerceptionSystem::ClearMemory() {
        m_memory.clear();
        m_stats.memorySize = 0;
    }

    void PerceptionSystem::UpdateMemory(float deltaTime) {
        auto now = std::chrono::steady_clock::now();

        // Remove old stimuli
        m_memory.erase(std::remove_if(m_memory.begin(), m_memory.end(),
            [this, now](const PerceptionStimulus& stimulus) {
                auto age = std::chrono::duration<float>(now - stimulus.timestamp).count();
                return age > m_memoryDuration;
            }), m_memory.end());

        m_stats.memorySize = m_memory.size();
    }

    void PerceptionSystem::RememberStimulus(const PerceptionStimulus& stimulus) {
        if (m_memory.size() >= m_stats.maxMemorySize) {
            // Remove oldest stimulus
            m_memory.erase(m_memory.begin());
        }

        m_memory.push_back(stimulus);
    }

    // PerceptionManager implementation
    PerceptionManager& PerceptionManager::GetInstance() {
        static PerceptionManager instance;
        return instance;
    }

    void PerceptionManager::RegisterEntity(Entity* entity, std::unique_ptr<PerceptionSystem> system) {
        if (entity && system) {
            m_perceptionSystems[entity] = std::move(system);
        }
    }

    void PerceptionManager::UnregisterEntity(Entity* entity) {
        if (entity) {
            m_perceptionSystems.erase(entity);
        }
    }

    void PerceptionManager::BroadcastStimulus(const PerceptionStimulus& stimulus, Entity* sourceEntity) {
        for (auto& pair : m_perceptionSystems) {
            // Don't send stimulus to source entity
            if (pair.first == sourceEntity) continue;

            if (pair.second && pair.second->IsEnabled()) {
                pair.second->CanPerceive(stimulus);
            }
        }
    }

    void PerceptionManager::Update(float deltaTime) {
        for (auto& pair : m_perceptionSystems) {
            if (pair.second) {
                pair.second->Update(deltaTime);
            }
        }
    }

    PerceptionSystem* PerceptionManager::GetPerceptionSystem(Entity* entity) const {
        auto it = m_perceptionSystems.find(entity);
        return (it != m_perceptionSystems.end()) ? it->second.get() : nullptr;
    }

} // namespace VoxelCraft
