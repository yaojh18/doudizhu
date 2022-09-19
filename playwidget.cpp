#include "playwidget.h"
#include "ui_playwidget.h"

#include <QtDebug>
#include <QTime>
#include <windows.h>

PlayWidget::PlayWidget(QTcpServer_ptr a, QTcpSocket_ptr b, QTcpSocket_ptr c, bool m, QWidget *parent)
    : QWidget(parent), server(a), socket(b), socket2(c), ui(new Ui::PlayWidget), isMaster(m)
{
    ui->setupUi(this);
    qsrand(QTime::currentTime().msec());
    if (isMaster){
        connect(socket.data(), SIGNAL(readyRead()), this, SLOT(first_read()));
        connect(socket2.data(), SIGNAL(readyRead()), this, SLOT(second_read()));
    }
    else{
        connect(socket.data(), SIGNAL(readyRead()), this, SLOT(self_read()));
    }
    init_game();
}
PlayWidget::~PlayWidget()
{
    delete ui;
}
void PlayWidget::init_game()
{
    isPlaying = false;
    restartCnt = 0;
    firstPicNum = 17;
    secondPicNum = 17;
    cards.clear();
    leftCards.clear();
    isCalledList.clear();

    //TO CHANGE
    for (int i = 0; i < 17; i++){
        firstPic[i] = QLabel_ptr(new QLabel(this));
        firstPic[i]->setPixmap(QPixmap(":/image/images/back.png"));
        firstPic[i]->setScaledContents(true);
        firstPic[i]->setGeometry(FIRST_X ,FIRST_Y + GAP * i, WIDTH, HEIGHT);
        firstPic[i]->setStyleSheet("background-color: rgb(0,0,0,0)");
        firstPic[i]->show();
        secondPic[i] = QLabel_ptr(new QLabel(this));
        secondPic[i]->setPixmap(QPixmap(":/image/images/back.png"));
        secondPic[i]->setScaledContents(true);
        secondPic[i]->setGeometry(SECOND_X ,SECOND_Y + GAP * i, WIDTH, HEIGHT);
        secondPic[i]->setStyleSheet("background-color: rgb(0,0,0,0)");
        secondPic[i]->show();
    }
    for (int i = 17; i < 20; i++){
        if (!firstPic[i].isNull())
            firstPic[i].clear();
        if (!secondPic[i].isNull())
            secondPic[i].clear();
    }
    myPic.clear();
    middlePic.clear();
    ui->showPic1->setPixmap(QPixmap(":/image/images/back.png"));
    ui->showPic2->setPixmap(QPixmap(":/image/images/back.png"));
    ui->showPic3->setPixmap(QPixmap(":/image/images/back.png"));
    //END CHANGE

    ui->myMessage->setText(QString());
    ui->secondMessage->setText(QString());
    ui->firstMessage->setText(QString());
    ui->myStatus->setText(QString());
    ui->firstStatus->setText(QString());
    ui->secondStatus->setText(QString());
    ui->firstStackedWidget->setCurrentWidget(ui->page11);
    ui->secondStackedWidget->setCurrentWidget(ui->page21);
    ui->callButton->setVisible(false);
    ui->notCallButton->setVisible(false);
    ui->firstCardsNum->setText("17");
    ui->secondCardsNum->setText("17");
    ui->playButton->setVisible(false);
    ui->notPlayButton->setVisible(false);
    ui->restartButton->setVisible(false);
    ui->exitButton->setVisible(false);

    if (isMaster){
        int newCards[54];
        for (int i = 0 ; i< 54; i++)
            newCards[i] = i;
        std::shuffle(std::begin(newCards), std::end(newCards), std::default_random_engine(QTime::currentTime().msec()));
        leftCards.append(newCards[51]);
        leftCards.append(newCards[52]);
        leftCards.append(newCards[53]);
        QJsonArray b;
        QJsonArray c;
        for (int i = 0; i < 51; i++){
            if (i % 3 == 0)
                cards.append(newCards[i]);
            else if (i % 3 == 1)
                b.append(newCards[i]);
            else
                c.append(newCards[i]);
        }
        deal_cards();
        socket->write(toWrite(DEAL,b));
        socket2->write(toWrite(DEAL,c));
        start = rand() % 3;
        if (start == 0)
            call_landlords();
        else if (start == 1)
            socket->write(toWrite(CALL));
        else
            socket2->write(toWrite(CALL));
    }
}

