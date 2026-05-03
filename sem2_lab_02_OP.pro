QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    entrypoint.c \
    file_loader.c \
    iterator.c \
    list.c \
    logic.c \
    main.cpp \
    mainwindow.cpp \
    parser.c \
    metrics.c

HEADERS += \
    appcontext.h \
    demography_record.h \
    entrypoint.h \
    file_loader.h \
    iterator.h \
    list.h \
    logic.h \
    mainwindow.h \
    parser.h \
    metrics.h

FORMS += \
    mainwindow.ui

#DISTFILES += \
#    README.md \
#    russian_demography.csv
#    empty.csv


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
