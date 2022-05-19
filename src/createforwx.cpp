/***************************************************************
 * Name:      createforwx.cpp
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2020-05-10
 * Modified:  2022-05-18
 * Copyright: LETARTARE
 * License:   GPL
 **************************************************************/
#include "createforwx.h"

#include <sdk.h>
#include <cbplugin.h>      // sdk version
// for Linux
#include <macrosmanager.h>
#include <configmanager.h>
#include <cbproject.h>
//

#include "infowindow.h"     // InfoWindow::Display(
// not change  !
#include "defines.h"

///-----------------------------------------------------------------------------
// Constuctor
//
// Called by :
//		1. Collector::OnAttach():1,
//
CreateForWx::CreateForWx(const wxString _namePlugin, const int _logIndex)
	: Pre(_namePlugin, _logIndex)
{
}
///-----------------------------------------------------------------------------
// Destructor
//
// Called by :
//		1. Collector::OnRelease():1,
//
CreateForWx::~CreateForWx()
{
	//m_pProject = nullptr;  m_pMam = nullptr; m_pMprj = nullptr;
}

// ----------------------------------------------------------------------------
// Detection of a 'Wx' Project : it contains at least one target 'Wx'
//
// Called by :
//		1. Collector::detectTypeProject(cbProject *, bool):1,
//		2. CreateForWx::searchLib(bool):1,
//		3. Pre::releaseProject(cbProject * _pProject, bool _withpot = true):1
// Call to :
//		1. CreateForWx::pathWx(CompileTargetBase *):2,
//
bool CreateForWx::detectLibProject(cbProject * _pProject, bool _report)
{
	if (! _pProject) return false;

_printD("=> Begin 'CreateForWx::detectLibProject(..., " + strBool(_report) + ")'");

	m_pProject = _pProject ;
	wxString name = m_pProject->GetTitle();
// libraries 'Wx' in targets ?
    m_nbTargets = m_pProject->GetBuildTargetsCount();
    ProjectBuildTarget * ptarget;
    wxUint16 nt = 0; m_isWxProject = false;
    while (nt < m_nbTargets && ! m_isWxProject )
    {
    // retrieve the target libraries
        ptarget = m_pProject->GetBuildTarget(nt++);
        if (!ptarget)       continue;
        if (isCommandTarget(ptarget)) continue;
    // search all path libaries Wx
        if(pathWx(ptarget))  m_isWxProject = true;
    }

// finded
    _report = _report && (m_State == fbWaitingForStart);
	if (m_isWxProject && _report)
	{
		wxString title = _("The project") +  quote(m_Nameproject) + _("uses 'Wx' libraries") + " !" ;
		wxString txt =  quote( m_namePlugin ) + _("can generate the translatable strings") + "...";
    // also to 'Code::Blocks' log
        InfoWindow::Display(title, txt, 5000);
	}

_printD("	<= End 'CreateForWx::detectLibProject()' => " + strBool(isWxProject) );

	return m_isWxProject;
}

// ----------------------------------------------------------------------------
// Allows to be called only with 'ProjectBuildTarget *' parameter
// which inherits from 'CompilerTargetBase' which inherits from 'CompileOptionsBase'
// The methods used are all methods of 'CompileOptionsBase'.
//
// Called by :
//     1. CreateForWx::detectLibProject(cbProject * _pProject, bool _report):2,
//
// Call to :
//		1. Pre::executeAndGetOutputAndError(const wxString& _command, bool _prepend_error) :1,
// ----------------------------------------------------------------------------

bool CreateForWx::pathWx(ProjectBuildTarget * _pTarget)
{
_printD("=> Begin 'CreateForWx::pathWx(...)'" );

	if(!_pTarget) return false;
	cbProject * pProject = _pTarget->GetParentProject();
    if(! pProject) return false;

    wxString nameTarget = _pTarget->GetTitle(), nameProject = pProject->GetTitle();
    m_Dirproject = pProject->GetBasePath();

// return true if good
	bool ok = false;
	wxString namevar = wxEmptyString;

// for 'Win'
	if (m_Win)
	{
	// path 'Wx'
		wxString nameinc = wxEmptyString;
        wxArrayString tabincs = pProject->GetIncludeDirs();
		wxUint32 ninc = tabincs.GetCount();
		if (ninc)
		{
            wxString  t = "wx"	,t1 = "$(#" + t, t2 = "$(" + t, t3 = "$" + t;
			wxUint32 u =0;
			while (u < ninc && !ok )
			{
				nameinc = tabincs.Item(u++).Lower();
				ok = nameinc.StartsWith(t1)
					|| nameinc.StartsWith(t2)
					|| nameinc.StartsWith(t3)
					|| nameinc.StartsWith(t)
					;
			}
			if (ok)  		namevar = nameinc.BeforeFirst(slash);
		}

		if (!ok)
		{
	// libraries path
			wxArrayString tablibs = _pTarget->GetLinkLibs();
			wxUint32 nlib = tablibs.GetCount();
			if (nlib)
			{
				wxString namelib, t1 = "libwx", t2 = "wx";
				wxUint32 u=0;
				while (u < nlib && !ok )
				{
				// replace macros
					namelib = tablibs.Item(u++);
					ok = namelib.StartsWith(t1) || namelib.StartsWith(t2);
				}
				if (ok)  		namevar = namelib.BeforeFirst(slash);
			}
		}

// lib finded
		if (ok)
		{
			m_pMam->ReplaceMacros(namevar, _pTarget);
			m_Wxpath = namevar;
		}
		else 	m_Wxpath = wxEmptyString;
	}
// -----------------------------------------------------------------
// for Linux and Mac who uses 'wx-config ...'
    // at first good target !!
	if (! m_Win && !ok)
	{
        wxString response = wxEmptyString ;
// search 'wx-config'
        wxString nameopt, mV1 = "WX_CONFIG", c1 = "wx-config";

    // 1- linker options
        wxArrayString tab = _pTarget->GetLinkerOptions();
        wxUint32 nopts = tab.GetCount();
        if (nopts)
        {
        // for debug
           // _print(allStrTable(nameTarget + " => " + _("Linker options"), tab));
        // search t1
            wxUint32 u=0;
            while (u < nopts && !ok )
            {
                nameopt = tab.Item(u++);
            // replace macros
                m_pMam->ReplaceMacros(nameopt, _pTarget);
                ok = nameopt.Contains(c1);
            }
        }
    // 2- compiler options
        if (!ok)
        {
            tab = _pTarget->GetCompilerOptions();
            nopts = tab.GetCount();
            if (nopts)
            {
            // search c1
                wxUint32 u=0;
                while (u < nopts && !ok )
                {
                  nameopt = tab.Item(u++);
                // replace macros
                  m_pMam->ReplaceMacros(nameopt, _pTarget);
                  ok = nameopt.Contains(c1);
                }
            }
        }
    // 3- custom variable
        if (!ok)
        {
            nameopt = pProject->GetVar(mV1);
            ok = nameopt.Contains(c1);
            nameopt += " --libs";
        }

    //4- verify executing :
        if (ok)
        {
        // 'wx-config --version=3.0 --libs'
            nameopt.Replace("`", wxEmptyString);
        // wx-config --version=3.0 --libs
            response = Pre::executeAndGetOutputAndError(nameopt, PREPEND_ERROR);
            // warning ...
            ok = ! response.Contains("Warning:");
            if (!ok)
            {
                Mes = Lf + _("ATTENTION") ;
                Mes += quote(nameProject + "::" + nameTarget);
                Mes += "=>" + quote(nameopt);
                _printError(Mes);
                _printWarn(quote(response));
                m_Wxpath = wxEmptyString;
            }
            else
            {
                m_Wxpath  = response.BeforeFirst(' ').AfterFirst('L');
            }
        }
        else	  m_Wxpath = wxEmptyString;
    }

_printD("	<= End ' CreateForWx::pathWx()' => " + strBool(ok) );

	return ok;
}

