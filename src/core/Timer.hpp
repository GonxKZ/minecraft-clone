/**
 * @file Timer.hpp
 * @brief VoxelCraft Engine High-Precision Timer System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file provides a comprehensive, high-precision timing system for the
 * VoxelCraft engine with support for multiple timer types, profiling,
 * performance monitoring, and cross-platform compatibility.
 */

#ifndef VOXELCRAFT_CORE_TIMER_HPP
#define VOXELCRAFT_CORE_TIMER_HPP

#include <chrono>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>
#include <optional>
#include <deque>

namespace VoxelCraft {

    /**
     * @typedef TimePoint
     * @brief High-resolution time point type
     */
    using TimePoint = std::chrono::steady_clock::time_point;

    /**
     * @typedef Duration
     * @brief High-resolution duration type
     */
    using Duration = std::chrono::steady_clock::duration;

    /**
     * @typedef TimerID
     * @brief Unique identifier for timers
     */
    using TimerID = uint64_t;

    /**
     * @enum TimerType
     * @brief Type of timer for different use cases
     */
    enum class TimerType {
        Once,           ///< Single-shot timer
        Repeating,      ///< Repeating timer
        Countdown,      ///< Countdown timer
        Stopwatch,      ///< Stopwatch timer
        Interval,       ///< Interval-based timer
        Performance,    ///< Performance profiling timer
        System          ///< System-level timer
    };

    /**
     * @struct TimerEvent
     * @brief Timer event data structure
     */
    struct TimerEvent {
        TimerID timerId;                    ///< Timer that triggered the event
        TimerType type;                     ///< Type of timer
        std::string name;                   ///< Timer name
        double elapsedTime;                 ///< Elapsed time in seconds
        double targetTime;                  ///< Target time for the timer
        int repeatCount;                    ///< Number of times timer has repeated
        bool isComplete;                    ///< Timer completion flag
        TimePoint triggerTime;              ///< Time when timer triggered
    };

    /**
     * @typedef TimerCallback
     * @brief Callback function type for timer events
     */
    using TimerCallback = std::function<void(const TimerEvent&)>;

    /**
     * @struct TimerStatistics
     * @brief Performance statistics for timer system
     */
    struct TimerStatistics {
        uint64_t totalTimers;               ///< Total number of active timers
        uint64_t timersTriggered;           ///< Total timers triggered
        double averageLatency;              ///< Average timer latency (μs)
        double maxLatency;                  ///< Maximum timer latency (μs)
        double minLatency;                  ///< Minimum timer latency (μs)
        uint64_t missedDeadlines;           ///< Number of missed deadlines
        double systemLoad;                  ///< Timer system CPU load percentage
    };

    /**
     * @class Timer
     * @brief High-precision timer with multiple modes and features
     *
     * The Timer class provides:
     * - High-precision timing using steady_clock
     * - Multiple timer types (one-shot, repeating, countdown, etc.)
     * - Callback system for timer events
     * - Performance profiling capabilities
     * - Thread-safe operations
     * - Memory pool allocation for performance
     */
    class Timer {
    public:
        /**
         * @brief Constructor
         * @param name Timer name
         * @param type Timer type
         */
        explicit Timer(const std::string& name, TimerType type = TimerType::Once);

        /**
         * @brief Destructor
         */
        ~Timer();

