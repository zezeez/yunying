#-------------------------------------------------
#
# Project created by QtCreator 2019-02-14T23:09:40
#
#-------------------------------------------------

QT       += core gui network multimedia charts xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = yuny
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += HAS_CDN

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

INCLUDEPATH += include
# DEPENDPATH = .

SOURCES += \
    lib/base64/src/base64.cpp \
    lib/dns/src/buffer.cpp \
    lib/dns/src/message.cpp \
    lib/dns/src/qs.cpp \
    lib/dns/src/rr.cpp \
    lib/ntp/src/ntp.cpp \
    src/analysis.cpp \
    src/cdn.cpp \
    src/chart/barchart.cpp \
    src/chart/linechart.cpp \
    src/frozentrain.cpp \
    src/loginconf.cpp \
    src/logindialog.cpp \
        src/main.cpp \
        src/mainwindow.cpp \
    src/passengerdialog.cpp \
    src/seatdialog.cpp \
    src/seattypedialog.cpp \
    src/selltimequerydialog.cpp \
    src/serverip.cpp \
    src/settingdialog.cpp \
    src/sliderwidget.cpp \
    src/sysutil.cpp \
    src/trainnodialog.cpp \
    src/upgrademanager.cpp \
    src/userdata.cpp \
    src/nethelper.cpp \
    src/completeredit.cpp \
    lib/smtp/src/smtp.cpp \
    lib/sm4/src/sm4.cpp

HEADERS += \
    include/12306.h \
    include/analysis.h \
    include/cdn.h \
    include/chart/barchart.h \
    include/chart/linechart.h \
    include/cookieapi.h \
    include/frozentrain.h \
    include/icondelegate.h \
    include/loginconf.h \
    include/logindialog.h \
        include/mainwindow.h \
    include/nethelper.h \
    include/passengerdialog.h \
    include/seatdialog.h \
    include/seattypedialog.h \
    include/selltimequerydialog.h \
    include/serverip.h \
    include/settingdialog.h \
    include/sliderwidget.h \
    include/sysutil.h \
    include/trainnodialog.h \
    include/upgrademanager.h \
    include/userdata.h \
    include/completeedit.h \
    include/version.h \
    lib/base64/include/base64.h \
    lib/dns/include/buffer.h \
    lib/dns/include/dns.h \
    lib/dns/include/exception.h \
    lib/dns/include/message.h \
    lib/dns/include/qs.h \
    lib/dns/include/rr.h \
    lib/ntp/include/ntp.h \
    lib/smtp/include/smtp.h \
    lib/sm4/include/sm4.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc

DISTFILES += \
    main.qss

RC_ICONS = images/ticket.ico

#win32 {
#    LIBS += -lUser32
#}

unix {
    qml_debug {
        QMAKE_CXXFLAGS += -fsanitize=address -fno-omit-frame-pointer -fsanitize-recover=address
        LIBS += -lasan
    }
}