///-----------------------------------------------------------------------------
//
// Called by :
//     1. CreateForWx::initTailored(const wxString _type):1,
//
///-----------------------------------------------------------------------------
void CreateForWx::initWx()
{
_printD("Begin 'CreateForWx::initWx()'" );
// platform
	if (m_Win)
	{
		m_Rexe = "wxrc.exe";
	// file extensions for 'Wx'
		EXT_WXS = "wxs"; m_Exttmp = "str";
	}
	else
	if (m_Linux)
	{
		m_Rexe = "wxrc";
		EXT_WXS = "wxs" ; m_Exttmp = "str" ;
	}
	else
	if (m_Mac) // TO VERIFY...
	{
		m_Rexe = "wxrc";
		EXT_WXS = "wxs" ; m_Exttmp = "str" ;//  ???
	}
//
	initTabResXml();

_printD("	<= End 'CreateForWx::initWx()'" );
}

///-----------------------------------------------------------------------------
//
// Called by :
//     1. CreateForWx::initTailored(const wxString _type):1,
//
///-----------------------------------------------------------------------------

void  CreateForWx::initTabResXml()
{
//
	m_TabMarkersRes = GetArrayFromString(m_marksRes, ' ');
// add :
	//m_marksXml += "<p> <i> " ;
	m_TabMarkersXml = GetArrayFromString(m_marksXml, ' ');
}

///-----------------------------------------------------------------------------
// Common init to 'List' and 'Extract': return 'true' on right
//
// Called by :
//		1. Pre::Init():1,
// Call	to :
//		1. CreateForWx::searchExe():1,
///-----------------------------------------------------------------------------
bool CreateForWx::initTailored(const wxString /*_type*/)
{
_printD("=> Begin 'CreateForWx::initTailored()'");

    bool ok = ! m_Wxpath.IsEmpty();
//8-  is project wxWidgets ?
    if (ok)
    {
    //9- init 'm_Rexe'
        initWx();

    //10- find executable paths  ('poedit', 'wxrc', 'xgettext', 'msmerge')
        bool ok = searchExe();
        if (ok)
        {
        //11- array clean
            m_NameprjWS.Clear();
          //  m_FileswithI18nWS.Clear();
            m_PrjwithI18nWS.Clear();
            m_Fileswithstrings.Clear();
        //12- display Wx
            // changed directory
            wxFileName::SetCwd(m_Dirproject);
            // messages
            Mes = Lf +  LineW;
            _print(Mes);
            Mes = _("Begin") + quote(m_Thename + "-" + VERSION_COLLECT) + ":";
            _print(Mes);
            m_Fileswithstrings.Add(Mes);

            // Wx path
            Mes = Tab + _("Project use 'Wx' path") + " : " +  quote(m_Wxpath);
            _print(Mes);
            m_Fileswithstrings.Add(Mes);
            Mes = Tab + _("Elegibles files") + " (*.h, *.cpp, *.script, *.xml, (*.xrc, *.wxs -> *_???.strNNN))";
            _print(Mes);
            m_Fileswithstrings.Add(Mes);
        }
        else
        {
            // TODO ...
            Mes = Tab + _("The search for executables failed") + " !!" ;
            _printError(Mes);

            return false;
        }
    }
    else
    {
		Mes = Tab + _("The library path is empty") + " !!" ;
        _printError(Mes);

        return false;
    }

_printD("	<= End 'CreateForWx::initTailored()' => " + strBool(ok) );

	return ok;
}

