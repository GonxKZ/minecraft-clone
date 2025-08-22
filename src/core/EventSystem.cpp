/**
 * @file EventSystem.cpp
 * @brief VoxelCraft Engine Event System Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file implements the event system for the VoxelCraft engine.
 */

#include "EventSystem.hpp"
#include "Logger.hpp"

#include <algorithm>

namespace VoxelCraft {

    // Static member initialization
    std::atomic<ListenerId> EventListener::s_nextId{1};
    std::atomic<ListenerId> EventSystem::s_nextListenerId{1};

    // EventListener implementation

    EventListener::EventListener()
        : m_id(s_nextId.fetch_add(1, std::memory_order_relaxed))
    {
    }

    // CallbackEventListener implementation

    CallbackEventListener::CallbackEventListener(
        const std::string& name,
        EventCallback callback,
        const std::vector<EventType>& eventTypes,
        EventPriority priority
    )
        : m_name(name)
        , m_callback(callback)
        , m_eventTypes(eventTypes)
        , m_priority(priority)
    {
    }

    bool CallbackEventListener::OnEvent(const Event& event) {
        if (m_callback) {
            return m_callback(event);
        }
        return false;
    }

    bool CallbackEventListener::HandlesEventType(const EventType& eventType) const {
        if (m_eventTypes.empty()) {
            return true; // Handle all event types if none specified
        }
        return std::find(m_eventTypes.begin(), m_eventTypes.end(), eventType) != m_eventTypes.end();
    }

    // EventSystem implementation

    EventSystem::EventSystem(size_t maxQueueSize, uint32_t numWorkerThreads)
        : m_maxQueueSize(maxQueueSize)
        , m_numWorkerThreads(numWorkerThreads)
        , m_running(false)
        , m_paused(false)
        , m_totalEvents(0)
        , m_queuedEvents(0)
        , m_droppedEvents(0)
    {
        VOXELCRAFT_TRACE("EventSystem instance created");

        m_startTime = std::chrono::steady_clock::now();
    }

    EventSystem::~EventSystem() {
        VOXELCRAFT_TRACE("EventSystem instance destroyed");

        Stop();

        // Clean up listeners
        std::lock_guard<std::mutex> lock(m_listenersMutex);
        m_listeners.clear();
    }

    bool EventSystem::QueueEvent(Event&& event) {
        std::lock_guard<std::mutex> lock(m_queueMutex);

        if (m_eventQueue.size() >= m_maxQueueSize) {
            m_droppedEvents++;
            VOXELCRAFT_WARNING("Event queue full, dropping event of type: {}", event.type);
            return false;
        }

        if (!PassesFilters(event)) {
            return true; // Event filtered out but not an error
        }

        m_eventQueue.push(std::move(event));
        m_queuedEvents++;

        m_queueCondition.notify_one();
        return true;
    }

    bool EventSystem::ProcessEventImmediately(const Event& event) {
        if (!PassesFilters(event)) {
            return true;
        }

        ProcessEvent(event);
        return true;
    }

    size_t EventSystem::DispatchEvents(size_t maxEvents) {
        std::queue<Event> eventsToProcess;
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            size_t eventsToDispatch = maxEvents > 0 ? std::min(maxEvents, m_eventQueue.size()) : m_eventQueue.size();

            for (size_t i = 0; i < eventsToDispatch; ++i) {
                eventsToProcess.push(std::move(m_eventQueue.front()));
                m_eventQueue.pop();
            }

            m_queuedEvents = m_eventQueue.size();
        }

        size_t processedCount = 0;
        while (!eventsToProcess.empty()) {
            ProcessEvent(eventsToProcess.front());
            eventsToProcess.pop();
            processedCount++;
            m_totalEvents++;
        }

