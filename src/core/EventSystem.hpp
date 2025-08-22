/**
 * @file EventSystem.hpp
 * @brief VoxelCraft Engine Advanced Event System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file provides a comprehensive, high-performance event system
 * for the VoxelCraft engine with support for synchronous and asynchronous
 * event processing, filtering, prioritization, and performance monitoring.
 */

#ifndef VOXELCRAFT_CORE_EVENT_SYSTEM_HPP
#define VOXELCRAFT_CORE_EVENT_SYSTEM_HPP

#include <string>
#include <memory>
#include <vector>
#include <queue>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <thread>
#include <future>
#include <optional>
#include <type_traits>
#include <any>

namespace VoxelCraft {

    /**
     * @typedef EventType
     * @brief Type identifier for events
     */
    using EventType = std::string;

    /**
     * @typedef EventId
     * @brief Unique identifier for events
     */
    using EventId = uint64_t;

    /**
     * @typedef ListenerId
     * @brief Unique identifier for event listeners
     */
    using ListenerId = uint64_t;

    /**
     * @enum EventPriority
     * @brief Event processing priority levels
     */
    enum class EventPriority {
        Lowest = 0,     ///< Lowest priority (processed last)
        Low,            ///< Low priority
        Normal,         ///< Normal priority (default)
        High,           ///< High priority
        Highest,        ///< Highest priority (processed first)
        Critical        ///< Critical priority (immediate processing)
    };

    /**
     * @struct Event
     * @brief Base event structure
     */
    struct Event {
        EventId id;                                    ///< Unique event ID
        EventType type;                                ///< Event type
        std::any data;                                 ///< Event data (type-safe)
        double timestamp;                              ///< Event timestamp
        std::string source;                            ///< Event source
        EventPriority priority;                        ///< Event priority
        bool isAsync;                                  ///< Asynchronous processing flag
        bool isCancellable;                            ///< Can be cancelled flag
        mutable bool cancelled;                        ///< Cancellation flag

        /**
         * @brief Constructor
         * @param type Event type
         * @param data Event data
         * @param source Event source
         * @param priority Event priority
         * @param isAsync Asynchronous processing
         * @param isCancellable Can be cancelled
         */
        Event(EventType type,
              std::any data = {},
              std::string source = "unknown",
              EventPriority priority = EventPriority::Normal,
              bool isAsync = false,
              bool isCancellable = false)
            : id(GenerateId())
            , type(std::move(type))
            , data(std::move(data))
            , timestamp(GetCurrentTime())
            , source(std::move(source))
            , priority(priority)
            , isAsync(isAsync)
            , isCancellable(isCancellable)
            , cancelled(false)
        {}

        /**
         * @brief Cancel the event
         * @return true if cancelled, false if not cancellable
         */
        bool Cancel() {
            if (isCancellable && !cancelled) {
                cancelled = true;
                return true;
            }
            return false;
        }

        /**
         * @brief Check if event is cancelled
         * @return true if cancelled
         */
        bool IsCancelled() const { return cancelled; }

    private:
        /**
         * @brief Generate unique event ID
         * @return Unique ID
         */
        static EventId GenerateId() {
            static std::atomic<EventId> nextId{1};
            return nextId.fetch_add(1, std::memory_order_relaxed);
        }

        /**
         * @brief Get current timestamp
         * @return Current time in seconds
         */
        static double GetCurrentTime() {
            return std::chrono::duration_cast<std::chrono::duration<double>>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();
        }
    };

    /**
     * @struct EventStatistics
     * @brief Event system performance statistics
     */
    struct EventStatistics {
        uint64_t totalEvents;                          ///< Total events processed
        uint64_t eventsPerSecond;                      ///< Current events per second
        uint64_t queuedEvents;                         ///< Events in queue
        uint64_t activeListeners;                      ///< Number of active listeners
        double averageProcessingTime;                  ///< Average event processing time (μs)
        double peakProcessingTime;                     ///< Peak event processing time (μs)
        uint64_t droppedEvents;                        ///< Events dropped due to queue overflow
        size_t memoryUsage;                            ///< Current memory usage
    };

    /**
     * @class EventListener
     * @brief Abstract base class for event listeners
     */
    class EventListener {
    public:
        /**
         * @brief Constructor
         */
        EventListener() = default;

        /**
         * @brief Destructor
         */
        virtual ~EventListener() = default;