void PlayWidget::deal_cards()
{
    std::sort(cards.begin(), cards.end());
    for (int i = 0; i < cards.size(); i++){
        myPic.append(ClickedLabel_ptr(new ClickedLabel(cards[i] ,this)));
    }
    show_mycards_messagess();

}

void PlayWidget::call_landlords()
{
    ui->callButton->setVisible(true);
    ui->notCallButton->setVisible(true);
}

void PlayWidget::finish_call_lanlords(bool isCalled)
{
    if (isMaster){
        isCalledList.append(isCalled);
        if (start == 1)
            decide_lanlords();
        else{
            socket->write(toWrite(SHOW_CALL_MESSAGE, QJsonArray({2, isCalled})));
            socket2->write(toWrite(SHOW_CALL_MESSAGE, QJsonArray({1, isCalled})));
            socket->write(toWrite(CALL));
        }
    }
    else{
        socket->write(toWrite(CALL_FINISH,QJsonArray({isCalled})));
    }
}

void PlayWidget::decide_lanlords()
{
    if (isCalledList[1] == 1){
        if (isCalledList[2] == 1)
            start = (start + 2) % 3;
        else
            start = (start + 1) % 3;
    }
    else{
        if (isCalledList[2] == 1)
            start = (start + 2) % 3;
        else
            start = (start + 3) % 3;
    }
    lastPlayed = start;
    int tmp[3];
    QJsonArray json_tmp;
    for (int i = 0; i < 3 ; i++){
        json_tmp.append(leftCards[i]);
        tmp[i] = leftCards[i];
    }
    begin_game(tmp, start);
    socket->write(toWrite(BEGIN_GAME, json_tmp + (start + 2) % 3));
    socket2->write(toWrite(BEGIN_GAME, json_tmp + (start + 1) % 3));
    if (start == 0){
        for (int i = 0; i < 3; i++)
            cards.append(tmp[i]);
        myPic.clear();
        deal_cards();
        play_cards(true);
        socket->write(toWrite(SHOW_CARDS_MESSAGE, QJsonArray({2, 3})));
        socket2->write(toWrite(SHOW_CARDS_MESSAGE, QJsonArray({1, 3})));
    }
    else if (start == 1){
        socket->write(toWrite(ADD_CARDS, json_tmp));
        socket->write(toWrite(PLAY_CARDS, QJsonArray({true})));
        show_cards_messagess(1, 3);
        socket2->write(toWrite(SHOW_CARDS_MESSAGE, QJsonArray({2, 3})));
    }
    else if (start == 2){
        socket2->write(toWrite(ADD_CARDS, json_tmp));
        socket2->write(toWrite(PLAY_CARDS, QJsonArray({true})));
        show_cards_messagess(2, 3);
        socket->write(toWrite(SHOW_CARDS_MESSAGE, QJsonArray({1, 3})));
    }


}

void PlayWidget::show_call_messages(int pos, bool isCalled)
{
    if (pos == 1){
        if (isCalled)
            ui->firstMessage->setText("叫地主");
        else
            ui->firstMessage->setText("不叫");
    }
    else{
        if (isCalled)
            ui->secondMessage->setText("叫地主");
        else
            ui->secondMessage->setText("不叫");
    }
}

