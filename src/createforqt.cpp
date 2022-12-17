/***************************************************************
 * Name:      createforqt.cpp
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2020-05-10
 * Modified:  2022-11-24
 * Copyright: LETARTARE
 * License:   GPL
 **************************************************************/
#include "createforqt.h"

#include <manager.h>
#include <cbproject.h>
#include <compiletargetbase.h>
#include <projectmanager.h>
#include <macrosmanager.h>
#include <editormanager.h>
#include <cbeditor.h>
#include <wx/datetime.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <projectfile.h>
#include "infowindow.h"     // InfoWindow::Display(
// not change  !
#include "defines.h"
// -----------------------------------------------------------------------------
// Constuctor

// Called by :
//		1. Collector::OnAttach():1,
//
CreateForQt::CreateForQt(const wxString & _namePlugin, const int _logIndex)
	: Pre(_namePlugin, _logIndex)
{

}
// -----------------------------------------------------------------------------
// Destructor
//
// Called by :
//		1. Collector::OnRelease():1,
//
CreateForQt::~CreateForQt()
{
	m_pProject = nullptr;  m_pMam = nullptr;
}

// -----------------------------------------------------------------------------
// Detection of a 'Qt' Project : it contains at least one target Qt
//
// Called by  :
//		1. Collector::detectTypeProject(cbProject * _pProject, bool _report):1,
//
// Calls to :
//		1. hasLibQt(CompileTargetBase * _pContainer):1,
//
bool CreateForQt::detectLibProject(cbProject * _pProject, bool _report)
{
	if (! _pProject)	return false;

_printD("=> Begin 'CreateForQt::detectLibProject (..., " + strBool(_report) + ")'" );

// libraries 'Qt' in project ?
	m_pProject = _pProject;
	bool isQtProject =  hasLibQt(m_pProject);
	if (!isQtProject)
	{
// libraries 'Qt' in targets ?
		// search in compileable targets
		for( wxString target: Pre::compileableProjectTargets())
		{
			isQtProject = hasLibQt(m_pProject->GetBuildTarget(target));
			if (isQtProject) break;
		}
	}
// finded
	if (isQtProject && _report)
	{
		wxString nametarget = m_pProject->GetTitle() ;
		wxString title = _("The project") +  quote(m_Nameproject) + _("uses 'Qt' libraries") + " !" ;
		wxString txt =  quote( m_namePlugin ) + _("can generate the translatable strings") + "..." ;
		// also to 'Code::Blocks' log
	    InfoWindow::Display(title, txt, 5000);
	}

_printD("	<= End 'CreateForQt::detectLibProject (...) => " + strBool(isQtProject) );

	return isQtProject;
}

// -----------------------------------------------------------------------------
// Find path 'Qt' container  ->  'CompileOptionsBase * container'
//
// Called by  :
//		1. findTargetQtexe(CompileTargetBase * _buildtarget):1,
//
wxString CreateForQt::pathlibQt(CompileTargetBase * _pContainer)
{
	wxString path;
	if (!_pContainer)
		return path;

	wxArrayString tablibdirs = _pContainer->GetLibDirs() ;
	wxUint32  npath = tablibdirs.GetCount() ;
//Mes = "path lib number = " + strInt(npath); _printError(Mes);
	if (!npath)
		return path;

	wxString  path_nomacro ;
	if (npath > 0 )
	{
		bool ok = false ;
		wxUint32 u = 0  ;
		while (u < npath && !ok )
		{
			path = tablibdirs.Item(u++);
			path.Lower();
			// used local variable ?
			ok = path.Find("$qt") !=  wxNOT_FOUND ;
			if (ok)
			{
//Mes = Lf + "$qt => " + quote( path ); _printWarn(Mes);
				m_pMam->ReplaceEnvVars(path) ;
//Mes = Lf + _("Local variable path 'Qt' => '") + path; Mes += "'"; _print(Mes);
				path_nomacro =  path ;
				// remove "\lib"
				path_nomacro = path_nomacro.BeforeLast(cSlash) ;
				path_nomacro += wxString(cSlash)  ;
			}
			else
			{  // use global variable ?
				ok = path.Find("#qt") !=  wxNOT_FOUND ;
				if (ok)
				{
//	debug qt
//Mes = Lf + _("#qt => ") + quote( path ); _printWarn(Mes);
					m_pMam->ReplaceMacros(path) ;
//Mes = Lf + _("Global variable path Qt => '") + path;Mes += "'"; _print(Mes);
					path_nomacro =  path ;
					// remove "\lib"
					path_nomacro = path_nomacro.BeforeLast(cSlash) ;
					path_nomacro += wxString(cSlash)  ;
				}
				// no variable ! , absolute path ?
				else
				{
//Mes = Lf + _("Path Qt => '") + path ; Mes += "'";	_printWarn(Mes);
					path_nomacro += path;
				}
			}
		}
	}
	Mes.Clear();

	return path_nomacro  ;
}

