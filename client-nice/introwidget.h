#ifndef INTROWIDGET_H
#define INTROWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

class IntroWidget : public QWidget
{
    Q_OBJECT
public:
    explicit IntroWidget(QWidget *parent = nullptr);

signals:

public slots:

private:
    QVBoxLayout * mainLayout;

    QLabel * imageLabel;
    QLabel * infoLabel;
    QLabel * infoTitle;
};

#endif // INTROWIDGET_H
