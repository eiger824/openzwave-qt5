#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QList>
#include <QTime>
#include <QTimer>
#include <QThread>

// OpenZWave includes
#include "Notification.h"

// Local includes
#include "timespec.h"

#include "defs.h"

namespace Ui {
class MainWindow;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * OpenZWave background thread * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

class OpenZWaveBackgroundThread : public QThread
{
    Q_OBJECT
public:
    void setData(QString const & _tty,
                 QString const & _conf,
                 bool _verbose)
    {
        tty = _tty;
        conf = _conf;
        verbose = _verbose;
    }
    void run() override;

signals:
    void resultReady(bool res);

private:
    QString tty;
    QString conf;
    bool verbose;
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "se.mysland.openzwave")

public:
    explicit MainWindow(bool _graphic,
                        bool _silent,
                        QString const & _ttyPort,
                        QString const & _configPath,
                        QWidget *parent = 0);
    ~MainWindow();
    static MainWindow * Get() { return instance; }
    void        acknowledgeTransferToNode(uint nodeId);
    inline bool isDBusWorking() const { return dbusSuccess; }
    void        startOZWEngine();

public Q_SLOTS:
    void        statusSet(uint deviceId, uint statusCode);
    void        statusGet(uint deviceId);

    void        serverReady();
    void        requestNodeTransfer();
    void        publishNrNodesAck();
    void        publishNodeDetailsAck(uint nodeId);

Q_SIGNALS:
    // Daemon informs whether the set status op was successful
    void        statusSetAck(uint requestNode, bool requestOk);
    // Daemon informs about node's current state
    void        statusGetRsp(bool successOp, uint deviceStatus);
    // Remote node has changed its status and confirms
    void        statusChangedCfm(uint nodeId);
    // Client asks if server ready
    void        serverReadyAck(bool ready);
    // Daemon informs about the number of nodes
    void        publishNrNodes(uint nodeNr);
    // Daemon sends details about the discovered nodes
    void        publishNodeDetails(uint nodeId, uint nodeIdMinVal, uint nodeIdMaxVal);

signals:
    void        newState(uint const & devId, uint const & status);
    void        startOZWInitialization();
    void        initDone(bool res);

private:
    void        appendText(QString const & txt);
    bool        toggleSwitchBinary(const int node_Id, bool status);
    bool        toggleSwitchMultilevel(const int node_Id,
                                       const uint8 level);
    bool        getSwitchBinaryStatus(const uint nodeId,
                                      uint & status);
    bool        getSwitchMultilevelStatus(const uint nodeId,
                                          uint & status);

    void        stopOpenZWave();
    bool        isValidNodeId(uint devId);
    bool        isValidValue(uint devId, uint val);

private slots:
    void        on_pushButton_clicked();
    void        initOpenZWave();
    void        beforeExit();
    void        initOpenZWaveDone(bool res);
    void        broadcastNodes();

private:
    OpenZWaveBackgroundThread * wt;

private:
    Ui::MainWindow *ui;
    bool graphic;
    bool silent;
    QString ttyPort;
    QString configPath;

    // Timer to measure different elapsed times
    QTime t;
    TimeSpec ts;

    // Timer to broadcast nodes
    QTimer * timer;

    // Defines whether the daemon has ended up initialization
    bool readyToServe;

    // Ptr of instance of this object
    static MainWindow * instance;

    // Defines whether everything 'looks good' from the d-bus side
    bool dbusSuccess;
};

#endif // MAINWINDOW_H
