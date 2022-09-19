#ifndef PLAYWIDGET_H
#define PLAYWIDGET_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QPixmap>
#include "defines.h"
#include "clickedlabel.h"

#define FIRST_X 700
#define FIRST_Y 70
#define SECOND_X 90
#define SECOND_Y 70
#define MY_X 220
#define MY_Y 530
#define MIDDLE_X 400
#define MIDDLE_Y 200
#define GAP 20
#define WIDTH 100
#define HEIGHT 120

typedef QSharedPointer<QLabel> QLabel_ptr;
typedef QSharedPointer<ClickedLabel> ClickedLabel_ptr;

namespace Ui {
    class PlayWidget;
}
class PlayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PlayWidget(QTcpServer_ptr a, QTcpSocket_ptr b, QTcpSocket_ptr c, bool m, QWidget *parent = nullptr);
    ~PlayWidget();

signals:
    void closed();

private slots:
    void self_read();
    void first_read();
    void second_read();
    void on_exitButton_clicked();

    void on_callButton_clicked();

    void on_notCallButton_clicked();

    void on_playButton_clicked();

    void on_notPlayButton_clicked();

    void on_restartButton_clicked();

private:
    QTcpServer_ptr server;
    QTcpSocket_ptr socket;
    QTcpSocket_ptr socket2;
    Ui::PlayWidget *ui;

    bool isMaster;
    bool isPlaying;
    int start;
    int restartCnt;
    int lastPlayed;
    Format formCheck;
    QList<int> cards;
    QList<int> leftCards;
    QList<bool> isCalledList;
    QList<int> selectCards;

    //ui control
    int firstPicNum;
    int secondPicNum;
    QLabel_ptr firstPic[20];
    QLabel_ptr secondPic[20];
    QList<QLabel_ptr> middlePic;
    QList<ClickedLabel_ptr> myPic;

private:
    void init_game();
    void deal_cards();
    void call_landlords();
    void finish_call_lanlords(bool isCalled);
    void decide_lanlords();
    void show_call_messages(int pos, bool isCalled);
    void show_cards_messagess(int pos, int num);
    void show_mycards_messagess();
    void show_middlecards_message(QList<int>& middleCards);
    void play_cards(bool isF, int format = CARD_TYPE_ALL_COMPARE, int argv1 = 0, int argv2 = 0);
    void decide_play(int format, int argv1, int argv2);
    void begin_game(int finalCards[], int pos);
    void restart_game();

public slots:
    void label_clicked(ClickedLabel* label);

};

typedef QSharedPointer<PlayWidget> PlayWidget_ptr;

#endif // PLAYWIDGET_H
