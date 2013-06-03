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

#ifdef TEXMAKERVERSION //only texmaker highlighter method is powerful enough for this feature
#define EDITOR_SUPPORT_MATHMODE
#endif

#include "qbidiextender2version.h"
#include "qbidiextender2.h"
#include <QPainter>
#include <QTextDocumentFragment>

#if defined( Q_WS_X11 )
#include "xkb/XKeyboard.h"
#endif

#include<QDebug>

//we need a non-static version of this variable!!
//Unfortunately in QBiDiExtender::pressedKeyEvent() we had BiDiBase Object not BiDiForEditor Object!!
//#if defined( Q_WS_WIN )
//HKL lastInputLangBeforeLTR_Text=0;
//#else
//int lastInputLangBeforeLTR_Text=0;
//#endif


const QString  docBiDiAlgorithm="BiDiAlgorithmDefault";//MODIFIED: 20 MAY 2010
const bool autoBiDiEnabled=true;
const bool editorLastAutoBidiState=true;
bool QBiDiExtender::forceDisableBiDirector=false;

bool QBiDiExtender::ptdOpenFlag = false;
#if defined(Q_WS_X11)
bool QBiDiExtender::kbdLayoutChangingCanEmited=false;
#else
bool QBiDiExtender::kbdLayoutChangingCanEmited=true;
#endif

//bool QBiDiExtender::blockBiDiState=false;
QBiDiExtender::LineBiDiMode QBiDiExtender::bidiMode = QBiDiExtender::AutoRightOrLeft;
QBiDiExtender::LineBiDiMode QBiDiExtender::ptdFileBiDimode = QBiDiExtender::Undefined;
bool QBiDiExtender::bidiEnabled = false;
//QString QBiDiExtender::BiDiLogoRES="";
bool QBiDiExtender::flagAutoLRM=true, QBiDiExtender::canChangesInputLang=true, QBiDiExtender::ptdSupportFlag=true, QBiDiExtender::noReverseArrowKeys=false;
bool QBiDiExtender::forceSkiped = false;
bool QBiDiExtender::applicationViewerLoadIt = false;

const QString PTD_VERSION = "2";

QBiDiExtender::QBiDiExtender(QTextEdit *qeditor, QBiDiInitializer *qBiDiLoader)
    : QObject(qeditor),
	editorForBiDi(qeditor), BiDiLoader(qBiDiLoader)
{
//++QBiDiInitializer::numberOfQBiDiExtenders;
//QMessageBox::information(0, "---QBiDiExtender----",tr("nums=%1\n+++++++\nQBiDiExtender is Created!!\n+++++++").arg(QBiDiInitializer::numberOfQBiDiExtenders));
init();
//forceSkiped = false;
//lastInputLangBeforeLTR_Text = 0;
lastInputLang = 0;
lastInputLangBeforeLTR_Text = 0;

connect(BiDiLoader, SIGNAL(	bidiModeToggled(QAction *)	)	, this, SLOT(	bidiModeToggle(QAction *)	));
connect(BiDiLoader, SIGNAL(	doCurrentLineRTL()	)			, this, SLOT(	currentLineRTL()			));
connect(BiDiLoader, SIGNAL(	doCurrentLineLTR()	)			, this, SLOT(	currentLineLTR()			));
connect(BiDiLoader, SIGNAL(	insertedLTR()	)				, this, SLOT(	insertLTR()					));
////////////
connect(this, SIGNAL(	doStoppedSteps()	)		, BiDiLoader, SIGNAL(	doStoppedThings()			));
}

QBiDiExtender::~QBiDiExtender()
{
//--QBiDiInitializer::numberOfQBiDiExtenders;
//QMessageBox::information(0, "---QBiDiExtender----",tr("nums=%1\n**************\nQBiDiExtender is Destroyed!!\n*****************").arg(QBiDiInitializer::numberOfQBiDiExtenders));
}

void QBiDiExtender::init()
{
editorLastBiDiModeApplied=QBiDiExtender::bidiMode;
oldLayoutDir=curLayoutDir=QApplication::keyboardInputDirection();
layoutChanged = false;
backSpaceFlag=false;
backSlashFlag=false;
appendLRM=false;
firstDollar=0;
processInputLang(false, true);
lastLangAutoChanged=false;
firstTimeBiDi=false;
if (docBiDiAlgorithm == "BiDiAlgorithmDefault")
	{initBiDi();}
}

void QBiDiExtender::contextMenuInsertTag()
	{
	QString actData;
	QAction *action = qobject_cast<QAction *>(sender());
	QTextCursor tmpCur = editorForBiDi->textCursor();
	if (action)
		{
		actData=action->data().toString();
		QChar controlCh;
		if (actData.at(0)=='R' && tmpCur.blockFormat().layoutDirection() == Qt::RightToLeft)
			controlCh=QChar(RLM);
		else
			controlCh=QChar(LRM);

		actData.remove(0,1);
		QString tmp=QString(0x2022);
		int pos=2;
		if (!tmpCur.selectedText().isEmpty())
			{
			tmp=tmpCur.selectedText();
			pos=0;
			}
		tmpCur.beginEditBlock();
		tmpCur.removeSelectedText();

		tmpCur.insertText(controlCh+actData+tmp+"}"+controlCh);
		if (pos!=0) ++pos;

		if (pos!=0)
			{
			tmpCur.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, pos);
			tmpCur.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
			}
		editorForBiDi->setTextCursor(tmpCur);
		tmpCur.endEditBlock();
		}
	}

//[start] MY_BIDI_ALGORITHM METHODS
void QBiDiExtender::contextMenuAddLRE()
{
disconnect(editorForBiDi->document(), SIGNAL(contentsChanged()), this, SLOT(addLRMinTyping()));
bool MODIFIED = editorForBiDi->document()->isModified();
QString word;
int startSel,endSel,pos;
QTextCursor curCursor, tmpCursor;
curCursor = tmpCursor = editorForBiDi->textCursor();
if ( editorForBiDi->textCursor().hasSelection() )
	{
	pos = editorForBiDi->textCursor().position();
	endSel = editorForBiDi->textCursor().anchor();
	startSel = editorForBiDi->textCursor().blockNumber();
	curCursor.setPosition( endSel, QTextCursor::MoveAnchor );
	editorForBiDi->setTextCursor(curCursor);
	endSel = editorForBiDi->textCursor().blockNumber();
	if (endSel != startSel) 
		{
		tmpCursor.clearSelection();
		editorForBiDi->setTextCursor(tmpCursor);
		editorForBiDi->document()->setModified( MODIFIED );
		if (QBiDiExtender::flagAutoLRM && QBiDiExtender::bidiEnabled)	connect(editorForBiDi->document(), SIGNAL(contentsChanged()), this, SLOT(addLRMinTyping()));
		return;
		}
	curCursor.setPosition( pos, QTextCursor::KeepAnchor );
	editorForBiDi->setTextCursor(curCursor);
	word = editorForBiDi->textCursor().selectedText();
	}
else 
	{
	QTextCursor tc=editorForBiDi->textCursor();
	word = QString(QChar(RLM))+"{"+QString(QChar(LRE))/*+"{}"*/+QString(QChar(LRM))+QString(QChar(PDF))+"}"+QString(QChar(RLM));
	tc.insertText(word);
	tc.movePosition(QTextCursor::PreviousCharacter,QTextCursor::MoveAnchor,4);
	
	//////////////////////////////////////////////////////////////////////////
	////////////////// Win32 API
	processInputLang(false, true);
	changeKbdLayout();
	if (lastInputLangBeforeLTR_Text == 0)
		lastInputLangBeforeLTR_Text = lastInputLang;
	////////////////////////////////////////////////////////////////////////////
	
	editorForBiDi->setTextCursor(tc);
	editorForBiDi->document()->setModified( MODIFIED );
	if (QBiDiExtender::flagAutoLRM && QBiDiExtender::bidiEnabled)	connect(editorForBiDi->document(), SIGNAL(contentsChanged()), this, SLOT(addLRMinTyping()));
	return;
	}
//Remove Unicode Control Characters
word.remove(QChar(LRM)).remove(QChar(RLM)).remove(QChar(LRE)).remove(QChar(RLE)).remove(QChar(PDF));

word = QString(QChar(RLM))+"{"+QString(QChar(LRE))+word+QString(QChar(LRM))+QString(QChar(PDF))+"}"+QString(QChar(RLM)); //.append(QChar(LRM)).append(QChar(PDF))).prepend(QChar(LRE));
editorForBiDi->textCursor().beginEditBlock();
editorForBiDi->textCursor().removeSelectedText();
editorForBiDi->textCursor().insertText(word);
editorForBiDi->textCursor().endEditBlock();
editorForBiDi->setTextCursor(tmpCursor);
editorForBiDi->document()->setModified( MODIFIED );
if (QBiDiExtender::flagAutoLRM && QBiDiExtender::bidiEnabled)	connect(editorForBiDi->document(), SIGNAL(contentsChanged()), this, SLOT(addLRMinTyping()));
}

void QBiDiExtender::contextMenuAddLRM()
{
disconnect(editorForBiDi->document(), SIGNAL(contentsChanged()), this, SLOT(addLRMinTyping()));
bool MODIFIED = editorForBiDi->document()->isModified();//new
QString word;
int startSel,endSel,pos;
QTextCursor curCursor, tmpCursor;
curCursor = tmpCursor = editorForBiDi->textCursor();
if ( editorForBiDi->textCursor().hasSelection() )
{
	pos = editorForBiDi->textCursor().position();
	endSel = editorForBiDi->textCursor().anchor();
	startSel = editorForBiDi->textCursor().blockNumber();
	curCursor.setPosition( endSel, QTextCursor::MoveAnchor );
	editorForBiDi->setTextCursor(curCursor);
	endSel = editorForBiDi->textCursor().blockNumber();
	if (endSel != startSel) 
	{
		tmpCursor.clearSelection();
		editorForBiDi->setTextCursor(tmpCursor);
		editorForBiDi->document()->setModified( MODIFIED );
		if (QBiDiExtender::flagAutoLRM && QBiDiExtender::bidiEnabled)	connect(editorForBiDi->document(), SIGNAL(contentsChanged()), this, SLOT(addLRMinTyping()));
		return;
	}
	curCursor.setPosition( pos, QTextCursor::KeepAnchor );
	editorForBiDi->setTextCursor(curCursor);
	word = editorForBiDi->textCursor().selectedText();
}
else 
	{
	word = QChar(LRM);
	editorForBiDi->textCursor().insertText(word);
	editorForBiDi->document()->setModified( MODIFIED );
	if (QBiDiExtender::flagAutoLRM && QBiDiExtender::bidiEnabled)	connect(editorForBiDi->document(), SIGNAL(contentsChanged()), this, SLOT(addLRMinTyping()));
	return;
	}
//Remove Unicode Control Characters
word.remove(QChar(LRM)).remove(QChar(RLM));
word = (word.append(QChar(LRM))).prepend(QChar(LRM));
editorForBiDi->textCursor().beginEditBlock();
editorForBiDi->textCursor().removeSelectedText();
editorForBiDi->textCursor().insertText(word);
editorForBiDi->textCursor().endEditBlock();
editorForBiDi->setTextCursor(tmpCursor);
editorForBiDi->document()->setModified( MODIFIED );
if (QBiDiExtender::flagAutoLRM && QBiDiExtender::bidiEnabled)	connect(editorForBiDi->document(), SIGNAL(contentsChanged()), this, SLOT(addLRMinTyping()));
}

int QBiDiExtender::whatIsDir(const QChar &ch, bool extendedMode)
{
if (!extendedMode) return whatIsDir(ch);
if (ch=='\"'  || ch=='~' || ch==QChar(12,6))//cell=12,row=6 is for persian and arabic comma
	return Qt::RightToLeft;
if ( ch.direction() == QChar::DirON || ch.direction() == QChar::DirET || ch.direction() == QChar::DirL || ch.direction() == QChar::DirEN ) //new
	return Qt::LeftToRight;
else 
	return Qt::RightToLeft;
}

