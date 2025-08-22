/**
 * @file Constraint.hpp
 * @brief VoxelCraft Physics Constraint System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the Constraint class and various constraint types
 * used for rigid body joints, springs, motors, and other physical connections.
 */

#ifndef VOXELCRAFT_PHYSICS_CONSTRAINT_HPP
#define VOXELCRAFT_PHYSICS_CONSTRAINT_HPP

#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>

#include "../core/Config.hpp"
#include "PhysicsTypes.hpp"

namespace VoxelCraft {

    // Forward declarations
    class RigidBody;
    class ConstraintSolver;

    /**
     * @enum ConstraintType
     * @brief Type of constraint
     */
    enum class ConstraintType {
        BallAndSocket,          ///< Ball and socket joint
        Hinge,                  ///< Hinge joint (revolute)
        Slider,                 ///< Slider joint (prismatic)
        Fixed,                  ///< Fixed joint
        Spring,                 ///< Spring constraint
        Damper,                 ///< Damper constraint
        Generic6DOF,            ///< Generic 6 degrees of freedom
        PointToPoint,           ///< Point to point constraint
        Gear,                   ///< Gear constraint
        Pulley,                 ///< Pulley constraint
        Distance,               ///< Distance constraint
        Angular,                ///< Angular constraint
        Motor,                  ///< Motor constraint
        Custom                  ///< Custom constraint
    };

    /**
     * @enum ConstraintFlags
     * @brief Constraint behavior flags
     */
    enum ConstraintFlags : uint32_t {
        FLAG_NONE           = 0x0000,
        FLAG_ENABLED        = 0x0001,      ///< Constraint is enabled
        FLAG_DISABLE_COLLISIONS = 0x0002,  ///< Disable collisions between constrained bodies
        FLAG_USE_GLOBAL_FRAME = 0x0004,    ///< Use global coordinate frame
        FLAG_USE_LOCAL_FRAME = 0x0008,     ///< Use local coordinate frame
        FLAG_ENABLE_MOTOR    = 0x0010,     ///< Enable motor functionality
        FLAG_ENABLE_LIMITS   = 0x0020,     ///< Enable joint limits
        FLAG_ENABLE_SPRING   = 0x0040,     ///< Enable spring functionality
        FLAG_ENABLE_FEEDBACK = 0x0080,     ///< Enable force feedback
        FLAG_BREAKABLE       = 0x0100,     ///< Constraint can break
        FLAG_DEBUG_DRAW      = 0x0200,     ///< Enable debug drawing
        FLAG_USER_FLAG_0     = 0x1000,     ///< User-defined flag 0
        FLAG_USER_FLAG_1     = 0x2000,     ///< User-defined flag 1
        FLAG_USER_FLAG_2     = 0x4000,     ///< User-defined flag 2
        FLAG_USER_FLAG_3     = 0x8000      ///< User-defined flag 3
    };

    /**
     * @struct ConstraintLimits
     * @brief Joint limits for constraints
     */
    struct ConstraintLimits {
        float lowerLimit;                  ///< Lower limit (radians or meters)
        float upperLimit;                  ///< Upper limit (radians or meters)
        float softness;                    ///< Limit softness (0.0 - 1.0)
        float biasFactor;                  ///< Limit bias factor
        float relaxationFactor;            ///< Limit relaxation factor
    };

    /**
     * @struct ConstraintMotor
     * @brief Motor properties for constraints
     */
    struct ConstraintMotor {
        float targetVelocity;              ///< Target velocity
        float maxMotorForce;               ///< Maximum motor force
        float maxMotorImpulse;             ///< Maximum motor impulse
        float damping;                     ///< Motor damping
    };

    /**
     * @struct ConstraintSpring
     * @brief Spring properties for constraints
     */
    struct ConstraintSpring {
        float stiffness;                   ///< Spring stiffness
        float damping;                     ///< Spring damping
        float equilibriumPoint;            ///< Spring equilibrium point
        float maxForce;                    ///< Maximum spring force
    };

