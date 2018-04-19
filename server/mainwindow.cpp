// Qt global includes
#include <QDBusConnection>
#include <QMovie>

// System includes
#include <iostream>
#include <pthread.h>
#include <unistd.h>

// Local includes
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "defs.h"

// D-Bus includes
#include "openzwave_adaptor.h"

// OpenZWave includes
#include "Options.h"
#include "Manager.h"
#include "Driver.h"
#include "Node.h"
#include "Group.h"
#include "value_classes/ValueStore.h"
#include "value_classes/Value.h"
#include "value_classes/ValueBool.h"
#include "platform/Log.h"
#include "command_classes/SwitchBinary.h"
#include "command_classes/SwitchMultilevel.h"
#include "defs.h"

// Bring on OpenZwave namespace
using namespace OpenZWave;

using std::cout;
using std::endl;

static uint32 g_homeId = 0;
static bool   g_initSuccess = false;
static bool   silentMode = false;

// List holding all currently discovered nodes
QList<NodeInfo*> g_nodes;

bool temp = false;

// Mutex and c.v. to avoid race conditions
static pthread_mutex_t g_criticalSection;
static pthread_cond_t  initCond  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t initMutex = PTHREAD_MUTEX_INITIALIZER;

MainWindow * MainWindow::instance { NULL };

NodeInfo *  GetNodeInfo     ( const OpenZWave::Notification * _notification );
void        OnNotification  ( const OpenZWave::Notification * _notification,
                              void * _context);
QString     get_elapsed_time(uint64 usecs, timespec_t * s);

void OpenZWaveBackgroundThread::run()

{
    pthread_mutexattr_t mutexattr;
    bool create_success {false};

    // Init the mutex and c.v. objects
    pthread_mutexattr_init ( &mutexattr );
    pthread_mutexattr_settype( &mutexattr, PTHREAD_MUTEX_RECURSIVE );
    pthread_mutex_init( &g_criticalSection, &mutexattr );
    pthread_mutexattr_destroy( &mutexattr );

    pthread_mutex_lock( &initMutex );

    // Create the OpenZWave Manager.
    Options::Create(conf.toStdString(), LOG_DIR, "", create_success);
    if (!create_success)
    {
        Log::Write(LogLevel_Error,
                   "Unable to find the configuration files neither at '%s', '%s' or at config/, exiting...",
                   conf.toStdString().c_str(),
                   SYSCONFDIR);
        Options::Destroy();
        pthread_mutex_destroy( &g_criticalSection );
        QCoreApplication::quit();
    }
    else
    {
        Options::Get()->AddOptionInt( "SaveLogLevel", LogLevel_Detail );
        Options::Get()->AddOptionInt( "QueueLogLevel", LogLevel_Debug );
        Options::Get()->AddOptionInt( "DumpTrigger", LogLevel_Error );
        Options::Get()->AddOptionInt( "PollInterval", 500 );
        Options::Get()->AddOptionBool( "IntervalBetweenPolls", true );
        Options::Get()->AddOptionBool("ValidateValueChanges", true);
        Options::Get()->AddOptionBool("ConsoleOutput", verbose);
        Options::Get()->Lock();

        Manager::Create();

        // Add a callback handler to the manager.  The second argument is a context that
        // is passed to the OnNotification method.  If the OnNotification is a method of
        // a class, the context would usually be a pointer to that class object, to
        // avoid the need for the notification handler to be a static.
        Manager::Get()->AddWatcher( OnNotification, NULL );

        // Add a Z-Wave Driver
        Manager::Get()->AddDriver( tty.toStdString() );

        // Now we just wait for either the AwakeNodesQueried or AllNodesQueried notification,
        // then write out the config file.
        // In a normal app, we would be handling notifications and building a UI for the user.
        pthread_cond_wait( &initCond, &initMutex );
    }
    emit resultReady(g_initSuccess);
}

