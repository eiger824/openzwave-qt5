#include <QHBoxLayout>

#include "introwidget.h"

IntroWidget::IntroWidget(QWidget *parent) : QWidget(parent)
{
    mainLayout = new QVBoxLayout{this};
    setLayout(mainLayout);

    QHBoxLayout * hl = new QHBoxLayout;
    QVBoxLayout * vl1 = new QVBoxLayout;

    infoTitle = new QLabel{"Welcome to the OpenZWave client!"};
    infoTitle->setWordWrap(true);
    infoTitle->setFixedSize(this->width() * 0.5, this->height() * 0.3);
    infoTitle->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    infoTitle->setContentsMargins(0,0,0,0);

    QFont font;
    font.setFamily("Lato");
    font.setPixelSize(32);
    infoTitle->setFont(font);

    infoLabel = new QLabel{"Please use the side buttons to navigate through the screens"};
    infoLabel->setWordWrap(true);
    infoLabel->setFixedSize(this->width() * 0.5, this->height() * 0.15);
    infoLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    infoLabel->setContentsMargins(0,0,0,0);
    font.setFamily("Lato");
    font.setPixelSize(18);
    infoLabel->setFont(font);

    vl1->addWidget(infoTitle);
    vl1->addWidget(infoLabel);
    hl->addLayout(vl1);

    imageLabel = new QLabel{this};
    QPixmap p {":/res/imgs/ozwlogo.png"};
    p.scaledToWidth(this->width());

    imageLabel->setPixmap(p);
    imageLabel->setScaledContents(true);

    mainLayout->addLayout(hl);
    mainLayout->addWidget(imageLabel);
}