        /**
         * @brief Deleted copy constructor
         */
        Timer(const Timer&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        Timer& operator=(const Timer&) = delete;

        // Timer identification

        /**
         * @brief Get timer ID
         * @return Timer ID
         */
        TimerID GetID() const { return m_id; }

        /**
         * @brief Get timer name
         * @return Timer name
         */
        const std::string& GetName() const { return m_name; }

        /**
         * @brief Get timer type
         * @return Timer type
         */
        TimerType GetType() const { return m_type; }

        // Timer state

        /**
         * @brief Check if timer is active
         * @return true if active, false otherwise
         */
        bool IsActive() const { return m_active; }

        /**
         * @brief Check if timer is paused
         * @return true if paused, false otherwise
         */
        bool IsPaused() const { return m_paused; }

        /**
         * @brief Check if timer has completed
         * @return true if completed, false otherwise
         */
        bool IsCompleted() const { return m_completed; }

        // Timer control

        /**
         * @brief Start the timer
         * @param duration Timer duration in seconds
         * @return true if started, false if already active
         */
        bool Start(double duration = 0.0);

        /**
         * @brief Stop the timer
         */
        void Stop();

        /**
         * @brief Pause the timer
         */
        void Pause();

        /**
         * @brief Resume the timer
         */
        void Resume();

        /**
         * @brief Reset the timer
         * @param duration New duration (0 = keep current)
         */
        void Reset(double duration = 0.0);

        /**
         * @brief Restart the timer
         * @param duration New duration (0 = keep current)
         */
        void Restart(double duration = 0.0);

        // Timer information

        /**
         * @brief Get elapsed time in seconds
         * @return Elapsed time
         */
        double GetElapsedTime() const;

        /**
         * @brief Get remaining time in seconds
         * @return Remaining time
         */
        double GetRemainingTime() const;

        /**
         * @brief Get total duration in seconds
         * @return Total duration
         */
        double GetDuration() const { return m_duration; }

        /**
         * @brief Get number of times timer has repeated
         * @return Repeat count
         */
        int GetRepeatCount() const { return m_repeatCount; }

        /**
         * @brief Get start time
         * @return Start time point
         */
        TimePoint GetStartTime() const { return m_startTime; }

        /**
         * @brief Get last trigger time
         * @return Last trigger time point
         */
        TimePoint GetLastTriggerTime() const { return m_lastTriggerTime; }

        // Timer configuration

        /**
         * @brief Set timer duration
         * @param duration New duration in seconds
         */
        void SetDuration(double duration);

        /**
         * @brief Set repeat count for repeating timers
         * @param count Number of repetitions (0 = infinite)
         */
        void SetRepeatCount(int count);

        /**
         * @brief Set timer callback
         * @param callback Callback function
         */
        void SetCallback(TimerCallback callback);

        /**
         * @brief Set user data
         * @param data User data pointer
         */
        void SetUserData(void* data) { m_userData = data; }

        /**
         * @brief Get user data
         * @return User data pointer
         */
        void* GetUserData() const { return m_userData; }

        // Timer utilities

        /**
         * @brief Get elapsed time as string
         * @param format Format string (e.g., "mm:ss.ms")
         * @return Formatted time string
         */
        std::string GetElapsedTimeAsString(const std::string& format = "ss.ms") const;

        /**
         * @brief Get progress as percentage (0.0 - 1.0)
         * @return Progress percentage
         */
        double GetProgress() const;

        /**
         * @brief Check if timer should trigger
         * @param currentTime Current time point
         * @return true if should trigger, false otherwise
         */
        bool ShouldTrigger(const TimePoint& currentTime);

        /**
         * @brief Trigger the timer manually
         */
        void Trigger();

    private:
        /**
         * @brief Update timer state
         * @param currentTime Current time point
         */
        void Update(const TimePoint& currentTime);

        /**
         * @brief Generate unique timer ID
         * @return Unique ID
         */
        static TimerID GenerateID();

        TimerID m_id;                       ///< Unique timer ID
        std::string m_name;                 ///< Timer name
        TimerType m_type;                   ///< Timer type

        bool m_active;                      ///< Timer active flag
        bool m_paused;                      ///< Timer paused flag
        bool m_completed;                   ///< Timer completed flag

        double m_duration;                  ///< Timer duration in seconds
        int m_repeatCount;                  ///< Number of repetitions
        int m_maxRepeatCount;               ///< Maximum repetitions (0 = infinite)

        TimePoint m_startTime;              ///< Timer start time
        TimePoint m_pauseTime;              ///< Timer pause time
        TimePoint m_lastTriggerTime;        ///< Last trigger time
        Duration m_pausedDuration;          ///< Total paused duration

        TimerCallback m_callback;           ///< Timer callback function
        void* m_userData;                   ///< User data pointer

        // Statistics
        double m_totalActiveTime;           ///< Total time timer was active
        int m_triggerCount;                 ///< Number of times triggered

        static std::atomic<TimerID> s_nextID;  ///< Next timer ID counter
    };