    /**
     * @struct ConstraintProperties
     * @brief General constraint properties
     */
    struct ConstraintProperties {
        std::string name;                  ///< Constraint name
        ConstraintType type;               ///< Constraint type
        uint32_t flags;                    ///< Constraint flags
        float breakingThreshold;           ///< Force threshold for breaking
        float tau;                         ///< Constraint solver parameter
        float damping;                     ///< General damping
        float impulseClamp;                ///< Maximum impulse clamp
        bool isEnabled;                    ///< Constraint is enabled
        bool debugDraw;                    ///< Enable debug drawing
    };

    /**
     * @struct ConstraintMetrics
     * @brief Performance metrics for a constraint
     */
    struct ConstraintMetrics {
        uint64_t solveCount;               ///< Number of solve iterations
        uint64_t breakCount;               ///< Number of times constraint broke
        double totalSolveTime;             ///< Total time spent solving
        double averageSolveTime;           ///< Average solve time (ms)
        float maxForce;                    ///< Maximum force applied
        float maxImpulse;                  ///< Maximum impulse applied
        float averageError;                ///< Average constraint error
        bool isBroken;                     ///< Constraint is currently broken
    };

    /**
     * @class Constraint
     * @brief Base class for physics constraints
     *
     * Constraint is the base class for all constraint types in the physics system.
     * Constraints define relationships between rigid bodies, such as joints,
     * springs, motors, and other physical connections that restrict relative motion.
     *
     * Key Features:
     * - Multiple constraint types (ball, hinge, slider, spring, etc.)
     * - Joint limits and motors
     * - Spring and damper effects
     * - Breakable constraints
     * - Force feedback
     * - Debug visualization
     */
    class Constraint {
    public:
        /**
         * @brief Constructor
         * @param type Constraint type
         */
        explicit Constraint(ConstraintType type);

        /**
         * @brief Destructor
         */
        virtual ~Constraint() = default;

