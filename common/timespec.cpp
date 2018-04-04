#include "timespec.h"

TimeSpec::TimeSpec() :
    days{0},
    hr{0},
    min{0},
    secs{0},
    msecs{0},
    usecs{0}
{

}

TimeSpec::~TimeSpec()
{

}

int TimeSpec::getElapsedTime(uint64 usecs)
{
    uint64_t div;
    uint64_t rem;
    // Check: if msecs < 1000 -> just fill msecs
    if (usecs < 1000)
    {
        usecs = usecs;
        return 0;
    }
    // First, compute nr milliseconds out of these usecs
    div = usecs / 1000;
    rem = usecs % 1000;
    if (div < 1000)
    {
        usecs = rem;
        msecs = div;
        return 0;
    }
    usecs = rem;
    // Next, compute seconds out of these msecs
    uint64_t seconds = div / 1000;
    rem = div % 1000;
    if (seconds < 60)
    {
        msecs = rem;
        secs = seconds;
        return 0;
    }
    msecs = rem;
    // Next, compute minutes out of these seconds
    uint64_t minutes = seconds / 60;
    rem = seconds % 60;
    if (minutes < 60)
    {
        secs = rem;
        min = minutes;
        return 0;
    }
    secs = rem;
    // Next, compute hours out of these minutes
    uint64_t hours = minutes / 60;
    rem = minutes % 60;
    if (hours < 60)
    {
        min = rem;
        hr = hours;
        return 0;
    }
    min = rem;
    // Next, compute days out of these hours
    uint64_t days = hours / 24;
    rem = hours % 24;
    hr = rem;
    days = days;

    return 0;
}

QString TimeSpec::toString()
{
    return QString::number(days) + " days, " +
            QString::number(hr) + "h, " +
            QString::number(min) +  "m, " +
            QString::number(secs) + "s, " +
            QString::number(msecs) + "ms";
}
