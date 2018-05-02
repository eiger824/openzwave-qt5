#include "mainwindow.h"
#include "pagestack.h"

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent)
{
    setFixedSize(QSize(800, 480));
    QVBoxLayout * ml = new QVBoxLayout;
    setLayout(ml);
    ml->addWidget(new PageStack);
}

MainWindow::~MainWindow()
{
}
