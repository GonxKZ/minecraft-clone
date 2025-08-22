/**
 * @file BehaviorTree.hpp
 * @brief VoxelCraft Behavior Tree System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the Behavior Tree system that provides a flexible and powerful
 * AI behavior system based on behavior trees, allowing for complex AI behaviors
 * that can be easily created, modified, and debugged.
 */

#ifndef VOXELCRAFT_AI_BEHAVIOR_TREE_HPP
#define VOXELCRAFT_AI_BEHAVIOR_TREE_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <chrono>
#include <any>

#include "../core/Config.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Mob;
    class BehaviorNode;
    class Blackboard;

    /**
     * @enum BehaviorNodeType
     * @brief Types of behavior tree nodes
     */
    enum class BehaviorNodeType {
        Action,                 ///< Leaf node that performs actions
        Condition,              ///< Leaf node that checks conditions
        Composite,              ///< Composite node with children
        Decorator,              ///< Decorator node that modifies child behavior
        Root                   ///< Root node of the behavior tree
    };

    /**
     * @enum BehaviorNodeStatus
     * @brief Status of behavior node execution
     */
    enum class BehaviorNodeStatus {
        Invalid,                ///< Node is in invalid state
        Running,                ///< Node is currently running
        Success,                ///< Node completed successfully
        Failure                ///< Node failed to complete
    };

    /**
     * @enum CompositeType
     * @brief Types of composite behavior nodes
     */
    enum class CompositeType {
        Sequence,               ///< Execute children in sequence until one fails
        Selector,               ///< Execute children in sequence until one succeeds
        Parallel,               ///< Execute all children in parallel
        RandomSelector,         ///< Execute random child
        WeightedRandom,         ///< Execute random child based on weights
        Priority,               ///< Execute highest priority child
        All,                    ///< Execute all children, succeed if all succeed
        Any                    ///< Execute all children, succeed if any succeed
    };

    /**
     * @enum DecoratorType
     * @brief Types of decorator behavior nodes
     */
    enum class DecoratorType {
        Inverter,               ///< Invert the result of the child
        Succeeder,              ///< Always return success
        Failer,                 ///< Always return failure
        Repeater,               ///< Repeat child execution
        RepeatUntilFail,        ///< Repeat child until it fails
        RepeatUntilSuccess,     ///< Repeat child until it succeeds
        Timer,                  ///< Execute child with time limit
        Cooldown,               ///< Execute child with cooldown
        Delay,                  ///< Delay child execution
        Conditional,            ///< Execute child based on condition
        Probability,            ///< Execute child with probability
        Counter,                ///< Execute child a limited number of times
        BlackboardCheck,        ///< Check blackboard value before executing
        Timeout                ///< Execute child with timeout
    };

    /**
     * @struct BehaviorNodeConfig
     * @brief Configuration for behavior nodes
     */
    struct BehaviorNodeConfig {
        std::string name;                       ///< Node name
        BehaviorNodeType type;                  ///< Node type
        std::string description;                ///< Node description
        std::unordered_map<std::string, std::any> properties; ///< Node properties
        float weight;                           ///< Node weight (for weighted composites)
        int priority;                           ///< Node priority (for priority composites)
        float probability;                      ///< Execution probability (for probability decorator)
        float timeout;                          ///< Timeout duration (seconds)
        float cooldown;                         ///< Cooldown duration (seconds)
        float delay;                            ///< Delay duration (seconds)
        int maxRepeats;                         ///< Maximum repetitions
        int currentRepeats;                     ///< Current repetition count
        bool isEnabled;                         ///< Node is enabled
        bool debugMode;                         ///< Enable debug mode for this node
    };

    /**
     * @struct BehaviorTreeConfig
     * @brief Configuration for behavior trees
     */
    struct BehaviorTreeConfig {
        std::string name;                       ///< Tree name
        std::string description;                ///< Tree description
        float updateInterval;                   ///< Update interval (seconds)
        float maxExecutionTime;                 ///< Maximum execution time (seconds)
        int maxDepth;                           ///< Maximum tree depth
        bool enableDebugging;                   ///< Enable tree debugging
        bool enableProfiling;                   ///< Enable performance profiling
        bool enableParallelExecution;           ///< Enable parallel node execution
        int maxParallelNodes;                   ///< Maximum parallel nodes
    };

    /**
     * @struct BehaviorTreeMetrics
     * @brief Performance metrics for behavior trees
     */
    struct BehaviorTreeMetrics {
        uint64_t executionCount;                ///< Number of executions
        double totalExecutionTime;              ///< Total execution time (ms)
        double averageExecutionTime;            ///< Average execution time (ms)
        double maxExecutionTime;                ///< Maximum execution time (ms)
        uint64_t nodeExecutions;                ///< Total node executions
        uint64_t successfulExecutions;          ///< Successful executions
        uint64_t failedExecutions;              ///< Failed executions
        uint32_t activeNodes;                   ///< Currently active nodes
        uint32_t totalNodes;                    ///< Total nodes in tree
        float successRate;                      ///< Success rate (0.0 - 1.0)
    };

    /**
     * @class BehaviorNode
     * @brief Base class for all behavior tree nodes
     *
     * BehaviorNode is the base class for all nodes in a behavior tree.
     * It provides the interface for node execution, status tracking,
     * and hierarchical organization of AI behaviors.
     */
    class BehaviorNode {
    public:
        /**
         * @brief Constructor
         * @param config Node configuration
         */
        explicit BehaviorNode(const BehaviorNodeConfig& config);

        /**
         * @brief Destructor
         */
        virtual ~BehaviorNode() = default;

        /**
         * @brief Deleted copy constructor
         */
        BehaviorNode(const BehaviorNode&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        BehaviorNode& operator=(const BehaviorNode&) = delete;

        // Node interface

        /**
         * @brief Execute the behavior node
         * @param deltaTime Time elapsed since last update
         * @param mob Mob executing the behavior
         * @param blackboard Blackboard for data sharing
         * @return Node execution status
         */
        virtual BehaviorNodeStatus Execute(double deltaTime, Mob* mob, Blackboard* blackboard) = 0;

        /**
         * @brief Initialize the behavior node
         * @param mob Mob that owns this behavior
         * @param blackboard Blackboard for data sharing
         * @return true if successful, false otherwise
         */
        virtual bool Initialize(Mob* mob, Blackboard* blackboard);

        /**
         * @brief Terminate the behavior node
         * @param mob Mob that owns this behavior
         * @param blackboard Blackboard for data sharing
         */
        virtual void Terminate(Mob* mob, Blackboard* blackboard);

        /**
         * @brief Reset the behavior node
         */
        virtual void Reset();

        /**
         * @brief Abort the behavior node
         */
        virtual void Abort();

        // Node information

        /**
         * @brief Get node type
         * @return Node type
         */
        virtual BehaviorNodeType GetType() const = 0;

        /**
         * @brief Get node name
         * @return Node name
         */
        const std::string& GetName() const { return m_config.name; }

        /**
         * @brief Set node name
         * @param name New node name
         */
        void SetName(const std::string& name) { m_config.name = name; }

        /**
         * @brief Get node ID
         * @return Unique node ID
         */
        uint32_t GetId() const { return m_id; }

        /**
         * @brief Get node configuration
         * @return Node configuration
         */
        const BehaviorNodeConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set node configuration
         * @param config New configuration
         */
        void SetConfig(const BehaviorNodeConfig& config);

        /**
         * @brief Get current node status
         * @return Current status
         */
        BehaviorNodeStatus GetStatus() const { return m_status; }

        /**
         * @brief Set node status
         * @param status New status
         */
        void SetStatus(BehaviorNodeStatus status) { m_status = status; }

        // Hierarchy management

        /**
         * @brief Get parent node
         * @return Parent node or nullptr
         */
        BehaviorNode* GetParent() const { return m_parent; }

        /**
         * @brief Set parent node
         * @param parent Parent node
         */
        void SetParent(BehaviorNode* parent) { m_parent = parent; }

        /**
         * @brief Add child node
         * @param child Child node to add
         * @return true if added, false otherwise
         */
        virtual bool AddChild(std::shared_ptr<BehaviorNode> child);

        /**
         * @brief Remove child node
         * @param child Child node to remove
         * @return true if removed, false otherwise
         */
        virtual bool RemoveChild(std::shared_ptr<BehaviorNode> child);

        /**
         * @brief Get child nodes
         * @return Vector of child nodes
         */
        const std::vector<std::shared_ptr<BehaviorNode>>& GetChildren() const { return m_children; }

        /**
         * @brief Get number of children
         * @return Number of child nodes
         */
        size_t GetChildCount() const { return m_children.size(); }

        // Timing and execution

        /**
         * @brief Get execution time
         * @return Execution time (seconds)
         */
        double GetExecutionTime() const { return m_executionTime; }

        /**
         * @brief Get total execution count
         * @return Number of executions
         */
        uint64_t GetExecutionCount() const { return m_executionCount; }

        /**
         * @brief Get success count
         * @return Number of successful executions
         */
        uint64_t GetSuccessCount() const { return m_successCount; }

        /**
         * @brief Get failure count
         * @return Number of failed executions
         */
        uint64_t GetFailureCount() const { return m_failureCount; }

        /**
         * @brief Check if node is active
         * @return true if active, false otherwise
         */
        bool IsActive() const { return m_status == BehaviorNodeStatus::Running; }

        /**
         * @brief Check if node is enabled
         * @return true if enabled, false otherwise
         */
        bool IsEnabled() const { return m_config.isEnabled; }

        /**
         * @brief Set node enabled state
         * @param enabled New enabled state
         */
        void SetEnabled(bool enabled) { m_config.isEnabled = enabled; }

        // Properties

        /**
         * @brief Get node property
         * @tparam T Property type
         * @param key Property key
         * @param defaultValue Default value
         * @return Property value or default
         */
        template<typename T>
        T GetProperty(const std::string& key, const T& defaultValue = T{}) const;

        /**
         * @brief Set node property
         * @tparam T Property type
         * @param key Property key
         * @param value Property value
         */
        template<typename T>
        void SetProperty(const std::string& key, const T& value);

        /**
         * @brief Check if property exists
         * @param key Property key
         * @return true if exists, false otherwise
         */
        bool HasProperty(const std::string& key) const;

        // Debug and monitoring

        /**
         * @brief Get debug information
         * @return Debug information string
         */
        virtual std::string GetDebugInfo() const;

        /**
         * @brief Validate node configuration
         * @return true if valid, false otherwise
         */
        virtual bool Validate() const;

        /**
         * @brief Clone the behavior node
         * @return Cloned node
         */
        virtual std::shared_ptr<BehaviorNode> Clone() const = 0;

    protected:
        /**
         * @brief Update execution metrics
         * @param deltaTime Time elapsed
         * @param status Execution status
         */
        void UpdateMetrics(double deltaTime, BehaviorNodeStatus status);

        /**
         * @brief Check execution timeout
         * @return true if timed out, false otherwise
         */
        bool CheckTimeout();

        /**
         * @brief Check execution cooldown
         * @return true if on cooldown, false otherwise
         */
        bool CheckCooldown();

        /**
         * @brief Apply node delay
         * @return true if delay active, false otherwise
         */
        bool ApplyDelay();

        uint32_t m_id;                                 ///< Unique node ID
        BehaviorNodeConfig m_config;                   ///< Node configuration
        BehaviorNodeStatus m_status;                   ///< Current execution status
        BehaviorNode* m_parent;                        ///< Parent node
        std::vector<std::shared_ptr<BehaviorNode>> m_children; ///< Child nodes

        // Execution state
        double m_startTime;                            ///< Execution start time
        double m_executionTime;                        ///< Total execution time
        uint64_t m_executionCount;                     ///< Execution count
        uint64_t m_successCount;                       ///< Success count
        uint64_t m_failureCount;                       ///< Failure count

        static std::atomic<uint32_t> s_nextNodeId;    ///< Next node ID counter
    };

    /**
     * @class CompositeNode
     * @brief Composite behavior node that executes multiple children
     */
    class CompositeNode : public BehaviorNode {
    public:
        /**
         * @brief Constructor
         * @param config Node configuration
         * @param compositeType Composite type
         */
        CompositeNode(const BehaviorNodeConfig& config, CompositeType compositeType);

        /**
         * @brief Get composite type
         * @return Composite type
         */
        CompositeType GetCompositeType() const { return m_compositeType; }

        /**
         * @brief Set composite type
         * @param type New composite type
         */
        void SetCompositeType(CompositeType type) { m_compositeType = type; }

        /**
         * @brief Get current child index
         * @return Current child index
         */
        size_t GetCurrentChildIndex() const { return m_currentChildIndex; }

        /**
         * @brief Set current child index
         * @param index New child index
         */
        void SetCurrentChildIndex(size_t index) { m_currentChildIndex = index; }

    protected:
        CompositeType m_compositeType;                 ///< Composite type
        size_t m_currentChildIndex;                    ///< Current child index
    };

    /**
     * @class DecoratorNode
     * @brief Decorator behavior node that modifies child behavior
     */
    class DecoratorNode : public BehaviorNode {
    public:
        /**
         * @brief Constructor
         * @param config Node configuration
         * @param decoratorType Decorator type
         */
        DecoratorNode(const BehaviorNodeConfig& config, DecoratorType decoratorType);

        /**
         * @brief Get decorator type
         * @return Decorator type
         */
        DecoratorType GetDecoratorType() const { return m_decoratorType; }

        /**
         * @brief Set decorator type
         * @param type New decorator type
         */
        void SetDecoratorType(DecoratorType type) { m_decoratorType = type; }

        /**
         * @brief Get child node
         * @return Child node or nullptr
         */
        std::shared_ptr<BehaviorNode> GetChild() const;

        /**
         * @brief Set child node
         * @param child New child node
         * @return true if set, false otherwise
         */
        bool SetChild(std::shared_ptr<BehaviorNode> child);

    protected:
        DecoratorType m_decoratorType;                 ///< Decorator type
    };

    /**
     * @class ActionNode
     * @brief Action behavior node that performs specific actions
     */
    class ActionNode : public BehaviorNode {
    public:
        /**
         * @brief Constructor
         * @param config Node configuration
         * @param action Action function
         */
        ActionNode(const BehaviorNodeConfig& config, std::function<BehaviorNodeStatus(double, Mob*, Blackboard*)> action);

        /**
         * @brief Set action function
         * @param action New action function
         */
        void SetAction(std::function<BehaviorNodeStatus(double, Mob*, Blackboard*)> action);

    protected:
        std::function<BehaviorNodeStatus(double, Mob*, Blackboard*)> m_action; ///< Action function
    };

    /**
     * @class ConditionNode
     * @brief Condition behavior node that checks conditions
     */
    class ConditionNode : public BehaviorNode {
    public:
        /**
         * @brief Constructor
         * @param config Node configuration
         * @param condition Condition function
         */
        ConditionNode(const BehaviorNodeConfig& config, std::function<bool(Mob*, Blackboard*)> condition);

        /**
         * @brief Set condition function
         * @param condition New condition function
         */
        void SetCondition(std::function<bool(Mob*, Blackboard*)> condition);

    protected:
        std::function<bool(Mob*, Blackboard*)> m_condition; ///< Condition function
    };

    /**
     * @class BehaviorTree
     * @brief Main behavior tree class that manages node execution
     *
     * BehaviorTree provides the main interface for creating, executing,
     * and managing behavior trees. It handles the execution flow,
     * node coordination, and provides debugging and profiling capabilities.
     */
    class BehaviorTree {
    public:
        /**
         * @brief Constructor
         * @param config Tree configuration
         */
        explicit BehaviorTree(const BehaviorTreeConfig& config);

        /**
         * @brief Destructor
         */
        ~BehaviorTree();

        /**
         * @brief Deleted copy constructor
         */
        BehaviorTree(const BehaviorTree&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        BehaviorTree& operator=(const BehaviorTree&) = delete;

        // Tree lifecycle

        /**
         * @brief Initialize behavior tree
         * @param mob Mob that owns this tree
         * @param blackboard Blackboard for data sharing
         * @return true if successful, false otherwise
         */
        bool Initialize(Mob* mob, Blackboard* blackboard);

        /**
         * @brief Shutdown behavior tree
         */
        void Shutdown();

        /**
         * @brief Update behavior tree
         * @param deltaTime Time elapsed since last update
         * @return Tree execution status
         */
        BehaviorNodeStatus Update(double deltaTime);

        /**
         * @brief Reset behavior tree
         */
        void Reset();

        /**
         * @brief Abort behavior tree execution
         */
        void Abort();

        // Tree configuration

        /**
         * @brief Get tree configuration
         * @return Tree configuration
         */
        const BehaviorTreeConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set tree configuration
         * @param config New configuration
         */
        void SetConfig(const BehaviorTreeConfig& config);

        /**
         * @brief Get tree name
         * @return Tree name
         */
        const std::string& GetName() const { return m_config.name; }

        /**
         * @brief Set tree name
         * @param name New tree name
         */
        void SetName(const std::string& name) { m_config.name = name; }

        /**
         * @brief Get root node
         * @return Root node or nullptr
         */
        std::shared_ptr<BehaviorNode> GetRoot() const { return m_root; }

        /**
         * @brief Set root node
         * @param root New root node
         * @return true if set, false otherwise
         */
        bool SetRoot(std::shared_ptr<BehaviorNode> root);

        // Tree information

        /**
         * @brief Get current execution status
         * @return Tree execution status
         */
        BehaviorNodeStatus GetStatus() const { return m_status; }

        /**
         * @brief Get currently executing node
         * @return Currently executing node or nullptr
         */
        BehaviorNode* GetCurrentNode() const { return m_currentNode; }

        /**
         * @brief Get tree depth
         * @return Current tree depth
         */
        size_t GetDepth() const;

        /**
         * @brief Get all nodes in tree
         * @return Vector of all nodes
         */
        std::vector<std::shared_ptr<BehaviorNode>> GetAllNodes() const;

        /**
         * @brief Get nodes by type
         * @param type Node type to search for
         * @return Vector of nodes of specified type
         */
        std::vector<std::shared_ptr<BehaviorNode>> GetNodesByType(BehaviorNodeType type) const;

        // Metrics and monitoring

        /**
         * @brief Get tree metrics
         * @return Performance metrics
         */
        const BehaviorTreeMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

        /**
         * @brief Get execution path
         * @return Vector of nodes in current execution path
         */
        std::vector<BehaviorNode*> GetExecutionPath() const;

        // Node factory methods

        /**
         * @brief Create action node
         * @param name Node name
         * @param action Action function
         * @return Created action node
         */
        static std::shared_ptr<ActionNode> CreateAction(const std::string& name,
                                                       std::function<BehaviorNodeStatus(double, Mob*, Blackboard*)> action);

        /**
         * @brief Create condition node
         * @param name Node name
         * @param condition Condition function
         * @return Created condition node
         */
        static std::shared_ptr<ConditionNode> CreateCondition(const std::string& name,
                                                             std::function<bool(Mob*, Blackboard*)> condition);

        /**
         * @brief Create composite node
         * @param name Node name
         * @param type Composite type
         * @return Created composite node
         */
        static std::shared_ptr<CompositeNode> CreateComposite(const std::string& name, CompositeType type);

        /**
         * @brief Create decorator node
         * @param name Node name
         * @param type Decorator type
         * @return Created decorator node
         */
        static std::shared_ptr<DecoratorNode> CreateDecorator(const std::string& name, DecoratorType type);

        // Debug and visualization

        /**
         * @brief Enable debug mode
         * @param enabled Enable state
         * @return Previous state
         */
        bool SetDebugMode(bool enabled);

        /**
         * @brief Check if debug mode is enabled
         * @return true if enabled, false otherwise
         */
        bool IsDebugModeEnabled() const { return m_config.enableDebugging; }

        /**
         * @brief Get debug information
         * @return Debug information string
         */
        std::string GetDebugInfo() const;

        /**
         * @brief Get tree visualization data
         * @return Visualization data
         */
        std::string GetVisualizationData() const;

        /**
         * @brief Validate behavior tree
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Clone behavior tree
         * @return Cloned tree
         */
        std::shared_ptr<BehaviorTree> Clone() const;

    private:
        /**
         * @brief Execute behavior tree
         * @param deltaTime Time elapsed
         * @return Tree execution status
         */
        BehaviorNodeStatus Execute(double deltaTime);

        /**
         * @brief Update execution path
         * @param node Current executing node
         */
        void UpdateExecutionPath(BehaviorNode* node);

        /**
         * @brief Update tree metrics
         * @param deltaTime Time elapsed
         * @param status Execution status
         */
        void UpdateMetrics(double deltaTime, BehaviorNodeStatus status);

        /**
         * @brief Collect all nodes recursively
         * @param node Current node
         * @param nodes Output vector of nodes
         */
        void CollectAllNodes(const std::shared_ptr<BehaviorNode>& node, std::vector<std::shared_ptr<BehaviorNode>>& nodes) const;

        BehaviorTreeConfig m_config;                    ///< Tree configuration
        BehaviorTreeMetrics m_metrics;                  ///< Performance metrics
        BehaviorNodeStatus m_status;                    ///< Current execution status

        // Tree structure
        std::shared_ptr<BehaviorNode> m_root;           ///< Root node
        BehaviorNode* m_currentNode;                    ///< Currently executing node
        std::vector<BehaviorNode*> m_executionPath;     ///< Current execution path

        // Integration
        Mob* m_mob;                                     ///< Mob that owns this tree
        Blackboard* m_blackboard;                       ///< Blackboard for data sharing

        // State
        bool m_isInitialized;                           ///< Tree is initialized
        double m_lastUpdateTime;                        ///< Last update timestamp
        double m_startTime;                             ///< Tree start time
    };

    // Template implementations

    template<typename T>
    T BehaviorNode::GetProperty(const std::string& key, const T& defaultValue) const {
        auto it = m_config.properties.find(key);
        if (it != m_config.properties.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast&) {
                return defaultValue;
            }
        }
        return defaultValue;
    }

    template<typename T>
    void BehaviorNode::SetProperty(const std::string& key, const T& value) {
        m_config.properties[key] = value;
    }

} // namespace VoxelCraft

#endif // VOXELCRAFT_AI_BEHAVIOR_TREE_HPP
