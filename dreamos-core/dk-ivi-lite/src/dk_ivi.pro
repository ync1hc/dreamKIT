QT += quick core gui network
QT += widgets
#QT += virtualkeyboard
CONFIG += c++11
CONFIG += console
CONFIG += no_keywords

QMAKE_CXXFLAGS += -fpermissive

# The following define makes your compiler Q_EMIT warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Refer to the documentation for the
# deprecated API to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main/main.cpp \
    library/vapiclient/vapiclient.cpp

RESOURCES += main/main.qml \
    main/settings.qml \
    uss/uss.qml

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH = 

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH = 

# Default rules for deployment.
qnx: target.path = /tmp/${TARGET}/bin
else: unix:!android: target.path = /opt/${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    library/vapiclient/vapiclient.hpp
 \

INCLUDEPATH += library/vapiclient


# this is for Orin
# LIBS += -lrt
# LIBS += -L$$PWD/library/target/sk_3.8/ -lKuksaClient -lrt
# Set base path
LIB_BASE = $$PWD/library/target

# Detect architecture and set the subdirectory
contains(QT_ARCH, arm64) | contains(QT_ARCH, aarch64) {
    LIB_DIR = $$LIB_BASE/arm64
} else:contains(QT_ARCH, x86_64) {
    LIB_DIR = $$LIB_BASE/amd64
} else {
    error("Unsupported architecture: $$QT_ARCH")
}

INCLUDEPATH += $$PWD/../library/include
LIBS += -L$$LIB_DIR -lKuksaClient -lrt


CONFIG += SOCKET_IT_CLIENT_CONFIG

SOCKET_IT_CLIENT_CONFIG {
    DEFINES += USING_SOCKET_IO_CLIENT
    LIBS += -lssl -lcrypto
}


# this is for NUC
#LIBS += -lrt

