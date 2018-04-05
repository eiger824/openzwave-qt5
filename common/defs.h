#ifndef DEFS_H_
#define DEFS_H_

#include <list>
#include <QList>
#include "value_classes/ValueID.h"

#define     SWITCH_BINARY_ID        21
#define     SWITCH_MULTILEVEL_ID    15
#define     LOG_DIR                 "logs"
#define     SYSCONFDIR              "/etc/openzwave"
#define     BROADCAST_TIMEOUT       30000 /* Every 30 seconds */

using uint64 = unsigned long long;
using uint32 = unsigned int;
using uint8  = unsigned char;

typedef struct
{
    uint32			m_homeId;
    uint8			m_nodeId;
    bool			m_polled;
    QList<OpenZWave::ValueID>	m_values;
} NodeInfo;

class NodeDetails
{
public:
    NodeDetails(uint _nodeId, uint _minVal, uint _maxVal) :
        nodeId{_nodeId},
        minVal{_minVal},
        maxVal{_maxVal} {}
public:
    uint nodeId;
    uint minVal;
    uint maxVal;
};


#endif // DEFS_H_
