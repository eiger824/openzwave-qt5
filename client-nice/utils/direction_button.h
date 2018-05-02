#ifndef DIRECTION_BUTTON_H_
#define DIRECTION_BUTTON_H_

#include <QObject>
#include <QLabel>
#include <QWidget>
#include <QSize>
#include <QMouseEvent>
#include <QString>

class ButtonIcon : public QLabel
{
    Q_OBJECT
public:
    explicit ButtonIcon(QSize const & s, QString const & logo, QWidget * parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~ButtonIcon();

public:
    inline bool clicksEnabled() const
    {
        return clicks;
    }
    void disableClicks();
    void enableClicks();

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent * event);

private:
    bool clicks {true};
    QString imageLogo;
};

#endif /* DIRECTION_BUTTON_H_ */