// ----------------------------------------------------------------------------
//
// Called by :
//		1. CreateForWx::initTailored():1,
//
// Call to :
//		1. Pre::findPathfromconf(wxString _pexe):1
//
bool CreateForWx::searchExe()
{
_printD("=> Begin 'CreateForWx::searchExe()'");

	bool ok = false;
	// ...
	wxUniChar excla  = '!';
	wxString tbegin = wxString(excla, 60) + Lf
		     ,tfin = wxString(excla, 60) + Lf
		     ,exe
		     ,newpath
		     ;
	m_Pexeishere = m_Gexeishere = m_Mexeishere = false;
	m_Pathpexe = m_Pathgexe  = m_Pathmexe = m_PathIts = wxEmptyString;
//7-1 search path 'poedit'
	// Win-7 (who contains 'xgettext' and 'msmerge')
	// Linux /usr/bin/xgettext
	wxString pathpoedit = Pre::findPathfromconf(m_Pexe);
	if (pathpoedit.IsEmpty())
	{
		Mes = "!! " + _("Could not query the executable") + " 'Poedit' !!" + Lf;
		Mes += "!! " + _("You can't translate extracted string") + " !!" + Lf;
		Mes = tbegin + Mes + tfin;
		ShowError(Mes);
		_printError(Mes);

	// search another place for 'xgettext' and 'msmerge'
		// 'LocateDataFile' give complete 'path + nameExe'
		// 1- search 'xgettext'
		m_Pathgexe = m_pMconf->LocateDataFile(m_Gexe, sdAllKnown);
		if (! m_Pathgexe.IsEmpty() )
		{
			exe = UnixFilename(m_Pathgexe, wxPATH_NATIVE);
			m_Gexeishere = wxFileName::FileExists(exe);
		}
		// 2- search 'its'
		if (m_Gexeishere)
		{
		/// Win-7
		// 'its' rules "path-poedit\Poeditxxx\GettextTools\share\gettext\its\glade1.its"
			newpath = "GettextTools\\share\\gettext\\its\\";
			exe.Replace(m_Gexe, newpath, FIRST_TXT);
			m_PathIts = UnixFilename(exe, wxPATH_NATIVE);
			// directory in project
			m_PathLocalIts = UnixFilename(".\\its\\", wxPATH_NATIVE);
        /// Linux
        //  'its' rules "/usr/share/gettext/its/glade.its"

		}
		else
		{
			// no path 'its' ??
		}
		// 3- search 'msmerge'
		m_Pathmexe = m_pMconf->LocateDataFile(m_Mexe, sdAllKnown);
		if (! m_Pathmexe.IsEmpty() )
		{
			exe = UnixFilename(m_Pathmexe, wxPATH_NATIVE);
			m_Mexeishere = wxFileName::FileExists(exe);
		}
	}
// 'Poedit' was found  !
	else
	{
		m_Pathpexe = UnixFilename(pathpoedit, wxPATH_NATIVE);
		m_Pexeishere = true;
// search 'xgettext' and 'merge' and path 'its'
	// old path : "path-poedit\Poeditxxx\Gexe"
		// 'xgettext'
		m_Pathgexe = m_Pathpexe.Mid(0, m_Pathpexe.Len());
		m_Pathgexe.Replace(m_Pexe, m_Gexe, FIRST_TXT);
		//exe = UnixFilename(m_Pathgexe, wxPATH_NATIVE);
		m_Gexeishere = wxFileName::FileExists(m_Pathgexe);
		// 'msmerge'
		m_Pathmexe = m_Pathpexe.Mid(0, m_Pathpexe.Len());
		m_Pathmexe.Replace(m_Pexe, m_Mexe, FIRST_TXT);
		//exe = UnixFilename(m_Pathmexe, ::wxPATH_NATIVE);
		m_Mexeishere = wxFileName::FileExists(m_Pathmexe);
	// new path : "path-poedit\Poeditxxx\GettextTools\bin\Gexe"
		if (! m_Gexeishere )
		{
		// 'xgettext'
			newpath = "GettextTools\\bin\\" + m_Gexe;
			m_Pathgexe.Replace(m_Gexe, newpath, FIRST_TXT);
			//exe = UnixFilename(m_Pathgexe, wxPATH_NATIVE);
			m_Gexeishere = wxFileName::FileExists(m_Pathgexe);
			if (m_Gexeishere)
			{
			// 'its' rules "path-poedit\Poeditxxx\GettextTools\share\gettext\its\glade1.its"
				newpath = "GettextTools\\share\\gettext\\its\\";
				exe = pathpoedit;
				exe.Replace(m_Pexe, newpath, FIRST_TXT);
				m_PathIts = UnixFilename(exe, wxPATH_NATIVE);
			// directory in project
				m_PathLocalIts = UnixFilename(".\\its\\", wxPATH_NATIVE);
			}
			else
			{
				// no path 'its' ??
			}
		}
		if (! m_Mexeishere)
		{
		// 'msmerge'
			m_Pathmexe = m_Pathgexe.Mid(0, m_Pathgexe.Len());
			m_Pathmexe.Replace(m_Gexe, m_Mexe, FIRST_TXT);
		//exe = UnixFilename(m_Pathmexe, wxPATH_NATIVE);
			m_Mexeishere = wxFileName::FileExists(m_Pathmexe);
		}
	}
	// 'xgettext'
	if (m_Gexeishere)
	{
	// uses "X:\path_gexe"
		m_Pathgexe = dquoteNS(m_Pathgexe);
	// 'its' rules "path-poedit\Poeditxxx\GettextTools\share\gettext\its\glade1.its"
		if (m_Win)
		{
			newpath = "GettextTools\\share\\gettext\\its\\";
			exe = pathpoedit;
			exe.Replace(m_Pexe, newpath, FIRST_TXT);
			m_PathIts = UnixFilename(exe, wxPATH_NATIVE);
		// directory in project
			m_PathLocalIts = UnixFilename(".\\its\\", wxPATH_NATIVE);
		}
		else
		if (m_Linux)
		{
			newpath = "/usr/share/gettext/its/";
			m_PathIts = UnixFilename(newpath, wxPATH_NATIVE);
			m_PathLocalIts = UnixFilename("./its/", wxPATH_NATIVE);
		}
//_printWarn("m_PathLocalIts : " + m_PathLocalIts);
//_printWarn("m_PathIts : " + m_PathIts);
	}
	else
	{
		Mes = "!! " + _("Could not query the executable 'xgettext'") + " !!" + Lf;
		Mes += "!! m_Pathgexe = " + dquote(m_Pathgexe) + Lf;
		Mes += "!! " + _("Strings to be translated will not be extracted.") + Lf;
		ShowError(Mes);
		Mes = tbegin + Mes + tfin;
		_printError(Mes);

		return false;
	}
// 'msmerge'
	if (m_Mexeishere)
	{
	// add  double quote
		m_Pathmexe = dquoteNS(m_Pathmexe);
	}
	else
	{
		Mes = "!! " + _("Could not query the executable 'msmerge'") + " !!" + Lf;
		Mes += "!! m_Pathmexe = " + dquote(m_Pathmexe) + Lf;
		Mes += "!! " + _("We can not merge to old '*.po'") + " !!" + Lf;
		ShowError(Mes);
		Mes = tbegin + Mes + tfin;
		_printError(Mes);

		return false;
	}

//7-2 find 'wxrc'
	m_Rexeishere = false;
	//1- into '$(#cb_exe)\wxrc' or into PATH system
	m_Pathrexe = m_pMconf->LocateDataFile(m_Rexe , sdAllKnown);
	if (! m_Pathrexe.IsEmpty())
	{
		exe = UnixFilename(m_Pathrexe, wxPATH_NATIVE);
		m_Rexeishere = wxFileName::FileExists(exe);
	}
	if (! m_Rexeishere)
	{
	//2- into 'Poedit' ?
		m_Pathrexe = m_Pathpexe.Mid(0, m_Pathpexe.Len());
		// 1-1 old path : "path-poedit\Poeditxxx\Rexe"
		m_Pathrexe.Replace(m_Pexe, m_Rexe, FIRST_TXT);
		//exe = UnixFilename(m_Pathrexe, wxPATH_NATIVE);
		m_Rexeishere = wxFileName::FileExists(m_Pathrexe);
		if (!m_Rexeishere)
		{
		// 2-2- new path : "path-poedit\Poeditxxx\GettextTools\bin\Rexe"
			newpath = "GettextTools\\bin\\" + m_Rexe;
			m_Pathrexe.Replace(m_Rexe, newpath, false);
			exe = UnixFilename(m_Pathrexe, wxPATH_NATIVE);
			m_Rexeishere = wxFileName::FileExists(exe);
		}
	//3- into '$(#wx)\utils\wxrc\mswu\wxrc
		if (!m_Rexeishere)
		{
		// find '$(#wx)\include\wx\version.h', search '#define wxMINOR_VERSION 3'
			m_Wxpath += strSlash;
			wxString pathversion_h  = m_Wxpath + "include" + strSlash;
			pathversion_h += "wx" + strSlash  + "version.h";
			pathversion_h = UnixFilename(pathversion_h, wxPATH_NATIVE);
			bool ok = wxFileName::FileExists(pathversion_h);
			if (ok)
			{
				wxString min_ver = "3"
						,source =  Pre::readFileContents(pathversion_h)
						;
				if (!source.IsEmpty())
				{
					wxInt32 pos = source.Find("wxMINOR_VERSION");
					if (pos != wxNOT_FOUND)
					{
						wxString line = source.Mid(pos, 30).BeforeFirst(Lf.GetChar(0));
						ok = line.Find(min_ver) != wxNOT_FOUND;
						if (ok)
						{
			// '$(#wx)\utils\wxrc\mswu\wxrc
							m_Pathrexe = m_Wxpath + "utils" + strSlash + "wxrc";
							m_Pathrexe += strSlash + "mswu" + strSlash + m_Rexe;
							exe = UnixFilename(m_Pathrexe, wxPATH_NATIVE);
							m_Rexeishere = wxFileName::FileExists(exe);
						}
					}
				}
			}
		}
	}
// not found. ouf !!
	if (m_Rexeishere)
	{
		m_Pathrexe = dquoteNS(m_Pathrexe);
	}
	else
	{
		Mes = "!! " + _("Could not query the executable 'wxrc'") + " !!"+ Lf;
		Mes += "!! m_Pathrexe = " + dquote(m_Pathrexe) + Lf;
		Mes += "!!" + _("Files '*.xrc, *.wxs' will not be taken into account.") + " !!" + Lf;
		ShowError(Mes);
		Mes = tbegin + Mes + tfin;
		_printError(Mes);

		return false;
	}

// local ok= Rexeishere && Gexeishere
	ok = m_Gexeishere;
	if (!ok)
	{
		wxInt32 choice;
		wxString title = _("WARNING");
		Mes = wxEmptyString;
		if( !m_Gexeishere)
			Mes += quote(m_Gexe) +  _("is absent") + " !!" + Lf;
		Mes += _("Do you want continue anywhere") + " ?";
		choice = cbMessageBox(Mes, title, wxICON_QUESTION | wxYES_NO);
		ok = choice == wxID_YES;
	}

_printD("	<= End 'CreateForWx::searchExe()'");

	return ok;
}

