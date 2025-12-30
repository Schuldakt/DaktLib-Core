#pragma once

// ============================================================================
// DaktLib Core - Time
// Time measurement and utilities using C++20/23 chrono
// ============================================================================

#include "Macros.hpp"
#include "Types.hpp"

#include <chrono>
#include <format>
#include <functional>
#include <string>

namespace dakt::core
{

// ============================================================================
// Time Types (using C++20 chrono)
// ============================================================================

using Clock = std::chrono::high_resolution_clock;
using SteadyClock = std::chrono::steady_clock;
using SystemClock = std::chrono::system_clock;

using TimePoint = Clock::time_point;
using SteadyTimePoint = SteadyClock::time_point;
using SystemTimePoint = SystemClock::time_point;

using Duration = Clock::duration;

using Nanoseconds = std::chrono::nanoseconds;
using Microseconds = std::chrono::microseconds;
using Milliseconds = std::chrono::milliseconds;
using Seconds = std::chrono::seconds;
using Minutes = std::chrono::minutes;
using Hours = std::chrono::hours;

// C++20 calendar types
using Days = std::chrono::days;
using Weeks = std::chrono::weeks;
using Months = std::chrono::months;
using Years = std::chrono::years;

// ============================================================================
// Time Functions
// ============================================================================

namespace time
{

// Get current time point
[[nodiscard]] DAKT_FORCEINLINE TimePoint now() noexcept
{
    return Clock::now();
}

[[nodiscard]] DAKT_FORCEINLINE SteadyTimePoint steadyNow() noexcept
{
    return SteadyClock::now();
}

[[nodiscard]] DAKT_FORCEINLINE SystemTimePoint systemNow() noexcept
{
    return SystemClock::now();
}

// Duration since a time point
template <typename D = Milliseconds>
[[nodiscard]] DAKT_FORCEINLINE i64 since(TimePoint start) noexcept
{
    return std::chrono::duration_cast<D>(now() - start).count();
}

// Duration between two time points
template <typename D = Milliseconds>
[[nodiscard]] DAKT_FORCEINLINE i64 between(TimePoint start, TimePoint end) noexcept
{
    return std::chrono::duration_cast<D>(end - start).count();
}

// Get current time in various units since epoch
[[nodiscard]] DAKT_FORCEINLINE i64 nowNanos() noexcept
{
    return std::chrono::duration_cast<Nanoseconds>(now().time_since_epoch()).count();
}

[[nodiscard]] DAKT_FORCEINLINE i64 nowMicros() noexcept
{
    return std::chrono::duration_cast<Microseconds>(now().time_since_epoch()).count();
}

[[nodiscard]] DAKT_FORCEINLINE i64 nowMillis() noexcept
{
    return std::chrono::duration_cast<Milliseconds>(now().time_since_epoch()).count();
}

[[nodiscard]] DAKT_FORCEINLINE i64 nowSeconds() noexcept
{
    return std::chrono::duration_cast<Seconds>(now().time_since_epoch()).count();
}

// ============================================================================
// Timestamp (using C++20 calendar types)
// ============================================================================

struct Timestamp
{
    i32 year;
    u32 month;        // 1-12
    u32 day;          // 1-31
    u32 hour;         // 0-23
    u32 minute;       // 0-59
    u32 second;       // 0-59
    u32 millisecond;  // 0-999
    u32 dayOfWeek;    // 0-6 (Sunday = 0)
    u32 dayOfYear;    // 1-366

    [[nodiscard]] bool isValid() const noexcept;

