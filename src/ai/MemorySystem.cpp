/**
 * @file MemorySystem.cpp
 * @brief VoxelCraft AI Memory System Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "MemorySystem.hpp"
#include "../entities/Entity.hpp"
#include <algorithm>
#include <cmath>

namespace VoxelCraft {

    // MemoryPattern implementation
    bool MemoryPattern::Matches(const std::vector<MemoryEntry>& memories) const {
        if (memories.size() < minOccurrences) return false;

        int matchCount = 0;

        for (const auto& memory : memories) {
            // Check if memory is within time window
            if (memory.GetAge() > timeWindow) continue;

            // Check if memory type is in required types
            if (std::find(requiredTypes.begin(), requiredTypes.end(), memory.type) != requiredTypes.end()) {
                matchCount++;
            }
        }

        return matchCount >= minOccurrences;
    }

    // MemorySystem implementation
    MemorySystem::MemorySystem(Entity* entity, int maxMemories)
        : m_entity(entity), m_maxMemories(maxMemories) {
    }

    void MemorySystem::Update(float deltaTime) {
        m_cleanupTimer += deltaTime;

        // Update memory strengths
        UpdateMemoryStrengths(deltaTime);

        // Check for patterns
        CheckPatterns();

        // Periodic cleanup
        if (m_cleanupTimer >= m_cleanupInterval) {
            m_cleanupTimer = 0.0f;
            CleanupMemories();
        }
    }

    void MemorySystem::AddMemory(const MemoryEntry& memory) {
        if (m_memories.size() >= m_maxMemories) {
            EnforceMemoryLimit();
        }

        m_memories.push_back(memory);
        m_stats.addedMemories++;
        m_stats.totalMemories++;
        m_stats.memoriesByType[memory.type]++;

        // Sort memories by priority and strength
        SortMemories();
    }

    void MemorySystem::AddMemory(MemoryType type, const glm::vec3& position,
                                MemoryPriority priority, float strength,
                                const std::string& description) {
        MemoryEntry memory(type, priority, position);
        memory.strength = strength;
        memory.description = description;
        memory.decayRate = m_defaultDecayRate;
        memory.relatedEntity = nullptr; // Could be set to player if relevant

        AddMemory(memory);
    }

    void MemorySystem::RemoveMemory(const MemoryEntry& memory) {
        auto it = std::find_if(m_memories.begin(), m_memories.end(),
            [&memory](const MemoryEntry& m) {
                return m.type == memory.type && m.position == memory.position && m.timestamp == memory.timestamp;
            });

        if (it != m_memories.end()) {
            m_stats.memoriesByType[it->type]--;
            m_memories.erase(it);
            m_stats.removedMemories++;
            m_stats.totalMemories--;
        }
    }

    void MemorySystem::ClearMemories() {
        m_memories.clear();
        m_stats.totalMemories = 0;
        m_stats.memoriesByType.clear();
    }

    std::vector<MemoryEntry> MemorySystem::QueryMemories(const MemoryQuery& query) const {
        std::vector<MemoryEntry> results;

        for (const auto& memory : m_memories) {
            if (query.Matches(memory)) {
                results.push_back(memory);
            }
        }

        // Sort by strength (strongest first)
        std::sort(results.begin(), results.end(),
            [](const MemoryEntry& a, const MemoryEntry& b) {
                return a.GetCurrentStrength() > b.GetCurrentStrength();
            });

        return results;
    }

    std::vector<MemoryEntry> MemorySystem::GetMemoriesByType(MemoryType type) const {
        std::vector<MemoryEntry> results;

        for (const auto& memory : m_memories) {
            if (memory.type == type) {
                results.push_back(memory);
            }
        }

        return results;
    }

    std::vector<MemoryEntry> MemorySystem::GetMemoriesNearPosition(const glm::vec3& position, float radius) const {
        std::vector<MemoryEntry> results;

        for (const auto& memory : m_memories) {
            float distance = glm::distance(position, memory.position);
            if (distance <= radius) {
                results.push_back(memory);
            }
        }

        // Sort by distance (closest first)
        std::sort(results.begin(), results.end(),
            [&position](const MemoryEntry& a, const MemoryEntry& b) {
                float distA = glm::distance(position, a.position);
                float distB = glm::distance(position, b.position);
                return distA < distB;
            });

        return results;
    }

    const MemoryEntry* MemorySystem::GetStrongestMemory(MemoryType type) const {
        const MemoryEntry* strongest = nullptr;
        float maxStrength = 0.0f;

        for (const auto& memory : m_memories) {
            if (memory.type == type) {
                float strength = memory.GetCurrentStrength();
                if (strength > maxStrength) {
                    maxStrength = strength;
                    strongest = &memory;
                }
            }
        }

        return strongest;
    }

    const MemoryEntry* MemorySystem::GetMostRecentMemory(MemoryType type) const {
        const MemoryEntry* mostRecent = nullptr;
        auto newestTime = std::chrono::steady_clock::time_point::min();

        for (const auto& memory : m_memories) {
            if (memory.type == type && memory.timestamp > newestTime) {
                newestTime = memory.timestamp;
                mostRecent = &memory;
            }
        }

        return mostRecent;
    }

    void MemorySystem::SetParameters(int maxMemories, float defaultDecayRate, float cleanupInterval) {
        m_maxMemories = maxMemories;
        m_defaultDecayRate = defaultDecayRate;
        m_cleanupInterval = cleanupInterval;

        EnforceMemoryLimit();
    }

    void MemorySystem::AddPattern(const MemoryPattern& pattern) {
        // Check if pattern already exists
        auto it = std::find_if(m_patterns.begin(), m_patterns.end(),
            [&pattern](const MemoryPattern& p) { return p.name == pattern.name; });

        if (it == m_patterns.end()) {
            m_patterns.push_back(pattern);
        }
    }

    void MemorySystem::RemovePattern(const std::string& patternName) {
        m_patterns.erase(std::remove_if(m_patterns.begin(), m_patterns.end(),
            [&patternName](const MemoryPattern& p) { return p.name == patternName; }), m_patterns.end());
    }

    void MemorySystem::CleanupMemories() {
        size_t beforeSize = m_memories.size();

        // Remove expired memories (older than 10 minutes by default)
        m_memories.erase(std::remove_if(m_memories.begin(), m_memories.end(),
            [](const MemoryEntry& memory) {
                return memory.IsExpired(600.0f); // 10 minutes
            }), m_memories.end());

        // Remove very weak memories
        m_memories.erase(std::remove_if(m_memories.begin(), m_memories.end(),
            [](const MemoryEntry& memory) {
                return memory.GetCurrentStrength() < 0.05f;
            }), m_memories.end());

        size_t removed = beforeSize - m_memories.size();
        m_stats.expiredMemories += removed;
        m_stats.totalMemories -= removed;

        // Update type counts
        std::unordered_map<MemoryType, int> newCounts;
        for (const auto& memory : m_memories) {
            newCounts[memory.type]++;
        }
        m_stats.memoriesByType = newCounts;
    }

    void MemorySystem::UpdateMemoryStrengths(float deltaTime) {
        int decayedCount = 0;

        for (auto& memory : m_memories) {
            float oldStrength = memory.GetCurrentStrength();
            memory.strength *= std::exp(-memory.decayRate * deltaTime);
            float newStrength = memory.GetCurrentStrength();

            if (oldStrength >= 0.1f && newStrength < 0.1f) {
                decayedCount++;
            }
        }

        m_stats.decayedMemories += decayedCount;

        // Update average strength
        if (!m_memories.empty()) {
            float totalStrength = 0.0f;
            float totalAge = 0.0f;

            for (const auto& memory : m_memories) {
                totalStrength += memory.GetCurrentStrength();
                totalAge += memory.GetAge();
            }

            m_stats.averageMemoryStrength = totalStrength / m_memories.size();
            m_stats.averageMemoryAge = totalAge / m_memories.size();
        }
    }

    void MemorySystem::CheckPatterns() {
        for (const auto& pattern : m_patterns) {
            if (pattern.Matches(m_memories)) {
                m_stats.patternMatches++;

                if (pattern.callback) {
                    // Find matching memories
                    std::vector<MemoryEntry> matchingMemories;
                    for (const auto& memory : m_memories) {
                        if (memory.GetAge() <= pattern.timeWindow &&
                            std::find(pattern.requiredTypes.begin(), pattern.requiredTypes.end(), memory.type) != pattern.requiredTypes.end()) {
                            matchingMemories.push_back(memory);
                        }
                    }

                    pattern.callback(matchingMemories);
                }
            }
        }
    }

    void MemorySystem::SortMemories() {
        std::sort(m_memories.begin(), m_memories.end(),
            [](const MemoryEntry& a, const MemoryEntry& b) {
                // Sort by priority first, then by strength
                if (a.priority != b.priority) {
                    return static_cast<int>(a.priority) > static_cast<int>(b.priority);
                }
                return a.GetCurrentStrength() > b.GetCurrentStrength();
            });
    }

    void MemorySystem::EnforceMemoryLimit() {
        if (m_memories.size() >= m_maxMemories) {
            // Remove weakest memories
            std::sort(m_memories.begin(), m_memories.end(),
                [](const MemoryEntry& a, const MemoryEntry& b) {
                    return a.GetCurrentStrength() < b.GetCurrentStrength();
                });

            size_t toRemove = m_memories.size() - m_maxMemories + 1;
            m_memories.erase(m_memories.begin(), m_memories.begin() + toRemove);
            m_stats.removedMemories += toRemove;
            m_stats.totalMemories -= toRemove;
        }
    }

    // MemoryManager implementation
    MemoryManager& MemoryManager::GetInstance() {
        static MemoryManager instance;
        return instance;
    }

    void MemoryManager::RegisterEntity(Entity* entity, std::unique_ptr<MemorySystem> system) {
        if (entity && system) {
            m_memorySystems[entity] = std::move(system);
        }
    }

    void MemoryManager::UnregisterEntity(Entity* entity) {
        if (entity) {
            m_memorySystems.erase(entity);
        }
    }

    void MemoryManager::Update(float deltaTime) {
        for (auto& pair : m_memorySystems) {
            if (pair.second) {
                pair.second->Update(deltaTime);
            }
        }
    }

    MemorySystem* MemoryManager::GetMemorySystem(Entity* entity) const {
        auto it = m_memorySystems.find(entity);
        return (it != m_memorySystems.end()) ? it->second.get() : nullptr;
    }

    void MemoryManager::BroadcastMemory(const MemoryEntry& memory, Entity* sourceEntity, float range) {
        for (auto& pair : m_memorySystems) {
            // Don't send memory to source entity
            if (pair.first == sourceEntity) continue;

            if (pair.second && pair.first) {
                // Check if entity is within range
                float distance = glm::distance(pair.first->GetPosition(), sourceEntity->GetPosition());
                if (distance <= range) {
                    pair.second->AddMemory(memory);
                }
            }
        }
    }

} // namespace VoxelCraft
