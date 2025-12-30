// ============================================================================
// DaktLib Core - Time Implementation
// ============================================================================

#include <dakt/core/Time.hpp>

#include <ctime>

#if defined(DAKT_PLATFORM_WINDOWS)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <Windows.h>
#else
    #include <sys/time.h>
#endif

namespace dakt::core
{

// ============================================================================
// Timestamp Validation
// ============================================================================

bool time::Timestamp::isValid() const noexcept
{
    return year >= 1970 && year <= 9999 && month >= 1 && month <= 12 && day >= 1 && day <= 31 && hour <= 23 &&
           minute <= 59 && second <= 59 && millisecond <= 999 && dayOfWeek <= 6 && dayOfYear >= 1 && dayOfYear <= 366;
}

// ============================================================================
// Time Functions
// ============================================================================

namespace
{

time::Timestamp tmToTimestamp(const std::tm& tm, u32 ms = 0)
{
    time::Timestamp ts{};
    ts.year = tm.tm_year + 1900;
    ts.month = static_cast<u32>(tm.tm_mon + 1);
    ts.day = static_cast<u32>(tm.tm_mday);
    ts.hour = static_cast<u32>(tm.tm_hour);
    ts.minute = static_cast<u32>(tm.tm_min);
    ts.second = static_cast<u32>(tm.tm_sec);
    ts.millisecond = ms;
    ts.dayOfWeek = static_cast<u32>(tm.tm_wday);
    ts.dayOfYear = static_cast<u32>(tm.tm_yday + 1);
    return ts;
}

}  // namespace

namespace time
{

Timestamp localTime()
{
    auto now = SystemClock::now();
    auto t = SystemClock::to_time_t(now);
    auto ms = std::chrono::duration_cast<Milliseconds>(now.time_since_epoch()) % 1000;

    std::tm tm{};
#if defined(DAKT_PLATFORM_WINDOWS)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    return tmToTimestamp(tm, static_cast<u32>(ms.count()));
}

Timestamp utcTime()
{
    auto now = SystemClock::now();
    auto t = SystemClock::to_time_t(now);
    auto ms = std::chrono::duration_cast<Milliseconds>(now.time_since_epoch()) % 1000;

    std::tm tm{};
#if defined(DAKT_PLATFORM_WINDOWS)
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif

    return tmToTimestamp(tm, static_cast<u32>(ms.count()));
}

i64 unixTimestamp()
{
    return std::chrono::duration_cast<Seconds>(SystemClock::now().time_since_epoch()).count();
}

i64 unixTimestampMs()
{
    return std::chrono::duration_cast<Milliseconds>(SystemClock::now().time_since_epoch()).count();
}

i64 toUnixTimestamp(const Timestamp& ts)
{
    std::tm tm{};
    tm.tm_year = ts.year - 1900;
    tm.tm_mon = static_cast<int>(ts.month) - 1;
    tm.tm_mday = static_cast<int>(ts.day);
    tm.tm_hour = static_cast<int>(ts.hour);
    tm.tm_min = static_cast<int>(ts.minute);
    tm.tm_sec = static_cast<int>(ts.second);
    tm.tm_isdst = -1;

#if defined(DAKT_PLATFORM_WINDOWS)
    auto t = _mkgmtime(&tm);
#else
    auto t = timegm(&tm);
#endif

    return static_cast<i64>(t);
}

Timestamp fromUnixTimestamp(i64 unixTs)
{
    auto t = static_cast<std::time_t>(unixTs);
    std::tm tm{};

#if defined(DAKT_PLATFORM_WINDOWS)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    return tmToTimestamp(tm);
}

Timestamp fromUnixTimestampUtc(i64 unixTs)
{
    auto t = static_cast<std::time_t>(unixTs);
    std::tm tm{};

#if defined(DAKT_PLATFORM_WINDOWS)
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif

    return tmToTimestamp(tm);
}

// ============================================================================
// Time Formatting
// ============================================================================

String formatTimestamp(const Timestamp& ts, StringView /*format*/)
{
    // TODO: Parse format string properly
    return std::format("{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}", ts.year, ts.month, ts.day, ts.hour, ts.minute,
                       ts.second);
}

String formatISO8601(const Timestamp& ts)
{
    return std::format("{:04d}-{:02d}-{:02d}T{:02d}:{:02d}:{:02d}", ts.year, ts.month, ts.day, ts.hour, ts.minute,
                       ts.second);
}

String formatDate(const Timestamp& ts)
{
    return std::format("{:04d}-{:02d}-{:02d}", ts.year, ts.month, ts.day);
}

String formatTime(const Timestamp& ts)
{
    return std::format("{:02d}:{:02d}:{:02d}", ts.hour, ts.minute, ts.second);
}

String formatDateTime(const Timestamp& ts)
{
    return std::format("{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}", ts.year, ts.month, ts.day, ts.hour, ts.minute,
                       ts.second);
}

}  // namespace time

}  // namespace dakt::core
