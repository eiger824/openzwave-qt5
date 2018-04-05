#include <iostream>

#include <QDBusConnection>
#include <QDBusMessage>

// D-Bus includes
#include "openzwave_adaptor.h"
#include "openzwave_interface.h"

// Local includes
#include "openzwaveproxyclient.h"

using std::cout;
using std::cerr;
using std::endl;

OpenZWaveProxyClient::OpenZWaveProxyClient(uint _nodeId, uint _value,
                                           QObject *parent) :
    nodeId{_nodeId},
    value{_value},
    QObject(parent)
{
    QDBusConnection conn = QDBusConnection::sessionBus();

    new OpenzwaveAdaptor(this);
    conn.registerObject("/", this);
    se::mysland::openzwave * iface;
    iface = new se::mysland::openzwave(QString{},
                                       QString{},
                                       conn,
                                       this);
    connect(iface, SIGNAL(serverReadyAck(bool)),
            this, SLOT(serverReadyAckSlot(bool)));

    // Ask the server if it is ready to accept commands
    QDBusMessage msg = QDBusMessage::createSignal("/",
                                                  "se.mysland.openzwave",
                                                  "serverReady");
    conn.send(msg);
    qApp->exit(0);
}

void OpenZWaveProxyClient::serverReadyAckSlot(bool res)
{
    if (!res)
    {
        cerr << "Error: daemon is still initializing." << endl;
        qApp->exit(1);
        return;
    }
    QDBusConnection conn = QDBusConnection::sessionBus();
    QDBusMessage msg = QDBusMessage::createSignal("/",
                                                  "se.mysland.openzwave",
                                                  "statusSet");
    msg << nodeId << value;
    conn.send(msg);
    // TODO: wait for an answer/ack?
    qApp->exit(0);
    return;
}