// ----------------------------------------------------------------------------
// Find path Qt exe : 'ProjectBuildTarget* _pBuildTarget'
//
// Called by  :
//		1.
//		2.
//
bool CreateForQt::findTargetQtexe(CompileTargetBase * _pBuildTarget)
{
	if (! _pBuildTarget)
		return false  ;

	wxString qtpath = pathlibQt(_pBuildTarget) ;
// Mes = Lf + "'Qt'" + _(" path for the target = ") + quote(qtpath); _print(Mes);
	if(qtpath.IsEmpty() || qtpath == "\\" )
	{
		Mes = _("No library path 'Qt' in the target") + Lf + _("or nothing library") + " !"  ;
		Mes += Lf +  _("Check that you have a correct path to the 'Qt' path libraries") + " !" ;
		Mes += Lf + _("Cannot continue") ;
		_printError(Mes);
		ShowError(Mes);
		return false ;
	}

	wxString qtexe = qtpath + "bin" + strSlash ;
//	Mes = "qtexe =" + quote(qtexe) ;  _printWarn(Mes);
	if (m_Win)
	{
		// update *.ts
		m_LUexe = qtexe + "lupdate.exe" ;
		// compile *_fr.ts -> *_fr.qm
		m_LRexe = qtexe + "lrelease.exe";;
		// llinguist
		m_LLexe = qtexe + "linguist.exe" ;
	}
	else
	if (m_Linux)
	{
		m_LUexe = qtexe + "lupdate" ;
		m_LRexe = qtexe + "lrelease" ;
		m_LLexe = qtexe + "linguist";
	}
	else
	if (m_Mac)
	{ 	// ??
		m_LUexe = qtexe + "lupdate" ;
		m_LRexe = qtexe + "lrelease" ;
		m_LLexe = qtexe + "linguist";
	}
//	Mes = "m_LUexe = "+ quote(m_LUexe) ;  _printWarn(Mes);
//	Mes = "m_LRexe = " + quote(m_LRexe) ;  _printWarn(Mes);
//	Mes = "m_LLexe = " + quote(m_LLexe) ;  _printWarn(Mes);
    bool Findqtexe = true, dquote = false;

	bool FindLexe  = ::wxFileExists(m_LRexe) ;
	if (!FindLexe)
    {
        Mes = _("Could not query the executable 'Qt'") +  quote(m_LRexe)  ;
        dquote = m_LRexe.find(Dquote);
        Findqtexe = false;
    }
// error on path ?
    if (!Findqtexe )
    {
		Mes += Lf + _("Cannot continue") ;
		Mes += ", " ;
		Mes += _("Verify your installation 'Qt'");
		if (dquote)
		{
            Mes += "(!! " + _("you have a quote in path") + " !!)";
		}
		Mes += "." ;
		_printError(Mes);

		wxString title = _("Search executable 'Qt'") ;
		title += " ..." ;
		cbMessageBox(Mes, title, wxICON_ERROR) ;
	}

	Mes.Clear();

	return Findqtexe ;
}
// ----------------------------------------------------------------------------
//
// Called by
//		1. Pre::findGoodfiles(bool _verify):1,
//
// Call to :
//     1. Pre::readFileContents(const wxString& _filename):1,
//     2. CreateForQt::listStringsCode(wxString _shortfile):1,
// ----------------------------------------------------------------------------
//bool CreateForQt::isReadableFile(const wxString& _file, const bool /*_absolute*/)
wxInt32 CreateForQt::isReadableFile(const wxString& _file, const bool /*_absolute*/)
{
_printD("=> Begin 'CreateForQt::isElegible(" + _file + ")'");

//	bool ok;
	wxInt32 nbStr =0;
	wxString ext = _file.AfterLast(cDot);
	bool okcode = ext.Matches(EXT_H)  || ext.Matches(EXT_CPP);
		  okcode = okcode || ext.Matches(EXT_CXX);
		  okcode = okcode || ext.Matches("ui");
		  okcode = okcode || ext.Matches("qrc"); // ??
//	ok = okcode;
/*
// debug : for test all files !!!
//	okcode = true
	if (ok)
	{
//_print("1- _file :" + _file + " => ok :" + strBool(ok) );
	//1- verify exist
		wxString namefile = m_Dirproject + _file;
		if (! wxFileName::FileExists(namefile))
		{
			Mes = "*** " + _("function 'isElegible' : file ");
			Mes += quote(namefile) + _("NOT FOUND") + " !!!" + Lf;
			_printError(Mes);
			m_Fileswithstrings.Add(Mes);
			return false;
		}
	//2- read file
		wxString source = Pre::readFileContents(namefile);
		if (source.IsEmpty())
		{
			Mes =  "*** " + _("function 'isElegible' : file ");
			Mes += quote(namefile) + _("is empty") +  " !!!" + Lf;
			_printError(Mes);
			return false;
		}
//_print("2- _file :" + _file + " => ok :" + strBool(ok) );

		wxString wait = wxEmptyString;
	// message
        // length
        size_t lfile = source.Len();
		Mes = Tab + strInt(++m_Nfanalyze) + "- " + quote(_file);
		Mes += "(" + strInt(lfile) + _("bytes") + ")" + wait;
		_print(Mes);
		m_Fileswithstrings.Add(Mes);

//_print("3- _file :" + _file + " => ok :" + strBool(ok) );
	//3- extensions *.h, *.cpp, *.cxx, *.script
		if (okcode)
		{
		// use 'xgettext'
			ok = CreateForQt::listStringsCode(_file);
//_print("4- _file :" + _file + " => ok :" + strBool(ok) );
			if (ok)
			{
				//
			}
		}
	} // end ok

*/

_printD("	<= End 'CreateForQt::isElegible(...)'");
	//return ok	;
	return nbStr;
}

