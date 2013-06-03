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


#include <QtGui>

#include "latexhighlighter.h"
#include "latexeditor.h"
#include "blockdata.h"

static bool stringGreaterSize(QString str1, QString str2) {
    return str1.size() > str2.size();
}

LatexHighlighter::LatexHighlighter(QTextDocument *parent,bool spelling,QString ignoredWords,Hunspell *spellChecker)
    : QSyntaxHighlighter(parent)
{
isGraphic=false;
ColorStandard = QColor("#FFFFFF");
ColorComment = QColor("#606060");
ColorMath = QColor(0x00,0x80, 0x00);
ColorCommand=QColor(0x80, 0x00, 0x00);
ColorKeyword=QColor(0x00, 0x00, 0xCC);
ColorVerbatim = QColor("#9A4D00");
ColorTodo=QColor("#FF0000");
ColorKeywordGraphic=QColor("#006699");
ColorNumberGraphic=QColor("#660066");
KeyWords= QString("section{,subsection{,subsubsection{,chapter{,part{,paragraph{,subparagraph{,section*{,subsection*{,subsubsection*{,chapter*{,part*{,paragraph*{,subparagraph*{,label{,includegraphics{,includegraphics[,includegraphics*{,includegraphics*[,include{,input{,begin{,end{").split(",");
QMap<QString, QString>::const_iterator it = LatexEditor::localizedStructureCommands.constBegin();
while (it != LatexEditor::localizedStructureCommands.constEnd()) {
    QStringList alternativeList = QString(it.value()).split("|", QString::SkipEmptyParts);
    foreach (const QString &alternative, alternativeList) {
        KeyWords << (alternative+"{");
        KeyWords << (alternative+"*{");
    }
    ++it;
}
KeyWordsGraphic=QString("void bool bool3 int real pair triple string").split(" ");
KeyWordsGraphicBis=QString("and controls tension atleast curl if else while for do return break continue struct typedef new access import unravel from include quote static public private restricted this explicit true false null cycle newframe operator").split(" ");
qSort(KeyWords.begin(), KeyWords.end(), stringGreaterSize);
qSort(KeyWordsGraphic.begin(), KeyWordsGraphic.end(), stringGreaterSize);
qSort(KeyWordsGraphicBis.begin(), KeyWordsGraphicBis.end(), stringGreaterSize);

//spellingErrorFormat.setFontUnderline(true);
spellingErrorFormat.setUnderlineColor(QColor(Qt::red));
spellingErrorFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
checkSpelling=spelling;
pChecker = spellChecker;
if (pChecker) spell_encoding=QString(pChecker->get_dic_encoding());
if (!ignoredWords.isEmpty()) alwaysignoredwordList=ignoredWords.split(",");
else alwaysignoredwordList.clear();
ignoredwordList=alwaysignoredwordList;
QFile wordsfile(":/spell/spellignore.txt");
QString line;
if (wordsfile.open(QFile::ReadOnly))
    {
    while (!wordsfile.atEnd()) 
	    {
	    line = wordsfile.readLine();
	    if (!line.isEmpty()) hardignoredwordList.append(line.trimmed());
	    }
    }
}

LatexHighlighter::~LatexHighlighter(){
//delete pChecker;
}

void LatexHighlighter::SetEditor(LatexEditor *ed)
{
editor=ed;
}

void LatexHighlighter::SetAlwaysIgnoredWords(QString ignoredWords)
{
if (!ignoredWords.isEmpty()) alwaysignoredwordList=ignoredWords.split(",");
else alwaysignoredwordList.clear();
ignoredwordList=alwaysignoredwordList;
}


void LatexHighlighter::setColors(QList<QColor> colors)
{
ColorStandard=colors.at(0);
ColorComment=colors.at(1);
ColorMath=colors.at(2);
ColorCommand=colors.at(3);
ColorKeyword=colors.at(4);
ColorVerbatim=colors.at(5);
ColorTodo=colors.at(6);
ColorKeywordGraphic=colors.at(7);
ColorNumberGraphic=colors.at(8);
rehighlight();
}

