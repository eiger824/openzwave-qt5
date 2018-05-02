#include <QDebug>

#include "direction_button.h"

ButtonIcon::ButtonIcon(QSize const & s, QString const & logo, QWidget * parent, Qt::WindowFlags f)
    : imageLogo(logo),
      QLabel(parent)
{
    setFixedSize(s);

    QPixmap p{logo};
    p.scaledToWidth(s.width());
    setPixmap(p);

    setScaledContents(true);
}

ButtonIcon::~ButtonIcon()
{

}

void ButtonIcon::disableClicks()
{
    if (clicks)
    {
        clicks = false;
        // Remove the image
        QPixmap p{};
        setPixmap(p);
    }
}

void ButtonIcon::enableClicks()
{
    if (!clicks)
    {
        clicks = true;
        QPixmap p {imageLogo};
        p.scaledToWidth(this->width());
        setPixmap(p);
    }
}

void ButtonIcon::mousePressEvent(QMouseEvent * event)
{
    if (clicks)
        emit clicked();
}