// ----------------------------------------------------------------------------
//
// Called by :
//		1. Pre::findGoodfiles(bool _verify):1,
// Call to :
//		1. Pre::GexewithError(wxString, wxString, bool):1,
// ----------------------------------------------------------------------------
wxInt32  CreateForQt::listStringsCode(const wxString& _shortfile)
{
_printD("=> Begin 'CreateForQt::listStringsCode(" + _shortfile + ")'");

	wxString ext = _shortfile.AfterLast(cDot);;
	wxString longfile = _shortfile;
// analyze shortfile
//1- absolute name inputfile
	if (m_Workspace) 	longfile.Prepend(m_Dirproject);
//2- build command : 'xgettext -C -k_ infile -o- ....'
	wxString key = "-k" + m_Keyword;
	wxString command = m_PathGexe + " --c++ --from-code=UTF-8 " + key;
	// for qt format
	//command += " --qt";
	// inputfile
	command += dquote(longfile) ;
	// output  to 'stdout'
	command += " -o-";
	// without header
//	command += " --omit-header";
	// text indent
	command += " -i";
	// files sorted
	//command += " -F";
	// data sorted to out
	//command += " -s" ;
	// text width before new line
	//command += " -w 180";
	command += " -w 80";
	//command += " --no-wrap " ;

//_print("command :" + quote(command));
//3- command execute with errors
	// list string into
	wxInt32 nbstr = Pre::GexewithError(_shortfile, command, PREPEND_ERROR);
//print("_shortfile :" + _shortfile + " => ok :" + strBool(ok));

	if (nbstr > 0)
	{
//4- print file name
		Mes = Tab + "-----> " + _("Elected") + strInt(m_nbFileEligible) + "-"  +  quote(_shortfile);;
		_printWarn(Mes);
		Mes = Tab + "E" + strInt( m_nbFileEligible) + "-" + quote(_shortfile);
		m_Fileswithstrings.Add(Mes);
    // use
        printWarn(Mes);
	}
	else
	{
		// TO DO ...
	}

_printD("	<= End 'CreateForQt::listStringsCode(...) => " + strInt(nbstr) + ")'");

	return nbstr;
}
// ----------------------------------------------------------------------------
// Common init to 'List' and 'Extract': return 'true' on right
//
// Called by :
//     1. Pre::Init():1,
// Call to :
//     1. CreateForQt::searchLib():1,
//     2. CreateForQt::searchExe():1,
///-----------------------------------------------------------------------------
bool CreateForQt::initTailored(const wxString /*_type*/)
{
_printD("=> Begin 'CreateForQt::initTailored()'");
/*
	//9- is project 'Qt' ?
	if (!searchLib())	return false ;
	//10- find executable paths  ('poedit', 'wxrc', 'xgettext', 'msmerge')
	bool ok = searchExe();
	if (!ok)	return false;
	//11- array clean
		m_Nameprjwx.Clear();
		m_FileswithI18n.Clear();
		m_PrjwithI18nWS.Clear();
		m_Fileswithstrings.Clear();
	//12- display Wx
		// changed directory
		wxFileName::SetCwd(m_Dirproject);
		// messages
		Mes = "-------------------------------------------------------------";
    	_print(Mes);
		Mes = _("Begin") + quote(m_Thename + "-" + VERSION_COLLECT) + ":";
		_print(Mes);
		m_Fileswithstrings.Add(Mes);

		// Wx path
		Mes = Tab + _("Project use 'Wx' path") + " :" +   quote(m_Wxpath);
		_print(Mes);
		m_Fileswithstrings.Add(Mes);
		Mes = Tab + _("Eligibles files");
		Mes += " (*.h, *.cpp, *.script, (*.xrc, *.wxs -> *_???.str) ");
		_print(Mes);
		m_Fileswithstrings.Add(Mes);
*/
_printD("	<= End 'CreateForQt::initTailored()'");

	return true;
}
// ----------------------------------------------------------------------------
// Reference of libQt by target
//		"" or  "5r" or "5d" or "0" (for 'qtmain')
//   	qt5 -> '5', release -> 'r' and debug -> 'd'
//		'qtmain' -> '0'
//
// Called by  :
//		-# isGoodTargetQt(const ProjectBuildTarget * _pBuildtarget):1,
//
/*
wxString CreateForQt::refTargetQt(const ProjectBuildTarget * _pBuildTarget)
{
	wxString refqt ;
	if (! _pBuildTarget)
		return refqt  ;

//Mes = "CreateForQt::refTargetQt(...)"; _printWarn(Mes);
	wxArrayString tablibs = _pBuildTarget->GetLinkLibs();
	wxUint32 nlib = tablibs.GetCount() ;
//Mes = "nlib = " + strInt(nlib); _printWarn(Mes);
    if (!nlib)
    {
        tablibs = m_pProject->GetLinkLibs();
        nlib = tablibs.GetCount() ;
        if (!nlib)
        {
            Mes = _("This target has no linked library") ;
            Mes += " !!";
            _printError(Mes);
            return refqt ;
        }
    }
// search lib
	bool ok = false ;
	wxString str =  wxEmptyString;
	wxString namelib ;
	wxUint32 u=0, index=  wxNOT_FOUND, pos ;
	while (u < nlib && !ok )
	{
		// lower, no extension
		namelib = tablibs.Item(u++).Lower().BeforeFirst(cDot) ;
//Mes = strInt(u) + "- namelib = " + quote( namelib ); _printWarn(Mes);
		// no prefix "lib"
		pos = namelib.Find("lib");
		if (pos == 0)
			namelib.Remove(0, 3) ;
		// begin == "qt"
		pos = namelib.Find("qt") ;
		if (pos != 0)	continue ;
//Mes = strInt(pos) ; _printWarn(Mes);
		// compare
		index = m_TablibQt.Index(namelib);
//Mes = strInt(index) ; _printWarn(Mes);
		ok = index !=  wxNOT_FOUND ;
		// first finded
		if (ok)
		{
			// == 'qtmain' or 'qtmaind'
			if ( namelib.StartsWith("qtmain" ) )
			{
				refqt = "0";
			}
			else
			// qt5xxxx or qt5xxxd
			if (namelib.GetChar(2) == '5')
			{
				refqt = "5";
				// the last
				str = namelib.Right(1) ;
				if ( str.Matches("d" ) )
					refqt += "d" ;
				else
					refqt += "r" ;
			}
			else
				refqt = wxEmptyString;

			break;
		}
	}
//Mes = " return refqt = " + quote( refqt ); _printWarn(Mes);
	Mes.Clear();

	return refqt ;
}
*/
// ----------------------------------------------------------------------------
// Give the good type of target for 'Qt'
//
// Called by  :
//		1. Collector::OnAddComplements(CodeBlocksEvent& event):1,
//		2. CreateForQt::findGoodfilesQt():1,
//
// Calls to:
//		1. refTargetQt(ProjectBuildTarget * _pBuildtarget):1,
//
/*
bool CreateForQt::isGoodTargetQt(const ProjectBuildTarget * _pBuildTarget)
{
	if (!_pBuildTarget)
		return false ;

	//bool ok = ! Pre::isCommandTarget(_pBuildTarget);
	bool ok = ! isCommandTarget(_pBuildTarget);
	wxString str = refTargetQt(_pBuildTarget)  ;
//Mes = "str = " + quote( str ); _printWarn(Mes);
	bool isqt = ! str.IsEmpty()  ;
	if (!isqt)
	{   // TODO ...
		Mes = Tab + _("This target have nothing library' Qt'") + " !" ;
		Mes += Lf + Tab + _("PreBuild cannot continue.") ;
		_printWarn(Mes);
	}
//Mes = "isGoodTargetQt  = " + strBool( ok && isqt); _printWarn(Mes);
	Mes.Clear();

	return ok && isqt ;
}
*/

