QT -= gui

QT  += core dbus

CONFIG += c++17 console
CONFIG -= app_bundle

TARGET = ozw-proxy-client

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QMAKE_INCDIR += ../common ${OZW_SRC}

SOURCES += main.cpp \
    openzwaveproxyclient.cpp

HEADERS += \
    openzwaveproxyclient.h

DBUS_ADAPTORS   += ../common/dbus/se.mysland.openzwave.xml
DBUS_INTERFACES += ../common/dbus/se.mysland.openzwave.xml

DISTFILES += \
    ../common/dbus/se.mysland.openzwave.xml

target.path = /usr/bin

INSTALLS += target
