QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
CONFIG += no_keywords

QMAKE_CXXFLAGS += -D_GLIBCXX_USE_CXX11_ABI=0

INCLUDEPATH += /usr/local/include

INCLUDEPATH += /Library/Frameworks/Python.framework/Versions/3.7/lib/python3.7/site-packages/torch/include
DEPENDPATH += /Library/Frameworks/Python.framework/Versions/3.7/lib/python3.7/site-packages/torch/include

LIBS += -L/Library/Frameworks/Python.framework/Versions/3.7/lib/python3.7/site-packages/torch/lib -ltorch -lc10

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ai.cpp \
    aicontroller.cpp \
    aimcts.cpp \
    board.cpp \
    boardview.cpp \
    direction.cpp \
    gobang.cpp \
    main.cpp \
    mainwindow.cpp \
    modedialog.cpp \
    placeevent.cpp \
    testboard.cpp \
    zobrist.cpp

HEADERS += \
    ai.h \
    aicontroller.h \
    aimcts.h \
    board.h \
    boardview.h \
    direction.h \
    gobang.h \
    mainwindow.h \
    modedialog.h \
    placeevent.h \
    testboard.h \
    zobrist.h

FORMS += \
    mainwindow.ui \
    modedialog.ui

RESOURCES +=


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
