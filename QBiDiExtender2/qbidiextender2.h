/***************************************************************************
 *   copyright       : (C) 2009-2012 by S. Razi Alavizadeh                 *
 *   WWW: http://pozh.org                                                  *
 *   email: S.R.Alavizadeh@gmail.com                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef QBIDIEXTENDER_H
#define QBIDIEXTENDER_H

#include <QTextEdit>
#include <QObject>
#include <QAction>
#include <QMessageBox>
#include <QString>
#include <QApplication>
#include <QTextCursor>
#include <QDesktopServices>
#include <QUrl>
#include <QTextBlock>
#include <QWidget>
#include <QScrollBar>
#include <QTextLayout>
#include <QFile>
#include <QFileInfo>
#include <QMenuBar>
#include <QPushButton>
#include <QTabWidget>
#include <QDateTime>
#include <QTextStream>
#include <QTextCodec>
#include <QSettings>
#include <QToolBar>

#if defined( Q_WS_WIN )
#include "Windows.h"
#endif


#define LRM 0x200E 
#define RLM 0x200F
#define LRE 0x202A
#define RLE 0x202B
#define PDF 0x202C
#define LRO 0x202D
#define RLO 0x202E

 #include <QtCore/QtGlobal>
 #if defined(QBIDIEXTENDER_EMBED)
	#define QBIDIEXTENDER_EXPORT
 #else
	 #if defined(QBIDIEXTENDER_LIB)
		#define QBIDIEXTENDER_EXPORT Q_DECL_EXPORT
	 #else
		#define QBIDIEXTENDER_EXPORT Q_DECL_IMPORT
	 #endif
#endif

#define CLEAR_CONTROL_CHARS(x) QBiDiExtender::removeUnicodeControlCharacters(x)
#define UNICODE_CONTROL_CHARS QBiDiExtender::unicodeControlCharacters()
#define PATTERN_BY_CONTROL_CHARS(x) QBiDiExtender::toPatternWithUnicodeControlCharacters(x)
#define REGEXP_BY_CONTROL_CHARS(x) QRegExp(QBiDiExtender::toPatternWithUnicodeControlCharacters(x))

#define PATTERN_BY_CONTROL_CHARS_FAST(x) QBiDiExtender::toPatternWithUnicodeControlCharactersFast(x)
#define REGEXP_BY_CONTROL_CHARS_FAST(x) QRegExp(QBiDiExtender::toPatternWithUnicodeControlCharactersFast(x))

class QBIDIEXTENDER_EXPORT QBiDiInitializer : public QObject
{
	Q_OBJECT
public:
	QBiDiInitializer(QWidget *qwidget=0);
	~QBiDiInitializer();

	void setParentLayoutDirection(Qt::LayoutDirection direction);
	QString openPtdFile(const QString &f, QTextCodec* codec, const QString &plainContent = "");
	void applicationBiDiMenu(QMenuBar *menuBar, QMenu *menu, const QString &type);
	void autoNewLine(bool selected);
	void setEditorActionState(bool enable);
	QToolBar *installBiDiToolBar();

private slots:
	void BiDiHelp();
	void aboutBidiPatch();
	void toggleAutoChangeLangState(bool checked);
	void toggleBiDiMode(QAction *action);
	
private:
	QToolBar *bidiToolBar;
	QWidget *editorWidget;
	QActionGroup *bidiModes;
	QAction *actFirstBiDiMode;
	QAction *actLTR, *actRTL, *actInsertLTR;
	//temp
	//static int numberOfQBiDiExtenders;

signals:
	void doCurrentLineRTL();
	void doCurrentLineLTR();
	void insertedLTR();
	void bidiModeToggled(QAction *);
	void changeEditorActionState(bool enable);
	void doStoppedThings();
	void openApplicationViewer(const QString &);
};

class QBIDIEXTENDER_EXPORT QBiDiExtender : public QObject
{
    Q_OBJECT
public:
	QBiDiExtender(QTextEdit *qeditor, QBiDiInitializer *qBiDiLoader);
	~QBiDiExtender();
	QTextEdit *editorForBiDi;
	QBiDiInitializer *BiDiLoader;
	
	enum LineBiDiMode
	{
		AutoRightOrLeft = 1,
		RightToLeft = 2,
		LeftToRight = 3,
		NewLinePerLanguageChanging = 4,
		Undefined = -1
	};

	//bool mustSkipedForcely();

	bool editorLastAutoBidiState, layoutChanged, appendLRM, backSpaceFlag, backSlashFlag, firstTimeBiDi, noLRMneed,  flagBigDoc;
	int startLine, endLine, lastBiDiEnabledStatus, editorLastBiDiModeApplied, oldNumOfLines, newNumOfLines, startcheck, endcheck;
	Qt::LayoutDirection oldLayoutDir, curLayoutDir;
	void editorBiDiMenu(QMenu *menu, const QString &type);
	void applyBiDiModeToEditor(int mode);
	bool pressedKeyEvent( QKeyEvent * e );
	bool lastLangAutoChanged;
	/////////////////////////////////////
	
	//definition of static variables
	static bool  ptdOpenFlag;
	static bool bidiEnabled;
	static bool  flagAutoLRM;
	static bool  ptdSupportFlag;
	static bool forceSkiped;
	static bool  kbdLayoutChangingCanEmited;
	static LineBiDiMode bidiMode;
	static LineBiDiMode  ptdFileBiDimode;
	static QString BiDiLogoRES;
	static bool  canChangesInputLang;
	static bool  noReverseArrowKeys;
	static bool applicationViewerLoadIt;

	//static functions
	static void readBiDiSettings(QSettings *settingsObject);
	static void saveBiDiSettings(QSettings *settingsObject);
	static void stampedBiDiLogoToImage(QPixmap *pixmap);
    static bool isUnicodeControlCharacters(const QChar &ch);
	static QString removeUnicodeControlCharacters(const QString &str);
    static QStringList unicodeControlCharacters();
    static QString toPatternWithUnicodeControlCharacters(const QString &str);
    static QString toPatternWithUnicodeControlCharactersFast(const QString &str);

	#if defined( Q_WS_WIN )
	HKL lastInputLang;
	//static 
	HKL lastInputLangBeforeLTR_Text;
	#else
	//#if defined( Q_WS_X11 )
	int lastInputLang;
	//static 
	int lastInputLangBeforeLTR_Text;
	#endif
	/////////////////////////////////////
	void init();
	QAction *actAutoLRM, *actPTDSupport;

public slots:
	void bidiModeToggle(QAction *action);

private:
	int firstDollar;
	//static bool  blockBiDiState;
	static bool  forceDisableBiDirector;

private slots:
	void contextMenuInsertTag();
	void currentLineLTR();
	void currentLineRTL();
	void RTL( QTextCursor *tmpCursor );
	void LTR( QTextCursor *tmpCursor );
	
	void toggleArrowKeysReverseState(bool checked);

//MY_BIDI_ALGORITHM METHODS
public:
    void initBiDi(const QString &documentContents = QString());
	void unInitBiDi();
	int whatIsDir(const QChar &ch);
	int whatIsDir(const QChar &ch, bool extendedMode);
	void savePtdFile(const QString &f, QTextCodec* codec);

public slots:
	void addLRMinTyping();
	QString insertLRMtoString(const QString &str);
	
	void changeKbdLayout();
	
	void processInputLang(bool loadLastInputLang, bool clearLastInputLang);
	int getLastInputLang();

private:
	void TeXtrimmed(QString *text);

private slots:
	void contextMenuAddLRM();
	void contextMenuAddLRE();
	void remLRMfromSelection();
    void addLRMToLatinWords(const QString &text = QString());
	void insertLTR();
	void ptdSaveEnabled(bool checked);
	void remLRMfromDocument();

signals:
	void removeLRM();
	void doStoppedSteps();
};
#endif // QBIDIEXTENDER_H