int QBiDiExtender::whatIsDir(const QChar &ch)
{
//if (ch=='\"'  || ch=='~' || ch==QChar(12,6))//cell=12,row=6 is for persian and arabic comma
//	return Qt::RightToLeft;
//if ( ch.direction() == QChar::DirON || ch.direction() == QChar::DirET || ch.direction() == QChar::DirL || ch.direction() == QChar::DirEN ) //new
//	return Qt::LeftToRight;
//else 
//	return Qt::RightToLeft;
QChar::Direction chDir = ch.direction();
if ( chDir == QChar::DirAL || chDir == QChar::DirR )
	return Qt::RightToLeft;
if ( chDir == QChar::DirL || /*temp*/ ch =='\\'/* || ch.isPunct()*/)
	return Qt::LeftToRight;

return -1;
}

void QBiDiExtender::addLRMinTyping()
{
if (forceDisableBiDirector || !editorForBiDi || !QBiDiExtender::flagAutoLRM) return;

disconnect(editorForBiDi->document(), SIGNAL(contentsChanged()), this, SLOT(addLRMinTyping()));
if (!QBiDiExtender::bidiEnabled) return;
bool tmpFlag=true;//, tmpAppendLRM=false;
QTextCharFormat LTRFormat;

if (kbdLayoutChangingCanEmited)
    {
    if ( layoutChanged )
	    {
	    curLayoutDir = QApplication::keyboardInputDirection();
	    if (oldLayoutDir != curLayoutDir)
		    {
		    oldLayoutDir = curLayoutDir;
		    QTextCursor tmpCur = editorForBiDi->textCursor();
		    //QTextBlock currentBlock = tmpCur.block();
		    if (QBiDiExtender::bidiMode==4)
			    {
			    QTextBlockFormat tmpBlockFormat = tmpCur.blockFormat();
			    tmpBlockFormat.setLayoutDirection( Qt::RightToLeft );
			    tmpCur.joinPreviousEditBlock();
			    tmpCur.setBlockFormat( tmpBlockFormat );
			    LTRFormat = tmpCur.charFormat();
			    tmpCur.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor);
			    if (curLayoutDir == Qt::RightToLeft)
				    tmpCur.insertText(QString(QChar(LRM))+"%\n");
			    else
				    tmpCur.insertText("%\n"+QString(QChar(LRM)));
			    tmpCur.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor);
			    tmpCur.insertText(QChar(LRM));
			    tmpCur.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor);
			    tmpCur.endEditBlock();
			    editorForBiDi->setTextCursor(tmpCur);
			    connect(editorForBiDi->document(), SIGNAL(contentsChanged()), this, SLOT(addLRMinTyping()));
			    return;
			    } //QBiDiExtender::bidiMode==3
    //////////////////////////////////////////////////////
		    tmpCur.joinPreviousEditBlock();
		    int tmpPos = tmpCur.position();//new
		    tmpCur.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);

		    QString curLine = tmpCur.selectedText();
		    QStringList lineWords = curLine.split("",QString::SkipEmptyParts);
		    /*bool appendLRM=false;*/
		    int lineSize=lineWords.size()-1;
		    QString ch;
		    if ( lineSize>=0 ) ch = lineWords.at(lineSize);
		    else ch = "";

		    if (!ch.isEmpty())
			    {
			    if ( curLayoutDir == Qt::LeftToRight )
				    {
				    if ( !ch.at(0).isSpace() )
					    {
					    //new
					    int tmpIndex=lineSize;
					    //QString tempp=lineWords.at(tmpIndex);
					    while (tmpIndex-1>=0)
						    {
							int tmpInd = tmpIndex;
							while(tmpInd-1>=0)
								{
								if (lineWords.at(tmpInd-1).isEmpty() || lineWords.at(tmpInd-1).at(0).isSpace())
									{
									--tmpInd;
									continue;
									}
								if ( lineWords.at(tmpInd-1).at(0) == QChar(LRM) )
									{
									lineWords[tmpInd-1] = "";
									--tmpInd;
									continue;
									}
								else break;
								}
////////////////////////////////////////////////////////////////////////

							if ( lineWords.at(tmpIndex-1).isEmpty() || (whatIsDir( lineWords.at(tmpIndex-1).at(0) ) == Qt::LeftToRight) )
							    {
							    --tmpIndex;
							    }
						    else break;
						    }
					    lineWords[tmpIndex].prepend(QChar(/*'i'*/LRM));
					    lineWords[lineSize].append(QChar(/*'i'*/LRM));
						appendLRM=true;
					    }
				    else
					    {
					    oldLayoutDir=Qt::RightToLeft;
					    //lineWords[lineSize].append(QChar(LRM));//There is a better method
					    }
					}
				else
					{	
					lineWords[lineSize].prepend(QChar(RLM));
					if (appendLRM)
						{
						lineWords[lineSize].prepend(QChar(LRM));
						appendLRM=false;
						tmpCur.insertText(lineWords.join(""));
						tmpCur.clearSelection();
						tmpCur.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
						if (tmpCur.hasSelection())
							{
							if (tmpCur.selectedText().at(0)==QChar(LRM))
								tmpCur.removeSelectedText();
							else
								tmpCur.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor);
							}
						//else
							//tmpCur.clearSelection();
							//tmpCur.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor);
						tmpFlag=false;
						}
					}
			    //else appendLRM=true;//new

			    //else //if I use better method above else part isn't needed
			    //	{
			    //	for (int je = lineSize-1; je>=0;--je)
			    //		{
			    //		if ( lineWords.at(je)!=" " && lineWords.at(je)!="\t" )
			    //			{
			    //			lineWords[je].append(QChar(LRM));
			    //			break;
			    //			}
			    //		}
			    //	}
				if (tmpFlag)
					{
					tmpCur.insertText(lineWords.join(""));
					tmpCur.clearSelection();
					}
			    if (appendLRM) 
					{
					tmpCur.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor);
					}
			    }
		    tmpCur.endEditBlock();
			editorForBiDi->setTextCursor(tmpCur);
			//tmpCur.endEditBlock();
		    }
	    }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Fixed: <LRM><some spaces><inserting LTR text without changing language>: the <LRM> have to be placed after spaces
//this fix works on Win32 and MACX not Linux(es)
		if (curLayoutDir == Qt::LeftToRight)
			{
			QTextCursor tmpCur = editorForBiDi->textCursor();
			tmpCur.joinPreviousEditBlock();
		    int tmpPos = tmpCur.position();
		    tmpCur.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
		    QString curLine = tmpCur.selectedText();
		    QStringList lineWords = curLine.split("",QString::SkipEmptyParts);
		    /*bool appendLRM=false;*/
		    int lineSize=lineWords.size()-1;
		    QString ch;
		    if ( lineSize>=0 ) ch = lineWords.at(lineSize);
		    else ch = "";
			if (!ch.isEmpty())
				{
				////////////////////////////////////////////////////////////////////////
				int tmpInd = lineSize;
				bool spaceFlag = false, insertLRM=false;;
				while(tmpInd-1>=0)
					{
					if (lineWords.at(tmpInd-1).isEmpty() || lineWords.at(tmpInd-1).at(0).isSpace())
						{
						--tmpInd;
						if (!lineWords.at(tmpInd).isEmpty())
							spaceFlag=true;
						continue;
						}
					if ( lineWords.at(tmpInd-1).at(0) == QChar(LRM) && spaceFlag)
						{
						lineWords[tmpInd-1] = "";
						insertLRM = true;
						--tmpInd;
						continue;
						}
					else break;
					}
				if (insertLRM)
					{
					lineWords[lineSize].prepend(QChar(LRM)).append(QChar(LRM));
					}
				curLine = lineWords.join("");
				tmpCur.removeSelectedText();
				tmpCur.insertText(curLine);
				if (insertLRM)
					{
					tmpCur.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor);
					insertLRM=false;
					}
				////////////////////////////////////////////////////////////////////////
				}
			tmpCur.endEditBlock();
			editorForBiDi->setTextCursor(tmpCur);
			//tmpCur.endEditBlock();
			}
//////////////////////////////////////////////////////////////
    }
else //else <- if (kbdLayoutChangingCanEmited)
    {
	if (backSpaceFlag)
		{
		connect(editorForBiDi->document(), SIGNAL(contentsChanged()), this, SLOT(addLRMinTyping()));
		return;
		}
    QTextCursor tmpCur = editorForBiDi->textCursor();
    //QTextBlock currentBlock = tmpCur.block();
    if (QBiDiExtender::bidiMode==4)
	    {
	    connect(editorForBiDi->document(), SIGNAL(contentsChanged()), this, SLOT(addLRMinTyping()));
	    return;
	    }
    //////////////////////////////////////////////////////
    tmpCur.joinPreviousEditBlock();
    //int tmpPos = tmpCur.position();
	//new//if (tmpCur.hasSelection())
    tmpCur.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);

    QString curLine = tmpCur.selectedText();
    QStringList lineWords = curLine.split("",QString::SkipEmptyParts);
    int lineSize=lineWords.size()-1;
    QString ch;
    if ( lineSize>=0 ) ch = lineWords.at(lineSize);
    else ch = "";

    if (!ch.isEmpty())
		{
		bool chIsMirrored=ch.at(0).hasMirrored();
		if (appendLRM)
			chIsMirrored=false;
		bool test=whatIsDir(QChar(LRM)) == Qt::RightToLeft;
	    if ( (whatIsDir( ch.at(0) ) == Qt::LeftToRight) && !chIsMirrored )
			{
			int tmpIndex=lineSize;
			while (tmpIndex-1>=0)
				{
				int tmpInd = tmpIndex;
				while(tmpInd-1>=0)
					{
					if (lineWords.at(tmpInd-1).isEmpty() || lineWords.at(tmpInd-1).at(0).isSpace())
						{
						--tmpInd;
						continue;
						}
					if ( lineWords.at(tmpInd-1).at(0) == QChar(LRM) )
						{
						lineWords[tmpInd-1] = "";
						--tmpInd;
						continue;
						}
					else break;
					}
				////////////////////////////////////////////////////////////////////////

				if ( lineWords.at(tmpIndex-1).isEmpty() || (whatIsDir( lineWords.at(tmpIndex-1).at(0) ) == Qt::LeftToRight) )
					{
					--tmpIndex;
					}
				else break;
				}
			for (int i=tmpIndex;i<lineSize;++i)
			    {
			    lineWords[i].remove(QChar(LRM));
			    }
			lineWords[tmpIndex].prepend(QChar(LRM));
			//if (!appendLRM)
				//{
				lineWords[lineSize].append(QChar(LRM));
				//appendLRM=true;
				//chIsMirrored=false;
				//}
			//appendLRM=true;
			}
	    //else
			//{
			//if (appendLRM)
			//	{
			//	lineWords[lineSize].prepend(QChar(LRM));
			//	appendLRM=false;
			//	tmpCur.insertText(lineWords.join(""));
			//	tmpCur.clearSelection();
			//	tmpCur.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
			//	if (tmpCur.hasSelection())
			//		{
			//		if (tmpCur.selectedText().at(0)==QChar(LRM))
			//			tmpCur.removeSelectedText();
			//		tmpCur.clearSelection();//new//temp
			//		}
			//	tmpFlag=false;
			//	}
			//}
	    //else //if I use better method above else part isn't needed
	    //	{
	    //	for (int je = lineSize-1; je>=0;--je)
	    //		{
	    //		if ( lineWords.at(je)!=" " && lineWords.at(je)!="\t" )
	    //			{
	    //			lineWords[je].append(QChar(LRM));
	    //			break;
	    //			}
	    //		}
	    //	}
	    if (tmpFlag)
			{
			tmpCur.insertText(lineWords.join(""));
			tmpCur.clearSelection();
			editorForBiDi->setTextCursor(tmpCur);//new//temp
			}
	    if ( (whatIsDir( ch.at(0) ) == Qt::LeftToRight) && !chIsMirrored && !appendLRM)
			{
			int tempPos=tmpCur.position();
			if (tempPos>=1)
				tmpCur.setPosition(tempPos-1, QTextCursor::MoveAnchor);
			appendLRM=true;
			}
		}
    tmpCur.endEditBlock();
	editorForBiDi->setTextCursor(tmpCur);
	}
