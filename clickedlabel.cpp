#include "clickedlabel.h"
#include <QtDebug>
ClickedLabel::ClickedLabel(int num, QWidget *parent):
    QLabel(parent)
{
    isClicked = false;
    setScaledContents(true);
    setPixmap(QPixmap(":/image/images/" + QString::number(num) + ".png"));
    setStyleSheet("background-color: rgb(0,0,0,0)");
    connect(this, SIGNAL(clicked(ClickedLabel*)), parent, SLOT(label_clicked(ClickedLabel*)));
    show();
}

void ClickedLabel::mousePressEvent(QMouseEvent *)
{
    emit clicked(this);
}