bool MainWindow::toggleSwitchBinary(const int node_Id, bool status)
{
    bool result {true};

    // Lock critical section
    pthread_mutex_lock (&g_criticalSection);

    for (auto const & node : g_nodes)
    {
        if (node->m_nodeId == node_Id)
        {
            for (auto const & value : node->m_values)
            {
                if (value.GetCommandClassId() == SwitchBinary::StaticGetCommandClassId())
                {
                    result = Manager::Get()->SetValue(value, status);
                }
            }
        }
    }
    // Unlock critical section
    pthread_mutex_unlock (&g_criticalSection);
    return result;
}

bool MainWindow::toggleSwitchMultilevel(const int node_Id, const uint8 level)
{
    bool result {true};
    // Lock critical section
    pthread_mutex_lock (&g_criticalSection);

    for (auto const & node : g_nodes)
    {
        if (node->m_nodeId == node_Id)
        {
            for (auto const & value : node->m_values)
            {
                if (value.GetCommandClassId() == SwitchMultilevel::StaticGetCommandClassId())
                {
                    result = Manager::Get()->SetValue(value, level);
                    // Avoid repetitions ...
                    break;
                }
            }
        }
    }
    // Unlock critical section
    pthread_mutex_unlock (&g_criticalSection);
    return result;
}

bool MainWindow::getSwitchBinaryStatus(const uint nodeId, uint & status)
{
    bool result {true};
    bool currentStatus {false};

    // Lock critical section
    pthread_mutex_lock (&g_criticalSection);

    for (auto const & node : g_nodes)
    {
        if (node->m_nodeId == nodeId)
        {
            for (auto const & value : node->m_values)
            {
                if (value.GetCommandClassId() == SwitchBinary::StaticGetCommandClassId())
                {
                    result = Manager::Get()->GetValueAsBool(value, &currentStatus);
                }
            }
        }
    }
    // Unlock critical section
    pthread_mutex_unlock (&g_criticalSection);
    // Cast back to uint and update `status`
    status = static_cast<uint>(currentStatus);
    return result;
}

bool MainWindow::getSwitchMultilevelStatus(const uint nodeId, uint &status)
{
    bool result {true};
    uint8_t byteStatus {0};
    // Lock critical section
    pthread_mutex_lock (&g_criticalSection);

    for (auto const & node : g_nodes)
    {
        if (node->m_nodeId == nodeId)
        {
            for (auto const & value : node->m_values)
            {
                if (value.GetCommandClassId() == SwitchMultilevel::StaticGetCommandClassId())
                {
                    result = Manager::Get()->GetValueAsByte(value, &byteStatus);
                    // Avoid repetitions ...
                    break;
                }
            }
        }
    }
    // Unlock critical section
    pthread_mutex_unlock (&g_criticalSection);
    status = static_cast<uint>(byteStatus);
    return result;
}

MainWindow::MainWindow(bool _graphic,
                       bool _silent,
                       const QString & _ttyPort,
                       const QString & _configPath,
                       QWidget *parent) :
    graphic{_graphic},
    silent{_silent},
    ttyPort{_ttyPort},
    configPath{_configPath},
    readyToServe{false},
    dbusSuccess{true},
    QMainWindow(parent)
{
    if (graphic)
    {
        ui = new Ui::MainWindow;
        ui->setupUi(this);
    }
    appendText("Server initialized! Listening to the bus...");

    instance = this;
    silentMode = silent;

    setWindowTitle("OpenZWave daemon");

    // Register adaptor
    new OpenzwaveAdaptor(this);
    // Add D-BUs interface and connect to it
    QDBusConnection conn = QDBusConnection::systemBus();
    if (!conn.registerObject("/se/mysland/openzwave", this))
    {
        cerr << "Error registering object path: "
             << conn.lastError().message().toStdString()
             << endl;
        dbusSuccess = false;
    }
    if (!conn.registerService("se.mysland.openzwave"))
    {
        cerr << "Error registering service: "
             << conn.lastError().message().toStdString()
             << endl;
        dbusSuccess = false;
    }

    // We want to free and stop the engine when exiting the application
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(beforeExit()));

    /*
     * Initialize OpenZWave engine, but we don't want to do this in the same thread
     * that creates the GUI (OpenZWave does this internally), but we want to finish
     * the MainWindow constructor by signaling that we are ready to go.
    */
    connect(this, SIGNAL(startOZWInitialization()),
            this, SLOT(initOpenZWave()));

    if (graphic)
    {
        // Start animation
        QMovie * movie = new QMovie(":/images/imgs/progress.gif");
        movie->setScaledSize(ui->label_2->size());
        ui->label_2->setMovie(movie);
        movie->start();
    }
    // Init timer
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(broadcastNodes()));
}

