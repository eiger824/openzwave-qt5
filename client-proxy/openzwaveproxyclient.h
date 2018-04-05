#ifndef OPENZWAVEPROXYCLIENT_H
#define OPENZWAVEPROXYCLIENT_H

#include <QObject>

#include "defs.h"

class OpenZWaveProxyClient : public QObject
{
    Q_OBJECT
public:
    explicit OpenZWaveProxyClient(uint _nodeId, uint _value,
                                  QObject *parent = nullptr);

public slots:
    void serverReadyAckSlot(bool res);

private:
    uint nodeId;
    uint value;
};

#endif // OPENZWAVEPROXYCLIENT_H