        /**
         * @brief Deleted copy constructor
         */
        EventListener(const EventListener&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        EventListener& operator=(const EventListener&) = delete;

        /**
         * @brief Handle event
         * @param event Event to handle
         * @return true if event was handled, false otherwise
         */
        virtual bool OnEvent(const Event& event) = 0;

        /**
         * @brief Get listener name
         * @return Listener name
         */
        virtual std::string GetName() const = 0;

        /**
         * @brief Get listener priority
         * @return Listener priority
         */
        virtual EventPriority GetPriority() const { return EventPriority::Normal; }

        /**
         * @brief Check if listener handles specific event type
         * @param eventType Event type to check
         * @return true if handles, false otherwise
         */
        virtual bool HandlesEventType(const EventType& eventType) const = 0;

        /**
         * @brief Check if listener is enabled
         * @return true if enabled, false otherwise
         */
        virtual bool IsEnabled() const { return true; }

        /**
         * @brief Get unique listener ID
         * @return Listener ID
         */
        ListenerId GetId() const { return m_id; }

    private:
        ListenerId m_id;                               ///< Unique listener ID
        static std::atomic<ListenerId> s_nextId;       ///< Next ID counter
    };

    /**
     * @class TypedEventListener
     * @brief Template class for type-safe event listeners
     */
    template<typename T>
    class TypedEventListener : public EventListener {
    public:
        /**
         * @brief Constructor
         * @param name Listener name
         * @param priority Listener priority
         */
        TypedEventListener(const std::string& name, EventPriority priority = EventPriority::Normal)
            : m_name(name), m_priority(priority) {}

        /**
         * @brief Destructor
         */
        ~TypedEventListener() override = default;

        /**
         * @brief Handle typed event
         * @param event Typed event to handle
         * @return true if event was handled, false otherwise
         */
        virtual bool OnTypedEvent(const T& event) = 0;

        /**
         * @brief Handle event (implementation)
         * @param event Event to handle
         * @return true if event was handled, false otherwise
         */
        bool OnEvent(const Event& event) override {
            if (event.type != T::GetEventType()) {
                return false;
            }

            try {
                const T& typedEvent = std::any_cast<const T&>(event.data);
                return OnTypedEvent(typedEvent);
            } catch (const std::bad_any_cast&) {
                return false;
            }
        }

        /**
         * @brief Get listener name
         * @return Listener name
         */
        std::string GetName() const override { return m_name; }

        /**
         * @brief Get listener priority
         * @return Listener priority
         */
        EventPriority GetPriority() const override { return m_priority; }

        /**
         * @brief Check if listener handles specific event type
         * @param eventType Event type to check
         * @return true if handles, false otherwise
         */
        bool HandlesEventType(const EventType& eventType) const override {
            return eventType == T::GetEventType();
        }

    private:
        std::string m_name;                            ///< Listener name
        EventPriority m_priority;                      ///< Listener priority
    };

    /**
     * @typedef EventCallback
     * @brief Function pointer type for event callbacks
     */
    using EventCallback = std::function<bool(const Event&)>;

    /**
     * @class CallbackEventListener
     * @brief Event listener that uses callback functions
     */
    class CallbackEventListener : public EventListener {
    public:
        /**
         * @brief Constructor
         * @param name Listener name
         * @param callback Event callback function
         * @param eventTypes Event types to handle
         * @param priority Listener priority
         */
        CallbackEventListener(const std::string& name,
                            EventCallback callback,
                            const std::vector<EventType>& eventTypes = {},
                            EventPriority priority = EventPriority::Normal);

        /**
         * @brief Destructor
         */
        ~CallbackEventListener() override = default;

        /**
         * @brief Handle event
         * @param event Event to handle
         * @return true if event was handled, false otherwise
         */
        bool OnEvent(const Event& event) override;

        /**
         * @brief Get listener name
         * @return Listener name
         */
        std::string GetName() const override { return m_name; }

        /**
         * @brief Get listener priority
         * @return Listener priority
         */
        EventPriority GetPriority() const override { return m_priority; }

        /**
         * @brief Check if listener handles specific event type
         * @param eventType Event type to check
         * @return true if handles, false otherwise
         */
        bool HandlesEventType(const EventType& eventType) const override;

    private:
        std::string m_name;                            ///< Listener name
        EventCallback m_callback;                      ///< Event callback
        std::vector<EventType> m_eventTypes;           ///< Handled event types
        EventPriority m_priority;                      ///< Listener priority
    };

