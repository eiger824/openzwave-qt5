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
#include "openzwave_interface.h"

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
#include "defs.h"

// Bring on OpenZwave namespace
using namespace OpenZWave;

using std::cout;
using std::endl;

static uint32 g_homeId = 0;
static bool   g_initFailed = false;

// List holding all currently discovered nodes
QList<NodeInfo*> g_nodes;

bool temp = false;

// Mutex and c.v. to avoid race conditions
static pthread_mutex_t g_criticalSection;
static pthread_cond_t  initCond  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t initMutex = PTHREAD_MUTEX_INITIALIZER;

NodeInfo *  GetNodeInfo     ( const OpenZWave::Notification * _notification );
void        OnNotification  ( const OpenZWave::Notification * _notification,
                              void * _context);
QString     get_elapsed_time(uint64 usecs, timespec_t * s);

void OpenZWaveBackgroundThread::run()

{
    bool result;
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
    emit resultReady(result);
}


bool MainWindow::ToggleSwitchBinary(const int node_Id, bool status)
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
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    appendText("Server initialized! Listening to the bus...");

    setWindowTitle("OpenZWave daemon");

    // Add D-BUs interface and connect to it
    QDBusConnection conn = QDBusConnection::sessionBus();

    new OpenzwaveAdaptor(this);
    conn.registerObject("/", this);
    conn.connect(QString{},                 /* Path */
                 QString{},                 /* Service */
                 "se.mysland.openzwave",    /* Interface name */
                 "statusSet",               /* Signal name */
                 this,                      /* This object, connect here */
                 SLOT(statusSetSlot(uint, uint))); /* Run slot when "statusSet" is received*/
    conn.connect(QString{},
                 QString{},
                 "se.mysland.openzwave",
                 "publishNrNodesAck",
                 this,
                 SLOT(publishNrSlotsRecvAck())); /* Run slot when client acknowledges receival */

    conn.connect(QString{},
                 QString{},
                 "se.mysland.openzwave",
                 "serverReady",
                 this,
                 SLOT(serverReadySlot()));
    conn.connect(QString{},
                 QString{},
                 "se.mysland.openzwave",
                 "requestNodeTransfer",
                 this,
                 SLOT(broadcastNodes()));

    // We want to free and stop the engine when exiting the application
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(beforeExit()));

    /*
     * Initialize OpenZWave engine, but we don't want to do this in the same thread
     * that creates the GUI (OpenZWave does this internally), but we want to finish
     * the MainWindow constructor by signaling that we are ready to go.
    */
    connect(this, SIGNAL(startOZWInitialization()),
            this, SLOT(InitOpenZWave()));
    emit startOZWInitialization();

    // Start animation
    QMovie * movie = new QMovie("./progress.gif");
    movie->setScaledSize(ui->label_2->size());
    ui->label_2->setMovie(movie);
    movie->start();

    // Init timer
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(broadcastNodes()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::InitOpenZWave()
{
    appendText("Initializing OpenZWave engine ...");

    // Create instance of the worker thread class
    wt = new OpenZWaveBackgroundThread();
    wt->setData(ttyPort, configPath, !silent);

    connect(wt, &OpenZWaveBackgroundThread::resultReady,
            this, &MainWindow::InitOpenZWaveDone);
    connect(wt, &OpenZWaveBackgroundThread::finished,
            wt, &QObject::deleteLater);

    // Start timer
    t.start();
    wt->start();
}

void MainWindow::InitOpenZWaveDone(bool res)
{
    ts.getElapsedTime(t.elapsed() * 1e3);
    appendText("Initialization done! Elapsed time: " + ts.toString());
    readyToServe = true;
    // Stop animation
    QMovie * mv = ui->label_2->movie();
    mv->stop();
    delete mv;
    QPixmap pm{"check.png"};
    ui->label_2->setPixmap(pm);
    // Now we want to publish the available nodes to the client
    broadcastNodes();
    timer->start(BROADCAST_TIMEOUT);
}

// This will be called every BROADCAST_TIMEOUT secs to notify changes to client
void MainWindow::broadcastNodes()
{
    if (timer->interval() < BROADCAST_TIMEOUT)
    {
        timer->start(BROADCAST_TIMEOUT);
    }
    appendText("Notifying clients");
    // First, publish the number of nodes discovered
    QDBusMessage msg = QDBusMessage::createSignal("/",
                                                  "se.mysland.openzwave",
                                                  "publishNrNodes");
    msg << static_cast<uint>(g_nodes.size());
    QDBusConnection::sessionBus().send(msg);
}

void MainWindow::serverReadySlot()
{
    QDBusMessage msg = QDBusMessage::createSignal("/",
                                                  "se.mysland.openzwave",
                                                  "serverReadyAck");
    msg << readyToServe;
    QDBusConnection::sessionBus().send(msg);
}

void MainWindow::StopOpenZWave()
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
    StopOpenZWave();
    QCoreApplication::quit();
}

bool MainWindow::validNodeId(uint devId)
{
    for (auto const & node : g_nodes)
    {
        if (node->m_nodeId == static_cast<uint32>(devId))
            return true;
    }
    return false;
}

void MainWindow::statusSetSlot(uint devId, uint statusCode)
{
    appendText("Received request: STATUS(" +
               QString::number(devId) + ") for node ID " +
               QString::number(statusCode));
    if (readyToServe)
    {
        // Check if devId is a valid one
        if (!validNodeId(devId))
        {
            appendText("Request for non-existent NodeID \"" +
                       QString::number(devId) + "\", skipping request.");
            return;
        }

        bool result = ToggleSwitchBinary(SWITCH_BINARY_ID,
                                         static_cast<bool>(statusCode));
        QDBusMessage msg = QDBusMessage::createSignal("/", "se.mysland.openzwave", "statusSetAck");
        msg << static_cast<uint>(result);
        QDBusConnection::sessionBus().send(msg);
    }
    else
    {
        appendText("The OpenZWave engine is still initializing ...");
    }
}

void MainWindow::publishNrSlotsRecvAck()
{
    // Once done this, we want to publish the nodes to the client
    QDBusMessage msg;
    QDBusConnection conn = QDBusConnection::sessionBus();
    for (auto const & node : g_nodes)
    {
        msg = QDBusMessage::createSignal("/", "se.mysland.openzwave", "publishNodeDetails");
        msg << static_cast<uint>(node->m_nodeId);
        if (node->m_nodeId == 21)
        {
            msg << static_cast<uint>(0);
            msg << static_cast<uint>(1);
        }
        else
        {
            if (node->m_nodeId != 1)
            {
                msg << static_cast<uint>(0);
                msg << static_cast<uint>(99);
            }
        }
        conn.send(msg);
    }
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
        // One of the node values has changed
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
        g_initFailed = true;
        pthread_cond_broadcast(&initCond);
        break;
    }

    case Notification::Type_AwakeNodesQueried:
    case Notification::Type_AllNodesQueried:
    case Notification::Type_AllNodesQueriedSomeDead:
    {
        pthread_cond_broadcast(&initCond);
        break;
    }

    case Notification::Type_DriverReset:
    case Notification::Type_Notification:
    case Notification::Type_NodeNaming:
    case Notification::Type_NodeProtocolInfo:
    case Notification::Type_NodeQueriesComplete:
    default:
    {
    }
    }

    pthread_mutex_unlock( &g_criticalSection );
}
