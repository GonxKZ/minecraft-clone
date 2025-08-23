/**
 * @file Mob.hpp
 * @brief VoxelCraft Mob System - Mobs, AI, and Behaviors
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_MOB_MOB_HPP
#define VOXELCRAFT_MOB_MOB_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <chrono>
#include <array>
#include <glm/glm.hpp>

#include "../entities/Entity.hpp"

namespace VoxelCraft {

    class World;
    class Player;
    class Pathfinding;

    /**
     * @enum MobType
     * @brief Types of mobs available in the game
     */
    enum class MobType {
        // Hostile Mobs
        CREEPER = 0,              ///< Exploding mob
        ZOMBIE,                   ///< Basic undead
        SKELETON,                 ///< Ranged undead
        SPIDER,                   ///< Climbing mob
        ENDERMAN,                 ///< Teleporting mob
        BLAZE,                    ///< Fire mob
        GHAST,                    ///< Flying ghost
        WITHER_SKELETON,         ///< Wither undead
        WITCH,                    ///< Potion-using mob
        PHANTOM,                  ///< Flying nightmare
        VEX,                      ///< Flying summon
        PILLAGER,                 ///< Crossbow-using mob
        RAVAGER,                  ///< Beast mount
        VINDICATOR,               ///< Axe-wielding mob
        EVOKER,                   ///< Spell-casting mob
        ILLUSIONER,               ///< Illusion-casting mob

        // Neutral Mobs
        IRON_GOLEM,               ///< Village protector
        SNOW_GOLEM,               ///< Snow-throwing mob
        WOLF,                     ///< Tamable canine
        OCELOT,                   ///< Tamable feline
        LLAMA,                    ///< Spitting mount
        TRADER_LLAMA,             ///< Merchant's llama
        PANDA,                    ///< Playful bear
        FOX,                      ///< Sneaky canine
        BEE,                      ///< Pollinating insect
        TURTLE,                   ///< Slow aquatic mob
        POLAR_BEAR,               ///< Arctic bear
        DOLPHIN,                  ///< Intelligent aquatic
        PUFFERFISH,               ///< Defensive fish
        TROPICAL_FISH,            ///< Colorful fish
        COD,                      ///< Basic fish
        SALMON,                   ///< Jumping fish

        // Passive Mobs
        CHICKEN,                  ///< Egg-laying bird
        COW,                      ///< Milk-giving bovine
        PIG,                      ///< Tamable pork
        SHEEP,                    ///< Wool-giving ovine
        RABBIT,                   ///< Jumping lagomorph
        HORSE,                    ///< Tamable mount
        DONKEY,                   ///< Storage mount
        MULE,                     ///< Hybrid mount
        SKELETON_HORSE,           ///< Undead mount
        ZOMBIE_HORSE,             ///< Undead mount
        PARROT,                   ///< Colorful bird
        CAT,                      ///< Tamable feline
        BAT,                      ///< Flying mammal

        // Nether Mobs
        PIGLIN,                   ///< Gold-loving mob
        PIGLIN_BRUTE,             ///< Armored piglin
        ZOGLIN,                   ///< Zombie-piglin hybrid
        HOGLIN,                   ///< Warthog-like mob
        STRIDER,                  ///< Lava-walking mount
        MAGMA_CUBE,               ///< Slime-like fire mob

        // End Mobs
        ENDERMITE,                ///< Small enderman
        SHULKER,                  ///< Levitating box

        // Boss Mobs
        WITHER,                   ///< Three-headed boss
        ENDER_DRAGON,             ///< Final boss
        ELDER_GUARDIAN,           ///< Ocean boss

        // Special Mobs
        VILLAGER,                 ///< Trading NPC
        WANDERING_TRADER,         ///< Mobile merchant
        ARMOR_STAND,              ///< Poseable armor
        ITEM_FRAME,               ///< Item display
        GLOW_ITEM_FRAME,          ///< Glowing item display
        PAINTING,                 ///< Wall art
        LEASH_KNOT,               ///< Leash attachment
        BOAT,                     ///< Water vehicle
        MINECART,                 ///< Rail vehicle
        CHEST_MINECART,           ///< Storage vehicle
        FURNACE_MINECART,         ///< Powered vehicle
        TNT_MINECART,             ///< Explosive vehicle
        HOPPER_MINECART,          ///< Collection vehicle
        COMMAND_BLOCK_MINECART,   ///< Command vehicle

        MAX_MOB_TYPES
    };

    /**
     * @enum MobBehavior
     * @brief AI behavior types for mobs
     */
    enum class MobBehavior {
        PASSIVE = 0,              ///< Does not attack
        NEUTRAL,                  ///< Attacks when provoked
        HOSTILE,                  ///< Always attacks
        TAMABLE,                  ///< Can be tamed
        MOUNTABLE,                ///< Can be mounted
        FLYING,                   ///< Can fly
        SWIMMING,                 ///< Can swim
        CLIMBING,                 ///< Can climb walls
        TELEPORTING,              ///< Can teleport
        EXPLOSIVE,                ///< Can explode
        RANGED,                   ///< Uses ranged attacks
        MELEE,                    ///< Uses melee attacks
        HEALING,                  ///< Can heal itself/allies
        SUMMONING,                ///< Can summon other mobs
        INVISIBLE,                ///< Can become invisible
        BURNING,                  ///< Takes fire damage
        FREEZING,                 ///< Takes freeze damage
        POISONOUS,                ///< Can poison targets
        WEBBING,                  ///< Can create webs
        STEALING,                 ///< Can steal items
        GUARDING,                 ///< Guards areas
        TRADING,                  ///< Can trade with players
        COLLECTING,               ///< Collects items
        FARMING,                  ///< Can farm crops
        BUILDING,                 ///< Can build structures
        DESTROYING,               ///< Can destroy blocks
        LIGHT_SENSITIVE,          ///< Burns in light
        DARKNESS_DWELLING,        ///< Prefers darkness
        PACK_HUNTING,             ///< Hunts in packs
        SOLITARY,                 ///< Hunts alone
        NOCTURNAL,                ///< Active at night
        DIURNAL                   ///< Active during day
    };

    /**
     * @enum MobState
     * @brief Current state of a mob
     */
    enum class MobState {
        IDLE = 0,                 ///< Doing nothing
        WANDERING,                ///< Moving randomly
        FOLLOWING,                ///< Following a target
        FLEEING,                  ///< Fleeing from danger
        ATTACKING,                ///< Attacking a target
        DEFENDING,                ///< Defending itself
        EATING,                   ///< Eating food
        SLEEPING,                 ///< Sleeping
        MATING,                   ///< Looking for mate
        TAMING,                   ///< Being tamed
        MOUNTING,                 ///< Being mounted
        RIDING,                   ///< Riding something
        DYING,                    ///< Dying animation
        DEAD,                     ///< Dead
        STUNNED,                  ///< Temporarily stunned
        BURNING,                  ///< On fire
        FROZEN,                   ///< Frozen in place
        POISONED,                 ///< Poisoned
        HEALING,                  ///< Healing itself
        SUMMONING,                ///< Summoning help
        TELEPORTING,              ///< Teleporting
        INVISIBLE,                ///< Invisible
        TRANSFORMING              ///< Transforming into something
    };

    /**
     * @struct MobAttributes
     * @brief Physical and behavioral attributes of a mob
     */
    struct MobAttributes {
        float health = 20.0f;             ///< Maximum health
        float speed = 0.2f;               ///< Movement speed
        float attackDamage = 2.0f;        ///< Base attack damage
        float attackSpeed = 1.0f;         ///< Attacks per second
        float attackRange = 2.0f;         ///< Attack range
        float followRange = 16.0f;        ///< Distance to follow targets
        float viewRange = 16.0f;          ///< Distance to see targets
        float jumpHeight = 1.0f;          ///< Maximum jump height
        float swimSpeed = 0.1f;           ///< Swimming speed
        float flySpeed = 0.3f;            ///< Flying speed
        float size = 1.0f;                ///< Size multiplier
        float armor = 0.0f;               ///< Armor value
        float armorToughness = 0.0f;      ///< Armor toughness
        float knockbackResistance = 0.0f; ///< Knockback resistance
        float fireResistance = 0.0f;      ///< Fire damage resistance
        float waterResistance = 0.0f;     ///< Water damage resistance
        float poisonResistance = 0.0f;    ///< Poison resistance
        float magicResistance = 0.0f;     ///< Magic damage resistance
        float explosionResistance = 0.0f; ///< Explosion resistance
        int experienceValue = 5;          ///< XP dropped when killed
        bool canBreatheUnderwater = false; ///< Can breathe underwater
        bool canFly = false;              ///< Can fly
        bool canSwim = false;             ///< Can swim
        bool canClimb = false;            ///< Can climb walls
        bool canTeleport = false;         ///< Can teleport
        bool burnsInSunlight = false;     ///< Burns in sunlight
        bool immuneToFire = false;        ///< Immune to fire
        bool immuneToWater = false;       ///< Immune to water
        bool immuneToPoison = false;      ///< Immune to poison
        bool immuneToMagic = false;       ///< Immune to magic
        bool dropsLoot = true;            ///< Drops items when killed
        bool canDespawn = true;           ///< Can despawn when far from players
        bool persistent = false;          ///< Stays in world permanently
        int spawnWeight = 10;             ///< Spawn weight for natural spawning
        int minSpawnGroup = 1;            ///< Minimum spawn group size
        int maxSpawnGroup = 1;            ///< Maximum spawn group size
        int spawnLightLevel = 7;          ///< Minimum light level for spawning
        bool spawnsInDarkness = true;     ///< Can spawn in darkness
        std::vector<std::string> spawnBiomes; ///< Biomes where this mob can spawn
        std::vector<int> dropItems;       ///< Items dropped when killed
        std::vector<float> dropChances;   ///< Drop chances for each item
        std::vector<std::string> sounds;  ///< Sound effects for this mob
    };

    /**
     * @struct MobMemory
     * @brief Memory system for mob AI
     */
    struct MobMemory {
        std::unordered_map<std::string, float> numericMemory;
        std::unordered_map<std::string, std::string> stringMemory;
        std::unordered_map<std::string, glm::vec3> positionMemory;
        std::unordered_map<std::string, Entity*> entityMemory;
        std::chrono::steady_clock::time_point lastUpdate;

        /**
         * @brief Get memory value with default
         * @param key Memory key
         * @param defaultValue Default value
         * @return Memory value or default
         */
        float GetFloat(const std::string& key, float defaultValue = 0.0f) const {
            auto it = numericMemory.find(key);
            return (it != numericMemory.end()) ? it->second : defaultValue;
        }

        /**
         * @brief Set memory value
         * @param key Memory key
         * @param value Value to set
         */
        void SetFloat(const std::string& key, float value) {
            numericMemory[key] = value;
        }

        /**
         * @brief Check if memory has expired
         * @param key Memory key
         * @param maxAge Maximum age in seconds
         * @return true if expired
         */
        bool IsExpired(const std::string& key, float maxAge) const {
            auto it = numericMemory.find(key + "_timestamp");
            if (it == numericMemory.end()) return true;

            auto now = std::chrono::steady_clock::now();
            auto timestamp = std::chrono::time_point<std::chrono::steady_clock>(
                std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                    std::chrono::duration<float>(it->second)));
            auto age = std::chrono::duration<float>(now - timestamp).count();
            return age > maxAge;
        }
    };

    /**
     * @class Mob
     * @brief Base class for all mobs in the game
     */
    class Mob : public Entity {
    public:
        /**
         * @brief Constructor
         * @param type Mob type
         * @param position Initial position
         * @param world World reference
         */
        Mob(MobType type, const glm::vec3& position, World* world);

        /**
         * @brief Destructor
         */
        ~Mob() override = default;

        /**
         * @brief Update mob logic
         * @param deltaTime Time since last update
         */
        void Update(float deltaTime) override;

        /**
         * @brief Fixed update for physics
         * @param deltaTime Time since last fixed update
         */
        void FixedUpdate(float deltaTime) override;

        /**
         * @brief Get mob type
         * @return Mob type
         */
        MobType GetMobType() const { return m_type; }

        /**
         * @brief Get mob attributes
         * @return Mob attributes
         */
        const MobAttributes& GetAttributes() const { return m_attributes; }

        /**
         * @brief Get current state
         * @return Current state
         */
        MobState GetState() const { return m_state; }

        /**
         * @brief Set mob state
         * @param state New state
         */
        void SetState(MobState state) { m_state = state; }

        /**
         * @brief Get current health
         * @return Current health
         */
        float GetHealth() const { return m_health; }

        /**
         * @brief Set health
         * @param health New health value
         */
        void SetHealth(float health) { m_health = std::max(0.0f, std::min(m_attributes.health, health)); }

        /**
         * @brief Deal damage to mob
         * @param damage Amount of damage
         * @param source Damage source
         */
        void TakeDamage(float damage, Entity* source = nullptr) override;

        /**
         * @brief Heal mob
         * @param amount Heal amount
         */
        void Heal(float amount) { SetHealth(m_health + amount); }

        /**
         * @brief Check if mob is alive
         * @return true if alive
         */
        bool IsAlive() const { return m_health > 0.0f; }

        /**
         * @brief Kill the mob
         */
        void Kill();

        /**
         * @brief Set target entity
         * @param target Target entity
         */
        void SetTarget(Entity* target) { m_target = target; }

        /**
         * @brief Get current target
         * @return Current target or nullptr
         */
        Entity* GetTarget() const { return m_target; }

        /**
         * @brief Check if mob has behavior
         * @param behavior Behavior to check
         * @return true if has behavior
         */
        bool HasBehavior(MobBehavior behavior) const;

        /**
         * @brief Add behavior to mob
         * @param behavior Behavior to add
         */
        void AddBehavior(MobBehavior behavior);

        /**
         * @brief Remove behavior from mob
         * @param behavior Behavior to remove
         */
        void RemoveBehavior(MobBehavior behavior);

        /**
         * @brief Get mob memory
         * @return Mob memory reference
         */
        MobMemory& GetMemory() { return m_memory; }

        /**
         * @brief Set mob position
         * @param position New position
         */
        void SetPosition(const glm::vec3& position) override;

        /**
         * @brief Move mob
         * @param direction Movement direction
         * @param speed Movement speed
         */
        void Move(const glm::vec3& direction, float speed);

        /**
         * @brief Jump
         */
        void Jump();

        /**
         * @brief Attack target
         * @param target Target to attack
         * @return true if attack was successful
         */
        bool Attack(Entity* target);

        /**
         * @brief Use ranged attack
         * @param target Target to attack
         * @return true if attack was successful
         */
        bool RangedAttack(Entity* target);

        /**
         * @brief Teleport to position
         * @param position Target position
         * @return true if teleport was successful
         */
        bool Teleport(const glm::vec3& position);

        /**
         * @brief Play sound
         * @param sound Sound name
         */
        void PlaySound(const std::string& sound);

        /**
         * @brief Spawn mob at position
         * @param position Spawn position
         * @return true if spawned successfully
         */
        virtual bool Spawn(const glm::vec3& position);

        /**
         * @brief Despawn mob
         */
        virtual void Despawn();

        /**
         * @brief Check if mob can spawn at position
         * @param position Position to check
         * @return true if can spawn
         */
        virtual bool CanSpawnAt(const glm::vec3& position) const;

        /**
         * @brief Get mob name
         * @return Mob name
         */
        virtual std::string GetName() const;

        /**
         * @brief Get mob display name
         * @return Display name
         */
        virtual std::string GetDisplayName() const;

        /**
         * @brief Get mob description
         * @return Mob description
         */
        virtual std::string GetDescription() const;

        /**
         * @brief Handle mob death
         */
        virtual void OnDeath();

        /**
         * @brief Handle mob spawn
         */
        virtual void OnSpawn();

        /**
         * @brief Handle mob attack
         * @param target Attack target
         */
        virtual void OnAttack(Entity* target);

        /**
         * @brief Handle mob damage
         * @param source Damage source
         */
        virtual void OnDamage(Entity* source);

        /**
         * @brief Handle mob healing
         * @param amount Heal amount
         */
        virtual void OnHeal(float amount);

    protected:
        MobType m_type;
        MobAttributes m_attributes;
        MobState m_state;
        float m_health;
        Entity* m_target;
        MobMemory m_memory;
        std::vector<MobBehavior> m_behaviors;
        std::unique_ptr<Pathfinding> m_pathfinding;

        // Movement and physics
        glm::vec3 m_velocity;
        float m_gravity;
        bool m_onGround;
        bool m_inWater;
        bool m_onFire;
        bool m_isFlying;
        bool m_isClimbing;

        // AI and behavior
        float m_wanderTimer;
        float m_attackTimer;
        float m_healTimer;
        float m_teleportTimer;
        glm::vec3 m_wanderTarget;
        glm::vec3 m_homePosition;
        float m_homeRadius;

        // Visual effects
        float m_animationTimer;
        float m_glowIntensity;
        float m_sizeMultiplier;

        /**
         * @brief Update AI behavior
         * @param deltaTime Time since last update
         */
        virtual void UpdateAI(float deltaTime);

        /**
         * @brief Update movement
         * @param deltaTime Time since last update
         */
        virtual void UpdateMovement(float deltaTime);

        /**
         * @brief Update animations
         * @param deltaTime Time since last update
         */
        virtual void UpdateAnimations(float deltaTime);

        /**
         * @brief Update visual effects
         * @param deltaTime Time since last update
         */
        virtual void UpdateEffects(float deltaTime);

        /**
         * @brief Check if target is valid
         * @param target Target to check
         * @return true if valid
         */
        bool IsValidTarget(Entity* target) const;

        /**
         * @brief Find nearest target
         * @param range Search range
         * @return Nearest target or nullptr
         */
        Entity* FindNearestTarget(float range) const;

        /**
         * @brief Calculate distance to target
         * @param target Target entity
         * @return Distance to target
         */
        float DistanceToTarget(Entity* target) const;

        /**
         * @brief Check if target is in range
         * @param target Target entity
         * @param range Range to check
         * @return true if in range
         */
        bool IsTargetInRange(Entity* target, float range) const;

        /**
         * @brief Check line of sight to target
         * @param target Target entity
         * @return true if has line of sight
         */
        bool HasLineOfSight(Entity* target) const;

        /**
         * @brief Initialize mob attributes based on type
         */
        virtual void InitializeAttributes();

        /**
         * @brief Initialize mob behaviors based on type
         */
        virtual void InitializeBehaviors();

        /**
         * @brief Initialize mob sounds
         */
        virtual void InitializeSounds();
    };

    /**
     * @class HostileMob
     * @brief Base class for hostile mobs
     */
    class HostileMob : public Mob {
    public:
        HostileMob(MobType type, const glm::vec3& position, World* world);

    protected:
        void UpdateAI(float deltaTime) override;
        void InitializeAttributes() override;
        void InitializeBehaviors() override;
    };

    /**
     * @class PassiveMob
     * @brief Base class for passive mobs
     */
    class PassiveMob : public Mob {
    public:
        PassiveMob(MobType type, const glm::vec3& position, World* world);

    protected:
        void UpdateAI(float deltaTime) override;
        void InitializeAttributes() override;
        void InitializeBehaviors() override;
    };

    /**
     * @class TamableMob
     * @brief Base class for tamable mobs
     */
    class TamableMob : public PassiveMob {
    public:
        TamableMob(MobType type, const glm::vec3& position, World* world);

        /**
         * @brief Check if mob is tamed
         * @return true if tamed
         */
        bool IsTamed() const { return m_tamed; }

        /**
         * @brief Get mob owner
         * @return Owner entity or nullptr
         */
        Entity* GetOwner() const { return m_owner; }

        /**
         * @brief Tame mob
         * @param owner New owner
         * @return true if tamed successfully
         */
        bool Tame(Entity* owner);

        /**
         * @brief Untame mob
         */
        void Untame();

    protected:
        bool m_tamed;
        Entity* m_owner;
        float m_tamingProgress;

        void UpdateAI(float deltaTime) override;
        void InitializeAttributes() override;
        void InitializeBehaviors() override;
    };

    /**
     * @class FlyingMob
     * @brief Base class for flying mobs
     */
    class FlyingMob : public Mob {
    public:
        FlyingMob(MobType type, const glm::vec3& position, World* world);

    protected:
        void UpdateMovement(float deltaTime) override;
        void InitializeAttributes() override;
        void InitializeBehaviors() override;
    };

    /**
     * @class AquaticMob
     * @brief Base class for aquatic mobs
     */
    class AquaticMob : public Mob {
    public:
        AquaticMob(MobType type, const glm::vec3& position, World* world);

    protected:
        void UpdateMovement(float deltaTime) override;
        void InitializeAttributes() override;
        void InitializeBehaviors() override;
    };

    /**
     * @class Creeper
     * @brief Creeper mob implementation
     */
    class Creeper : public HostileMob {
    public:
        Creeper(const glm::vec3& position, World* world);

        void UpdateAI(float deltaTime) override;
        void OnDeath() override;

    private:
        float m_explosionTimer;
        float m_explosionRadius;
        bool m_charged;
        bool m_ignited;

        void Explode();
    };

    /**
     * @class Zombie
     * @brief Zombie mob implementation
     */
    class Zombie : public HostileMob {
    public:
        Zombie(const glm::vec3& position, World* world);

        void UpdateAI(float deltaTime) override;
        void OnAttack(Entity* target) override;

    private:
        bool m_canBreakDoors;
        float m_doorBreakTimer;
        int m_doorBreakProgress;
    };

    /**
     * @class Enderman
     * @brief Enderman mob implementation
     */
    class Enderman : public HostileMob {
    public:
        Enderman(const glm::vec3& position, World* world);

        void UpdateAI(float deltaTime) override;
        void TakeDamage(float damage, Entity* source = nullptr) override;
        bool Teleport(const glm::vec3& position) override;

    private:
        int m_heldBlockID;
        glm::vec3 m_lastTeleportPosition;
        float m_stareTimer;
        bool m_isStaring;
        bool m_isProvoked;

        void PickUpBlock();
        void PlaceBlock();
    };

    /**
     * @class Wolf
     * @brief Wolf mob implementation
     */
    class Wolf : public TamableMob {
    public:
        Wolf(const glm::vec3& position, World* world);

        void UpdateAI(float deltaTime) override;
        void OnAttack(Entity* target) override;

    private:
        bool m_isAngry;
        float m_hungerLevel;
        Entity* m_packLeader;

        void Howl();
        void Hunt();
        void BegForFood();
    };

    /**
     * @class Villager
     * @brief Villager mob implementation
     */
    class Villager : public PassiveMob {
    public:
        Villager(const glm::vec3& position, World* world);

        void UpdateAI(float deltaTime) override;

    private:
        std::string m_profession;
        int m_level;
        std::unordered_map<std::string, int> m_inventory;
        glm::vec3 m_workstation;
        glm::vec3 m_bedPosition;
        bool m_hasWorkstation;
        bool m_hasBed;
        float m_workTimer;
        float m_socialTimer;

        void Work();
        void Socialize();
        void Sleep();
        void Trade();
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_MOB_MOB_HPP