    /**
     * @class EventSystem
     * @brief Main event system manager
     */
    class EventSystem {
    public:
        /**
         * @brief Constructor
         * @param maxQueueSize Maximum event queue size
         * @param numWorkerThreads Number of worker threads for async events
         */
        explicit EventSystem(size_t maxQueueSize = 10000, uint32_t numWorkerThreads = 2);

        /**
         * @brief Destructor
         */
        ~EventSystem();

        /**
         * @brief Deleted copy constructor
         */
        EventSystem(const EventSystem&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        EventSystem& operator=(const EventSystem&) = delete;

        // Event dispatching

        /**
         * @brief Queue event for processing
         * @param event Event to queue
         * @return true if queued, false if queue full
         */
        bool QueueEvent(Event&& event);

        /**
         * @brief Queue event for immediate processing
         * @param event Event to process immediately
         * @return true if processed, false otherwise
         */
        bool ProcessEventImmediately(const Event& event);

        /**
         * @brief Dispatch pending events
         * @param maxEvents Maximum events to process (0 = all)
         * @return Number of events processed
         */
        size_t DispatchEvents(size_t maxEvents = 0);

        /**
         * @brief Wait for events and dispatch them
         * @param timeoutMs Timeout in milliseconds (0 = infinite)
         * @param maxEvents Maximum events to process
         * @return Number of events processed
         */
        size_t WaitAndDispatchEvents(uint32_t timeoutMs = 0, size_t maxEvents = 0);

        // Listener management

        /**
         * @brief Add event listener
         * @param listener Listener to add
         * @return Listener ID
         */
        ListenerId AddListener(std::unique_ptr<EventListener> listener);

        /**
         * @brief Add event listener with callback
         * @param name Listener name
         * @param callback Event callback
         * @param eventTypes Event types to handle
         * @param priority Listener priority
         * @return Listener ID
         */
        ListenerId AddListener(const std::string& name,
                             EventCallback callback,
                             const std::vector<EventType>& eventTypes = {},
                             EventPriority priority = EventPriority::Normal);

        /**
         * @brief Remove event listener
         * @param listenerId Listener ID to remove
         * @return true if removed, false if not found
         */
        bool RemoveListener(ListenerId listenerId);

        /**
         * @brief Enable/disable event listener
         * @param listenerId Listener ID
         * @param enabled Enable state
         * @return true if successful, false if not found
         */
        bool SetListenerEnabled(ListenerId listenerId, bool enabled);

        /**
         * @brief Get event listener by ID
         * @param listenerId Listener ID
         * @return Listener pointer or nullptr if not found
         */
        EventListener* GetListener(ListenerId listenerId);

        // Event filtering and monitoring

        /**
         * @brief Add event type filter
         * @param eventType Event type to filter
         * @param enabled Enable filter
         */
        void SetEventTypeFilter(const EventType& eventType, bool enabled);

        /**
         * @brief Add event source filter
         * @param source Event source to filter
         * @param enabled Enable filter
         */
        void SetEventSourceFilter(const std::string& source, bool enabled);

        /**
         * @brief Clear all filters
         */
        void ClearFilters();

        // Statistics and monitoring

        /**
         * @brief Get event system statistics
         * @return System statistics
         */
        EventStatistics GetStatistics() const;

        /**
         * @brief Reset statistics
         */
        void ResetStatistics();

        /**
         * @brief Get queue size
         * @return Number of events in queue
         */
        size_t GetQueueSize() const;

        /**
         * @brief Check if queue is empty
         * @return true if empty, false otherwise
         */
        bool IsQueueEmpty() const;

        /**
         * @brief Check if queue is full
         * @return true if full, false otherwise
         */
        bool IsQueueFull() const;

        // System control

        /**
         * @brief Start event processing
         */
        void Start();

        /**
         * @brief Stop event processing
         */
        void Stop();

        /**
         * @brief Pause event processing
         */
        void Pause();

        /**
         * @brief Resume event processing
         */
        void Resume();

        /**
         * @brief Check if system is running
         * @return true if running, false otherwise
         */
        bool IsRunning() const { return m_running; }

        /**
         * @brief Check if system is paused
         * @return true if paused, false otherwise
         */
        bool IsPaused() const { return m_paused; }

    private:
        /**
         * @brief Event processing loop for worker threads
         */
        void EventProcessingLoop();

        /**
         * @brief Process single event
         * @param event Event to process
         */
        void ProcessEvent(const Event& event);

        /**
         * @brief Sort listeners by priority
         * @param listeners List of listeners to sort
         */
        void SortListenersByPriority(std::vector<EventListener*>& listeners);

        /**
         * @brief Check if event passes filters
         * @param event Event to check
         * @return true if passes, false otherwise
         */
        bool PassesFilters(const Event& event);

        // Event queue and processing
        std::queue<Event> m_eventQueue;                    ///< Main event queue
        mutable std::mutex m_queueMutex;                   ///< Queue synchronization
        std::condition_variable m_queueCondition;          ///< Queue condition variable
        size_t m_maxQueueSize;                             ///< Maximum queue size

        // Listeners
        std::unordered_map<ListenerId, std::unique_ptr<EventListener>> m_listeners;
        mutable std::mutex m_listenersMutex;               ///< Listeners synchronization
        static std::atomic<ListenerId> s_nextListenerId;   ///< Next listener ID

        // Worker threads
        std::vector<std::thread> m_workerThreads;          ///< Worker threads
        uint32_t m_numWorkerThreads;                       ///< Number of worker threads
        std::atomic<bool> m_running;                       ///< System running flag
        std::atomic<bool> m_paused;                        ///< System paused flag

        // Filtering
        std::unordered_map<EventType, bool> m_eventTypeFilters;     ///< Event type filters
        std::unordered_map<std::string, bool> m_eventSourceFilters; ///< Event source filters
        mutable std::mutex m_filterMutex;                  ///< Filter synchronization

        // Statistics
        mutable std::atomic<uint64_t> m_totalEvents;       ///< Total events processed
        mutable std::atomic<uint64_t> m_queuedEvents;      ///< Events in queue
        mutable std::atomic<uint64_t> m_droppedEvents;     ///< Dropped events
        mutable std::chrono::steady_clock::time_point m_startTime; ///< Start time
    };

