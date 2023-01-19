/***************************************************************
 * Name:      defines.h
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2020-05-10
 * Modified:  2023-01-17
 * Copyright: LETARTARE
 * License:   GPL
 **************************************************************/

///-----------------------------------------------------------------------------
#ifndef _DEFINES_H_
	#define _DEFINES_H_

///-----------------------------------------------------------------------------
/** Version
 */
#define VERSION_COLLECT wxString("-1.8.0")

//=========================================================
/** \brief  For 'Collector::MesToLog'
 */
#include "collector.h"
//=========================================================

/** \brief display begin and end messages of function
 */
//#define WITH_MES_DEBUG

/** \brief drive for extract string from 'code', 'res', 'xml'
 */
#define NO_CODE		true
#define NO_RES    	true
#define NO_XML    	true

/** \brief  external traductor tools
 */
#define EXTERN_TRANSLATOR wxString("poedit")

/** \brief the keys for translate
 */
#define	KEY_WX	"_"
#define	KEY_QT	"tr"

/** \brief name of directory for complements of 'Qt' project
 */
#define dirAddon	"adding"

/** @brief end of line for Win/Linux/OX
 */
#define 	cCr			'\r'
#define 	Cr 			wxString("\r")
#define 	cLf			'\n'
#define 	Lf 			wxString("\n")
#define 	CrLf 		wxString("\r\n")
#define 	Eol 		wxString("\r\n")
#define     cQuote      '\''
#define 	Quote 		wxString("'")
#define		Dquote  	wxString("\"")
#define     cDquote     '"'
#define 	Tab			wxString("\t")
#define 	cTab		'\t'
#define 	cSpace		' '
// not uses 'space' reserved word
#define 	Space		wxString(" ")
#define 	space(__n)	wxString(' ', __n)
#define		cDot		'.'
#define 	Dot			wxString(".")
#define		cScol		';'
#define 	Scol		wxString(";")
#define 	Scol2		wxString(";;")
//#define 	tLf			wxString("\\n")
#define 	LfScol 			wxString("\n;")

//#define 	EXTRADISPLAY	50

#define     BIGFILE     70000

/** \brief text separator
 */
#define 	SepD 		char(13) 	// 0xD, \r
#define 	SepA 		char(10)	// 0xA, \n
#define 	SizeSep 	2
#define		SizeLe		16
/** \brief lines
 */
#define 	Line(__n)	wxString('-', __n)
#define     Dline(__n)  wxString('=', __n)

/** @brief surrounded by 'Quote'
 */
#define 	quote(__s)		(Space + Quote + wxString(__s) + Quote + Space)
#define 	quoteNS(__s)	(Quote + wxString(__s) + Quote)
#define 	dquote(__s)		(Space + Dquote + wxString(__s) + Dquote + Space)
#define 	dquoteNS(__s)	(Dquote + wxString(__s) + Dquote)
#define 	markNS(__s)		("=>>" + Lf + wxString(__s) + Lf + "<<=")
// for __s is a string, __c is a character
#define     within(__s, __c) (__s.AfterFirst('__c').BeforeLast('__c') )

/** \brief  for print an integer and a boolean withless space
 */
#define strInt(__n)			( wxString()<<__n )
	#define iToStr(__n)			( wxString()<<__n )
// length constant = __l
#define strIntZ(__n, __l)	( wxString('0', __l-(strInt(__n).Len())) += strInt(__n) )
// for __s is a string, __l is a number
#define strZ(__s, __l)		( __s.Prepend( (__s.Len() <= __l) ? wxString('0', __l-__s.Len()) : "" ) )
#define strBool(__b)		( wxString()<<(__b==0 ? _("false" ): _("true")) )
	#define bToStr(__b)			( wxString()<<(__b==0 ? _("false" ): _("true")) )
#define strChar(__c)		wxString(__c)
	#define cToStr(__c)		wxString(__c)

#include <wx/filefn.h>
/** @brief directory separator
 *  use "...." +  strSlash ,  not  "...."  +  cSlash !!
 */
#define 	cSlash 		wxFILE_SEP_PATH
#define 	strSlash  	wxString(cSlash)
///-----------------------------------------------------------------------------
#include <logmanager.h>
/** @brief Methods withless progress bar  -> 'Collector' log
 */
#define pLm					Manager::Get()->GetLogManager()
#define __p              	m_LogPageIndex
#define _print(__a)	    	pLm->Log(__a, __p)
#define _printBox(__a)	    _print(Lf + DLine(__a.Len()) + Lf + __a + DLine(__a.Len()) )
#define _printLn			pLm->Log(wxEmptyString, __p)
#define _printWarn(__a)		pLm->LogWarning(__a, __p)
#define _printError(__a)	pLm->LogError(__a, __p)
//#define _printCritical(__a) pLm->LogCritical(__a, __p)

#ifdef WITH_MES_DEBUG
	#define _printD(__a)		pLm->Log(__a, __p)
	#define _printWarnD(__a)	pLm->LogWarn(__a, __p)
	#define _printErrorD(__a)	pLm->LogError(__a, __p)
#else
	#define _printD(__a)
	#define _printWarnD(__a)
	#define _printErrorD(__a)
#endif

/** @brief messages  -> 'Code::Blocks log'
 */
#define _Print			pLm->Log
#define _PrintLn		pLm->Log(wxEmptyString)
#define _PrintWarn		pLm->LogWarning
#define _PrintError		pLm->LogError
//#define _PrintCritical  pLm->LogCritical
/** @brief messages  -> 'Code::Blocks Debug log'
 */
#define DPrint			pLm->DebugLog
#define DPrintErr		pLm->DebugLogError

