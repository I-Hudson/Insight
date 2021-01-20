// Copyright (c) 2012-2021 Wojciech Figat. All rights reserved.

#include "ispch.h"
#include "Core/Utils.h"
#include "TimeSpan.h"

TimeSpan TimeSpan::FromDays(double days)
{
    ASSERT_LOW_LAYER((days >= MinValue().GetTotalDays()) && (days <= MaxValue().GetTotalDays()));
    return TimeSpan(static_cast<I64>(days * Constants::TicksPerDay));
}

TimeSpan TimeSpan::FromHours(double hours)
{
    ASSERT_LOW_LAYER((hours >= MinValue().GetTotalHours()) && (hours <= MaxValue().GetTotalHours()));
    return TimeSpan(static_cast<I64>(hours * Constants::TicksPerHour));
}

TimeSpan TimeSpan::FromMilliseconds(double milliseconds)
{
    ASSERT_LOW_LAYER((milliseconds >= MinValue().GetTotalMilliseconds()) && (milliseconds <= MaxValue().GetTotalMilliseconds()));
    return TimeSpan(static_cast<I64>(milliseconds * Constants::TicksPerMillisecond));
}

TimeSpan TimeSpan::FromMinutes(double minutes)
{
    ASSERT_LOW_LAYER((minutes >= MinValue().GetTotalMinutes()) && (minutes <= MaxValue().GetTotalMinutes()));
    return TimeSpan(static_cast<I64>(minutes * Constants::TicksPerMinute));
}

TimeSpan TimeSpan::FromSeconds(double seconds)
{
    ASSERT_LOW_LAYER((seconds >= MinValue().GetTotalSeconds()) && (seconds <= MaxValue().GetTotalSeconds()));
    return TimeSpan(static_cast<I64>(seconds * Constants::TicksPerSecond));
}

void TimeSpan::Set(I32 days, I32 hours, I32 minutes, I32 seconds, I32 milliseconds)
{
    const I64 totalMs = 1000 * (60 * 60 * 24 * (I64)days + 60 * 60 * (I64)hours + 60 * (I64)minutes + (I64)seconds) + (I64)milliseconds;
    ASSERT_LOW_LAYER((totalMs >= MinValue().GetTotalMilliseconds()) && (totalMs <= MaxValue().GetTotalMilliseconds()));
    Ticks = totalMs * Constants::TicksPerMillisecond;
}

std::string TimeSpan::ToString() const
{
    return StringFormat("{}", *this);
}

std::string TimeSpan::ToString(const char option) const
{
    switch (option)
    {
    case 'a':
        return StringFormat("{:0>2}:{:0>2}:{:0>2}.{:0>3}", GetHours(), GetMinutes(), GetSeconds(), GetMilliseconds());
    default:
        return StringFormat("{:0>2}:{:0>2}:{:0>2}.{:0>7}", GetHours(), GetMinutes(), GetSeconds(), Ticks % 10000000);
    }
}