connect(editorForBiDi->document(), SIGNAL(contentsChanged()), this, SLOT(addLRMinTyping()));
return;
}

void QBiDiExtender::remLRMfromSelection()
{
emit removeLRM();
}

void QBiDiExtender::TeXtrimmed(QString *text)
{
if (!text) return;
text->simplified();
text->remove(QChar(' '), Qt::CaseInsensitive);
text->remove(QChar('%'), Qt::CaseInsensitive);
text->remove(QChar(LRM), Qt::CaseInsensitive);

if (!text->isEmpty())
	{
	while ( text->at(0).direction() != QChar::DirAL && text->at(0).direction() != QChar::DirR && text->at(0).direction() != QChar::DirL )
		{
			QChar firstChar = text->at(0);
			text->remove(firstChar, Qt::CaseInsensitive);
			if (text->isEmpty()) break;
		}
	}
}

QString QBiDiExtender::insertLRMtoString(const QString &str)
{
//if (!text) return;
QString text = str;
QString word, tmpWord;
int prevWordFlag=-1,lastLatinIndex=-1;
text.remove(QChar(LRM), Qt::CaseInsensitive);
text.replace( "\n", " \n ", Qt::CaseInsensitive );
QStringList splittedDoc = text.split(" ", QString::SkipEmptyParts);

for (int i = 0; i < splittedDoc.size(); ++i)
	{
	int startIndex=0;
	word = splittedDoc.at(i);//new
	if (word=="\n")
		{
		if ( lastLatinIndex!=-1 )
		{
		splittedDoc[lastLatinIndex] = splittedDoc[lastLatinIndex].append(QChar(LRM));
		lastLatinIndex=-1;
		}
		prevWordFlag = -1;
		continue;
		}
	tmpWord = word;
	tmpWord.remove("%",Qt::CaseInsensitive);
	tmpWord.remove("(",Qt::CaseInsensitive);
	tmpWord.remove("\\{",Qt::CaseInsensitive);
	tmpWord.remove("{",Qt::CaseInsensitive);
	tmpWord.remove("[",Qt::CaseInsensitive);
	tmpWord.remove(")",Qt::CaseInsensitive);
	tmpWord.remove("\\}",Qt::CaseInsensitive);
	tmpWord.remove("}",Qt::CaseInsensitive);
	tmpWord.remove("]",Qt::CaseInsensitive);
	//new//
	if ( tmpWord.isEmpty() ) continue;

	if ( word.at(0).isSpace() && word.size()>1) startIndex = 1;
	if ( (whatIsDir(word.at(startIndex), true) == Qt::LeftToRight) && (whatIsDir(word.at(word.size()-1), true)  == Qt::LeftToRight) )//new
		{
		if ( prevWordFlag==-1 )
		splittedDoc[i] = word.prepend(QChar(LRM));
		lastLatinIndex = i;
		prevWordFlag = 1;
		}
	else
		{
		if ( (whatIsDir(word.at(startIndex), true) == Qt::RightToLeft) && (whatIsDir(word.at(word.size()-1), true) == Qt::RightToLeft) )//new
		{
		if ( lastLatinIndex!=-1 )
			{
			splittedDoc[lastLatinIndex] = splittedDoc[lastLatinIndex].append(QChar(LRM));
			lastLatinIndex=-1;
			}
		prevWordFlag = -1;
		////////////////////////////////////////////////
		tmpWord = word;
		while( tmpWord.at(0).direction()!=QChar::DirL )
			{
			QChar firstChar = tmpWord.at(0);
			tmpWord = tmpWord.remove(firstChar, Qt::CaseInsensitive);
			if (tmpWord.isEmpty()) break;
			}
		if (!tmpWord.isEmpty())
			{
			int je = word.size()-1;
			while ( word.at(je).direction()==QChar::DirR || word.at(je).direction()==QChar::DirAL )//whatIsDir(word.at(je), true) )
			{
			--je;
			}
			int js = startIndex;
			while ( word.at(js).direction()==QChar::DirR || word.at(js).direction()==QChar::DirAL )//(whatIsDir(word.at(js), true) )
			{
			++js;
			}
			word.insert(je+1,QChar(LRM));
			word.insert(js,QChar(LRM));
			splittedDoc[i] = word;
			}
	///////////////////////////////////////////
		}
		else
			{
			if ( (whatIsDir(word.at(startIndex), true) == Qt::RightToLeft) && (whatIsDir(word.at(word.size()-1), true) == Qt::LeftToRight) )//new
				{
				if ( lastLatinIndex!=-1 )
					{
					splittedDoc[lastLatinIndex] = splittedDoc[lastLatinIndex].append(QChar(LRM));
					}
				int j = word.size()-1;
				if ( whatIsDir(word.at(j), true) == Qt::LeftToRight )
					{
					--j;
					while( word.at(j).direction()!=QChar::DirR && word.at(j).direction()!=QChar::DirAL && word.at(j).direction()!=QChar::DirRLE && word.at(j).direction()!=QChar::DirRLO  && j>0 )
						--j;
					}
				if (word.at(j+1)!='}' && word.at(j+1)!=']' && word.at(j+1)!=')' )
					{
					word.insert(j+1,QChar(LRM));
					splittedDoc[i] = word;
					lastLatinIndex=i;
					prevWordFlag = 1;
					}
				}
			else
				{
				if ( (whatIsDir(word.at(startIndex), true) == Qt::LeftToRight) && (whatIsDir(word.at(word.size()-1), true) == Qt::RightToLeft) )//new
					{
					if ( word.at(startIndex)=='%' ) continue;
					int j = startIndex;
					if ( whatIsDir(word.at(j), true) == Qt::LeftToRight )
						{
						++j;
						while( word.at(j).direction()!=QChar::DirR && word.at(j).direction()!=QChar::DirAL && word.at(j).direction()!=QChar::DirRLE && word.at(j).direction()!=QChar::DirRLO  && j<word.size()-1)
						++j;
						}
					if (word.at(j-1)!='{' && word.at(j-1)!='[' && word.at(j-1)!='(' )
						{
						word.insert(j,QChar(LRM));
						if ( prevWordFlag==-1 )
							word.prepend(QChar(LRM));
						}
					splittedDoc[i] = word;
					lastLatinIndex = -1;
					prevWordFlag=-1;
					}
				}
			}
		}
	}
text = splittedDoc.join(" ");
text.replace( " \n ", "\n", Qt::CaseInsensitive );

//temp, line started by LRM are layouted from left to right!

QRegExp lrmExpCommand("\n"+QString(LRM)+"*\\\\"+QString(LRM)+"+",  Qt::CaseInsensitive);
text.replace( lrmExpCommand, "\n\\");


QStringList lineList = text.split("\n", QString::SkipEmptyParts);
bool changed = false;
for (int lineIndex = 0; lineIndex < lineList.size(); ++lineIndex) {
    QString line = lineList.at(lineIndex);
    QString tmp = CLEAR_CONTROL_CHARS(line);
    tmp = tmp.remove("\\");
    TeXtrimmed(&tmp);
    if (tmp.isEmpty()) {
        continue;
    }
    if (tmp.isRightToLeft() && !line.isRightToLeft() || (!tmp.isRightToLeft() && line.isRightToLeft())) {
        int index = 0;
        while (index < line.size() &&
               (line.at(index) == QChar(LRM) || line.at(index) == QChar(RLM))) {
                ++index;
                continue;
        }
        if (index < line.size()) {
            if (index>1) {
                line = line.remove(0, index - 1);
                //QMessageBox::information(0,"blockText.remove",QString("index=%1").arg(index));
            }

            if (tmp.isRightToLeft()) {
                if (line.at(0) != QChar(RLM)) {
                    line.prepend(QChar(RLM));
                }
            }
            else {
                if (line.at(0) != QChar(LRM)) {
                    line.prepend(QChar(LRM));
                }
            }
            lineList[lineIndex] = line;
            changed = true;
        }
    }
}
if (changed) {
    text = lineList.join("\n");
}
//this function removes all LRMs, we don't need the following lines!
//QRegExp lrmExp("\n"+QString(LRM)+"+",  Qt::CaseInsensitive);
//text.replace(lrmExp, "\n");

///text->replace( "\n"+QString(LRM)+"\\"+QString(LRM), "\n\\", Qt::CaseInsensitive );
////temp, line started by RLM are layouted from right to  left!
////text->replace( QChar(RLM)+"\n", "\n", Qt::CaseInsensitive );

return text;
}

void QBiDiExtender::addLRMToLatinWords()
{
if (!editorForBiDi || !QBiDiExtender::bidiEnabled) return;
bool  MODIFIED = editorForBiDi->document()->isModified();
disconnect(editorForBiDi->document(), SIGNAL(contentsChanged()), this, SLOT(addLRMinTyping()));
bool tmpFlag=false,selFlag=false;
QString wholeDoc;
QTextCursor curCursor = editorForBiDi->textCursor();
QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
selFlag=curCursor.hasSelection();

if (selFlag)
	{
	wholeDoc = curCursor.selectedText();
	}
else
	{
	wholeDoc = editorForBiDi->toPlainText();
	}
///////////////////////////////////////////////////////////////
if (!QBiDiExtender::ptdOpenFlag)
	{
	wholeDoc = insertLRMtoString(wholeDoc);
	}
///////////////////////////////////////////////////////////////
if (!selFlag)
    {
	if (!QBiDiExtender::ptdOpenFlag)
		{
		//editorForBiDi->textCursor().beginEditBlock();
		//editorForBiDi->clear();
		editorForBiDi->setPlainText( wholeDoc );
		//editorForBiDi->textCursor().endEditBlock();
		}
		QApplication::restoreOverrideCursor();
	}
else
    {
    editorForBiDi->textCursor().beginEditBlock();
    QTextCursor tmpCursor = curCursor;
    int endSel = tmpCursor.anchor();
    int startSel = tmpCursor.blockNumber();
    editorForBiDi->textCursor().removeSelectedText();
    editorForBiDi->textCursor().insertText(wholeDoc);
    tmpCursor.setPosition( endSel, QTextCursor::MoveAnchor );
    editorForBiDi->setTextCursor(tmpCursor);
    endSel = tmpCursor.blockNumber();
    if (startSel<endSel)
		{
		startcheck = startSel;
		endcheck = endSel;
		}
    else
		{
		endcheck = startSel;
		startcheck = endSel;
		}
    editorForBiDi->textCursor().endEditBlock();

	QApplication::restoreOverrideCursor();
	}
QApplication::restoreOverrideCursor();
editorForBiDi->document()->setModified( MODIFIED );
QTextCursor tc(editorForBiDi->document());
editorForBiDi->setTextCursor(tc);
if (QBiDiExtender::flagAutoLRM && QBiDiExtender::bidiEnabled)
	connect(editorForBiDi->document(), SIGNAL(contentsChanged()), this, SLOT(addLRMinTyping()));
else 
	disconnect(editorForBiDi->document(), SIGNAL(contentsChanged()), this, SLOT(addLRMinTyping()));
}

void QBiDiExtender::insertLTR()
{
if (editorForBiDi) 
	{
	if (!editorForBiDi->textCursor().hasSelection())
		contextMenuAddLRE();
	}
}

void QBiDiExtender::initBiDi()
{
if (!editorForBiDi) return;
connect(this, SIGNAL(removeLRM()), this, SLOT(remLRMfromDocument()));
if (QBiDiExtender::bidiEnabled && QBiDiExtender::flagAutoLRM)
	connect(editorForBiDi->document(), SIGNAL(contentsChanged()), this, SLOT(addLRMinTyping()));
else
	disconnect(editorForBiDi->document(), SIGNAL(contentsChanged()), this, SLOT(addLRMinTyping()));

flagBigDoc=false;

if (autoBiDiEnabled)
	{
	addLRMToLatinWords();
	}
}

