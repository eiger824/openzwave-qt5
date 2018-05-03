#ifndef PAGESTACK_H
#define PAGESTACK_H

#include <QWidget>
#include <QStackedWidget>
#include <QProgressBar>
#include <QVBoxLayout>

#include "utils/direction_button.h"

class PageStack : public QWidget
{
    Q_OBJECT
public:
    explicit PageStack(QWidget *parent = Q_NULLPTR);
    ~PageStack();

signals:

private slots:
    void loadLastPage();
    void loadNextPage();
    void switchBinarySelectedSlot();
    void switchMultilevelSelectedSlot();
    void returnToMainPage();

private:
    void checkPages();

private:
    QStackedWidget * stack;
    QProgressBar * bar;
    ButtonIcon * fwd;
    ButtonIcon * bkw;

    // Layouts
    QVBoxLayout * mainLayout;
};

#endif // PAGESTACK_H