// ----------------------------------------------------------------------------
//	nbstr : number strings, if  < 0 =>  error
//
// Called by
//      1. Pre::findGoodfiles():1,
//		2. CreateForWx::listStringsXml(const wxString& _shortfile):1,
//
// Call to :
//    1. Pre::readFileContents(const wxString& _filename):1,
//
// ----------------------------------------------------------------------------
wxInt32 CreateForWx::isReadableFile(const wxString& _file, const bool _absolute)
{
_printD("=> Begin 'CreateForWx::isReadableFile(" + _file + ", " + strBool(_absolute) + ")'");

	bool ok = true;
	wxInt32 nbStr = 0;
//1- verify exist
	wxString namefile ;
	// absolute or not
	if (_absolute)	namefile = _file ;
	else 			namefile = m_Dirproject + _file;
	ok = wxFileName::FileExists(namefile) ;
	if (! ok )
	{
        wxString mes;
		Mes = Lf + "*** " + _("in function 'CreateForWx::isReadableFile'") + " : ";
		Mes += Lf + _("file") + quote(_file) + _("NOT FOUND") + " !!!";
		_printError(Mes);
		mes = Lf + _("Check for the presence of this file, or remove it from the project");
		_printWarn(mes);
		m_Fileswithstrings.Add(Mes + mes);
		// dialogue
		wxInt32 choice = wxID_YES;
		Mes = quote(namefile) + Lf;
		Mes += Lf + _("Do you want continue ?");
		// ask user
		choice = cbMessageBox(Mes, "!! " + _("FILE NOT FOUND") + " !!", wxICON_QUESTION | wxYES_NO);
		m_Stop = (choice == wxID_NO);

       return wxNOT_FOUND ;
    }

    if (ok)
    {
    //2- read file
        wxString source = Pre::readFileContents(namefile);
        // file length
        wxUint32 lenfile = source.Len();
        ok = lenfile > 0;
        if (! ok )
        {
            Mes =  "*** " + _("in function 'CreateForWx::isReadableFile'") + " : " ;
            Mes +=  Lf + _("file") + quote(namefile) + _("is empty") + " !!!" + Lf;
            _printError(Mes);
            return 0;
        }
        else
		{
//_printWarn(quote(_file));
		// search once mark
			wxString ext, mark ;
			ext = _file.AfterLast('.') ;
			wxUint16 nb;
//_printWarn("search marks :" + quote(ext)) ;
		// 'xml'
			if ( ext == "xml")
			{
			// table contains all possible marks
				nb = 0;
				wxUint32 nbCells = m_TabMarkersXml.Count();
				for (wxUint16 pos =0; pos < nbCells; pos++)
				{
					mark = m_TabMarkersXml.Item(pos);
					nb = Pre::freqStr(source, mark) ;
					nbStr += nb;
        //_printError(Tab + quote(mark) + "=> " + strInt(nb));
				}
			}
			else
		// 'res'
			if ( ext == "wks" || ext == "xrc")
			{
			// table contains all possible marks
				wxUint32 nbCells = m_TabMarkersRes.Count();
				for (wxUint16 pos =0; pos < nbCells; pos++)
				{
					mark = m_TabMarkersRes.Item(pos);
					nb = Pre::freqStr(source, mark);
					nbStr += nb;
        //_printError(Tab + quote(mark) + "=> " + strInt(nb));
				}
			}
		// 'code'
			else
			{
				mark = "_(";
			// search 'mark'
				nbStr += Pre::freqStr(source, mark);
			}
		//_printError("nbStr = " + strInt(nbStr));
		}
    }

_printD("	<= End 'CreateForWx::isReadableFile(...)' => " + strInt(nbStr) );

	return nbStr;
}
// ----------------------------------------------------------------------------
//
// Called by :
//		1. Pre::listGoodfiles(bool _verify):1,
// Call to :
//		1. Pre::GexewithError(wxString, wxString, bool):1,
// ----------------------------------------------------------------------------
wxInt32 CreateForWx::listStringsCode(const wxString& _shortfile)
{
_printD("=> Begin 'CreateForWx::listStringsCode(" + _shortfile + ")'");

	wxString longfile = _shortfile;
//1- absolute name inputfile
	if (m_Workspace) 	longfile.Prepend(m_Dirproject);
//2- build command : 'xgettext -C -k_ infile -o- ....'  => console
	wxString key = "-k" + m_Keyword;
	wxString command = m_Pathgexe + " --c++ --from-code=UTF-8 " + key;
	// inputfile
	command += dquote(longfile) ;
	// output  to 'stdout'
	command += " -o-";
	// without header to out
	command += " --omit-header";
	// text indent
	//command += " -i";
	// files sorted
	//command += " -F";
	// exclusif !
	//command += " -s" ;
	// text width before new line
	command += " -w 180";

//3- command execute with errors
	// list string into
	wxInt32 nbstr = Pre::GexewithError(_shortfile, command, PREPEND);

_printD("	<= End 'CreateForWx::listStringsCode(...) => " + strInt(nbstr) + ")'");

	return nbstr;
}

