/**
 * @file GameConstants.hpp
 * @brief VoxelCraft Game Constants - Minecraft-like Values
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_CORE_GAME_CONSTANTS_HPP
#define VOXELCRAFT_CORE_GAME_CONSTANTS_HPP

namespace VoxelCraft {

    /**
     * @namespace GameConstants
     * @brief Game constants matching Minecraft mechanics
     */
    namespace GameConstants {

        // World Constants
        constexpr int CHUNK_SIZE = 16;                    ///< Standard Minecraft chunk size
        constexpr int WORLD_HEIGHT = 384;                 ///< Total world height (from -64 to 320)
        constexpr int MIN_WORLD_HEIGHT = -64;             ///< Minimum world height
        constexpr int MAX_WORLD_HEIGHT = 320;             ///< Maximum world height
        constexpr int SEA_LEVEL = 63;                     ///< Sea level height
        constexpr int CAVE_MIN_HEIGHT = -59;              ///< Minimum cave generation height
        constexpr int CAVE_MAX_HEIGHT = 320;              ///< Maximum cave generation height

        // Player Constants (matching Minecraft physics)
        constexpr float PLAYER_EYE_HEIGHT = 1.62f;        ///< Player eye height
        constexpr float PLAYER_HEIGHT = 1.8f;             ///< Player total height
        constexpr float PLAYER_WIDTH = 0.6f;              ///< Player collision width
        constexpr float PLAYER_WALK_SPEED = 4.317f;       ///< Walking speed (m/s)
        constexpr float PLAYER_SPRINT_SPEED = 5.612f;     ///< Sprinting speed (m/s)
        constexpr float PLAYER_SNEAK_SPEED = 1.295f;      ///< Sneaking speed (m/s)
        constexpr float PLAYER_FLY_SPEED = 10.89f;        ///< Flying speed (m/s)
        constexpr float PLAYER_JUMP_VELOCITY = 8.0f;      ///< Jump velocity
        constexpr float PLAYER_GRAVITY = 32.0f;           ///< Gravity acceleration (m/s²)
        constexpr float PLAYER_MAX_HEALTH = 20.0f;        ///< Maximum player health
        constexpr float PLAYER_MAX_FALL_DISTANCE = 3.0f;  ///< Safe fall distance (blocks)

        // Physics Constants
        constexpr float GRAVITY = 32.0f;                  ///< World gravity (m/s²)
        constexpr float TERMINAL_VELOCITY = 78.4f;        ///< Maximum fall speed (m/s)
        constexpr float FRICTION = 0.91f;                 ///< Ground friction
        constexpr float AIR_FRICTION = 0.98f;             ///< Air friction
        constexpr float WATER_FRICTION = 0.8f;            ///< Water friction
        constexpr float LAVA_FRICTION = 0.5f;             ///< Lava friction

        // Block Constants
        constexpr float BLOCK_SIZE = 1.0f;                ///< Size of one block
        constexpr int TICKS_PER_SECOND = 20;              ///< Game ticks per second
        constexpr float TICK_DELTA_TIME = 1.0f / TICKS_PER_SECOND; ///< Time per tick

        // Mining Constants
        constexpr float MINING_SPEED_AIR = 1.0f;          ///< Mining speed in air
        constexpr float MINING_SPEED_WATER = 0.2f;        ///< Mining speed underwater
        constexpr float MINING_SPEED_LAVA = 0.2f;         ///< Mining speed in lava

        // Tool Multipliers (matching Minecraft)
        constexpr float TOOL_MULTIPLIER_WOOD = 2.0f;      ///< Wooden tool multiplier
        constexpr float TOOL_MULTIPLIER_STONE = 4.0f;     ///< Stone tool multiplier
        constexpr float TOOL_MULTIPLIER_IRON = 6.0f;      ///< Iron tool multiplier
        constexpr float TOOL_MULTIPLIER_DIAMOND = 8.0f;   ///< Diamond tool multiplier
        constexpr float TOOL_MULTIPLIER_GOLD = 12.0f;     ///< Gold tool multiplier
        constexpr float TOOL_MULTIPLIER_NETHERITE = 9.0f; ///< Netherite tool multiplier

        // Enchantment Constants
        constexpr float EFFICIENCY_MULTIPLIER = 0.25f;    ///< Efficiency level multiplier
        constexpr float FORTUNE_MULTIPLIER = 1.0f;        ///< Base fortune multiplier
        constexpr float SILK_TOUCH_CHANCE = 1.0f;         ///< Silk touch success chance

        // Item Stack Constants
        constexpr int MAX_STACK_SIZE = 64;                ///< Maximum stack size for most items
        constexpr int MAX_STACK_SIZE_TOOLS = 1;           ///< Stack size for tools
        constexpr int MAX_STACK_SIZE_ARMOR = 1;           ///< Stack size for armor

        // Inventory Constants
        constexpr int PLAYER_INVENTORY_SIZE = 41;         ///< Player inventory slots (36 + 4 armor + 1 offhand)
        constexpr int HOTBAR_SIZE = 9;                    ///< Hotbar slots
        constexpr int CRAFTING_GRID_SIZE = 9;             ///< Crafting grid slots (3x3)

        // Rendering Constants
        constexpr int DEFAULT_RENDER_DISTANCE = 8;        ///< Default render distance (chunks)
        constexpr int MIN_RENDER_DISTANCE = 2;            ///< Minimum render distance
        constexpr int MAX_RENDER_DISTANCE = 32;           ///< Maximum render distance
        constexpr int SIMULATION_DISTANCE = 6;            ///< Entity simulation distance (chunks)
        constexpr float FOV_DEFAULT = 70.0f;              ///< Default field of view
        constexpr float FOV_MIN = 30.0f;                  ///< Minimum field of view
        constexpr float FOV_MAX = 110.0f;                 ///< Maximum field of view

        // Lighting Constants
        constexpr int MAX_LIGHT_LEVEL = 15;               ///< Maximum light level
        constexpr int MIN_LIGHT_LEVEL = 0;                ///< Minimum light level
        constexpr float LIGHT_ATTENUATION = 0.0625f;      ///< Light attenuation per block
        constexpr int SKY_LIGHT_LEVEL = 15;               ///< Full daylight level
        constexpr int NETHER_LIGHT_LEVEL = 8;             ///< Nether ambient light level

        // Time Constants
        constexpr int DAY_LENGTH = 24000;                 ///< Length of a Minecraft day (ticks)
        constexpr int HALF_DAY = DAY_LENGTH / 2;          ///< Half day length
        constexpr int SUNRISE_START = 23000;              ///< Sunrise start time
        constexpr int SUNRISE_END = 24000;                ///< Sunrise end time (dawn)
        constexpr int SUNSET_START = 11000;               ///< Sunset start time
        constexpr int SUNSET_END = 13000;                 ///< Sunset end time (dusk)
        constexpr int MIDNIGHT = 18000;                   ///< Midnight time
        constexpr int NOON = 6000;                        ///< Noon time

        // Weather Constants
        constexpr float RAIN_CHANCE_CLEAR = 0.001f;       ///< Rain chance when clear
        constexpr float RAIN_CHANCE_RAINING = 0.01f;      ///< Rain chance when already raining
        constexpr float THUNDER_CHANCE_RAINING = 0.1f;    ///< Thunder chance when raining
        constexpr int MIN_RAIN_TIME = 6000;               ///< Minimum rain duration (ticks)
        constexpr int MAX_RAIN_TIME = 24000;              ///< Maximum rain duration (ticks)
        constexpr int MIN_THUNDER_TIME = 3000;            ///< Minimum thunder duration (ticks)
        constexpr int MAX_THUNDER_TIME = 18000;           ///< Maximum thunder duration (ticks)

        // Entity Constants
        constexpr float ENTITY_DESPAWN_DISTANCE = 128.0f; ///< Entity despawn distance
        constexpr float ENTITY_TRACKING_DISTANCE = 64.0f; ///< Entity tracking distance
        constexpr int MAX_ENTITIES_PER_CHUNK = 16;        ///< Maximum entities per chunk
        constexpr int MAX_MOB_SPAWN_ATTEMPTS = 3;         ///< Max spawn attempts per tick

        // Network Constants (for multiplayer)
        constexpr int DEFAULT_SERVER_PORT = 25565;       ///< Default Minecraft server port
        constexpr int MAX_PLAYERS_PER_SERVER = 20;       ///< Maximum players per server
        constexpr float NETWORK_UPDATE_RATE = 20.0f;     ///< Network updates per second
        constexpr int NETWORK_TIMEOUT = 30000;           ///< Connection timeout (ms)

        // Debug Constants
        constexpr bool ENABLE_DEBUG_OVERLAY = false;     ///< Enable debug overlay by default
        constexpr bool ENABLE_CHUNK_BORDERS = false;     ///< Show chunk borders
        constexpr bool ENABLE_LIGHT_DEBUG = false;       ///< Show light levels
        constexpr bool ENABLE_PROFILING = false;         ///< Enable performance profiling

    } // namespace GameConstants

    /**
     * @namespace BiomeConstants
     * @brief Biome-specific constants
     */
    namespace BiomeConstants {

        // Temperature ranges
        constexpr float MIN_TEMPERATURE = -1.0f;          ///< Minimum biome temperature
        constexpr float MAX_TEMPERATURE = 2.0f;           ///< Maximum biome temperature
        constexpr float FROZEN_TEMPERATURE = 0.15f;       ///< Temperature below which water freezes
        constexpr float HOT_TEMPERATURE = 0.95f;          ///< Temperature above which water evaporates

        // Humidity ranges
        constexpr float MIN_HUMIDITY = 0.0f;              ///< Minimum biome humidity
        constexpr float MAX_HUMIDITY = 1.0f;              ///< Maximum biome humidity
        constexpr float DRY_HUMIDITY = 0.3f;              ///< Humidity below which desert forms
        constexpr float WET_HUMIDITY = 0.8f;              ///< Humidity above which swamp forms

        // Height ranges
        constexpr float MIN_HEIGHT = -2.0f;               ///< Minimum biome height
        constexpr float MAX_HEIGHT = 2.0f;                ///< Maximum biome height
        constexpr float OCEAN_HEIGHT = -1.0f;             ///< Height below which ocean forms
        constexpr float MOUNTAIN_HEIGHT = 1.0f;           ///< Height above which mountains form

    } // namespace BiomeConstants

} // namespace VoxelCraft

#endif // VOXELCRAFT_CORE_GAME_CONSTANTS_HPP
