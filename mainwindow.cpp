#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    server(new QTcpServer),
    socket(new QTcpSocket),
    socket2(new QTcpSocket), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;

}

void MainWindow::on_connectButton_clicked()
{
    socket->abort();
    socket->connectToHost("127.0.0.1",8000);
    if (!socket->waitForConnected(100)){
        server->listen(QHostAddress("127.0.0.1"),8000);
        connect(server.data(),SIGNAL(newConnection()),this,SLOT(new_connection()));
        ui->connectStatus->setText("状态：服务器启动，现有1名玩家加入");
        isMaster = true;
    }
    else{
        connect(socket.data(), SIGNAL(readyRead()), this, SLOT(read1()));
        ui->connectStatus->setText("状态：已连接到服务器！");
        isMaster = false;
    }
    ui->connectButton->setEnabled(false);
}
void MainWindow::new_connection()
{
    static int cnt = 0;
    if (cnt == 0){
        ui->connectStatus->setText("状态：服务器启动，现有2名玩家加入");
        socket = QTcpSocket_ptr(server->nextPendingConnection());
        connect(socket.data(), SIGNAL(readyRead()), this, SLOT(read1()));
    }
    else if (cnt == 1){
        ui->connectStatus->setText("状态：服务器启动，现有3名玩家加入，游戏即将开始");
        socket2 = QTcpSocket_ptr(server->nextPendingConnection());
        connect(socket2.data(), SIGNAL(readyRead()), this, SLOT(read2()));
        socket->write(toWrite(NEXT_PAGE));
        socket2->write(toWrite(NEXT_PAGE));
    }
    cnt = (cnt+1) % 2;

}
void MainWindow::next_page(bool m)
{
    static int cnt = 0;
    if (!isMaster || cnt == 1){
        this->hide();
        if (isMaster){
            disconnect(socket.data(), SIGNAL(readyRead()), this, SLOT(read1()));
            disconnect(socket2.data(), SIGNAL(readyRead()), this, SLOT(read2()));
        }
        else
            disconnect(socket.data(), SIGNAL(readyRead()), this, SLOT(read1()));
        playwidget = PlayWidget_ptr(new PlayWidget(server, socket, socket2, m));
        connect(playwidget.data(), SIGNAL(closed()), this, SLOT(reshow()));
        playwidget->move(x() + width()/2 - 450, y() + height()/2 - 300);
        playwidget->show();
        if (!isMaster){
            socket->write(toWrite(NEXT_PAGE));
        }
    }
    cnt = (cnt + 1) % 2;

}

void MainWindow::read1()
{
    QJsonArray msg_list = toRead(socket);
    foreach (auto msg_value, msg_list){
        QJsonObject msg = msg_value.toObject();
        switch (msg["REQUEST"].toInt()) {
        case NEXT_PAGE:
            next_page(isMaster);
            break;
        }
    }
}
void MainWindow::read2()
{
    QJsonArray msg_list = toRead(socket2);
    foreach (auto msg_value, msg_list){
        QJsonObject msg = msg_value.toObject();
        switch (msg["REQUEST"].toInt()) {
        case NEXT_PAGE:
            next_page(isMaster);
            break;
        }
    }

}
void MainWindow::reshow()
{
    if (playwidget->isVisible() || !this->isVisible()){
        if (playwidget->isVisible())
            playwidget->close();
        if (!this->isVisible()){
            ui->connectStatus->setText("等待连接");
            ui->connectButton->setEnabled(true);
            this->show();
        }
        if (isMaster){
            socket->write(toWrite(LAST_PAGE));
            socket2->write(toWrite(LAST_PAGE));
            disconnect(socket.data(), SIGNAL(readyRead()), playwidget.data(), SLOT(first_read()));
            disconnect(socket2.data(), SIGNAL(readyRead()), playwidget.data(), SLOT(second_read()));
            server->close();
            disconnect(server.data(),SIGNAL(newConnection()),this,SLOT(new_connection()));
        }
        else{
            disconnect(socket.data(), SIGNAL(readyRead()), playwidget.data(), SLOT(self_read()));
            socket->write(toWrite(LAST_PAGE));
        }

    }
}
