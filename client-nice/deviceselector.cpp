#include "deviceselector.h"

DeviceSelector::DeviceSelector(QWidget *parent) : QWidget(parent)
{
    mainLayout = new QHBoxLayout;
    setLayout(mainLayout);

    switchBinary = new ButtonIcon{QSize(this->width() / 2, this->height()), ":/res/imgs/binary.png"};
    switchMultilevel = new ButtonIcon{QSize(this->width() / 2, this->height()), ":/res/imgs/multilevel.png"};

    connect(switchBinary, SIGNAL(clicked()),
            this, SIGNAL(switchBinarySelected()));
    connect(switchMultilevel, SIGNAL(clicked()),
            this, SIGNAL(switchMultilevelSelected()));

    mainLayout->addWidget(switchBinary);
    mainLayout->addWidget(switchMultilevel);
}
