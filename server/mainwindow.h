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

public:
    explicit MainWindow(bool _graphic,
                        bool _silent,
                        QString const & _ttyPort,
                        QString const & _configPath,
                        QWidget *parent = 0);
    ~MainWindow();

signals:
    void        newState(uint const & devId, uint const & status);
    void        startOZWInitialization();
    void        initDone(bool res);

private:
    void        appendText(QString const & txt);
    bool        ToggleSwitchBinary(const int node_Id, bool status);
    bool        ToggleSwitchMultilevel(const int node_Id,
                                       const uint8 level);
    void        StopOpenZWave();
    bool        validNodeId(uint devId);
    bool        validValue(uint devId, uint val);

private slots:
    void        statusSetSlot(uint devId, uint statusCode);
    void        publishNrSlotsRecvAck();
    void        on_pushButton_clicked();
    void        InitOpenZWave();
    void        beforeExit();
    void        InitOpenZWaveDone(bool res);
    void        broadcastNodes();
    void        serverReadySlot();

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
};

#endif // MAINWINDOW_H
