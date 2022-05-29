/***************************************************************
 * Name:      defines.h
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2020-05-10
 * Modified:  2022-05-30
 * Copyright: LETARTARE
 * License:   GPL
 **************************************************************/

///-----------------------------------------------------------------------------
#ifndef _DEFINES_H_
	#define _DEFINES_H_

///-----------------------------------------------------------------------------
/** Version
 */
#define VERSION_COLLECT wxString("1.7.1")

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

/** \brief  external traductor
 */
#define EXTERN_TRANSLATOR wxString("Poedit")

/** @brief end of line for Win/Linux/OX
 */
#define 	Cr 			wxString("\r")
#define 	Lf 			wxString("\n")
#define 	CrLf 		wxString("\r\n")
#define 	Eol 		wxString("\r\n")
#define 	Quote 		wxString("'")
#define     cQuote      '\''
#define		Dquote  	wxString("\"")
#define     cDquote     '"'
#define 	Tab			wxString("\t")
#define 	cTab		'\t'
#define 	Space		wxString(" ")
#define 	space(__n)	wxString(' ', __n)
// not uses 'space' reserved word
#define 	cSpace		' '
#define 	Dot			wxString(".")
#define		cDot		'.'

//#define 	EXTRADISPLAY	50

/** \brief text separator
 */
#define 	SepD 		char(13) 	// 0xD, \n
#define 	SepA 		char(10)	// 0xA, \r
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
#define 	markNS(__s)		("=>" + Eol + wxString(__s) + Eol + "<=")
// for __s is a string, __c is a character
#define     within(__s, __c) (__s.AfterFirst('__c').BeforeLast('__c') )

/** \brief  for print an integer and a boolean withless space
 */
#define strInt(__n)			( wxString()<<__n )
// length constant = __l
#define strIntZ(__n, __l)	( wxString('0', __l-(strInt(__n).Len())) += strInt(__n) )
// for __s is a string, __l is a number
#define strZ(__s, __l)		( __s.Prepend( (__s.Len() <= __l) ? wxString('0', __l-__s.Len()) : "" ) )
#define strBool(__b)		( wxString()<<(__b==0 ? _("false" ): _("true")) )
#define strChar(__c)		wxString(__c)

#include <wx/filefn.h>
/** @brief directory separator
 *  use "...." +  strSlash ,  not  "...."  +  Slash !!
 */
#define 	slash 		wxFILE_SEP_PATH
#define 	strSlash  	wxString(slash)
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
#define _printCritical(__a) pLm->LogCritical(__a, __p)

#ifdef WITH_MES_DEBUG
	#define _printD(__a)	pLm->Log(__a, __p)
#else
	#define _printD(__a)
#endif

/** @brief messages  -> 'Code::Blocks log'
 */
#define _Print			pLm->Log
#define _PrintLn		pLm->Log(wxEmptyString)
#define _PrintWarn		pLm->LogWarning
#define _PrintError		pLm->LogError
#define _PrintCritical   pLm->LogCritical
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
#define printCritical(__a)	Collector->MesToLog(__a, Logger::critical)

///-----------------------------------------------------------------------------
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
/// wks
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
///-----------------------------------------------------------------------------

#endif // _DEFINES_H_
