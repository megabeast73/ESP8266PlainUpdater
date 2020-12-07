#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void processEmerge();
private slots:
    void on_btnOn_clicked();
    void on_pushButton_clicked();
    void on_newConnection();


    void on_btnScan_clicked();

    void on_btnPush_clicked();

    void on_btnAbout_clicked();

private:
    Ui::MainWindow *ui;
    QTcpServer m_server;
    QTcpSocket* m_emergeSocket;
};
#endif // MAINWINDOW_H
