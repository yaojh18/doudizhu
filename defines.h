#ifndef DEFINES_H
#define DEFINES_H
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QSharedPointer>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QMap>

#define NEXT_PAGE 100
#define LAST_PAGE 101

#define DEAL 102
#define CALL 103
#define CALL_FINISH 104

#define ADD_CARDS 105
#define PLAY_CARDS 106
#define NOT_PLAY 108
#define PLAY 109

#define SHOW_CALL_MESSAGE 110
#define SHOW_CARDS_MESSAGE 111
#define SHOW_NOT_PLAY_MESSAGE 112
#define SHOW_PLAY_MESSAGE 113

#define BEGIN_GAME 115
#define RESTART_GAME 116
#define ASK_RESTART 117
#define END_GAME 118

#define CARD_TYPE_ALL_COMPARE 201
#define CARD_TYPE_INVALID 202
#define CARD_TYPE_SINGLE  203
#define CARD_TYPE_DOUBLE  204
#define CARD_TYPE_THREE  205
#define CARD_TYPE_THREE_ONE 206
#define CARD_TYPE_THREE_TWO  207
#define CARD_TYPE_BOMB  208
#define CARD_TYPE_FOUR_ONE  209
#define CARD_TYPE_FOUR_TWO  210
#define CARD_TYPE_CONTINUOUS_SIGNGLE  211
#define CARD_TYPE_CONTINUOUS_DOUBLE  212
#define CARD_TYPE_KING  213


typedef QSharedPointer<QTcpServer> QTcpServer_ptr;
typedef QSharedPointer<QTcpSocket> QTcpSocket_ptr;

QByteArray toWrite(int request, QJsonArray argv = {});
QJsonArray toRead(QTcpSocket_ptr socket);

class PlayWideget;

class Format{
    int format;
    int argv[2];
public:
    Format();
    Format(int form, int a = 0, int b = 0);
    bool isValid(QList<int>& cards);
    static Format getFormat(QList<int>& tcards);
    friend class PlayWidget;
};

#endif // DEFINES_H
