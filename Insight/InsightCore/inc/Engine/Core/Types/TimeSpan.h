// Copyright (c) 2012-2021 Wojciech Figat. All rights reserved.

#pragma once

#include "Engine/Core/Common.h"
#include "Engine/Core/Templates.h"

namespace Constants
{
    // The number of timespan ticks per day.
    const I64 TicksPerDay = 864000000000;

    // The number of timespan ticks per hour.
    const I64 TicksPerHour = 36000000000;

    // The number of timespan ticks per millisecond.
    const I64 TicksPerMillisecond = 10000;

    // The number of timespan ticks per minute.
    const I64 TicksPerMinute = 600000000;

    // The number of timespan ticks per second.
    const I64 TicksPerSecond = 10000000;

    // The number of timespan ticks per week.
    const I64 TicksPerWeek = 6048000000000;
}

/// <summary>
/// Represents the difference between two dates and times.
/// </summary>
struct IS_API TimeSpan
{
public:

    /// <summary>
    /// Time span in 100 nanoseconds resolution.
    /// </summary>
    I64 Ticks;

public:

    /// <summary>
    /// Empty constructor.
    /// </summary>
    TimeSpan()
    {
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="TimeSpan"/> struct.
    /// </summary>
    /// <param name="ticks">The ticks in 100 nanoseconds resolution.</param>
    TimeSpan(I64 ticks)
        : Ticks(ticks)
    {
    }

    // Init
    // @param Days Amount of days 
    // @param Hours Amount of hours
    // @param Minutes Amount of minutes
    TimeSpan(I32 days, I32 hours, I32 minutes)
    {
        Set(days, hours, minutes, 0, 0);
    }

    // Init
    // @param Days Amount of days 
    // @param Hours Amount of hours
    // @param Minutes Amount of minutes
    // @param Seconds Amount of seconds
    TimeSpan(I32 days, I32 hours, I32 minutes, I32 seconds)
    {
        Set(days, hours, minutes, seconds, 0);
    }

    // Init
    // @param Days Amount of days 
    // @param Hours Amount of hours
    // @param Minutes Amount of minutes
    // @param Seconds Amount of seconds
    // @param Milliseconds Amount of milliseconds
    TimeSpan(I32 days, I32 hours, I32 minutes, I32 seconds, I32 milliseconds)
    {
        Set(days, hours, minutes, seconds, milliseconds);
    }

public:

    // Get string
    std::string ToString() const;

    // Get string
    // @param option Custom formatting. Possible values:
    // a: 11:54:22.097
    // default: 11:54:22.0972244
    std::string ToString(const char option) const;

public:

    FORCE_INLINE TimeSpan operator+(const TimeSpan& other) const
    {
        return TimeSpan(Ticks + other.Ticks);
    }

    FORCE_INLINE TimeSpan& operator+=(const TimeSpan& other)
    {
        Ticks += other.Ticks;
        return *this;
    }

    FORCE_INLINE TimeSpan operator-() const
    {
        return TimeSpan(-Ticks);
    }

    FORCE_INLINE TimeSpan operator-(const TimeSpan& other) const
    {
        return TimeSpan(Ticks - other.Ticks);
    }

    FORCE_INLINE TimeSpan& operator-=(const TimeSpan& other)
    {
        Ticks -= other.Ticks;
        return *this;
    }

    FORCE_INLINE TimeSpan operator*(float scalar) const
    {
        return TimeSpan((I64)((float)Ticks * scalar));
    }

    FORCE_INLINE TimeSpan& operator*=(float scalar)
    {
        Ticks = (I64)((float)Ticks * scalar);
        return *this;
    }

    FORCE_INLINE bool operator==(const TimeSpan& other) const
    {
        return Ticks == other.Ticks;
    }

    FORCE_INLINE bool operator!=(const TimeSpan& other) const
    {
        return Ticks != other.Ticks;
    }

    FORCE_INLINE bool operator>(const TimeSpan& other) const
    {
        return Ticks > other.Ticks;
    }

    FORCE_INLINE bool operator>=(const TimeSpan& other) const
    {
        return Ticks >= other.Ticks;
    }

    FORCE_INLINE bool operator<(const TimeSpan& other) const
    {
        return Ticks < other.Ticks;
    }

    FORCE_INLINE bool operator<=(const TimeSpan& other) const
    {
        return Ticks <= other.Ticks;
    }

public:

    /// <summary>
    /// Gets the days component of this time span.
    /// </summary>
    FORCE_INLINE I32 GetDays() const
    {
        return (I32)(Ticks / Constants::TicksPerDay);
    }

    /// <summary>
    /// Returns a time span with the absolute value of this time span.
    /// </summary>
    FORCE_INLINE TimeSpan GetDuration() const
    {
        return TimeSpan(Ticks >= 0 ? Ticks : -Ticks);
    }

    /// <summary>
    /// Gets the hours component of this time span.
    /// </summary>
    FORCE_INLINE I32 GetHours() const
    {
        return (I32)(Ticks / Constants::TicksPerHour % 24);
    }

    /// <summary>
    /// Gets the milliseconds component of this time span.
    /// </summary>
    FORCE_INLINE I32 GetMilliseconds() const
    {
        return (I32)(Ticks / Constants::TicksPerMillisecond % 1000);
    }

    /// <summary>
    /// Gets the minutes component of this time span.
    /// </summary>
    FORCE_INLINE I32 GetMinutes() const
    {
        return (I32)(Ticks / Constants::TicksPerMinute % 60);
    }

    /// <summary>
    /// Gets the seconds component of this time span.
    /// </summary>
    FORCE_INLINE I32 GetSeconds() const
    {
        return (I32)(Ticks / Constants::TicksPerSecond % 60);
    }

    /// <summary>
    /// Gets the total number of days represented by this time span.
    /// </summary>
    FORCE_INLINE double GetTotalDays() const
    {
        return (double)Ticks / Constants::TicksPerDay;
    }

    /// <summary>
    /// Gets the total number of hours represented by this time span.
    /// </summary>
    FORCE_INLINE double GetTotalHours() const
    {
        return (double)Ticks / Constants::TicksPerHour;
    }

    /// <summary>
    /// Gets the total number of milliseconds represented by this time span.
    /// </summary>
    FORCE_INLINE double GetTotalMilliseconds() const
    {
        return (double)Ticks / Constants::TicksPerMillisecond;
    }

    /// <summary>
    /// Gets the total number of minutes represented by this time span.
    /// </summary>
    FORCE_INLINE double GetTotalMinutes() const
    {
        return (double)Ticks / Constants::TicksPerMinute;
    }

    /// <summary>
    /// Gets the total number of seconds represented by this time span
    /// </summary>
    FORCE_INLINE float GetTotalSeconds() const
    {
        return static_cast<float>(Ticks) / Constants::TicksPerSecond;
    }

public:

    /// <summary>
    /// Creates a time span that represents the specified number of days.
    /// </summary>
    /// <param name="days">The number of days.</param>
    /// <returns>The time span.</returns>
    static TimeSpan FromDays(double days);

    /// <summary>
    /// Creates a time span that represents the specified number of hours.
    /// </summary>
    /// <param name="hours">The number of hours.</param>
    /// <returns>The time span.</returns>
    static TimeSpan FromHours(double hours);

    /// <summary>
    /// Creates a time span that represents the specified number of milliseconds.
    /// </summary>
    /// <param name="milliseconds">The number of milliseconds.</param>
    /// <returns>The time span.</returns>
    static TimeSpan FromMilliseconds(double milliseconds);

    /// <summary>
    /// Creates a time span that represents the specified number of minutes.
    /// </summary>
    /// <param name="minutes">The number of minutes.</param>
    /// <returns>The time span.</returns>
    static TimeSpan FromMinutes(double minutes);

    /// <summary>
    /// Creates a time span that represents the specified number of seconds.
    /// </summary>
    /// <param name="seconds">The number of seconds.</param>
    /// <returns>The time span.</returns>
    static TimeSpan FromSeconds(double seconds);

public:

    /// <summary>
    /// Returns the maximum time span value.
    /// </summary>
    /// <returns>The time span.</returns>
    static TimeSpan MaxValue()
    {
        return TimeSpan(9223372036854775807);
    }

    /// <summary>
    /// Returns the minimum time span value.
    /// </summary>
    /// <returns>The time span.</returns>
    static TimeSpan MinValue()
    {
        return TimeSpan(-9223372036854775807 - 1);
    }

    /// <summary>
    /// Returns the zero time span value.
    /// </summary>
    /// <returns>The time span.</returns>
    static TimeSpan Zero()
    {
        return TimeSpan(0);
    }

private:

    void Set(I32 days, I32 hours, I32 minutes, I32 seconds, I32 milliseconds);
};

inline TimeSpan operator*(float scalar, const TimeSpan& timespan)
{
    return timespan.operator*(scalar);
}

template<>
struct TIsPODType<TimeSpan>
{
    enum { Value = true };
};