void QBiDiExtender::unInitBiDi()
{
if (!editorForBiDi) return;
disconnect(this, SIGNAL(removeLRM()), this, SLOT(remLRMfromDocument()));
disconnect(editorForBiDi->document(), SIGNAL(contentsChanged()), this, SLOT(addLRMinTyping()));
firstTimeBiDi=false;
}

void QBiDiExtender::ptdSaveEnabled(bool checked)
{
QBiDiExtender::ptdSupportFlag=checked;
}

void QBiDiExtender::remLRMfromDocument()
{
if (!editorForBiDi) return;
bool  MODIFIED = editorForBiDi->document()->isModified();
disconnect(this, SIGNAL(removeLRM()), this, SLOT(remLRMfromDocument()));

QTextCursor curCursor = editorForBiDi->textCursor();
QTextCursor tmpCursor = curCursor;
QString tmp;
int startSel,endSel;
if (tmpCursor.hasSelection())
	{
	startcheck = tmpCursor.position();
	endcheck = tmpCursor.anchor();
	tmp = tmpCursor.selectedText();
	startSel = tmpCursor.blockNumber();
	tmpCursor.setPosition( endcheck, QTextCursor::MoveAnchor );
	endSel = tmpCursor.blockNumber();
	//Remove Unicode Control Characters
	tmp.remove(QChar(LRM)).remove(QChar(RLM)).remove(QChar(LRE)).remove(QChar(RLE)).remove(QChar(PDF));
	curCursor.beginEditBlock();
	curCursor.removeSelectedText();
	curCursor.insertText(tmp);
	curCursor.endEditBlock();
	curCursor.clearSelection();
	editorForBiDi->setTextCursor(curCursor);
	if (startSel>endSel)
		{
		startcheck = endSel;
		endcheck = startSel;
		}
	else
		{
		startcheck = startSel;
		endcheck = endSel;
		}
	editorForBiDi->document()->setModified( MODIFIED );//new
	connect(this, SIGNAL(removeLRM()), this, SLOT(remLRMfromDocument()));
	return;
	}
}

void QBiDiExtender::savePtdFile(const QString &f, QTextCodec* codec)
{
if (!editorForBiDi) return;
QString fileType = QFileInfo(f).suffix().toLower() == "tex" ? "" : ("_"+QFileInfo(f).suffix().toLower());
QDateTime fileLastModification=QFileInfo(f).lastModified();
QString ptdFileName=f.left(f.size()-4)+fileType+".ptd";
ptdFileName.replace(QChar('\\'),QChar('/'));
QString tmpName = ptdFileName.right( ptdFileName.size()-ptdFileName.lastIndexOf(QChar('/'))-1 );
ptdFileName = ptdFileName.left( ptdFileName.lastIndexOf(QChar('/'))+1 )+"."+tmpName;
QFile ptdFile( ptdFileName );
if ( !ptdFile.open( QIODevice::WriteOnly ) )
    {
	QMessageBox::warning( editorForBiDi,tr("Error"),tr("The PTD file could not be saved. Please check if you have write permission."));
    return;
    }
QTextStream ptdTS( &ptdFile );
ptdTS.setCodec(codec ? codec : QTextCodec::codecForLocale());
QTextDocument *tmpDoc = editorForBiDi->document()->clone();
//QTextBlock b(tmpDoc->firstBlock());
//while (b.isValid()) {
QTextCursor tc(tmpDoc);
while (!tc.atEnd()) {
    QTextBlock b(tc.block());
    if (!b.isValid()) {
        if (tc.movePosition(QTextCursor::NextBlock))
            continue;
        else
            break;
    }
    //QTextCursor tc(b);
    // QMessageBox::information(0,"pos",QString::number(tc.position()));

//    QTextBlock b = tc.block();
    Qt::LayoutDirection blockDirection = b.blockFormat().layoutDirection();
    if (blockDirection != Qt::LayoutDirectionAuto) {
        if (blockDirection == Qt::LeftToRight && b.text().isRightToLeft() ||
                blockDirection == Qt::RightToLeft && !b.text().isRightToLeft()) {
            // apply
            QString blockText = b.text();
            if (!removeUnicodeControlCharacters(blockText).isEmpty()) {
                //int anchor = tc.position();
                int index = 0;
                while (index < blockText.size() &&
                       (blockText.at(index) == QChar(LRM) || blockText.at(index) == QChar(RLM))) {
                        ++index;
                        continue;
                }
                if (index < blockText.size()) {
                    if (index>1) {
                        blockText = blockText.remove(0, index - 1);
                        //QMessageBox::information(0,"blockText.remove",QString("index=%1").arg(index));
                    }
                    if (blockDirection == Qt::LeftToRight) {
                        if (blockText.at(0) != QChar(LRM)) {
                            blockText = blockText.prepend(QChar(LRM));
//                            QMessageBox::information(0,"LRM",QString("text=%1\nb.blockFormat().layoutDirection=%2\n"
//                                                                     "b.text().layoutDirection=%3\nb.textDirection=%4")
//                                                     .arg(b.text()).arg(b.blockFormat().layoutDirection()).arg((b.text().isRightToLeft() ? "RightToLeft" : "LeftToRight")).arg(b.textDirection()));
                        }
                    }
                    else {
                        if (blockText.at(0) != QChar(RLM)) {
                           blockText = blockText.prepend(QChar(RLM));
//                           QMessageBox::information(0,"RLM",QString("text=%1\nb.blockFormat().layoutDirection=%2\n"
//                                                                    "b.text().layoutDirection=%3\nb.textDirection=%4")
//                                                    .arg(b.text()).arg(b.blockFormat().layoutDirection()).arg((b.text().isRightToLeft() ? "RightToLeft" : "LeftToRight")).arg(b.textDirection()));
                        }
                    }
                    QTextBlockFormat bF = b.blockFormat();
                    bF.setLayoutDirection(Qt::LayoutDirectionAuto);
                    tc.setBlockFormat(bF);
                    tc.movePosition(QTextCursor::StartOfBlock);
                    tc.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
                    //QMessageBox::information(0,"selectedText",tc.selectedText());
                    tc.removeSelectedText();
                    tc.insertText(blockText);
                }
            }
        }
    }
    //b = b.next();
    if (!tc.movePosition(QTextCursor::NextBlock)) {
        break;
    }
    //tc.setPosition(b.position());
}
//while (b.isValid()) {
//    b
//    b = b.next();
//}
QString tmpHtml=tmpDoc->toHtml("utf-8");
delete tmpDoc;

int index1=tmpHtml.indexOf("<body")+5;
int index2=tmpHtml.indexOf("<p")-2;
tmpHtml.remove(index1, index2-index1);
ptdTS << "<!"+QString::number(QBiDiExtender::bidiMode)+","+fileLastModification.toString("yyyyMMddhh:mm:ss")+",V."+PTD_VERSION+">\n" << tmpHtml;
ptdFile.close();

#if defined(Q_WS_WIN)
//It sets true hidden flag of file true
ptdFileName.replace(QChar('/'), QChar('\\'));
ptdFileName="\\\\?\\"+ptdFileName;
const ushort *winFileName=ptdFileName.utf16();
DWORD dwAttrs= GetFileAttributes(winFileName);
if (dwAttrs==INVALID_FILE_ATTRIBUTES) return;
 if (!(dwAttrs & FILE_ATTRIBUTE_HIDDEN))
    {
    SetFileAttributes(winFileName, dwAttrs | FILE_ATTRIBUTE_HIDDEN);
    }
#endif
}

void QBiDiExtender::changeKbdLayout()
{
lastLangAutoChanged = true;
bool KbLchangeFlag=false, flagFirstLTRKbL=false;
int firstLTRKbL=-1,defaultIndex=-1, KbLcounts=0;
#if defined( Q_WS_WIN )
HKL HKLlplist[10];
KbLcounts=GetKeyboardLayoutList(0, HKLlplist);//this doesn't work on Win7 64Bit
if (KbLcounts <= 0)
	KbLcounts=GetKeyboardLayoutList(10, HKLlplist);//this seems be slow on some systems
else
	GetKeyboardLayoutList(KbLcounts, HKLlplist);

for (int i=0; i<KbLcounts; ++i)
	{
	int id = (int)HKLlplist[i] & 0x000000FF;
	//qDebug() << "HKL-"<<i<<"=" << (int)HKLlplist[i] << "id=" << id<<HKLlplist[i];
	if (!flagFirstLTRKbL && id != LANG_PERSIAN && id != LANG_ARABIC && id != LANG_HEBREW && id!=LANG_URDU)
		{
		firstLTRKbL=i;
		flagFirstLTRKbL=true;
		}
	if (lastInputLang!=0)
		{
		KbLchangeFlag=true;
		break;
		}
	if (id==LANG_ENGLISH)
		{
		lastInputLang=ActivateKeyboardLayout(HKLlplist[i], KLF_SETFORPROCESS);
		KbLchangeFlag=true;
		break;
		}
	}
if (!KbLchangeFlag)
	{
	if (defaultIndex!=-1)
		{
		lastInputLang=ActivateKeyboardLayout(HKLlplist[defaultIndex], KLF_SETFORPROCESS);
		KbLchangeFlag=true;					
		}
	else
		if (firstLTRKbL!=-1)
			{
			lastInputLang=ActivateKeyboardLayout(HKLlplist[firstLTRKbL], KLF_SETFORPROCESS);
			KbLchangeFlag=true;
			}
	}
#endif //Q_WS_WIN

#if defined( Q_WS_X11 )
if (lastInputLang!=0)
	{
	return;
	}
XKeyboard xkb;
KbLcounts = xkb.groupCount();
StringVector installedLangSymbols = xkb.groupSymbols();
lastInputLang = xkb.currentGroupNum()+1;
for (int i=0; i<KbLcounts; ++i)
	{
	std::string symb = installedLangSymbols.at(i);
	if (!flagFirstLTRKbL && symb != "ir" && symb != "ara" && symb != "il" && symb !="af" && symb != "pk")
		{
		firstLTRKbL=i;
		flagFirstLTRKbL=true;
		}
	if (symb == "us")
		{
		xkb.setGroupByNum(i);
		KbLchangeFlag=true;
		break;
		}
	}
if (!KbLchangeFlag)
	{
	if (defaultIndex!=-1)
		{
		xkb.setGroupByNum(defaultIndex);
		KbLchangeFlag=true;					
		}
	else
		if (firstLTRKbL!=-1)
			{
			xkb.setGroupByNum(firstLTRKbL);
			KbLchangeFlag=true;
			}
	}
#endif // Q_WS_X11
//lastLangAutoChanged = false;
}

void QBiDiExtender::bidiModeToggle(QAction *action)
{
QBiDiExtender::bidiMode=(QBiDiExtender::LineBiDiMode)action->data().toInt();

if (QBiDiExtender::bidiMode==4)
	{
	BiDiLoader->autoNewLine(true);
	return;
	}
else BiDiLoader->autoNewLine(false);

if (QBiDiExtender::bidiMode==2)
	{
	if (autoBiDiEnabled) lastBiDiEnabledStatus=1;
	else lastBiDiEnabledStatus=0;

	BiDiLoader->setParentLayoutDirection(Qt::RightToLeft);

	//QTextCursor tmpCur;
	if (editorForBiDi)
		{
		applyBiDiModeToEditor(2);
		}
	return;
	}
else
	{
	if (QBiDiExtender::bidiMode==3 || QBiDiExtender::bidiMode==1)
		{
		BiDiLoader->setParentLayoutDirection(Qt::LeftToRight);
		}
	if (lastBiDiEnabledStatus!=-1)
		{
		lastBiDiEnabledStatus=-1;
		}
	}

	if (editorForBiDi && autoBiDiEnabled)
		{
		applyBiDiModeToEditor(QBiDiExtender::bidiMode);
		}
}

