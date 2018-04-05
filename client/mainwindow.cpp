#include <QDBusConnection>

// D-Bus includes
#include "openzwave_adaptor.h"
#include "openzwave_interface.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("OpenZWave client");

    // Set the send button disabled
    ui->pushButton->setEnabled(false);
    ui->comboBox->clear();
    ui->comboBox_2->clear();

    // Set the fancy OZW logo
    QPixmap pm{"ozwlogo.png"};

    ui->label_3->setPixmap(pm);

    QDBusConnection conn = QDBusConnection::sessionBus();

    new OpenzwaveAdaptor(this);
    conn.registerObject("/", this);
    se::mysland::openzwave * iface;
    iface = new se::mysland::openzwave(QString{},
                                       QString{},
                                       conn,
                                       this);
    connect(iface, SIGNAL(statusSetAck(uint)),
            this, SLOT(statusSetAckSlot(uint)));
    connect(iface, SIGNAL(publishNrNodes(uint)),
            this, SLOT(publishNrNodesSlot(uint)));
    connect(iface, SIGNAL(publishNodeDetails(uint,uint,uint)),
            this, SLOT(publishNodeDetailsSlot(uint,uint,uint)));
    connect(iface, SIGNAL(serverReadyAck(bool)),
            this, SLOT(serverReadyAckSlot(bool)));

    // Ask the server if it is ready to accept commands
    QDBusMessage msg = QDBusMessage::createSignal("/",
                                                  "se.mysland.openzwave",
                                                  "serverReady");
    conn.send(msg);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    uint nodeId = static_cast<uint>(ui->comboBox->currentText().toInt());
    uint statusCode = static_cast<uint>(ui->comboBox_2->currentText().toInt());
    appendText(ui->plainTextEdit, "Request for node(" +
               QString::number(nodeId) + ") to set status(" +
               QString::number(statusCode) + "). Awaiting server response.......", false);
    QDBusMessage msg = QDBusMessage::createSignal("/", "se.mysland.openzwave", "statusSet");
    msg << nodeId << statusCode;
    QDBusConnection::sessionBus().send(msg);
}

void MainWindow::appendText(QPlainTextEdit * qpt, QString const & txt, bool nl)
{
    mutex.lock();
    if (nl)
    {
        QChar c;
        if (!qpt->toPlainText().isEmpty())
            c = qpt->toPlainText().back();
        else
            qpt->setPlainText("[" + QDateTime::currentDateTime().toString() + "] ");

        qpt->setPlainText( qpt->toPlainText() +
                           (c == '\n' ? ("[" + QDateTime::currentDateTime().toString() + "] ") : "") +
                           txt + "\n");
    }
    else
        qpt->setPlainText(qpt->toPlainText() +
                          "[" + QDateTime::currentDateTime().toString() + "] " +
                          txt);
    mutex.unlock();
}

bool MainWindow::isNodeNew(NodeDetails *nd)
{
    for (auto const & node : list)
    {
        if (nd->nodeId == node->nodeId and
                nd->minVal == node->minVal and
                nd->maxVal == node->maxVal)
            return false;
    }
    return true;
}

void MainWindow::statusSetAckSlot(uint responseCode)
{
    appendText(ui->plainTextEdit,
               responseCode ? "[ok]" : "[fail]", true);
}

void MainWindow::publishNodeDetailsSlot(uint nodeID, uint minVal, uint maxVal)
{
    NodeDetails nd {nodeID, minVal, maxVal};
    if (isNodeNew(&nd))
    {
        appendText(ui->plainTextEdit,
                   "New node available: NodeID(" +
                   QString::number(nodeID) + "), MinVal(" +
                   QString::number(minVal) + "), MaxVal(" +
                   QString::number(maxVal) + ")",
                   true);
        // Append it to the list
        list.append(new NodeDetails(nodeID, minVal, maxVal));
        // Send Ack for this node
        QDBusConnection conn = QDBusConnection::sessionBus();
        QDBusMessage msg = QDBusMessage::createSignal("/", "se.mysland.openzwave", "publishNodeDetailsAck");
        msg << nodeID;
        conn.send(msg);
        // Add it to the combo boxes
        ui->comboBox->addItem(QString::number(nodeID));
        for (uint i=minVal; i <= maxVal; ++i)
        {
            ui->comboBox_2->addItem(QString::number(i));
        }
        // Re-enable button
        if (!ui->pushButton->isEnabled())
            ui->pushButton->setEnabled(true);
    }
}

void MainWindow::publishNrNodesSlot(uint _nrNodes)
{
    nrNodes = _nrNodes;
    // Send ack through bus
    QDBusConnection conn = QDBusConnection::sessionBus();
    QDBusMessage msg = QDBusMessage::createSignal("/",
                                                  "se.mysland.openzwave",
                                                  "publishNrNodesAck");
    conn.send(msg);
}

void MainWindow::serverReadyAckSlot(bool ready)
{
    if (ready)
    {
        // Then request that the daemon sends us the nodes
        QDBusMessage msg = QDBusMessage::createSignal("/",
                                                      "se.mysland.openzwave",
                                                      "requestNodeTransfer");
        QDBusConnection::sessionBus().send(msg);
    }
    else
    {
        appendText(ui->plainTextEdit,
                   "OpenZWave daemon is still initializing...",
                   true);
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    ui->comboBox->setCurrentText("0");
    ui->comboBox_2->setCurrentText("0");
    ui->plainTextEdit->clear();
}

void MainWindow::on_comboBox_currentTextChanged(const QString &arg1)
{
    // Check which NodeId is currently displayed
    uint currentDevId = ui->comboBox->currentText().toInt();
    for (auto const & node : list)
    {
        if (node->nodeId == currentDevId)
        {
            ui->comboBox_2->clear();
            for (uint i = node->minVal; i <= node->maxVal; ++i)
            {
                ui->comboBox_2->addItem(QString::number(i));
            }
        }
    }
}
