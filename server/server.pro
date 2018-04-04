#-------------------------------------------------
#
# Project created by QtCreator 2018-04-03T11:19:41
#
#-------------------------------------------------

QT       += core gui dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ozw-daemon
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


QMAKE_CXXFLAGS += -g -fPIC
QMAKE_INCDIR += ${OZW_SRC} ${OZW_AES} ${OZW_CMD_CLASSES} ${OZW_PLATFORM} ${OZW_VALUE_CLASSES}
QMAKE_INCDIR += ../common

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        ../common/timespec.cpp

HEADERS += \
        mainwindow.h \
    defs.h \
    ../common/timespec.h

FORMS += \
        mainwindow.ui

DBUS_ADAPTORS   += ../common/dbus/se.mysland.openzwave.xml
DBUS_INTERFACES += ../common/dbus/se.mysland.openzwave.xml

DISTFILES += \
    ../common/dbus/se.mysland.openzwave.xml

QMAKE_LFLAGS += -fPIC -pthread
QMAKE_LIBS += -L${OZW_ROOT} -lopenzwave

RESOURCES += \
    resources.qrc
