// Copyright (c) 2012-2021 Wojciech Figat. All rights reserved.

#include "ispch.h"
#include "DateTime.h"
#include "TimeSpan.h"
#include "Engine/Platform/Platform.h"

const I32 DateTime::CachedDaysPerMonth[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
const I32 DateTime::CachedDaysToMonth[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };

DateTime::DateTime(I32 year, I32 month, I32 day, I32 hour, I32 minute, I32 second, I32 millisecond)
{
    //ASSERT_LOW_LAYER(Validate(year, month, day, hour, minute, second, millisecond));
    I32 totalDays = 0;
    if (month > 2 && IsLeapYear(year))
        totalDays++;
    year--;
    month--;
    totalDays += year * 365 + year / 4 - year / 100 + year / 400 + CachedDaysToMonth[month] + day - 1;
    Ticks = totalDays * Constants::TicksPerDay
            + hour * Constants::TicksPerHour
            + minute * Constants::TicksPerMinute
            + second * Constants::TicksPerSecond
            + millisecond * Constants::TicksPerMillisecond;
}

void DateTime::GetDate(I32& year, I32& month, I32& day) const
{
    // Based on:
    // Fliegel, H. F. and van Flandern, T. C.,
    // Communications of the ACM, Vol. 11, No. 10 (October 1968).

    I32 l = static_cast<int>(std::floor(static_cast<float>(GetJulianDay() + 0.5))) + 68569;
    const I32 n = 4 * l / 146097;
    l = l - (146097 * n + 3) / 4;
    I32 i = 4000 * (l + 1) / 1461001;
    l = l - 1461 * i / 4 + 31;
    I32 j = 80 * l / 2447;
    const I32 k = l - 2447 * j / 80;
    l = j / 11;
    j = j + 2 - 12 * l;
    i = 100 * (n - 49) + i + l;

    year = i;
    month = j;
    day = k;
}

I32 DateTime::GetDay() const
{
    I32 year, month, day;
    GetDate(year, month, day);
    return day;
}

DayOfWeek DateTime::GetDayOfWeek() const
{
    return static_cast<DayOfWeek>((Ticks / Constants::TicksPerDay) % 7);
}

I32 DateTime::GetDayOfYear() const
{
    I32 year, month, day;
    GetDate(year, month, day);
    for (I32 i = 1; i < month; i++)
        day += DaysInMonth(year, i);
    return day;
}

I32 DateTime::GetHour12() const
{
    const I32 hour = GetHour();
    if (hour < 1)
        return 12;
    if (hour > 12)
        return hour - 12;
    return hour;
}

I32 DateTime::GetMonth() const
{
    I32 year, month, day;
    GetDate(year, month, day);
    return month;
}

I32 DateTime::GetYear() const
{
    I32 year, month, day;
    GetDate(year, month, day);
    return year;
}

I32 DateTime::DaysInMonth(I32 year, I32 month)
{
    //ASSERT_LOW_LAYER((month >= 1) && (month <= 12));
    if (month == 2 && IsLeapYear(year))
        return 29;
    return CachedDaysPerMonth[month];
}

I32 DateTime::DaysInYear(I32 year)
{
    return IsLeapYear(year) ? 366 : 365;
}

bool DateTime::IsLeapYear(I32 year)
{
    if ((year % 4) == 0)
    {
        return (((year % 100) != 0) || ((year % 400) == 0));
    }
    return false;
}

DateTime DateTime::Now()
{
    I32 year, month, day, dayOfWeek, hour, minute, second, millisecond;
    Platform::GetSystemTime(year, month, dayOfWeek, day, hour, minute, second, millisecond);
    return DateTime(year, month, day, hour, minute, second, millisecond);
}

DateTime DateTime::NowUTC()
{
    I32 year, month, day, dayOfWeek, hour, minute, second, millisecond;
    Platform::GetUTCTime(year, month, dayOfWeek, day, hour, minute, second, millisecond);
    return DateTime(year, month, day, hour, minute, second, millisecond);
}

bool DateTime::Validate(I32 year, I32 month, I32 day, I32 hour, I32 minute, I32 second, I32 millisecond)
{
    return year >= 1 && year <= 999999 && month >= 1 && month <= 12 && day >= 1 && day <= DaysInMonth(year, month) && hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59 && second >= 0 && second <= 59 && millisecond >= 0 && millisecond <= 999;
}

std::string DateTime::ToString() const
{
    return StringFormat("{}", *this);
}

std::string DateTime::ToFileNameString() const
{
    I32 year, month, day;
    GetDate(year, month, day);
    return StringFormat("{0}_{1:0>2}_{2:0>2}_{3:0>2}_{4:0>2}_{5:0>2}", year, month, day, GetHour(), GetMinute(), GetSecond());
}
