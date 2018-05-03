#include <QDebug>

#include "operationcomplete.h"

OperationComplete::OperationComplete(QLabel *parent)
    : QLabel(parent)
{
    mainLayout = new QHBoxLayout;
    setLayout(mainLayout);

    QLabel * info = new QLabel("Done! Tap anywhere to return to main screen.");
    info->setWordWrap(true);
    info->setFixedSize(this->width() * 0.5, this->height() * 0.3);
    info->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    info->setContentsMargins(0,0,0,0);

    QFont font;
    font.setFamily("Lato");
    font.setPixelSize(32);
    info->setFont(font);

    mainLayout->addWidget(info);
}

void OperationComplete::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "Returning to main screen!";
    emit clicked();
}