    /**
     * @class TimerManager
     * @brief Global timer management system
     *
     * The TimerManager provides:
     * - Centralized timer management
     * - High-precision timer updates
     * - Performance monitoring
     * - Thread-safe operations
     * - Memory pool for timer objects
     */
    class TimerManager {
    public:
        /**
         * @brief Constructor
         */
        TimerManager();

        /**
         * @brief Destructor
         */
        ~TimerManager();

        /**
         * @brief Deleted copy constructor
         */
        TimerManager(const TimerManager&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        TimerManager& operator=(const TimerManager&) = delete;

        // Timer creation

        /**
         * @brief Create a new timer
         * @param name Timer name
         * @param type Timer type
         * @return Timer pointer
         */
        std::shared_ptr<Timer> CreateTimer(const std::string& name, TimerType type = TimerType::Once);

        /**
         * @brief Create one-shot timer
         * @param name Timer name
         * @param duration Timer duration in seconds
         * @param callback Timer callback
         * @return Timer pointer
         */
        std::shared_ptr<Timer> CreateOneShot(const std::string& name,
                                           double duration,
                                           TimerCallback callback);

        /**
         * @brief Create repeating timer
         * @param name Timer name
         * @param interval Timer interval in seconds
         * @param callback Timer callback
         * @param repeatCount Number of repetitions (0 = infinite)
         * @return Timer pointer
         */
        std::shared_ptr<Timer> CreateRepeating(const std::string& name,
                                             double interval,
                                             TimerCallback callback,
                                             int repeatCount = 0);

        /**
         * @brief Create countdown timer
         * @param name Timer name
         * @param duration Countdown duration in seconds
         * @param callback Timer callback
         * @return Timer pointer
         */
        std::shared_ptr<Timer> CreateCountdown(const std::string& name,
                                             double duration,
                                             TimerCallback callback);

        /**
         * @brief Create stopwatch timer
         * @param name Timer name
         * @return Timer pointer
         */
        std::shared_ptr<Timer> CreateStopwatch(const std::string& name);

        // Timer management

        /**
         * @brief Get timer by ID
         * @param id Timer ID
         * @return Timer pointer or nullptr if not found
         */
        std::shared_ptr<Timer> GetTimer(TimerID id);

        /**
         * @brief Get timer by name
         * @param name Timer name
         * @return Timer pointer or nullptr if not found
         */
        std::shared_ptr<Timer> GetTimer(const std::string& name);

        /**
         * @brief Destroy timer
         * @param timer Timer to destroy
         * @return true if destroyed, false if not found
         */
        bool DestroyTimer(std::shared_ptr<Timer> timer);

        /**
         * @brief Destroy timer by ID
         * @param id Timer ID
         * @return true if destroyed, false if not found
         */
        bool DestroyTimer(TimerID id);

        /**
         * @brief Destroy all timers
         */
        void DestroyAllTimers();

        // Timer control

        /**
         * @brief Update all timers
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Pause all timers
         */
        void PauseAllTimers();

        /**
         * @brief Resume all timers
         */
        void ResumeAllTimers();

        /**
         * @brief Stop all timers
         */
        void StopAllTimers();

        // Timer information

        /**
         * @brief Get number of active timers
         * @return Number of active timers
         */
        size_t GetActiveTimerCount() const;

        /**
         * @brief Get number of paused timers
         * @return Number of paused timers
         */
        size_t GetPausedTimerCount() const;

        /**
         * @brief Get all timer names
         * @return Vector of timer names
         */
        std::vector<std::string> GetTimerNames() const;

        /**
         * @brief Check if timer exists
         * @param name Timer name
         * @return true if exists, false otherwise
         */
        bool TimerExists(const std::string& name) const;

        // Statistics and monitoring

        /**
         * @brief Get timer statistics
         * @return Timer statistics
         */
        TimerStatistics GetStatistics() const;

        /**
         * @brief Reset statistics
         */
        void ResetStatistics();

        /**
         * @brief Get performance report
         * @return Performance report as string
         */
        std::string GetPerformanceReport() const;

        // Global timer utilities

        /**
         * @brief Get current high-precision time in seconds
         * @return Current time
         */
        static double GetCurrentTime();

        /**
         * @brief Get current time point
         * @return Current time point
         */
        static TimePoint GetCurrentTimePoint();

        /**
         * @brief Convert duration to seconds
         * @param duration Duration to convert
         * @return Duration in seconds
         */
        static double DurationToSeconds(const Duration& duration);

        /**
         * @brief Convert seconds to duration
         * @param seconds Seconds to convert
         * @return Duration object
         */
        static Duration SecondsToDuration(double seconds);

        /**
         * @brief Format time as string
         * @param seconds Time in seconds
         * @param format Format string
         * @return Formatted time string
         */
        static std::string FormatTime(double seconds, const std::string& format = "mm:ss.ms");

    private:
        /**
         * @brief Process timer events
         */
        void ProcessTimerEvents();

        /**
         * @brief Update statistics
         * @param deltaTime Time elapsed since last update
         */
        void UpdateStatistics(double deltaTime);

        /**
         * @brief Cleanup completed timers
         */
        void CleanupCompletedTimers();

        std::unordered_map<TimerID, std::shared_ptr<Timer>> m_timers;
        std::unordered_map<std::string, TimerID> m_nameToId;

        mutable std::shared_mutex m_timerMutex;

        // Timer event processing
        std::deque<TimerEvent> m_timerEvents;
        mutable std::mutex m_eventMutex;

        // Statistics
        TimerStatistics m_statistics;
        TimePoint m_lastUpdateTime;
        double m_updateLatencyAccumulator;
        int m_updateLatencySampleCount;

        // Configuration
        size_t m_maxTimers;                    ///< Maximum number of timers
        bool m_autoCleanup;                    ///< Auto-cleanup completed timers
        double m_cleanupInterval;              ///< Cleanup interval in seconds
        double m_lastCleanupTime;              ///< Last cleanup time
    };

