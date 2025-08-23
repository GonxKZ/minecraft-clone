/**
 * @file EventSystem.cpp
 * @brief VoxelCraft Advanced Event System Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "EventSystem.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <any>
#include <functional>
#include <memory>
#include <filesystem>

#include "../utils/Random.hpp"
#include "../logging/Logger.hpp"

namespace VoxelCraft {

    // Static member initialization
    std::atomic<HandlerId> EventDispatcher::s_nextHandlerId{1};
    std::atomic<EventId> EventDispatcher::s_nextEventId{1};

    // EventBase implementation
    EventBase::EventBase(EventTypeId typeId, const std::string& typeName)
        : m_id(EventDispatcher::s_nextEventId++),
          m_typeId(typeId),
          m_typeName(typeName),
          m_timestamp(std::chrono::system_clock::now()),
          m_priority(EventPriority::NORMAL),
          m_delivery(EventDelivery::SYNCHRONOUS),
          m_consumed(false),
          m_cancelled(false) {
    }

    // EventHandler implementation
    EventHandler::EventHandler(const std::string& name)
        : m_id(EventDispatcher::s_nextHandlerId++),
          m_name(name),
          m_enabled(true),
          m_priority(EventPriority::NORMAL) {
    }

    // EventFilter implementations
    EventFilter::EventFilter(const std::string& name)
        : m_name(name), m_enabled(true) {
    }

    TypeEventFilter::TypeEventFilter(EventFilterMode mode, const std::string& name)
        : EventFilter(name), m_mode(mode) {
    }

    bool TypeEventFilter::ShouldFilter(const EventBase& event) {
        if (!m_enabled) {
            return false;
        }

        bool found = m_types.find(event.GetTypeId()) != m_types.end();

        switch (m_mode) {
            case EventFilterMode::WHITELIST:
                return !found;  // Filter out if not in list
            case EventFilterMode::BLACKLIST:
                return found;   // Filter out if in list
            default:
                return false;
        }
    }

    PriorityEventFilter::PriorityEventFilter(EventPriority minPriority, EventPriority maxPriority,
                                           const std::string& name)
        : EventFilter(name), m_minPriority(minPriority), m_maxPriority(maxPriority) {
    }

    bool PriorityEventFilter::ShouldFilter(const EventBase& event) {
        if (!m_enabled) {
            return false;
        }

        EventPriority eventPriority = event.GetPriority();
        return static_cast<int>(eventPriority) < static_cast<int>(m_minPriority) ||
               static_cast<int>(eventPriority) > static_cast<int>(m_maxPriority);
    }

    CustomEventFilter::CustomEventFilter(FilterPredicate predicate, const std::string& name)
        : EventFilter(name), m_predicate(predicate) {
    }

    bool CustomEventFilter::ShouldFilter(const EventBase& event) {
        if (!m_enabled || !m_predicate) {
            return false;
        }

        return m_predicate(event);
    }

    // EventQueue implementation
    EventQueue::EventQueue(size_t maxSize)
        : m_maxSize(maxSize) {
    }

    EventQueue::~EventQueue() = default;

    bool EventQueue::Push(std::unique_ptr<EventBase> event) {
        std::unique_lock<std::mutex> lock(m_mutex);

        if (m_queue.size() >= m_maxSize) {
            return false;  // Queue is full
        }

        m_queue.push(std::move(event));
        m_condition.notify_one();
        return true;
    }

    std::unique_ptr<EventBase> EventQueue::Pop() {
        std::unique_lock<std::mutex> lock(m_mutex);

        if (m_queue.empty()) {
            return nullptr;
        }

        auto event = std::move(m_queue.front());
        m_queue.pop();
        return event;
    }

    bool EventQueue::IsEmpty() const {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    size_t EventQueue::Size() const {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

    void EventQueue::Clear() {
        std::unique_lock<std::mutex> lock(m_mutex);

        std::queue<std::unique_ptr<EventBase>> empty;
        std::swap(m_queue, empty);
    }

    // EventProfiler implementation
    EventProfiler::EventProfiler() = default;

    void EventProfiler::StartProfiling(EventTypeId typeId) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        m_activeProfiles[typeId] = std::chrono::steady_clock::now();
    }

    void EventProfiler::StopProfiling(EventTypeId typeId) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);

        auto it = m_activeProfiles.find(typeId);
        if (it != m_activeProfiles.end()) {
            m_activeProfiles.erase(it);
        }
    }

    void EventProfiler::RecordEventDispatch(const EventBase& event, uint64_t processingTimeNs) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);

        EventStats& stats = m_eventStats[event.GetTypeId()];
        stats.totalDispatches++;
        stats.totalProcessingTimeNs += processingTimeNs;
        stats.averageProcessingTimeNs = stats.totalProcessingTimeNs / stats.totalDispatches;
        stats.maxProcessingTimeNs = std::max(stats.maxProcessingTimeNs, processingTimeNs);
        stats.minProcessingTimeNs = stats.minProcessingTimeNs == 0 ?
            processingTimeNs : std::min(stats.minProcessingTimeNs, processingTimeNs);
    }

    void EventProfiler::RecordHandlerExecution(HandlerId handlerId, EventTypeId eventType,
                                             uint64_t executionTimeNs) {
        // This could be extended to track per-handler statistics
        (void)handlerId;
        (void)eventType;
        (void)executionTimeNs;
    }

    EventProfiler::EventStats EventProfiler::GetEventStats(EventTypeId typeId) const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);

        auto it = m_eventStats.find(typeId);
        return it != m_eventStats.end() ? it->second : EventStats{};
    }

    std::unordered_map<EventTypeId, EventProfiler::EventStats> EventProfiler::GetAllEventStats() const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        return m_eventStats;
    }

    void EventProfiler::Reset() {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        m_eventStats.clear();
        m_activeProfiles.clear();
    }

    // EventDispatcher implementation
    EventDispatcher& EventDispatcher::GetInstance() {
        static EventDispatcher instance;
        return instance;
    }

    EventDispatcher::EventDispatcher()
        : m_asyncEnabled(true),
          m_running(true),
          m_workerThreadCount(4),
          m_profilingEnabled(false),
          m_processedEventCount(0),
          m_queuedEventCount(0),
          m_filteredEventCount(0) {
    }

    EventDispatcher::~EventDispatcher() {
        Shutdown();
    }

    bool EventDispatcher::Initialize() {
        if (!System::Initialize()) {
            return false;
        }

        Logger::GetInstance().Info("EventDispatcher initializing", "EventSystem");

        // Initialize worker threads
        StartWorkerThreads();

        // Register common event types
        RegisterCommonEventTypes();

        Logger::GetInstance().Info("EventDispatcher initialized successfully", "EventSystem");
        return true;
    }

    void EventDispatcher::Update(float deltaTime) {
        System::Update(deltaTime);

        // Process queued events
        ProcessQueuedEvents();

        // Update statistics
        UpdateStatistics();
    }

    void EventDispatcher::Shutdown() {
        if (!System::Shutdown()) {
            return;
        }

        Logger::GetInstance().Info("EventDispatcher shutting down", "EventSystem");

        // Stop worker threads
        StopWorkerThreads();

        // Clear all handlers
        ClearAllHandlers();

        // Clear event queue
        m_eventQueue.Clear();

        // Clear filters
        {
            std::unique_lock<std::shared_mutex> lock(m_filtersMutex);
            m_globalFilters.clear();
        }

        // Clear types
        {
            std::unique_lock<std::shared_mutex> lock(m_typesMutex);
            m_registeredTypes.clear();
        }

        Logger::GetInstance().Info("EventDispatcher shutdown complete", "EventSystem");
    }

    void EventDispatcher::Dispatch(std::unique_ptr<EventBase> event) {
        if (!event) {
            return;
        }

        if (m_asyncEnabled && event->GetDelivery() == EventDelivery::ASYNCHRONOUS) {
            QueueEvent(std::move(event));
        } else {
            ProcessEvent(std::move(event));
        }
    }

    void EventDispatcher::DispatchImmediate(std::unique_ptr<EventBase> event) {
        if (!event) {
            return;
        }

        ProcessImmediateEvent(std::move(event));
    }

    void EventDispatcher::QueueEvent(std::unique_ptr<EventBase> event) {
        if (!event) {
            return;
        }

        if (m_eventQueue.Push(std::move(event))) {
            m_queuedEventCount++;
        } else {
            Logger::GetInstance().Warning("Event queue is full, dropping event", "EventSystem");
        }
    }

    HandlerId EventDispatcher::RegisterHandler(std::shared_ptr<EventHandler> handler) {
        if (!handler) {
            return 0;
        }

        std::unique_lock<std::shared_mutex> lock(m_handlersMutex);

        HandlerId id = handler->GetId();
        m_handlers[id] = handler;
        m_handlerNames[handler->GetName()] = id;

        Logger::GetInstance().Debug("Registered event handler: " + handler->GetName(), "EventSystem");
        return id;
    }

    bool EventDispatcher::UnregisterHandler(HandlerId handlerId) {
        std::unique_lock<std::shared_mutex> lock(m_handlersMutex);

        auto it = m_handlers.find(handlerId);
        if (it == m_handlers.end()) {
            return false;
        }

        std::string name = it->second->GetName();
        m_handlers.erase(it);

        // Remove from name map
        for (auto nameIt = m_handlerNames.begin(); nameIt != m_handlerNames.end(); ++nameIt) {
            if (nameIt->second == handlerId) {
                m_handlerNames.erase(nameIt);
                break;
            }
        }

        Logger::GetInstance().Debug("Unregistered event handler: " + name, "EventSystem");
        return true;
    }

    bool EventDispatcher::UnregisterHandler(const std::string& name) {
        std::unique_lock<std::shared_mutex> lock(m_handlersMutex);

        auto nameIt = m_handlerNames.find(name);
        if (nameIt == m_handlerNames.end()) {
            return false;
        }

        HandlerId id = nameIt->second;
        auto handlerIt = m_handlers.find(id);

        if (handlerIt != m_handlers.end()) {
            m_handlers.erase(handlerIt);
        }

        m_handlerNames.erase(nameIt);

        Logger::GetInstance().Debug("Unregistered event handler: " + name, "EventSystem");
        return true;
    }

    std::shared_ptr<EventHandler> EventDispatcher::GetHandler(HandlerId handlerId) const {
        std::shared_lock<std::shared_mutex> lock(m_handlersMutex);

        auto it = m_handlers.find(handlerId);
        return it != m_handlers.end() ? it->second : nullptr;
    }

    std::shared_ptr<EventHandler> EventDispatcher::GetHandler(const std::string& name) const {
        std::shared_lock<std::shared_mutex> lock(m_handlersMutex);

        auto nameIt = m_handlerNames.find(name);
        if (nameIt == m_handlerNames.end()) {
            return nullptr;
        }

        auto handlerIt = m_handlers.find(nameIt->second);
        return handlerIt != m_handlers.end() ? handlerIt->second : nullptr;
    }

    void EventDispatcher::AddGlobalFilter(std::shared_ptr<EventFilter> filter) {
        if (!filter) {
            return;
        }

        std::unique_lock<std::shared_mutex> lock(m_filtersMutex);
        m_globalFilters.push_back(filter);

        Logger::GetInstance().Debug("Added global filter: " + filter->GetName(), "EventSystem");
    }

    void EventDispatcher::RemoveGlobalFilter(const std::string& name) {
        std::unique_lock<std::shared_mutex> lock(m_filtersMutex);

        auto it = std::remove_if(m_globalFilters.begin(), m_globalFilters.end(),
            [&name](const std::shared_ptr<EventFilter>& filter) {
                return filter->GetName() == name;
            });

        if (it != m_globalFilters.end()) {
            m_globalFilters.erase(it, m_globalFilters.end());
            Logger::GetInstance().Debug("Removed global filter: " + name, "EventSystem");
        }
    }

    std::shared_ptr<EventFilter> EventDispatcher::GetGlobalFilter(const std::string& name) const {
        std::shared_lock<std::shared_mutex> lock(m_filtersMutex);

        auto it = std::find_if(m_globalFilters.begin(), m_globalFilters.end(),
            [&name](const std::shared_ptr<EventFilter>& filter) {
                return filter->GetName() == name;
            });

        return it != m_globalFilters.end() ? *it : nullptr;
    }

    void EventDispatcher::RegisterEventType(EventTypeId typeId, const std::string& typeName) {
        std::unique_lock<std::shared_mutex> lock(m_typesMutex);
        m_registeredTypes[typeId] = typeName;

        Logger::GetInstance().Debug("Registered event type: " + typeName, "EventSystem");
    }

    bool EventDispatcher::IsEventTypeRegistered(EventTypeId typeId) const {
        std::shared_lock<std::shared_mutex> lock(m_typesMutex);
        return m_registeredTypes.find(typeId) != m_registeredTypes.end();
    }

    std::string EventDispatcher::GetEventTypeName(EventTypeId typeId) const {
        std::shared_lock<std::shared_mutex> lock(m_typesMutex);

        auto it = m_registeredTypes.find(typeId);
        return it != m_registeredTypes.end() ? it->second : "Unknown";
    }

    size_t EventDispatcher::GetHandlerCount() const {
        std::shared_lock<std::shared_mutex> lock(m_handlersMutex);
        return m_handlers.size();
    }

    size_t EventDispatcher::GetQueuedEventCount() const {
        return m_eventQueue.Size();
    }

    size_t EventDispatcher::GetProcessedEventCount() const {
        return m_processedEventCount;
    }

    std::vector<HandlerId> EventDispatcher::GetAllHandlerIds() const {
        std::shared_lock<std::shared_mutex> lock(m_handlersMutex);

        std::vector<HandlerId> ids;
        ids.reserve(m_handlers.size());

        for (const auto& pair : m_handlers) {
            ids.push_back(pair.first);
        }

        return ids;
    }

    std::vector<std::string> EventDispatcher::GetAllHandlerNames() const {
        std::shared_lock<std::shared_mutex> lock(m_handlersMutex);

        std::vector<std::string> names;
        names.reserve(m_handlers.size());

        for (const auto& pair : m_handlers) {
            names.push_back(pair.second->GetName());
        }

        return names;
    }

    EventProfiler::EventStats EventDispatcher::GetEventStats(EventTypeId typeId) const {
        return m_profiler.GetEventStats(typeId);
    }

    std::unordered_map<EventTypeId, EventProfiler::EventStats> EventDispatcher::GetAllEventStats() const {
        return m_profiler.GetAllEventStats();
    }

    void EventDispatcher::FlushEventQueue() {
        ProcessQueuedEvents();
    }

    void EventDispatcher::ClearAllHandlers() {
        std::unique_lock<std::shared_mutex> lock(m_handlersMutex);

        size_t count = m_handlers.size();
        m_handlers.clear();
        m_handlerNames.clear();

        if (count > 0) {
            Logger::GetInstance().Info("Cleared " + std::to_string(count) + " event handlers", "EventSystem");
        }
    }

    void EventDispatcher::ResetStatistics() {
        std::unique_lock<std::shared_mutex> lock(m_statsMutex);

        m_processedEventCount = 0;
        m_queuedEventCount = 0;
        m_filteredEventCount = 0;

        m_profiler.Reset();
    }

    std::string EventDispatcher::GetSystemInfo() const {
        std::stringstream ss;
        ss << "Event System Info:\n";
        ss << "Handlers: " << GetHandlerCount() << "\n";
        ss << "Queued Events: " << GetQueuedEventCount() << "\n";
        ss << "Processed Events: " << GetProcessedEventCount() << "\n";
        ss << "Global Filters: " << m_globalFilters.size() << "\n";
        ss << "Registered Types: " << m_registeredTypes.size() << "\n";
        ss << "Async Processing: " << (m_asyncEnabled ? "Enabled" : "Disabled") << "\n";
        ss << "Profiling: " << (m_profilingEnabled ? "Enabled" : "Disabled") << "\n";
        ss << "Worker Threads: " << m_workerThreads.size() << "\n";
        return ss.str();
    }

    void EventDispatcher::ProcessEvent(std::unique_ptr<EventBase> event) {
        if (!event) {
            return;
        }

        // Filter event
        if (ShouldFilterEvent(*event)) {
            m_filteredEventCount++;
            return;
        }

        auto startTime = std::chrono::steady_clock::now();

        // Find and execute handlers
        std::vector<std::shared_ptr<EventHandler>> handlers = FindHandlersForEvent(*event);
        ExecuteHandlers(*event, handlers);

        auto endTime = std::chrono::steady_clock::now();
        uint64_t processingTime = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count();

        // Profile event if enabled
        if (m_profilingEnabled) {
            ProfileEventDispatch(*event, processingTime);
        }

        m_processedEventCount++;
    }

    void EventDispatcher::ProcessImmediateEvent(std::unique_ptr<EventBase> event) {
        if (!event) {
            return;
        }

        // Skip filtering for immediate events
        std::vector<std::shared_ptr<EventHandler>> handlers = FindHandlersForEvent(*event);
        ExecuteHandlers(*event, handlers);

        m_processedEventCount++;
    }

    void EventDispatcher::ProcessQueuedEvents() {
        // Process up to 100 events per frame to avoid blocking
        const size_t maxEventsPerFrame = 100;
        size_t processed = 0;

        while (processed < maxEventsPerFrame) {
            auto event = m_eventQueue.Pop();
            if (!event) {
                break;
            }

            ProcessEvent(std::move(event));
            processed++;
        }
    }

    void EventDispatcher::StartWorkerThreads() {
        m_running = true;

        for (uint32_t i = 0; i < m_workerThreadCount; ++i) {
            m_workerThreads.emplace_back(&EventDispatcher::WorkerThreadFunction, this);
        }

        Logger::GetInstance().Info("Started " + std::to_string(m_workerThreadCount) + " event worker threads", "EventSystem");
    }

    void EventDispatcher::StopWorkerThreads() {
        m_running = false;

        // Wake up all worker threads
        m_eventQueue.Clear();  // This will wake up threads waiting on empty queue

        // Wait for threads to finish
        for (auto& thread : m_workerThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        m_workerThreads.clear();

        Logger::GetInstance().Info("Stopped event worker threads", "EventSystem");
    }

    void EventDispatcher::WorkerThreadFunction() {
        while (m_running) {
            auto event = m_eventQueue.Pop();
            if (!event) {
                // Queue is empty, continue waiting
                continue;
            }

            ProcessEvent(std::move(event));
        }
    }

    bool EventDispatcher::ExecuteHandler(const std::shared_ptr<EventHandler>& handler, const EventBase& event) {
        if (!handler || !handler->IsEnabled()) {
            return false;
        }

        if (!handler->CanHandle(event.GetTypeId())) {
            return false;
        }

        try {
            auto startTime = std::chrono::steady_clock::now();
            bool result = handler->HandleEvent(event);
            auto endTime = std::chrono::steady_clock::now();

            uint64_t executionTime = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count();

            // Record execution time
            if (m_profilingEnabled) {
                m_profiler.RecordHandlerExecution(handler->GetId(), event.GetTypeId(), executionTime);
            }

            return result;

        } catch (const std::exception& e) {
            Logger::GetInstance().Error("Exception in event handler '" + handler->GetName() + "': " + e.what(), "EventSystem");
            return false;
        }
    }

    void EventDispatcher::ExecuteHandlers(const EventBase& event, std::vector<std::shared_ptr<EventHandler>>& handlers) {
        // Sort handlers by priority (higher priority first)
        std::sort(handlers.begin(), handlers.end(),
            [](const std::shared_ptr<EventHandler>& a, const std::shared_ptr<EventHandler>& b) {
                return static_cast<int>(a->GetPriority()) > static_cast<int>(b->GetPriority());
            });

        // Execute handlers
        for (const auto& handler : handlers) {
            if (event.IsConsumed()) {
                break;  // Stop if event is consumed
            }

            ExecuteHandler(handler, event);
        }
    }

    std::vector<std::shared_ptr<EventHandler>> EventDispatcher::FindHandlersForEvent(const EventBase& event) {
        std::vector<std::shared_ptr<EventHandler>> handlers;

        std::shared_lock<std::shared_mutex> lock(m_handlersMutex);

        for (const auto& pair : m_handlers) {
            if (pair.second->CanHandle(event.GetTypeId())) {
                handlers.push_back(pair.second);
            }
        }

        return handlers;
    }

    bool EventDispatcher::ShouldFilterEvent(const EventBase& event) const {
        std::shared_lock<std::shared_mutex> lock(m_filtersMutex);

        for (const auto& filter : m_globalFilters) {
            if (filter->ShouldFilter(event)) {
                return true;
            }
        }

        return false;
    }

    void EventDispatcher::ProfileEventDispatch(const EventBase& event, uint64_t processingTimeNs) {
        m_profiler.RecordEventDispatch(event, processingTimeNs);
    }

    void EventDispatcher::UpdateStatistics() {
        // Update per-frame statistics here if needed
    }

    void EventDispatcher::RegisterCommonEventTypes() {
        // Register all the event types defined in Events namespace
        // This would be a very long list, so we'll register a few key ones

        RegisterEventType(Events::KeyPressedEvent::GetStaticTypeId(), "KeyPressed");
        RegisterEventType(Events::PlayerSpawnedEvent::GetStaticTypeId(), "PlayerSpawned");
        RegisterEventType(Events::BlockPlacedEvent::GetStaticTypeId(), "BlockPlaced");
        RegisterEventType(Events::EntitySpawnedEvent::GetStaticTypeId(), "EntitySpawned");
        RegisterEventType(Events::ChatMessageEvent::GetStaticTypeId(), "ChatMessage");
        RegisterEventType(Events::WorldLoadedEvent::GetStaticTypeId(), "WorldLoaded");
        RegisterEventType(Events::GameStartedEvent::GetStaticTypeId(), "GameStarted");
        RegisterEventType(Events::SystemInitializedEvent::GetStaticTypeId(), "SystemInitialized");
        RegisterEventType(Events::LogMessageEvent::GetStaticTypeId(), "LogMessage");
        RegisterEventType(Events::PerformanceWarningEvent::GetStaticTypeId(), "PerformanceWarning");

        Logger::GetInstance().Info("Registered common event types", "EventSystem");
    }

} // namespace VoxelCraft