// ----------------------------------------------------------------------------
//
// Called by :
//		1. Pre::listGoodfiles(bool _verify):1,
//
// Call to :
//		1. CreateForWx::expandName(wxString _file, wxInt32 _indexfree):1,
//		1. CreateForWx::RexewithError(wxString, wxString, bool):1,
// ----------------------------------------------------------------------------
wxInt32 CreateForWx::listStringsRes(wxString& _file)
{
_printD("=> Begin 'CreateForWx::listStringsRes(" + quoteNS(_file) + ")'");

	if (m_Pathrexe.IsEmpty() )	return wxNOT_FOUND;
//1- path name output file
	// => 'xxxx_wxs.str???' or 'xxxx_wrc.str???' or ...
	// index free
	wxUint32 indexfree =  m_FileStrcreated.Count();
	// new name for output
	wxString filemod = expandName(_file, indexfree);
	m_nameFileTemp = m_Temp + filemod.AfterLast(slash);
	if (m_Workspace)
	{
	// change 'm_DirProject' ->  'm_DirprojectLeader'
		m_nameFileTemp.Prepend(m_DirprojectLeader);
	}

//2- build command for 'wxrc'
/*
** Win
	wxrc [/h] [/v] [/e] [/c] [/p] [/g] [/n <str>] [/o <str>] [--validate]
			[--validate-only] [--xrc-schema <str>] input file(s)..
	/h, --help            show help message
	/v, --verbose         be verbose
	/e, --extra-cpp-code  output C++ header file with XRC derived classes
	/c, --cpp-code        output C++ source rather than .rsc file
	/p, --python-code     output wxPython source rather than .rsc file
	/g, --gettext         output list of translatable strings (to stdout or file if -o used)
	/n, --function=<str>  C++/Python function name (with -c or -p) [InitXmlResource]
	/o, --output=<str>    output file [resource.xrs/cpp]
	--validate            check XRC correctness (in addition to other processing)
	--validate-only       check XRC correctness and do nothing else
	--xrc-schema=<str>    RELAX NG schema file to validate against (optional)
** Linux
Usage: wxrc [-h] [-v] [-e] [-c] [-p] [-g] [-n <str>] [-o <str>] [--validate]
			[--validate-only] [--xrc-schema <str>] input file(s)...
  -h, --help            show help message
  -v, --verbose         be verbose
  -e, --extra-cpp-code  output C++ header file with XRC derived classes
  -c, --cpp-code        output C++ source rather than .rsc file
  -p, --python-code     output wxPython source rather than .rsc file
  -g, --gettext         output list of translatable strings (to stdout or file if -o used)
  -n, --function=<str>  C++/Python function name (with -c or -p) [InitXmlResource]
  -o, --output=<str>    output file [resource.xrs/cpp]
  --validate            check XRC correctness (in addition to other processing)
  --validate-only       check XRC correctness and do nothing else
  --xrc-schema=<str>    RELAX NG schema file to validate against (optional)
*/
	wxString command = m_Pathrexe;
	command += " -v" ;
	command += " -g" ;
	//command += "--validate";
	command += " -o" + dquote(m_nameFileTemp);
	command += dquote(_file);

//3- command execute with errors
	wxInt32 nbstr = RexewithError(_file, command);

//4- copy of temporary file : 'temp/dummy_xrc.strxxx' -> 'temp/dummy.xrc'
	if (nbstr)
	{
		bool ok = copyRes(m_nameFileTemp);
		if (!ok)
		{
			Mes = _("The copy of the") + quote(m_nameFileTemp) + _("file failed");
			_printError(Mes);
		}
	}

_printD("	<= End 'CreateForWx::listStringsRes(...) => " + strInt(nbstr) + ")'");

	return nbstr;
}

// ----------------------------------------------------------------------------
// Display formatting for resources
// Called by :
//		1. CreateForWx::listStringsRes(wxString _file):2,
//
// Call to :
//		1. Pre::executeAndGetOutputAndError(const wxString& _command,
//											const bool& _prepend_error);1,
//		2. Pre::readFileContents(const wxString& _filename):1,
//		3. CreateForWx::wxrcOut(const wxString _txt):1,
//
wxInt32 CreateForWx::RexewithError(const wxString& _shortfile, const wxString& _command,
									const bool& _prepend)
{
_printD("=> Begin 'CreateForWx::RexeWithError(...)'");

//1- execute command line who creates 'xxx_xrc.strnnn' file on disk
	wxString result;
	if(_prepend) result = Pre::executeAndGetOutputAndError(_command, PREPEND_ERROR);
	else	     result = Pre::executeAndGetOutputAndError(_command, NO_PREPEND_ERROR);

	if (result.IsEmpty()) 	return -2;	// ??

//2- if '-v' result error starts with  = "10:16:18: Error: ..\n"
	if (result.Find(_("Error:")) != wxNOT_FOUND)
	{
		_printError("result: " + quoteNS(result)  );

		return wxNOT_FOUND;
	}
//3- if '-v' result starts with  = "processing name.xrc...\n"
	wxString mark =  "...\n";
	// no result withless "-v"
	if (result.EndsWith(mark))
	{
	// result withless 'mark' and withless  "processing "
		result.Replace("processing ", wxEmptyString);
		result.Replace(mark, wxEmptyString);
	}

//4- print 'm_nameFileTemp' into 'Fileswithstrings'
	wxInt32 nbstr = 0;
	// read source 'xxx_xrc.strnnn' => 'm_nameFileTemp'
	wxString source = Pre::readFileContents(m_nameFileTemp);
	if (!source.IsEmpty())
	{
		wxArrayString tabmes = GetArrayFromString(source, Lf, true);
		wxInt32 nbstrfile = tabmes.GetCount();
		if (nbstrfile <= 1)
		{
			Mes = "*** " + _("Wrong 'Eol'") + " !! " + _("no analyze, adjust file ");
			Mes += quote(m_nameFileTemp);
			m_Fileswithstrings.Add(Mes);
			return wxNOT_FOUND;
		}

	// give the all string(s) finded with separator 'Lf'
		// if error 'result = "" and m_Strwitherror = true
		if (m_Strwitherror)
		{
			Mes =  "** " + _("SYNTAX error") + " : ";
			Mes += "==> " + _(result) + "<==";
			_print(Mes);
			m_Fileswithstrings.Add(Mes);
			return wxNOT_FOUND;
		}
	// good result
		m_goodListing = true;

//5- analysis and formatting
	// memorize strings count
		wxUint32 nbstrBefore;
		if (m_goodListing)      nbstrBefore = m_nbListStrings;
		else                	nbstrBefore = m_nbExtractStrings;
	// contains all strings
		wxString mes = CreateForWx::wxrcOut(source);

	// numbers strings
		if (m_goodListing)      nbstr = m_nbListStrings - nbstrBefore;
		else                	nbstr = m_nbExtractStrings - nbstrBefore;
	// header
		++m_nbFileEligible;
		_printLn;
		wxString header = Tab + "E"+ strIntZ(m_nbFileEligible, 4) + "-";
		header += quote(_shortfile) + " ==> " + strInt(nbstr) + Space + _("string(s)");
		_printWarn(header);
		m_Fileswithstrings.Add(header);
	// list numbers line
		m_Fileswithstrings.Add(mes);
	// display 'mes' line by line
		tabmes = GetArrayFromString(mes, Lf, true);
		nbstrfile = tabmes.GetCount();
		for (wxInt32 n = 0 ; n < nbstrfile; n++)
		{
        // uses 'Collector::MesToLog(...)'
			print(Tab + "   " + tabmes.Item(n));
		}
		Mes = Tab + "  -> " + _("creates temporary") + quote(m_nameFileTemp);
		_printWarn(Mes);
		m_Fileswithstrings.Add(Mes);
	// memorize temporary file -> path
		m_FileStrcreated.Add(m_nameFileTemp);
	}
	else
    {
        nbstr = 0;
    }

_printD("	<= End 'CreateForWx::RexeWithError(...)'");

	return nbstr;
}