MainWindow::~MainWindow()
{
    if (graphic)
        delete ui;
}

void MainWindow::acknowledgeTransferToNode(uint nodeId)
{
    emit statusChangedCfm(nodeId);
}

void MainWindow::startOZWEngine()
{
    emit startOZWInitialization();
}

void MainWindow::statusSet(uint devId, uint statusCode)
{
    appendText("Received request: STATUS(" +
               QString::number(statusCode) + ") for node ID " +
               QString::number(devId));
    if (readyToServe)
    {
        bool result { false };
        // Check if devId is a valid one
        if (!isValidNodeId(devId))
        {
            appendText("Request for non-existent NodeID \"" +
                       QString::number(devId) + "\", skipping request.");
            result = false;
            emit statusSetAck(devId, result);
            return;
        }
        if (!isValidValue(devId, statusCode))
        {
            appendText("Invalid status '" +
                       QString::number(statusCode) + "' for NodeID \"" +
                       QString::number(devId) + "\", skipping request.");
            result = false;
            emit statusSetAck(devId, result);
            return;
        }
        switch (devId)
        {
        case SWITCH_BINARY_ID:
            result = toggleSwitchBinary(SWITCH_BINARY_ID,
                                        static_cast<bool>(statusCode));
            break;
        case SWITCH_MULTILEVEL_ID:
            result = toggleSwitchMultilevel(SWITCH_MULTILEVEL_ID,
                                            static_cast<uint8>(statusCode));
            break;
        }

        // Write back response code
        emit statusSetAck(devId, result);
    }
    else
    {
        appendText("The OpenZWave engine is still initializing ...");
    }
}

void MainWindow::statusGet(uint deviceId)
{
    // Once a status request is received, we want to retrieve it from the Z-Wave controller
    bool result {false};
    uint currentStatus {0};
    switch (deviceId)
    {
    case SWITCH_BINARY_ID:
        result = getSwitchBinaryStatus(deviceId, currentStatus);
        break;
    case SWITCH_MULTILEVEL_ID:
        result = getSwitchMultilevelStatus(deviceId, currentStatus);
        break;
    default:
        appendText("Unknown device ID "
                   + QString::number(deviceId));
        break;
    }
    emit statusGetRsp(result, currentStatus);
}

void MainWindow::serverReady()
{
    appendText("Someone just queried my state. Replying...");
    emit serverReadyAck(readyToServe);
}

void MainWindow::requestNodeTransfer()
{
    if (timer->interval() < BROADCAST_TIMEOUT)
    {
        timer->start(BROADCAST_TIMEOUT);
    }
    emit publishNrNodes(g_nodes.size());
}

void MainWindow::publishNrNodesAck()
{
    // Once done this, we want to publish the nodes to the client
    for (auto const & node : g_nodes)
    {
        if (node->m_nodeId == SWITCH_BINARY_ID)
        {
            emit publishNodeDetails(node->m_nodeId, 0, 1);
        }
        else
        {
            if (node->m_nodeId != 1)
            {
                emit publishNodeDetails(node->m_nodeId, 0, 99);
            }
        }
    }
}

void MainWindow::publishNodeDetailsAck(uint nodeId)
{
    appendText("Node ID " +
               QString::number(nodeId) +
               " has been acknowledged.");
}

void MainWindow::initOpenZWave()
{
    appendText("Initializing OpenZWave engine ...");

    // Create instance of the worker thread class
    wt = new OpenZWaveBackgroundThread();
    wt->setData(ttyPort, configPath, !silent);

    connect(wt, &OpenZWaveBackgroundThread::resultReady,
            this, &MainWindow::initOpenZWaveDone);
    connect(wt, &OpenZWaveBackgroundThread::finished,
            wt, &QObject::deleteLater);

    // Start timer
    t.start();
    wt->start();
}