void QBiDiExtender::RTL( QTextCursor *tmpCursor )
{
if (!editorForBiDi || !tmpCursor) return;
QTextBlockFormat tmpBlockFormat = tmpCursor->blockFormat();
//if (tmpBlockFormat.layoutDirection() == Qt::RightToLeft) {
//    return;
//}
//#ifndef TEXMAKERVERSION
tmpBlockFormat.setLayoutDirection( Qt::RightToLeft );
tmpCursor->setBlockFormat( tmpBlockFormat );
//#else
//if (tmpBlockFormat.layoutDirection() != Qt::LayoutDirectionAuto) {
//    tmpBlockFormat.setLayoutDirection( Qt::RightToLeft );
//    tmpCursor->setBlockFormat( tmpBlockFormat );
//}
//else {
//    if (!tmpCursor->hasSelection()) {
//        int oldPos = tmpCursor->position();
//        QTextBlock b(tmpCursor->block());
//        if (b.isValid()) {
//            BlockData* blockData = BlockData::data(b);
//            if (!blockData) {
//                blockData = new BlockData;
//            }

//            QString blockText = b.text();
//            if (blockText.isEmpty() || blockText.at(0) != QChar(RLM) ) {
//                tmpCursor->setPosition(b.position());
//                tmpCursor->
//            }
//        }
//    }
//    else {

//    }
//}
//#endif
forceDisableBiDirector=true;//new//setTextCursor(tmpCursor) emits verticalSlider()::valueChanged(int) Signal
editorForBiDi->setTextCursor(*tmpCursor);//new//added for selection bug
forceDisableBiDirector=false;
}

void QBiDiExtender::LTR( QTextCursor *tmpCursor )
{
if (!editorForBiDi || !tmpCursor) return;
QTextBlockFormat tmpBlockFormat = tmpCursor->blockFormat();
tmpBlockFormat.setLayoutDirection( Qt::LeftToRight );
tmpCursor->setBlockFormat( tmpBlockFormat );
forceDisableBiDirector=true;//new//setTextCursor(tmpCursor) emits verticalSlider()::valueChanged(int) Signal
editorForBiDi->setTextCursor(*tmpCursor);//new//added for selection bug
forceDisableBiDirector=false;
}

void QBiDiExtender::currentLineLTR()
{
if (!editorForBiDi) return;
QTextCursor tmpCursor = editorForBiDi->textCursor();
//QTextBlock currentBlock = tmpCursor.block();
LTR(&tmpCursor);
editorForBiDi->setTextCursor(tmpCursor);
}

void QBiDiExtender::currentLineRTL()
{
if (!editorForBiDi) return;
QTextCursor tmpCursor = editorForBiDi->textCursor();
//QTextBlock currentBlock = tmpCursor.block();
RTL(&tmpCursor);
editorForBiDi->setTextCursor(tmpCursor);
}

void QBiDiExtender::applyBiDiModeToEditor(int mode)
{
if (!editorForBiDi) return;
editorLastBiDiModeApplied=mode;

QBiDiExtender::forceSkiped = true;

bool  MODIFIED =editorForBiDi->document()->isModified();
if (mode==2)
	{
	editorForBiDi->setLayoutDirection(Qt::RightToLeft);
	
	QTextCursor tmpCur=editorForBiDi->textCursor();
	tmpCur.select(QTextCursor::Document);
	RTL(&tmpCur);
	tmpCur.clearSelection();
	editorForBiDi->setTextCursor(tmpCur);
	}
else if (mode==3)
	{
	editorForBiDi->setLayoutDirection(Qt::LeftToRight);
	
	QTextCursor tmpCur=editorForBiDi->textCursor();
	tmpCur.select(QTextCursor::Document);
	LTR(&tmpCur);
	tmpCur.clearSelection();
	editorForBiDi->setTextCursor(tmpCur);
	}
else if (mode == 1) //mode == 1
	{
    //if (!QBiDiExtender::ptdOpenFlag)
    {
        //QMessageBox::information(0,"editorForBiDi->layoutDirection()",QString::number((int)editorForBiDi->layoutDirection()));
        editorForBiDi->setLayoutDirection(Qt::LayoutDirectionAuto);

#if QT_VERSION >= 0x040700
        QTextCursor tmpCur=editorForBiDi->textCursor();
        tmpCur.select(QTextCursor::Document);
    QTextBlockFormat tmpBlockFormat = tmpCur.blockFormat();
    tmpBlockFormat.setLayoutDirection( Qt::LayoutDirectionAuto );
    tmpCur.setBlockFormat( tmpBlockFormat );
    forceDisableBiDirector=true;//new//setTextCursor(tmpCursor) emits verticalSlider()::valueChanged(int) Signal
    //editorForBiDi->setTextCursor(tmpCur);//new//added for selection bug
    forceDisableBiDirector=false;
        tmpCur.clearSelection();
        editorForBiDi->setTextCursor(tmpCur);
#endif
        }
//        else {
//#if QT_VERSION >= 0x040700
//            editorForBiDi->setLayoutDirection(Qt::LayoutDirectionAuto);
//            QTextCursor tmpCur=editorForBiDi->textCursor();
//            tmpCur.select(QTextCursor::Document);
//            QTextBlockFormat tmpBlockFormat = tmpCur.blockFormat();
//            tmpBlockFormat.setLayoutDirection( Qt::LayoutDirectionAuto );
//            tmpCur.mergeBlockFormat(tmpBlockFormat);
//            tmpCur.clearSelection();
//            editorForBiDi->setTextCursor(tmpCur);
//#endif
//        }
	}
if( MODIFIED ) 
	editorForBiDi->document()->setModified(TRUE );
else
	editorForBiDi->document()->setModified( FALSE );
editorForBiDi->verticalScrollBar()->setValue(0);
//qDebug() << "applyBiDiModeToEditor-Before-FALSE";
QBiDiExtender::forceSkiped = false;
emit doStoppedSteps();
//qDebug() << "applyBiDiModeToEditor-AFTER-False";
}

void QBiDiExtender::toggleArrowKeysReverseState(bool checked)
{
QBiDiExtender::noReverseArrowKeys=checked;
}

//bool QBiDiExtender::mustSkipedForcely()
//{
//	return forceSkiped;
//}