        /**
         * @brief Deleted copy constructor
         */
        Constraint(const Constraint&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        Constraint& operator=(const Constraint&) = delete;

        // Constraint identification

        /**
         * @brief Get constraint ID
         * @return Unique constraint ID
         */
        uint32_t GetID() const { return m_id; }

        /**
         * @brief Get constraint type
         * @return Constraint type
         */
        ConstraintType GetType() const { return m_type; }

        /**
         * @brief Get constraint name
         * @return Constraint name
         */
        const std::string& GetName() const { return m_properties.name; }

        /**
         * @brief Set constraint name
         * @param name New constraint name
         */
        void SetName(const std::string& name) { m_properties.name = name; }

        // Body access

        /**
         * @brief Get first rigid body
         * @return First body
         */
        RigidBody* GetBodyA() const { return m_bodyA; }

        /**
         * @brief Get second rigid body
         * @return Second body (can be nullptr for single-body constraints)
         */
        RigidBody* GetBodyB() const { return m_bodyB; }

        /**
         * @brief Set rigid bodies
         * @param bodyA First body
         * @param bodyB Second body (can be nullptr)
         */
        void SetBodies(RigidBody* bodyA, RigidBody* bodyB = nullptr);

        // Constraint solving

        /**
         * @brief Solve constraint
         * @param timeStep Time step for simulation
         * @param iteration Solver iteration
         * @return true if constraint was solved, false otherwise
         */
        virtual bool Solve(double timeStep, int iteration) = 0;

        /**
         * @brief Prepare constraint for solving
         * @param timeStep Time step for simulation
         */
        virtual void PrepareSolve(double timeStep) = 0;

        /**
         * @brief Finalize constraint solving
         */
        virtual void FinalizeSolve() = 0;

        /**
         * @brief Get constraint error
         * @return Current constraint error
         */
        virtual float GetError() const = 0;

        // Properties access

        /**
         * @brief Get constraint properties
         * @return Constraint properties
         */
        const ConstraintProperties& GetProperties() const { return m_properties; }

        /**
         * @brief Set constraint properties
         * @param properties New properties
         */
        void SetProperties(const ConstraintProperties& properties);

        /**
         * @brief Check if constraint is enabled
         * @return true if enabled, false otherwise
         */
        bool IsEnabled() const { return m_properties.isEnabled; }

        /**
         * @brief Set constraint enabled state
         * @param enabled New enabled state
         */
        void SetEnabled(bool enabled);

        /**
         * @brief Check if constraint is broken
         * @return true if broken, false otherwise
         */
        bool IsBroken() const { return m_metrics.isBroken; }

        /**
         * @brief Get breaking threshold
         * @return Breaking threshold force
         */
        float GetBreakingThreshold() const { return m_properties.breakingThreshold; }

        /**
         * @brief Set breaking threshold
         * @param threshold New breaking threshold
         */
        void SetBreakingThreshold(float threshold) { m_properties.breakingThreshold = threshold; }

        // Flags

        /**
         * @brief Check if flag is set
         * @param flag Flag to check
         * @return true if set, false otherwise
         */
        bool HasFlag(ConstraintFlags flag) const { return (m_properties.flags & flag) != 0; }

        /**
         * @brief Set flag
         * @param flag Flag to set
         */
        void SetFlag(ConstraintFlags flag);

        /**
         * @brief Clear flag
         * @param flag Flag to clear
         */
        void ClearFlag(ConstraintFlags flag);

        /**
         * @brief Toggle flag
         * @param flag Flag to toggle
         */
        void ToggleFlag(ConstraintFlags flag);

        /**
         * @brief Get all flags
         * @return Current flags
         */
        uint32_t GetFlags() const { return m_properties.flags; }

        /**
         * @brief Set all flags
         * @param flags New flags
         */
        void SetFlags(uint32_t flags);

        // Frame access (for articulated constraints)

        /**
         * @brief Get constraint frame in body A
         * @return Frame transform for body A
         */
        virtual glm::mat4 GetFrameA() const = 0;

        /**
         * @brief Get constraint frame in body B
         * @return Frame transform for body B
         */
        virtual glm::mat4 GetFrameB() const = 0;

        /**
         * @brief Set constraint frame for body A
         * @param frame New frame transform
         */
        virtual void SetFrameA(const glm::mat4& frame) = 0;

        /**
         * @brief Set constraint frame for body B
         * @param frame New frame transform
         */
        virtual void SetFrameB(const glm::mat4& frame) = 0;

        // Limits

        /**
         * @brief Check if constraint has limits
         * @return true if has limits, false otherwise
         */
        virtual bool HasLimits() const = 0;

        /**
         * @brief Get constraint limits
         * @return Constraint limits
         */
        virtual ConstraintLimits GetLimits() const = 0;

        /**
         * @brief Set constraint limits
         * @param limits New limits
         */
        virtual void SetLimits(const ConstraintLimits& limits) = 0;

        // Motor

        /**
         * @brief Check if constraint has motor
         * @return true if has motor, false otherwise
         */
        virtual bool HasMotor() const = 0;

        /**
         * @brief Get motor properties
         * @return Motor properties
         */
        virtual ConstraintMotor GetMotor() const = 0;

        /**
         * @brief Set motor properties
         * @param motor New motor properties
         */
        virtual void SetMotor(const ConstraintMotor& motor) = 0;

        // Spring

        /**
         * @brief Check if constraint has spring
         * @return true if has spring, false otherwise
         */
        virtual bool HasSpring() const = 0;

        /**
         * @brief Get spring properties
         * @return Spring properties
         */
        virtual ConstraintSpring GetSpring() const = 0;

        /**
         * @brief Set spring properties
         * @param spring New spring properties
         */
        virtual void SetSpring(const ConstraintSpring& spring) = 0;

        // Force and feedback

        /**
         * @brief Get applied impulse
         * @return Applied impulse
         */
        virtual glm::vec3 GetAppliedImpulse() const = 0;

        /**
         * @brief Get applied torque
         * @return Applied torque
         */
        virtual glm::vec3 GetAppliedTorque() const = 0;

        /**
         * @brief Enable force feedback
         * @param enabled Enable state
         */
        virtual void SetForceFeedbackEnabled(bool enabled) = 0;

        /**
         * @brief Check if force feedback is enabled
         * @return true if enabled, false otherwise
         */
        virtual bool IsForceFeedbackEnabled() const = 0;

        // Breakable constraints

        /**
         * @brief Break constraint
         */
        virtual void Break();

        /**
         * @brief Repair constraint
         */
        virtual void Repair();

        // Metrics and debugging

        /**
         * @brief Get constraint metrics
         * @return Performance metrics
         */
        const ConstraintMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

        /**
         * @brief Get debug information
         * @return Debug information string
         */
        virtual std::string GetDebugInfo() const;

        /**
         * @brief Validate constraint
         * @return true if valid, false otherwise
         */
        virtual bool Validate() const;

        /**
         * @brief Update constraint state
         * @param deltaTime Time elapsed
         */
        virtual void Update(double deltaTime);

    protected:
        /**
         * @brief Calculate constraint violation
         * @return Constraint violation vector
         */
        virtual glm::vec3 CalculateViolation() const = 0;

        /**
         * @brief Apply constraint impulse
         * @param impulse Impulse to apply
         * @param torque Torque to apply
         */
        virtual void ApplyImpulse(const glm::vec3& impulse, const glm::vec3& torque) = 0;

        /**
         * @brief Check if constraint should break
         * @return true if should break, false otherwise
         */
        virtual bool ShouldBreak() const;

        /**
         * @brief Update constraint metrics
         * @param solveTime Time spent solving
         */
        void UpdateMetrics(double solveTime);

        uint32_t m_id;                      ///< Unique constraint ID
        ConstraintType m_type;              ///< Constraint type
        ConstraintProperties m_properties;  ///< Constraint properties
        ConstraintMetrics m_metrics;        ///< Performance metrics
        RigidBody* m_bodyA;                 ///< First rigid body
        RigidBody* m_bodyB;                 ///< Second rigid body (can be nullptr)
        ConstraintState m_state;            ///< Constraint state
        bool m_isBroken;                    ///< Constraint is broken

        static std::atomic<uint32_t> s_nextConstraintId; ///< Next constraint ID
    };