    // Comparison
    [[nodiscard]] auto operator<=>(const Timestamp&) const = default;
};

// Get current local time
[[nodiscard]] DAKT_API Timestamp localTime();

// Get current UTC time
[[nodiscard]] DAKT_API Timestamp utcTime();

// Get Unix timestamp (seconds since 1970-01-01 00:00:00 UTC)
[[nodiscard]] DAKT_API i64 unixTimestamp();

// Get Unix timestamp in milliseconds
[[nodiscard]] DAKT_API i64 unixTimestampMs();

// Convert timestamp to Unix timestamp
[[nodiscard]] DAKT_API i64 toUnixTimestamp(const Timestamp& ts);

// Convert Unix timestamp to Timestamp (local time)
[[nodiscard]] DAKT_API Timestamp fromUnixTimestamp(i64 unixTs);

// Convert Unix timestamp to Timestamp (UTC)
[[nodiscard]] DAKT_API Timestamp fromUnixTimestampUtc(i64 unixTs);

// ============================================================================
// Time Formatting (using C++20 chrono formatting)
// ============================================================================

// Format timestamp as string using std::format
[[nodiscard]] DAKT_API String formatTimestamp(const Timestamp& ts, StringView format = "{:%Y-%m-%d %H:%M:%S}");

// Format helpers
[[nodiscard]] DAKT_API String formatISO8601(const Timestamp& ts);   // 2024-01-15T14:30:00
[[nodiscard]] DAKT_API String formatDate(const Timestamp& ts);      // 2024-01-15
[[nodiscard]] DAKT_API String formatTime(const Timestamp& ts);      // 14:30:00
[[nodiscard]] DAKT_API String formatDateTime(const Timestamp& ts);  // 2024-01-15 14:30:00

// Format system time point directly
template <typename Duration>
[[nodiscard]] String formatTimePoint(std::chrono::time_point<SystemClock, Duration> tp)
{
    return std::format("{:%Y-%m-%d %H:%M:%S}", tp);
}

// Format duration
template <typename Rep, typename Period>
[[nodiscard]] String formatDuration(std::chrono::duration<Rep, Period> dur)
{
    auto hours = std::chrono::duration_cast<Hours>(dur);
    dur -= hours;
    auto mins = std::chrono::duration_cast<Minutes>(dur);
    dur -= mins;
    auto secs = std::chrono::duration_cast<Seconds>(dur);

    if (hours.count() > 0)
    {
        return std::format("{}h {}m {}s", hours.count(), mins.count(), secs.count());
    }
    else if (mins.count() > 0)
    {
        return std::format("{}m {}s", mins.count(), secs.count());
    }
    else
    {
        return std::format("{}s", secs.count());
    }
}

}  // namespace time

// ============================================================================
// Stopwatch - High precision timer
// ============================================================================

class Stopwatch
{
public:
    Stopwatch() = default;

    // Start the stopwatch
    void start()
    {
        if (!m_running)
        {
            m_start = time::now();
            m_running = true;
        }
    }

    // Stop the stopwatch
    void stop()
    {
        if (m_running)
        {
            m_elapsed += time::now() - m_start;
            m_running = false;
        }
    }

    // Reset the stopwatch
    void reset()
    {
        m_elapsed = Duration::zero();
        m_running = false;
    }

    // Restart (reset and start)
    void restart()
    {
        reset();
        start();
    }

    // Is the stopwatch running?
    [[nodiscard]] bool isRunning() const noexcept { return m_running; }

    // Get elapsed time
    [[nodiscard]] Duration elapsed() const
    {
        if (m_running)
        {
            return m_elapsed + (time::now() - m_start);
        }
        return m_elapsed;
    }

    [[nodiscard]] i64 elapsedNanos() const { return std::chrono::duration_cast<Nanoseconds>(elapsed()).count(); }

    [[nodiscard]] i64 elapsedMicros() const { return std::chrono::duration_cast<Microseconds>(elapsed()).count(); }

    [[nodiscard]] i64 elapsedMillis() const { return std::chrono::duration_cast<Milliseconds>(elapsed()).count(); }

    [[nodiscard]] f64 elapsedSeconds() const { return std::chrono::duration<f64>(elapsed()).count(); }

    // Static helper to create and start
    [[nodiscard]] static Stopwatch startNew()
    {
        Stopwatch sw;
        sw.start();
        return sw;
    }

private:
    TimePoint m_start{};
    Duration m_elapsed = Duration::zero();
    bool m_running = false;
};

// ============================================================================
// ScopedTimer - Measures scope execution time
// ============================================================================

class ScopedTimer
{
public:
    using Callback = std::function<void(i64 elapsedMicros)>;

    explicit ScopedTimer(Callback callback) : m_callback(std::move(callback)), m_start(time::now()) {}

    ~ScopedTimer()
    {
        auto elapsed = std::chrono::duration_cast<Microseconds>(time::now() - m_start).count();
        if (m_callback)
        {
            m_callback(elapsed);
        }
    }

