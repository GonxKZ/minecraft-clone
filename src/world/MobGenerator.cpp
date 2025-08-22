/**
 * @file MobGenerator.cpp
 * @brief VoxelCraft Procedural Mob Generator - Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "MobGenerator.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace VoxelCraft {

    MobGenerator::MobGenerator() {
        m_seed = std::chrono::system_clock::now().time_since_epoch().count();
        m_randomEngine.seed(m_seed);
    }

    MobGenerator::~MobGenerator() {
        // Cleanup any generated data
    }

    void MobGenerator::Initialize(uint64_t seed) {
        m_seed = seed;
        m_randomEngine.seed(seed);
        InitializeMobTemplates();
        InitializeSkinGenerators();
        InitializeSoundGenerators();
    }

    void MobGenerator::InitializeMobTemplates() {
        // Zombie template
        MobStats zombieStats;
        zombieStats.health = 20.0f;
        zombieStats.attackDamage = 3.0f;
        zombieStats.movementSpeed = 0.23f;
        zombieStats.followRange = 35.0f;
        zombieStats.isUndead = true;
        zombieStats.avoidsSunlight = true;
        zombieStats.experienceValue = 5;
        m_mobStatsTemplates[MobType::ZOMBIE] = zombieStats;

        MobAI zombieAI;
        zombieAI.behavior = MobBehavior::HOSTILE;
        zombieAI.detectionRange = 16.0f;
        zombieAI.attackRange = 2.0f;
        zombieAI.attackCooldown = 1.0f;
        zombieAI.avoidsSunlight = true;
        zombieAI.targetEntities = {"player"};
        m_mobAITemplates[MobType::ZOMBIE] = zombieAI;

        // Skeleton template
        MobStats skeletonStats;
        skeletonStats.health = 20.0f;
        skeletonStats.attackDamage = 3.0f;
        skeletonStats.movementSpeed = 0.25f;
        skeletonStats.followRange = 16.0f;
        skeletonStats.isUndead = true;
        skeletonStats.avoidsSunlight = true;
        skeletonStats.experienceValue = 5;
        m_mobStatsTemplates[MobType::SKELETON] = skeletonStats;

        MobAI skeletonAI;
        skeletonAI.behavior = MobBehavior::HOSTILE;
        skeletonAI.detectionRange = 16.0f;
        skeletonAI.attackRange = 15.0f; // Ranged attack
        skeletonAI.attackCooldown = 3.0f; // Bow cooldown
        skeletonAI.avoidsSunlight = true;
        skeletonAI.targetEntities = {"player"};
        m_mobAITemplates[MobType::SKELETON] = skeletonAI;

        // Creeper template
        MobStats creeperStats;
        creeperStats.health = 20.0f;
        creeperStats.attackDamage = 0.0f; // Special explosion damage
        creeperStats.movementSpeed = 0.2f;
        creeperStats.followRange = 16.0f;
        creeperStats.experienceValue = 5;
        m_mobStatsTemplates[MobType::CREEPER] = creeperStats;

        MobAI creeperAI;
        creeperAI.behavior = MobBehavior::HOSTILE;
        creeperAI.detectionRange = 16.0f;
        creeperAI.attackRange = 3.0f; // Explosion range
        creeperAI.attackCooldown = 1.5f;
        creeperAI.targetEntities = {"player"};
        m_mobAITemplates[MobType::CREEPER] = creeperAI;

        // Spider template
        MobStats spiderStats;
        spiderStats.health = 16.0f;
        spiderStats.attackDamage = 2.0f;
        spiderStats.movementSpeed = 0.3f;
        spiderStats.followRange = 16.0f;
        spiderStats.isArthropod = true;
        spiderStats.experienceValue = 5;
        m_mobStatsTemplates[MobType::SPIDER] = spiderStats;

        MobAI spiderAI;
        spiderAI.behavior = MobBehavior::HOSTILE;
        spiderAI.detectionRange = 16.0f;
        spiderAI.attackRange = 2.0f;
        spiderAI.attackCooldown = 1.0f;
        spiderAI.targetEntities = {"player"};
        m_mobAITemplates[MobType::SPIDER] = spiderAI;

        // Pig template
        MobStats pigStats;
        pigStats.health = 10.0f;
        pigStats.attackDamage = 0.0f;
        pigStats.movementSpeed = 0.25f;
        pigStats.experienceValue = 1;
        m_mobStatsTemplates[MobType::PIG] = pigStats;

        MobAI pigAI;
        pigAI.behavior = MobBehavior::PASSIVE;
        pigAI.wanderRadius = 15.0f;
        m_mobAITemplates[MobType::PIG] = pigAI;

        // Cow template
        MobStats cowStats;
        cowStats.health = 10.0f;
        cowStats.attackDamage = 0.0f;
        cowStats.movementSpeed = 0.2f;
        cowStats.experienceValue = 1;
        m_mobStatsTemplates[MobType::COW] = cowStats;

        MobAI cowAI;
        cowAI.behavior = MobBehavior::PASSIVE;
        cowAI.wanderRadius = 15.0f;
        m_mobAITemplates[MobType::COW] = cowAI;

        // Chicken template
        MobStats chickenStats;
        chickenStats.health = 4.0f;
        chickenStats.attackDamage = 0.0f;
        chickenStats.movementSpeed = 0.25f;
        chickenStats.experienceValue = 1;
        m_mobStatsTemplates[MobType::CHICKEN] = chickenStats;

        MobAI chickenAI;
        chickenAI.behavior = MobBehavior::PASSIVE;
        chickenAI.wanderRadius = 10.0f;
        m_mobAITemplates[MobType::CHICKEN] = chickenAI;

        // Enderman template
        MobStats endermanStats;
        endermanStats.health = 40.0f;
        endermanStats.attackDamage = 7.0f;
        endermanStats.movementSpeed = 0.3f;
        endermanStats.followRange = 64.0f;
        endermanStats.experienceValue = 5;
        m_mobStatsTemplates[MobType::ENDERMAN] = endermanStats;

        MobAI endermanAI;
        endermanAI.behavior = MobBehavior::NEUTRAL;
        endermanAI.detectionRange = 64.0f;
        endermanAI.attackRange = 2.0f;
        endermanAI.attackCooldown = 1.0f;
        endermanAI.targetEntities = {"player"};
        m_mobAITemplates[MobType::ENDERMAN] = endermanAI;

        // Blaze template
        MobStats blazeStats;
        blazeStats.health = 20.0f;
        blazeStats.attackDamage = 6.0f;
        blazeStats.movementSpeed = 0.23f;
        blazeStats.followRange = 48.0f;
        blazeStats.isImmuneToFire = true;
        blazeStats.isFlying = true;
        blazeStats.experienceValue = 10;
        m_mobStatsTemplates[MobType::BLAZE] = blazeStats;

        MobAI blazeAI;
        blazeAI.behavior = MobBehavior::HOSTILE;
        blazeAI.detectionRange = 48.0f;
        blazeAI.attackRange = 24.0f; // Fireball range
        blazeAI.attackCooldown = 2.0f;
        blazeAI.targetEntities = {"player"};
        m_mobAITemplates[MobType::BLAZE] = blazeAI;
    }

    void MobGenerator::InitializeSkinGenerators() {
        m_skinGenerators[MobType::ZOMBIE] = [this](uint64_t seed) -> void* {
            return GenerateZombieSkin(seed);
        };

        m_skinGenerators[MobType::SKELETON] = [this](uint64_t seed) -> void* {
            return GenerateSkeletonSkin(seed);
        };

        m_skinGenerators[MobType::CREEPER] = [this](uint64_t seed) -> void* {
            return GenerateCreeperSkin(seed);
        };

        m_skinGenerators[MobType::SPIDER] = [this](uint64_t seed) -> void* {
            return GenerateSpiderSkin(seed);
        };

        m_skinGenerators[MobType::PIG] = [this](uint64_t seed) -> void* {
            return GeneratePigSkin(seed);
        };

        m_skinGenerators[MobType::COW] = [this](uint64_t seed) -> void* {
            return GenerateCowSkin(seed);
        };

        m_skinGenerators[MobType::CHICKEN] = [this](uint64_t seed) -> void* {
            return GenerateChickenSkin(seed);
        };

        m_skinGenerators[MobType::ENDERMAN] = [this](uint64_t seed) -> void* {
            return GenerateEndermanSkin(seed);
        };

        m_skinGenerators[MobType::BLAZE] = [this](uint64_t seed) -> void* {
            return GenerateBlazeSkin(seed);
        };

        m_skinGenerators[MobType::CUSTOM] = [this](uint64_t seed) -> void* {
            return GenerateCustomSkin(seed);
        };
    }

    void MobGenerator::InitializeSoundGenerators() {
        m_soundGenerators[MobType::ZOMBIE] = [this](uint64_t seed) -> void* {
            return GenerateZombieSounds(seed);
        };

        m_soundGenerators[MobType::SKELETON] = [this](uint64_t seed) -> void* {
            return GenerateSkeletonSounds(seed);
        };

        m_soundGenerators[MobType::CREEPER] = [this](uint64_t seed) -> void* {
            return GenerateCreeperSounds(seed);
        };

        m_soundGenerators[MobType::SPIDER] = [this](uint64_t seed) -> void* {
            return GenerateSpiderSounds(seed);
        };

        m_soundGenerators[MobType::PIG] = [this](uint64_t seed) -> void* {
            return GeneratePigSounds(seed);
        };

        m_soundGenerators[MobType::COW] = [this](uint64_t seed) -> void* {
            return GenerateCowSounds(seed);
        };

        m_soundGenerators[MobType::CHICKEN] = [this](uint64_t seed) -> void* {
            return GenerateChickenSounds(seed);
        };

        m_soundGenerators[MobType::ENDERMAN] = [this](uint64_t seed) -> void* {
            return GenerateEndermanSounds(seed);
        };

        m_soundGenerators[MobType::BLAZE] = [this](uint64_t seed) -> void* {
            return GenerateBlazeSounds(seed);
        };

        m_soundGenerators[MobType::CUSTOM] = [this](uint64_t seed) -> void* {
            return GenerateCustomSounds(seed);
        };
    }

    GeneratedMob MobGenerator::GenerateMob(MobType type, const Vec3& position, const std::string& biome) {
        switch (type) {
            case MobType::ZOMBIE:
                return GenerateZombie(position, biome);
            case MobType::SKELETON:
                return GenerateSkeleton(position, biome);
            case MobType::CREEPER:
                return GenerateCreeper(position, biome);
            case MobType::SPIDER:
                return GenerateSpider(position, biome);
            case MobType::PIG:
                return GeneratePig(position, biome);
            case MobType::COW:
                return GenerateCow(position, biome);
            case MobType::CHICKEN:
                return GenerateChicken(position, biome);
            case MobType::ENDERMAN:
                return GenerateEnderman(position, biome);
            case MobType::BLAZE:
                return GenerateBlaze(position, biome);
            case MobType::CUSTOM:
                return GenerateCustomMob(position, biome);
            default:
                return GenerateZombie(position, biome); // Default fallback
        }
    }

    GeneratedMob MobGenerator::GenerateRandomMob(const Vec3& position, const std::string& biome) {
        std::vector<MobType> possibleMobs = GetMobsForBiome(biome);
        if (possibleMobs.empty()) {
            return GenerateZombie(position, biome); // Default fallback
        }

        std::uniform_int_distribution<size_t> dist(0, possibleMobs.size() - 1);
        MobType selectedType = possibleMobs[dist(m_randomEngine)];

        return GenerateMob(selectedType, position, biome);
    }

    GeneratedMob MobGenerator::GenerateZombie(const Vec3& position, const std::string& biome) {
        GeneratedMob mob;
        mob.type = MobType::ZOMBIE;
        mob.name = GenerateUniqueMobName("Zombie", position);
        mob.position = position;
        mob.biome = biome;
        mob.stats = m_mobStatsTemplates[MobType::ZOMBIE];
        mob.ai = m_mobAITemplates[MobType::ZOMBIE];
        mob.spawnWeight = GetMobSpawnWeight(MobType::ZOMBIE, biome);
        mob.drops = {"rotten_flesh", "zombie_head"};

        // Generate unique ID
        std::stringstream ss;
        ss << "zombie_" << std::hex << std::hash<std::string>()(mob.name);
        mob.id = ss.str();

        // Generate appearance
        mob.appearance.scale = 1.0f + (std::uniform_real_distribution<float>(-0.1f, 0.2f)(m_randomEngine));
        mob.appearance.variant = std::uniform_int_distribution<int>(0, 3)(m_randomEngine);

        // Generate skin and sounds
        mob.skinData = GenerateMobSkin(MobType::ZOMBIE, m_seed + static_cast<uint64_t>(position.x * 100 + position.z * 200));
        mob.soundData = GenerateMobSounds(MobType::ZOMBIE, m_seed + static_cast<uint64_t>(position.x * 150 + position.z * 250));

        ApplyBiomeVariations(mob, biome);
        return mob;
    }

    GeneratedMob MobGenerator::GenerateSkeleton(const Vec3& position, const std::string& biome) {
        GeneratedMob mob;
        mob.type = MobType::SKELETON;
        mob.name = GenerateUniqueMobName("Skeleton", position);
        mob.position = position;
        mob.biome = biome;
        mob.stats = m_mobStatsTemplates[MobType::SKELETON];
        mob.ai = m_mobAITemplates[MobType::SKELETON];
        mob.spawnWeight = GetMobSpawnWeight(MobType::SKELETON, biome);
        mob.drops = {"bone", "arrow", "bow"};

        std::stringstream ss;
        ss << "skeleton_" << std::hex << std::hash<std::string>()(mob.name);
        mob.id = ss.str();

        mob.appearance.scale = 1.0f + (std::uniform_real_distribution<float>(-0.05f, 0.1f)(m_randomEngine));
        mob.appearance.variant = std::uniform_int_distribution<int>(0, 2)(m_randomEngine);

        mob.skinData = GenerateMobSkin(MobType::SKELETON, m_seed + static_cast<uint64_t>(position.x * 110 + position.z * 210));
        mob.soundData = GenerateMobSounds(MobType::SKELETON, m_seed + static_cast<uint64_t>(position.x * 160 + position.z * 260));

        ApplyBiomeVariations(mob, biome);
        return mob;
    }

    GeneratedMob MobGenerator::GenerateCreeper(const Vec3& position, const std::string& biome) {
        GeneratedMob mob;
        mob.type = MobType::CREEPER;
        mob.name = GenerateUniqueMobName("Creeper", position);
        mob.position = position;
        mob.biome = biome;
        mob.stats = m_mobStatsTemplates[MobType::CREEPER];
        mob.ai = m_mobAITemplates[MobType::CREEPER];
        mob.spawnWeight = GetMobSpawnWeight(MobType::CREEPER, biome);
        mob.drops = {"gunpowder", "creeper_head"};

        std::stringstream ss;
        ss << "creeper_" << std::hex << std::hash<std::string>()(mob.name);
        mob.id = ss.str();

        mob.appearance.scale = 1.0f + (std::uniform_real_distribution<float>(-0.1f, 0.3f)(m_randomEngine));
        mob.appearance.variant = std::uniform_int_distribution<int>(0, 2)(m_randomEngine);

        mob.skinData = GenerateMobSkin(MobType::CREEPER, m_seed + static_cast<uint64_t>(position.x * 120 + position.z * 220));
        mob.soundData = GenerateMobSounds(MobType::CREEPER, m_seed + static_cast<uint64_t>(position.x * 170 + position.z * 270));

        ApplyBiomeVariations(mob, biome);
        return mob;
    }

    GeneratedMob MobGenerator::GenerateSpider(const Vec3& position, const std::string& biome) {
        GeneratedMob mob;
        mob.type = MobType::SPIDER;
        mob.name = GenerateUniqueMobName("Spider", position);
        mob.position = position;
        mob.biome = biome;
        mob.stats = m_mobStatsTemplates[MobType::SPIDER];
        mob.ai = m_mobAITemplates[MobType::SPIDER];
        mob.spawnWeight = GetMobSpawnWeight(MobType::SPIDER, biome);
        mob.drops = {"string", "spider_eye"};

        std::stringstream ss;
        ss << "spider_" << std::hex << std::hash<std::string>()(mob.name);
        mob.id = ss.str();

        mob.appearance.scale = 1.0f + (std::uniform_real_distribution<float>(-0.1f, 0.2f)(m_randomEngine));
        mob.appearance.variant = std::uniform_int_distribution<int>(0, 3)(m_randomEngine);

        mob.skinData = GenerateMobSkin(MobType::SPIDER, m_seed + static_cast<uint64_t>(position.x * 130 + position.z * 230));
        mob.soundData = GenerateMobSounds(MobType::SPIDER, m_seed + static_cast<uint64_t>(position.x * 180 + position.z * 280));

        ApplyBiomeVariations(mob, biome);
        return mob;
    }

    GeneratedMob MobGenerator::GeneratePig(const Vec3& position, const std::string& biome) {
        GeneratedMob mob;
        mob.type = MobType::PIG;
        mob.name = GenerateUniqueMobName("Pig", position);
        mob.position = position;
        mob.biome = biome;
        mob.stats = m_mobStatsTemplates[MobType::PIG];
        mob.ai = m_mobAITemplates[MobType::PIG];
        mob.spawnWeight = GetMobSpawnWeight(MobType::PIG, biome);
        mob.drops = {"porkchop", "leather"};

        std::stringstream ss;
        ss << "pig_" << std::hex << std::hash<std::string>()(mob.name);
        mob.id = ss.str();

        mob.appearance.scale = 1.0f + (std::uniform_real_distribution<float>(-0.1f, 0.2f)(m_randomEngine));
        mob.appearance.variant = std::uniform_int_distribution<int>(0, 4)(m_randomEngine);

        mob.skinData = GenerateMobSkin(MobType::PIG, m_seed + static_cast<uint64_t>(position.x * 140 + position.z * 240));
        mob.soundData = GenerateMobSounds(MobType::PIG, m_seed + static_cast<uint64_t>(position.x * 190 + position.z * 290));

        ApplyBiomeVariations(mob, biome);
        return mob;
    }

    GeneratedMob MobGenerator::GenerateCow(const Vec3& position, const std::string& biome) {
        GeneratedMob mob;
        mob.type = MobType::COW;
        mob.name = GenerateUniqueMobName("Cow", position);
        mob.position = position;
        mob.biome = biome;
        mob.stats = m_mobStatsTemplates[MobType::COW];
        mob.ai = m_mobAITemplates[MobType::COW];
        mob.spawnWeight = GetMobSpawnWeight(MobType::COW, biome);
        mob.drops = {"beef", "leather", "milk_bucket"};

        std::stringstream ss;
        ss << "cow_" << std::hex << std::hash<std::string>()(mob.name);
        mob.id = ss.str();

        mob.appearance.scale = 1.0f + (std::uniform_real_distribution<float>(-0.1f, 0.2f)(m_randomEngine));
        mob.appearance.variant = std::uniform_int_distribution<int>(0, 3)(m_randomEngine);

        mob.skinData = GenerateMobSkin(MobType::COW, m_seed + static_cast<uint64_t>(position.x * 150 + position.z * 250));
        mob.soundData = GenerateMobSounds(MobType::COW, m_seed + static_cast<uint64_t>(position.x * 200 + position.z * 300));

        ApplyBiomeVariations(mob, biome);
        return mob;
    }

    GeneratedMob MobGenerator::GenerateChicken(const Vec3& position, const std::string& biome) {
        GeneratedMob mob;
        mob.type = MobType::CHICKEN;
        mob.name = GenerateUniqueMobName("Chicken", position);
        mob.position = position;
        mob.biome = biome;
        mob.stats = m_mobStatsTemplates[MobType::CHICKEN];
        mob.ai = m_mobAITemplates[MobType::CHICKEN];
        mob.spawnWeight = GetMobSpawnWeight(MobType::CHICKEN, biome);
        mob.drops = {"chicken", "feather", "egg"};

        std::stringstream ss;
        ss << "chicken_" << std::hex << std::hash<std::string>()(mob.name);
        mob.id = ss.str();

        mob.appearance.scale = 1.0f + (std::uniform_real_distribution<float>(-0.15f, 0.1f)(m_randomEngine));
        mob.appearance.variant = std::uniform_int_distribution<int>(0, 5)(m_randomEngine);

        mob.skinData = GenerateMobSkin(MobType::CHICKEN, m_seed + static_cast<uint64_t>(position.x * 160 + position.z * 260));
        mob.soundData = GenerateMobSounds(MobType::CHICKEN, m_seed + static_cast<uint64_t>(position.x * 210 + position.z * 310));

        ApplyBiomeVariations(mob, biome);
        return mob;
    }

    GeneratedMob MobGenerator::GenerateEnderman(const Vec3& position, const std::string& biome) {
        GeneratedMob mob;
        mob.type = MobType::ENDERMAN;
        mob.name = GenerateUniqueMobName("Enderman", position);
        mob.position = position;
        mob.biome = biome;
        mob.stats = m_mobStatsTemplates[MobType::ENDERMAN];
        mob.ai = m_mobAITemplates[MobType::ENDERMAN];
        mob.spawnWeight = GetMobSpawnWeight(MobType::ENDERMAN, biome);
        mob.drops = {"ender_pearl", "enderman_head"};

        std::stringstream ss;
        ss << "enderman_" << std::hex << std::hash<std::string>()(mob.name);
        mob.id = ss.str();

        mob.appearance.scale = 1.0f + (std::uniform_real_distribution<float>(-0.05f, 0.15f)(m_randomEngine));
        mob.appearance.variant = std::uniform_int_distribution<int>(0, 2)(m_randomEngine);

        mob.skinData = GenerateMobSkin(MobType::ENDERMAN, m_seed + static_cast<uint64_t>(position.x * 170 + position.z * 270));
        mob.soundData = GenerateMobSounds(MobType::ENDERMAN, m_seed + static_cast<uint64_t>(position.x * 220 + position.z * 320));

        ApplyBiomeVariations(mob, biome);
        return mob;
    }

    GeneratedMob MobGenerator::GenerateBlaze(const Vec3& position, const std::string& biome) {
        GeneratedMob mob;
        mob.type = MobType::BLAZE;
        mob.name = GenerateUniqueMobName("Blaze", position);
        mob.position = position;
        mob.biome = biome;
        mob.stats = m_mobStatsTemplates[MobType::BLAZE];
        mob.ai = m_mobAITemplates[MobType::BLAZE];
        mob.spawnWeight = GetMobSpawnWeight(MobType::BLAZE, biome);
        mob.drops = {"blaze_rod", "blaze_powder"};

        std::stringstream ss;
        ss << "blaze_" << std::hex << std::hash<std::string>()(mob.name);
        mob.id = ss.str();

        mob.appearance.scale = 1.0f + (std::uniform_real_distribution<float>(-0.05f, 0.1f)(m_randomEngine));
        mob.appearance.variant = std::uniform_int_distribution<int>(0, 2)(m_randomEngine);

        mob.skinData = GenerateMobSkin(MobType::BLAZE, m_seed + static_cast<uint64_t>(position.x * 180 + position.z * 280));
        mob.soundData = GenerateMobSounds(MobType::BLAZE, m_seed + static_cast<uint64_t>(position.x * 230 + position.z * 330));

        ApplyBiomeVariations(mob, biome);
        return mob;
    }

    GeneratedMob MobGenerator::GenerateCustomMob(const Vec3& position, const std::string& biome) {
        GeneratedMob mob;
        mob.type = MobType::CUSTOM;
        mob.name = GenerateUniqueMobName("Custom", position);
        mob.position = position;
        mob.biome = biome;

        // Generate random stats
        mob.stats.health = std::uniform_real_distribution<float>(5.0f, 50.0f)(m_randomEngine);
        mob.stats.attackDamage = std::uniform_real_distribution<float>(0.0f, 10.0f)(m_randomEngine);
        mob.stats.movementSpeed = std::uniform_real_distribution<float>(0.1f, 0.5f)(m_randomEngine);
        mob.stats.experienceValue = std::uniform_int_distribution<int>(1, 20)(m_randomEngine);

        // Random behavior
        std::vector<MobBehavior> behaviors = {MobBehavior::PASSIVE, MobBehavior::NEUTRAL, MobBehavior::HOSTILE};
        mob.ai.behavior = behaviors[std::uniform_int_distribution<size_t>(0, behaviors.size() - 1)(m_randomEngine)];

        mob.spawnWeight = std::uniform_int_distribution<int>(50, 200)(m_randomEngine);
        mob.drops = {"custom_drop"};

        std::stringstream ss;
        ss << "custom_" << std::hex << std::hash<std::string>()(mob.name);
        mob.id = ss.str();

        mob.appearance.scale = std::uniform_real_distribution<float>(0.8f, 1.5f)(m_randomEngine);
        mob.appearance.variant = std::uniform_int_distribution<int>(0, 10)(m_randomEngine);

        mob.skinData = GenerateMobSkin(MobType::CUSTOM, m_seed + static_cast<uint64_t>(position.x * 190 + position.z * 290));
        mob.soundData = GenerateMobSounds(MobType::CUSTOM, m_seed + static_cast<uint64_t>(position.x * 240 + position.z * 340));

        ApplyBiomeVariations(mob, biome);
        return mob;
    }

    void MobGenerator::ApplyBiomeVariations(GeneratedMob& mob, const std::string& biome) {
        if (biome == "snow") {
            // Snow biomes make mobs tougher
            mob.stats.health *= 1.2f;
            mob.stats.attackDamage *= 1.1f;
        } else if (biome == "desert") {
            // Desert biomes make mobs faster
            mob.stats.movementSpeed *= 1.1f;
        } else if (biome == "forest") {
            // Forest biomes add camouflage
            mob.appearance.specialFeatures.push_back("forest_camouflage");
        }
    }

    std::string MobGenerator::GenerateUniqueMobName(const std::string& baseName, const Vec3& position) {
        uint64_t positionHash = static_cast<uint64_t>(position.x * 1000 + position.y * 100 + position.z * 10);
        std::mt19937_64 nameGen(m_seed + positionHash);
        std::uniform_int_distribution<int> nameDist(1000, 9999);

        std::stringstream ss;
        ss << baseName << "_" << nameDist(nameGen);
        return ss.str();
    }

    void* MobGenerator::GenerateMobSkin(MobType mobType, uint64_t seed) {
        auto it = m_skinGenerators.find(mobType);
        if (it != m_skinGenerators.end()) {
            return it->second(seed);
        }
        return nullptr;
    }

    void* MobGenerator::GenerateMobSounds(MobType mobType, uint64_t seed) {
        auto it = m_soundGenerators.find(mobType);
        if (it != m_soundGenerators.end()) {
            return it->second(seed);
        }
        return nullptr;
    }

    std::shared_ptr<Entity> MobGenerator::CreateEntityFromMob(const GeneratedMob& mob, EntityManager* entityManager) {
        auto entity = entityManager->CreateEntity(mob.name);

        // Add transform component
        auto transform = entity->AddComponent<TransformComponent>();
        transform->SetPosition(mob.position);
        transform->SetScale(Vec3(mob.appearance.scale, mob.appearance.scale, mob.appearance.scale));

        // Add physics component
        auto physics = entity->AddComponent<PhysicsComponent>();
        physics->SetMass(mob.appearance.scale * 50.0f); // Scale mass with size
        physics->SetVelocity(Vec3(0, 0, 0));

        // Add mob-specific component
        // (This would be a custom component for mob behavior)

        return entity;
    }

    int MobGenerator::GetMobSpawnWeight(MobType mobType, const std::string& biome) const {
        // Base weights
        std::unordered_map<MobType, int> baseWeights = {
            {MobType::ZOMBIE, 100},
            {MobType::SKELETON, 100},
            {MobType::CREEPER, 100},
            {MobType::SPIDER, 100},
            {MobType::PIG, 100},
            {MobType::COW, 100},
            {MobType::CHICKEN, 100},
            {MobType::ENDERMAN, 10}, // Rare
            {MobType::BLAZE, 10}     // Rare
        };

        int baseWeight = baseWeights.count(mobType) ? baseWeights.at(mobType) : 50;

        // Biome modifiers
        if (biome == "plains" && (mobType == MobType::COW || mobType == MobType::PIG || mobType == MobType::CHICKEN)) {
            baseWeight *= 2; // More passive mobs in plains
        } else if (biome == "forest" && (mobType == MobType::SPIDER || mobType == MobType::ZOMBIE)) {
            baseWeight *= 2; // More hostile mobs in forests
        } else if (biome == "desert" && mobType == MobType::SKELETON) {
            baseWeight *= 3; // Many skeletons in deserts
        } else if (biome == "snow" && mobType == MobType::ZOMBIE) {
            baseWeight *= 2; // More zombies in snow
        }

        return baseWeight;
    }

    bool MobGenerator::CanMobSpawnAt(MobType mobType, const Vec3& position, const std::string& biome) const {
        // Check light levels for undead mobs
        if ((mobType == MobType::ZOMBIE || mobType == MobType::SKELETON) && biome != "nether") {
            // These mobs avoid sunlight (would need light level check)
            return true; // Simplified for now
        }

        // Check if mob type is appropriate for biome
        if (mobType == MobType::BLAZE && biome != "nether") {
            return false; // Blazes only in nether
        }

        if (mobType == MobType::ENDERMAN && biome == "end") {
            return false; // Endermen don't spawn in end
        }

        return true;
    }

    std::vector<MobType> MobGenerator::GetMobsForBiome(const std::string& biome) {
        std::vector<MobType> mobs;

        if (biome == "plains") {
            mobs = {MobType::ZOMBIE, MobType::SKELETON, MobType::CREEPER, MobType::SPIDER,
                   MobType::COW, MobType::PIG, MobType::CHICKEN};
        } else if (biome == "forest") {
            mobs = {MobType::ZOMBIE, MobType::SKELETON, MobType::CREEPER, MobType::SPIDER,
                   MobType::ENDERMAN, MobType::COW, MobType::PIG, MobType::CHICKEN};
        } else if (biome == "desert") {
            mobs = {MobType::ZOMBIE, MobType::SKELETON, MobType::CREEPER};
        } else if (biome == "snow") {
            mobs = {MobType::ZOMBIE, MobType::SKELETON, MobType::CREEPER};
        } else if (biome == "nether") {
            mobs = {MobType::BLAZE};
        } else {
            // Default mobs for unknown biomes
            mobs = {MobType::ZOMBIE, MobType::SKELETON, MobType::COW, MobType::PIG};
        }

        return mobs;
    }

    // Skin generation implementations
    void* MobGenerator::GenerateZombieSkin(uint64_t seed) {
        std::mt19937_64 gen(seed);
        std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);
        std::uniform_int_distribution<int> intDist(0, 10);

        ZombieSkin* skin = new ZombieSkin();
        skin->decayLevel = colorDist(gen) * 0.8f; // 0-0.8 decay
        skin->skinColor = Vec3(0.3f + colorDist(gen) * 0.3f, 0.2f + colorDist(gen) * 0.2f, 0.1f + colorDist(gen) * 0.1f);
        skin->decayColor = Vec3(colorDist(gen) * 0.5f, colorDist(gen) * 0.3f, colorDist(gen) * 0.2f);
        skin->missingLimbs = intDist(gen) % 4; // 0-3 missing limbs
        skin->hasHelmet = (intDist(gen) % 10) < 2; // 20% chance
        skin->hasArmor = (intDist(gen) % 10) < 3; // 30% chance
        skin->variant = intDist(gen) % 4;

        return skin;
    }

    void* MobGenerator::GenerateSkeletonSkin(uint64_t seed) {
        std::mt19937_64 gen(seed);
        std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);
        std::uniform_int_distribution<int> intDist(0, 10);

        SkeletonSkin* skin = new SkeletonSkin();
        skin->boneCondition = colorDist(gen); // 0-1 condition
        skin->boneColor = Vec3(0.8f + colorDist(gen) * 0.2f, 0.8f + colorDist(gen) * 0.2f, 0.8f + colorDist(gen) * 0.2f);
        skin->hasHelmet = (intDist(gen) % 10) < 3; // 30% chance
        skin->hasArmor = (intDist(gen) % 10) < 2; // 20% chance
        skin->bowType = intDist(gen) % 3;
        skin->variant = intDist(gen) % 3;

        return skin;
    }

    void* MobGenerator::GenerateCreeperSkin(uint64_t seed) {
        std::mt19937_64 gen(seed);
        std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);
        std::uniform_int_distribution<int> intDist(0, 10);

        CreeperSkin* skin = new CreeperSkin();
        skin->size = 1.0f + (colorDist(gen) - 0.5f) * 0.4f; // 0.8-1.2
        skin->bodyColor = Vec3(0.2f + colorDist(gen) * 0.3f, 0.8f + colorDist(gen) * 0.2f, 0.2f + colorDist(gen) * 0.3f);
        skin->stripeColor = Vec3(0.1f, 0.1f, 0.1f);
        skin->pattern = intDist(gen) % 4;
        skin->isCharged = (intDist(gen) % 10) < 1; // 10% chance
        skin->variant = intDist(gen) % 3;

        return skin;
    }

    void* MobGenerator::GenerateSpiderSkin(uint64_t seed) {
        std::mt19937_64 gen(seed);
        std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);
        std::uniform_int_distribution<int> intDist(0, 10);

        SpiderSkin* skin = new SpiderSkin();
        skin->size = 1.0f + (colorDist(gen) - 0.5f) * 0.4f; // 0.8-1.2
        skin->bodyColor = Vec3(0.1f + colorDist(gen) * 0.2f, 0.1f + colorDist(gen) * 0.2f, 0.1f + colorDist(gen) * 0.2f);
        skin->eyeColor = Vec3(0.8f + colorDist(gen) * 0.2f, 0.1f, 0.1f);
        skin->eyeCount = 6 + intDist(gen) % 3; // 6-8 eyes
        skin->pattern = intDist(gen) % 4;
        skin->variant = intDist(gen) % 4;

        return skin;
    }

    void* MobGenerator::GeneratePigSkin(uint64_t seed) {
        std::mt19937_64 gen(seed);
        std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);
        std::uniform_int_distribution<int> intDist(0, 10);

        PigSkin* skin = new PigSkin();
        skin->furColor = Vec3(0.8f + colorDist(gen) * 0.2f, 0.6f + colorDist(gen) * 0.3f, 0.5f + colorDist(gen) * 0.2f);
        skin->spotDensity = intDist(gen) % 11; // 0-10
        skin->spotColor = Vec3(0.6f + colorDist(gen) * 0.3f, 0.4f + colorDist(gen) * 0.3f, 0.3f + colorDist(gen) * 0.2f);
        skin->hasSaddle = (intDist(gen) % 10) < 1; // 10% chance
        skin->size = 1.0f + (colorDist(gen) - 0.5f) * 0.4f; // 0.8-1.2
        skin->variant = intDist(gen) % 5;

        return skin;
    }

    void* MobGenerator::GenerateCowSkin(uint64_t seed) {
        std::mt19937_64 gen(seed);
        std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);
        std::uniform_int_distribution<int> intDist(0, 10);

        CowSkin* skin = new CowSkin();
        skin->furColor = Vec3(0.7f + colorDist(gen) * 0.3f, 0.6f + colorDist(gen) * 0.3f, 0.5f + colorDist(gen) * 0.2f);
        skin->spotPattern = intDist(gen) % 5;
        skin->spotColor = Vec3(0.2f + colorDist(gen) * 0.3f, 0.2f + colorDist(gen) * 0.3f, 0.2f + colorDist(gen) * 0.2f);
        skin->spotDensity = colorDist(gen);
        skin->variant = intDist(gen) % 4;

        return skin;
    }

    void* MobGenerator::GenerateChickenSkin(uint64_t seed) {
        std::mt19937_64 gen(seed);
        std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);
        std::uniform_int_distribution<int> intDist(0, 10);

        ChickenSkin* skin = new ChickenSkin();
        skin->featherColor = Vec3(0.9f + colorDist(gen) * 0.1f, 0.9f + colorDist(gen) * 0.1f, 0.8f + colorDist(gen) * 0.2f);
        skin->combColor = Vec3(0.8f + colorDist(gen) * 0.2f, 0.3f + colorDist(gen) * 0.4f, 0.2f + colorDist(gen) * 0.3f);
        skin->featherPattern = intDist(gen) % 4;
        skin->combSize = 0.8f + colorDist(gen) * 0.4f; // 0.8-1.2
        skin->variant = intDist(gen) % 6;

        return skin;
    }

    void* MobGenerator::GenerateEndermanSkin(uint64_t seed) {
        std::mt19937_64 gen(seed);
        std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);
        std::uniform_int_distribution<int> intDist(0, 10);

        EndermanSkin* skin = new EndermanSkin();
        skin->eyeColor = Vec3(colorDist(gen), colorDist(gen), colorDist(gen)); // Random eye color
        skin->bodyColor = Vec3(0.1f, 0.1f, 0.1f); // Black body
        skin->eyePattern = intDist(gen) % 4;
        skin->isProvoked = false; // Default to not provoked
        skin->variant = intDist(gen) % 3;

        return skin;
    }

    void* MobGenerator::GenerateBlazeSkin(uint64_t seed) {
        std::mt19937_64 gen(seed);
        std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);
        std::uniform_int_distribution<int> intDist(0, 10);

        BlazeSkin* skin = new BlazeSkin();
        skin->flameColor = Vec3(1.0f, 0.5f + colorDist(gen) * 0.5f, 0.1f + colorDist(gen) * 0.3f);
        skin->flamePattern = intDist(gen) % 4;
        skin->rodCount = 8 + intDist(gen) % 5; // 8-12 rods
        skin->intensity = 0.8f + colorDist(gen) * 0.4f; // 0.8-1.2
        skin->variant = intDist(gen) % 3;

        return skin;
    }

    void* MobGenerator::GenerateCustomSkin(uint64_t seed) {
        std::mt19937_64 gen(seed);
        std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);
        std::uniform_int_distribution<int> intDist(0, 10);

        CustomSkin* skin = new CustomSkin();
        skin->primaryColor = Vec3(colorDist(gen), colorDist(gen), colorDist(gen));
        skin->secondaryColor = Vec3(colorDist(gen), colorDist(gen), colorDist(gen));
        skin->accentColor = Vec3(colorDist(gen), colorDist(gen), colorDist(gen));
        skin->shapeType = intDist(gen) % 5;
        skin->featureCount = intDist(gen) % 6; // 0-5 features
        skin->variant = intDist(gen) % 11; // 0-10 variants

        // Generate random features
        std::vector<std::string> possibleFeatures = {
            "spikes", "tentacles", "wings", "glowing_eyes", "crystals", "fur", "scales", "feathers"
        };

        for (int i = 0; i < skin->featureCount; ++i) {
            int featureIndex = intDist(gen) % possibleFeatures.size();
            skin->features.push_back(possibleFeatures[featureIndex]);
        }

        return skin;
    }

    // Sound generation implementations (simplified)
    void* MobGenerator::GenerateZombieSounds(uint64_t seed) {
        MobSounds* sounds = new MobSounds();
        sounds->volume = 1.0f;
        sounds->pitch = 0.9f + (seed % 100) / 100.0f * 0.4f; // 0.9-1.3
        sounds->variant = seed % 3;
        return sounds;
    }

    void* MobGenerator::GenerateSkeletonSounds(uint64_t seed) {
        MobSounds* sounds = new MobSounds();
        sounds->volume = 1.0f;
        sounds->pitch = 1.1f + (seed % 100) / 100.0f * 0.3f; // 1.1-1.4
        sounds->variant = seed % 2;
        return sounds;
    }

    void* MobGenerator::GenerateCreeperSounds(uint64_t seed) {
        MobSounds* sounds = new MobSounds();
        sounds->volume = 1.0f;
        sounds->pitch = 0.8f + (seed % 100) / 100.0f * 0.4f; // 0.8-1.2
        sounds->variant = seed % 2;
        return sounds;
    }

    void* MobGenerator::GenerateSpiderSounds(uint64_t seed) {
        MobSounds* sounds = new MobSounds();
        sounds->volume = 0.8f;
        sounds->pitch = 1.2f + (seed % 100) / 100.0f * 0.4f; // 1.2-1.6
        sounds->variant = seed % 3;
        return sounds;
    }

    void* MobGenerator::GeneratePigSounds(uint64_t seed) {
        MobSounds* sounds = new MobSounds();
        sounds->volume = 1.0f;
        sounds->pitch = 0.8f + (seed % 100) / 100.0f * 0.8f; // 0.8-1.6
        sounds->variant = seed % 4;
        return sounds;
    }

    void* MobGenerator::GenerateCowSounds(uint64_t seed) {
        MobSounds* sounds = new MobSounds();
        sounds->volume = 1.0f;
        sounds->pitch = 0.7f + (seed % 100) / 100.0f * 0.6f; // 0.7-1.3
        sounds->variant = seed % 3;
        return sounds;
    }

    void* MobGenerator::GenerateChickenSounds(uint64_t seed) {
        MobSounds* sounds = new MobSounds();
        sounds->volume = 0.8f;
        sounds->pitch = 1.3f + (seed % 100) / 100.0f * 0.7f; // 1.3-2.0
        sounds->variant = seed % 5;
        return sounds;
    }

    void* MobGenerator::GenerateEndermanSounds(uint64_t seed) {
        MobSounds* sounds = new MobSounds();
        sounds->volume = 0.8f;
        sounds->pitch = 0.5f + (seed % 100) / 100.0f * 0.5f; // 0.5-1.0
        sounds->variant = seed % 2;
        return sounds;
    }

    void* MobGenerator::GenerateBlazeSounds(uint64_t seed) {
        MobSounds* sounds = new MobSounds();
        sounds->volume = 1.0f;
        sounds->pitch = 0.8f + (seed % 100) / 100.0f * 0.4f; // 0.8-1.2
        sounds->variant = seed % 2;
        return sounds;
    }

    void* MobGenerator::GenerateCustomSounds(uint64_t seed) {
        MobSounds* sounds = new MobSounds();
        sounds->volume = 0.8f + (seed % 100) / 100.0f * 0.4f; // 0.8-1.2
        sounds->pitch = 0.5f + (seed % 100) / 100.0f * 1.5f; // 0.5-2.0
        sounds->variant = seed % 5;
        return sounds;
    }

} // namespace VoxelCraft