// ----------------------------------------------------------------------------
//
// Called by :
//		1. Pre::listGoodfiles(bool _verify):1,
// Call to :
//		1. CreateForWx::listStringsXml(const wxString& _shortfile, wxString _rulesIts):1,
// ----------------------------------------------------------------------------
wxInt32 CreateForWx::listStringsXml(const wxString& _shortfile)
{
_printD("=> Begin 'CreateForWx::listStringsXml(" + _shortfile + ")'");

	wxString longfile = _shortfile;
//1- absolute name inputfile
	if (m_Workspace) 	longfile.Prepend(m_Dirproject);
//2- rules file 'its'
	wxString rules = "";
	//rules = "appdata.its";
	//rules = "docbook.its";
	//rules = "docbook5.its";
	//rules = "glade1.its";
	//rules = "glade2.its";
	//rules = "gsettings.its";
	//rules = "gschema.its";
	//rules = "gtkbuilder.its";
	//rules = "lexerCB.its";
	//rules = "mallard.its";
	//rules = "metainfo.its";
	//rules = "cbmanifest.its";
   /*
    // choice of '*.its'
    if (_shortfile.Contains("metainfo") )   rules = "metainfo.its";
    else
    if (_shortfile.Contains("appdata") )    rules = "appdata.its";
    else
    if (_shortfile.Contains("manifest") )   rules = "manifest.its";
    else
    if (_shortfile.Contains("glade") )      rules = "glade1.its";
    else
    if (_shortfile.Contains("lexer") )      rules = "cblexer.its";
    else                                    rules = "mallard.its";
  */
    rules = "codeblocks.its";

// you have to check if the 'rulesIts' file is readable
	m_PathRulesIts = m_PathIts + rules;
	// local path
	//wxString pathits = m_PathLocalIts + rules;

//3- call 'lisStringXmlIts(...)' with rules
	wxInt32 nbstr = listStringsXmlIts(longfile, m_PathRulesIts);

_printD("	<= End 'CreateForWx::listStringsXml(...) => " + strInt(nbstr) + ")'");

	return nbstr;
}
// ----------------------------------------------------------------------------
//
// Called by :
//		1. CreateForWx::listStringsXml(const wxString& _shortfile):1,
// Call to :
//		1. Pre::GexewithError(wxString, wxString, bool):1,
// ----------------------------------------------------------------------------
wxInt32 CreateForWx::listStringsXmlIts(const wxString& _shortfile, wxString _pathrulesIts)
{
_printD("=> Begin 'CreateForWx::listStringsXmlIts(" + _shortfile + ", " + _pathrulesIts + ")'");

	wxString longfile = _shortfile;
//1- build command : 'xgettext ...'
	// for 'xgettext' and 'its' for *.xml
	wxString its = " --its=" + dquoteNS(_pathrulesIts);
	wxString command = m_Pathgexe;
	// codage UTF-8
	command += " --from-code=UTF-8 ";
	// inputfile
	command += dquote(longfile) ;
	// output  to 'stdout'
	command += " -o-";
	// no header
	command += " --omit-header";
	// text indent
	command += " -i";
	// files sorted
	command += " -F";
	// sorted
	//command += " -s" ;
	// text width before new line
	command += " -w 320 ";
	// + 'itstool' comment
	command += "--itstool ";
	// path absolute 'its' file
	command += its;
//_printWarn(quote(command));
//2- command execute with errors
	// list string into
	wxInt32 nbstr = Pre::GexewithError(_shortfile, command, PREPEND);
    if(!nbstr)
    {
        Mes = Lf + Tab + "!!!" + quote(_shortfile) + "has no translatable strings" + " !!!";
        _printError(Mes);
    }
// all
    Mes = Tab + "  ** " +  "Rules template used :" + quote(_pathrulesIts);
    _printWarn(Mes);

_printD("	<= End 'CreateForWx::listStringsXmlIts(...) => " + strInt(nbstr) + ")'");

	return nbstr;
}

