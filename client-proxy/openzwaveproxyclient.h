#ifndef OPENZWAVEPROXYCLIENT_H
#define OPENZWAVEPROXYCLIENT_H

#include <QObject>

#include "openzwave_interface.h"
#include "defs.h"

class OpenZWaveProxyClient : public QObject
{
    Q_OBJECT
public:
    explicit OpenZWaveProxyClient(uint _nodeId, uint _value,
                                  QObject *parent = nullptr);
    inline bool isInterfaceValid() const { return success; }
public slots:
    void statusSetAckSlot(uint nodeId, bool res);
    void statusChangedCfmSlot(uint nodeId);
    void serverReadyAckSlot(bool res);

private:
    uint nodeId;
    uint value;
    se::mysland::openzwave * iface;
    bool success {false};
};

#endif // OPENZWAVEPROXYCLIENT_H