void PlayWidget::show_cards_messagess(int pos, int num)
{
    if (pos == 1){
        if (num > 0)
            for (int i = firstPicNum; i < num + firstPicNum;i++){
                firstPic[i] = QLabel_ptr(new QLabel(this));
                firstPic[i]->setPixmap(QPixmap(":/image/images/back.png"));
                firstPic[i]->setScaledContents(true);
                firstPic[i]->setGeometry(FIRST_X ,FIRST_Y + GAP * i, WIDTH, HEIGHT);
                firstPic[i]->setStyleSheet("background-color: rgb(0,0,0,0)");
                firstPic[i]->show();
            }
        else
            for (int i = firstPicNum + num; i <firstPicNum; i++)
                firstPic[i].clear();
        firstPicNum += num;
        ui->firstCardsNum->setText(QString::number(firstPicNum));
    }
    else{
        if (num > 0)
            for (int i = secondPicNum; i < num + secondPicNum;i++){
                secondPic[i] = QLabel_ptr(new QLabel(this));
                secondPic[i]->setPixmap(QPixmap(":/image/images/back.png"));
                secondPic[i]->setScaledContents(true);
                secondPic[i]->setGeometry(SECOND_X ,SECOND_Y + GAP * i, WIDTH, HEIGHT);
                secondPic[i]->setStyleSheet("background-color: rgb(0,0,0,0)");
                secondPic[i]->show();
            }
        else
            for (int i = secondPicNum + num; i <secondPicNum; i++)
                secondPic[i].clear();
        secondPicNum += num;
        ui->secondCardsNum->setText(QString::number(secondPicNum));
    }
}
void PlayWidget::show_middlecards_message(QList<int>& middleCards)
{
    middlePic.clear();
    for (int i = 0; i < middleCards.size(); i++){
        middlePic.append(QLabel_ptr(new QLabel(this)));
        middlePic[i]->setPixmap(QPixmap(":/image/images/" + QString::number(middleCards[i]) + ".png"));
        middlePic[i]->setScaledContents(true);
        middlePic[i]->setGeometry(MIDDLE_X + GAP * (i - middleCards.size()/2), MIDDLE_Y, WIDTH, HEIGHT);
        middlePic[i]->setStyleSheet("background-color: rgb(0,0,0,0)");
        middlePic[i]->show();
    }
}

void PlayWidget::show_mycards_messagess()
{
    for (int i = 0; i < myPic.size(); i++){
        myPic[i]->setGeometry(MY_X + GAP * i, MY_Y, WIDTH, HEIGHT);
    }
}

void PlayWidget::begin_game(int finalCards[], int pos)
{
    //SHOW FINAL CARDS
    ui->showPic1->setPixmap(QPixmap(":/image/images/" + QString::number(finalCards[0]) + ".png"));
    ui->showPic2->setPixmap(QPixmap(":/image/images/" + QString::number(finalCards[1]) + ".png"));
    ui->showPic3->setPixmap(QPixmap(":/image/images/" + QString::number(finalCards[2]) + ".png"));
    //INIT MAP
    ui->myMessage->setText("");
    ui->firstMessage->setText("");
    ui->secondMessage->setText("");
    ui->firstStackedWidget->setCurrentWidget(ui->page12);
    ui->secondStackedWidget->setCurrentWidget(ui->page22);
    if (pos == 0){
        ui->myStatus->setText("地主");
        ui->firstStatus->setText("农民");
        ui->secondStatus->setText("农民");
    }
    else if (pos == 1){
        ui->myStatus->setText("农民");
        ui->firstStatus->setText("地主");
        ui->secondStatus->setText("农民");
    }
    else if (pos == 2){
        ui->myStatus->setText("农民");
        ui->firstStatus->setText("农民");
        ui->secondStatus->setText("地主");
    }
}

void PlayWidget::restart_game()
{
    if (isMaster){
        socket->write(toWrite(RESTART_GAME));
        socket2->write(toWrite(RESTART_GAME));
    }
    init_game();
}

void PlayWidget::play_cards(bool isF, int format, int argv1, int argv2){
    ui->playButton->setVisible(true);
    ui->notPlayButton->setVisible(true);
    ui->playButton->setEnabled(true);
    if (!isF)
        ui->notPlayButton->setEnabled(true);
    else
        ui->notPlayButton->setEnabled(false);
    isPlaying = true;
    formCheck = Format(format, argv1, argv2);
    selectCards.clear();

}
void PlayWidget::decide_play(int format, int argv1, int argv2)
{
    start = (start + 1) % 3;
    if (start == lastPlayed){
        if (start == 0)
            play_cards(true);
        else if (start == 1)
            socket->write(toWrite(PLAY_CARDS, QJsonArray({true})));
        else
            socket2->write(toWrite(PLAY_CARDS, QJsonArray({true})));
    }
    else{
        QJsonArray json_temp;
        json_temp.append(false);
        json_temp.append(format);
        json_temp.append(argv1);
        json_temp.append(argv2);
        if (start == 0)
            play_cards(false, format, argv1, argv2);
        else if (start == 1)
            socket->write(toWrite(PLAY_CARDS, json_temp));
        else
            socket2->write(toWrite(PLAY_CARDS, json_temp));
    }
}

