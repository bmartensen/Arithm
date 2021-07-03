QT += core gui widgets charts

CONFIG += c++11

QMAKE_CXXFLAGS += -Ofast -flto
QMAKE_CXXFLAGS += -BigObj

DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += \
	$$PWD/exprtk \

RC_ICONS = $$PWD/media/Arithm.ico

RESOURCES = Arithm.qrc

SOURCES += \
    arithm_dialog.cpp \
    main.cpp \

HEADERS += \
    arithm_dialog.h \
    exprtk/exprtk.hpp \ \
    settings.h

FORMS += \
    arithm_dialog.ui \

VERSION = 1.2.1

QMAKE_TARGET_COMPANY = "Martensening.com"
QMAKE_TARGET_COPYRIGHT = "Martensening.com"
QMAKE_TARGET_PRODUCT = "Arithm"
QMAKE_TARGET_DESCRIPTION = "Arithmetic Function Parser"

DEFINES += APP_VERSION=\\\"$$VERSION\\\"

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