void MainWindow::initOpenZWaveDone(bool res)
{
    ts.getElapsedTime(t.elapsed() * 1e3);
    appendText("Initialization " +
               ((res) ? QString::fromStdString("done.") : QString::fromStdString("failed."))
               + " Elapsed time: " + ts.toString());
    if (graphic)
    {
        // Stop animation
        QMovie * mv = ui->label_2->movie();
        mv->stop();
        delete mv;

        QPixmap pm;
        if (res)
        {
            pm.load(":/images/imgs/check.png");
        }
        else
        {
            pm.load(":/images/imgs/warning.png");
            appendText("The OpenZWave engine failed to initialize. Nothing will happen now.");
        }
        ui->label_2->setPixmap(pm);
    }
    else
    {
        if (!res)
        {
            appendText("The OpenZWave engine failed to initialize. The application will now exit.");
            // Double-check that the working thread is not running
            if (wt->isRunning())
                wt->quit();
            qApp->exit(1);
        }
    }
    // Now we want to publish the available nodes to the client
    if (res)
    {
        broadcastNodes();
        timer->start(BROADCAST_TIMEOUT);
        readyToServe = true;
    }
}

// This will be called every BROADCAST_TIMEOUT secs to notify changes to client
void MainWindow::broadcastNodes()
{
    requestNodeTransfer();
}

void MainWindow::stopOpenZWave()
{
    // Check only if wt is active
    if (wt->isRunning())
        wt->quit();

    Manager::Get()->RemoveDriver( ttyPort.toStdString() );
    Manager::Get()->RemoveWatcher( OnNotification, NULL );
    Manager::Destroy();

    Options::Destroy();
    pthread_mutex_destroy( &g_criticalSection );
}

void MainWindow::beforeExit()
{
    appendText("Exiting...");
    stopOpenZWave();
    QCoreApplication::quit();
}

bool MainWindow::isValidNodeId(uint devId)
{
    for (auto const & node : g_nodes)
    {
        if (node->m_nodeId == static_cast<uint32>(devId))
            return true;
    }
    return false;
}

bool MainWindow::isValidValue(uint devId, uint val)
{
    // TODO: check this via OpenZWave
    switch (devId)
    {
    case SWITCH_BINARY_ID:
        return val < 2; /* 0 or 1 */
    case SWITCH_MULTILEVEL_ID:
        return val < 100; /* 0 to 99, see page 518 of Specification */
    default:
        break;
    }
    return false;
}

void MainWindow::appendText(QString const & txt)
{
    if (!silent)
    {
        if (graphic)
        {
            ui->plainTextEdit->appendPlainText(txt);
        }
        else
        {
            cout << txt.toStdString() << endl;
        }
    }
}

void MainWindow::on_pushButton_clicked()
{
    ui->plainTextEdit->clear();
}


NodeInfo * GetNodeInfo ( const OpenZWave::Notification * _notification )
{
    uint32 const homeId = _notification->GetHomeId();
    uint8 const nodeId = _notification->GetNodeId();

    for (auto const & node : g_nodes)
    {
        if (node->m_homeId == homeId && node->m_nodeId == nodeId)
            return node;
    }

    return NULL;
}

