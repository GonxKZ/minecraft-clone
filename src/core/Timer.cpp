/**
 * @file Timer.cpp
 * @brief VoxelCraft Engine Timer Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file implements the high-precision timer system for the VoxelCraft engine.
 */

#include "Timer.hpp"
#include "Logger.hpp"

#include <algorithm>

namespace VoxelCraft {

    // Global timer manager instance
    static std::unique_ptr<TimerManager> s_instance;
    static std::mutex s_instanceMutex;

    // Timer implementation

    Timer::Timer(TimerID id, const std::string& name, TimerType type)
        : m_id(id)
        , m_name(name)
        , m_type(type)
        , m_running(false)
        , m_paused(false)
        , m_startTime(TimePoint{})
        , m_pauseTime(Duration::zero())
        , m_elapsed(Duration::zero())
        , m_callback(nullptr)
    {
    }

    Timer::~Timer() {
        VOXELCRAFT_TRACE("Timer '{}' destroyed", m_name);
    }

    void Timer::Start() {
        if (!m_running) {
            m_startTime = std::chrono::steady_clock::now();
            m_running = true;
            m_paused = false;
            VOXELCRAFT_TRACE("Timer '{}' started", m_name);
        }
    }

    void Timer::Stop() {
        if (m_running) {
            UpdateElapsed();
            m_running = false;
            m_paused = false;
            VOXELCRAFT_TRACE("Timer '{}' stopped", m_name);
        }
    }

    void Timer::Pause() {
        if (m_running && !m_paused) {
            UpdateElapsed();
            m_paused = true;
            VOXELCRAFT_TRACE("Timer '{}' paused", m_name);
        }
    }

    void Timer::Resume() {
        if (m_running && m_paused) {
            m_pauseTime += std::chrono::steady_clock::now() - m_startTime;
            m_paused = false;
            VOXELCRAFT_TRACE("Timer '{}' resumed", m_name);
        }
    }

    void Timer::Reset() {
        m_elapsed = Duration::zero();
        m_pauseTime = Duration::zero();
        m_startTime = std::chrono::steady_clock::now();
        VOXELCRAFT_TRACE("Timer '{}' reset", m_name);
    }

    void Timer::Restart() {
        Reset();
        Start();
    }

    double Timer::GetElapsedSeconds() const {
        return std::chrono::duration_cast<std::chrono::duration<double>>(GetElapsed()).count();
    }

    double Timer::GetElapsedMilliseconds() const {
        return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(GetElapsed()).count();
    }

    Timer::Duration Timer::GetElapsed() const {
        if (!m_running) {
            return m_elapsed;
        }

        if (m_paused) {
            return m_elapsed;
        }

        return m_elapsed + (std::chrono::steady_clock::now() - m_startTime - m_pauseTime);
    }

    void Timer::UpdateElapsed() {
        if (m_running && !m_paused) {
            m_elapsed += std::chrono::steady_clock::now() - m_startTime - m_pauseTime;
            m_startTime = std::chrono::steady_clock::now();
            m_pauseTime = Duration::zero();
        }
    }

    // ProfilerTimer implementation

    ProfilerTimer::ProfilerTimer(TimerID id, const std::string& name)
        : Timer(id, name, TimerType::Profiler)
        , m_callCount(0)
        , m_totalTime(Duration::zero())
        , m_minTime(Duration::max())
        , m_maxTime(Duration::min())
        , m_averageTime(Duration::zero())
    {
    }

    void ProfilerTimer::Start() {
        Timer::Start();
    }

    void ProfilerTimer::Stop() {
        if (m_running) {
            auto elapsed = GetElapsed();
            m_callCount++;
            m_totalTime += elapsed;
            m_minTime = std::min(m_minTime, elapsed);
            m_maxTime = std::max(m_maxTime, elapsed);
            m_averageTime = m_totalTime / m_callCount;
        }
        Timer::Stop();
    }

    // TimerManager implementation

    TimerManager::TimerManager()
        : m_nextTimerId(1)
        , m_profilingEnabled(false)
    {
        VOXELCRAFT_TRACE("Timer manager instance created");
    }

    TimerManager::~TimerManager() {
        VOXELCRAFT_TRACE("Timer manager instance destroyed");

        StopAllTimers();
    }

    TimerID TimerManager::CreateTimer(const std::string& name, TimerType type) {
        std::lock_guard<std::mutex> lock(m_timerMutex);

        TimerID id = m_nextTimerId++;
        auto timer = std::make_unique<Timer>(id, name, type);
        m_timers[id] = std::move(timer);
        m_timerNames[name] = id;

        VOXELCRAFT_TRACE("Created timer '{}' (ID: {})", name, id);
        return id;
    }

    Timer* TimerManager::GetTimer(TimerID id) {
        std::lock_guard<std::mutex> lock(m_timerMutex);
        auto it = m_timers.find(id);
        return it != m_timers.end() ? it->second.get() : nullptr;
    }

    Timer* TimerManager::GetTimer(const std::string& name) {
        std::lock_guard<std::mutex> lock(m_timerMutex);
        auto it = m_timerNames.find(name);
        if (it != m_timerNames.end()) {
            return GetTimer(it->second);
        }
        return nullptr;
    }