    // Specific constraint implementations

    /**
     * @class BallAndSocketConstraint
     * @brief Ball and socket joint constraint
     */
    class BallAndSocketConstraint : public Constraint {
    public:
        /**
         * @brief Constructor
         * @param pivotPoint Pivot point in world space
         */
        explicit BallAndSocketConstraint(const glm::vec3& pivotPoint = glm::vec3(0.0f));

        /**
         * @brief Get pivot point in world space
         * @return Pivot point
         */
        glm::vec3 GetPivotPoint() const { return m_pivotPoint; }

        /**
         * @brief Set pivot point in world space
         * @param pivot New pivot point
         */
        void SetPivotPoint(const glm::vec3& pivot);

    protected:
        bool Solve(double timeStep, int iteration) override;
        void PrepareSolve(double timeStep) override;
        void FinalizeSolve() override;
        float GetError() const override;
        glm::mat4 GetFrameA() const override;
        glm::mat4 GetFrameB() const override;
        void SetFrameA(const glm::mat4& frame) override;
        void SetFrameB(const glm::mat4& frame) override;
        bool HasLimits() const override;
        ConstraintLimits GetLimits() const override;
        void SetLimits(const ConstraintLimits& limits) override;
        bool HasMotor() const override;
        ConstraintMotor GetMotor() const override;
        void SetMotor(const ConstraintMotor& motor) override;
        bool HasSpring() const override;
        ConstraintSpring GetSpring() const override;
        void SetSpring(const ConstraintSpring& spring) override;
        glm::vec3 GetAppliedImpulse() const override;
        glm::vec3 GetAppliedTorque() const override;
        void SetForceFeedbackEnabled(bool enabled) override;
        bool IsForceFeedbackEnabled() const override;
        glm::vec3 CalculateViolation() const override;
        void ApplyImpulse(const glm::vec3& impulse, const glm::vec3& torque) override;

