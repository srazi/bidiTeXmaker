#for shared lib comment out the below line.
DEFINES += QBIDIEXTENDER_EMBED

HEADERS	+= $$PWD/qbidiextender2.h
SOURCES	+= $$PWD/qbidiextender2.cpp
INCLUDEPATH += $$PWD/.
RESOURCES += $$PWD/qbidiextender2.qrc

win32 {
LIBS += -luser32
win32-msvc*{
DEFINES += D_MSVC_CC
GUID = {AE276D93-FB6D-3DF7-8512-F8A027A84100}
}

win32-g++{
DEFINES += D_MINGW_CC
}
}

unix:!macx {
HEADERS	+= $$PWD/xkb/XKeyboard.h \
           $$PWD/xkb/X11Exception.h
SOURCES	+= $$PWD/xkb/XKeyboard.cpp

LIBS += -L/usr/lib -lX11
}
