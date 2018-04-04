#ifndef TIMESPEC_H
#define TIMESPEC_H

#include <QString>

#include "defs.h"

typedef struct
{

} timespec_t;

class TimeSpec
{
public:
    TimeSpec();
    ~TimeSpec();
    int getElapsedTime(uint64 usecs);
    QString toString();

private:
    uint32         days;
    uint32         hr;
    uint32         min;
    uint32         secs;
    uint32         msecs;
    uint32         usecs;
};

#endif // TIMESPEC_H
