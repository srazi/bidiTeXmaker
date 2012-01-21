/***************************************************************************
 *   copyright       : (C) 2003-2009 by Pascal Brachet                     *
 *   addons by Frederic Devernay <frederic.devernay@m4x.org>               *
 *   addons by Luis Silvestre                                              *
 *   http://www.xm1math.net/texmaker/                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "texmakerapp.h"
#include <QStringList>

//DDE Support
#ifdef Q_WS_WIN
#include "winuser.h"
#endif

/////////////////////////////////////////////////
//added by S. R. Alavizadeh
//Extra Feature: easy portable
bool isPortable;
/////////////////////////////////////////////////

int main( int argc, char ** argv )
{
#ifdef Q_WS_WIN
#ifdef D_MSVC_CC
//This allows other foreground process to set texmaker to foreground
//we need use this code in the beginning of the application
AllowSetForegroundWindow(ASFW_ANY);
#endif
#endif
TexmakerApp app("bidiTeXmaker", argc, argv ); // This is a dummy constructor so that the programs loads fast.

QStringList args = QCoreApplication::arguments();//add by S. R. Alavizadeh
//for ( int i=0; i < argc; i++) args += QString::fromLocal8Bit(argv[i]);

if ( app.isRunning() ) 
    {
    QString msg;
    msg = args.join("#!#");
    msg += "#!#";
    app.sendMessage( msg );
    return 0;
    }

app.init(args); // Initialization takes place only if there is no other instance running.

app.setActivationWindow(app.mw, true);//activating window on reciving message

QObject::connect( &app, SIGNAL( messageReceived(const QString &) ), 
                  app.mw,   SLOT( onOtherInstanceMessage(const QString &) ) );

return app.exec();
}
