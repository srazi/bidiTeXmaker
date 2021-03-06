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


#ifndef LATEXHIGHLIGHTER_H
#define LATEXHIGHLIGHTER_H


#include <QSyntaxHighlighter>
#include <QHash>
#include <QTextCharFormat>
#include <QColor>
#include <QTextBlockUserData>
//#include "latexeditor.h"
#include "hunspell/hunspell.hxx"

/////////////////////////////////////////////////
//added by S. R. Alavizadeh
//Bi-Di Support
#include "qbidiextender2.h"
/////////////////////////////////////////////////

class QTextDocument;
class LatexEditor;

class LatexHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    LatexHighlighter(QTextDocument *parent = 0,bool spelling=false, QString ignoredWords="",Hunspell *spellChecker=0);
    ~LatexHighlighter();
    QColor ColorStandard, ColorComment, ColorMath, ColorCommand, ColorKeyword, ColorVerbatim, ColorTodo, ColorKeywordGraphic, ColorNumberGraphic;
    QStringList KeyWords, KeyWordsGraphic, KeyWordsGraphicBis;
    QTextCharFormat spellingErrorFormat;
QStringList alwaysignoredwordList;

void reComputeKeywords();
inline void setHighlighterEnabled(bool enable) { m_highlighterEnabled = enable; }

public slots:
void SetAlwaysIgnoredWords(QString ignoredWords);
void setColors(QList<QColor> colors);
void setSpellChecker(Hunspell * checker);
void activateInlineSpell(bool enable);
void SetEditor(LatexEditor *ed);
void setModeGraphic(bool m);
private :
QHash<QString, QRegExp> kewordsRegExpList;
QHash<QString, QString> envsPatternList;

QTextCharFormat structFormat;
QTextCharFormat asyFormat;
QTextCharFormat todoFormat;

QBrush brushstandard;
QBrush brushverbatim;
QBrush brushmath;
QBrush brushcomment;

bool m_highlighterEnabled;
QString spell_dic, spell_encoding;
QStringList ignoredwordList, hardignoredwordList;
Hunspell * pChecker;
bool checkSpelling;
bool isWordSeparator(QChar c) const;
bool isSpace(QChar c) const;
bool isGraphic;
protected:
LatexEditor *editor;
void highlightBlock(const QString &text);
};


#endif