// ----------------------------------------------------------------------------
// Called by :
//		1. Pre::extraction(bool _prjfirst, cbProject * _prj):1,
//Call to :
//		1. Pre::GexewithError(wxString _shortfile, wxString _command, bool _prepend):1,
//
bool CreateForWx::createPot(bool _prjfirst)
{
_printD("=> Begin createPot(" + strBool(_prjfirst) + ")" );

    if (m_Pathgexe.empty())
    {
        Mes = " !!! 'xgettext' " + _("not defined") + " !!!" ;
        _printError(Mes);
        return false;
    }

// local variables
	bool ok = false, good = false;
	wxString name, nameprj, shortfile, longfile, filein, pActualprj ;
	wxUint32 cells, index = 0;
	if(!m_Workspace) 	cells = m_FileswithI18n.GetCount()  ;
	else 				cells = m_FileswithI18nWS.GetCount()  ;

	Mes = Tab + "--> " + _("Extract of") + Space + strInt(cells) + Space;
	Mes += _("file(s) by 'xgettext'");
	_printWarn(Mes);
	m_Fileswithstrings.Add(Mes);

// globales variables
	m_DirlocaleLeader = m_DirprojectLeader + m_Dirlocale;
	Mes = Tab;
	if (m_Workspace)	Mes += _("Leader locale");
	else				Mes += _("Locale");
	Mes += Space + _("directory used") + " :" + quote(m_DirprojectLeader);
	_printWarn(Mes);
	// for 'xml'
    m_PathRulesIts = m_PathIts + "codeblocks.its";

//1- command common for 'xgettext'
	// keyword
	wxString key = "-k" + m_Keyword;
	// command  'xgettext'
	wxString command , begincommand;

//2- extract all elegible files from 'm_FileswithI18n' or 'm_FileswithI18nWS'
	bool code, res, xml ;
	wxString dirproject, tempres, ext ;
	wxUint32 nbprjWS = 0;  wxInt32 nbstring = 0;
	// range 'g_MaxProgress'
	BuildLogger::g_MaxProgress = cells ;
	BuildLogger::g_CurrentProgress = 0 ;

/// for debug
//_printError("CreatePot : g_MaxProgress = " + strInt(BuildLogger::g_MaxProgress) + " files");

    // read all files in  'm_FileswithI18n' or 'm_FileswithI18nWS'
	for (wxUint32 i=0; i < cells ; i++ )
	{
	// control to pending messages in the event loop
		 m_pM->Yield();
	// analysis stopped manually
		if (m_Stop)
		{
			good = false ;
			break;
		}
		// actualize
		if (_prjfirst == FIRST_PRJ)  _prjfirst = (i==0);

//2-1- One project only
		if (!m_Workspace)
		{
			name =  m_FileswithI18n.Item(i);
		// it's a path : always absolute path
			if (name.Last() == slash)
			{
			// project path
				dirproject = name;
				continue;
			}
		// its a file  : always relative path
			if (name.Last() != slash)
			{
			// file name : relative path
				shortfile = name;
				filein = shortfile;
			}
		}
//2-2 Workspace
		if (m_Workspace)
		{
			name =  m_FileswithI18nWS.Item(i);
		// it's a path : always absolute path
			if (name.Last() == slash)
			{
			// read name project
				nameprj = m_PrjwithI18nWS.Item(i);
			// project path
				dirproject = name;
				++nbprjWS;
				Mes = Lf + _("Project") + "-" + strIntZ(nbprjWS, 4) + " :" + quote(nameprj) ;
				Mes += Lf + Tab + _("path") + " :" + quote(dirproject);
				_printWarn(Mes);
				continue;
			}
		// it's a file : always relative path
			if (name.Last() != slash)
			{
				nameprj = m_PrjwithI18nWS.Item(i);
			// file name : relative path
				shortfile = name;
			// file name : absolute path
				longfile = dirproject + shortfile;
			// ressource *_xrc.strxxx' or '_wks.strxxx'
				if (shortfile.Contains(".str"))
					filein = shortfile;
				else
					filein = longfile;
			}
		}
	// file extension
		wxString ext = filein.AfterLast(cDot);
		code = ext.Matches(EXT_H) || ext.Matches(EXT_HPP) || ext.Matches(EXT_CPP)
			   || ext.Matches(EXT_CXX) || ext.Matches(EXT_SCR) ;
		res = ext.Matches(EXT_XRC) || ext.Matches(EXT_WXS);
		xml = ext.Matches(EXT_XML);
	// log message
		++index;
		Mes =  Tab + "F" + strIntZ(index, 4) + ":" + quote(shortfile);
	// it's necessary that file 'm_ShortNamepot' exists already
		if (_prjfirst)	Mes += " --> ";
		else			Mes += " -->>";
		Mes += quote(m_Shortnamepot);
		// uses 'Collector::MesToLog(...)'
		print (Mes);
		m_Fileswithstrings.Add(Mes);
	// replace '*.xrc' by 'm_Temp/*.xrc'  ...
		if (res)
		{
		  tempres = filein;
		  filein = tempres.AfterLast(slash).Prepend(m_Temp) ;
		}

//2-3- build command : xgettext  -n -k_ -o Namepot infile
        command = m_Pathgexe ;
		if (code || res )
		{
        // extract comments before the key == '--add-comments=//'
            begincommand = " -cTranslators: ";
		// language = C++, input file encoding = utf-8
			begincommand +=  " -C --from-code=UTF-8 " + key;
		}
		else
		if (xml)
		{
		// language = xml
			begincommand = " --its=" + dquoteNS(m_PathRulesIts);
		// add comment '#. xxxxxx" from position in tree xml
			begincommand += " --itstool ";
		}
	// inputfile
		command += begincommand + dquote(filein) ;
		// it's necessary that file 'm_Namepot' exists already
		if (!_prjfirst)
		{
		// --join-existing :  attach messages to join existing file
			command += " -j ";
		}
	// output file to '*.pot' locally
		command += " -o" + dquote(m_Dirlocale + m_Shortnamepot);

//2-4- command execute : nbstring == 0 it's good
		nbstring = Pre::GexewithError(filein, command, NO_PREPEND_ERROR);
		good = ok = nbstring == 0 ;
		if (!good)
		{
			Mes = "'CreateForWx::createPot() :" + _("Error from return of") ;
			Mes += " 'Pre::GexewithError(...)'" ;
			_printError(Mes);
			return  false;
		}

//2-5- filters ".", ":", "!", "..." ...
		// TODO

	} // end for

//3- delete all temporary files
	wxInt32 nbfiles = CreateForWx::Cleantemp();
	good = nbfiles >= 0 ;
	// no files to delete
	m_FileStrcreated.Clear();

//4- 'Stop'
	if (!good)
	{
		return false;
	}

_printD("	<= End 'CreateForWx::createPot(...)' => " + strBool(ok && good) );

	return ok && good ;
}

// ----------------------------------------------------------------------------
// Modify ressource or xml file name
//		=> 'xxxx_wxs.str???' or 'xxxx_wrc.str???' or ...
//		??? is an insertion index in 'm_FileStrCreated'
//
// Called by :
//		1. wxInt32 CreateForWx::listStringsRes(wxString& _file):1,
//
// Call to : none
// ----------------------------------------------------------------------------
wxString CreateForWx::expandName(wxString _file, wxInt32 _indexfree)
{
_printD("=> Begin 'CreateForWx::expandName(" + quote(_file) + ", " + strInt(_indexfree) + ")");
// _indexfree == 0  => name = 'm_NameProjectLeader'
	if (_indexfree == 0)
	{
	// already regitered ?
		_indexfree = m_FileStrcreated.Index(_file);
		if (_indexfree == wxNOT_FOUND)
			return wxEmptyString;
	}
// new extension
	wxString ext(".str");
	ext += strInt(_indexfree);
// only name file witless directory
	_file = _file.AfterLast(slash);
// old extension
	wxString oldext = _file.AfterLast(cDot);
	if (oldext.Matches(EXT_XRC) )
	{
		_file = _file.BeforeLast(cDot) + "_xrc" + ext;
    // to 'm_Temp'
		_file.Prepend(m_Temp);
	}
	else
	if (oldext.Matches(EXT_WXS) )
	{
		_file = _file.BeforeLast(cDot) + "_wxs" + ext;
    // to 'm_Temp'
		_file.Prepend(m_Temp);
	}

_printD("	<= End 'CreateForWx::expandName(" + _file + ")'");

	return  _file;
}
// ----------------------------------------------------------------------------
// Copy a temporary file to orginal
//	'xxxx_wxs.str???' => 'xxxx.wxs'
//	'xxxx_wrc.str???' => 'xxxx.xrc'
//
// Called by :
//		1.  CreateForWx::listStringsRes(wxString& _file):1,
//
// Call to : none
// -----------------------------------------------------------------------------
bool CreateForWx::copyRes(wxString _file)
{
	bool ok = true;
	wxString nameres, ext ;
	// find last '_'
	nameres = _file.BeforeLast('_');
	ext = _file.AfterLast('_').BeforeFirst(cDot);
	nameres += Dot + ext;
//1- copy '*_xrc.strxxx' => '*.xrc'
	ok = wxCopyFile(_file, nameres);
	if (ok)
	{
//2- delete all '\r' for 'xgettext'
		wxString stringsRes = readFileContents(nameres);
		//wxInt32 nb =
		stringsRes.Replace("\r", "", ALL_TXT) ;
	//printWarn("Number '\r' : " + strInt(nb));
		ok = cbSaveToFile(nameres, stringsRes);
	}

	return ok;
}