void PlayWidget::label_clicked(ClickedLabel* label)
{
    if (isPlaying){
        int idx;
        for(int i = 0; i< myPic.size(); i++)
            if (myPic[i].data() == label){
                idx = i;
                break;
            }
        if (!label->isClicked){
            selectCards.append(cards[idx]);
            label->setGeometry(MY_X + GAP * idx, MY_Y - 30, WIDTH, HEIGHT);
        }
        else{
            selectCards.removeAt(selectCards.indexOf(cards[idx]));
            label->setGeometry(MY_X + GAP * idx, MY_Y, WIDTH, HEIGHT);
        }
        label->isClicked = !label->isClicked;

    }
}

void PlayWidget::self_read()
{
    QJsonArray msg_list = toRead(socket);
    foreach (auto msg_value, msg_list){
        QJsonObject msg = msg_value.toObject();
        switch(msg["REQUEST"].toInt()){
            case DEAL:
                foreach (auto i, msg["ARGV"].toArray()){
                    cards.append(i.toInt());
                }
                deal_cards();
                break;
            case CALL:
                call_landlords();
                break;
            case SHOW_CALL_MESSAGE:
            {
                int pos = msg["ARGV"].toArray()[0].toInt();
                bool isCalled = msg["ARGV"].toArray()[1].toBool();
                show_call_messages(pos, isCalled);
                break;
            }
            case BEGIN_GAME:
            {
                QJsonArray arr = msg["ARGV"].toArray();
                int tmp[3];
                for (int i = 0; i <3 ;i++)
                    tmp[i] = arr[i].toInt();
                begin_game(tmp, arr[3].toInt());
                break;
            }
            case ADD_CARDS:
            {
                QJsonArray arr = msg["ARGV"].toArray();
                for (int i = 0; i <3 ;i++)
                    cards.append(arr[i].toInt());
                myPic.clear();
                deal_cards();
                break;
            }
            case SHOW_CARDS_MESSAGE:
            {
                int pos = msg["ARGV"].toArray()[0].toInt();
                int num = msg["ARGV"].toArray()[1].toInt();
                show_cards_messagess(pos, num);
                break;
            }
            case PLAY_CARDS:
            {
                QJsonArray arr = msg["ARGV"].toArray();
                bool isForced = arr[0].toBool();
                if (isForced)
                    play_cards(isForced);
                else{
                    int format = arr[1].toInt();
                    int argv1 = arr[2].toInt();
                    int argv2 = arr[3].toInt();
                    play_cards(isForced, format, argv1, argv2);
                }
            }
                break;
            case SHOW_PLAY_MESSAGE:
            {
                QJsonArray arr = msg["ARGV"].toArray();
                int num = arr[0].toInt();
                QList<int> chosenCards;
                for (int i = 1; i < 1+num; i++)
                    chosenCards.append(arr[i].toInt());
                int pos = arr[num + 1].toInt();
                if (pos == 1)
                    ui->firstMessage->setText("");
                else
                    ui->secondMessage->setText("");
                show_middlecards_message(chosenCards);
                show_cards_messagess(pos, -num);
                break;
            }
            case SHOW_NOT_PLAY_MESSAGE:
            {
                int pos = msg["ARGV"].toArray()[0].toInt();
                if (pos == 1)
                    ui->firstMessage->setText("不出");
                else
                    ui->secondMessage->setText("不出");
                break;
            }
            case END_GAME:
            {
                int flag = msg["ARGV"].toArray()[0].toInt();
                if (flag == 0 && ui->myStatus->text() == "农民")
                    ui->myMessage->setText("你赢了");
                else
                    ui->myMessage->setText("你输了");
                ui->restartButton->setVisible(true);
                ui->exitButton->setVisible(true);
                break;
            }
            case RESTART_GAME:
                restart_game();
                break;
            case LAST_PAGE:
                emit closed();
                break;
        }
    }

}

