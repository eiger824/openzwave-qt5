#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QList>
#include <QMutex>

#include "defs.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void appendText(QPlainTextEdit * qpt, QString const & txt, bool nl);
    bool isNodeNew(NodeDetails * nd);

private slots:
    void on_pushButton_clicked();
    void statusSetAckSlot(uint devId, bool result);
    void statusChangedCfgSlot(uint devId);
    void publishNodeDetailsSlot(uint nodeID ,uint minVal ,uint maxVal);
    void publishNrNodesSlot(uint _nrNodes);
    void serverReadyAckSlot(bool ready);
    void on_pushButton_2_clicked();

    void on_comboBox_currentTextChanged(const QString &arg1);

signals:
    void newState(uint const & devId, uint const & status);

private:
    Ui::MainWindow *ui;
    QList<NodeDetails*> list;
    uint nrNodes;
    QMutex mutex;
};

#endif // MAINWINDOW_H
