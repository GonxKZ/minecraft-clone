/**
 * @file EventSystem.hpp
 * @brief VoxelCraft Advanced Event System Header
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_EVENTS_EVENT_SYSTEM_HPP
#define VOXELCRAFT_EVENTS_EVENT_SYSTEM_HPP

#include <functional>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <atomic>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <chrono>
#include <string>
#include <any>
#include <type_traits>
#include <optional>

#include "../core/System.hpp"
#include "../utils/Delegate.hpp"
#include "../utils/TypeTraits.hpp"

namespace VoxelCraft {

    // Forward declarations
    class EventBase;
    class EventHandler;
    class EventDispatcher;
    class EventQueue;
    class EventFilter;
    class EventProfiler;
    class EventSerializer;

    /**
     * @brief Unique identifier for event types
     */
    using EventTypeId = uint32_t;

    /**
     * @brief Unique identifier for event handlers
     */
    using HandlerId = uint64_t;

    /**
     * @brief Unique identifier for event instances
     */
    using EventId = uint64_t;

    /**
     * @brief Priority levels for event handling
     */
    enum class EventPriority {
        LOWEST = 0,
        LOW = 25,
        NORMAL = 50,
        HIGH = 75,
        HIGHEST = 100,
        IMMEDIATE = 999
    };

    /**
     * @brief Event delivery modes
     */
    enum class EventDelivery {
        SYNCHRONOUS,        // Immediate processing
        ASYNCHRONOUS,       // Queued processing
        DEFERRED,          // Process at end of frame
        NETWORK_REPLICATED // Replicate across network
    };

    /**
     * @brief Event filtering modes
     */
    enum class EventFilterMode {
        WHITELIST,          // Only allow specified types
        BLACKLIST,          // Block specified types
        PRIORITY_FILTER,    // Only allow certain priorities
        CUSTOM_FILTER       // Use custom filter function
    };

    /**
     * @brief Base class for all events
     */
    class EventBase {
    public:
        EventBase(EventTypeId typeId, const std::string& typeName);
        virtual ~EventBase() = default;

        EventId GetId() const { return m_id; }
        EventTypeId GetTypeId() const { return m_typeId; }
        const std::string& GetTypeName() const { return m_typeName; }

        std::chrono::system_clock::time_point GetTimestamp() const { return m_timestamp; }
        EventPriority GetPriority() const { return m_priority; }
        void SetPriority(EventPriority priority) { m_priority = priority; }

        EventDelivery GetDelivery() const { return m_delivery; }
        void SetDelivery(EventDelivery delivery) { m_delivery = delivery; }

        bool IsConsumed() const { return m_consumed; }
        void Consume() { m_consumed = true; }

        bool IsCancelled() const { return m_cancelled; }
        void Cancel() { m_cancelled = true; }

        const std::unordered_map<std::string, std::any>& GetMetadata() const { return m_metadata; }
        void SetMetadata(const std::string& key, const std::any& value) { m_metadata[key] = value; }

        template<typename T>
        std::optional<T> GetMetadata(const std::string& key) const {
            auto it = m_metadata.find(key);
            if (it != m_metadata.end() && it->second.type() == typeid(T)) {
                return std::any_cast<T>(it->second);
            }
            return std::nullopt;
        }

        virtual std::unique_ptr<EventBase> Clone() const = 0;
        virtual std::string ToString() const = 0;

        // Serialization support
        virtual bool Serialize(std::vector<uint8_t>& buffer) const { return false; }
        virtual bool Deserialize(const std::vector<uint8_t>& buffer) { return false; }

    protected:
        EventId m_id;
        EventTypeId m_typeId;
        std::string m_typeName;
        std::chrono::system_clock::time_point m_timestamp;
        EventPriority m_priority;
        EventDelivery m_delivery;
        bool m_consumed;
        bool m_cancelled;
        std::unordered_map<std::string, std::any> m_metadata;
    };

    /**
     * @brief Template base class for typed events
     */
    template<typename T>
    class Event : public EventBase {
    public:
        static EventTypeId GetStaticTypeId() {
            static EventTypeId typeId = GenerateTypeId();
            return typeId;
        }

        static std::string GetStaticTypeName() {
            return typeid(T).name();
        }

        Event()
            : EventBase(GetStaticTypeId(), GetStaticTypeName()) {
        }

        std::unique_ptr<EventBase> Clone() const override {
            return std::make_unique<T>(static_cast<const T&>(*this));
        }

        std::string ToString() const override {
            return GetStaticTypeName();
        }

    private:
        static EventTypeId GenerateTypeId() {
            static std::atomic<EventTypeId> nextId{1};
            return nextId++;
        }
    };

    /**
     * @brief Event handler interface
     */
    class EventHandler {
    public:
        EventHandler(const std::string& name = "Anonymous");
        virtual ~EventHandler() = default;

        HandlerId GetId() const { return m_id; }
        const std::string& GetName() const { return m_name; }

        virtual bool CanHandle(EventTypeId typeId) const = 0;
        virtual bool HandleEvent(const EventBase& event) = 0;

        bool IsEnabled() const { return m_enabled; }
        void SetEnabled(bool enabled) { m_enabled = enabled; }

        EventPriority GetPriority() const { return m_priority; }
        void SetPriority(EventPriority priority) { m_priority = priority; }

        const std::unordered_set<EventTypeId>& GetSubscribedTypes() const { return m_subscribedTypes; }
        void SubscribeToType(EventTypeId typeId) { m_subscribedTypes.insert(typeId); }
        void UnsubscribeFromType(EventTypeId typeId) { m_subscribedTypes.erase(typeId); }

    protected:
        HandlerId m_id;
        std::string m_name;
        bool m_enabled;
        EventPriority m_priority;
        std::unordered_set<EventTypeId> m_subscribedTypes;
    };

    /**
     * @brief Template event handler for specific event types
     */
    template<typename T>
    class TypedEventHandler : public EventHandler {
    public:
        using EventCallback = std::function<bool(const T&)>;

        TypedEventHandler(const std::string& name = "TypedHandler", EventCallback callback = nullptr)
            : EventHandler(name), m_callback(callback) {
            SubscribeToType(T::GetStaticTypeId());
        }

        bool CanHandle(EventTypeId typeId) const override {
            return typeId == T::GetStaticTypeId();
        }

        bool HandleEvent(const EventBase& event) override {
            if (!m_enabled || !m_callback) {
                return false;
            }

            if (event.GetTypeId() != T::GetStaticTypeId()) {
                return false;
            }

            const T* typedEvent = dynamic_cast<const T*>(&event);
            if (!typedEvent) {
                return false;
            }

            return m_callback(*typedEvent);
        }

        void SetCallback(EventCallback callback) {
            m_callback = callback;
        }

    private:
        EventCallback m_callback;
    };

    /**
     * @brief Generic event handler using function objects
     */
    class GenericEventHandler : public EventHandler {
    public:
        using EventCallback = std::function<bool(const EventBase&)>;

        GenericEventHandler(const std::string& name = "GenericHandler", EventCallback callback = nullptr)
            : EventHandler(name), m_callback(callback) {
        }

        bool CanHandle(EventTypeId typeId) const override {
            return m_subscribedTypes.find(typeId) != m_subscribedTypes.end();
        }

        bool HandleEvent(const EventBase& event) override {
            if (!m_enabled || !m_callback) {
                return false;
            }

            if (!CanHandle(event.GetTypeId())) {
                return false;
            }

            return m_callback(event);
        }

        void SetCallback(EventCallback callback) {
            m_callback = callback;
        }

    private:
        EventCallback m_callback;
    };

    /**
     * @brief Event filter interface
     */
    class EventFilter {
    public:
        EventFilter(const std::string& name = "Filter");
        virtual ~EventFilter() = default;

        const std::string& GetName() const { return m_name; }
        bool IsEnabled() const { return m_enabled; }
        void SetEnabled(bool enabled) { m_enabled = enabled; }

        virtual bool ShouldFilter(const EventBase& event) = 0;

    protected:
        std::string m_name;
        bool m_enabled;
    };

    /**
     * @brief Type-based event filter
     */
    class TypeEventFilter : public EventFilter {
    public:
        TypeEventFilter(EventFilterMode mode, const std::string& name = "TypeFilter");

        void AddType(EventTypeId typeId);
        void RemoveType(EventTypeId typeId);
        void ClearTypes();

        bool ShouldFilter(const EventBase& event) override;

    private:
        EventFilterMode m_mode;
        std::unordered_set<EventTypeId> m_types;
    };

    /**
     * @brief Priority-based event filter
     */
    class PriorityEventFilter : public EventFilter {
    public:
        PriorityEventFilter(EventPriority minPriority, EventPriority maxPriority,
                           const std::string& name = "PriorityFilter");

        bool ShouldFilter(const EventBase& event) override;

    private:
        EventPriority m_minPriority;
        EventPriority m_maxPriority;
    };

    /**
     * @brief Custom event filter with predicate
     */
    class CustomEventFilter : public EventFilter {
    public:
        using FilterPredicate = std::function<bool(const EventBase&)>;

        CustomEventFilter(FilterPredicate predicate, const std::string& name = "CustomFilter");

        bool ShouldFilter(const EventBase& event) override;

    private:
        FilterPredicate m_predicate;
    };

    /**
     * @brief Event queue for asynchronous processing
     */
    class EventQueue {
    public:
        EventQueue(size_t maxSize = 10000);
        ~EventQueue();

        bool Push(std::unique_ptr<EventBase> event);
        std::unique_ptr<EventBase> Pop();
        bool IsEmpty() const;
        size_t Size() const;
        void Clear();

        void SetMaxSize(size_t maxSize) { m_maxSize = maxSize; }
        size_t GetMaxSize() const { return m_maxSize; }

    private:
        mutable std::mutex m_mutex;
        std::condition_variable m_condition;
        std::queue<std::unique_ptr<EventBase>> m_queue;
        size_t m_maxSize;
    };

    /**
     * @brief Event profiler for performance monitoring
     */
    class EventProfiler {
    public:
        EventProfiler();
        ~EventProfiler() = default;

        void StartProfiling(EventTypeId typeId);
        void StopProfiling(EventTypeId typeId);

        void RecordEventDispatch(const EventBase& event, uint64_t processingTimeNs);
        void RecordHandlerExecution(HandlerId handlerId, EventTypeId eventType, uint64_t executionTimeNs);

        struct EventStats {
            uint64_t totalDispatches;
            uint64_t totalProcessingTimeNs;
            uint64_t averageProcessingTimeNs;
            uint64_t maxProcessingTimeNs;
            uint64_t minProcessingTimeNs;
        };

        EventStats GetEventStats(EventTypeId typeId) const;
        std::unordered_map<EventTypeId, EventStats> GetAllEventStats() const;

        void Reset();

    private:
        mutable std::shared_mutex m_mutex;
        std::unordered_map<EventTypeId, EventStats> m_eventStats;
        std::unordered_map<EventTypeId, std::chrono::steady_clock::time_point> m_activeProfiles;
    };

    /**
     * @brief Main event dispatcher system
     */
    class EventDispatcher : public System {
    public:
        static EventDispatcher& GetInstance();

        EventDispatcher();
        ~EventDispatcher() override;

        // System interface
        bool Initialize() override;
        void Update(float deltaTime) override;
        void Shutdown() override;
        std::string GetName() const override { return "EventSystem"; }
        System::Type GetType() const override { return System::Type::EVENT; }

        // Event dispatching
        void Dispatch(std::unique_ptr<EventBase> event);
        void DispatchImmediate(std::unique_ptr<EventBase> event);
        void QueueEvent(std::unique_ptr<EventBase> event);

        // Handler management
        HandlerId RegisterHandler(std::shared_ptr<EventHandler> handler);
        bool UnregisterHandler(HandlerId handlerId);
        bool UnregisterHandler(const std::string& name);
        std::shared_ptr<EventHandler> GetHandler(HandlerId handlerId) const;
        std::shared_ptr<EventHandler> GetHandler(const std::string& name) const;

        // Filter management
        void AddGlobalFilter(std::shared_ptr<EventFilter> filter);
        void RemoveGlobalFilter(const std::string& name);
        std::shared_ptr<EventFilter> GetGlobalFilter(const std::string& name) const;

        // Type registration
        void RegisterEventType(EventTypeId typeId, const std::string& typeName);
        bool IsEventTypeRegistered(EventTypeId typeId) const;
        std::string GetEventTypeName(EventTypeId typeId) const;

        // Configuration
        void SetAsyncProcessingEnabled(bool enabled) { m_asyncEnabled = enabled; }
        bool IsAsyncProcessingEnabled() const { return m_asyncEnabled; }

        void SetMaxQueueSize(size_t size) { m_eventQueue.SetMaxSize(size); }
        size_t GetMaxQueueSize() const { return m_eventQueue.GetMaxSize(); }

        void SetProfilingEnabled(bool enabled) { m_profilingEnabled = enabled; }
        bool IsProfilingEnabled() const { return m_profilingEnabled; }

        // Query and monitoring
        size_t GetHandlerCount() const;
        size_t GetQueuedEventCount() const;
        size_t GetProcessedEventCount() const;

        std::vector<HandlerId> GetAllHandlerIds() const;
        std::vector<std::string> GetAllHandlerNames() const;

        // Performance monitoring
        EventProfiler::EventStats GetEventStats(EventTypeId typeId) const;
        std::unordered_map<EventTypeId, EventProfiler::EventStats> GetAllEventStats() const;

        // Utility methods
        void FlushEventQueue();
        void ClearAllHandlers();
        void ResetStatistics();

        std::string GetSystemInfo() const;

    private:
        // Internal processing
        void ProcessEvent(std::unique_ptr<EventBase> event);
        void ProcessImmediateEvent(std::unique_ptr<EventBase> event);
        void ProcessQueuedEvents();

        // Async processing
        void StartWorkerThreads();
        void StopWorkerThreads();
        void WorkerThreadFunction();

        // Handler execution
        bool ExecuteHandler(const std::shared_ptr<EventHandler>& handler, const EventBase& event);
        void ExecuteHandlers(const EventBase& event, std::vector<std::shared_ptr<EventHandler>>& handlers);

        // Filtering
        bool ShouldFilterEvent(const EventBase& event) const;

        // Profiling
        void ProfileEventDispatch(const EventBase& event, uint64_t processingTimeNs);

        // Statistics
        void UpdateStatistics(const EventBase& event);

    private:
        // Core components
        mutable std::shared_mutex m_handlersMutex;
        std::unordered_map<HandlerId, std::shared_ptr<EventHandler>> m_handlers;
        std::unordered_map<std::string, HandlerId> m_handlerNames;

        mutable std::shared_mutex m_filtersMutex;
        std::vector<std::shared_ptr<EventFilter>> m_globalFilters;

        mutable std::shared_mutex m_typesMutex;
        std::unordered_map<EventTypeId, std::string> m_registeredTypes;

        EventQueue m_eventQueue;
        EventProfiler m_profiler;

        // Async processing
        std::atomic<bool> m_asyncEnabled;
        std::atomic<bool> m_running;
        std::vector<std::thread> m_workerThreads;
        size_t m_workerThreadCount;

        // Profiling
        std::atomic<bool> m_profilingEnabled;

        // Statistics
        mutable std::shared_mutex m_statsMutex;
        uint64_t m_processedEventCount;
        uint64_t m_queuedEventCount;
        uint64_t m_filteredEventCount;

        // ID generation
        static std::atomic<HandlerId> s_nextHandlerId;
        static std::atomic<EventId> s_nextEventId;
    };

    /**
     * @brief Event system configuration
     */
    struct EventSystemConfig {
        bool enableAsyncProcessing = true;
        size_t maxQueueSize = 10000;
        size_t workerThreadCount = 4;
        bool enableProfiling = true;
        bool enableNetworkReplication = false;
        std::string networkAddress = "localhost";
        uint16_t networkPort = 0;
        bool enableSerialization = true;
    };

    /**
     * @brief Helper macros for event creation and dispatching
     */
    #define DECLARE_EVENT_TYPE(EventType) \
        static const EventTypeId TYPE_ID = EventType::GetStaticTypeId();

    #define DEFINE_EVENT(EventType) \
        class EventType : public Event<EventType> { \
        public: \
            EventType() = default; \
            static std::string GetTypeName() { return #EventType; }

    #define END_DEFINE_EVENT() \
        };

    #define REGISTER_EVENT_TYPE(EventType) \
        EventDispatcher::GetInstance().RegisterEventType(EventType::GetStaticTypeId(), EventType::GetTypeName());

    #define DISPATCH_EVENT(EventType, ...) \
        EventDispatcher::GetInstance().Dispatch(std::make_unique<EventType>(__VA_ARGS__));

    #define QUEUE_EVENT(EventType, ...) \
        EventDispatcher::GetInstance().QueueEvent(std::make_unique<EventType>(__VA_ARGS__));

    // Common event types
    namespace Events {

        // Input Events
        DEFINE_EVENT(KeyPressedEvent)
            int key;
            int scancode;
            int mods;
        END_DEFINE_EVENT()

        DEFINE_EVENT(KeyReleasedEvent)
            int key;
            int scancode;
            int mods;
        END_DEFINE_EVENT()

        DEFINE_EVENT(MouseButtonPressedEvent)
            int button;
            int mods;
        END_DEFINE_EVENT()

        DEFINE_EVENT(MouseButtonReleasedEvent)
            int button;
            int mods;
        END_DEFINE_EVENT()

        DEFINE_EVENT(MouseMovedEvent)
            double x;
            double y;
            double dx;
            double dy;
        END_DEFINE_EVENT()

        DEFINE_EVENT(MouseScrolledEvent)
            double xOffset;
            double yOffset;
        END_DEFINE_EVENT()

        DEFINE_EVENT(GamepadConnectedEvent)
            int gamepadId;
            std::string name;
        END_DEFINE_EVENT()

        DEFINE_EVENT(GamepadDisconnectedEvent)
            int gamepadId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(GamepadButtonPressedEvent)
            int gamepadId;
            int button;
        END_DEFINE_EVENT()

        DEFINE_EVENT(GamepadButtonReleasedEvent)
            int gamepadId;
            int button;
        END_DEFINE_EVENT()

        DEFINE_EVENT(GamepadAxisMovedEvent)
            int gamepadId;
            int axis;
            float value;
        END_DEFINE_EVENT()

        DEFINE_EVENT(TouchStartedEvent)
            int finger;
            double x;
            double y;
        END_DEFINE_EVENT()

        DEFINE_EVENT(TouchMovedEvent)
            int finger;
            double x;
            double y;
            double dx;
            double dy;
        END_DEFINE_EVENT()

        DEFINE_EVENT(TouchEndedEvent)
            int finger;
            double x;
            double y;
        END_DEFINE_EVENT()

        // Player Events
        DEFINE_EVENT(PlayerSpawnedEvent)
            uint32_t playerId;
            glm::vec3 position;
            std::string playerName;
        END_DEFINE_EVENT()

        DEFINE_EVENT(PlayerDiedEvent)
            uint32_t playerId;
            uint32_t killerId;
            std::string deathReason;
        END_DEFINE_EVENT()

        DEFINE_EVENT(PlayerHealthChangedEvent)
            uint32_t playerId;
            float oldHealth;
            float newHealth;
            float maxHealth;
        END_DEFINE_EVENT()

        DEFINE_EVENT(PlayerHungerChangedEvent)
            uint32_t playerId;
            float oldHunger;
            float newHunger;
            float maxHunger;
        END_DEFINE_EVENT()

        DEFINE_EVENT(PlayerThirstChangedEvent)
            uint32_t playerId;
            float oldThirst;
            float newThirst;
            float maxThirst;
        END_DEFINE_EVENT()

        DEFINE_EVENT(PlayerStaminaChangedEvent)
            uint32_t playerId;
            float oldStamina;
            float newStamina;
            float maxStamina;
        END_DEFINE_EVENT()

        DEFINE_EVENT(PlayerTemperatureChangedEvent)
            uint32_t playerId;
            float oldTemperature;
            float newTemperature;
            float minTemperature;
            float maxTemperature;
        END_DEFINE_EVENT()

        DEFINE_EVENT(PlayerOxygenChangedEvent)
            uint32_t playerId;
            float oldOxygen;
            float newOxygen;
            float maxOxygen;
        END_DEFINE_EVENT()

        DEFINE_EVENT(PlayerSanityChangedEvent)
            uint32_t playerId;
            float oldSanity;
            float newSanity;
            float maxSanity;
        END_DEFINE_EVENT()

        DEFINE_EVENT(PlayerLevelChangedEvent)
            uint32_t playerId;
            uint32_t oldLevel;
            uint32_t newLevel;
            uint32_t experience;
        END_DEFINE_EVENT()

        DEFINE_EVENT(PlayerSkillChangedEvent)
            uint32_t playerId;
            std::string skillName;
            float oldValue;
            float newValue;
            float maxValue;
        END_DEFINE_EVENT()

        DEFINE_EVENT(PlayerEquipmentChangedEvent)
            uint32_t playerId;
            uint32_t slot;
            uint32_t oldItemId;
            uint32_t newItemId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(PlayerInventoryChangedEvent)
            uint32_t playerId;
            uint32_t itemId;
            int32_t delta;
            uint32_t newCount;
        END_DEFINE_EVENT()

        // World Events
        DEFINE_EVENT(WorldLoadedEvent)
            std::string worldName;
            uint32_t seed;
            glm::vec3 spawnPosition;
        END_DEFINE_EVENT()

        DEFINE_EVENT(WorldUnloadedEvent)
            std::string worldName;
        END_DEFINE_EVENT()

        DEFINE_EVENT(ChunkLoadedEvent)
            int32_t chunkX;
            int32_t chunkY;
            int32_t chunkZ;
            uint32_t blockCount;
        END_DEFINE_EVENT()

        DEFINE_EVENT(ChunkUnloadedEvent)
            int32_t chunkX;
            int32_t chunkY;
            int32_t chunkZ;
        END_DEFINE_EVENT()

        DEFINE_EVENT(BlockPlacedEvent)
            glm::vec3 position;
            uint32_t blockId;
            uint32_t oldBlockId;
            uint32_t playerId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(BlockBrokenEvent)
            glm::vec3 position;
            uint32_t blockId;
            uint32_t playerId;
            uint32_t toolId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(BlockChangedEvent)
            glm::vec3 position;
            uint32_t oldBlockId;
            uint32_t newBlockId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(LiquidFlowEvent)
            glm::vec3 position;
            uint32_t liquidId;
            uint32_t level;
            glm::vec3 flowDirection;
        END_DEFINE_EVENT()

        DEFINE_EVENT(WeatherChangedEvent)
            std::string oldWeather;
            std::string newWeather;
            float intensity;
        END_DEFINE_EVENT()

        DEFINE_EVENT(TimeChangedEvent)
            float oldTime;
            float newTime;
            bool isDay;
        END_DEFINE_EVENT()

        DEFINE_EVENT(BiomeChangedEvent)
            glm::vec3 position;
            uint32_t oldBiomeId;
            uint32_t newBiomeId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(StructureGeneratedEvent)
            std::string structureName;
            glm::vec3 position;
            glm::vec3 size;
        END_DEFINE_EVENT()

        DEFINE_EVENT(TerrainModifiedEvent)
            glm::vec3 position;
            float radius;
            uint32_t modificationType;
        END_DEFINE_EVENT()

        // Entity Events
        DEFINE_EVENT(EntitySpawnedEvent)
            uint32_t entityId;
            std::string entityType;
            glm::vec3 position;
        END_DEFINE_EVENT()

        DEFINE_EVENT(EntityDespawnedEvent)
            uint32_t entityId;
            std::string reason;
        END_DEFINE_EVENT()

        DEFINE_EVENT(EntityMovedEvent)
            uint32_t entityId;
            glm::vec3 oldPosition;
            glm::vec3 newPosition;
            glm::vec3 velocity;
        END_DEFINE_EVENT()

        DEFINE_EVENT(EntityRotatedEvent)
            uint32_t entityId;
            glm::vec3 oldRotation;
            glm::vec3 newRotation;
        END_DEFINE_EVENT()

        DEFINE_EVENT(EntityDamagedEvent)
            uint32_t entityId;
            uint32_t attackerId;
            float damage;
            std::string damageType;
        END_DEFINE_EVENT()

        DEFINE_EVENT(EntityHealedEvent)
            uint32_t entityId;
            uint32_t healerId;
            float healAmount;
        END_DEFINE_EVENT()

        DEFINE_EVENT(EntityDiedEvent)
            uint32_t entityId;
            uint32_t killerId;
            std::string deathReason;
        END_DEFINE_EVENT()

        DEFINE_EVENT(EntityInteractedEvent)
            uint32_t entityId;
            uint32_t interactorId;
            std::string interactionType;
        END_DEFINE_EVENT()

        // AI Events
        DEFINE_EVENT(AIStateChangedEvent)
            uint32_t entityId;
            std::string oldState;
            std::string newState;
        END_DEFINE_EVENT()

        DEFINE_EVENT(AIPathFoundEvent)
            uint32_t entityId;
            std::vector<glm::vec3> path;
            bool success;
        END_DEFINE_EVENT()

        DEFINE_EVENT(AITargetChangedEvent)
            uint32_t entityId;
            uint32_t oldTargetId;
            uint32_t newTargetId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(AIBehaviorTriggeredEvent)
            uint32_t entityId;
            std::string behaviorName;
            std::string triggerReason;
        END_DEFINE_EVENT()

        // Network Events
        DEFINE_EVENT(NetworkConnectedEvent)
            uint32_t connectionId;
            std::string address;
            uint16_t port;
        END_DEFINE_EVENT()

        DEFINE_EVENT(NetworkDisconnectedEvent)
            uint32_t connectionId;
            std::string reason;
        END_DEFINE_EVENT()

        DEFINE_EVENT(NetworkPacketReceivedEvent)
            uint32_t connectionId;
            uint32_t packetType;
            std::vector<uint8_t> data;
        END_DEFINE_EVENT()

        DEFINE_EVENT(NetworkPacketSentEvent)
            uint32_t connectionId;
            uint32_t packetType;
            size_t dataSize;
        END_DEFINE_EVENT()

        DEFINE_EVENT(NetworkLatencyChangedEvent)
            uint32_t connectionId;
            uint32_t oldLatency;
            uint32_t newLatency;
        END_DEFINE_EVENT()

        DEFINE_EVENT(NetworkErrorEvent)
            uint32_t connectionId;
            std::string errorMessage;
            int errorCode;
        END_DEFINE_EVENT()

        // GUI Events
        DEFINE_EVENT(GUIWindowOpenedEvent)
            std::string windowName;
            uint32_t windowId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(GUIWindowClosedEvent)
            std::string windowName;
            uint32_t windowId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(GUIButtonClickedEvent)
            std::string buttonName;
            uint32_t buttonId;
            int mouseButton;
        END_DEFINE_EVENT()

        DEFINE_EVENT(GUISliderChangedEvent)
            std::string sliderName;
            uint32_t sliderId;
            float oldValue;
            float newValue;
        END_DEFINE_EVENT()

        DEFINE_EVENT(GUITextInputEvent)
            std::string inputName;
            uint32_t inputId;
            std::string text;
        END_DEFINE_EVENT()

        DEFINE_EVENT(GUIFocusChangedEvent)
            uint32_t oldFocusId;
            uint32_t newFocusId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(GUIHoverEvent)
            uint32_t elementId;
            bool entered;
        END_DEFINE_EVENT()

        // Audio Events
        DEFINE_EVENT(SoundPlayedEvent)
            std::string soundName;
            uint32_t soundId;
            glm::vec3 position;
            float volume;
            float pitch;
        END_DEFINE_EVENT()

        DEFINE_EVENT(SoundStoppedEvent)
            uint32_t soundId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(MusicChangedEvent)
            std::string oldTrack;
            std::string newTrack;
            bool loop;
        END_DEFINE_EVENT()

        DEFINE_EVENT(AudioVolumeChangedEvent)
            std::string channel;
            float oldVolume;
            float newVolume;
        END_DEFINE_EVENT()

        // Physics Events
        DEFINE_EVENT(CollisionEvent)
            uint32_t entityA;
            uint32_t entityB;
            glm::vec3 contactPoint;
            glm::vec3 normal;
            float impulse;
        END_DEFINE_EVENT()

        DEFINE_EVENT(PhysicsSimulationStepEvent)
            float deltaTime;
            uint32_t stepCount;
        END_DEFINE_EVENT()

        DEFINE_EVENT(RigidbodyAddedEvent)
            uint32_t entityId;
            float mass;
        END_DEFINE_EVENT()

        DEFINE_EVENT(RigidbodyRemovedEvent)
            uint32_t entityId;
        END_DEFINE_EVENT()

        // Crafting Events
        DEFINE_EVENT(CraftingStartedEvent)
            uint32_t playerId;
            uint32_t recipeId;
            std::vector<uint32_t> ingredients;
        END_DEFINE_EVENT()

        DEFINE_EVENT(CraftingCompletedEvent)
            uint32_t playerId;
            uint32_t recipeId;
            uint32_t resultItemId;
            uint32_t resultCount;
        END_DEFINE_EVENT()

        DEFINE_EVENT(CraftingFailedEvent)
            uint32_t playerId;
            uint32_t recipeId;
            std::string reason;
        END_DEFINE_EVENT()

        DEFINE_EVENT(RecipeUnlockedEvent)
            uint32_t playerId;
            uint32_t recipeId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(RecipeLockedEvent)
            uint32_t playerId;
            uint32_t recipeId;
        END_DEFINE_EVENT()

        // Achievement Events
        DEFINE_EVENT(AchievementUnlockedEvent)
            uint32_t playerId;
            std::string achievementName;
            uint32_t achievementId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(AchievementProgressEvent)
            uint32_t playerId;
            uint32_t achievementId;
            float oldProgress;
            float newProgress;
            float targetProgress;
        END_DEFINE_EVENT()

        // Multiplayer Events
        DEFINE_EVENT(PlayerJoinedEvent)
            uint32_t playerId;
            std::string playerName;
            std::string address;
        END_DEFINE_EVENT()

        DEFINE_EVENT(PlayerLeftEvent)
            uint32_t playerId;
            std::string reason;
        END_DEFINE_EVENT()

        DEFINE_EVENT(ChatMessageEvent)
            uint32_t senderId;
            std::string senderName;
            std::string message;
            bool isPrivate;
        END_DEFINE_EVENT()

        DEFINE_EVENT(PlayerKickedEvent)
            uint32_t playerId;
            std::string reason;
            uint32_t moderatorId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(PlayerBannedEvent)
            uint32_t playerId;
            std::string reason;
            uint32_t moderatorId;
            uint32_t banDuration;
        END_DEFINE_EVENT()

        // System Events
        DEFINE_EVENT(SystemInitializedEvent)
            std::string systemName;
        END_DEFINE_EVENT()

        DEFINE_EVENT(SystemShutdownEvent)
            std::string systemName;
        END_DEFINE_EVENT()

        DEFINE_EVENT(ConfigurationChangedEvent)
            std::string configFile;
            std::string changedKey;
            std::string oldValue;
            std::string newValue;
        END_DEFINE_EVENT()

        DEFINE_EVENT(LogMessageEvent)
            std::string level;
            std::string category;
            std::string message;
        END_DEFINE_EVENT()

        DEFINE_EVENT(PerformanceWarningEvent)
            std::string component;
            std::string warning;
            float severity;
        END_DEFINE_EVENT()

        DEFINE_EVENT(MemoryWarningEvent)
            uint64_t currentUsage;
            uint64_t peakUsage;
            float usagePercentage;
        END_DEFINE_EVENT()

        // Game State Events
        DEFINE_EVENT(GameStartedEvent)
            std::string gameMode;
            uint32_t seed;
            std::vector<uint32_t> players;
        END_DEFINE_EVENT()

        DEFINE_EVENT(GamePausedEvent)
            bool paused;
        END_DEFINE_EVENT()

        DEFINE_EVENT(GameSavedEvent)
            std::string saveName;
            std::string savePath;
        END_DEFINE_EVENT()

        DEFINE_EVENT(GameLoadedEvent)
            std::string saveName;
            std::string savePath;
        END_DEFINE_EVENT()

        DEFINE_EVENT(GameEndedEvent)
            std::string endReason;
            uint32_t winnerId;
            std::vector<uint32_t> scores;
        END_DEFINE_EVENT()

        // World Generation Events
        DEFINE_EVENT(TerrainGeneratedEvent)
            int32_t chunkX;
            int32_t chunkZ;
            uint32_t blockCount;
            float generationTime;
        END_DEFINE_EVENT()

        DEFINE_EVENT(CaveGeneratedEvent)
            int32_t chunkX;
            int32_t chunkY;
            int32_t chunkZ;
            uint32_t caveSize;
        END_DEFINE_EVENT()

        DEFINE_EVENT(OreGeneratedEvent)
            int32_t chunkX;
            int32_t chunkY;
            int32_t chunkZ;
            uint32_t oreType;
            uint32_t oreCount;
        END_DEFINE_EVENT()

        DEFINE_EVENT(VegetationGeneratedEvent)
            int32_t chunkX;
            int32_t chunkZ;
            uint32_t plantCount;
            uint32_t treeCount;
        END_DEFINE_EVENT()

        DEFINE_EVENT(AnimalSpawnedEvent)
            uint32_t animalId;
            std::string animalType;
            glm::vec3 position;
        END_DEFINE_EVENT()

        // Combat Events
        DEFINE_EVENT(CombatStartedEvent)
            uint32_t attackerId;
            uint32_t defenderId;
            std::string combatType;
        END_DEFINE_EVENT()

        DEFINE_EVENT(CombatEndedEvent)
            uint32_t winnerId;
            uint32_t loserId;
            std::string endReason;
        END_DEFINE_EVENT()

        DEFINE_EVENT(WeaponFiredEvent)
            uint32_t playerId;
            uint32_t weaponId;
            glm::vec3 position;
            glm::vec3 direction;
        END_DEFINE_EVENT()

        DEFINE_EVENT(ProjectileHitEvent)
            uint32_t projectileId;
            uint32_t targetId;
            glm::vec3 hitPosition;
            float damage;
        END_DEFINE_EVENT()

        DEFINE_EVENT(SpellCastEvent)
            uint32_t casterId;
            std::string spellName;
            uint32_t spellId;
            glm::vec3 targetPosition;
        END_DEFINE_EVENT()

        DEFINE_EVENT(SpellHitEvent)
            uint32_t casterId;
            uint32_t targetId;
            uint32_t spellId;
            float damage;
            std::string effect;
        END_DEFINE_EVENT()

        // Exploration Events
        DEFINE_EVENT(LocationDiscoveredEvent)
            uint32_t playerId;
            std::string locationName;
            glm::vec3 position;
            std::string discoveryType;
        END_DEFINE_EVENT()

        DEFINE_EVENT(BiomeDiscoveredEvent)
            uint32_t playerId;
            uint32_t biomeId;
            std::string biomeName;
            glm::vec3 position;
        END_DEFINE_EVENT()

        DEFINE_EVENT(StructureDiscoveredEvent)
            uint32_t playerId;
            std::string structureName;
            glm::vec3 position;
            float distanceFromPlayer;
        END_DEFINE_EVENT()

        DEFINE_EVENT(TreasureFoundEvent)
            uint32_t playerId;
            std::string treasureType;
            glm::vec3 position;
            uint32_t value;
        END_DEFINE_EVENT()

        DEFINE_EVENT(SecretFoundEvent)
            uint32_t playerId;
            std::string secretName;
            glm::vec3 position;
        END_DEFINE_EVENT()

        // Building Events
        DEFINE_EVENT(BuildingStartedEvent)
            uint32_t playerId;
            std::string buildingType;
            glm::vec3 position;
            glm::vec3 size;
        END_DEFINE_EVENT()

        DEFINE_EVENT(BuildingCompletedEvent)
            uint32_t playerId;
            uint32_t buildingId;
            std::string buildingType;
            float constructionTime;
        END_DEFINE_EVENT()

        DEFINE_EVENT(BuildingDestroyedEvent)
            uint32_t playerId;
            uint32_t buildingId;
            std::string destructionReason;
        END_DEFINE_EVENT()

        DEFINE_EVENT(BlueprintCreatedEvent)
            uint32_t playerId;
            std::string blueprintName;
            uint32_t blockCount;
        END_DEFINE_EVENT()

        DEFINE_EVENT(BlueprintUsedEvent)
            uint32_t playerId;
            std::string blueprintName;
            glm::vec3 position;
        END_DEFINE_EVENT()

        // Farming Events
        DEFINE_EVENT(CropPlantedEvent)
            uint32_t playerId;
            uint32_t cropType;
            glm::vec3 position;
            uint32_t count;
        END_DEFINE_EVENT()

        DEFINE_EVENT(CropHarvestedEvent)
            uint32_t playerId;
            uint32_t cropType;
            glm::vec3 position;
            uint32_t yield;
        END_DEFINE_EVENT()

        DEFINE_EVENT(AnimalTamedEvent)
            uint32_t playerId;
            uint32_t animalId;
            std::string animalType;
        END_DEFINE_EVENT()

        DEFINE_EVENT(AnimalBredEvent)
            uint32_t playerId;
            uint32_t parent1Id;
            uint32_t parent2Id;
            uint32_t offspringId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(AnimalSlaughteredEvent)
            uint32_t playerId;
            uint32_t animalId;
            std::string animalType;
            uint32_t meatYield;
        END_DEFINE_EVENT()

        // Magic Events
        DEFINE_EVENT(ManaChangedEvent)
            uint32_t playerId;
            float oldMana;
            float newMana;
            float maxMana;
        END_DEFINE_EVENT()

        DEFINE_EVENT(SpellLearnedEvent)
            uint32_t playerId;
            std::string spellName;
            uint32_t spellId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(EnchantmentAppliedEvent)
            uint32_t itemId;
            std::string enchantmentName;
            uint32_t enchantmentId;
            uint32_t level;
        END_DEFINE_EVENT()

        DEFINE_EVENT(PotionBrewedEvent)
            uint32_t playerId;
            std::string potionName;
            uint32_t potionId;
            std::vector<uint32_t> ingredients;
        END_DEFINE_EVENT()

        DEFINE_EVENT(PotionConsumedEvent)
            uint32_t playerId;
            uint32_t potionId;
            std::vector<std::string> effects;
        END_DEFINE_EVENT()

        // Vehicle Events
        DEFINE_EVENT(VehicleSpawnedEvent)
            uint32_t vehicleId;
            std::string vehicleType;
            glm::vec3 position;
        END_DEFINE_EVENT()

        DEFINE_EVENT(VehicleDestroyedEvent)
            uint32_t vehicleId;
            std::string destructionReason;
        END_DEFINE_EVENT()

        DEFINE_EVENT(VehicleEnteredEvent)
            uint32_t playerId;
            uint32_t vehicleId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(VehicleExitedEvent)
            uint32_t playerId;
            uint32_t vehicleId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(VehicleMovedEvent)
            uint32_t vehicleId;
            glm::vec3 oldPosition;
            glm::vec3 newPosition;
            glm::vec3 velocity;
        END_DEFINE_EVENT()

        // Mount Events
        DEFINE_EVENT(MountTamedEvent)
            uint32_t playerId;
            uint32_t mountId;
            std::string mountType;
        END_DEFINE_EVENT()

        DEFINE_EVENT(MountRiddenEvent)
            uint32_t playerId;
            uint32_t mountId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(MountDismountedEvent)
            uint32_t playerId;
            uint32_t mountId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(MountDiedEvent)
            uint32_t mountId;
            uint32_t killerId;
        END_DEFINE_EVENT()

        // Pet Events
        DEFINE_EVENT(PetTamedEvent)
            uint32_t playerId;
            uint32_t petId;
            std::string petType;
        END_DEFINE_EVENT()

        DEFINE_EVENT(PetCommandEvent)
            uint32_t playerId;
            uint32_t petId;
            std::string command;
        END_DEFINE_EVENT()

        DEFINE_EVENT(PetDiedEvent)
            uint32_t petId;
            uint32_t killerId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(PetLeveledEvent)
            uint32_t petId;
            uint32_t oldLevel;
            uint32_t newLevel;
        END_DEFINE_EVENT()

        // Dimension Events
        DEFINE_EVENT(DimensionChangedEvent)
            uint32_t entityId;
            std::string oldDimension;
            std::string newDimension;
            glm::vec3 position;
        END_DEFINE_EVENT()

        DEFINE_EVENT(DimensionCreatedEvent)
            std::string dimensionName;
            uint32_t dimensionId;
            std::string dimensionType;
        END_DEFINE_EVENT()

        DEFINE_EVENT(DimensionDestroyedEvent)
            std::string dimensionName;
            uint32_t dimensionId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(PortalCreatedEvent)
            uint32_t portalId;
            std::string sourceDimension;
            std::string targetDimension;
            glm::vec3 position;
        END_DEFINE_EVENT()

        DEFINE_EVENT(PortalUsedEvent)
            uint32_t entityId;
            uint32_t portalId;
        END_DEFINE_EVENT()

        // Time Events
        DEFINE_EVENT(EraChangedEvent)
            std::string oldEra;
            std::string newEra;
            uint32_t eraId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(TimelineShiftedEvent)
            std::string shiftType;
            int32_t timeDelta;
            std::string cause;
        END_DEFINE_EVENT()

        DEFINE_EVENT(TimeAcceleratedEvent)
            float timeScale;
            std::string cause;
            uint32_t duration;
        END_DEFINE_EVENT()

        // World Disaster Events
        DEFINE_EVENT(DisasterStartedEvent)
            std::string disasterType;
            std::string severity;
            glm::vec3 epicenter;
            float radius;
        END_DEFINE_EVENT()

        DEFINE_EVENT(DisasterProgressEvent)
            std::string disasterType;
            float progress;
            uint32_t affectedEntities;
        END_DEFINE_EVENT()

        DEFINE_EVENT(DisasterEndedEvent)
            std::string disasterType;
            std::string outcome;
            uint32_t totalDamage;
        END_DEFINE_EVENT()

        DEFINE_EVENT(EarthquakeEvent)
            glm::vec3 epicenter;
            float magnitude;
            uint32_t duration;
        END_DEFINE_EVENT()

        DEFINE_EVENT(VolcanoEruptionEvent)
            glm::vec3 position;
            uint32_t lavaFlow;
            uint32_t ashCloudRadius;
        END_DEFINE_EVENT()

        DEFINE_EVENT(MeteorImpactEvent)
            glm::vec3 impactPosition;
            float meteorSize;
            float impactForce;
        END_DEFINE_EVENT()

        DEFINE_EVENT(FloodEvent)
            glm::vec3 center;
            float waterLevel;
            float radius;
        END_DEFINE_EVENT()

        DEFINE_EVENT(StormEvent)
            std::string stormType;
            glm::vec3 center;
            float radius;
            float windSpeed;
        END_DEFINE_EVENT()

        // Phenomenon Events
        DEFINE_EVENT(AuroraEvent)
            std::string auroraType;
            float intensity;
            uint32_t duration;
        END_DEFINE_EVENT()

        DEFINE_EVENT(SolarEclipseEvent)
            float duration;
            float darknessLevel;
        END_DEFINE_EVENT()

        DEFINE_EVENT(MeteorShowerEvent)
            std::string showerName;
            uint32_t meteorCount;
            uint32_t duration;
        END_DEFINE_EVENT()

        DEFINE_EVENT(BloodMoonEvent)
            uint32_t duration;
            float intensity;
        END_DEFINE_EVENT()

        // Economy Events
        DEFINE_EVENT(TradeCompletedEvent)
            uint32_t sellerId;
            uint32_t buyerId;
            uint32_t itemId;
            uint32_t quantity;
            uint32_t price;
        END_DEFINE_EVENT()

        DEFINE_EVENT(MarketPriceChangedEvent)
            uint32_t itemId;
            uint32_t oldPrice;
            uint32_t newPrice;
            std::string reason;
        END_DEFINE_EVENT()

        DEFINE_EVENT(AuctionStartedEvent)
            uint32_t auctionId;
            uint32_t itemId;
            uint32_t startingPrice;
            uint32_t sellerId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(AuctionEndedEvent)
            uint32_t auctionId;
            uint32_t winnerId;
            uint32_t finalPrice;
        END_DEFINE_EVENT()

        DEFINE_EVENT(BankTransactionEvent)
            uint32_t accountId;
            int32_t amount;
            std::string transactionType;
            std::string description;
        END_DEFINE_EVENT()

        // Guild Events
        DEFINE_EVENT(GuildCreatedEvent)
            uint32_t guildId;
            std::string guildName;
            uint32_t founderId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(GuildMemberJoinedEvent)
            uint32_t guildId;
            uint32_t memberId;
            std::string joinReason;
        END_DEFINE_EVENT()

        DEFINE_EVENT(GuildMemberLeftEvent)
            uint32_t guildId;
            uint32_t memberId;
            std::string leaveReason;
        END_DEFINE_EVENT()

        DEFINE_EVENT(GuildRankChangedEvent)
            uint32_t guildId;
            uint32_t memberId;
            std::string oldRank;
            std::string newRank;
        END_DEFINE_EVENT()

        DEFINE_EVENT(GuildWarDeclaredEvent)
            uint32_t guildA;
            uint32_t guildB;
            std::string reason;
        END_DEFINE_EVENT()

        DEFINE_EVENT(GuildWarEndedEvent)
            uint32_t winnerGuild;
            uint32_t loserGuild;
            std::string outcome;
        END_DEFINE_EVENT()

        // Social Events
        DEFINE_EVENT(FriendRequestSentEvent)
            uint32_t senderId;
            uint32_t receiverId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(FriendRequestAcceptedEvent)
            uint32_t senderId;
            uint32_t receiverId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(FriendRemovedEvent)
            uint32_t removerId;
            uint32_t removedId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(PlayerReportedEvent)
            uint32_t reporterId;
            uint32_t reportedId;
            std::string reason;
        END_DEFINE_EVENT()

        DEFINE_EVENT(PlayerMutedEvent)
            uint32_t muterId;
            uint32_t mutedId;
            uint32_t duration;
        END_DEFINE_EVENT()

        // Technology Events
        DEFINE_EVENT(ResearchCompletedEvent)
            uint32_t playerId;
            std::string researchName;
            uint32_t researchId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(TechnologyUnlockedEvent)
            uint32_t playerId;
            std::string technologyName;
            uint32_t technologyId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(FactoryProductionEvent)
            uint32_t factoryId;
            uint32_t itemId;
            uint32_t quantity;
            float productionTime;
        END_DEFINE_EVENT()

        DEFINE_EVENT(RobotTaskCompletedEvent)
            uint32_t robotId;
            std::string taskType;
            bool success;
        END_DEFINE_EVENT()

        DEFINE_EVENT(AutomationErrorEvent)
            uint32_t automationId;
            std::string errorMessage;
            std::string component;
        END_DEFINE_EVENT()

        // Environmental Events
        DEFINE_EVENT(PollutionLevelChangedEvent)
            glm::vec3 position;
            float oldLevel;
            float newLevel;
            std::string pollutionType;
        END_DEFINE_EVENT()

        DEFINE_EVENT(RecyclingCompletedEvent)
            uint32_t playerId;
            uint32_t itemId;
            uint32_t quantity;
            uint32_t recycledMaterials;
        END_DEFINE_EVENT()

        DEFINE_EVENT(EcosystemChangedEvent)
            std::string ecosystemType;
            glm::vec3 position;
            float oldHealth;
            float newHealth;
        END_DEFINE_EVENT()

        DEFINE_EVENT(ClimateChangedEvent)
            std::string oldClimate;
            std::string newClimate;
            float temperatureChange;
        END_DEFINE_EVENT()

        // Energy Events
        DEFINE_EVENT(PowerGeneratedEvent)
            uint32_t generatorId;
            float energyGenerated;
            std::string energyType;
        END_DEFINE_EVENT()

        DEFINE_EVENT(PowerConsumedEvent)
            uint32_t consumerId;
            float energyConsumed;
            std::string consumerType;
        END_DEFINE_EVENT()

        DEFINE_EVENT(PowerGridOverloadEvent)
            std::string gridName;
            float overloadPercentage;
            uint32_t affectedConsumers;
        END_DEFINE_EVENT()

        DEFINE_EVENT(BatteryChargedEvent)
            uint32_t batteryId;
            float chargeAdded;
            float newChargeLevel;
        END_DEFINE_EVENT()

        // VR/AR Events
        DEFINE_EVENT(VRControllerConnectedEvent)
            uint32_t controllerId;
            std::string controllerType;
        END_DEFINE_EVENT()

        DEFINE_EVENT(VRControllerDisconnectedEvent)
            uint32_t controllerId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(VRGestureRecognizedEvent)
            uint32_t controllerId;
            std::string gestureName;
            float confidence;
        END_DEFINE_EVENT()

        DEFINE_EVENT(ARObjectRecognizedEvent)
            std::string objectType;
            glm::vec3 position;
            glm::vec3 rotation;
            float confidence;
        END_DEFINE_EVENT()

        DEFINE_EVENT(AROverlayUpdatedEvent)
            std::string overlayType;
            glm::vec3 position;
            std::string content;
        END_DEFINE_EVENT()

        // Accessibility Events
        DEFINE_EVENT(AccessibilityOptionChangedEvent)
            std::string optionName;
            std::string oldValue;
            std::string newValue;
        END_DEFINE_EVENT()

        DEFINE_EVENT(ScreenReaderOutputEvent)
            std::string text;
            std::string priority;
        END_DEFINE_EVENT()

        DEFINE_EVENT(ColorBlindFilterChangedEvent)
            std::string filterType;
            float intensity;
        END_DEFINE_EVENT()

        // Performance Events
        DEFINE_EVENT(FrameTimeChangedEvent)
            float oldFrameTime;
            float newFrameTime;
            bool isPerformanceIssue;
        END_DEFINE_EVENT()

        DEFINE_EVENT(MemoryUsageChangedEvent)
            uint64_t oldUsage;
            uint64_t newUsage;
            bool isCritical;
        END_DEFINE_EVENT()

        DEFINE_EVENT(GPUUsageChangedEvent)
            float oldUsage;
            float newUsage;
            bool isCritical;
        END_DEFINE_EVENT()

        // Debug Events
        DEFINE_EVENT(DebugCommandExecutedEvent)
            std::string command;
            std::string result;
            uint32_t executorId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(DebugBreakpointHitEvent)
            std::string file;
            uint32_t line;
            std::string condition;
        END_DEFINE_EVENT()

        DEFINE_EVENT(DebugVariableChangedEvent)
            std::string variableName;
            std::string oldValue;
            std::string newValue;
        END_DEFINE_EVENT()

        // Analytics Events
        DEFINE_EVENT(AnalyticsEvent)
            std::string eventName;
            std::unordered_map<std::string, std::string> parameters;
            uint32_t userId;
        END_DEFINE_EVENT()

        DEFINE_EVENT(PlayerBehaviorEvent)
            uint32_t playerId;
            std::string behaviorType;
            std::string details;
            uint32_t duration;
        END_DEFINE_EVENT()

        DEFINE_EVENT(GameMetricsEvent)
            std::string metricName;
            float value;
            std::string unit;
        END_DEFINE_EVENT()

    } // namespace Events

} // namespace VoxelCraft

#endif // VOXELCRAFT_EVENTS_EVENT_SYSTEM_HPP