// ----------------------------------------------------------------------------
// Gives the name of the file to create on
//		file = xxx.h(pp) ->	moc_xxx.cpp
//		file = xxx.ui	->	ui_xxx.h
//		file = xxx.rc	->  rc_xxx.cpp
//		file = xxx.cpp	->  xxx.moc
//
// Called by  :
//	1. CreateForQt::buildOneFile(cbProject * _pProject, const wxString& _fcreator):1,
//	2. CreateForQt::unregisterComplementFile(const wxString & file, bool _creator, bool _first):1,
//	3. CreateForAt::addAllFiles():1,
//
wxString CreateForQt::nameCreated(const wxString& _file)
{
	wxString name  = _file.BeforeLast(cDot) ;
	wxString fout ;
	if (name.Find(cSlash) > 0)
	{
	// short name
		name = name.AfterLast(cSlash) ;
	// before path
		fout += _file.BeforeLast(cSlash) + wxString(cSlash) ;
	}
	wxString ext  = _file.AfterLast(cDot)  ;
//1- file *.ui  (forms)
	if ( ext.Matches(m_UI) )
		fout += m_UI + "_" + name + DOT_EXT_H  ;
	else
//2- file *.qrc  (resource)
	if (ext.Matches(m_Qrc) )
		fout += m_Qrc + "_" + name + DOT_EXT_CPP  ;
	else
//3- file *.h or *hpp with 'Q_OBJECT' and/or 'Q_GADGET'
	if (ext.Matches(EXT_H) || ext.Matches(EXT_HPP)  )
		fout +=  m_Moc + "_" + name + DOT_EXT_CPP ;
	else
//4- file *.cpp with 'Q_OBJECT' and/or 'Q_GADGET'
	if (ext.Matches(EXT_CPP) )
		fout +=  name + DOT_EXT_MOC ;

	fout = fout.AfterLast(cSlash) ;

	return fout  ;
}