// ----------------------------------------------------------------------------
// Display formatting for resources  ( 'wxrc' executable )
// Called by :
//		1. CreateForWx::RexewithError(const wxString& _shortfile,
//									  const wxString& _command,
//									  const bool& _prepend)
// Call to : none
//
/// only displays in the log, not in the '*.list' file
//
wxString CreateForWx::wxrcOut(const wxString _txt)
{
_printD("=> Begin 'CreateForWx::wxrcOut(" + quoteNS(_txt) + ", ...)'");

// Global variable to prevent errors in resources
	m_Strwitherror = false;
	wxString tn = "\r\n",  result = wxEmptyString, mes = wxEmptyString;
	wxArrayString tabout = wxArrayString()
				 ,tabline = wxArrayString();
// error ?
	wxString line, tmp = wxEmptyString;
	wxInt32 pos = _txt.Find("Error:" );
	if (pos != wxNOT_FOUND )
	{
	// error parsing
		m_Strwitherror = true;
		tabout = GetArrayFromString(_txt, Lf, false);
    // first line if not verbose
		line = tabout.Item(0);
		line.Remove(0, pos);
		tmp = line.AfterFirst(' ');

		return "";
	}
// it's good
	// array from string
	tabout = GetArrayFromString(_txt, Lf, false);
	wxUint32 nbl = 0 ; pos = 0;
	Mes = wxEmptyString;
	// all array lines
	for (wxUint32 u=0; u < tabout.GetCount(); u++)
	{
		line = tabout.Item(u);
	// if verbose '-v'
		if (line.Matches("processing") ) 	continue;
	// new line '#'
		if (line.Find("#line ") != wxNOT_FOUND)
		{
		// extract line number between two spaces
			tmp = line.AfterFirst(cSpace).BeforeFirst(cSpace);
		// "1" => "00001" and "895" => "00985"
			strZ(tmp, 5);
			mes	=  Tab  + "  :L" + tmp;
		}
		else
	// line with 'Keyword' => translatable string
		if (line.Find(m_Keyword) != wxNOT_FOUND )
		{
		// find string between two quotation mark : ATTENTION _("\"no target\"") !
			tmp = line.AfterFirst('"').BeforeLast('"');

		// filter not tranlatables
			if ( tmp == "-" || tmp == "+" || tmp == "..."
				 || tmp == "*" || tmp == "\\" // || tmp = "\"\""
				 || tmp == "m_" || tmp == "ID:"
				 || tmp == "xxx" || tmp == "www" || tmp == "aaa" || tmp == "eee"
				 || tmp == "vvv"
				 || tmp.IsNumber() //|| tmp.Matches("?") // one numeric
				/// TO REVIEW
				 || tmp.StartsWith("http:")

			/// for C::B
			/// "GNU", "Linux", "Allman (ANSI)", "Java","K&&R", "Stroustrup", "VTK",
			///	"Horstmann", 'Ratliff", "Whitesmith", "1TBS", "Google", "Mozilla"
			///	"Pico", "Lisp", "GDB", "CDB", "AT&T", "Intel", "TCP", "UDP",
			/// "115200", ..., "192.168.0.1", all number
			/// 'wizard.xrc' : "atxxxxx", "mxxxx", "avrxxxx", ... "wxXXXX"
			/// "include", "lib", "obj", "cflags', "ldflags"
			/// "func", "addr',
			/// "vvv', "nnn", "aaa", "www",
			/// "CR", "LF", "CR LF" "AUTO"
			/// "Windows (CR && LF)", ">Mac (CR)", "ASCII (ISO-8859-1)",
			/// "ID:"
				)
			{
				mes += quote(tmp);
				mes += Tab + _("string not retained because not translatable") + " !!";
				mes += Lf;
			}
			else
			{
			// find text "\r\n" not Eol
				pos = tmp.Find(tn);
				if (pos != wxNOT_FOUND)
				{
					tmp.Replace("\\r\\n", Eol + Tab + Tab, ALL_TXT);
					tmp.Replace(tn, Eol + Tab + Tab, ALL_TXT);
					tabline = GetArrayFromString(tmp, Lf, false);
					for (wxUint32 k=0; k < tabline.GetCount(); k++)
					{
						mes += tabline.Item(k);
					}
				}
				else
				{
					mes += quote(tmp);
				}
			// memorize for late treating
				result.Append(mes + Lf );

				nbl++;
			}
		}
	}
// delete last Lf
	result.EndsWith(Lf, &result);
// total
	if (m_goodListing)		m_nbListStrings += nbl;
	else					m_nbExtractStrings += nbl;

_printD("	<= End 'CreateForWx::wxrcOut(...)'");

	return result;
}

// ----------------------------------------------------------------------------
// Delete all temporary file
//
// Called by :
//		1. Collector::OnMenuCleanTemp(wxCommandEvent& _pEvent):1,
//		2. CreateForWx::createPot(bool _prjfirst):1,
//
// Call to :
//		1.bool Pre::recursRmDir(wxString _rmDir):1
//
wxUint32 CreateForWx::Cleantemp()
{
_printD("=> Begin 'CreateForWx::Cleantemp()'");
	bool ok = false;
// messages
	Mes = Lf + "   --> " + _("begin 'Clean temporary files'");
	_printWarn(Mes);
	m_Fileswithstrings.Add(Mes);

//1- delete all temporary files of last project
	wxUint32 nfc =0;
	m_DirlocaleLeader = m_DirprojectLeader + m_Temp ;
	// récupérer le répertoire du projet pilote 'm_DirprojectLeader'
	if (!m_DirlocaleLeader.IsEmpty())
	{
		m_nbFilesdeleted = 0;
		ok = recursRmDir(m_DirlocaleLeader);
		if (ok)
		{
			nfc = m_nbFilesdeleted;
			if (nfc)
			{
				Mes =  Tab + "***" + quote(strInt(nfc));
				Mes += _("files have been deleted in the temporary directory.");
			}
			else
				Mes = Tab + _("No temporary files to delete.");

			_print(Mes);
			m_Fileswithstrings.Add(Mes);
		}
	}

//2- init
	m_FileStrcreated.Clear();

	// messages
	Mes =  "   <-- "  + _("end 'Clean temporary files'");
	_printWarn(Mes);
    m_Fileswithstrings.Add(Mes);

_printD("	<= End 'CreateForWx::Cleantemp()' => " + strInt(nfc)  );

	return nfc;
}

///-----------------------------------------------------------------------------