void PlayWidget::first_read()
{
    QJsonArray msg_list = toRead(socket);
    foreach (auto msg_value, msg_list){
        QJsonObject msg = msg_value.toObject();
        switch(msg["REQUEST"].toInt()){
            case CALL_FINISH:
            {
                bool isCalled = msg["ARGV"].toArray()[0].toBool();
                isCalledList.append(isCalled);
                show_call_messages(1, isCalled);
                socket2->write(toWrite(SHOW_CALL_MESSAGE,QJsonArray({2, isCalled})));
                if (start == 2)
                    decide_lanlords();
                else
                    socket2->write(toWrite(CALL));
                break;
            }
            case NOT_PLAY:
                {
                    QJsonArray arr = msg["ARGV"].toArray();
                    int format = arr[0].toInt();
                    int argv1 = arr[1].toInt();
                    int argv2 = arr[2].toInt();
                    decide_play(format, argv1, argv2);
                    ui->firstMessage->setText("不出");
                    socket2->write(toWrite(SHOW_NOT_PLAY_MESSAGE, QJsonArray({2})));
                    break;
                }
            case PLAY:{
                QJsonArray arr = msg["ARGV"].toArray();
                QJsonArray json_temp;

                int num = arr[0].toInt();
                json_temp.append(arr[0]);
                QList<int> chosenCards;
                for (int i = 1; i < 1+num; i++){
                    chosenCards.append(arr[i].toInt());
                    json_temp.append(arr[i]);
                }
                json_temp.append(2);
                bool isEnded = arr[1 + num].toBool();
                show_cards_messagess(1, -num);
                show_middlecards_message(chosenCards);
                ui->firstMessage->setText("");
                socket2->write(toWrite(SHOW_PLAY_MESSAGE, json_temp));

                if (isEnded){
                    int flag = arr[2 + num].toInt();
                    if (flag == 0 && ui->myStatus->text() == "农民")
                        ui->myMessage->setText("你赢了");
                    else
                        ui->myMessage->setText("你输了");
                    ui->restartButton->setVisible(true);
                    ui->exitButton->setVisible(true);
                    socket2->write(toWrite(END_GAME, QJsonArray({flag})));
                }
                else{
                    lastPlayed = start;
                    int format = arr[2 + num].toInt();
                    int argv1 = arr[3 + num].toInt();
                    int argv2 = arr[4 + num].toInt();
                    decide_play(format, argv1, argv2);
                }
                break;
            }
            case ASK_RESTART:
                restartCnt |= 2;
                if (restartCnt == 7)
                    restart_game();
                break;
            case LAST_PAGE:
                emit closed();
                break;
        }
    }
}

void PlayWidget::second_read()
{
    QJsonArray msg_list = toRead(socket2);
    foreach (auto msg_value, msg_list){
        QJsonObject msg = msg_value.toObject();
        switch(msg["REQUEST"].toInt()){
            case CALL_FINISH:
            {
                bool isCalled = msg["ARGV"].toArray()[0].toBool();
                isCalledList.append(isCalled);
                show_call_messages(2, isCalled);
                socket->write(toWrite(SHOW_CALL_MESSAGE,QJsonArray({1, isCalled})));
                if (start == 0)
                    decide_lanlords();
                else
                    call_landlords();
                break;
            }
            case NOT_PLAY:
            {
                QJsonArray arr = msg["ARGV"].toArray();
                int format = arr[0].toInt();
                int argv1 = arr[1].toInt();
                int argv2 = arr[2].toInt();
                decide_play(format, argv1, argv2);
                ui->secondMessage->setText("不出");
                socket->write(toWrite(SHOW_NOT_PLAY_MESSAGE, QJsonArray({1})));
                break;
            }
            case PLAY:{
                QJsonArray arr = msg["ARGV"].toArray();
                QJsonArray json_temp;

                int num = arr[0].toInt();
                json_temp.append(arr[0]);
                QList<int> chosenCards;
                for (int i = 1; i < 1+num; i++){
                    chosenCards.append(arr[i].toInt());
                    json_temp.append(arr[i]);
                }
                json_temp.append(1);
                bool isEnded = arr[1 + num].toBool();
                show_cards_messagess(2, -num);
                show_middlecards_message(chosenCards);
                ui->secondMessage->setText("");
                socket->write(toWrite(SHOW_PLAY_MESSAGE, json_temp));
                if (isEnded){
                    int flag = arr[2 + num].toInt();
                    if (flag == 0 && ui->myStatus->text() == "农民")
                        ui->myMessage->setText("你赢了");
                    else
                        ui->myMessage->setText("你输了");
                    ui->restartButton->setVisible(true);
                    ui->exitButton->setVisible(true);
                    socket->write(toWrite(END_GAME, QJsonArray({flag})));
                }
                else{
                    int format = arr[2 + num].toInt();
                    int argv1 = arr[3 + num].toInt();
                    int argv2 = arr[4 + num].toInt();
                    lastPlayed = start;
                    decide_play(format, argv1, argv2);
                }
                break;
            }
            case ASK_RESTART:
                restartCnt |= 4;
                if (restartCnt == 7)
                    restart_game();
                break;
            case LAST_PAGE:
                emit closed();
                break;
        }
    }
}

