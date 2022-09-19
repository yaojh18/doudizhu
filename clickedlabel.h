#ifndef CLICKEDLABEL_H
#define CLICKEDLABEL_H

#include <QWidget>
#include <QLabel>
#include <QPixmap>

class ClickedLabel : public QLabel
{
    Q_OBJECT
public:
    ClickedLabel(int num, QWidget *parent=0);
    ~ClickedLabel() {}
signals:
    void clicked(ClickedLabel* click);
protected:
    void mousePressEvent(QMouseEvent*);

public:
    bool isClicked;
};

#endif // CLICKEDLABEL_H
