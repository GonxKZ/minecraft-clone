/**
 * @file MobGenerator.hpp
 * @brief VoxelCraft Procedural Mob Generator - AI Creatures with Auto-Generated Skins
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_WORLD_MOB_GENERATOR_HPP
#define VOXELCRAFT_WORLD_MOB_GENERATOR_HPP

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <random>
#include <functional>
#include "ProceduralGenerator.hpp"

namespace VoxelCraft {

    // Forward declarations
    class EntityManager;
    class Entity;
    struct Vec3;

    /**
     * @enum MobType
     * @brief Types of mobs that can be generated
     */
    enum class MobType {
        // Hostile mobs
        ZOMBIE = 0,
        SKELETON,
        CREEPER,
        SPIDER,
        ENDERMAN,
        BLAZE,
        GHAST,

        // Neutral mobs
        PIG,
        COW,
        CHICKEN,
        SHEEP,
        WOLF,
        OCELOT,
        BAT,

        // Passive mobs
        VILLAGER,
        IRON_GOLEM,
        SNOW_GOLEM,

        // Boss mobs
        WITHER,
        ENDER_DRAGON,

        // Special mobs
        CUSTOM
    };

    /**
     * @enum MobBehavior
     * @brief AI behaviors for mobs
     */
    enum class MobBehavior {
        PASSIVE = 0,        ///< Doesn't attack unless provoked
        NEUTRAL,            ///< Attacks when provoked
        HOSTILE,            ///< Always hostile
        FRIENDLY,           ///< Always friendly
        SCARED,             ///< Runs away from threats
        TAMABLE             ///< Can be tamed by player
    };

    /**
     * @enum MobSize
     * @brief Size categories for mobs
     */
    enum class MobSize {
        TINY = 0,           ///< 0.25x0.25x0.25 blocks (bat)
        SMALL,              ///< 0.5x0.5x0.5 blocks (chicken)
        NORMAL,             ///< 1x1x1 blocks (most mobs)
        LARGE,              ///< 2x2x2 blocks (iron golem)
        HUGE                ///< 3x3x3+ blocks (ender dragon)
    };

    /**
     * @struct MobStats
     * @brief Statistics and properties of a mob
     */
    struct MobStats {
        float health;                   ///< Maximum health
        float attackDamage;             ///< Base attack damage
        float movementSpeed;            ///< Movement speed
        float followRange;              ///< How far mob will follow/attack
        float knockbackResistance;      ///< Resistance to knockback
        bool canBreatheUnderwater;      ///< Can breathe underwater
        bool isFlying;                  ///< Can fly
        bool isUndead;                  ///< Is undead (affected by smite, heals in dark)
        bool isArthropod;               ///< Is arthropod (affected by bane of arthropods)
        bool isImmuneToFire;            ///< Immune to fire damage
        bool dropsExperience;           ///< Drops experience when killed
        int experienceValue;            ///< Experience points dropped

        MobStats()
            : health(20.0f)
            , attackDamage(1.0f)
            , movementSpeed(0.2f)
            , followRange(16.0f)
            , knockbackResistance(0.0f)
            , canBreatheUnderwater(false)
            , isFlying(false)
            , isUndead(false)
            , isArthropod(false)
            , isImmuneToFire(false)
            , dropsExperience(true)
            , experienceValue(5)
        {}
    };

    /**
     * @struct MobAppearance
     * @brief Visual appearance data for a mob
     */
    struct MobAppearance {
        std::string skinTexture;        ///< Path to skin texture
        Vec3 primaryColor;              ///< Primary color
        Vec3 secondaryColor;            ///< Secondary color
        Vec3 eyeColor;                  ///< Eye color
        float scale;                    ///< Size scale
        int variant;                    ///< Visual variant
        bool hasArmor;                  ///< Has armor
        bool hasAccessories;            ///< Has accessories
        std::vector<std::string> specialFeatures; ///< Special visual features

        MobAppearance()
            : skinTexture("")
            , primaryColor(1.0f, 1.0f, 1.0f)
            , secondaryColor(0.8f, 0.8f, 0.8f)
            , eyeColor(0.2f, 0.4f, 0.8f)
            , scale(1.0f)
            , variant(0)
            , hasArmor(false)
            , hasAccessories(false)
        {}
    };

    /**
     * @struct MobAI
     * @brief AI behavior configuration for a mob
     */
    struct MobAI {
        MobBehavior behavior;            ///< Base behavior type
        float detectionRange;            ///< How far mob can detect targets
        float attackRange;               ///< Attack range
        float attackCooldown;            ///< Time between attacks
        float wanderRadius;              ///< How far mob wanders from spawn
        float fleeThreshold;             ///< Health percentage to flee at
        bool canOpenDoors;               ///< Can open doors
        bool canBreakDoors;              ///< Can break doors
        bool avoidsWater;                ///< Avoids water
        bool avoidsSunlight;             ///< Avoids sunlight (undead)
        std::vector<std::string> targetEntities; ///< Entities this mob targets

        MobAI()
            : behavior(MobBehavior::NEUTRAL)
            , detectionRange(16.0f)
            , attackRange(2.0f)
            , attackCooldown(1.0f)
            , wanderRadius(10.0f)
            , fleeThreshold(0.2f)
            , canOpenDoors(false)
            , canBreakDoors(false)
            , avoidsWater(false)
            , avoidsSunlight(false)
        {}
    };

    /**
     * @struct GeneratedMob
     * @brief Complete mob data generated procedurally
     */
    struct GeneratedMob {
        std::string id;                 ///< Unique mob identifier
        std::string name;               ///< Mob name
        MobType type;                   ///< Mob type
        Vec3 position;                  ///< Spawn position
        MobStats stats;                 ///< Mob statistics
        MobAppearance appearance;       ///< Visual appearance
        MobAI ai;                       ///< AI behavior
        std::string biome;              ///< Biome this mob spawns in
        int spawnWeight;                ///< Spawn weight (rarity)
        bool isBoss;                    ///< Is this a boss mob?
        std::vector<std::string> drops; ///< Items this mob can drop
        void* skinData;                 ///< Generated skin data
        void* soundData;                ///< Generated sound data

        GeneratedMob()
            : id("")
            , name("")
            , type(MobType::ZOMBIE)
            , spawnWeight(100)
            , isBoss(false)
            , skinData(nullptr)
            , soundData(nullptr)
        {}
    };

    /**
     * @class MobGenerator
     * @brief Procedural mob generator with auto-generated skins and behaviors
     */
    class MobGenerator {
    public:
        /**
         * @brief Constructor
         */
        MobGenerator();

        /**
         * @brief Destructor
         */
        ~MobGenerator();

        /**
         * @brief Initialize the mob generator
         * @param seed Random seed for generation
         */
        void Initialize(uint64_t seed);

        /**
         * @brief Generate a mob of specified type
         * @param type Mob type to generate
         * @param position Spawn position
         * @param biome Biome where mob spawns
         * @return Generated mob data
         */
        GeneratedMob GenerateMob(MobType type, const Vec3& position, const std::string& biome);

        /**
         * @brief Generate a random mob for biome
         * @param position Spawn position
         * @param biome Biome type
         * @return Generated mob data
         */
        GeneratedMob GenerateRandomMob(const Vec3& position, const std::string& biome);

        /**
         * @brief Generate mob skin procedurally
         * @param mobType Type of mob
         * @param seed Generation seed
         * @return Generated skin data
         */
        void* GenerateMobSkin(MobType mobType, uint64_t seed);

        /**
         * @brief Generate mob sounds procedurally
         * @param mobType Type of mob
         * @param seed Generation seed
         * @return Generated sound data
         */
        void* GenerateMobSounds(MobType mobType, uint64_t seed);

        /**
         * @brief Create entity from generated mob
         * @param mob Generated mob data
         * @param entityManager Entity manager to create in
         * @return Created entity
         */
        std::shared_ptr<Entity> CreateEntityFromMob(const GeneratedMob& mob, EntityManager* entityManager);

        /**
         * @brief Get mob spawn weight for biome
         * @param mobType Mob type
         * @param biome Biome type
         * @return Spawn weight
         */
        int GetMobSpawnWeight(MobType mobType, const std::string& biome) const;

        /**
         * @brief Check if mob can spawn at position
         * @param mobType Mob type
         * @param position Position to check
         * @param biome Biome at position
         * @return true if can spawn
         */
        bool CanMobSpawnAt(MobType mobType, const Vec3& position, const std::string& biome) const;

    private:
        uint64_t m_seed;
        std::mt19937_64 m_randomEngine;

        // Mob templates for each type
        std::unordered_map<MobType, MobStats> m_mobStatsTemplates;
        std::unordered_map<MobType, MobAI> m_mobAITemplates;
        std::unordered_map<MobType, std::function<void*(uint64_t)>> m_skinGenerators;
        std::unordered_map<MobType, std::function<void*(uint64_t)>> m_soundGenerators;

        /**
         * @brief Initialize mob stat templates
         */
        void InitializeMobTemplates();

        /**
         * @brief Initialize skin generators for each mob type
         */
        void InitializeSkinGenerators();

        /**
         * @brief Initialize sound generators for each mob type
         */
        void InitializeSoundGenerators();

        /**
         * @brief Generate zombie mob
         * @param position Spawn position
         * @param biome Biome type
         * @return Generated zombie
         */
        GeneratedMob GenerateZombie(const Vec3& position, const std::string& biome);

        /**
         * @brief Generate skeleton mob
         * @param position Spawn position
         * @param biome Biome type
         * @return Generated skeleton
         */
        GeneratedMob GenerateSkeleton(const Vec3& position, const std::string& biome);

        /**
         * @brief Generate creeper mob
         * @param position Spawn position
         * @param biome Biome type
         * @return Generated creeper
         */
        GeneratedMob GenerateCreeper(const Vec3& position, const std::string& biome);

        /**
         * @brief Generate spider mob
         * @param position Spawn position
         * @param biome Biome type
         * @return Generated spider
         */
        GeneratedMob GenerateSpider(const Vec3& position, const std::string& biome);

        /**
         * @brief Generate pig mob
         * @param position Spawn position
         * @param biome Biome type
         * @return Generated pig
         */
        GeneratedMob GeneratePig(const Vec3& position, const std::string& biome);

        /**
         * @brief Generate cow mob
         * @param position Spawn position
         * @param biome Biome type
         * @return Generated cow
         */
        GeneratedMob GenerateCow(const Vec3& position, const std::string& biome);

        /**
         * @brief Generate chicken mob
         * @param position Spawn position
         * @param biome Biome type
         * @return Generated chicken
         */
        GeneratedMob GenerateChicken(const Vec3& position, const std::string& biome);

        /**
         * @brief Generate enderman mob
         * @param position Spawn position
         * @param biome Biome type
         * @return Generated enderman
         */
        GeneratedMob GenerateEnderman(const Vec3& position, const std::string& biome);

        /**
         * @brief Generate blaze mob
         * @param position Spawn position
         * @param biome Biome type
         * @return Generated blaze
         */
        GeneratedMob GenerateBlaze(const Vec3& position, const std::string& biome);

        /**
         * @brief Generate custom mob with random properties
         * @param position Spawn position
         * @param biome Biome type
         * @return Generated custom mob
         */
        GeneratedMob GenerateCustomMob(const Vec3& position, const std::string& biome);

        /**
         * @brief Apply biome variations to mob
         * @param mob Mob to modify
         * @param biome Biome type
         */
        void ApplyBiomeVariations(GeneratedMob& mob, const std::string& biome);

        /**
         * @brief Generate unique mob name
         * @param baseName Base mob name
         * @param position Position for seed
         * @return Unique name
         */
        std::string GenerateUniqueMobName(const std::string& baseName, const Vec3& position);

        // Skin generation methods
        void* GenerateZombieSkin(uint64_t seed);
        void* GenerateSkeletonSkin(uint64_t seed);
        void* GenerateCreeperSkin(uint64_t seed);
        void* GenerateSpiderSkin(uint64_t seed);
        void* GeneratePigSkin(uint64_t seed);
        void* GenerateCowSkin(uint64_t seed);
        void* GenerateChickenSkin(uint64_t seed);
        void* GenerateEndermanSkin(uint64_t seed);
        void* GenerateBlazeSkin(uint64_t seed);
        void* GenerateCustomSkin(uint64_t seed);

        // Sound generation methods
        void* GenerateZombieSounds(uint64_t seed);
        void* GenerateSkeletonSounds(uint64_t seed);
        void* GenerateCreeperSounds(uint64_t seed);
        void* GenerateSpiderSounds(uint64_t seed);
        void* GeneratePigSounds(uint64_t seed);
        void* GenerateCowSounds(uint64_t seed);
        void* GenerateChickenSounds(uint64_t seed);
        void* GenerateEndermanSounds(uint64_t seed);
        void* GenerateBlazeSounds(uint64_t seed);
        void* GenerateCustomSounds(uint64_t seed);
    };

    /**
     * @struct ZombieSkin
     * @brief Zombie appearance data
     */
    struct ZombieSkin {
        float decayLevel;               ///< How decayed the zombie looks (0-1)
        Vec3 skinColor;                 ///< Base skin color
        Vec3 decayColor;                ///< Color of decayed areas
        int missingLimbs;               ///< Number of missing limbs
        bool hasHelmet;                 ///< Wearing helmet
        bool hasArmor;                  ///< Wearing armor
        int variant;                    ///< Visual variant
    };

    /**
     * @struct SkeletonSkin
     * @brief Skeleton appearance data
     */
    struct SkeletonSkin {
        float boneCondition;            ///< Condition of bones (0-1, 1=perfect)
        Vec3 boneColor;                 ///< Color of bones
        bool hasHelmet;                 ///< Wearing helmet
        bool hasArmor;                  ///< Wearing armor
        int bowType;                    ///< Type of bow (0=normal, 1=enchanted)
        int variant;                    ///< Visual variant
    };

    /**
     * @struct CreeperSkin
     * @brief Creeper appearance data
     */
    struct CreeperSkin {
        float size;                     ///< Size multiplier
        Vec3 bodyColor;                 ///< Main body color
        Vec3 stripeColor;               ///< Stripe color
        int pattern;                    ///< Pattern type (0-3)
        bool isCharged;                 ///< Is charged creeper
        int variant;                    ///< Visual variant
    };

    /**
     * @struct SpiderSkin
     * @brief Spider appearance data
     */
    struct SpiderSkin {
        float size;                     ///< Size multiplier
        Vec3 bodyColor;                 ///< Main body color
        Vec3 eyeColor;                  ///< Eye color
        int eyeCount;                   ///< Number of eyes (6-8)
        int pattern;                    ///< Pattern type
        int variant;                    ///< Visual variant
    };

    /**
     * @struct PigSkin
     * @brief Pig appearance data
     */
    struct PigSkin {
        Vec3 furColor;                  ///< Fur color
        int spotDensity;                ///< Density of spots (0-10)
        Vec3 spotColor;                 ///< Spot color
        bool hasSaddle;                 ///< Has saddle
        float size;                     ///< Size multiplier
        int variant;                    ///< Visual variant
    };

    /**
     * @struct CowSkin
     * @brief Cow appearance data
     */
    struct CowSkin {
        Vec3 furColor;                  ///< Fur color
        int spotPattern;                ///< Spot pattern type
        Vec3 spotColor;                 ///< Spot color
        float spotDensity;              ///< Density of spots
        int variant;                    ///< Visual variant
    };

    /**
     * @struct ChickenSkin
     * @brief Chicken appearance data
     */
    struct ChickenSkin {
        Vec3 featherColor;              ///< Feather color
        Vec3 combColor;                 ///< Comb/wattle color
        int featherPattern;             ///< Feather pattern
        float combSize;                 ///< Comb size
        int variant;                    ///< Visual variant
    };

    /**
     * @struct EndermanSkin
     * @brief Enderman appearance data
     */
    struct EndermanSkin {
        Vec3 eyeColor;                  ///< Eye color
        Vec3 bodyColor;                 ///< Body color
        int eyePattern;                 ///< Eye pattern type
        bool isProvoked;                ///< Is provoked (affects appearance)
        int variant;                    ///< Visual variant
    };

    /**
     * @struct BlazeSkin
     * @brief Blaze appearance data
     */
    struct BlazeSkin {
        Vec3 flameColor;                ///< Flame color
        int flamePattern;               ///< Flame pattern type
        int rodCount;                   ///< Number of flame rods
        float intensity;                ///< Flame intensity
        int variant;                    ///< Visual variant
    };

    /**
     * @struct CustomSkin
     * @brief Custom mob appearance data
     */
    struct CustomSkin {
        Vec3 primaryColor;              ///< Primary color
        Vec3 secondaryColor;            ///< Secondary color
        Vec3 accentColor;               ///< Accent color
        int shapeType;                  ///< Body shape type
        int featureCount;               ///< Number of special features
        std::vector<std::string> features; ///< List of features
        int variant;                    ///< Visual variant
    };

    /**
     * @struct MobSounds
     * @brief Sound data for mobs
     */
    struct MobSounds {
        void* idleSound;                ///< Idle/ambient sound
        void* hurtSound;                ///< Hurt sound
        void* deathSound;               ///< Death sound
        void* attackSound;              ///< Attack sound (if applicable)
        void* specialSound;             ///< Special action sound
        float volume;                   ///< Base volume
        float pitch;                    ///< Base pitch
        int variant;                    ///< Sound variant
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_WORLD_MOB_GENERATOR_HPP