/*
// ----------------------------------------------------------------------------
// Search virtual Qt target
//
// Called by  :
//		1. Pre::detectQtTarget(const wxString& _nametarget, bool _report):1
//	Call to :
//		1.
//		2.
bool CreateForQt::isVirtualQtTarget(const wxString& _namevirtualtarget, bool _warning)
{
	bool ok = m_pProject->HasVirtualBuildTarget(_namevirtualtarget);
	if (ok)
	{
		//ProjectBuildTarget * pBuildTarget ;
		wxArrayString realtargets = Pre::compileableVirtualTargets(_namevirtualtarget);
		// search the first Qt targets
		for (wxString target : realtargets)
		{
			ok = hasLibQt(m_pProject->GetBuildTarget(target));
			if (ok) break;
		}
		if (ok && _warning)
		{
			Mes = quote(_namevirtualtarget) + _("is a virtual 'Qt' target") + " !!" ;
			_print(Mes);
		}
	}

	return ok  ;
}
*/

// ----------------------------------------------------------------------------
// Detection of a 'Qt' target
//
// Called by  :
//		1. Collector::OnActivateTarget(CodeBlocksEvent& _pEvent):1,
//
// Calls to :
//		1. Pre::hasLibQt(CompileTargetBase * _pContainer):1,
//
/*
bool CreateForQt::detectLibTarget(const wxString& _nametarget, bool _report)
{
_printD("=> Begin 'CreateForQt::detectQtTarget(" + _nametarget + ", " + strBool(_report) + ")'" ) ;

	bool ok = false ;
	if(!m_pProject)  return ok;
	//if (Pre::isCommandTarget(_nametarget)) 	return ok;
	if (Pre::isCommandTarget(_nametarget)) 	return ok;
// is one virtual target
	bool virtQt = isVirtualQtTarget(_nametarget);
	wxArrayString compilTargets;	//
	if (virtQt)
	{
		compilTargets =  Pre::compileableVirtualTargets(_nametarget);
		if (_report)
		{
			Mes = Tab + quote("::" + _nametarget);
			Mes += _("is a virtual 'Qt' target that drives") ;
			Mes += " ...";  _printWarn(Mes);
		}
	}
	else
		compilTargets.Add(_nametarget) ;

// all compileable targets
	bool goodQt = false;
	for (wxString target : compilTargets)
	{
//Mes = "compare : " + quote( target) +  ", " + quote( _nametarget);
//_print(Mes);
		ok = hasLibQt(m_pProject->GetBuildTarget(target));
//Mes = strBool(ok) ; _printError(Mes);
		if (_report)
		{
			if (virtQt) 	Mes = Tab ;
			else			Mes = wxEmptyString;
			Mes += Tab + quote( "::" + target);
		// advices
			Mes += Tab + Tab + _("is") + Space;
			if(!ok)		Mes += _("NOT") + Space;
			Mes += _("a 'Qt' target.");
			_print(Mes);
		}
		goodQt = goodQt || ok ;
	}

_printD("	=> End 'CreateForQt::detectQtTarget(...)" + strBool(goodQt) );

	return goodQt;
}
*/
// -----------------------------------------------------------------------------
// Return true if good  'CompileTargetBase* container'
//
// Called by  :
//		2. CreateForQt::detectQtProject(cbProject * _pProject, bool _report):1,
//
bool CreateForQt::hasLibQt(CompileTargetBase * _pContainer)
{
_printD("=> Begin 'CreateForQt::hasLibQt(...)'");

	bool ok = false;
	if (!_pContainer) 	return ok;

	wxArrayString tablibs = _pContainer->GetLinkLibs() ;
	wxUint16 nlib = tablibs.GetCount() ;
//_print("nlib = " + strInt(nlib) );
	if (nlib > 0)
	{
		wxString namelib ;
		wxUint16 u=0;
		while (u < nlib && !ok )
		{
			// lower
			namelib = tablibs.Item(u++).Lower() ;
			ok = namelib.StartsWith("libqt")
				|| namelib.StartsWith("qt")
				;
			if (ok) break;
		}
	}

_printD("	<= End 'Pre::hasLibQt(...)' => " + strBool(ok));

	return ok;
}

