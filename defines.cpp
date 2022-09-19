#include "defines.h"
QByteArray toWrite(int request, QJsonArray argv){
    QJsonObject json;
    json["REQUEST"] = request;
    json["ARGV"] = argv;
    QByteArray bytes = QJsonDocument(json).toJson(QJsonDocument::JsonFormat::Compact);
    bytes.append('\n');
    return bytes;
}

QJsonArray toRead(QTcpSocket_ptr socket){
    QJsonArray list;
    while(socket->bytesAvailable()){
        char data[100];
        socket->readLine(data, 100);
        QByteArray buffer(data);
        list.append(QJsonDocument::fromJson(buffer).object());
    }
    return list;
}
Format::Format(){
    format = CARD_TYPE_ALL_COMPARE;
}

Format::Format(int form, int a, int b):
    format(form){
    argv[0] = a;
    argv[1] = b;
}

bool Format::isValid(QList<int>& cards){
    //return true;
    Format thisFormat = getFormat(cards);
    if (thisFormat.format == CARD_TYPE_INVALID || format == CARD_TYPE_KING)
        return false;
    if (thisFormat.format == CARD_TYPE_KING || format == CARD_TYPE_ALL_COMPARE)
        return true;
    if (thisFormat.format == CARD_TYPE_BOMB){
        if (format != CARD_TYPE_BOMB)
            return true;
        if (argv[0] < thisFormat.argv[0])
            return true;
        else
            return false;
    }
    if (thisFormat.format == format){
        if (format == CARD_TYPE_SINGLE || format == CARD_TYPE_DOUBLE || format == CARD_TYPE_FOUR_ONE || format == CARD_TYPE_FOUR_TWO){
            if (thisFormat.argv[0] > argv[0])
                return true;
            else
                return false;
        }
        else{
            if (thisFormat.argv[0] == argv[0] && thisFormat.argv[1] > argv[1])
                return true;
            else
                return false;
        }
    }
    else
        return false;
}
Format Format::getFormat(QList<int>& tcards){
    int len = tcards.size();
    QList<int> cards;
    for (int i = 0; i < len; i++)
        cards.append((tcards[i] > 51)? (tcards[i] - 39) :(tcards[i]/4));
    //check single
    if (len == 1)
        return Format(CARD_TYPE_SINGLE, cards[0]);
    //check double
    if (len == 2){
        if ((cards[0] == 13 && cards[1] == 14) || (cards[0] == 14 && cards[1] == 13))
            return Format(CARD_TYPE_KING);
        if (cards[0] == cards[1])
            return Format(CARD_TYPE_DOUBLE, cards[0]);
        return Format(CARD_TYPE_INVALID);
    }

    std::sort(cards.begin(), cards.end());
    //check continue single
    if (len >= 5 && cards[len - 1] < 12){
        bool isContinueSingle = true;
        for (int i = 1; i < len; i++)
            if (cards[i-1] != cards[i] - 1){
                isContinueSingle = false;
                break;
            }
        if (isContinueSingle)
            return Format(CARD_TYPE_CONTINUOUS_SIGNGLE, len, cards[0]);
    }
    //check continue double
    if (len >= 6 && len % 2 == 0 && cards[len - 1] < 12){
        bool isContinueDouble = true;
        for (int i = 1; i < len; i++)
            if ((i % 2 == 0 && cards[i-1] != cards[i] - 1) || (i % 2 == 1 && cards[i-1] != cards[i])){
                isContinueDouble = false;
                break;
            }
        if (isContinueDouble)
            return Format(CARD_TYPE_CONTINUOUS_DOUBLE, len, cards[0]);
    }
    int cardsIdx[15] = {0};
    for (int i = 0; i < len; i++)
        cardsIdx[cards[i]]++;
    int oneCnt = 0;
    int twoCnt = 0;
    int threeCnt = 0;
    int fourCnt = 0;
    int fourFirst = -1;
    int threeFirst = -1;
    for (int i = 0; i < 13; i++){
        if (cardsIdx[i] == 1)
            oneCnt++;
        if (cardsIdx[i] == 2)
            twoCnt++;
        if (cardsIdx[i] == 3){
            if (threeFirst == -1)
                threeFirst = i;
            threeCnt++;
        }
        if (cardsIdx[i] == 4){
            if (fourFirst == -1)
                fourFirst = i;
            fourCnt++;
        }
    }
    if (cardsIdx[13] == 1 && cardsIdx[14] == 1)
        twoCnt++;
    else if (cardsIdx[13] == 1)
        oneCnt++;
    else if (cardsIdx[14] == 1)
        oneCnt++;
    //check three
    if (threeCnt != 0){
        bool isContinous = true;
        for (int i = threeFirst; i < threeCnt + threeFirst; i++)
            if (cardsIdx[i] != 3){
                isContinous = false;
                break;
            }
        if (threeCnt > 1 && cardsIdx[12] == 3){
            threeCnt--;
            oneCnt++;
            twoCnt++;
        }
        if (isContinous){
            if (oneCnt == 0 && (twoCnt + 2 *fourCnt == threeCnt))
                return Format(CARD_TYPE_THREE_TWO, threeCnt, threeFirst);
            if (oneCnt + twoCnt * 2 + fourCnt * 4 == threeCnt)
                return Format(CARD_TYPE_THREE_ONE, threeCnt, threeFirst);
            if (oneCnt == 0 && twoCnt == 0 && fourCnt == 0)
                return Format(CARD_TYPE_THREE, threeCnt, threeFirst);
        }
    }
    //check four
    if (fourCnt != 0){
        if (fourCnt == 1 && threeCnt == 0 && twoCnt == 0 && oneCnt == 0)
            return Format(CARD_TYPE_BOMB, fourFirst);
        if (fourCnt == 1 && threeCnt == 0 && ((oneCnt == 2 && twoCnt == 0) || (oneCnt == 0 && twoCnt == 1)))
            return Format(CARD_TYPE_FOUR_ONE, fourFirst);
        if (threeCnt == 0 && oneCnt == 0 && ((fourCnt == 1 && twoCnt == 2) || (fourCnt == 2 && twoCnt == 0))) //may change
            return Format(CARD_TYPE_FOUR_TWO, fourFirst);
    }
    return Format(CARD_TYPE_INVALID);
}
