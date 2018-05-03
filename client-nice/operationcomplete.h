#ifndef OPERATIONCOMPLETE_H
#define OPERATIONCOMPLETE_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>

class OperationComplete : public QLabel
{
    Q_OBJECT
public:
    explicit OperationComplete(QLabel * parent = Q_NULLPTR);

signals:
    void clicked();
protected:
    void mousePressEvent(QMouseEvent * event);

private:
    QHBoxLayout * mainLayout;

};

#endif // OPERATIONCOMPLETE_H