        return processedCount;
    }

    size_t EventSystem::WaitAndDispatchEvents(uint32_t timeoutMs, size_t maxEvents) {
        std::unique_lock<std::mutex> lock(m_queueMutex);

        if (timeoutMs > 0) {
            m_queueCondition.wait_for(lock, std::chrono::milliseconds(timeoutMs),
                [this]() { return !m_eventQueue.empty() || !m_running; });
        } else {
            m_queueCondition.wait(lock, [this]() { return !m_eventQueue.empty() || !m_running; });
        }

        if (!m_running) {
            return 0;
        }

        size_t eventsToDispatch = maxEvents > 0 ? std::min(maxEvents, m_eventQueue.size()) : m_eventQueue.size();
        std::queue<Event> eventsToProcess;

        for (size_t i = 0; i < eventsToDispatch; ++i) {
            eventsToProcess.push(std::move(m_eventQueue.front()));
            m_eventQueue.pop();
        }

        m_queuedEvents = m_eventQueue.size();
        lock.unlock();

        size_t processedCount = 0;
        while (!eventsToProcess.empty()) {
            ProcessEvent(eventsToProcess.front());
            eventsToProcess.pop();
            processedCount++;
            m_totalEvents++;
        }

        return processedCount;
    }

    ListenerId EventSystem::AddListener(std::unique_ptr<EventListener> listener) {
        std::lock_guard<std::mutex> lock(m_listenersMutex);
        ListenerId id = listener->GetId();
        m_listeners[id] = std::move(listener);
        return id;
    }

    ListenerId EventSystem::AddListener(
        const std::string& name,
        EventCallback callback,
        const std::vector<EventType>& eventTypes,
        EventPriority priority
    ) {
        auto listener = std::make_unique<CallbackEventListener>(name, callback, eventTypes, priority);
        return AddListener(std::move(listener));
    }

    bool EventSystem::RemoveListener(ListenerId listenerId) {
        std::lock_guard<std::mutex> lock(m_listenersMutex);
        return m_listeners.erase(listenerId) > 0;
    }

    bool EventSystem::SetListenerEnabled(ListenerId listenerId, bool enabled) {
        // Simplified implementation - in a real system you'd track enabled state
        std::lock_guard<std::mutex> lock(m_listenersMutex);
        auto it = m_listeners.find(listenerId);
        if (it != m_listeners.end()) {
            // For now, just return true as if it was set
            return true;
        }
        return false;
    }

    EventListener* EventSystem::GetListener(ListenerId listenerId) {
        std::lock_guard<std::mutex> lock(m_listenersMutex);
        auto it = m_listeners.find(listenerId);
        return it != m_listeners.end() ? it->second.get() : nullptr;
    }

    void EventSystem::SetEventTypeFilter(const EventType& eventType, bool enabled) {
        std::lock_guard<std::mutex> lock(m_filterMutex);
        m_eventTypeFilters[eventType] = enabled;
    }

    void EventSystem::SetEventSourceFilter(const std::string& source, bool enabled) {
        std::lock_guard<std::mutex> lock(m_filterMutex);
        m_eventSourceFilters[source] = enabled;
    }

    void EventSystem::ClearFilters() {
        std::lock_guard<std::mutex> lock(m_filterMutex);
        m_eventTypeFilters.clear();
        m_eventSourceFilters.clear();
    }

    EventStatistics EventSystem::GetStatistics() const {
        EventStatistics stats;
        stats.totalEvents = m_totalEvents;
        stats.queuedEvents = m_queuedEvents;
        stats.droppedEvents = m_droppedEvents;
        stats.activeListeners = m_listeners.size();

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_startTime).count();
        stats.eventsPerSecond = elapsed > 0 ? stats.totalEvents / elapsed : 0;

        return stats;
    }

    void EventSystem::ResetStatistics() {
        m_totalEvents = 0;
        m_droppedEvents = 0;
        m_startTime = std::chrono::steady_clock::now();
    }

    size_t EventSystem::GetQueueSize() const {
        return m_queuedEvents;
    }

    bool EventSystem::IsQueueEmpty() const {
        return m_queuedEvents == 0;
    }

    bool EventSystem::IsQueueFull() const {
        return m_queuedEvents >= m_maxQueueSize;
    }

    void EventSystem::Start() {
        if (m_running) {
            return;
        }

        m_running = true;
        m_paused = false;

        // Start worker threads for async event processing
        for (uint32_t i = 0; i < m_numWorkerThreads; ++i) {
            m_workerThreads.emplace_back(&EventSystem::EventProcessingLoop, this);
        }

        VOXELCRAFT_INFO("EventSystem started with {} worker threads", m_numWorkerThreads);
    }

    void EventSystem::Stop() {
        if (!m_running) {
            return;
        }

        m_running = false;
        m_paused = false;

        // Notify all waiting threads
        m_queueCondition.notify_all();

        // Wait for worker threads to finish
        for (auto& thread : m_workerThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        m_workerThreads.clear();

        VOXELCRAFT_INFO("EventSystem stopped");
    }

    void EventSystem::Pause() {
        m_paused = true;
        VOXELCRAFT_INFO("EventSystem paused");
    }

    void EventSystem::Resume() {
        m_paused = false;
        VOXELCRAFT_INFO("EventSystem resumed");
    }

    void EventSystem::EventProcessingLoop() {
        VOXELCRAFT_INFO("Event processing thread started");

        while (m_running) {
            std::unique_lock<std::mutex> lock(m_queueMutex);

            m_queueCondition.wait(lock, [this]() {
                return !m_eventQueue.empty() || !m_running;
            });

            if (!m_running) {
                break;
            }

            if (m_paused) {
                continue;
            }

            // Process one event
            if (!m_eventQueue.empty()) {
                Event event = std::move(m_eventQueue.front());
                m_eventQueue.pop();
                m_queuedEvents--;

                lock.unlock();

                ProcessEvent(event);
                m_totalEvents++;
            }
        }

        VOXELCRAFT_INFO("Event processing thread stopped");
    }

    void EventSystem::ProcessEvent(const Event& event) {
        if (event.isAsync && m_workerThreads.empty()) {
            // If async but no worker threads, process synchronously
            VOXELCRAFT_WARNING("Async event processed synchronously due to no worker threads");
        }

        std::vector<EventListener*> listenersToNotify;

        {
            std::lock_guard<std::mutex> lock(m_listenersMutex);
            for (const auto& pair : m_listeners) {
                auto& listener = pair.second;
                if (listener->IsEnabled() && listener->HandlesEventType(event.type)) {
                    listenersToNotify.push_back(listener.get());
                }
            }
        }

        // Sort listeners by priority (highest first)
        SortListenersByPriority(listenersToNotify);

        // Notify listeners
        for (auto* listener : listenersToNotify) {
            if (event.IsCancelled()) {
                break; // Stop processing if event was cancelled
            }

            try {
                listener->OnEvent(event);
            } catch (const std::exception& e) {
                VOXELCRAFT_ERROR("Exception in event listener {}: {}", listener->GetName(), e.what());
            }
        }
    }

    void EventSystem::SortListenersByPriority(std::vector<EventListener*>& listeners) {
        std::sort(listeners.begin(), listeners.end(),
            [](EventListener* a, EventListener* b) {
                return static_cast<int>(a->GetPriority()) > static_cast<int>(b->GetPriority());
            });
    }

    bool EventSystem::PassesFilters(const Event& event) {
        std::lock_guard<std::mutex> lock(m_filterMutex);

        // Check event type filter
        auto typeIt = m_eventTypeFilters.find(event.type);
        if (typeIt != m_eventTypeFilters.end() && !typeIt->second) {
            return false;
        }

        // Check event source filter
        auto sourceIt = m_eventSourceFilters.find(event.source);
        if (sourceIt != m_eventSourceFilters.end() && !sourceIt->second) {
            return false;
        }

        return true;
    }

} // namespace VoxelCraft