    bool TimerManager::DestroyTimer(TimerID id) {
        std::lock_guard<std::mutex> lock(m_timerMutex);
        auto it = m_timers.find(id);
        if (it != m_timers.end()) {
            std::string name = it->second->GetName();
            m_timers.erase(it);
            m_timerNames.erase(name);
            VOXELCRAFT_TRACE("Destroyed timer '{}' (ID: {})", name, id);
            return true;
        }
        return false;
    }

    bool TimerManager::DestroyTimer(const std::string& name) {
        std::lock_guard<std::mutex> lock(m_timerMutex);
        auto it = m_timerNames.find(name);
        if (it != m_timerNames.end()) {
            return DestroyTimer(it->second);
        }
        return false;
    }

    void TimerManager::StartTimer(TimerID id) {
        Timer* timer = GetTimer(id);
        if (timer) {
            timer->Start();
        }
    }

    void TimerManager::StartTimer(const std::string& name) {
        Timer* timer = GetTimer(name);
        if (timer) {
            timer->Start();
        } else {
            // Auto-create timer if it doesn't exist
            TimerID id = CreateTimer(name, TimerType::Manual);
            StartTimer(id);
        }
    }

    void TimerManager::StopTimer(TimerID id) {
        Timer* timer = GetTimer(id);
        if (timer) {
            timer->Stop();
        }
    }

    void TimerManager::StopTimer(const std::string& name) {
        Timer* timer = GetTimer(name);
        if (timer) {
            timer->Stop();
        }
    }

    void TimerManager::PauseTimer(TimerID id) {
        Timer* timer = GetTimer(id);
        if (timer) {
            timer->Pause();
        }
    }

    void TimerManager::PauseTimer(const std::string& name) {
        Timer* timer = GetTimer(name);
        if (timer) {
            timer->Pause();
        }
    }

    void TimerManager::ResumeTimer(TimerID id) {
        Timer* timer = GetTimer(id);
        if (timer) {
            timer->Resume();
        }
    }

    void TimerManager::ResumeTimer(const std::string& name) {
        Timer* timer = GetTimer(name);
        if (timer) {
            timer->Resume();
        }
    }

    void TimerManager::ResetTimer(TimerID id) {
        Timer* timer = GetTimer(id);
        if (timer) {
            timer->Reset();
        }
    }

    void TimerManager::ResetTimer(const std::string& name) {
        Timer* timer = GetTimer(name);
        if (timer) {
            timer->Reset();
        }
    }

    double TimerManager::GetTimerElapsedSeconds(TimerID id) const {
        const Timer* timer = const_cast<TimerManager*>(this)->GetTimer(id);
        return timer ? timer->GetElapsedSeconds() : 0.0;
    }

    double TimerManager::GetTimerElapsedSeconds(const std::string& name) const {
        const Timer* timer = const_cast<TimerManager*>(this)->GetTimer(name);
        return timer ? timer->GetElapsedSeconds() : 0.0;
    }

    std::vector<TimerInfo> TimerManager::GetTimerInfo() const {
        std::lock_guard<std::mutex> lock(m_timerMutex);
        std::vector<TimerInfo> info;

        for (const auto& pair : m_timers) {
            const Timer* timer = pair.second.get();
            TimerInfo timerInfo;
            timerInfo.id = timer->GetID();
            timerInfo.name = timer->GetName();
            timerInfo.type = timer->GetType();
            timerInfo.running = timer->IsRunning();
            timerInfo.elapsedSeconds = timer->GetElapsedSeconds();
            info.push_back(timerInfo);
        }

        return info;
    }

    void TimerManager::StopAllTimers() {
        std::lock_guard<std::mutex> lock(m_timerMutex);

        for (auto& pair : m_timers) {
            pair.second->Stop();
        }

        VOXELCRAFT_INFO("Stopped all {} timers", m_timers.size());
    }

    void TimerManager::ResetAllTimers() {
        std::lock_guard<std::mutex> lock(m_timerMutex);

        for (auto& pair : m_timers) {
            pair.second->Reset();
        }

        VOXELCRAFT_INFO("Reset all {} timers", m_timers.size());
    }

    double TimerManager::GetTime() const {
        auto now = std::chrono::steady_clock::now();
        static auto start = now;
        return std::chrono::duration_cast<std::chrono::duration<double>>(now - start).count();
    }

    double TimerManager::GetDeltaTime() const {
        static auto lastTime = std::chrono::steady_clock::now();
        auto currentTime = std::chrono::steady_clock::now();
        double delta = std::chrono::duration_cast<std::chrono::duration<double>>(currentTime - lastTime).count();
        lastTime = currentTime;
        return delta;
    }

    // ScopedTimer implementation

    ScopedTimer::ScopedTimer(const std::string& name, TimerType type)
        : m_timerId(GetTimerManager().CreateTimer(name, type))
    {
        GetTimerManager().StartTimer(m_timerId);
    }

    ScopedTimer::~ScopedTimer() {
        GetTimerManager().StopTimer(m_timerId);
    }

    // Global accessor

    TimerManager& GetTimerManager() {
        std::lock_guard<std::mutex> lock(s_instanceMutex);
        if (!s_instance) {
            s_instance = std::make_unique<TimerManager>();
        }
        return *s_instance;
    }

} // namespace VoxelCraft
