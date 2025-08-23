/**
 * @file BehaviorTree.hpp
 * @brief VoxelCraft AI Behavior Tree System
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_AI_BEHAVIOR_TREE_HPP
#define VOXELCRAFT_AI_BEHAVIOR_TREE_HPP

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <chrono>
#include <any>

namespace VoxelCraft {

    class Entity;
    class World;
    class Player;

    /**
     * @enum NodeStatus
     * @brief Status of behavior tree node execution
     */
    enum class NodeStatus {
        SUCCESS = 0,    ///< Node completed successfully
        FAILURE,        ///< Node failed to complete
        RUNNING,        ///< Node is still executing
        INVALID         ///< Node is in invalid state
    };

    /**
     * @enum NodeType
     * @brief Types of behavior tree nodes
     */
    enum class NodeType {
        ACTION = 0,     ///< Leaf node that performs an action
        CONDITION,      ///< Leaf node that checks a condition
        COMPOSITE,      ///< Node with children
        DECORATOR       ///< Node that modifies child behavior
    };

    /**
     * @enum CompositeType
     * @brief Types of composite nodes
     */
    enum class CompositeType {
        SEQUENCE = 0,   ///< Execute children in order until one fails
        SELECTOR,       ///< Execute children in order until one succeeds
        PARALLEL,       ///< Execute all children simultaneously
        RANDOM          ///< Execute random child
    };

    /**
     * @enum DecoratorType
     * @brief Types of decorator nodes
     */
    enum class DecoratorType {
        INVERTER = 0,   ///< Invert child result
        REPEAT,         ///< Repeat child execution
        RETRY,          ///< Retry child on failure
        SUCCEEDER,      ///< Always return success
        FAILER,         ///< Always return failure
        TIMER,          ///< Add time limit to child
        COOLDOWN        ///< Add cooldown between executions
    };

    /**
     * @struct BehaviorContext
     * @brief Context passed to behavior tree nodes
     */
    struct BehaviorContext {
        Entity* entity = nullptr;
        World* world = nullptr;
        Player* targetPlayer = nullptr;
        float deltaTime = 0.0f;
        std::unordered_map<std::string, std::any> blackboard;
        std::chrono::steady_clock::time_point currentTime;

        BehaviorContext() : currentTime(std::chrono::steady_clock::now()) {}
    };

    /**
     * @class BehaviorNode
     * @brief Base class for all behavior tree nodes
     */
    class BehaviorNode {
    public:
        /**
         * @brief Constructor
         * @param name Node name
         * @param type Node type
         */
        BehaviorNode(const std::string& name, NodeType type);

        /**
         * @brief Destructor
         */
        virtual ~BehaviorNode() = default;

        /**
         * @brief Execute the node
         * @param context Behavior context
         * @return Node execution status
         */
        virtual NodeStatus Execute(BehaviorContext& context) = 0;

        /**
         * @brief Get node name
         * @return Node name
         */
        const std::string& GetName() const { return m_name; }

        /**
         * @brief Get node type
         * @return Node type
         */
        NodeType GetType() const { return m_type; }

        /**
         * @brief Reset node state
         */
        virtual void Reset();

        /**
         * @brief Get debug info
         * @return Debug information string
         */
        virtual std::string GetDebugInfo() const;

        /**
         * @brief Set node enabled state
         * @param enabled Whether node is enabled
         */
        void SetEnabled(bool enabled) { m_enabled = enabled; }

        /**
         * @brief Check if node is enabled
         * @return true if enabled
         */
        bool IsEnabled() const { return m_enabled; }

    protected:
        std::string m_name;
        NodeType m_type;
        bool m_enabled = true;
        NodeStatus m_lastStatus = NodeStatus::INVALID;
        int m_executionCount = 0;
        float m_lastExecutionTime = 0.0f;

        /**
         * @brief Update execution statistics
         * @param status Execution status
         * @param executionTime Time taken
         */
        void UpdateStats(NodeStatus status, float executionTime);
    };

    /**
     * @class ActionNode
     * @brief Leaf node that performs an action
     */
    class ActionNode : public BehaviorNode {
    public:
        using ActionFunction = std::function<NodeStatus(BehaviorContext&)>;

        /**
         * @brief Constructor
         * @param name Node name
         * @param action Action function
         */
        ActionNode(const std::string& name, ActionFunction action);

        /**
         * @brief Execute the action
         * @param context Behavior context
         * @return Node execution status
         */
        NodeStatus Execute(BehaviorContext& context) override;

    private:
        ActionFunction m_action;
    };

    /**
     * @class ConditionNode
     * @brief Leaf node that checks a condition
     */
    class ConditionNode : public BehaviorNode {
    public:
        using ConditionFunction = std::function<bool(BehaviorContext&)>;

        /**
         * @brief Constructor
         * @param name Node name
         * @param condition Condition function
         */
        ConditionNode(const std::string& name, ConditionFunction condition);

        /**
         * @brief Execute the condition
         * @param context Behavior context
         * @return Node execution status
         */
        NodeStatus Execute(BehaviorContext& context) override;

    private:
        ConditionFunction m_condition;
    };

    /**
     * @class CompositeNode
     * @brief Node with multiple children
     */
    class CompositeNode : public BehaviorNode {
    public:
        /**
         * @brief Constructor
         * @param name Node name
         * @param type Composite type
         */
        CompositeNode(const std::string& name, CompositeType type);

        /**
         * @brief Add child node
         * @param child Child node to add
         */
        void AddChild(std::shared_ptr<BehaviorNode> child);

        /**
         * @brief Remove child node
         * @param child Child node to remove
         */
        void RemoveChild(std::shared_ptr<BehaviorNode> child);

        /**
         * @brief Get children
         * @return Vector of child nodes
         */
        const std::vector<std::shared_ptr<BehaviorNode>>& GetChildren() const { return m_children; }

        /**
         * @brief Clear all children
         */
        void ClearChildren();

        /**
         * @brief Get composite type
         * @return Composite type
         */
        CompositeType GetCompositeType() const { return m_compositeType; }

    protected:
        CompositeType m_compositeType;
        std::vector<std::shared_ptr<BehaviorNode>> m_children;
        size_t m_currentChildIndex = 0;
    };

    /**
     * @class SequenceNode
     * @brief Execute children in order until one fails
     */
    class SequenceNode : public CompositeNode {
    public:
        /**
         * @brief Constructor
         * @param name Node name
         */
        SequenceNode(const std::string& name);

        /**
         * @brief Execute the sequence
         * @param context Behavior context
         * @return Node execution status
         */
        NodeStatus Execute(BehaviorContext& context) override;

        /**
         * @brief Reset sequence state
         */
        void Reset() override;
    };

    /**
     * @class SelectorNode
     * @brief Execute children in order until one succeeds
     */
    class SelectorNode : public CompositeNode {
    public:
        /**
         * @brief Constructor
         * @param name Node name
         */
        SelectorNode(const std::string& name);

        /**
         * @brief Execute the selector
         * @param context Behavior context
         * @return Node execution status
         */
        NodeStatus Execute(BehaviorContext& context) override;

        /**
         * @brief Reset selector state
         */
        void Reset() override;
    };

    /**
     * @class ParallelNode
     * @brief Execute all children simultaneously
     */
    class ParallelNode : public CompositeNode {
    public:
        /**
         * @brief Constructor
         * @param name Node name
         * @param successThreshold Number of children that must succeed
         * @param failureThreshold Number of children that can fail
         */
        ParallelNode(const std::string& name, int successThreshold = 1, int failureThreshold = 1);

        /**
         * @brief Execute the parallel node
         * @param context Behavior context
         * @return Node execution status
         */
        NodeStatus Execute(BehaviorContext& context) override;

        /**
         * @brief Reset parallel state
         */
        void Reset() override;

    private:
        int m_successThreshold;
        int m_failureThreshold;
        std::vector<NodeStatus> m_childStatuses;
    };

    /**
     * @class DecoratorNode
     * @brief Node that modifies child behavior
     */
    class DecoratorNode : public BehaviorNode {
    public:
        /**
         * @brief Constructor
         * @param name Node name
         * @param type Decorator type
         * @param child Child node
         */
        DecoratorNode(const std::string& name, DecoratorType type, std::shared_ptr<BehaviorNode> child);

        /**
         * @brief Set child node
         * @param child Child node
         */
        void SetChild(std::shared_ptr<BehaviorNode> child) { m_child = child; }

        /**
         * @brief Get child node
         * @return Child node
         */
        std::shared_ptr<BehaviorNode> GetChild() const { return m_child; }

        /**
         * @brief Get decorator type
         * @return Decorator type
         */
        DecoratorType GetDecoratorType() const { return m_decoratorType; }

    protected:
        DecoratorType m_decoratorType;
        std::shared_ptr<BehaviorNode> m_child;
    };

    /**
     * @class InverterNode
     * @brief Invert child result
     */
    class InverterNode : public DecoratorNode {
    public:
        /**
         * @brief Constructor
         * @param name Node name
         * @param child Child node
         */
        InverterNode(const std::string& name, std::shared_ptr<BehaviorNode> child);

        /**
         * @brief Execute the inverter
         * @param context Behavior context
         * @return Node execution status
         */
        NodeStatus Execute(BehaviorContext& context) override;
    };

    /**
     * @class RepeatNode
     * @brief Repeat child execution
     */
    class RepeatNode : public DecoratorNode {
    public:
        /**
         * @brief Constructor
         * @param name Node name
         * @param child Child node
         * @param count Number of repetitions (-1 for infinite)
         */
        RepeatNode(const std::string& name, std::shared_ptr<BehaviorNode> child, int count = -1);

        /**
         * @brief Execute the repeat
         * @param context Behavior context
         * @return Node execution status
         */
        NodeStatus Execute(BehaviorContext& context) override;

        /**
         * @brief Reset repeat state
         */
        void Reset() override;

    private:
        int m_repeatCount;
        int m_currentCount;
    };

    /**
     * @class BehaviorTree
     * @brief Main behavior tree class
     */
    class BehaviorTree {
    public:
        /**
         * @brief Constructor
         * @param name Tree name
         */
        BehaviorTree(const std::string& name);

        /**
         * @brief Destructor
         */
        ~BehaviorTree() = default;

        /**
         * @brief Set root node
         * @param root Root node
         */
        void SetRoot(std::shared_ptr<BehaviorNode> root) { m_root = root; }

        /**
         * @brief Get root node
         * @return Root node
         */
        std::shared_ptr<BehaviorNode> GetRoot() const { return m_root; }

        /**
         * @brief Execute the behavior tree
         * @param context Behavior context
         * @return Tree execution status
         */
        NodeStatus Execute(BehaviorContext& context);

        /**
         * @brief Get tree name
         * @return Tree name
         */
        const std::string& GetName() const { return m_name; }

        /**
         * @brief Reset tree state
         */
        void Reset();

        /**
         * @brief Get debug info for the entire tree
         * @return Debug information string
         */
        std::string GetDebugInfo() const;

        /**
         * @brief Get execution statistics
         * @return Execution stats
         */
        const BehaviorTreeStats& GetStats() const { return m_stats; }

    private:
        std::string m_name;
        std::shared_ptr<BehaviorNode> m_root;
        BehaviorTreeStats m_stats;
        std::chrono::steady_clock::time_point m_lastExecutionTime;

        /**
         * @brief Traverse tree for debug info
         * @param node Current node
         * @param indent Indentation level
         * @return Debug string
         */
        std::string TraverseDebugInfo(std::shared_ptr<BehaviorNode> node, int indent = 0) const;
    };

    /**
     * @struct BehaviorTreeStats
     * @brief Statistics for behavior tree execution
     */
    struct BehaviorTreeStats {
        int totalExecutions = 0;
        int successfulExecutions = 0;
        int failedExecutions = 0;
        float averageExecutionTime = 0.0f;
        float minExecutionTime = 999999.0f;
        float maxExecutionTime = 0.0f;
        int nodesExecuted = 0;
        int nodesSucceeded = 0;
        int nodesFailed = 0;
        std::unordered_map<std::string, int> nodeExecutionCounts;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_AI_BEHAVIOR_TREE_HPP