void OnNotification ( const OpenZWave::Notification * _notification, void * _context)
{
    // Must do this inside a critical section to avoid conflicts with the main thread
    pthread_mutex_lock( &g_criticalSection );

    if (!silentMode)
        cout << "Notification! Type = " << _notification->GetAsString() << endl;

    switch( _notification->GetType() )
    {
    case Notification::Type_ValueAdded:
    {
        if( NodeInfo* nodeInfo = GetNodeInfo( _notification ) )
        {
            // Add the new value to our list
            nodeInfo->m_values.push_back( _notification->GetValueID() );
        }
        break;
    }

    case Notification::Type_ValueRemoved:
    {
        // Remove the value from out list
        if( NodeInfo* nodeInfo = GetNodeInfo( _notification ) )
        {
            for (int current = 0; current < nodeInfo->m_values.size(); ++current)
            {
                if (nodeInfo->m_values.at(current) == _notification->GetValueID())
                {
                    nodeInfo->m_values.removeAt(current);
                    break;
                }
            }
        }
        break;
    }

    case Notification::Type_ValueChanged:
    {
        if( NodeInfo* nodeInfo = GetNodeInfo( _notification ) )
        {
            nodeInfo = nodeInfo;		// placeholder for real action
        }
        break;
    }

    case Notification::Type_Group:
    {
        // One of the node's association groups has changed
        if( NodeInfo* nodeInfo = GetNodeInfo( _notification ) )
        {
            nodeInfo = nodeInfo;		// placeholder for real action
        }
        break;
    }

    case Notification::Type_NodeAdded:
    {
        // Add the new node to our list
        NodeInfo* nodeInfo = new NodeInfo();
        nodeInfo->m_homeId = _notification->GetHomeId();
        nodeInfo->m_nodeId = _notification->GetNodeId();
        nodeInfo->m_polled = false;
        g_nodes.push_back( nodeInfo );
        if (temp == true) {
            Manager::Get()->CancelControllerCommand( _notification->GetHomeId() );
        }
        break;
    }

    case Notification::Type_NodeRemoved:
    {
        // Remove the node from our list
        uint32 const homeId = _notification->GetHomeId();
        uint8 const nodeId = _notification->GetNodeId();

        for (int current = 0; current < g_nodes.size(); ++current)
        {
            NodeInfo * nodeInfo = g_nodes.at(current);
            if (nodeInfo->m_homeId == homeId && nodeInfo->m_nodeId == nodeId)
            {
                g_nodes.removeAt(current);
                delete nodeInfo;
                break;
            }
        }
        break;
    }

    case Notification::Type_NodeEvent:
    {
        // We have received an event from the node, caused by a
        // basic_set or hail message.
        if( NodeInfo* nodeInfo = GetNodeInfo( _notification ) )
        {
            nodeInfo = nodeInfo;		// placeholder for real action
        }
        break;
    }

    case Notification::Type_PollingDisabled:
    {
        if( NodeInfo* nodeInfo = GetNodeInfo( _notification ) )
        {
            nodeInfo->m_polled = false;
        }
        break;
    }

    case Notification::Type_PollingEnabled:
    {
        if( NodeInfo* nodeInfo = GetNodeInfo( _notification ) )
        {
            nodeInfo->m_polled = true;
        }
        break;
    }

    case Notification::Type_DriverReady:
    {
        g_homeId = _notification->GetHomeId();
        break;
    }

    case Notification::Type_DriverFailed:
    {
        g_initSuccess = false;
        pthread_cond_broadcast(&initCond);
        break;
    }

    case Notification::Type_AwakeNodesQueried:
    case Notification::Type_AllNodesQueried:
    case Notification::Type_AllNodesQueriedSomeDead:
    {
        /* Even if some nodes are dead, at least the
            engine has completed initialization */
        g_initSuccess = true;
        pthread_cond_broadcast(&initCond);
        break;
    }

    case Notification::Type_DriverReset:
    case Notification::Type_Notification:
    {
        if (!silentMode)
            cout << "Type Notification!! Byte is: " << static_cast<int>(_notification->GetByte()) << endl;
        switch (_notification->GetByte())
        {
        case Notification::NotificationCode::Code_MsgComplete:
            if (!silentMode)
                cout << "Message complete for node " << static_cast<int>(_notification->GetNodeId())
                     << "! Sending ACK over the bus..." << endl;
            MainWindow * w = MainWindow::Get();
            w->acknowledgeTransferToNode(_notification->GetNodeId());
            break;
        }
    }
    case Notification::Type_NodeNaming:
    case Notification::Type_NodeProtocolInfo:
    case Notification::Type_NodeQueriesComplete:
    default:
    {
    }
    }

    pthread_mutex_unlock( &g_criticalSection );
}