    // Event creation helpers

    /**
     * @brief Create event with type and data
     * @tparam T Event data type
     * @param type Event type
     * @param data Event data
     * @param source Event source
     * @param priority Event priority
     * @param isAsync Asynchronous processing
     * @param isCancellable Can be cancelled
     * @return Created event
     */
    template<typename T>
    Event CreateEvent(const EventType& type,
                     const T& data,
                     const std::string& source = "unknown",
                     EventPriority priority = EventPriority::Normal,
                     bool isAsync = false,
                     bool isCancellable = false) {
        return Event(type, data, source, priority, isAsync, isCancellable);
    }

    /**
     * @brief Create typed event
     * @tparam T Event type (must have static GetEventType() method)
     * @param data Event data
     * @param source Event source
     * @param priority Event priority
     * @param isAsync Asynchronous processing
     * @param isCancellable Can be cancelled
     * @return Created event
     */
    template<typename T>
    Event CreateTypedEvent(const typename T::DataType& data,
                          const std::string& source = "unknown",
                          EventPriority priority = EventPriority::Normal,
                          bool isAsync = false,
                          bool isCancellable = false) {
        return Event(T::GetEventType(), T{data}, source, priority, isAsync, isCancellable);
    }

    // Common event types

    /**
     * @struct WindowResizeEvent
     * @brief Window resize event data
     */
    struct WindowResizeEvent {
        static EventType GetEventType() { return "WindowResize"; }
        struct DataType {
            int width;
            int height;
        } data;
    };

    /**
     * @struct KeyEvent
     * @brief Keyboard event data
     */
    struct KeyEvent {
        static EventType GetEventType() { return "KeyEvent"; }
        struct DataType {
            int key;
            int scancode;
            int action;
            int mods;
        } data;
    };

    /**
     * @struct MouseEvent
     * @brief Mouse event data
     */
    struct MouseEvent {
        static EventType GetEventType() { return "MouseEvent"; }
        struct DataType {
            int button;
            int action;
            int mods;
            double x;
            double y;
        } data;
    };

    /**
     * @struct ApplicationEvent
     * @brief Application lifecycle event data
     */
    struct ApplicationEvent {
        static EventType GetEventType() { return "ApplicationEvent"; }
        enum class Type { Start, Stop, Pause, Resume, FocusGained, FocusLost };
        struct DataType {
            Type type;
        } data;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_CORE_EVENT_SYSTEM_HPP
