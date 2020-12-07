#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTcpSocket>
#include <qtimer.h>
#include <qeventloop.h>
#include <qudpsocket.h>
#include <qnetworkdatagram.h>
#include <qdebug.h>
#include <AboutDialog.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_emergeSocket = nullptr;
    connect(&m_server,&QTcpServer::newConnection,this,&MainWindow::on_newConnection);
    setFixedSize(size());
    //Scan for devices on start
    QTimer::singleShot(100,this,&MainWindow::on_btnScan_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_btnOn_clicked()
{
    if (!ui->btnOn->isChecked())
    {
        //Turno off the server
        m_server.close();
        return;
    }
    QFile f(ui->edtFile->text());
    if (!f.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this,"Error","No file selected or file path is invalid");
        ui->btnOn->setChecked(false);
        return;
    }
    f.close();
    //Set the server to listening mode, when the button is on
    m_server.setMaxPendingConnections(1);
    m_server.listen(QHostAddress::Any,ui->spnPort->value());
}

void MainWindow::on_pushButton_clicked()
{
    //Select file name to upload
    QString str (QFileDialog::getOpenFileName(this,"Select file",QString(),"(*.bin)"));
    if (str.isEmpty())
        return;
    ui->edtFile->setText(str);
}
void MainWindow::on_newConnection()
{
    m_emergeSocket = m_server.nextPendingConnection();
    QFile f(ui->edtFile->text());
    //Change the thread
    m_emergeSocket->moveToThread(this->thread());
    QMetaObject::invokeMethod(this,"processEmerge");

}
//Process uploading a file to the device
void MainWindow::processEmerge()
{
    if (!m_emergeSocket)
        return;

    QFile f(ui->edtFile->text());
    if (!f.open(QIODevice::ReadOnly))
    {
        ui->btnOn->click();
        QMessageBox::critical(this,"Error","Client is connected, but the file can not be opened!");
        delete m_emergeSocket;
        m_emergeSocket = nullptr;
        return;
    }
    quint32 sz = f.size();
    quint32 read = 0;
    //send the file size at the beginnig
    m_emergeSocket->write((const char *)&sz,sizeof(sz));
    m_emergeSocket->waitForBytesWritten(1000);
    //Now send the file
    QByteArray data;
    while (read < sz)
    {
        data = f.read(1024);
        read += data.size();
        m_emergeSocket->write(data);
        if (!m_emergeSocket->waitForBytesWritten(3000))
        {
            m_emergeSocket->close();
            delete m_emergeSocket;
            m_emergeSocket = nullptr;
            f.close();
            ui->btnOn->click();
            QMessageBox::critical(this,"Error","Client closed connection");
            return;
        }
    }
    m_emergeSocket->flush();
//    Give some time to OS to flush the buffers, before closing connection.
    QEventLoop loop;
    QTimer::singleShot(1000,&loop,&QEventLoop::quit);
    loop.exec();

    m_emergeSocket->close();
    delete m_emergeSocket;
    m_emergeSocket = nullptr;

    f.close();
    ui->btnOn->click();
}

//Send an UDP broadcast to discover devices in the local network
void MainWindow::on_btnScan_clicked()
{
    QUdpSocket socket;
    quint32 mark = ui->edtMark->text().toUInt(nullptr,16);
    socket.bind();
    socket.open(QIODevice::ReadWrite);
    socket.writeDatagram((const char *) &mark,sizeof (mark),QHostAddress::Broadcast,ui->spnBroadcast->value());
    socket.flush();
    //1.5 sec reply time
    QEventLoop loop;
    QTimer::singleShot(1500,&loop,&QEventLoop::quit);
    loop.exec();
    ui->lstDevices->clear();
    while (socket.hasPendingDatagrams())
    {
        QNetworkDatagram dgram(socket.receiveDatagram());
        QByteArray data (dgram.data());
        if (data.size() < 2 * sizeof (quint32))
            continue; // Not enough data in this datagram
        quint32* ref = (quint32*)data.data();
        if (*ref != mark)
            continue; //Invalid response bytes
        data.remove(0,sizeof (quint32));
        //Get the TCP listen port from the reply
        ref = (quint32*)data.data();
        QString strAddr(dgram.senderAddress().toString().split(":").last());
        strAddr.append(':');
        strAddr.append(QString::number(*ref));
        ui->lstDevices->addItem(strAddr);
    }
    socket.close();
}

void MainWindow::on_btnPush_clicked()
{

    //IP:PORT from the list
    QStringList addrSplit(ui->lstDevices->currentItem()->text().split(':'));
    if (!addrSplit.count())
    {
        QMessageBox::critical(this,"Error","Device not selected");
        return;
    }

    QFile f(ui->edtFile->text());
    if (!f.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this,"Error","Selected file can not be opened");
        return;
    }
    //Open connection to the device
    QTcpSocket socket;
    socket.connectToHost(addrSplit.first(),addrSplit.last().toInt());
    if (!socket.waitForConnected(3000))
    {
        QMessageBox::critical(this,"Error","Unable to connect selected device");
        return;
    }
    quint32 sz = f.size();
    //Send size as first 32 bits
    socket.write((const char *)&sz,sizeof(sz));
    socket.waitForBytesWritten(1000);
    quint32 read = 0;
    QByteArray data;
    while (read < sz)
    {
        data = f.read(1024);
        read += data.size();
        socket.write(data);
        if (!socket.waitForBytesWritten(3000))
        {
            socket.close();
            f.close();
            QMessageBox::critical(this,"Error","Device closed connection");
            return;
        }
    }
    socket.flush();
    //Some time before closing connection.
    QEventLoop loop;
    QTimer::singleShot(1000,&loop,&QEventLoop::quit);
    loop.exec();

    socket.close();
    f.close();
}

void MainWindow::on_btnAbout_clicked()
{
    AboutDialog().exec();
}