    DAKT_NON_COPYABLE_NON_MOVABLE(ScopedTimer);

private:
    Callback m_callback;
    TimePoint m_start;
};

// Helper macro for scope timing
#define DAKT_TIMED_SCOPE(name)                           \
    ::dakt::core::ScopedTimer DAKT_UNIQUE_NAME(_timer_)( \
        [](i64 us) { std::printf("%s: %lld us\n", name, static_cast<long long>(us)); })

// ============================================================================
// Timer - Periodic timer
// ============================================================================

class Timer
{
public:
    Timer() = default;
    explicit Timer(f64 intervalSeconds) : m_interval(intervalSeconds) {}

    // Set interval
    void setInterval(f64 seconds) { m_interval = seconds; }
    [[nodiscard]] f64 interval() const noexcept { return m_interval; }

    // Start the timer
    void start() { m_lastTick = time::now(); }

    // Reset (restart with same interval)
    void reset() { m_lastTick = time::now(); }

    // Check if timer has elapsed (and reset if so)
    [[nodiscard]] bool elapsed()
    {
        if (check())
        {
            m_lastTick = time::now();
            return true;
        }
        return false;
    }

    // Check if timer has elapsed without resetting
    [[nodiscard]] bool check() const
    {
        auto now = time::now();
        auto elapsed = std::chrono::duration<f64>(now - m_lastTick).count();
        return elapsed >= m_interval;
    }

    // Get time remaining until next tick
    [[nodiscard]] f64 remaining() const
    {
        auto now = time::now();
        auto elapsed = std::chrono::duration<f64>(now - m_lastTick).count();
        return std::max(0.0, m_interval - elapsed);
    }

    // Get progress (0.0 to 1.0)
    [[nodiscard]] f64 progress() const
    {
        if (m_interval <= 0.0)
            return 1.0;
        auto now = time::now();
        auto elapsed = std::chrono::duration<f64>(now - m_lastTick).count();
        return std::min(1.0, elapsed / m_interval);
    }

private:
    f64 m_interval = 1.0;
    TimePoint m_lastTick{};
};

// ============================================================================
// FrameTimer - For game loops and frame timing
// ============================================================================

class FrameTimer
{
public:
    FrameTimer() : m_lastFrame(time::now()) {}

    // Call once per frame
    void tick()
    {
        auto now = time::now();
        m_deltaTime = std::chrono::duration<f64>(now - m_lastFrame).count();
        m_lastFrame = now;
        m_totalTime += m_deltaTime;
        ++m_frameCount;

        // Calculate FPS
        m_fps = (m_deltaTime > 0.0) ? (1.0 / m_deltaTime) : 0.0;

        // Update average FPS
        m_fpsSamples[m_fpsSampleIndex] = m_fps;
        m_fpsSampleIndex = (m_fpsSampleIndex + 1) % FPS_SAMPLE_COUNT;

        f64 sum = 0.0;
        for (auto sample : m_fpsSamples)
        {
            sum += sample;
        }
        m_avgFps = sum / static_cast<f64>(FPS_SAMPLE_COUNT);
    }

    // Get delta time (time since last frame)
    [[nodiscard]] f64 deltaTime() const noexcept { return m_deltaTime; }
    [[nodiscard]] f32 deltaTimeF() const noexcept { return static_cast<f32>(m_deltaTime); }

    // Get frames per second
    [[nodiscard]] f64 fps() const noexcept { return m_fps; }

    // Get average fps over last N frames
    [[nodiscard]] f64 averageFps() const noexcept { return m_avgFps; }

    // Get total elapsed time
    [[nodiscard]] f64 totalTime() const noexcept { return m_totalTime; }

    // Get frame count
    [[nodiscard]] u64 frameCount() const noexcept { return m_frameCount; }

private:
    static constexpr usize FPS_SAMPLE_COUNT = 60;

    TimePoint m_lastFrame;
    f64 m_deltaTime = 0.0;
    f64 m_totalTime = 0.0;
    f64 m_fps = 0.0;
    f64 m_avgFps = 0.0;
    u64 m_frameCount = 0;

    f64 m_fpsSamples[FPS_SAMPLE_COUNT] = {};
    usize m_fpsSampleIndex = 0;
};

}  // namespace dakt::core