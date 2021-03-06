#!/bin/sh
echo "Texmaker compilation :"
echo "----------------------------------------"
echo "Enter path to QT4 (/usr/lib/qt4 or ...):"
read QTDIR
echo "Enter SYSTEM (1: UNIX ; 2: MACOSX) :"
read SYSTEM
PATH=$QTDIR/bin:$PATH
LD_LIBRARY_PATH=$QTDIR/lib:$LD_LIBRARY_PATH
DYLD_LIBRARY_PATH=$QTDIR/lib:$DYLD_LIBRARY_PATH
export QTDIR PATH LD_LIBRARY_PATH DYLD_LIBRARY_PATH
if [ "$SYSTEM" = 1 ] 
then
echo "Enter PREFIX (/usr/local , /usr or /opt) :"
read PREFIX
echo "Enter  DIRECTORY for the desktop file (/usr/share/applications) :"
read DESKTOPDIR
echo "Enter  DIRECTORY for the icon file (/usr/share/pixmaps) :"
read ICONDIR
qmake -unix PREFIX=$PREFIX DESKTOPDIR=$DESKTOPDIR ICONDIR=$ICONDIR biditexmaker.pro
make
make install
echo "Compilation and installation done"
# set the -spec option, if necessary. Ex : qmake -unix -spec linux-g++ PREFIX=$PREFIX biditexmaker.pro
exit 0
fi

if [ "$SYSTEM" = 2 ] 
then
qmake -macx -spec macx-g++ biditexmaker.pro
make
make install
echo "Compilation and installation done"
exit 0
fi

exit 0