void LatexHighlighter::highlightBlock(const QString &text)
{
    // structure cleared with empty text!!!
//	if (text.isEmpty())
//		return;

QRegExp rxSweave("<<(.*)>>=");
QStringList types;
types << QLatin1String("article") << QLatin1String("book")
  << QLatin1String("booklet") << QLatin1String("inbook")
  << QLatin1String("incollection") << QLatin1String("inproceedings")
  << QLatin1String("manual") << QLatin1String("mastersthesis")
  << QLatin1String("misc") << QLatin1String("phdthesis")
  << QLatin1String("proceedings") << QLatin1String("techreport")
  << QLatin1String("unpublished") << QLatin1String("periodical")
  << QLatin1String("conference");
QRegExp rxBib("@("+types.join("|")+")\\s*\\{\\s*(.*),", Qt::CaseInsensitive);
rxBib.setMinimal(true);

const int StateStandard = 0;
const int StateComment = 1;
const int StateMath = 2;
const int StateCommand=3;
const int StateVerbatim =4;
const int StateVerbatimCommand =5;
const int StateSweave =6;
const int StateSweaveCommand =7;
const int StateGraphic =8;
const int StateGraphicCommand =9;
const int StateGraphicMath =10;
const int StateBib =11;
const int StateGraphicComment =12;
const int StateGraphicAsy =13;
const int StateGraphicAsyCommand =14;
const int StateGraphicAsyMath =15;
const int StateGraphicAsyComment =16;

int i = 0;
int state = previousBlockState();
int oldstate=state;
QChar last, next ,ch,tmp, verbflag;
verbflag=' ';
QString buffer;


BlockData *blockData = new BlockData;
int leftPos = text.indexOf( '{' );
while ( leftPos != -1 ) 
  {
  ParenthesisInfo *info = new ParenthesisInfo;
  info->character = '{';
  info->position = leftPos;

  blockData->insertPar( info );
  leftPos = text.indexOf( '{', leftPos+1 );
  }

int rightPos = text.indexOf('}');
while ( rightPos != -1 ) 
  {
  ParenthesisInfo *info = new ParenthesisInfo;
  info->character = '}';
  info->position = rightPos;

  blockData->insertPar( info );
  rightPos = text.indexOf( '}', rightPos+1 );
  }
  
leftPos = text.indexOf( REGEXP_BY_CONTROL_CHARS("\\begin{") );
while ( leftPos != -1 ) 
  {
  LatexBlockInfo *info = new LatexBlockInfo;
  info->character = 'b';
  info->position = leftPos;

  blockData->insertLat( info );
  leftPos = text.indexOf(REGEXP_BY_CONTROL_CHARS("\\begin{"), leftPos+1 );
  }

rightPos = text.indexOf(REGEXP_BY_CONTROL_CHARS("\\end{"));
while ( rightPos != -1 ) 
  {
  LatexBlockInfo *info = new LatexBlockInfo;
  info->character = 'e';
  info->position = rightPos;

  blockData->insertLat( info );
  rightPos = text.indexOf(REGEXP_BY_CONTROL_CHARS("\\end{"), rightPos+1 );
  }

setCurrentBlockUserData(blockData);


/////////////////////

/////////////////
QRegExp rxverb(PATTERN_BY_CONTROL_CHARS("verb")+"\\*?([^\\*])");
QRegExp rxlst(PATTERN_BY_CONTROL_CHARS("lstinline")+"(.)");
QTextCharFormat structFormat;
structFormat.setFontWeight(QFont::Bold);
structFormat.setForeground(ColorKeyword);

QTextCharFormat asyFormat;
asyFormat.setFontWeight(QFont::Bold);
asyFormat.setForeground(ColorCommand);

if (!isGraphic)
{
if (state<0) state=0;
i=0;
blockData->code.clear(); 
blockData->misspelled.clear(); 
for (int j=0; j < text.length(); j++) {blockData->code.append(0);blockData->misspelled.append(false);}
while (i < text.length())
	{
        ch = text.at( i );
		/////////////////////////////////////////////////
		//added by S. R. Alavizadeh
		if (QBiDiExtender::bidiEnabled)
			{
            if ( QBiDiExtender::isUnicodeControlCharacters(ch) )
				{
				buffer += ch;
				++i;
				continue;
				}
			}
		/////////////////////////////////////////////////
		
		buffer += ch;
		
		/////////////////////////////////////////////////
		//added by S. R. Alavizadeh
		int k=i;
		if (QBiDiExtender::bidiEnabled)
			{
			if ( i < text.length()-1 )
				{
                while ( k<text.length()-2 && QBiDiExtender::isUnicodeControlCharacters(text.at(k+1)))
					{
					++k;
					}
				next = text.at( k+1 );
				}
			}
		else	/////////////////////////////////////////////////
			if ( i < text.length()-1 ) next = text.at( i+1 );
//tmp = ch;
        switch (state) {
	
	case StateStandard: {
		tmp=text.at( i );
		if (tmp=='\\') {
			if (next=='[')
				{
				setFormat( i, 1,ColorMath );
				blockData->code[i]=1;
				state=StateMath;
				i = k+1;//i++;;
				if ( i < text.length())
					{
					setFormat( i, 1,ColorMath);
					blockData->code[i]=1;
					}
				}
			else
			{
			setFormat( i, 1,ColorCommand );
			blockData->code[i]=1;
			state=StateCommand;
			}
			buffer = QString::null;
		} else
		if (tmp=='$') {
			setFormat( i, 1,ColorMath);
			blockData->code[i]=1;
			state=StateMath;
			if (next=='$')
			{
				i = k+1;//i++;;
				if ( i < text.length())
					{
					setFormat( i, 1,ColorMath);
					blockData->code[i]=1;
					}
				}
			buffer = QString::null;
		} else
		if (tmp== '%' ){
            setFormat( i, 1,ColorComment);
			state=StateComment;
			blockData->code[i]=1;
			buffer = QString::null;
		} else
		if (tmp== '{' ){
			blockData->code[i]=1;
			setFormat( i, 1,ColorStandard);
			state=StateStandard;
		} else
		if (tmp== '}' ){
			blockData->code[i]=1;
			setFormat( i, 1,ColorStandard);
			state=StateStandard;
            if(buffer.indexOf(REGEXP_BY_CONTROL_CHARS("begin{verbatim}")) != -1) {state=StateVerbatim;}
            else if(buffer.indexOf(REGEXP_BY_CONTROL_CHARS("begin{verbatim*}")) != -1) {state=StateVerbatim;}
            else if(buffer.indexOf(REGEXP_BY_CONTROL_CHARS("begin{lstlisting}")) != -1) {state=StateVerbatim;}
            else if(buffer.indexOf(REGEXP_BY_CONTROL_CHARS("begin{gnuplot}")) != -1) {state=StateVerbatim;}
            else if(buffer.indexOf(REGEXP_BY_CONTROL_CHARS("begin{asy}")) != -1) {state=StateGraphicAsy;}
            else if(buffer.indexOf(REGEXP_BY_CONTROL_CHARS("begin{tikzpicture}")) != -1) {state=StateGraphic;}
            else if(buffer.indexOf(REGEXP_BY_CONTROL_CHARS("begin{pspicture}")) != -1) {state=StateGraphic;}
            else if(buffer.indexOf(REGEXP_BY_CONTROL_CHARS("begin{pspicture*}")) != -1) {state=StateGraphic;}
			buffer = QString::null;
		} else
		if (tmp== '<' ){
			blockData->code[i]=1;
			setFormat( i, 1,ColorStandard);
			state=StateStandard;
		} else
		if (tmp== '>' ){
			blockData->code[i]=1;
			setFormat( i, 1,ColorStandard);
			state=StateStandard;
		} else
		if (tmp== ',' ){
			blockData->code[i]=1;
			setFormat( i, 1,ColorStandard);
			state=StateStandard;
            if(CLEAR_CONTROL_CHARS(buffer).indexOf(rxBib) != -1) {state=StateBib;buffer = QString::null;}
		} else
		if (tmp== '=' ){
			blockData->code[i]=1;
			setFormat( i, 1,ColorStandard);
            if(CLEAR_CONTROL_CHARS(buffer).indexOf(rxSweave) != -1) {state=StateSweave;buffer = QString::null;}
		} else
		if (tmp== '(' ){
			blockData->code[i]=1;
			setFormat( i, 1,ColorStandard);
			state=StateStandard;
			buffer = QString::null;
		} else
		if (tmp== ')' ){
			blockData->code[i]=1;
			setFormat( i, 1,ColorStandard);
			state=StateStandard;
			buffer = QString::null;
		} else
		if (isWordSeparator(tmp)){
			blockData->code[i]=1;
			setFormat( i, 1,ColorStandard);
			//buffer = QString::null;
		} else
		 {
			setFormat( i, 1,ColorStandard);
			state=StateStandard;
			//buffer = QString::null;
		}
//	buffer = QString::null;
	} break;
	
	case StateComment: {
	setFormat( i, 1,ColorComment);
	blockData->code[i]=1;
	state=StateComment;
	buffer = QString::null;
	} break;

	case StateMath: {
		tmp=text.at( i );
		if (tmp== '$') {
			setFormat( i, 1,ColorMath);
			blockData->code[i]=1;
			state=StateStandard;
			if (next=='$')
				{
				i = k+1;//i++;;
				if ( i < text.length())
					{
					setFormat( i, 1,ColorMath);
					blockData->code[i]=1;
					}
				}
		} else if (tmp== '\\') {
			if (next==']')
				{
				setFormat( i, 1,ColorMath);
				blockData->code[i]=1;
				state=StateStandard;
				i = k+1;//i++;;
				if ( i < text.length())
					{
					setFormat( i, 1,ColorMath);
					blockData->code[i]=1;
					}
				}
			else if (next=='$')
				{
				setFormat( i, 1,ColorMath);
				blockData->code[i]=1;
				state=StateMath;
				i = k+1;//i++;;
				if ( i < text.length())
					{
					setFormat( i, 1,ColorMath/*QColor("#838307")*/);
					blockData->code[i]=1;
					}
				}
			else
				{
				setFormat( i, 1,ColorMath);
				blockData->code[i]=1;
				state=StateMath;
				}
		} else
		if (tmp== '{' ){
			setFormat( i, 1,ColorMath);
			blockData->code[i]=1;
			state=StateMath;
		} else
		if (tmp== '}' ){
			setFormat( i, 1,ColorMath);
			blockData->code[i]=1;
			state=StateMath;
		} else
		 {
			setFormat( i, 1,ColorMath);
			blockData->code[i]=1;
			state=StateMath;
		}
	buffer = QString::null;
	} break;
	case StateGraphicMath: {
		tmp=text.at( i );
		if (tmp== '$') {
			setFormat( i, 1,ColorMath);
			blockData->code[i]=1;
			state=StateGraphic;
			if (next=='$')
				{
				i = k+1;//i++;;
				if ( i < text.length())
					{
					setFormat( i, 1,ColorMath);
					blockData->code[i]=1;
					}
				}
		} else if (tmp== '\\') {
			if (next==']')
				{
				setFormat( i, 1,ColorMath);
				blockData->code[i]=1;
				state=StateGraphic;
				i = k+1;//i++;;
				if ( i < text.length())
					{
					setFormat( i, 1,ColorMath);
					blockData->code[i]=1;
					}
				}
			else
				{
				setFormat( i, 1,ColorMath);
				blockData->code[i]=1;
				state=StateGraphicMath;
				}
		} else
		if (tmp== '{' ){
			setFormat( i, 1,ColorMath);
			blockData->code[i]=1;
			state=StateGraphicMath;
		} else
		if (tmp== '}' ){
			setFormat( i, 1,ColorMath);
			blockData->code[i]=1;
			state=StateGraphicMath;
		} else
		 {
			setFormat( i, 1,ColorMath);
			blockData->code[i]=1;
			state=StateGraphicMath;
		}
	buffer = QString::null;
	} break;
	case StateGraphicAsyMath: {
		tmp=text.at( i );
		if (tmp== '$') {
			setFormat( i, 1,ColorMath);
			blockData->code[i]=1;
			state=StateGraphicAsy;
			if (next=='$')
				{
				i = k+1;//i++;;
				if ( i < text.length())
					{
					setFormat( i, 1,ColorMath);
					blockData->code[i]=1;
					}
				}
		} else if (tmp== '\\') {
			if (next==']')
				{
				setFormat( i, 1,ColorMath);
				blockData->code[i]=1;
				state=StateGraphicAsy;
				i = k+1;//i++;;
				if ( i < text.length())
					{
					setFormat( i, 1,ColorMath);
					blockData->code[i]=1;
					}
				}
			else
				{
				setFormat( i, 1,ColorMath);
				blockData->code[i]=1;
				state=StateGraphicAsyMath;
				}
		} else
		if (tmp== '{' ){
			setFormat( i, 1,ColorMath);
			blockData->code[i]=1;
			state=StateGraphicAsyMath;
		} else
		if (tmp== '}' ){
			setFormat( i, 1,ColorMath);
			blockData->code[i]=1;
			state=StateGraphicAsyMath;
		} else
		 {
			setFormat( i, 1,ColorMath);
			blockData->code[i]=1;
			state=StateGraphicAsyMath;
		}
	buffer = QString::null;
	} break;
	case StateCommand:{
		tmp=text.at( i );
        if (rxverb.exactMatch(buffer))
				{
                 verbflag=rxverb.cap(1).at(0);
				 blockData->code[i]=1;
				 setFormat( i, 1,ColorCommand);
				 state=StateVerbatim;
		} else
		if (rxlst.exactMatch(buffer))
				{
				 verbflag=rxlst.cap(1).at(0);
				 blockData->code[i]=1;
				 setFormat( i, 1,ColorCommand);
				 state=StateVerbatim;
		} else
		if (tmp=='$') {
			if (last=='\\')
				{
				setFormat( i, 1,ColorCommand);
				blockData->code[i]=1;
				state=StateStandard;
				}
			else
				{
				setFormat( i, 1,ColorMath);
				blockData->code[i]=1;
				state=StateMath;
				}
		} else
		if (tmp=='%') {
			if (last=='\\')
				{
				setFormat( i, 1,ColorStandard);
				state=StateStandard;
				}
			else
				{
				setFormat( i, 1,ColorComment);
				blockData->code[i]=1;
				state=StateComment;
         			}
		} else
		if (tmp== ' ') {
         		setFormat( i, 1,ColorStandard);
         		state=StateStandard;
		} else
		if (tmp== '\t') {
         		setFormat( i, 1,ColorStandard);
         		state=StateStandard;
		}  else
		if (tmp=='(' || tmp=='[' || tmp=='{' || tmp==')' || tmp==']' || tmp=='}') {
			blockData->code[i]=1;
			setFormat( i, 1,ColorStandard);
			state=StateStandard;
            if ( buffer.length() > 0 ) {
				/////////////////////////////////////////////////
				//added by S. R. Alavizadeh
				//Bi-Di Support
				QString tmpBuffer = buffer;
				//This just skips Unicode Control Characters.
				if (QBiDiExtender::bidiEnabled)
					tmpBuffer = QBiDiExtender::removeUnicodeControlCharacters(buffer);
				/////////////////////////////////////////////////
                for ( QStringList::Iterator it = KeyWords.begin(); it != KeyWords.end(); ++it ) {
                    if (( *it ).indexOf( tmpBuffer )!=-1)
                    {
                        QString keyword(*it);
                        keyword.remove(keyword.size()-1, 1);
                        keyword.prepend("\\");
                        QRegExp kewordsRegExp(REGEXP_BY_CONTROL_CHARS(keyword));
                        int index = text.lastIndexOf(kewordsRegExp, i);
                        if (index != -1) {
                            if (*it!="begin{" && *it!="end{") {
                                setFormat(index, kewordsRegExp.cap(0).length(),structFormat);
                            }
                            else {
                                setFormat(index, kewordsRegExp.cap(0).length(),ColorKeyword);
                            }
                            blockData->code[i]=1;

                            break;
                        }
                    }
                }
            }
		} else
		if (tmp=='\\' || tmp==',' || tmp==';' /*|| tmp=='\''*/ || tmp=='\"' || tmp=='`' || tmp=='^' || tmp=='~') {
			blockData->code[i]=1;
			if (last=='\\')
			{
				setFormat( i, 1,ColorCommand);
				blockData->code[i]=1;
				state=StateStandard;
			}
			else
			{
				setFormat( i, 1,ColorCommand);
				blockData->code[i]=1;
				state=StateCommand;
			}
		} else
	     		{
         		setFormat( i, 1,ColorCommand);
			blockData->code[i]=1;
         		state=StateCommand;
		}
	} break;
       case StateVerbatim: {
               tmp=text.at( i );
		if (tmp==verbflag && verbflag!=' ')
		{
		setFormat( i, 1,ColorCommand);
		blockData->code[i]=1;
		state=StateStandard;  
		verbflag=' ';
		} else
		if (tmp=='\\') {
			if (next=='[')
				{
				setFormat( i, 1,ColorVerbatim );
				blockData->code[i]=1;
				i = k+1;//i++;;
				if ( i < text.length())
					{
					setFormat( i, 1,ColorVerbatim);
					blockData->code[i]=1;
					}
				}
			else
			{
			setFormat( i, 1,ColorVerbatim );
			blockData->code[i]=1;
			state=StateVerbatimCommand;
			}
		} else
		if (tmp=='$') {
			setFormat( i, 1,ColorVerbatim);
			blockData->code[i]=1;
			if (next=='$')
				{
				i = k+1;//i++;;
				if ( i < text.length())
					{
					setFormat( i, 1,ColorVerbatim);
					blockData->code[i]=1;
					}
				}
			buffer = QString::null;
		} else
		if (tmp== '%' ){
			setFormat( i, 1,ColorVerbatim);
			blockData->code[i]=1;
			buffer = QString::null;
		} else
		if (tmp== '{' ){
			blockData->code[i]=1;
			setFormat( i, 1,ColorVerbatim);
		} else
		if (tmp== '}' ){
			blockData->code[i]=1;
			setFormat( i, 1,ColorVerbatim);
			state=StateVerbatim;
            QStringList envs = QStringList() << "verbatim" << "verbatim\*"
                                             << "lstlisting" << "gnuplot";
            foreach (const QString &env, envs) {
                QRegExp eVerb("("+PATTERN_BY_CONTROL_CHARS("\\end")+")("+
                              PATTERN_BY_CONTROL_CHARS("{"+env+"}")+")");
                int pos=buffer.indexOf(eVerb);
                if( pos!= -1)
                {
                    state=StateStandard;
                    setFormat(pos,eVerb.cap(1).size(),ColorKeyword);
                    setFormat(pos+eVerb.cap(1).size(),eVerb.cap(2).size(),ColorStandard);
                }
            }
			buffer = QString::null;
		} else
		if (tmp== '(' ){
			blockData->code[i]=1;
			setFormat( i, 1,ColorVerbatim);
			buffer = QString::null;
		} else
		if (tmp== ')' ){
			blockData->code[i]=1;
			setFormat( i, 1,ColorVerbatim);
			buffer = QString::null;
		} else
		if (isWordSeparator(tmp)){
			blockData->code[i]=1;
			setFormat( i, 1,ColorVerbatim);
			buffer = QString::null;
		} else
		 {
			setFormat( i, 1,ColorVerbatim);
			//buffer = QString::null;
		}
       } break;
	case StateVerbatimCommand:{
		tmp=text.at( i );
		if (tmp==verbflag && verbflag!=' ')
		{
		setFormat( i, 1,ColorCommand);
		blockData->code[i]=1;
		state=StateStandard;  
		verbflag=' ';
		} else
		if (tmp=='$') {
			if (last=='\\')
				{
				setFormat( i, 1,ColorVerbatim);
				blockData->code[i]=1;
				state=StateVerbatim;
				}
			else
				{
				setFormat( i, 1,ColorVerbatim);
				blockData->code[i]=1;
				state=StateVerbatim;
				}
		} else
		if (tmp=='%') {
			if (last=='\\')
				{
				setFormat( i, 1,ColorVerbatim);
				state=StateVerbatim;
				}
			else
				{
				setFormat( i, 1,ColorVerbatim);
				blockData->code[i]=1;
				state=StateVerbatim;
         			}
		} else
		if (tmp== ' ') {
         		setFormat( i, 1,ColorVerbatim);
         		state=StateVerbatim;
		}  else
		if (tmp=='(' || tmp=='[' || tmp=='{' || tmp==')' || tmp==']' || tmp=='}') {
			blockData->code[i]=1;
			setFormat( i, 1,ColorVerbatim);
			state=StateVerbatim;
		} else
		if (tmp=='\\' || tmp==',' || tmp==';' || /*tmp=='\'' ||*/ tmp=='\"' || tmp=='`' || tmp=='^' || tmp=='~') {
			blockData->code[i]=1;
			if (last=='\\')
			{
				setFormat( i, 1,ColorVerbatim);
				blockData->code[i]=1;
				state=StateVerbatim;
			}
			else
			{
				setFormat( i, 1,ColorVerbatim);
				blockData->code[i]=1;
				state=StateVerbatim;
			}
		} else
	     		{
         		setFormat( i, 1,ColorVerbatim);
			blockData->code[i]=1;
         		state=StateVerbatimCommand;
		}
	} break;
       case StateGraphic: {
               tmp=text.at( i );
 		if (tmp=='\\') {
			if (next=='[')
				{
				setFormat( i, 1,ColorVerbatim );
				blockData->code[i]=1;
				i = k+1;//i++;;
				if ( i < text.length())
					{
					setFormat( i, 1,ColorVerbatim);
					blockData->code[i]=1;
					}
				}
			else
			{
 			setFormat( i, 1,ColorVerbatim );
			blockData->code[i]=1;
			state=StateGraphicCommand;
			}
 		} else
		if (tmp=='$') {
			setFormat( i, 1,ColorMath);
			blockData->code[i]=1;
			state=StateGraphicMath;
			if (next=='$')
				{
				i = k+1;//i++;;
				if ( i < text.length())
					{
					setFormat( i, 1,ColorMath);
					blockData->code[i]=1;
					}
				}
			buffer = QString::null;
		} else
		if (tmp== '/' && next=='/'){
			setFormat( i, 1,ColorComment);
			state=StateGraphicComment;
			blockData->code[i]=1;
			buffer = QString::null;
		} else
		if (tmp== '%'){
			setFormat( i, 1,ColorComment);
			state=StateGraphicComment;
			//setFormat( i, 1,ColorVerbatim);
			blockData->code[i]=1;
			buffer = QString::null;
		} else
		if (tmp== '{' ){
			blockData->code[i]=1;
			setFormat( i, 1,ColorVerbatim);
		} else
		if (tmp== '}' ){
		  	blockData->code[i]=1;
			setFormat( i, 1,ColorVerbatim);
			state=StateGraphic;
            QStringList envs = QStringList() << "asy" << "tikzpicture"
                                             << "pspicture" << "pspicture*";
            foreach (const QString &env, envs) {
                QRegExp eVerb("("+PATTERN_BY_CONTROL_CHARS("\\end")+")("+
                              PATTERN_BY_CONTROL_CHARS("{"+env+"}")+")");
                int pos=buffer.indexOf(eVerb);
                if( pos!= -1)
                {
                    state=StateStandard;
                    setFormat(pos,eVerb.cap(1).size(),ColorKeyword);
                    setFormat(pos+eVerb.cap(1).size(),eVerb.cap(2).size(),ColorStandard);
                }
            }
			buffer = QString::null;
		} else
		if (tmp== '(' ){
			blockData->code[i]=1;
			setFormat( i, 1,ColorVerbatim);
			buffer = QString::null;
		} else
		if (tmp== ')' ){
			blockData->code[i]=1;
			setFormat( i, 1,ColorVerbatim);
			buffer = QString::null;
		} else
		if (isWordSeparator(tmp) && tmp!='"' && tmp!='.' && tmp!='-'){
			blockData->code[i]=1;
			setFormat( i, 1,ColorVerbatim);
			buffer = QString::null;
		} else
		 {
			setFormat( i, 1,ColorVerbatim);
			//buffer = QString::null;
		}
       } break;
       case StateGraphicAsy: {
               tmp=text.at( i );
 		if (tmp=='\\') {
			if (next=='[')
				{
				setFormat( i, 1,ColorVerbatim );
				blockData->code[i]=1;
				i = k+1;//i++;;
				if ( i < text.length())
					{
					setFormat( i, 1,ColorVerbatim);
					blockData->code[i]=1;
					}
				}
			else
			{
 			setFormat( i, 1,ColorVerbatim );
			blockData->code[i]=1;
			state=StateGraphicAsyCommand;
			}
 		} else
		if (tmp=='$') {
			setFormat( i, 1,ColorMath);
			blockData->code[i]=1;
			state=StateGraphicAsyMath;
			if (next=='$')
				{
				i = k+1;//i++;;
				if ( i < text.length())
					{
					setFormat( i, 1,ColorMath);
					blockData->code[i]=1;
					}
				}
			buffer = QString::null;
		} else
		if (tmp== '/' && next=='/'){
			setFormat( i, 1,ColorComment);
			state=StateGraphicAsyComment;
			blockData->code[i]=1;
			buffer = QString::null;
		} else
		if (tmp== '{' ){
			blockData->code[i]=1;
			setFormat( i, 1,ColorVerbatim);
		} else
		if (tmp== '}' ){
		  	blockData->code[i]=1;
			setFormat( i, 1,ColorVerbatim);
			state=StateGraphicAsy;
            QStringList envs = QStringList() << "asy" << "tikzpicture"
                                             << "pspicture" << "pspicture*";
            foreach (const QString &env, envs) {
                QRegExp eVerb("("+PATTERN_BY_CONTROL_CHARS("\\end")+")("+
                              PATTERN_BY_CONTROL_CHARS("{"+env+"}")+")");
                int pos=buffer.indexOf(eVerb);
                if( pos!= -1)
                {
                    state=StateStandard;
                    setFormat(pos,eVerb.cap(1).size(),ColorKeyword);
                    setFormat(pos+eVerb.cap(1).size(),eVerb.cap(2).size(),ColorStandard);
                }
            }
			buffer = QString::null;
		} else
		if (tmp== '(' ){
			blockData->code[i]=1;
			setFormat( i, 1,ColorVerbatim);
			buffer = QString::null;
		} else
		if (tmp== ')' ){
			blockData->code[i]=1;
			setFormat( i, 1,ColorVerbatim);
			buffer = QString::null;
		} else
		if (isWordSeparator(tmp) && tmp!='"' && tmp!='.' && tmp!='-'){
			blockData->code[i]=1;
			setFormat( i, 1,ColorVerbatim);
			buffer = QString::null;
		} else
		 {
			setFormat( i, 1,ColorVerbatim);
			//buffer = QString::null;
		}
       } break;       
	case StateGraphicComment: {
	setFormat( i, 1,ColorComment);
	blockData->code[i]=1;
	state=StateGraphicComment;
	buffer = QString::null;
	} break;
	case StateGraphicAsyComment: {
	setFormat( i, 1,ColorComment);
	blockData->code[i]=1;
	state=StateGraphicAsyComment;
	buffer = QString::null;
	} break;
	case StateGraphicCommand:{
		tmp=text.at( i );
		if (tmp=='$') {
			if (last=='\\')
				{
				setFormat( i, 1,ColorVerbatim);
				blockData->code[i]=1;
				state=StateGraphic;
				}
			else
				{
				setFormat( i, 1,ColorVerbatim);
				blockData->code[i]=1;
				state=StateGraphic;
				}
		} else
		if (tmp=='%') {
			if (last=='\\')
				{
				setFormat( i, 1,ColorVerbatim);
				state=StateGraphic;
				}
			else
				{
				setFormat( i, 1,ColorVerbatim);
				blockData->code[i]=1;
				state=StateGraphic;
         			}
		} else
		if (tmp== ' ') {
         		setFormat( i, 1,ColorVerbatim);
         		state=StateGraphic;
		}  else
		if (tmp=='(' || tmp=='[' || tmp=='{' || tmp==')' || tmp==']' || tmp=='}') {
			blockData->code[i]=1;
			setFormat( i, 1,ColorVerbatim);
			state=StateGraphic;
		} else
		if (tmp=='\\' || tmp==',' || tmp==';' || /*tmp=='\'' ||*/ tmp=='\"' || tmp=='`' || tmp=='^' || tmp=='~') {
			blockData->code[i]=1;
			if (last=='\\')
			{
				setFormat( i, 1,ColorVerbatim);
				blockData->code[i]=1;
				state=StateGraphic;
			}
			else
			{
				setFormat( i, 1,ColorVerbatim);
				blockData->code[i]=1;
				state=StateGraphic;
			}
		} else
	     		{
         		setFormat( i, 1,ColorVerbatim);
			//blockData->code[i]=1;
         		state=StateGraphicCommand;
		}
	} break;
	case StateGraphicAsyCommand:{
		tmp=text.at( i );
		if (tmp=='$') {
			if (last=='\\')
				{
				setFormat( i, 1,ColorVerbatim);
				blockData->code[i]=1;
				state=StateGraphicAsy;
				}
			else
				{
				setFormat( i, 1,ColorVerbatim);
				blockData->code[i]=1;
				state=StateGraphicAsy;
				}
		} else
		if (tmp=='%') {
			if (last=='\\')
				{
				setFormat( i, 1,ColorVerbatim);
				state=StateGraphicAsy;
				}
			else
				{
				setFormat( i, 1,ColorVerbatim);
				blockData->code[i]=1;
				state=StateGraphicAsy;
         			}
		} else
		if (tmp== ' ') {
         		setFormat( i, 1,ColorVerbatim);
         		state=StateGraphicAsy;
		}  else
		if (tmp=='(' || tmp=='[' || tmp=='{' || tmp==')' || tmp==']' || tmp=='}') {
			blockData->code[i]=1;
			setFormat( i, 1,ColorVerbatim);
			state=StateGraphicAsy;
		} else
		if (tmp=='\\' || tmp==',' || tmp==';' || /*tmp=='\'' ||*/ tmp=='\"' || tmp=='`' || tmp=='^' || tmp=='~') {
			blockData->code[i]=1;
			if (last=='\\')
			{
				setFormat( i, 1,ColorVerbatim);
				blockData->code[i]=1;
				state=StateGraphicAsy;
			}
			else
			{
				setFormat( i, 1,ColorVerbatim);
				blockData->code[i]=1;
				state=StateGraphicAsy;
			}
		} else
	     		{
         		setFormat( i, 1,ColorVerbatim);
			//blockData->code[i]=1;
         		state=StateGraphicAsyCommand;
		}
	} break;
/****************************/
       case StateBib: {
//	 qDebug() << "bib" << buffer << next;
               tmp=text.at( i );
/*		if (tmp== '\"' ){
			blockData->code[i]=1;
			setFormat( i, 1,ColorVerbatim);
			state=StateBibCommand;
		} else*/
		if (tmp== '}' ){
			blockData->code[i]=1;
			setFormat( i, 1,ColorVerbatim);
			if (next=='\n' || i == text.length()-1)
			  {
			  state=StateStandard;
			  buffer = QString::null;
			  }
		} else
		 {
			setFormat( i, 1,ColorVerbatim);
			state=StateBib;
			//buffer = QString::null;
		}
       } break;
/*       case StateBibCommand: {
	 	 qDebug() << "bibcommand" << buffer;
               tmp=text.at( i );
		if (tmp== '\"' ){
			blockData->code[i]=1;
			setFormat( i, 1,ColorVerbatim);
			state=StateBib;
		} else
		if (tmp== '}' ){
			blockData->code[i]=1;
			setFormat( i, 1,ColorVerbatim);
			if (next!=',' || i == text.length()-1)
			  {
			  state=StateBib;
			  }
			else 
			{
			 state=StateBibCommand; 
			}
		} else
		 {
			setFormat( i, 1,ColorVerbatim);
			state=StateBibCommand;
		}
       } break;*/
/***************************/
       case StateSweave: {
               tmp=text.at( i );
		if (tmp=='\\') {
			if (next=='[')
				{
				setFormat( i, 1,ColorVerbatim );
				blockData->code[i]=1;
				i = k+1;//i++;;
				if ( i < text.length())
					{
					setFormat( i, 1,ColorVerbatim);
					blockData->code[i]=1;
					}
				}
			else
			{
			setFormat( i, 1,ColorVerbatim );
			blockData->code[i]=1;
			state=StateSweaveCommand;
			}
		} else
		if (tmp=='$') {
			setFormat( i, 1,ColorVerbatim);
			blockData->code[i]=1;
			if (next=='$')
				{
				i = k+1;//i++;;
				if ( i < text.length())
					{
					setFormat( i, 1,ColorVerbatim);
					blockData->code[i]=1;
					}
				}
			buffer = QString::null;
		} else
		if (tmp== '%' ){
			setFormat( i, 1,ColorVerbatim);
			blockData->code[i]=1;
			buffer = QString::null;
		} else
		if (tmp== '{' ){
			blockData->code[i]=1;
			setFormat( i, 1,ColorVerbatim);
		} else
		if (tmp== '}' ){
			blockData->code[i]=1;
			setFormat( i, 1,ColorVerbatim);
			buffer = QString::null;
		} else
		if (tmp== '@' ){
			blockData->code[i]=1;
			setFormat( i, 1,ColorVerbatim);
			state=StateStandard;
			buffer = QString::null;
		} else
		if (tmp== '(' ){
			blockData->code[i]=1;
			setFormat( i, 1,ColorVerbatim);
			buffer = QString::null;
		} else
		if (tmp== ')' ){
			blockData->code[i]=1;
			setFormat( i, 1,ColorVerbatim);
			buffer = QString::null;
		} else
		if (isWordSeparator(tmp)){
			blockData->code[i]=1;
			setFormat( i, 1,ColorVerbatim);
			buffer = QString::null;
		} else
		 {
			setFormat( i, 1,ColorVerbatim);
			//buffer = QString::null;
		}
       } break;
	case StateSweaveCommand:{
		tmp=text.at( i );
		if (tmp=='$') {
			if (last=='\\')
				{
				setFormat( i, 1,ColorVerbatim);
				blockData->code[i]=1;
				state=StateSweave;
				}
			else
				{
				setFormat( i, 1,ColorVerbatim);
				blockData->code[i]=1;
				state=StateSweave;
				}
		} else
		if (tmp=='%') {
			if (last=='\\')
				{
				setFormat( i, 1,ColorVerbatim);
				state=StateSweave;
				}
			else
				{
				setFormat( i, 1,ColorVerbatim);
				blockData->code[i]=1;
				state=StateSweave;
         			}
		} else
		if (tmp== ' ') {
         		setFormat( i, 1,ColorVerbatim);
         		state=StateSweave;
		}  else
		if (tmp=='(' || tmp=='[' || tmp=='{' || tmp==')' || tmp==']' || tmp=='}') {
			blockData->code[i]=1;
			setFormat( i, 1,ColorVerbatim);
			state=StateSweave;
		} else
		if (tmp=='\\' || tmp==',' || tmp==';' /*|| tmp=='\''*/ || tmp=='\"' || tmp=='`' || tmp=='^' || tmp=='~') {
			blockData->code[i]=1;
			if (last=='\\')
			{
				setFormat( i, 1,ColorVerbatim);
				blockData->code[i]=1;
				state=StateSweave;
			}
			else
			{
				setFormat( i, 1,ColorVerbatim);
				blockData->code[i]=1;
				state=StateSweave;
			}
		} else
	     		{
         		setFormat( i, 1,ColorVerbatim);
			blockData->code[i]=1;
         		state=StateSweaveCommand;
		}
	} break;
	}
	last = ch;
    i = k+1;//i++;
	}
if ( state == StateComment ) 
 	{
	setCurrentBlockState(StateStandard);
    	} 
else if ( state == StateMath ) 
	{
	setCurrentBlockState(StateMath) ;
    	}
else if ( state == StateGraphicMath ) 
	{
	setCurrentBlockState(StateGraphicMath) ;
    	}
else if ( state == StateVerbatim ) 
       {
       setCurrentBlockState(StateVerbatim) ;
       }
else if ( state == StateVerbatimCommand ) 
       {
       setCurrentBlockState(StateVerbatimCommand) ;
       }
else if ( state == StateGraphicComment ) 
       {
       setCurrentBlockState(StateGraphic) ;
       }
else if ( state == StateGraphic ) 
       {
       setCurrentBlockState(StateGraphic) ;
       }
else if ( state == StateGraphicCommand ) 
       {
       setCurrentBlockState(StateGraphicCommand) ;
       }
else if ( state == StateGraphicAsyComment ) 
       {
       setCurrentBlockState(StateGraphicAsy) ;
       }
else if ( state == StateGraphicAsy ) 
       {
       setCurrentBlockState(StateGraphicAsy) ;
       }
else if ( state == StateGraphicAsyCommand ) 
       {
       setCurrentBlockState(StateGraphicAsyCommand) ;
       }
else if ( state == StateSweave ) 
       {
       setCurrentBlockState(StateSweave) ;
       }
else if ( state == StateSweaveCommand ) 
       {
       setCurrentBlockState(StateSweaveCommand) ;
       }
else if ( state == StateBib ) 
       {
       setCurrentBlockState(StateBib) ;
       }
/*else if ( state == StateBibCommand ) 
       {
       setCurrentBlockState(StateBibCommand) ;
       } */
else 
	{
	setCurrentBlockState(StateStandard) ;
	state=StateStandard;
    	}
/////////////////////

//if ((oldstate == StateStandard) && (state == StateStandard))
//{
i=currentBlock().blockNumber();
editor->checkStructUpdate(document(),currentBlock().position(),text,i);
//}
////////////////////

if (text.isEmpty()) return;
QBrush brushstandard(ColorStandard);
QBrush brushverbatim(ColorVerbatim);
QBrush brushmath(ColorMath);
QBrush brushcomment(ColorComment);
QTextCharFormat todoFormat;
todoFormat.setFontWeight(QFont::Bold);
todoFormat.setForeground(ColorTodo);
todoFormat.setBackground(Qt::yellow);
if (state == StateComment)
{
    int pos = 0;
    while (pos < text.length() && pos != -1) {
        if (CLEAR_CONTROL_CHARS(text).indexOf("%TODO", pos) != -1) {
            QRegExp todoRegExp(REGEXP_BY_CONTROL_CHARS("%TODO"));
            int pos1 = text.indexOf(todoRegExp, pos);
            pos = text.indexOf("%", pos1);

            if (format(pos).foreground() == brushcomment) {
                setFormat(pos1, todoRegExp.cap(0).size(), todoFormat);
                pos=pos1+todoRegExp.cap(0).size();
            }
            else {
                ++pos;
            }
        }
        else {
            break;
        }
    }

//	i=0;
//	while (i < text.length())
//		{
//		buffer = QString::null;
//		ch = text.at( i );
//		while (!isSpace(ch) || ch=='%')
//		      {
//		      buffer += ch;
//		      ++i;
//		      if (i < text.length()) ch = text.at( i );
//		      else break;
//		      }
//        if ((CLEAR_CONTROL_CHARS(buffer).length() > 0) /*&& (format(i - buffer.length()).foreground()==brushcomment)*/)
//		    {
//            if ((format(i - buffer.length()).foreground()==brushcomment)) {
//                QMessageBox::information(0,"ttttttttt",QString("foreground()==brushcomment"));
//            }
//            QMessageBox::information(0,"Aiiiiiiiii",QString("i=%1\nbuffer.length()=%2").arg(i).arg(buffer.length()));
//            /////////////////////////////////////////////////
//            //added by S. R. Alavizadeh
//            //Bi-Di Support
//            QString tmpBuffer = buffer;
//            //This just skips Unicode Control Characters.
//            if (QBiDiExtender::bidiEnabled)
//                tmpBuffer = QBiDiExtender::removeUnicodeControlCharacters(buffer);
//            /////////////////////////////////////////////////

//            if (tmpBuffer=="%TODO")
//                {
//                setFormat( i - buffer.length()+1, buffer.length()-1,todoFormat);
//                }
//            // QMessageBox::information(0,"ttttttttt",QString("buffer=%1\ntmpBuffer=%2").arg(buffer).arg(tmpBuffer));
//		    }
//		++i;
//		}
}

if (state == StateGraphic || state == StateGraphicCommand || state == StateGraphicComment || state == StateGraphicAsy || state == StateGraphicAsyCommand || state == StateGraphicAsyComment)
{
	QRegExp number("[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?");
    int index = number.indexIn(CLEAR_CONTROL_CHARS(text));
        while (index >= 0) {
            int length = number.matchedLength();
            if (blockData->code[index]!=1) setFormat(index, length, ColorNumberGraphic);
            index = number.indexIn(CLEAR_CONTROL_CHARS(text), index + length);
        }
	if (state == StateGraphicAsy || state == StateGraphicAsyCommand || state == StateGraphicAsyComment)
	{
        QRegExp expression("\\b[A-Za-z0-9_"+UNICODE_CONTROL_CHARS.join("")+"]+(?=\\()");
        index = expression.indexIn(text);
		while (index >= 0) {
		    int length = expression.matchedLength();
		    if (blockData->code[index]!=1) setFormat(index, length, ColorKeyword);
            index = expression.indexIn(text, index + length);
		}
	i=0;
	while (i < text.length())
		{
		buffer = QString::null;
		ch = text.at( i );
		while ((blockData->code[i]!=1 || ch=='\\') && (!isSpace(ch)))
		      {
		      buffer += ch;
		      ++i;
		      if (i < text.length()) ch = text.at( i );
		      else break;
		      }
		if ((buffer.length() > 0) && (format(i - buffer.length()).foreground()==brushverbatim)) 
		    {
			/////////////////////////////////////////////////
			//added by S. R. Alavizadeh
			//Bi-Di Support
			QString tmpBuffer = buffer;
			//This just skips Unicode Control Characters.
			if (QBiDiExtender::bidiEnabled)
				tmpBuffer = QBiDiExtender::removeUnicodeControlCharacters(buffer);
			/////////////////////////////////////////////////
		    for ( QStringList::Iterator it = KeyWordsGraphic.begin(); it != KeyWordsGraphic.end(); ++it ) 
					{
        				if (*it==tmpBuffer ) 
						{
                        setFormat( i - buffer.length(), buffer.length(),ColorKeywordGraphic);
                        break;
                        }
					}
		    for ( QStringList::Iterator it = KeyWordsGraphicBis.begin(); it != KeyWordsGraphicBis.end(); ++it ) 
					{
        				if (*it==tmpBuffer ) 
						{
                        setFormat( i - buffer.length(), buffer.length(),asyFormat);
                        break;
                        }
					}
		    }

		++i;
		}
	}
    else if ((CLEAR_CONTROL_CHARS(text).indexOf("pspicture") == -1) && (CLEAR_CONTROL_CHARS(text).indexOf("tikzpicture") == -1))
	{
        QRegExp expression("\\\\[A-Za-z"+UNICODE_CONTROL_CHARS.join("")+"]+");
        index = expression.indexIn(text);
		while (index >= 0) {
		    int length = expression.matchedLength();
		    if ((format(index).foreground()!=brushmath) && (format(index).foreground()!=brushcomment)) setFormat(index, length, ColorKeywordGraphic);
            index = expression.indexIn(text, index + length);
        }
	}

}

if (pChecker && state!=StateGraphic && state!=StateGraphicCommand && state!=StateGraphicComment && state!=StateGraphicAsy && state!=StateGraphicAsyCommand && state!=StateGraphicAsyComment)
	{
	i=0;
	int check;
	int offset ;
	QTextCodec *codec = QTextCodec::codecForName(spell_encoding.toLatin1());
	QByteArray encodedString;

	while (i < text.length())
		{
		buffer = QString::null;
		ch = text.at( i );
		offset=0;
		while ((blockData->code[i]!=1) && (!isSpace(ch)))
		      {
		      buffer += ch;
		      ++i;
		      if (i < text.length()) ch = text.at( i );
		      else break;
		      }
		while (buffer.startsWith('\''))
		  {
		  buffer=buffer.right(buffer.size()-1);
		  }
		while (buffer.endsWith('\''))
		  {
		  buffer.chop(1);
		  offset++;
		  }

        /////////////////////////////////////////////////
        //added by S. R. Alavizadeh
        //Bi-Di Support
        QString tmpBuffer = buffer;
        //This just skips Unicode Control Characters.
        if (QBiDiExtender::bidiEnabled)
            tmpBuffer = QBiDiExtender::removeUnicodeControlCharacters(buffer);
        /////////////////////////////////////////////////
        if ( (tmpBuffer.length() > 1) && (!ignoredwordList.contains(tmpBuffer)) && (!hardignoredwordList.contains(tmpBuffer)))
		      {
              if (format(i - buffer.length()-offset).foreground()==brushverbatim) spellingErrorFormat.setForeground(brushverbatim);
		      else spellingErrorFormat.setForeground(brushstandard);
		      encodedString = codec->fromUnicode(tmpBuffer);
		      check = pChecker->spell(encodedString.data());
		      if (!check) 
			{
            if (checkSpelling) setFormat(i - buffer.length()-offset, buffer.length(), spellingErrorFormat);
            for (int k=i - buffer.length()-offset;  k< i-offset; k++) blockData->misspelled[k]=true;
			}
		      }
		++i;
		}
	}
	
}
//****************************************************************
else
{
if (state<0) state=8;
i=0;
blockData->code.clear(); 
blockData->misspelled.clear(); 
for (int j=0; j < text.length(); j++) {blockData->code.append(0);blockData->misspelled.append(false);}
while (i < text.length())
	{
        ch = text.at( i );

	/////////////////////////////////////////////////
	//added by S. R. Alavizadeh
	if (QBiDiExtender::bidiEnabled)
		{
        if ( QBiDiExtender::isUnicodeControlCharacters(ch) )
			{
			buffer += ch;
			++i;
			continue;
			}
		}
	/////////////////////////////////////////////////
	
	buffer += ch;
	
	/////////////////////////////////////////////////
	//added by S. R. Alavizadeh
	int k=i;
	if (QBiDiExtender::bidiEnabled)
		{
		if ( i < text.length()-1 )
			{
            while ( k<text.length()-2 && QBiDiExtender::isUnicodeControlCharacters(text.at(k+1)) )
				{
				++k;
				}
			next = text.at( k+1 );
			}
		}
	else	/////////////////////////////////////////////////
		if ( i < text.length()-1 ) next = text.at( i+1 );

        switch (state) {
	

	case StateGraphicMath: {
		tmp=text.at( i );
		if (tmp== '$') {
			setFormat( i, 1,ColorMath);
			blockData->code[i]=1;
			state=StateGraphic;
			if (next=='$')
				{
				i = k+1;//i++;;
				if ( i < text.length())
					{
					setFormat( i, 1,ColorMath);
					blockData->code[i]=1;
					}
				}
		} else if (tmp== '\\') {
			if (next==']')
				{
				setFormat( i, 1,ColorMath);
				blockData->code[i]=1;
				state=StateGraphic;
				i = k+1;//i++;;
				if ( i < text.length())
					{
					setFormat( i, 1,ColorMath);
					blockData->code[i]=1;
					}
				}
			else
				{
				setFormat( i, 1,ColorMath);
				blockData->code[i]=1;
				state=StateGraphicMath;
				}
		} else
		if (tmp== '{' ){
			setFormat( i, 1,ColorMath);
			blockData->code[i]=1;
			state=StateGraphicMath;
		} else
		if (tmp== '}' ){
			setFormat( i, 1,ColorMath);
			blockData->code[i]=1;
			state=StateGraphicMath;
		} else
		 {
			setFormat( i, 1,ColorMath);
			blockData->code[i]=1;
			state=StateGraphicMath;
		}
	buffer = QString::null;
	} break;

       case StateGraphic: {
               tmp=text.at( i );
		if (tmp=='\\') {
			if (next=='[')
				{
				setFormat( i, 1,ColorStandard );
				blockData->code[i]=1;
				i = k+1;//i++;;
				if ( i < text.length())
					{
					setFormat( i, 1,ColorStandard);
					blockData->code[i]=1;
					}
				}
			else
			{
			setFormat( i, 1,ColorStandard );
			blockData->code[i]=1;
			state=StateGraphicCommand;
			}
		} else
		if (tmp=='$') {
			setFormat( i, 1,ColorMath);
			blockData->code[i]=1;
			state=StateGraphicMath;
			if (next=='$')
				{
				i = k+1;//i++;;
				if ( i < text.length())
					{
					setFormat( i, 1,ColorMath);
					blockData->code[i]=1;
					}
				}
			buffer = QString::null;
		} else
		if (tmp== '/' && next=='/'){
			setFormat( i, 1,ColorComment);
			state=StateGraphicComment;
			blockData->code[i]=1;
			buffer = QString::null;
// 		} else
// 		if (tmp== '%' ){
// 			setFormat( i, 1,ColorComment);
// 			state=StateGraphicComment;
// 			blockData->code[i]=1;
// 			buffer = QString::null;
		} else
		if (tmp== '{' ){
			blockData->code[i]=1;
			setFormat( i, 1,ColorStandard);
		} else
		if (tmp== '}' ){
		  	blockData->code[i]=1;
			setFormat( i, 1,ColorStandard);
			state=StateGraphic;
			buffer = QString::null;
		} else
		if (tmp== '(' ){
			blockData->code[i]=1;
			setFormat( i, 1,ColorStandard);
			buffer = QString::null;
		} else
		if (tmp== ')' ){
			blockData->code[i]=1;
			setFormat( i, 1,ColorStandard);
			buffer = QString::null;
		} else
		if (isWordSeparator(tmp) && tmp!='"' && tmp!='.'  && tmp!='-'){
			blockData->code[i]=1;
			setFormat( i, 1,ColorStandard);
			buffer = QString::null;
		} else
		 {
			setFormat( i, 1,ColorStandard);
			//buffer = QString::null;
		}
       } break;
       
	case StateGraphicComment: {
	setFormat( i, 1,ColorComment);
	blockData->code[i]=1;
	state=StateGraphicComment;
	buffer = QString::null;
	} break;
	
	case StateGraphicCommand:{
		tmp=text.at( i );
		if (tmp=='$') {
			if (last=='\\')
				{
				setFormat( i, 1,ColorStandard);
				blockData->code[i]=1;
				state=StateGraphic;
				}
			else
				{
				setFormat( i, 1,ColorStandard);
				blockData->code[i]=1;
				state=StateGraphic;
				}
		} else
		if (tmp=='%') {
			if (last=='\\')
				{
				setFormat( i, 1,ColorStandard);
				state=StateGraphic;
				}
			else
				{
				setFormat( i, 1,ColorStandard);
				blockData->code[i]=1;
				state=StateGraphic;
         			}
		} else
		if (tmp== ' ') {
         		setFormat( i, 1,ColorStandard);
         		state=StateGraphic;
		}  else
		if (tmp=='(' || tmp=='[' || tmp=='{' || tmp==')' || tmp==']' || tmp=='}') {
			blockData->code[i]=1;
			setFormat( i, 1,ColorStandard);
			state=StateGraphic;
		} else
		if (tmp=='\\' || tmp==',' || tmp==';' || /*tmp=='\'' ||*/ tmp=='\"' || tmp=='`' || tmp=='^' || tmp=='~') {
			blockData->code[i]=1;
			if (last=='\\')
			{
				setFormat( i, 1,ColorStandard);
				blockData->code[i]=1;
				state=StateGraphic;
			}
			else
			{
				setFormat( i, 1,ColorStandard);
				blockData->code[i]=1;
				state=StateGraphic;
			}
		} else
	     		{
         		setFormat( i, 1,ColorStandard);
			//blockData->code[i]=1;
         		state=StateGraphicCommand;
		}
	} break;

	}
	last = ch;
    i = k+1;i++;
	}
if ( state == StateGraphicMath ) 
	{
	setCurrentBlockState(StateGraphicMath) ;
    	}
else if ( state == StateGraphicComment ) 
       {
       setCurrentBlockState(StateGraphic) ;
       }
else if ( state == StateGraphic ) 
       {
       setCurrentBlockState(StateGraphic) ;
       }
else if ( state == StateGraphicCommand ) 
       {
       setCurrentBlockState(StateGraphicCommand) ;
       }
else 
	{
	setCurrentBlockState(StateGraphic) ;
	state=StateGraphic;
    	}

if (CLEAR_CONTROL_CHARS(text).isEmpty()) return;
if (state == StateGraphic || state == StateGraphicCommand || state == StateGraphicComment)
{
	QRegExp number("[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?");
    int index = number.indexIn(CLEAR_CONTROL_CHARS(text));
        while (index >= 0) {
            int length = number.matchedLength();
            if (blockData->code[index]!=1) setFormat(index, length, ColorNumberGraphic);
            index = number.indexIn(CLEAR_CONTROL_CHARS(text), index + length);
        }
        QRegExp expression("\\b[A-Za-z0-9_"+UNICODE_CONTROL_CHARS.join("")+"]+(?=\\()");
    index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            if (blockData->code[index]!=1) setFormat(index, length, ColorKeyword);
            index = expression.indexIn(text, index + length);
        }

	i=0;
	while (i < text.length())
		{
		buffer = QString::null;
		ch = text.at( i );
		while ((blockData->code[i]!=1 || ch=='\\') && (!isSpace(ch)))
		      {
		      buffer += ch;
		      i++;
		      if (i < text.length()) ch = text.at( i );
		      else break;
		      }
		if ((buffer.length() > 0) ) 
		    {
			/////////////////////////////////////////////////
			//added by S. R. Alavizadeh
			//Bi-Di Support
			QString tmpBuffer = buffer;
			//This just skips Unicode Control Characters.
			if (QBiDiExtender::bidiEnabled)
				tmpBuffer = QBiDiExtender::removeUnicodeControlCharacters(buffer);
			/////////////////////////////////////////////////
		    for ( QStringList::Iterator it = KeyWordsGraphic.begin(); it != KeyWordsGraphic.end(); ++it ) 
					{
        				if (*it==tmpBuffer ) 
						{
                        setFormat( i - buffer.length(), buffer.length(),ColorKeywordGraphic);
						}
					}
		    for ( QStringList::Iterator it = KeyWordsGraphicBis.begin(); it != KeyWordsGraphicBis.end(); ++it ) 
					{
        				if (*it==tmpBuffer ) 
						{
                        setFormat( i - buffer.length(), buffer.length(),asyFormat);
						}
					}
		    }
		i++;
		}

}
}
//*****************************************************************

	
	
}

bool LatexHighlighter::isWordSeparator(QChar c) const
{
    switch (c.toLatin1()) {
    case '.':
    case ',':
    case '?':
    case '!':
    case ':':
    case ';':
    case '-':
    case '<':
    case '>':
    case '[':
    case ']':
    case '(':
    case ')':
    case '{':
    case '}':
    case '=':
    case '/':
    case '+':
    case '%':
    case '&':
    case '^':
    case '`':
//    case '*':
//    case '\'':
    case '"':
    case '~':
        return true;
    default:
        return false;
    }
}

bool LatexHighlighter::isSpace(QChar c) const
{
    return c == QLatin1Char(' ')
        || c == QChar::Nbsp
        || c == QChar::LineSeparator
        || c == QLatin1Char('\t')
        ;
}

 void LatexHighlighter::setSpellChecker(Hunspell * checker)
{
pChecker = checker;
if (pChecker) spell_encoding=QString(pChecker->get_dic_encoding());
}

void LatexHighlighter::activateInlineSpell(bool enable)
{
checkSpelling=enable;
}

void LatexHighlighter::setModeGraphic(bool m)
{
isGraphic=m;
}