// /////////////////////////// Search into files //////////////////////////////


// -----------------------------------------------------------------------------
// Creating a file '*.pot'
//
// Called by :
//		1. Pre::extraction(bool _prjfirst, cbProject * _prj):1,
//Call to :
//		1. Pre::GexewithError(wxString _shortfile, wxString _command, bool _prepend):1,
//
bool CreateForQt::creatingPot(bool _prjfirst)
{
_printD("=> Begin 'CreateForQt::creatingPot(" + strBool(_prjfirst) + ")" );

    if (m_PathGexe.empty())
    {
        Mes = " !!! 'xgettext' " + _("not defined") + " !!!" ;
        _printError(Mes);
        return false;
    }

// local variables
	bool ok = false, good = false;

_printD("	<= End 'CreateForQt::creatingPot(...)' => " + strBool(ok && good) );

	return ok && good ;
}

// ----------------------------------------------------------------------------
// Called by :
//		1. Collector::OnMenuCleanTemp(wxCommandEvent& _pEvent):1,
//
// Call to :
//		1. CreateForQt::deletefilesStr(bool _same):1,
//
wxUint32 CreateForQt::Cleantemp()
{
_printD("=> Begin 'CreateForQt::Cleantemp()'");

// messages
	Mes = "---> " + _("begin 'Clean temporary files'");
	_print(Lf + Line(Mes.Len()));
	_print(Mes);

	m_Fileswithstrings.Add(Mes);

//1- delete all temporary files of last project
	// TO DO ...
	wxUint32 nfc =	0; // deletefilesStr(false);

//2- init
	m_FileStrcreated.Clear();

	// messages
	Mes = Lf + "<--- " + _("end 'Clean temporary files'");
	_print(Mes);
	_print(Line(Mes.Len()));
    m_Fileswithstrings.Add(Mes);

_printD("	<= End 'CreateForQt::Cleantemp()' =>" + strInt(nfc)  );

	return nfc;
}

///-----------------------------------------------------------------------------
