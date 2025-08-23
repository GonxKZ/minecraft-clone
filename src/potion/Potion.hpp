/**
 * @file Potion.hpp
 * @brief VoxelCraft Potion System - Potions, Effects, and Brewing
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_POTION_POTION_HPP
#define VOXELCRAFT_POTION_POTION_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <chrono>
#include <glm/glm.hpp>

namespace VoxelCraft {

    class Entity;
    class Player;
    class World;

    /**
     * @enum PotionType
     * @brief Types of potions available
     */
    enum class PotionType {
        WATER_BOTTLE = 0,      ///< Plain water bottle
        MUNDANE,               ///< Mundane potion
        THICK,                 ///< Thick potion
        AWKWARD,               ///< Awkward potion
        NIGHT_VISION,          ///< Night vision
        INVISIBILITY,          ///< Invisibility
        LEAPING,               ///< Jump boost
        FIRE_RESISTANCE,      ///< Fire resistance
        SWIFTNESS,             ///< Speed
        SLOWNESS,              ///< Slowness
        WATER_BREATHING,      ///< Water breathing
        HEALING,               ///< Instant health
        HARMING,               ///< Instant damage
        POISON,                ///< Poison
        REGENERATION,          ///< Regeneration
        STRENGTH,              ///< Strength
        WEAKNESS,              ///< Weakness
        LUCK,                  ///< Luck
        BAD_LUCK,              ///< Bad luck
        SLOW_FALLING,          ///< Slow falling
        TURTLE_MASTER          ///< Turtle master
    };

    /**
     * @enum PotionEffectType
     * @brief Types of potion effects
     */
    enum class PotionEffectType {
        SPEED = 0,             ///< Movement speed
        SLOWNESS,              ///< Movement slowness
        HASTE,                 ///< Mining speed
        MINING_FATIGUE,        ///< Mining slowness
        STRENGTH,              ///< Attack damage
        INSTANT_HEALTH,        ///< Instant health
        INSTANT_DAMAGE,        ///< Instant damage
        JUMP_BOOST,            ///< Jump height
        NAUSEA,                ///< Nausea
        REGENERATION,          ///< Health regeneration
        RESISTANCE,            ///< Damage resistance
        FIRE_RESISTANCE,      ///< Fire damage immunity
        WATER_BREATHING,      ///< Underwater breathing
        INVISIBILITY,          ///< Invisibility
        BLINDNESS,             ///< Blindness
        NIGHT_VISION,          ///< Night vision
        HUNGER,                ///< Hunger increase
        WEAKNESS,              ///< Attack damage reduction
        POISON,                ///< Poison damage
        WITHER,                ///< Wither damage
        HEALTH_BOOST,          ///< Max health increase
        ABSORPTION,            ///< Absorption hearts
        SATURATION,            ///< Saturation (food)
        GLOWING,               ///< Glowing effect
        LEVITATION,            ///< Levitation
        LUCK,                  ///< Luck
        BAD_LUCK,              ///< Bad luck
        SLOW_FALLING,          ///< Slow falling
        CONDUIT_POWER,         ///< Conduit power
        DOLPHINS_GRACE         ///< Dolphin's grace
    };

    /**
     * @enum PotionRarity
     * @brief Rarity levels for potions
     */
    enum class PotionRarity {
        COMMON = 0,            ///< Common potions
        UNCOMMON,              ///< Uncommon potions
        RARE,                  ///< Rare potions
        EPIC                   ///< Epic potions
    };

    /**
     * @struct PotionEffect
     * @brief Single potion effect with its properties
     */
    struct PotionEffect {
        PotionEffectType type;
        int amplifier;                         ///< Effect amplifier (0-255)
        int duration;                          ///< Effect duration in ticks (20 ticks = 1 second)
        bool isAmbient;                        ///< Whether effect is ambient
        bool showParticles;                    ///< Whether to show particles
        bool showIcon;                         ///< Whether to show effect icon
        std::chrono::steady_clock::time_point startTime;
        std::function<void(Entity*, int, int)> onApply;    ///< Called when effect is applied
        std::function<void(Entity*, int, int)> onTick;     ///< Called every tick
        std::function<void(Entity*, int, int)> onExpire;   ///< Called when effect expires

        PotionEffect()
            : amplifier(0)
            , duration(0)
            , isAmbient(false)
            , showParticles(true)
            , showIcon(true)
            , startTime(std::chrono::steady_clock::now())
        {}

        /**
         * @brief Check if effect is expired
         * @return true if expired
         */
        bool IsExpired() const {
            auto now = std::chrono::steady_clock::now();
            auto elapsedTicks = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - startTime).count() / 50; // 50ms per tick
            return elapsedTicks >= duration;
        }

        /**
         * @brief Get remaining duration in ticks
         * @return Remaining ticks
         */
        int GetRemainingTicks() const {
            auto now = std::chrono::steady_clock::now();
            auto elapsedTicks = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - startTime).count() / 50;
            return std::max(0, duration - static_cast<int>(elapsedTicks));
        }
    };

    /**
     * @struct PotionDefinition
     * @brief Complete definition of a potion
     */
    struct PotionDefinition {
        PotionType type;
        std::string name;
        std::string displayName;
        std::string description;
        PotionRarity rarity;
        int itemID;                            ///< Item ID for the potion
        int maxUses;                           ///< Maximum uses before empty
        std::vector<PotionEffect> baseEffects; ///< Base effects of the potion
        bool isSplash;                         ///< Whether potion is splash variant
        bool isLingering;                      ///< Whether potion is lingering variant
        int color;                             ///< Potion color (RGB)
        std::string textureName;               ///< Potion texture name
        std::unordered_map<std::string, std::any> customProperties;
    };

    /**
     * @struct PotionInstance
     * @brief Instance of a potion with current state
     */
    struct PotionInstance {
        PotionType type;
        int usesRemaining;
        std::chrono::steady_clock::time_point createdTime;
        std::vector<PotionEffect> activeEffects;
        std::unordered_map<std::string, float> modifiers;

        PotionInstance() : usesRemaining(1), createdTime(std::chrono::steady_clock::now()) {}

        /**
         * @brief Check if potion is empty
         * @return true if empty
         */
        bool IsEmpty() const { return usesRemaining <= 0; }

        /**
         * @brief Get potion age in seconds
         * @return Age in seconds
         */
        float GetAge() const {
            auto now = std::chrono::steady_clock::now();
            return std::chrono::duration<float>(now - createdTime).count();
        }
    };

    /**
     * @class PotionEffectManager
     * @brief Manager for potion effects on entities
     */
    class PotionEffectManager {
    public:
        /**
         * @brief Apply effect to entity
         * @param entity Target entity
         * @param effect Effect to apply
         * @return true if applied successfully
         */
        static bool ApplyEffect(Entity* entity, const PotionEffect& effect);

        /**
         * @brief Remove effect from entity
         * @param entity Target entity
         * @param effectType Effect type to remove
         * @return true if removed successfully
         */
        static bool RemoveEffect(Entity* entity, PotionEffectType effectType);

        /**
         * @brief Check if entity has effect
         * @param entity Entity to check
         * @param effectType Effect type to check
         * @return true if has effect
         */
        static bool HasEffect(Entity* entity, PotionEffectType effectType);

        /**
         * @brief Get effect amplifier
         * @param entity Entity to check
         * @param effectType Effect type
         * @return Amplifier level or 0 if not found
         */
        static int GetEffectAmplifier(Entity* entity, PotionEffectType effectType);

        /**
         * @brief Get effect duration
         * @param entity Entity to check
         * @param effectType Effect type
         * @return Remaining duration in ticks or 0 if not found
         */
        static int GetEffectDuration(Entity* entity, PotionEffectType effectType);

        /**
         * @brief Update all effects on entity
         * @param entity Entity to update
         * @param deltaTime Time since last update
         */
        static void UpdateEntityEffects(Entity* entity, float deltaTime);

        /**
         * @brief Clear all effects from entity
         * @param entity Entity to clear
         */
        static void ClearAllEffects(Entity* entity);
    };

    /**
     * @class Potion
     * @brief Base class for all potions
     */
    class Potion {
    public:
        /**
         * @brief Constructor
         * @param definition Potion definition
         */
        Potion(const PotionDefinition& definition);

        /**
         * @brief Destructor
         */
        virtual ~Potion() = default;

        /**
         * @brief Get potion type
         * @return Potion type
         */
        PotionType GetType() const { return m_definition.type; }

        /**
         * @brief Get potion name
         * @return Potion name
         */
        const std::string& GetName() const { return m_definition.name; }

        /**
         * @brief Get potion definition
         * @return Potion definition
         */
        const PotionDefinition& GetDefinition() const { return m_definition; }

        /**
         * @brief Use potion on entity
         * @param entity Target entity
         * @param instance Potion instance
         * @return true if used successfully
         */
        virtual bool UsePotion(Entity* entity, PotionInstance& instance);

        /**
         * @brief Throw potion as splash
         * @param thrower Entity throwing the potion
         * @param position Throw position
         * @param direction Throw direction
         * @param instance Potion instance
         * @return true if thrown successfully
         */
        virtual bool ThrowSplashPotion(Entity* thrower, const glm::vec3& position,
                                     const glm::vec3& direction, PotionInstance& instance);

        /**
         * @brief Create lingering cloud
         * @param position Cloud position
         * @param instance Potion instance
         * @return true if created successfully
         */
        virtual bool CreateLingeringCloud(const glm::vec3& position, PotionInstance& instance);

        /**
         * @brief Get potion effects
         * @param instance Potion instance
         * @return Vector of potion effects
         */
        virtual std::vector<PotionEffect> GetPotionEffects(const PotionInstance& instance) const;

        /**
         * @brief Check if potion can be brewed
         * @param ingredients Available ingredients
         * @return true if can be brewed
         */
        virtual bool CanBrew(const std::vector<int>& ingredients) const;

        /**
         * @brief Get required ingredients for brewing
         * @return Vector of required ingredient item IDs
         */
        virtual std::vector<int> GetBrewingIngredients() const;

    protected:
        PotionDefinition m_definition;

        /**
         * @brief Apply potion effects to entity
         * @param entity Target entity
         * @param effects Effects to apply
         */
        void ApplyPotionEffects(Entity* entity, const std::vector<PotionEffect>& effects);

        /**
         * @brief Create splash potion entity
         * @param thrower Entity throwing
         * @param position Throw position
         * @param direction Throw direction
         * @param instance Potion instance
         * @return true if created successfully
         */
        bool CreateSplashPotionEntity(Entity* thrower, const glm::vec3& position,
                                    const glm::vec3& direction, PotionInstance& instance);
    };

    /**
     * @class HealingPotion
     * @brief Healing potion implementation
     */
    class HealingPotion : public Potion {
    public:
        HealingPotion();

        bool UsePotion(Entity* entity, PotionInstance& instance) override;
    };

    /**
     * @class SpeedPotion
     * @brief Speed potion implementation
     */
    class SpeedPotion : public Potion {
    public:
        SpeedPotion();

        bool UsePotion(Entity* entity, PotionInstance& instance) override;
    };

    /**
     * @class StrengthPotion
     * @brief Strength potion implementation
     */
    class StrengthPotion : public Potion {
    public:
        StrengthPotion();

        bool UsePotion(Entity* entity, PotionInstance& instance) override;
    };

    /**
     * @class PotionManager
     * @brief Manager for all potions and effects
     */
    class PotionManager {
    public:
        /**
         * @brief Get singleton instance
         * @return PotionManager instance
         */
        static PotionManager& GetInstance();

        /**
         * @brief Initialize potion manager
         * @return true if successful
         */
        bool Initialize();

        /**
         * @brief Shutdown potion manager
         */
        void Shutdown();

        /**
         * @brief Register potion
         * @param potion Potion to register
         */
        void RegisterPotion(std::shared_ptr<Potion> potion);

        /**
         * @brief Get potion by type
         * @param type Potion type
         * @return Potion instance or nullptr
         */
        std::shared_ptr<Potion> GetPotion(PotionType type) const;

        /**
         * @brief Get all available potions
         * @return Vector of all potions
         */
        std::vector<std::shared_ptr<Potion>> GetAllPotions() const;

        /**
         * @brief Create potion instance
         * @param type Potion type
         * @param isSplash Whether potion is splash variant
         * @param isLingering Whether potion is lingering variant
         * @return Potion instance
         */
        PotionInstance CreatePotionInstance(PotionType type, bool isSplash = false, bool isLingering = false);

        /**
         * @brief Brew potion from ingredients
         * @param ingredients Vector of ingredient item IDs
         * @return Brewed potion type or WATER_BOTTLE if failed
         */
        PotionType BrewPotion(const std::vector<int>& ingredients);

        /**
         * @brief Get brewing recipes
         * @return Map of ingredient combinations to resulting potions
         */
        const std::unordered_map<std::string, PotionType>& GetBrewingRecipes() const;

        /**
         * @brief Update all active potion effects
         * @param deltaTime Time since last update
         */
        void UpdatePotionEffects(float deltaTime);

        /**
         * @brief Get potion statistics
         * @return Potion statistics
         */
        const PotionStats& GetStats() const { return m_stats; }

    private:
        PotionManager() = default;
        ~PotionManager() = default;

        // Prevent copying
        PotionManager(const PotionManager&) = delete;
        PotionManager& operator=(const PotionManager&) = delete;

        std::unordered_map<PotionType, std::shared_ptr<Potion>> m_potions;
        std::unordered_map<std::string, PotionType> m_brewingRecipes;
        PotionStats m_stats;
        bool m_initialized = false;

        void RegisterDefaultPotions();
        void RegisterBrewingRecipes();
    };

    /**
     * @struct PotionStats
     * @brief Statistics for potion system
     */
    struct PotionStats {
        int totalPotions = 0;
        int potionsBrewed = 0;
        int potionsUsed = 0;
        int splashPotionsThrown = 0;
        int effectsApplied = 0;
        int effectsExpired = 0;
        float totalBrewingTime = 0.0f;
        std::unordered_map<PotionType, int> potionsByType;
        std::unordered_map<PotionEffectType, int> effectsByType;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_POTION_POTION_HPP