bool QBiDiExtender::pressedKeyEvent( QKeyEvent * e )
{
#if defined EDITOR_SUPPORT_MATHMODE
if ( e->text()==QString(QChar('\\')) && QBiDiExtender::canChangesInputLang )
	{
	QTextCursor tc=editorForBiDi->textCursor();	
	int lastBlockState=tc.block().userState();//temp
	qDebug()<<"000backSlashFlag="<<backSlashFlag<<"lastInputLang="<<lastInputLang<<"lastBlockState="<<lastBlockState;
	if (lastBlockState!=2 && lastInputLang==0) // last BlockState is not MathState
		{
		backSlashFlag=true;
		/////// changeKbdLayout() uses win32 API///////////
		changeKbdLayout();
		qDebug()<<"11111backSlashFlag="<<backSlashFlag<<"lastInputLang="<<lastInputLang<<"lastBlockState="<<lastBlockState;
		}
	}
else
	{
		qDebug()<<"2222222backSlashFlag="<<backSlashFlag<<"lastInputLang="<<lastInputLang;
	if ( QBiDiExtender::canChangesInputLang && backSlashFlag && lastInputLang!=0
		&& (e->text()=="%" || e->text()==" " || e->key() == Qt::Key_Enter
		|| e->key() == Qt::Key_Return  || e->text()=="\t" || e->text()=="{"/*added for commands with argumants*/) )
		{qDebug()<<"333333333333intoooooo IF";
		backSlashFlag=false;
		if (e->text()!="{")
			processInputLang(true, true);
		else
			processInputLang(false, true);
		//lastInputLang=0;
		}
	}

if ( e->modifiers()==Qt::ShiftModifier /*&& QBiDiExtender::canChangesInputLang*/)
	{
#if defined(Q_WS_WIN)
	if (e->nativeVirtualKey() == Qt::Key_4)
#endif
#if defined(Q_WS_X11)
	if (e->nativeScanCode() == 13) //scan code=13 is Qt::Key_4//Linux based OSes
#endif
#if defined(Q_WS_MACX)
	if (e->nativeVirtualKey() == 21) //virtual code=21 is Qt::Key_4//MAC OS X
#endif
		{
		if (backSlashFlag)
			{
			qDebug()<<"-----3-into-backSlashFlag="<<backSlashFlag;
			backSlashFlag=false;
			return true;
			//processInputLang(true, true);
			//ActivateKeyboardLayout(lastInputLang, KLF_SETFORPROCESS);
			//lastInputLang=0;
			}
		forceDisableBiDirector = true;
		//e->ignore();
		Qt::LayoutDirection tmpLayoutDir = curLayoutDir;
		QString text;
		QTextCursor tc=editorForBiDi->textCursor();
		tc.joinPreviousEditBlock();
		tc.removeSelectedText();
		int tmpBlockState1, tmpBlockState2, tmpBlockState3;
		tmpBlockState1=tc.block().userState();//temp
		/////////////////////////////////////////////////////////////////////////////////
		tc.insertText(QString("$"));
		tc.endEditBlock();
		editorForBiDi->setTextCursor(tc);
		tc.joinPreviousEditBlock();

		tmpBlockState2=tc.block().userState();//temp
		tc.insertText(QString("$"));
		tc.endEditBlock();
		editorForBiDi->setTextCursor(tc);
		tc.joinPreviousEditBlock();
		tmpBlockState3=tc.block().userState();//temp
		tc.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
		QString tmpline=tc.selectedText();
		tmpline=tmpline.left(tmpline.lastIndexOf(QString("$")));
		tmpline=tmpline.left(tmpline.lastIndexOf(QString("$")));
		tc.insertText(tmpline);
		tc.clearSelection();
		tc.endEditBlock();
		editorForBiDi->setTextCursor(tc);
		tc.joinPreviousEditBlock();
		int dollarFlag= 100*tmpBlockState1+10*tmpBlockState2+tmpBlockState3;
		if (dollarFlag==22)
			firstDollar=0;
		else
			if (dollarFlag==220)
				firstDollar=9;
			else
				if (dollarFlag==200)
					firstDollar=1;
				else
					if (dollarFlag==2)
						firstDollar=-1;
		if (docBiDiAlgorithm=="BiDiAlgorithmDefault")
			{
			text=QChar(LRM)+QString("$")+QChar(LRM);//new
			}
		else
			text=QString("$");

		if (firstDollar==0)
			{
			/////// changeKbdLayout() uses win32 API///////////
			changeKbdLayout();
			/////////////////////////
			tc.insertText(text);
			tc.endEditBlock();
			editorForBiDi->setTextCursor(tc);
			tc.joinPreviousEditBlock();
			}
		else
			if (firstDollar==1 || firstDollar==-1)
				{
				if (dollarFlag!=2)
					{
					processInputLang(true, true);
					//ActivateKeyboardLayout(lastInputLang, KLF_SETFORPROCESS);
					//lastInputLang=0;
					}
				else
					{
						//for skip last RLM and insert '$' before it.
						QString line = tc.block().text();
						int lastDollar = line.lastIndexOf("$");
						tc.setPosition(tc.block().position()+lastDollar+1);
					}
				
				tc.insertText(text+QChar(RLM));
				
				tc.endEditBlock();
				editorForBiDi->setTextCursor(tc);
				tc.joinPreviousEditBlock();
				}
			else
				{
				tc.insertText("$");
				tc.endEditBlock();
				editorForBiDi->setTextCursor(tc);
				tc.joinPreviousEditBlock();
				}
		tc.endEditBlock();
		forceDisableBiDirector = false;
		return false;//e->ignore();
		}
	}
#endif //EDITOR_SUPPORT_MATHMODE

if (!e->text().isEmpty() && e->text().at(0) == QChar(12,6))//cell=12,row=6 is for persian and arabic comma
	{
	QTextCursor ttc=editorForBiDi->textCursor();
	ttc.insertText(QChar(RLM));
	editorForBiDi->setTextCursor(ttc);
	}

if (e->key() == Qt::Key_End)
	{
	if (lastInputLangBeforeLTR_Text)
		{
#if defined( Q_WS_WIN )
		ActivateKeyboardLayout(lastInputLangBeforeLTR_Text, KLF_SETFORPROCESS);
#endif

#if defined( Q_WS_X11 )
		XKeyboard xkb;
		xkb.setGroupByNum(lastInputLangBeforeLTR_Text-1);
#endif
		//changeKbdLayout();
		lastInputLangBeforeLTR_Text = 0;
		}
		//processInputLang(true, true);
	}

if (e->key() == Qt::Key_Up)//new
	{
	QTextCursor ttc=editorForBiDi->textCursor();
	int curLinPos=ttc.position();
	QTextCursor::MoveMode ttcMM;
	if ( QApplication::keyboardModifiers() == Qt::ShiftModifier )
		ttcMM=QTextCursor::KeepAnchor;
	else
		ttcMM=QTextCursor::MoveAnchor;
	ttc.movePosition(QTextCursor::Up, ttcMM);
	int tmpPos=ttc.position();
	if ( tmpPos == curLinPos && editorForBiDi->textCursor().block().blockNumber()!=0 )
		{
		ttc.movePosition(QTextCursor::Left, ttcMM);
		ttc.movePosition(QTextCursor::Up, ttcMM);
		ttc.movePosition(QTextCursor::Right, ttcMM);
		}
	editorForBiDi->setTextCursor(ttc);
	return false;//e->ignore();
	}

if (docBiDiAlgorithm=="BiDiAlgorithmDefault" && curLayoutDir == Qt::LeftToRight
	&& (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) )
	{
	editorForBiDi->textCursor().insertText(QChar(LRM));
	}
if (e->key() == Qt::Key_Left)//new
	{
	noLRMneed = true;//new

	if (docBiDiAlgorithm=="BiDiAlgorithmDefault")
		{
		disconnect(editorForBiDi->document(), SIGNAL(contentsChanged()), this, SLOT(addLRMinTyping()));
		}
	QTextCursor tc = editorForBiDi->textCursor();

	QTextCursor::MoveMode ttcMM;
	if ( QApplication::keyboardModifiers() == Qt::ShiftModifier )
		ttcMM=QTextCursor::KeepAnchor;
	else
		ttcMM=QTextCursor::MoveAnchor;
	if (e->isAutoRepeat() && QBiDiExtender::noReverseArrowKeys)
		{
		tc.movePosition(QTextCursor::PreviousCharacter, ttcMM);
		editorForBiDi->setTextCursor(tc);
		return false;			
		}

	if (docBiDiAlgorithm=="BiDiAlgorithmDefault")
		{
		if (!tc.hasSelection())
			{
			int curPos = tc.position();
			int oldPos=curPos;
			tc.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
			curPos = tc.position();
			if (tc.hasSelection())
				{
				int index;
				if (curPos >= oldPos) index=tc.selectedText().size()-1;
				else index=0;
				while (tc.selectedText().at(index)==QChar(LRM) && !tc.atEnd() && !tc.atStart() )
					{
					oldPos=curPos;
					tc.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
					curPos = tc.position();
					if (curPos >= oldPos) index=tc.selectedText().size()-1;
					else index=0;
					}
				if ( tc.selectedText().at(index)!=QChar(LRM) )//new//if
					tc.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
				editorForBiDi->setTextCursor(tc);
				}
			}
		if (QBiDiExtender::flagAutoLRM && QBiDiExtender::bidiEnabled) connect(editorForBiDi->document(), SIGNAL(contentsChanged()), this, SLOT(addLRMinTyping()));
		}
	}
if (e->key() == Qt::Key_Right)
	{
	noLRMneed = true;

	if (docBiDiAlgorithm=="BiDiAlgorithmDefault")
		{
		disconnect(editorForBiDi->document(), SIGNAL(contentsChanged()), this, SLOT(addLRMinTyping()));
		}
	QTextCursor tc = editorForBiDi->textCursor();
	QTextCursor::MoveMode ttcMM;
	if ( QApplication::keyboardModifiers() == Qt::ShiftModifier )
		ttcMM=QTextCursor::KeepAnchor;
	else
		ttcMM=QTextCursor::MoveAnchor;
	if (e->isAutoRepeat() && QBiDiExtender::noReverseArrowKeys)
		{
		tc.movePosition(QTextCursor::NextCharacter, ttcMM);
		editorForBiDi->setTextCursor(tc);
		return false;			
		}
	if (docBiDiAlgorithm=="BiDiAlgorithmDefault")
		{
		int curPos = tc.position();
		int oldPos=curPos;
		tc.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
		curPos = tc.position();
		if (tc.hasSelection())
			{
			int index;
			if (curPos >= oldPos) index=tc.selectedText().size()-1;
			else index=0;
			while (tc.selectedText().at(index)==QChar(LRM) && !tc.atEnd() && !tc.atStart() )
				{
				oldPos=curPos;
				tc.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
				curPos = tc.position();
				if (curPos >= oldPos) index=tc.selectedText().size()-1;
				else index=0;
				}
			if ( tc.selectedText().at(index)!=QChar(LRM) )//new//if
				tc.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
			editorForBiDi->setTextCursor(tc);
			}
		if (QBiDiExtender::flagAutoLRM && QBiDiExtender::bidiEnabled) connect(editorForBiDi->document(), SIGNAL(contentsChanged()), this, SLOT(addLRMinTyping()));
		}
	}

	/////Qt::Key_Backspace
if (e->key() == Qt::Key_Backspace && !editorForBiDi->textCursor().hasSelection())
	{
	backSpaceFlag = true;
	disconnect(editorForBiDi->document(), SIGNAL(contentsChanged()), this, SLOT(addLRMinTyping()));
	QTextCursor tc = editorForBiDi->textCursor();
	if (tc.hasSelection()) tc.removeSelectedText();
	int curPos = tc.position();
	int oldPos=curPos;
	tc.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
	curPos = tc.position();
	QString tempSel;//temp
	QChar tempCh;//temp
	if (tc.hasSelection())
		{
		int sizee=tc.selectedText().size();
		int countLRM=tc.selectedText().count(QChar(LRM));
		tempSel=tc.selectedText();
		tempCh=tc.selectedText().at(0);//temp
		bool temp1=false,temp2=false;
		if (tc.selectedText().at(0)==QChar(LRM))
			temp1=true;
		if (tc.selectedText()==QChar(LRM))
			temp2=true;
		while (tc.selectedText().at(0)==QChar(LRM) && !tc.atEnd() && !tc.atStart() )
			{
			tempCh=tc.selectedText().at(0);//temp
			oldPos=curPos;
			tc.clearSelection();
			tc.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
			tempSel=tc.selectedText();//temp
			sizee=tc.selectedText().size();
			countLRM=tc.selectedText().count(QChar(LRM));
			curPos = tc.position();
			}
		if ( tc.selectedText().at(0)!=QChar(LRM) )//&& (tc.atEnd() || tc.atStart()) )
			tc.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor);
		tc.clearSelection();
		editorForBiDi->setTextCursor(tc);
		}
	if (QBiDiExtender::flagAutoLRM && QBiDiExtender::bidiEnabled) connect(editorForBiDi->document(), SIGNAL(contentsChanged()), this, SLOT(addLRMinTyping()));
	}
else
	backSpaceFlag = false;
	///////////////////////////////////////////////////////////////
	////Key_Delete
if (e->key() == Qt::Key_Delete && !editorForBiDi->textCursor().hasSelection())
	{
	disconnect(editorForBiDi->document(), SIGNAL(contentsChanged()), this, SLOT(addLRMinTyping()));
	QTextCursor tc = editorForBiDi->textCursor();
	if (tc.hasSelection()) tc.removeSelectedText();
	int curPos = tc.position();
	int oldPos=curPos;
	tc.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
	curPos = tc.position();
	QString tempSel;//temp
	QChar tempCh;//temp
	if (tc.hasSelection())
		{
		int sizee=tc.selectedText().size();
		int countLRM=tc.selectedText().count(QChar(LRM));
		tempSel=tc.selectedText();
		tempCh=tc.selectedText().at(0);//temp
		bool temp1=false,temp2=false;
		if (tc.selectedText().at(0)==QChar(LRM))
			temp1=true;
		if (tc.selectedText()==QChar(LRM))
			temp2=true;
		while (tc.selectedText().at(0)==QChar(LRM) && !tc.atEnd() && !tc.atStart() )
			{
			tempCh=tc.selectedText().at(0);//temp
			oldPos=curPos;
			tc.clearSelection();
			tc.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
			tempSel=tc.selectedText();//temp
			sizee=tc.selectedText().size();
			countLRM=tc.selectedText().count(QChar(LRM));
			curPos = tc.position();
			}
		if ( tc.selectedText().at(0)!=QChar(LRM) )//&& (tc.atEnd() || tc.atStart()) )
			tc.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor);
		tc.clearSelection();
		editorForBiDi->setTextCursor(tc);
		}
	if (QBiDiExtender::flagAutoLRM && QBiDiExtender::bidiEnabled) connect(editorForBiDi->document(), SIGNAL(contentsChanged()), this, SLOT(addLRMinTyping()));
	}
	//////////////////////////////////////////////////
if (e->key() == Qt::Key_Space /*|| e->key() == Qt::Key_Tab*/)//new//Key_Tab has very bad effect in LTR text!
	{
	appendLRM=false;
	noLRMneed = true;//new
	disconnect(editorForBiDi->document(), SIGNAL(contentsChanged()), this, SLOT(addLRMinTyping()));
	QTextCursor tc = editorForBiDi->textCursor();
	tc.joinPreviousEditBlock();
	if (tc.hasSelection()) tc.removeSelectedText();

	tc.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
	QString tmpSel = tc.selectedText();
	QStringList selChars = tmpSel.split("", QString::SkipEmptyParts);
	bool flagInsertLRM=false;
	if ( !selChars.isEmpty() )
		{
		int tmpInd = selChars.size()-1;
		while(tmpInd-1>=0)
			{
			if (selChars.at(tmpInd-1).isEmpty() || selChars.at(tmpInd-1).at(0).isSpace())
				{
				--tmpInd;
				continue;
				}
			if ( selChars.at(tmpInd-1).at(0) == QChar(LRM) )
				{
				--tmpInd;
				if (tmpInd-1>=0)
					{
					if ( selChars.at(tmpInd-1).at(0).isSpace() || selChars.at(tmpInd-1).at(0) == QChar(LRM) || (whatIsDir(selChars.at(tmpInd-1).at(0)) == Qt::RightToLeft) )
						{
						selChars[tmpInd] = "";
						flagInsertLRM = true;
						continue;
						}
					else break;
					}
				else
					{
					selChars[tmpInd] = "";
					flagInsertLRM = true;
					break;
					}
				}
			else break;
			}
		tmpSel = selChars.join("");
		tc.removeSelectedText();
		tc.insertText(tmpSel);
		}
///////////////////////////////////////////////////////////////////

	int curPos = tc.position();
	int oldPos=curPos;
	int firstPos=curPos;
	tc.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
	curPos = tc.position();
	QString tempSel;//temp
	QChar tempCh;//temp
	bool tcEditBlockEnded = false;
	if (tc.hasSelection())
		{
		int sizee=tc.selectedText().size();
		int countLRM=tc.selectedText().count(QChar(LRM));
		tempSel=tc.selectedText();
		tempCh=tc.selectedText().at(0);//temp
		bool temp1=false,temp2=false;
		if (tc.selectedText().at(0)==QChar(LRM))
			temp1=true;
		if (tc.selectedText()==QChar(LRM))
			temp2=true;
		while (tc.selectedText().at(0)==QChar(LRM) && !tc.atEnd() && !tc.atStart() )
			{
			tempCh=tc.selectedText().at(0);//temp
			oldPos=curPos;
			tc.clearSelection();
			tc.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
			tempSel=tc.selectedText();//temp
			sizee=tc.selectedText().size();
			countLRM=tc.selectedText().count(QChar(LRM));
			curPos = tc.position();
			}
		if ( tc.selectedText().at(0)!=QChar(LRM) )
			{
			if ( (whatIsDir( tc.selectedText().at(0)) == Qt::RightToLeft) ||  tc.selectedText().at(0).isSpace() )
				tc.setPosition(oldPos, QTextCursor::MoveAnchor);
			else
				tc.setPosition(firstPos, QTextCursor::MoveAnchor);
			}
		tc.clearSelection();
		tc.endEditBlock();
		tcEditBlockEnded = true;
		editorForBiDi->setTextCursor(tc);
		}
	//if (QBiDiExtender::flagAutoLRM && QBiDiExtender::bidiEnabled) connect(editorForBiDi->document(), SIGNAL(contentsChanged()), this, SLOT(addLRMinTyping()));
	if (flagInsertLRM /*&& (QApplication::keyboardInputDirection() == Qt::LeftToRight)*/ )
	{
		if (tcEditBlockEnded)
			tc.joinPreviousEditBlock();
		tc.insertText(QString(" ")+QChar(LRM)+QChar(LRM));
		tc.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor);
		tc.endEditBlock();
		editorForBiDi->setTextCursor(tc);
		if (QBiDiExtender::flagAutoLRM && QBiDiExtender::bidiEnabled) connect(editorForBiDi->document(), SIGNAL(contentsChanged()), this, SLOT(addLRMinTyping()));
		return false;
		}
		
	if (QBiDiExtender::flagAutoLRM && QBiDiExtender::bidiEnabled) connect(editorForBiDi->document(), SIGNAL(contentsChanged()), this, SLOT(addLRMinTyping()));
	if (!tcEditBlockEnded)
		tc.endEditBlock();
	}