    /**
     * @class ScopedTimer
     * @brief RAII timer for performance profiling
     */
    class ScopedTimer {
    public:
        /**
         * @brief Constructor
         * @param name Timer name
         * @param autoReport Auto-report on destruction
         */
        explicit ScopedTimer(const std::string& name, bool autoReport = true);

        /**
         * @brief Destructor
         */
        ~ScopedTimer();

        /**
         * @brief Stop the timer
         */
        void Stop();

        /**
         * @brief Get elapsed time
         * @return Elapsed time in seconds
         */
        double GetElapsedTime() const;

        /**
         * @brief Get timer name
         * @return Timer name
         */
        const std::string& GetName() const { return m_name; }

    private:
        std::string m_name;                    ///< Timer name
        TimePoint m_startTime;                 ///< Start time
        bool m_stopped;                        ///< Stopped flag
        bool m_autoReport;                     ///< Auto-report flag
        double m_elapsedTime;                  ///< Elapsed time
    };

    // Global timer manager accessor

    /**
     * @brief Get global timer manager instance
     * @return Timer manager reference
     */
    TimerManager& GetTimerManager();

    // Convenience macros

    /**
     * @def VOXELCRAFT_SCOPED_TIMER(name)
     * @brief Create a scoped timer for performance profiling
     */
    #define VOXELCRAFT_SCOPED_TIMER(name) \
        VoxelCraft::ScopedTimer scopedTimer_##__LINE__(name)

    /**
     * @def VOXELCRAFT_PROFILE_FUNCTION()
     * @brief Profile the current function
     */
    #define VOXELCRAFT_PROFILE_FUNCTION() \
        VOXELCRAFT_SCOPED_TIMER(__FUNCTION__)

} // namespace VoxelCraft

#endif // VOXELCRAFT_CORE_TIMER_HPP