/** \brief New method : messages  -> 'Collector' log
 *  \brief with progress bar
 */
// to display or not the progression in 'print(__a)'
//#define PRINT_DISPLAY   true
#define PRINT_DISPLAY       false
#define print(__a)			Collector::MesToLog(__a, Logger::info)
#define printLn		   	 	Collector::MesToLog(wxEmptyString, Logger::info)
#define printWarn(__a)	    Collector::MesToLog(__a, Logger::warning)
#define printError(__a)	    Collector::MesToLog(__a, Logger::error)
//#define printCritical(__a)	Collector->MesToLog(__a, Logger::critical)

///-----------------------------------------------------------------------------
/** @brief directory temporary name
 */
#define	DIR_TEMP	wxString("temp")

/** @brief news extensions for 'Qt'
 */
#define		EXT_UI			"ui"
#define		EXT_MOC			"moc"
#define		EXT_QRC			"qrc"
#define		DOT_EXT_UI		".ui"
#define		DOT_EXT_MOC		".moc"
#define		DOT_EXT_QRC		".qrc"

/** @brief news extensions for 'gettext'
 */
#define		EXT_PO			"po"
#define		EXT_POT			"pot"
#define		EXT_MO			"mo"
#define		DOT_EXT_PO		".po"
#define		DOT_EXT_POT		".pot"
#define		DOT_EXT_MO		".mo"
// for messages 'gettext'
#define 	T_FLAG			"#, "
#define 	T_COMM			"#. "
#define 	T_MES			"#: "
#define		T_FUZZY			T_FLAG"fuzzy"
#define 	T_BANNED		T_COMM"banned"

/** @brief news extensions for 'lconvert'
 */
#define		DOT_EXT_QM		".qm"
#define		DOT_EXT_TS		".ts"

/** Project unknow
 */
#define 	L_UNKNOWN		""
/** Project 'Wx'
 */
#define 	L_WX			"_WX"
/** Project 'Qt'
 */
#define 	L_QT			"_QT"
/** Project 'Wx' + targets 'Qt'
 */
#define 	L_WX_QT			"_WX_QT"
/** Project 'Qt' + targets 'Wx'
 */
#define 	L_QT_WX			"_QT_WX"

/** Name prolong for workspace
 */
#define 	PROLONG_WS		"_ws"
/** New extensions internals
 */
#define 	EXT_DUP			wxString("dup")
#define 	DOT_EXT_DUP		cDot + EXT_DUP
#define 	EXT_UNI			wxString("uni")
#define 	DOT_EXT_UNI		cDot + EXT_UNI

/** Extensions externals
 */
#include <filefilters.h>
/// c++ code
#define  	EXT_H 				FileFilters::H_EXT
#define 	DOT_EXT_H 	        FileFilters::H_DOT_EXT
#define  	EXT_HPP 			FileFilters::HPP_EXT
#define  	DOT_EXT_HPP 		FileFilters::HPP_DOT_EXT
#define  	EXT_CPP 			FileFilters::CPP_EXT
#define 	DOT_EXT_CPP 	    FileFilters::CPP_DOT_EXT
#define  	EXT_CXX 			FileFilters::CXX_EXT
#define  	DOT_EXT_CXX 		FileFilters::CXX_DOT_EXT
#define  	EXT_HXX 			FileFilters::HPP_EXT
#define  	DOT_EXT_HXX 		FileFilters::HXX_DOT_EXT
/// script
#define  	EXT_SCR 			FileFilters::SCRIPT_EXT
#define  	DOT_EXT_SCR 		FileFilters::SCRIPT_DOT_EXT
/// xrc
#define  	EXT_XRC 			FileFilters::XRCRESOURCE_EXT
#define  	DOT_EXT_XRC 		FileFilters::XRCRESOURCE_DOT_EXT
/// wxs
#define		EXT_WXS			 	wxString("wxs")
#define  	DOT_EXT_WXS 		cDot + EXT_WXS
/// xml
#define 	EXT_XML				FileFilters::XML_EXT
#define 	DOT_EXT_XML			FileFilters::XML_DOT_EXT
/// .dll
#define     DOT_DYNAMICLIB_EXT  FileFilters::DYNAMICLIB_DOT_EXT

///-----------------------------------------------------------------------------
/** @brief local booleans
 */

#define WITH_REPORT			true
#define NO_REPORT			false

#define PREPEND				true
#define NO_PREPEND			false

#define PREPEND_ERROR		true
#define NO_PREPEND_ERROR	false

#define WITH_POT			true
#define NO_POT				false

#define FIX_LOG_FONT		true
#define NO_FIX_LOG_FONT		false

#define SEPARATOR_AT_END	true
#define NO_SEPARATOR_AT_END	false

#define IS_RELATIVE			true
#define NO_RELATIVE			false

#define IS_ABSOLUTE			true
#define NO_ABSOLUTE			false

#define IS_UNIX_FILENAME	true
#define NO_UNIX_FILENAME	false

#define WITH_WARNING		true
#define NO_WARNING			false

#define WITH_DEBUG			true
#define NO_DEBUG			false

#define FIRST_PRJ			true
#define NO_FIRST_PRJ		false

#define REFRESH				true
#define NO_REFRESH			false

#define ALL_TXT				true
#define FIRST_TXT			false

#define VERIFY_ONLY			true
#define NO_VERIFY			false

#define FROM_LEFT			false
#define FROM_RIGHT			true

#define TRIM_SPACE			true
#define NO_TRIM_SPACE		false
///-----------------------------------------------------------------------------

#endif // _DEFINES_H_
