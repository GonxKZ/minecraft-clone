/**
 * @file RedstoneComponent.hpp
 * @brief VoxelCraft Redstone System - Redstone Components and Logic
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_REDSTONE_REDSTONE_COMPONENT_HPP
#define VOXELCRAFT_REDSTONE_REDSTONE_COMPONENT_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <queue>
#include <functional>
#include <chrono>
#include <glm/glm.hpp>

namespace VoxelCraft {

    class World;
    class Chunk;
    struct Vec3;

    /**
     * @enum RedstoneType
     * @brief Types of redstone components
     */
    enum class RedstoneType {
        WIRE = 0,           ///< Redstone wire/dust
        TORCH,              ///< Redstone torch
        REPEATER,           ///< Redstone repeater
        COMPARATOR,         ///< Redstone comparator
        LAMP,               ///< Redstone lamp
        BLOCK,              ///< Redstone block
        PISTON,             ///< Piston
        STICKY_PISTON,      ///< Sticky piston
        DOOR,               ///< Redstone-activated door
        TRAPDOOR,           ///< Redstone-activated trapdoor
        FENCE_GATE,         ///< Redstone-activated fence gate
        NOTE_BLOCK,         ///< Note block
        DISPENSER,          ///< Dispenser
        DROPPER,            ///< Dropper
        HOPPER,             ///< Hopper
        OBSERVER,           ///< Observer block
        TARGET,             ///< Target block
        LEVER,              ///< Lever
        BUTTON,             ///< Button
        PRESSURE_PLATE,     ///< Pressure plate
        TRIPWIRE,           ///< Tripwire
        RAIL,               ///< Powered rail
        TNT,                ///< TNT
        CUSTOM              ///< Custom redstone component
    };

    /**
     * @enum RedstoneState
     * @brief Power state of redstone components
     */
    enum class RedstoneState {
        OFF = 0,            ///< Not powered
        POWERING,           ///< Currently powering up
        ON,                 ///< Powered and stable
        UNPOWERING          ///< Currently powering down
    };

    /**
     * @enum PistonState
     * @brief State of piston components
     */
    enum class PistonState {
        RETRACTED = 0,      ///< Piston head retracted
        EXTENDING,          ///< Piston extending
        EXTENDED,           ///< Piston head extended
        RETRACTING          ///< Piston retracting
    };

    /**
     * @struct RedstoneSignal
     * @brief Redstone signal information
     */
    struct RedstoneSignal {
        int powerLevel;             ///< Power level (0-15)
        glm::ivec3 sourcePos;       ///< Source position
        glm::ivec3 targetPos;       ///< Target position
        RedstoneType sourceType;    ///< Type of source component
        std::chrono::steady_clock::time_point timestamp;
        bool isStrong;              ///< Strong vs weak power

        RedstoneSignal()
            : powerLevel(0)
            , sourceType(RedstoneType::WIRE)
            , isStrong(false)
            , timestamp(std::chrono::steady_clock::now())
        {}
    };

    /**
     * @struct RedstoneConnection
     * @brief Connection between redstone components
     */
    struct RedstoneConnection {
        glm::ivec3 fromPos;
        glm::ivec3 toPos;
        int powerLevel;
        bool isLocked;
        std::chrono::steady_clock::time_point lastUpdate;

        RedstoneConnection()
            : powerLevel(0)
            , isLocked(false)
            , lastUpdate(std::chrono::steady_clock::now())
        {}
    };

    /**
     * @class RedstoneComponent
     * @brief Base class for all redstone components
     */
    class RedstoneComponent {
    public:
        /**
         * @brief Constructor
         * @param type Component type
         * @param position World position
         * @param world World reference
         */
        RedstoneComponent(RedstoneType type, const glm::ivec3& position, World* world);

        /**
         * @brief Destructor
         */
        virtual ~RedstoneComponent() = default;

        /**
         * @brief Update component logic
         * @param deltaTime Time since last update
         */
        virtual void Update(float deltaTime) = 0;

        /**
         * @brief Get component type
         * @return Redstone type
         */
        RedstoneType GetType() const { return m_type; }

        /**
         * @brief Get component position
         * @return World position
         */
        const glm::ivec3& GetPosition() const { return m_position; }

        /**
         * @brief Get current power level
         * @return Power level (0-15)
         */
        int GetPowerLevel() const { return m_powerLevel; }

        /**
         * @brief Set power level
         * @param power New power level (0-15)
         */
        virtual void SetPowerLevel(int power);

        /**
         * @brief Get component state
         * @return Redstone state
         */
        RedstoneState GetState() const { return m_state; }

        /**
         * @brief Check if component is powered
         * @return true if powered
         */
        bool IsPowered() const { return m_powerLevel > 0; }

        /**
         * @brief Get maximum power output
         * @return Maximum power level
         */
        virtual int GetMaxPowerOutput() const { return 15; }

        /**
         * @brief Check if component provides strong power
         * @return true if strong power
         */
        virtual bool ProvidesStrongPower() const { return false; }

        /**
         * @brief Check if component provides weak power
         * @return true if weak power
         */
        virtual bool ProvidesWeakPower() const { return false; }

        /**
         * @brief Get power propagation delay
         * @return Delay in ticks
         */
        virtual int GetPropagationDelay() const { return 0; }

        /**
         * @brief Handle neighbor updates
         * @param neighborPos Neighbor position
         * @param oldPower Old power level
         * @param newPower New power level
         */
        virtual void OnNeighborUpdate(const glm::ivec3& neighborPos, int oldPower, int newPower) {}

        /**
         * @brief Get connected components
         * @return List of connected positions
         */
        virtual std::vector<glm::ivec3> GetConnectedComponents() const;

        /**
         * @brief Check if component can connect to another
         * @param otherPos Other component position
         * @return true if can connect
         */
        virtual bool CanConnectTo(const glm::ivec3& otherPos) const;

        /**
         * @brief Get component facing direction
         * @return Facing direction
         */
        virtual glm::ivec3 GetFacingDirection() const { return glm::ivec3(0, 0, 1); }

        /**
         * @brief Set component facing direction
         * @param direction New facing direction
         */
        virtual void SetFacingDirection(const glm::ivec3& direction) { m_facingDirection = direction; }

    protected:
        RedstoneType m_type;
        glm::ivec3 m_position;
        World* m_world;
        int m_powerLevel;
        RedstoneState m_state;
        glm::ivec3 m_facingDirection;
        std::chrono::steady_clock::time_point m_lastUpdateTime;
        float m_updateTimer;

        /**
         * @brief Update power state based on current power level
         */
        void UpdatePowerState();

        /**
         * @brief Notify neighbors of power change
         */
        void NotifyNeighbors();

        /**
         * @brief Calculate power level from inputs
         * @return Calculated power level
         */
        virtual int CalculatePowerLevel() { return 0; }
    };

    /**
     * @class RedstoneWire
     * @brief Redstone dust/wire component
     */
    class RedstoneWire : public RedstoneComponent {
    public:
        RedstoneWire(const glm::ivec3& position, World* world);

        void Update(float deltaTime) override;
        int GetMaxPowerOutput() const override { return 15; }
        bool ProvidesWeakPower() const override { return true; }
        std::vector<glm::ivec3> GetConnectedComponents() const override;
        bool CanConnectTo(const glm::ivec3& otherPos) const override;

    private:
        int CalculatePowerLevel() override;
        bool IsWireAt(const glm::ivec3& pos) const;
        int GetWirePowerAt(const glm::ivec3& pos) const;
    };

    /**
     * @class RedstoneTorch
     * @brief Redstone torch component
     */
    class RedstoneTorch : public RedstoneComponent {
    public:
        RedstoneTorch(const glm::ivec3& position, World* world);

        void Update(float deltaTime) override;
        void SetPowerLevel(int power) override;
        int GetMaxPowerOutput() const override { return 15; }
        bool ProvidesStrongPower() const override { return true; }
        int GetPropagationDelay() const override { return 1; }

    private:
        bool m_lit;
        int CalculatePowerLevel() override;
    };

    /**
     * @class RedstoneRepeater
     * @brief Redstone repeater component
     */
    class RedstoneRepeater : public RedstoneComponent {
    public:
        RedstoneRepeater(const glm::ivec3& position, World* world);

        void Update(float deltaTime) override;
        void SetPowerLevel(int power) override;
        int GetMaxPowerOutput() const override { return 15; }
        bool ProvidesStrongPower() const override { return true; }
        int GetPropagationDelay() const override { return m_delayTicks; }
        void OnNeighborUpdate(const glm::ivec3& neighborPos, int oldPower, int newPower) override;

        /**
         * @brief Set repeater delay
         * @param ticks Delay in ticks (1, 2, 3, or 4)
         */
        void SetDelay(int ticks) { m_delayTicks = std::max(1, std::min(4, ticks)); }

        /**
         * @brief Get repeater delay
         * @return Delay in ticks
         */
        int GetDelay() const { return m_delayTicks; }

        /**
         * @brief Check if repeater is locked
         * @return true if locked
         */
        bool IsLocked() const { return m_locked; }

    private:
        int m_delayTicks;
        bool m_locked;
        int m_inputPower;
        std::chrono::steady_clock::time_point m_lastInputChange;
        int CalculatePowerLevel() override;
    };

    /**
     * @class RedstoneComparator
     * @brief Redstone comparator component
     */
    class RedstoneComparator : public RedstoneComponent {
    public:
        /**
         * @enum ComparatorMode
         * @brief Comparator operation modes
         */
        enum class ComparatorMode {
            COMPARE = 0,    ///< Compare signal levels
            SUBTRACT       ///< Subtract signal levels
        };

        RedstoneComparator(const glm::ivec3& position, World* world);

        void Update(float deltaTime) override;
        int GetMaxPowerOutput() const override { return 15; }
        bool ProvidesStrongPower() const override { return true; }
        void OnNeighborUpdate(const glm::ivec3& neighborPos, int oldPower, int newPower) override;

        /**
         * @brief Set comparator mode
         * @param mode New comparator mode
         */
        void SetMode(ComparatorMode mode) { m_mode = mode; }

        /**
         * @brief Get comparator mode
         * @return Current mode
         */
        ComparatorMode GetMode() const { return m_mode; }

    private:
        ComparatorMode m_mode;
        int m_inputPower1;  // Back input
        int m_inputPower2;  // Side inputs
        int CalculatePowerLevel() override;
    };

    /**
     * @class RedstonePiston
     * @brief Piston component
     */
    class RedstonePiston : public RedstoneComponent {
    public:
        RedstonePiston(const glm::ivec3& position, World* world, bool sticky = false);

        void Update(float deltaTime) override;
        void SetPowerLevel(int power) override;
        void OnNeighborUpdate(const glm::ivec3& neighborPos, int oldPower, int newPower) override;

        /**
         * @brief Check if piston is sticky
         * @return true if sticky
         */
        bool IsSticky() const { return m_sticky; }

        /**
         * @brief Get piston state
         * @return Current piston state
         */
        PistonState GetPistonState() const { return m_pistonState; }

        /**
         * @brief Get piston extension progress
         * @return Progress (0.0 to 1.0)
         */
        float GetExtensionProgress() const { return m_extensionProgress; }

    private:
        bool m_sticky;
        PistonState m_pistonState;
        float m_extensionProgress;
        float m_extensionSpeed;
        std::chrono::steady_clock::time_point m_lastStateChange;

        void Extend();
        void Retract();
        bool CanExtend() const;
        bool CanRetract() const;
        void MoveBlocks();
    };

    /**
     * @struct RedstoneCircuit
     * @brief Redstone circuit information
     */
    struct RedstoneCircuit {
        std::vector<std::shared_ptr<RedstoneComponent>> components;
        std::vector<RedstoneConnection> connections;
        glm::ivec3 origin;
        int tickDelay;
        bool isActive;
        std::string circuitType;

        RedstoneCircuit()
            : tickDelay(0)
            , isActive(false)
        {}
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_REDSTONE_REDSTONE_COMPONENT_HPP
