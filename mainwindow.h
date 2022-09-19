#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include "playwidget.h"
#include "defines.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_connectButton_clicked();
    void new_connection();
    void next_page(bool m);
    void read1();
    void read2();
    void reshow();


private:
    QTcpServer_ptr server;
    QTcpSocket_ptr socket;
    QTcpSocket_ptr socket2;
    PlayWidget_ptr playwidget;
    Ui::MainWindow *ui;
    bool isMaster;
};

#endif // MAINWINDOW_H
