#include <QDebug>

#include "definitions.h"
#include "pagestack.h"
#include "introwidget.h"
#include "deviceselector.h"

PageStack::PageStack(QWidget *parent) : QWidget(parent)
{
    // Init layout
    mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    // Add a horizontal layout
    QHBoxLayout * layout = new QHBoxLayout;
    mainLayout->addLayout(layout);

    bkw = new ButtonIcon(QSize(60, 60), ":/res/imgs/arrow_left.png", this);
    fwd = new ButtonIcon(QSize(60, 60), ":/res/imgs/arrow_right.png", this);

    connect(bkw, SIGNAL(clicked()),
            this, SLOT(loadLastPage()));
    connect(fwd, SIGNAL(clicked()),
            this, SLOT(loadNextPage()));

    stack = new QStackedWidget(this);
    stack->addWidget(new IntroWidget());

    DeviceSelector * selector = new DeviceSelector;
    connect(selector, SIGNAL(switchBinarySelected()),
            this, SLOT(switchBinarySelectedSlot()));
    connect(selector, SIGNAL(switchMultilevelSelected()),
            this, SLOT(switchMultilevelSelectedSlot()));
    stack->addWidget(selector);

    stack->addWidget(new QWidget());
    stack->addWidget(new QWidget());
    stack->addWidget(new QWidget());

    layout->addWidget(bkw);
    layout->addWidget(stack);
    layout->addWidget(fwd);

    bar = new QProgressBar(this);

    mainLayout->addWidget(bar);

    checkPages();
}

PageStack::~PageStack()
{
    delete stack;
    delete bar;
    delete fwd;
    delete bkw;
    delete qobject_cast<QHBoxLayout*>(mainLayout->itemAt(0)->layout());
}

void PageStack::loadLastPage()
{


    int index = stack->currentIndex();
    if (index > 0)
    {
        qDebug() << "Going back";
        stack->setCurrentIndex(index - 1);
    }

    checkPages();
}

void PageStack::loadNextPage()
{


    int index = stack->currentIndex();
    if (index < stack->count() - 1)
    {
        qDebug() << "Going forward";
        stack->setCurrentIndex(index + 1);
    }
    checkPages();
}

void PageStack::switchBinarySelectedSlot()
{
    stack->setCurrentIndex(SWITCH_BINARY_INDEX);
    checkPages();
}

void PageStack::switchMultilevelSelectedSlot()
{
    stack->setCurrentIndex(SWITCH_MULTILEVEL_INDEX);
    checkPages();
}

/*
 * Checks whether the page buttons should be enabled or not
*/
void PageStack::checkPages()
{
    int index = stack->currentIndex();
    qDebug() << "Currently on index " << index << " / " << stack->count();

    // No widgets yet, then disable all buttons
    if (index == -1)
    {
        bkw->disableClicks();
        fwd->disableClicks();
    }
    else
    {
        if (0 < index and index < stack->count() - 1)
        {
            // Enable clicks
            bkw->enableClicks();
            fwd->enableClicks();
        }
        else
        {
            if (index == 0)
            {
                bkw->disableClicks();
            }
            else
            {
                fwd->disableClicks();
            }
        }
    }
    bar->setValue((stack->count() > 1) ?
                      index * bar->maximum() / (stack->count() - 1) :
                      bar->maximum());
}
