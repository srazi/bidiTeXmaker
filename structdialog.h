/***************************************************************************
 *   copyright       : (C) 2003-2009 by Pascal Brachet                     *
 *   http://www.xm1math.net/texmaker/                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef STRUCTDIALOG_H
#define STRUCTDIALOG_H

#include "ui_structdialog.h"


class StructDialog : public QDialog  {
   Q_OBJECT
public:
	StructDialog(QWidget *parent=0, QString name="");
	~StructDialog();
	Ui::StructDialog ui;

};

#endif
