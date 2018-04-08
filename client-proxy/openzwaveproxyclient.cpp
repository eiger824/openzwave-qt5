#include <iostream>

#include <QDBusConnection>
#include <QDBusMessage>

// D-Bus includes
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
    iface = new se::mysland::openzwave(QString{"se.mysland.openzwave"},
                                       QString{"/se/mysland/openzwave"},
                                       QDBusConnection::systemBus(),
                                       this);

    connect(iface, SIGNAL(statusSetAck(uint,bool)),
            this, SLOT(statusSetAckSlot(uint,bool)));
    connect(iface, SIGNAL(serverReadyAck(bool)),
            this, SLOT(serverReadyAckSlot(bool)));
    connect(iface, SIGNAL(statusChangedCfm(uint)),
            this, SLOT(statusChangedCfmSlot(uint)));

    if (!iface->isValid())
    {
        cerr << "Error: daemon doesn't seem to be running."
             << endl;
        success = false;
    }
    else
    {
        success = true;
        // Ask the server if it is ready to accept commands
        iface->serverReady();
    }
}

void OpenZWaveProxyClient::serverReadyAckSlot(bool res)
{
    if (!res)
    {
        cerr << "Error: daemon is still initializing." << endl;
        qApp->exit(1);
        return;
    }
    iface->statusSet(nodeId, value);
}

void OpenZWaveProxyClient::statusSetAckSlot(uint nodeId, bool res)
{
    cerr << "Received result: " << res << endl;
    // Simply exit application with exit code in `!res`
    if (!res)
    {
        qApp->exit(1); // Exit with error if the operation was unsuccessful
    }
}

void OpenZWaveProxyClient::statusChangedCfmSlot(uint nodeId)
{
    cout << "Status was changed! Exiting program" << endl;
    qApp->exit(0);
}