    private:
        glm::vec3 m_pivotPoint;             ///< Pivot point in world space
        glm::vec3 m_localPivotA;            ///< Pivot point in body A local space
        glm::vec3 m_localPivotB;            ///< Pivot point in body B local space
    };

    /**
     * @class HingeConstraint
     * @brief Hinge joint constraint (revolute joint)
     */
    class HingeConstraint : public Constraint {
    public:
        /**
         * @brief Constructor
         * @param pivotPoint Pivot point in world space
         * @param axis Hinge axis in world space
         */
        HingeConstraint(const glm::vec3& pivotPoint = glm::vec3(0.0f),
                       const glm::vec3& axis = glm::vec3(0.0f, 0.0f, 1.0f));

        /**
         * @brief Get pivot point in world space
         * @return Pivot point
         */
        glm::vec3 GetPivotPoint() const { return m_pivotPoint; }

        /**
         * @brief Set pivot point in world space
         * @param pivot New pivot point
         */
        void SetPivotPoint(const glm::vec3& pivot);

        /**
         * @brief Get hinge axis in world space
         * @return Hinge axis
         */
        glm::vec3 GetAxis() const { return m_axis; }

        /**
         * @brief Set hinge axis in world space
         * @param axis New hinge axis
         */
        void SetAxis(const glm::vec3& axis);

        /**
         * @brief Get current hinge angle
         * @return Current angle (radians)
         */
        float GetHingeAngle() const;

        /**
         * @brief Set hinge angle limits
         * @param minAngle Minimum angle (radians)
         * @param maxAngle Maximum angle (radians)
         */
        void SetAngularLimits(float minAngle, float maxAngle);

    protected:
        bool Solve(double timeStep, int iteration) override;
        void PrepareSolve(double timeStep) override;
        void FinalizeSolve() override;
        float GetError() const override;
        glm::mat4 GetFrameA() const override;
        glm::mat4 GetFrameB() const override;
        void SetFrameA(const glm::mat4& frame) override;
        void SetFrameB(const glm::mat4& frame) override;
        bool HasLimits() const override;
        ConstraintLimits GetLimits() const override;
        void SetLimits(const ConstraintLimits& limits) override;
        bool HasMotor() const override;
        ConstraintMotor GetMotor() const override;
        void SetMotor(const ConstraintMotor& motor) override;
        bool HasSpring() const override;
        ConstraintSpring GetSpring() const override;
        void SetSpring(const ConstraintSpring& spring) override;
        glm::vec3 GetAppliedImpulse() const override;
        glm::vec3 GetAppliedTorque() const override;
        void SetForceFeedbackEnabled(bool enabled) override;
        bool IsForceFeedbackEnabled() const override;
        glm::vec3 CalculateViolation() const override;
        void ApplyImpulse(const glm::vec3& impulse, const glm::vec3& torque) override;

    private:
        glm::vec3 m_pivotPoint;             ///< Pivot point in world space
        glm::vec3 m_axis;                   ///< Hinge axis in world space
        glm::vec3 m_localPivotA;            ///< Pivot point in body A local space
        glm::vec3 m_localAxisA;             ///< Hinge axis in body A local space
        glm::vec3 m_localPivotB;            ///< Pivot point in body B local space
        glm::vec3 m_localAxisB;             ///< Hinge axis in body B local space
        ConstraintLimits m_angularLimits;   ///< Angular limits
        bool m_hasAngularLimits;            ///< Has angular limits
    };

    /**
     * @class SliderConstraint
     * @brief Slider joint constraint (prismatic joint)
     */
    class SliderConstraint : public Constraint {
    public:
        /**
         * @brief Constructor
         * @param frameA Frame in body A
         * @param frameB Frame in body B
         */
        SliderConstraint(const glm::mat4& frameA = glm::mat4(1.0f),
                        const glm::mat4& frameB = glm::mat4(1.0f));

        /**
         * @brief Get current linear position
         * @return Current position along slider axis
         */
        float GetLinearPosition() const;

        /**
         * @brief Set linear limits
         * @param minPos Minimum position
         * @param maxPos Maximum position
         */
        void SetLinearLimits(float minPos, float maxPos);