qDebug() << "pKey--1714";
return true;//e->accept();
}

//#if defined( Q_WS_MACX )
////#include </Developer/SDKs/MacOSX10.5.sdk/System/Library/Frameworks/Carbon.framework/Versions/A/Frameworks/TextInputSources.h>
//#include "TextInputSources.h"
//#endif
void QBiDiExtender::processInputLang(bool loadLastInputLang, bool clearLastInputLang)
{
if (loadLastInputLang && (int)lastInputLang!=0)
	{
#if defined( Q_WS_WIN )
	ActivateKeyboardLayout(lastInputLang, KLF_SETFORPROCESS);
#endif

#if defined( Q_WS_X11 )
	XKeyboard xkb;
	xkb.setGroupByNum(lastInputLang-1);
#endif
//#if defined( Q_WS_MACX )
//		CFArrayRef selectableInputSource=TISCreateInputSourceList (NULL, false);
//		
//		CFTypeID dd=TISInputSourceGetTypeID();
//		QMessageBox::information(0,"111111111MAC-MAC",tr("currentID=*%1*").arg((int)dd));
//#endif
	}
if (clearLastInputLang)
	{
	lastInputLang=0;
	}
}

int QBiDiExtender::getLastInputLang()
{
#if defined( Q_WS_WIN ) || defined( Q_WS_X11 )
return (int)lastInputLang;
#else
return 0;
#endif
}

void QBiDiExtender::editorBiDiMenu(QMenu *menu, const QString &type)
{
QAction *Act;

if (type=="BIDI_CONTEXT_MENU")
	{
	Act = menu->addAction(tr("insert: \\lr{...}"), this, SLOT(contextMenuInsertTag()));
	Act->setData("L\\lr{");
	Act = menu->addAction(tr("insert: \\rl{...}"), this, SLOT(contextMenuInsertTag()));
	Act->setData("R\\rl{");

	Act = menu->addAction(tr("insert LRM"), this, SLOT(contextMenuAddLRM()));
	Act->setEnabled(true);

	if (!editorForBiDi->textCursor().hasSelection())
		{
		Act = menu->addAction(tr("insert LTR text"), this, SLOT(contextMenuAddLRE()));
		Act->setShortcut(Qt::CTRL+Qt::Key_Period);
		}
	else
		Act = menu->addAction(tr("consider as LTR text"), this, SLOT(contextMenuAddLRE()));
	Act = menu->addAction(tr("Remove Unicode Control Characters"), this, SLOT(remLRMfromSelection()));
	Act->setEnabled(editorForBiDi->textCursor().hasSelection());
	menu->addSeparator();

	if (editorForBiDi)//MODIFIED:17 APRIL 2010
		{
		QMenu *temp=editorForBiDi->createStandardContextMenu();
		QAction *unicodeControlCharacterAction=temp->actions().last();
		menu->insertAction(0, unicodeControlCharacterAction);
		menu->addSeparator();
		}
	}
//	if (type=="BIDI_MAIN_MENU")
//		{
//		//do somethings
//		}
//	if (type=="BIDI_HELP_MENU")
//		{
//		//do somethings
//		}
}

void QBiDiExtender::readBiDiSettings(QSettings *settingsObject)
{
settingsObject->beginGroup("QBiDiExtender");
QBiDiExtender::bidiEnabled=settingsObject->value("Bi-Di Support",true).toBool();
//QBiDiExtender::autoBiDiEnabled=settingsObject->value("Auto Bidirector",true).toBool();
QBiDiExtender::flagAutoLRM=settingsObject->value("Add LRM Automatically",true).toBool();
QBiDiExtender::bidiMode=(QBiDiExtender::LineBiDiMode)settingsObject->value("bidiMode",(int)QBiDiExtender::AutoRightOrLeft).toInt();
QBiDiExtender::canChangesInputLang=settingsObject->value("BackSlash Change Lang",true).toBool();
QBiDiExtender::ptdSupportFlag=true;//settingsObject->value("PTD File Save",true).toBool();
QBiDiExtender::noReverseArrowKeys=settingsObject->value("Reverse Arrow Keys",false).toBool();
//QBiDiExtender::docBiDiAlgorithm=settingsObject->value("BiDiAlgorithm", QString("BiDiAlgorithmDefault")).toString();
//BiDiSettings_newDocBiDiAlgorithm=QBiDiExtender::docBiDiAlgorithm;//MODIFIED: 21 MAY 2010
settingsObject->endGroup();
}

void QBiDiExtender::saveBiDiSettings(QSettings *settingsObject)
{
settingsObject->beginGroup("QBiDiExtender");
settingsObject->setValue("Bi-Di Support",QBiDiExtender::bidiEnabled);
settingsObject->setValue("bidiMode",QBiDiExtender::bidiMode);
settingsObject->setValue("PTD File Save",	QBiDiExtender::ptdSupportFlag);
//settingsObject->setValue("Auto Bidirector",QBiDiExtender::autoBiDiEnabled);
settingsObject->setValue("Add LRM Automatically",QBiDiExtender::flagAutoLRM);
settingsObject->setValue("BackSlash Change Lang",QBiDiExtender::canChangesInputLang);
settingsObject->setValue("Reverse Arrow Keys",QBiDiExtender::noReverseArrowKeys);
//settingsObject->setValue("BiDiAlgorithm", BiDiSettings_newDocBiDiAlgorithm);//MODIFIED: 21 MAY 2010
settingsObject->endGroup();
}

void QBiDiExtender::stampedBiDiLogoToImage(QPixmap *pixmap)
{
QPixmap bidiLogoPixmap(":/image/BiDi-Logo.png");
QRect targetImageRect(pixmap->rect());
QRectF targetRect(targetImageRect.left(),targetImageRect.top()+targetImageRect.height()/2, targetImageRect.width(),targetImageRect.height()/2);
QRectF bidiLogoRect(bidiLogoPixmap.rect());

QPainter painter(pixmap);
painter.drawPixmap(targetRect, bidiLogoPixmap, bidiLogoRect);
}

bool QBiDiExtender::isUnicodeControlCharacters(const QChar &ch)
{
    if (!unicodeControlCharacters().contains(QString(ch))) {
        return false;
    }
    else {
        return true;
    }
}

QString QBiDiExtender::removeUnicodeControlCharacters(const QString &str)
{
	QString text(str);
    return text.remove(QRegExp("["+unicodeControlCharacters().join("")+"]+"));
}

QStringList QBiDiExtender::unicodeControlCharacters()
{
    QStringList controlChars;
    QString lrm(QChar(LRM));
    QString rlm(QChar(RLM));
    QString lre(QChar(LRE));
    QString rle(QChar(RLE));
    QString pdf(QChar(PDF));
    QString lro(QChar(LRO));
    QString rlo(QChar(RLO));
    controlChars << lrm << rlm << lre << rle << pdf << lro << rlo;

    return controlChars;
}

QString QBiDiExtender::toPatternWithUnicodeControlCharacters(const QString &str)
{
    QString result;
    QString controlCharacters = "["+unicodeControlCharacters().join("")+"]*";
    QStringList characterlist = str.split("", QString::SkipEmptyParts);
    const QString special = "[]{}\\()\"\'.*?^$";
    foreach (QString character, characterlist) {
        if (special.contains(character)) {
            character.prepend("\\");
        }
        character.append(controlCharacters);
        result.append(character);
    }
    result.prepend(controlCharacters);

    return result;
}

/*******************************************************/
/*******************************************************/
///////////////////QBiDiInitializer Class////////////////
/*******************************************************/
/*******************************************************/


//Members definition of QBiDiInitializer class
//temp
//int QBiDiInitializer::numberOfQBiDiExtenders=0;

QBiDiInitializer::QBiDiInitializer(QWidget *qwidget)
    : QObject(qwidget), editorWidget(qwidget), bidiToolBar(0)
{
//QBiDiExtender::BiDiLogoRES=logoPath;

if (QBiDiExtender::bidiMode==QBiDiExtender::NewLinePerLanguageChanging)
	autoNewLine(true);
else
	autoNewLine(false);
if (QBiDiExtender::bidiMode==QBiDiExtender::RightToLeft)
	editorWidget->setLayoutDirection(Qt::RightToLeft);
if (QBiDiExtender::bidiMode==QBiDiExtender::LeftToRight)
	editorWidget->setLayoutDirection(Qt::LeftToRight);
}

QBiDiInitializer::~QBiDiInitializer()
{
}

QString QBiDiInitializer::openPtdFile(const QString &f, QTextCodec* codec, const QString &plainContent)
{
QString ptdFileVersion = "1";
QBiDiExtender::ptdOpenFlag = false;
QString tmpContent=plainContent;
QString fileType = QFileInfo(f).suffix().toLower() == "tex" ? "" : ("_"+QFileInfo(f).suffix().toLower());

QString ptdName=QFile( f ).fileName().left(QFile( f ).fileName().size()-4)+fileType+".ptd";
ptdName.replace(QChar('\\'),QChar('/'));
QString tmpName = ptdName.right( ptdName.size()-ptdName.lastIndexOf(QChar('/'))-1 );
ptdName = ptdName.left( ptdName.lastIndexOf(QChar('/'))+1 )+"."+tmpName;
QFile ptdFile(ptdName);
if (ptdFile.exists())
	{
	if(!codec) codec = QTextCodec::codecForLocale();
	if ( ptdFile.open( QIODevice::ReadOnly ) )
		{
		if (tmpContent.isEmpty())
			{
			QFile texFile(f);
			if (texFile.open( QIODevice::ReadOnly ))
				{
				QTextStream texTS( &texFile );
				texTS.setCodec(codec);
				tmpContent = texTS.readAll();
				texFile.close();
				}
			}
		QTextStream ptdTS( &ptdFile );
		ptdTS.setCodec(codec);
        QString metaDataLine=ptdTS.readLine(0).trimmed();
        QRegExp ptdVersionRegExp("[Vv]\\.([0-9]+)");
        if (metaDataLine.contains(ptdVersionRegExp)) {
            ptdFileVersion = ptdVersionRegExp.cap(1);
        }
		QString htmlContent = ptdTS.readAll();//from second line to the EOF
		//qDebug() << htmlContent;
		//qDebug() << QTextDocumentFragment::fromHtml(htmlContent).toPlainText();
//		if (QBiDiExtender::removeUnicodeControlCharacters(QTextDocumentFragment::fromHtml(htmlContent).toPlainText())
//				==
//			QBiDiExtender::removeUnicodeControlCharacters(tmpContent))
//			qDebug() << "HTML == PLAIN";
//		else
//			qDebug() << "HTML NOT PLAIN";
        metaDataLine.remove("<!").remove(">");
        if (metaDataLine.contains(","))
			{
            QBiDiExtender::ptdFileBiDimode=(QBiDiExtender::LineBiDiMode)QString(metaDataLine.at(0)).toInt();
            metaDataLine.remove(0,2);
			}
		else
			QBiDiExtender::ptdFileBiDimode = QBiDiExtender::Undefined;//maybe bidiMode be better
		QDateTime lMod=QFileInfo(f).lastModified();
		QString lMOD_ST=lMod.toString("yyyyMMddhh:mm:ss");
        if ( /*metaDataLine == lMOD_ST*/
			QBiDiExtender::removeUnicodeControlCharacters(QTextDocumentFragment::fromHtml(htmlContent).toPlainText())
						==	QBiDiExtender::removeUnicodeControlCharacters(tmpContent)
			 && QBiDiExtender::ptdFileBiDimode == QBiDiExtender::bidiMode )//The saved bidiMode must be equal to application's bidiMode
			{
			tmpContent = htmlContent;
			QBiDiExtender::ptdOpenFlag = true;
			}
		ptdFile.close();
		}
	}

    if (ptdFileVersion == "2" && QBiDiExtender::ptdOpenFlag) {
       tmpContent = QTextDocumentFragment::fromHtml(tmpContent).toPlainText();
       //QBiDiExtender::ptdOpenFlag = false;
       //QMessageBox::information(0, "Version", ("ptdVersion="+ptdFileVersion));
    }
return tmpContent;
}