void PlayWidget::on_exitButton_clicked()
{
    this->close();
    emit closed();
}

void PlayWidget::on_callButton_clicked()
{
    ui->callButton->setVisible(false);
    ui->notCallButton->setVisible(false);
    ui->myMessage->setText("叫地主");
    finish_call_lanlords(true);
}

void PlayWidget::on_notCallButton_clicked()
{
    ui->callButton->setVisible(false);
    ui->notCallButton->setVisible(false);
    ui->myMessage->setText("不叫");
    finish_call_lanlords(false);
}


void PlayWidget::on_playButton_clicked()
{
    if (formCheck.isValid(selectCards)){
        ui->myMessage->setText("");
    }
    else{
        ui->myMessage->setText("不能这样出牌");
        return;
    }
    isPlaying = false;
    ui->playButton->setVisible(false);
    ui->notPlayButton->setVisible(false);
    QJsonArray json_tmp;
    json_tmp.append(selectCards.size());
    std::sort(selectCards.begin(), selectCards.end());
    for (int i = 0; i < selectCards.size(); i++){
        int idx = cards.indexOf(selectCards[i]);
        cards.removeAt(idx);
        myPic.removeAt(idx);
        json_tmp.append(selectCards[i]);
    }
    show_mycards_messagess();
    show_middlecards_message(selectCards);
    Format form_temp = Format::getFormat(selectCards);
    if (isMaster){
        socket->write(toWrite(SHOW_PLAY_MESSAGE, json_tmp + 2));
        socket2->write(toWrite(SHOW_PLAY_MESSAGE, json_tmp + 1));
        if (cards.size() != 0){
            lastPlayed = start;
            decide_play(form_temp.format, form_temp.argv[0], form_temp.argv[1]);
        }
        else{
            ui->myMessage->setText("你赢了");
            ui->restartButton->setVisible(true);
            ui->exitButton->setVisible(true);
            int flag = 1;
            if (ui->myStatus->text() == "地主")
                flag = 1;
            else
                flag = 0;
            socket->write(toWrite(END_GAME, QJsonArray({flag})));
            socket2->write(toWrite(END_GAME, QJsonArray({flag})));
        }
    }
    else{
        if (cards.size() == 0){
            ui->myMessage->setText("你赢了");
            ui->restartButton->setVisible(true);
            ui->exitButton->setVisible(true);
            json_tmp.append(true);
            int flag = 1;
            if (ui->myStatus->text() == "地主")
                flag = 1;
            else
                flag = 0;
            json_tmp.append(flag);
        }
        else {
            json_tmp.append(false);
            json_tmp.append(form_temp.format);
            for (int i = 0; i < 2; i++)
                json_tmp.append(form_temp.argv[i]);
        }

        socket->write(toWrite(PLAY, json_tmp));
    }
}

void PlayWidget::on_notPlayButton_clicked()
{
    isPlaying = false;
    if (isMaster){
        socket->write(toWrite(SHOW_NOT_PLAY_MESSAGE, QJsonArray({2})));
        socket2->write(toWrite(SHOW_NOT_PLAY_MESSAGE, QJsonArray({1})));
        decide_play(formCheck.format, formCheck.argv[0], formCheck.argv[1]);
    }
    else{
        QJsonArray json_tmp;
        json_tmp.append(formCheck.format);
        for (int i = 0; i < 2; i++)
            json_tmp.append(formCheck.argv[i]);
        socket->write(toWrite(NOT_PLAY, json_tmp));
    }
    show_mycards_messagess();
    ui->myMessage->setText("不出");
    ui->playButton->setVisible(false);
    ui->notPlayButton->setVisible(false);
}

void PlayWidget::on_restartButton_clicked()
{
    if (isMaster){
        restartCnt |= 1;
        if (restartCnt == 7)
            restart_game();
    }
    else
        socket->write(toWrite(ASK_RESTART));
}