    protected:
        bool Solve(double timeStep, int iteration) override;
        void PrepareSolve(double timeStep) override;
        void FinalizeSolve() override;
        float GetError() const override;
        glm::mat4 GetFrameA() const override;
        glm::mat4 GetFrameB() const override;
        void SetFrameA(const glm::mat4& frame) override;
        void SetFrameB(const glm::mat4& frame) override;
        bool HasLimits() const override;
        ConstraintLimits GetLimits() const override;
        void SetLimits(const ConstraintLimits& limits) override;
        bool HasMotor() const override;
        ConstraintMotor GetMotor() const override;
        void SetMotor(const ConstraintMotor& motor) override;
        bool HasSpring() const override;
        ConstraintSpring GetSpring() const override;
        void SetSpring(const ConstraintSpring& spring) override;
        glm::vec3 GetAppliedImpulse() const override;
        glm::vec3 GetAppliedTorque() const override;
        void SetForceFeedbackEnabled(bool enabled) override;
        bool IsForceFeedbackEnabled() const override;
        glm::vec3 CalculateViolation() const override;
        void ApplyImpulse(const glm::vec3& impulse, const glm::vec3& torque) override;

    private:
        glm::mat4 m_frameA;                 ///< Frame in body A
        glm::mat4 m_frameB;                 ///< Frame in body B
        ConstraintLimits m_linearLimits;    ///< Linear limits
        bool m_hasLinearLimits;             ///< Has linear limits
    };

    /**
     * @class SpringConstraint
     * @brief Spring constraint between two bodies
     */
    class SpringConstraint : public Constraint {
    public:
        /**
         * @brief Constructor
         * @param pivotA Pivot point in body A
         * @param pivotB Pivot point in body B
         * @param restLength Rest length of spring
         * @param stiffness Spring stiffness
         * @param damping Spring damping
         */
        SpringConstraint(const glm::vec3& pivotA = glm::vec3(0.0f),
                        const glm::vec3& pivotB = glm::vec3(0.0f),
                        float restLength = 1.0f,
                        float stiffness = 100.0f,
                        float damping = 10.0f);

        /**
         * @brief Get rest length
         * @return Rest length
         */
        float GetRestLength() const { return m_restLength; }

        /**
         * @brief Set rest length
         * @param length New rest length
         */
        void SetRestLength(float length) { m_restLength = length; }

        /**
         * @brief Get current length
         * @return Current spring length
         */
        float GetCurrentLength() const;

    protected:
        bool Solve(double timeStep, int iteration) override;
        void PrepareSolve(double timeStep) override;
        void FinalizeSolve() override;
        float GetError() const override;
        glm::mat4 GetFrameA() const override;
        glm::mat4 GetFrameB() const override;
        void SetFrameA(const glm::mat4& frame) override;
        void SetFrameB(const glm::mat4& frame) override;
        bool HasLimits() const override;
        ConstraintLimits GetLimits() const override;
        void SetLimits(const ConstraintLimits& limits) override;
        bool HasMotor() const override;
        ConstraintMotor GetMotor() const override;
        void SetMotor(const ConstraintMotor& motor) override;
        bool HasSpring() const override;
        ConstraintSpring GetSpring() const override;
        void SetSpring(const ConstraintSpring& spring) override;
        glm::vec3 GetAppliedImpulse() const override;
        glm::vec3 GetAppliedTorque() const override;
        void SetForceFeedbackEnabled(bool enabled) override;
        bool IsForceFeedbackEnabled() const override;
        glm::vec3 CalculateViolation() const override;
        void ApplyImpulse(const glm::vec3& impulse, const glm::vec3& torque) override;

    private:
        glm::vec3 m_pivotA;                 ///< Pivot point in body A
        glm::vec3 m_pivotB;                 ///< Pivot point in body B
        float m_restLength;                 ///< Rest length of spring
        float m_currentLength;              ///< Current spring length
        ConstraintSpring m_springProps;     ///< Spring properties
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_PHYSICS_CONSTRAINT_HPP
