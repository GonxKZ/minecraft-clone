/**
 * @file BehaviorTree.cpp
 * @brief VoxelCraft AI Behavior Tree Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "BehaviorTree.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace VoxelCraft {

    // BehaviorNode implementation
    BehaviorNode::BehaviorNode(const std::string& name, NodeType type)
        : m_name(name), m_type(type) {
    }

    void BehaviorNode::Reset() {
        m_lastStatus = NodeStatus::INVALID;
        m_executionCount = 0;
        m_lastExecutionTime = 0.0f;
    }

    std::string BehaviorNode::GetDebugInfo() const {
        std::stringstream ss;
        ss << m_name << " [";
        switch (m_type) {
            case NodeType::ACTION: ss << "ACTION"; break;
            case NodeType::CONDITION: ss << "CONDITION"; break;
            case NodeType::COMPOSITE: ss << "COMPOSITE"; break;
            case NodeType::DECORATOR: ss << "DECORATOR"; break;
        }
        ss << "] Status: ";
        switch (m_lastStatus) {
            case NodeStatus::SUCCESS: ss << "SUCCESS"; break;
            case NodeStatus::FAILURE: ss << "FAILURE"; break;
            case NodeStatus::RUNNING: ss << "RUNNING"; break;
            case NodeStatus::INVALID: ss << "INVALID"; break;
        }
        ss << " Executions: " << m_executionCount;
        return ss.str();
    }

    void BehaviorNode::UpdateStats(NodeStatus status, float executionTime) {
        m_lastStatus = status;
        m_executionCount++;
        m_lastExecutionTime = executionTime;
    }

    // ActionNode implementation
    ActionNode::ActionNode(const std::string& name, ActionFunction action)
        : BehaviorNode(name, NodeType::ACTION), m_action(action) {
    }

    NodeStatus ActionNode::Execute(BehaviorContext& context) {
        if (!m_enabled) {
            return NodeStatus::FAILURE;
        }

        auto startTime = std::chrono::high_resolution_clock::now();

        NodeStatus result = NodeStatus::FAILURE;
        if (m_action) {
            result = m_action(context);
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        float executionTime = std::chrono::duration<float>(endTime - startTime).count();

        UpdateStats(result, executionTime);
        return result;
    }

    // ConditionNode implementation
    ConditionNode::ConditionNode(const std::string& name, ConditionFunction condition)
        : BehaviorNode(name, NodeType::CONDITION), m_condition(condition) {
    }

    NodeStatus ConditionNode::Execute(BehaviorContext& context) {
        if (!m_enabled) {
            return NodeStatus::FAILURE;
        }

        auto startTime = std::chrono::high_resolution_clock::now();

        NodeStatus result = NodeStatus::FAILURE;
        if (m_condition && m_condition(context)) {
            result = NodeStatus::SUCCESS;
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        float executionTime = std::chrono::duration<float>(endTime - startTime).count();

        UpdateStats(result, executionTime);
        return result;
    }

    // CompositeNode implementation
    CompositeNode::CompositeNode(const std::string& name, CompositeType type)
        : BehaviorNode(name, NodeType::COMPOSITE), m_compositeType(type) {
    }

    void CompositeNode::AddChild(std::shared_ptr<BehaviorNode> child) {
        if (child) {
            m_children.push_back(child);
        }
    }

    void CompositeNode::RemoveChild(std::shared_ptr<BehaviorNode> child) {
        if (child) {
            m_children.erase(std::remove(m_children.begin(), m_children.end(), child), m_children.end());
        }
    }

    void CompositeNode::ClearChildren() {
        m_children.clear();
        m_currentChildIndex = 0;
    }

    // SequenceNode implementation
    SequenceNode::SequenceNode(const std::string& name)
        : CompositeNode(name, CompositeType::SEQUENCE) {
    }

    NodeStatus SequenceNode::Execute(BehaviorContext& context) {
        if (!m_enabled || m_children.empty()) {
            return NodeStatus::FAILURE;
        }

        auto startTime = std::chrono::high_resolution_clock::now();

        NodeStatus result = NodeStatus::SUCCESS;

        for (size_t i = m_currentChildIndex; i < m_children.size(); ++i) {
            auto child = m_children[i];
            NodeStatus childResult = child->Execute(context);

            if (childResult == NodeStatus::FAILURE) {
                result = NodeStatus::FAILURE;
                m_currentChildIndex = 0; // Reset for next execution
                break;
            } else if (childResult == NodeStatus::RUNNING) {
                result = NodeStatus::RUNNING;
                m_currentChildIndex = i; // Continue from here next time
                break;
            }
        }

        if (result == NodeStatus::SUCCESS) {
            m_currentChildIndex = 0; // Reset for next execution
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        float executionTime = std::chrono::duration<float>(endTime - startTime).count();

        UpdateStats(result, executionTime);
        return result;
    }

    void SequenceNode::Reset() {
        BehaviorNode::Reset();
        m_currentChildIndex = 0;
        for (auto& child : m_children) {
            if (child) {
                child->Reset();
            }
        }
    }

    // SelectorNode implementation
    SelectorNode::SelectorNode(const std::string& name)
        : CompositeNode(name, CompositeType::SELECTOR) {
    }

    NodeStatus SelectorNode::Execute(BehaviorContext& context) {
        if (!m_enabled || m_children.empty()) {
            return NodeStatus::FAILURE;
        }

        auto startTime = std::chrono::high_resolution_clock::now();

        NodeStatus result = NodeStatus::FAILURE;

        for (size_t i = m_currentChildIndex; i < m_children.size(); ++i) {
            auto child = m_children[i];
            NodeStatus childResult = child->Execute(context);

            if (childResult == NodeStatus::SUCCESS) {
                result = NodeStatus::SUCCESS;
                m_currentChildIndex = 0; // Reset for next execution
                break;
            } else if (childResult == NodeStatus::RUNNING) {
                result = NodeStatus::RUNNING;
                m_currentChildIndex = i; // Continue from here next time
                break;
            }
        }

        if (result == NodeStatus::FAILURE) {
            m_currentChildIndex = 0; // Reset for next execution
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        float executionTime = std::chrono::duration<float>(endTime - startTime).count();

        UpdateStats(result, executionTime);
        return result;
    }

    void SelectorNode::Reset() {
        BehaviorNode::Reset();
        m_currentChildIndex = 0;
        for (auto& child : m_children) {
            if (child) {
                child->Reset();
            }
        }
    }

    // ParallelNode implementation
    ParallelNode::ParallelNode(const std::string& name, int successThreshold, int failureThreshold)
        : CompositeNode(name, CompositeType::PARALLEL),
          m_successThreshold(successThreshold),
          m_failureThreshold(failureThreshold) {
    }

    NodeStatus ParallelNode::Execute(BehaviorContext& context) {
        if (!m_enabled || m_children.empty()) {
            return NodeStatus::FAILURE;
        }

        auto startTime = std::chrono::high_resolution_clock::now();

        m_childStatuses.clear();
        m_childStatuses.resize(m_children.size(), NodeStatus::INVALID);

        int successCount = 0;
        int failureCount = 0;
        int runningCount = 0;

        for (size_t i = 0; i < m_children.size(); ++i) {
            auto child = m_children[i];
            NodeStatus childResult = child->Execute(context);
            m_childStatuses[i] = childResult;

            switch (childResult) {
                case NodeStatus::SUCCESS:
                    successCount++;
                    break;
                case NodeStatus::FAILURE:
                    failureCount++;
                    break;
                case NodeStatus::RUNNING:
                    runningCount++;
                    break;
                default:
                    break;
            }
        }

        NodeStatus result = NodeStatus::RUNNING;

        if (successCount >= m_successThreshold) {
            result = NodeStatus::SUCCESS;
        } else if (failureCount >= m_failureThreshold) {
            result = NodeStatus::FAILURE;
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        float executionTime = std::chrono::duration<float>(endTime - startTime).count();

        UpdateStats(result, executionTime);
        return result;
    }

    void ParallelNode::Reset() {
        BehaviorNode::Reset();
        m_childStatuses.clear();
        for (auto& child : m_children) {
            if (child) {
                child->Reset();
            }
        }
    }

    // DecoratorNode implementation
    DecoratorNode::DecoratorNode(const std::string& name, DecoratorType type, std::shared_ptr<BehaviorNode> child)
        : BehaviorNode(name, NodeType::DECORATOR), m_decoratorType(type), m_child(child) {
    }

    // InverterNode implementation
    InverterNode::InverterNode(const std::string& name, std::shared_ptr<BehaviorNode> child)
        : DecoratorNode(name, DecoratorType::INVERTER, child) {
    }

    NodeStatus InverterNode::Execute(BehaviorContext& context) {
        if (!m_enabled || !m_child) {
            return NodeStatus::FAILURE;
        }

        auto startTime = std::chrono::high_resolution_clock::now();

        NodeStatus childResult = m_child->Execute(context);
        NodeStatus result = NodeStatus::INVALID;

        switch (childResult) {
            case NodeStatus::SUCCESS:
                result = NodeStatus::FAILURE;
                break;
            case NodeStatus::FAILURE:
                result = NodeStatus::SUCCESS;
                break;
            case NodeStatus::RUNNING:
                result = NodeStatus::RUNNING;
                break;
            default:
                result = NodeStatus::FAILURE;
                break;
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        float executionTime = std::chrono::duration<float>(endTime - startTime).count();

        UpdateStats(result, executionTime);
        return result;
    }

    // RepeatNode implementation
    RepeatNode::RepeatNode(const std::string& name, std::shared_ptr<BehaviorNode> child, int count)
        : DecoratorNode(name, DecoratorType::REPEAT, child),
          m_repeatCount(count), m_currentCount(0) {
    }

    NodeStatus RepeatNode::Execute(BehaviorContext& context) {
        if (!m_enabled || !m_child) {
            return NodeStatus::FAILURE;
        }

        auto startTime = std::chrono::high_resolution_clock::now();

        NodeStatus result = NodeStatus::SUCCESS;

        while ((m_repeatCount == -1 || m_currentCount < m_repeatCount)) {
            NodeStatus childResult = m_child->Execute(context);

            if (childResult == NodeStatus::FAILURE) {
                result = NodeStatus::FAILURE;
                break;
            } else if (childResult == NodeStatus::RUNNING) {
                result = NodeStatus::RUNNING;
                break;
            }

            m_currentCount++;

            if (m_repeatCount != -1 && m_currentCount >= m_repeatCount) {
                break;
            }
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        float executionTime = std::chrono::duration<float>(endTime - startTime).count();

        UpdateStats(result, executionTime);
        return result;
    }

    void RepeatNode::Reset() {
        BehaviorNode::Reset();
        m_currentCount = 0;
        if (m_child) {
            m_child->Reset();
        }
    }

    // BehaviorTree implementation
    BehaviorTree::BehaviorTree(const std::string& name)
        : m_name(name) {
    }

    NodeStatus BehaviorTree::Execute(BehaviorContext& context) {
        if (!m_root) {
            return NodeStatus::FAILURE;
        }

        auto startTime = std::chrono::steady_clock::now();

        NodeStatus result = m_root->Execute(context);

        auto endTime = std::chrono::steady_clock::now();
        float executionTime = std::chrono::duration<float>(endTime - startTime).count();

        // Update statistics
        m_stats.totalExecutions++;
        if (result == NodeStatus::SUCCESS) {
            m_stats.successfulExecutions++;
        } else if (result == NodeStatus::FAILURE) {
            m_stats.failedExecutions++;
        }

        m_stats.averageExecutionTime = (m_stats.averageExecutionTime * (m_stats.totalExecutions - 1) + executionTime) / m_stats.totalExecutions;
        m_stats.minExecutionTime = std::min(m_stats.minExecutionTime, executionTime);
        m_stats.maxExecutionTime = std::max(m_stats.maxExecutionTime, executionTime);

        return result;
    }

    void BehaviorTree::Reset() {
        if (m_root) {
            m_root->Reset();
        }
        m_lastExecutionTime = std::chrono::steady_clock::now();
    }

    std::string BehaviorTree::GetDebugInfo() const {
        std::stringstream ss;
        ss << "BehaviorTree: " << m_name << std::endl;
        ss << "Executions: " << m_stats.totalExecutions << " (S:" << m_stats.successfulExecutions
           << " F:" << m_stats.failedExecutions << ")" << std::endl;
        ss << "Avg Time: " << std::fixed << std::setprecision(3) << m_stats.averageExecutionTime * 1000 << "ms" << std::endl;
        ss << "Min/Max Time: " << m_stats.minExecutionTime * 1000 << "/" << m_stats.maxExecutionTime * 1000 << "ms" << std::endl;

        if (m_root) {
            ss << "Root: " << std::endl << TraverseDebugInfo(m_root, 1);
        }

        return ss.str();
    }

    std::string BehaviorTree::TraverseDebugInfo(std::shared_ptr<BehaviorNode> node, int indent) const {
        if (!node) {
            return "";
        }

        std::stringstream ss;
        std::string indentStr(indent * 2, ' ');

        ss << indentStr << node->GetDebugInfo() << std::endl;

        // If it's a composite or decorator, traverse children
        if (node->GetType() == NodeType::COMPOSITE) {
            auto composite = std::dynamic_pointer_cast<CompositeNode>(node);
            if (composite) {
                for (auto& child : composite->GetChildren()) {
                    ss << TraverseDebugInfo(child, indent + 1);
                }
            }
        } else if (node->GetType() == NodeType::DECORATOR) {
            auto decorator = std::dynamic_pointer_cast<DecoratorNode>(node);
            if (decorator && decorator->GetChild()) {
                ss << TraverseDebugInfo(decorator->GetChild(), indent + 1);
            }
        }

        return ss.str();
    }

} // namespace VoxelCraft
