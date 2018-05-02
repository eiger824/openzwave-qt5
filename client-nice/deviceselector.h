#ifndef DEVICESELECTOR_H
#define DEVICESELECTOR_H

#include <QWidget>
#include <QHBoxLayout>

#include "utils/direction_button.h"

class DeviceSelector : public QWidget
{
    Q_OBJECT
public:
    explicit DeviceSelector(QWidget *parent = nullptr);

signals:
    void switchBinarySelected();
    void switchMultilevelSelected();

private:
    QHBoxLayout * mainLayout;

    // TODO: get this information dynamically, now assumming two devices
    ButtonIcon * switchBinary;
    ButtonIcon * switchMultilevel;
};

#endif // DEVICESELECTOR_H