void QBiDiInitializer::applicationBiDiMenu(QMenuBar *menuBar, QMenu *menu, const QString &type)
{
QAction *Act;

if (type=="BIDI_MAIN_MENU")
	{
	if (!menu)
		{
		if (menuBar)
			menu = menuBar->addMenu(tr("Bi-&Directional"));
		else return;
		}

	//BidiMode Menu [start]
	QMenu *bidiModeMenu=menu->addMenu(tr("Bi-Directing &Method"));
	bidiModes = new QActionGroup(editorWidget);
	actFirstBiDiMode = new QAction(tr("&Automatic Direction"), bidiModes);
	actFirstBiDiMode->setData(QBiDiExtender::AutoRightOrLeft);
	actFirstBiDiMode->setCheckable(true);
	if (QBiDiExtender::bidiMode == QBiDiExtender::AutoRightOrLeft)	actFirstBiDiMode->setChecked(true);
#if QT_VERSION >= 0x040700
	bidiModeMenu->addAction(actFirstBiDiMode);
#endif
	Act = new QAction(tr("&RTL Environment"), bidiModes);
	Act->setData(QBiDiExtender::RightToLeft);
	Act->setCheckable(true);
	if (QBiDiExtender::bidiMode == QBiDiExtender::RightToLeft)	Act->setChecked(true);
	bidiModeMenu->addAction(Act);

	Act = new QAction(tr("&LTR Environment"), bidiModes);//old text="&All Strong Character(RTL)"
	Act->setData(QBiDiExtender::LeftToRight);
	Act->setCheckable(true);
	if (QBiDiExtender::bidiMode == QBiDiExtender::LeftToRight)	Act->setChecked(true);
	bidiModeMenu->addAction(Act);
	//////////////////////////////////////////
	
	Act = new QAction(tr("Auto New &Line(RTL)"), bidiModes);
	Act->setData(QBiDiExtender::NewLinePerLanguageChanging);
	Act->setCheckable(true);
	if (QBiDiExtender::bidiMode == QBiDiExtender::NewLinePerLanguageChanging)	Act->setChecked(true);
	bidiModeMenu->addAction(Act);

	connect(bidiModes, SIGNAL(triggered(QAction *)), this, SLOT(toggleBiDiMode(QAction *)));
	bidiModeMenu->addAction(Act);
	//// BidiMode Menu [end]
	
	actRTL = new QAction(QIcon(":/image/format_text_direction_rtl.png"), tr("Text Direction &Right to Left"), editorWidget);
    actRTL->setShortcut(QKeySequence(Qt::CTRL+Qt::ALT+Qt::Key_B,Qt::CTRL+Qt::ALT+Qt::Key_R));
	connect(actRTL, SIGNAL(triggered()), this, SIGNAL(doCurrentLineRTL()));
//	connect(this, SIGNAL(changeEditorActionState(bool)), actRTL, SLOT(setEnabled(bool)));
	menu->addAction(actRTL);
qDebug() << "actRTL";
	actLTR = new QAction(QIcon(":/image/format_text_direction_ltr.png"), tr("Text Direction &Left to Right"), editorWidget);
    actLTR->setShortcut(QKeySequence(Qt::CTRL+Qt::ALT+Qt::Key_B,Qt::CTRL+Qt::ALT+Qt::Key_L));
	connect(actLTR, SIGNAL(triggered()), this, SIGNAL(doCurrentLineLTR()));
//	connect(this, SIGNAL(changeEditorActionState(bool)), actLTR, SLOT(setEnabled(bool)));
	menu->addAction(actLTR);
qDebug() << "actLTR";
	//QAction *
	Act = new QAction(tr("Insert LTR Text"), editorWidget);
	Act->setShortcut(QKeySequence(Qt::CTRL+Qt::Key_Period));
	connect(Act, SIGNAL(triggered()), this, SIGNAL(insertedLTR()));
//	connect(this, SIGNAL(changeEditorActionState(bool)), Act, SLOT(setEnabled(bool)));
	menu->addAction(Act);
	//actInsertLTR = Act;

	menu->addSeparator();
	
	//ptdSaveEnabled(true);//force enabling of PTD Support when needed
	QBiDiExtender::ptdSupportFlag = true;//force enabling of PTD Support when needed

#if defined EDITOR_SUPPORT_MATHMODE
#if defined( Q_WS_WIN ) || defined( Q_WS_X11 )
	Act = new QAction(tr("Changing Language by typing \'\\\'"), editorWidget);
	Act->setToolTip("After typing \'\\\' input language is automatically changed to EN");
	Act->setCheckable(true);
	if (QBiDiExtender::canChangesInputLang) Act->setChecked(true);
	else Act->setChecked(false);
	menu->addAction(Act);
	connect(Act, SIGNAL(toggled(bool)), this, SLOT(toggleAutoChangeLangState(bool)));
#endif
#endif EDITOR_SUPPORT_MATHMODE
	QBiDiExtender::noReverseArrowKeys=false;
    emit changeEditorActionState(false);
	}
if (type=="BIDI_HELP_MENU")
	{
	if (!menu)
		{
		if (menuBar)
			menu = menuBar->addMenu(tr("&BiDi Help"));
		else return;
		}
	QMenu *subMenu=menu->addMenu(QIcon(":/image/BiDi-Logo.png"), tr("Extended &BiDi Help"));
	Act = new QAction(QIcon(":/image/BiDi-Logo.png"), tr("Extended BiDi &Manual..."), editorWidget);
	connect(Act, SIGNAL(triggered()), this, SLOT(BiDiHelp()));
	subMenu->addAction(Act);
	Act = new QAction(QIcon(":/image/BiDi-Logo.png"), tr("About Extended BiDi Support..."), editorWidget);
	connect(Act, SIGNAL(triggered()), this, SLOT(aboutBidiPatch()));
	subMenu->addAction(Act);
	}
//if (type=="BIDI_CONTEXT_MENU")
//	{
//	//do somethings
//	}
}

void QBiDiInitializer::BiDiHelp()
{
QString manFileName, docfile;
#ifdef TEXMAKERVERSION
manFileName = "exBiDiManual-TeXmaker.pdf";
#else //it is texworks
manFileName = "exBiDiManual-TeXworks.pdf";
#endif

#if defined( Q_WS_X11 )
docfile=PREFIX"/share/texmaker/";
#endif
#if defined( Q_WS_MACX )
docfile=QCoreApplication::applicationDirPath() + "/../Resources/";
#endif
#if defined(Q_WS_WIN)
docfile=QCoreApplication::applicationDirPath() + "/";
#endif
docfile = docfile+manFileName;
QFileInfo fic(docfile);

if (fic.exists() && fic.isReadable() )
	{
	emit openApplicationViewer(docfile);
	if (QBiDiExtender::applicationViewerLoadIt)
		{
		QBiDiExtender::applicationViewerLoadIt = false;
		return;
		}
	QDesktopServices::openUrl("file:///"+docfile);
	}
else { QMessageBox::warning( editorWidget,tr("Error"),tr("Extended BiDi help file not found"));}
}

void QBiDiInitializer::aboutBidiPatch()
{
QString applicationVersionStr = tr("This application");
#ifdef bidiTEXMAKERVERSION
applicationVersionStr = tr("bidiTeXmaker-")+QString(bidiTEXMAKERVERSION);
#endif
#ifdef bidiTEXWORKSVERSION
applicationVersionStr = tr("bidiTeXworks-")+QString(bidiTEXWORKSVERSION);
#endif
QMessageBox aboutBidi(editorWidget);
QPixmap bidiLogoPixmap(":/image/BiDi-Logo.png");

aboutBidi.setIconPixmap(bidiLogoPixmap);
aboutBidi.setTextFormat(Qt::RichText);

aboutBidi.setWindowTitle(tr("About Extended BiDi Support"/*BiDi Texmaker*/));

aboutBidi.setText(tr("<br/>%1 uses QBiDiExtender2.<br/>The QBiDiExtender is a Qt class that extends bidi support of QTextEdit class.<br/> The QBiDiExtender2 is prepared for Qt Version > 4.7.0 .<br/><br/>Author: <a href=\"http://pozh.org/\">S. Razi Alavizadeh</a><br/><br/>Project Home Page: <a href=\"http://pozh.org/qbidiextender\">http://pozh.org/qbidiextender</a><br/>ParsiLaTeX: <a href=\"http://www.parsilatex.com/\">http://www.parsilatex.com/</a><br/><br/>Version: %2<br/>Rev: %3<br/>Build Time: %4")
					 .arg(applicationVersionStr).arg(Version::versionString()).arg(Version::globalRevision()).arg(Version::versionDate()) );

//aboutBidi.setDetailedText(tr("QBiDiExtender Project Home Page:\nhttp://pozh.org/qbidiextender\n\nParsiLaTeX webpage:\nhttp://www.parsilatex.com/"/*\n\nDownload Links:\nhttp://srazi.blogfa.com/page/biditexmaker.aspx"*/));
QAbstractButton *aboutQtButton = aboutBidi.addButton(tr("About Qt..."), QMessageBox::ActionRole);
aboutBidi.setStandardButtons(QMessageBox::Ok);
aboutBidi.setEscapeButton(QMessageBox::Ok);
aboutBidi.exec();
if (aboutBidi.clickedButton() == aboutQtButton) QMessageBox::aboutQt(editorWidget, tr("About Qt"));
}

void QBiDiInitializer::autoNewLine(bool checked)
{
if (checked)
	{
	QBiDiExtender::flagAutoLRM = true;
	editorWidget->setLayoutDirection(Qt::RightToLeft);
	}
else
	{
#if QT_VERSION >= 0x040700	
	editorWidget->setLayoutDirection(Qt::LayoutDirectionAuto);
#else
	editorWidget->setLayoutDirection(Qt::LeftToRight);
#endif
	}
}

void QBiDiInitializer::toggleBiDiMode(QAction *action)
{
if (action)
	{
	QBiDiExtender::bidiMode=(QBiDiExtender::LineBiDiMode)action->data().toInt();
	emit bidiModeToggled(action);
	}
}

void QBiDiInitializer::toggleAutoChangeLangState(bool checked)
{
QBiDiExtender::canChangesInputLang=checked;
}

void QBiDiInitializer::setParentLayoutDirection(Qt::LayoutDirection direction)
{
if (editorWidget)
	editorWidget->setLayoutDirection(direction);
}

void QBiDiInitializer::setEditorActionState(bool enable)
{
emit changeEditorActionState(enable);
}

QToolBar *QBiDiInitializer::installBiDiToolBar()
{
	if (bidiToolBar) return bidiToolBar;
	bidiToolBar = new QToolBar(tr("BiDi Tools"), editorWidget);
	bidiToolBar->setObjectName("BiDiToolBar");
	bidiToolBar->addAction(actLTR);
	bidiToolBar->addAction(actRTL);
	return bidiToolBar;
}
