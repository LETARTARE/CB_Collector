/***************************************************************
 * Name:      createforwx.cpp
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2020-05-10
 * Modified:  2022-12-11
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

#include "infowindow.h"     // InfoWindow::Display(...)
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
}

// -----------------------------------------------------------------------------
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
    m_Nameactivetarget = m_pProject->GetActiveBuildTarget() ;
    ProjectBuildTarget * pTarget =  m_pProject->GetBuildTarget(m_Nameactivetarget);
//_print("m_Nameactivetarget : " + quote(m_Nameactivetarget) ) ;
    m_isWxProject = false ;
// verify target type : 'virtual' or 'command'
    //====================================================
    if(Pre::isCommandOrVirtualTarget(m_Nameactivetarget) )
    //====================================================
    {
//_printError("it's a command or virtual target ...");
    // read all real targets
        wxUint16 nt = 0; m_isWxProject = false;
        // all targets
        m_nbTargets = m_pProject->GetBuildTargetsCount();
        while (nt < m_nbTargets && ! m_isWxProject )
        {
        // retrieve the target libraries
            pTarget = m_pProject->GetBuildTarget(nt++);
            if (!pTarget)       continue;
            //====================================================
            if (Pre::isCommandOrVirtualTarget(pTarget))  continue;
            //====================================================
//_print("other target : " + quote(pTarget->GetTitle()) ) ;
    // search all path libaries Wx
            //==============================
            m_isWxProject = pathWx(pTarget);
            //==============================
        }
    }
    else
    {
//_printError("it's a real target ...");
         //==============================
         m_isWxProject = pathWx(pTarget);
         //==============================
    }
//_printError(strBool(m_isWxProject));

// finded
    _report = _report && (m_State == fbWaitingForStart);
	if (m_isWxProject && _report)
	{
		wxString title = _("The project") +  quote(m_Nameproject) + _("uses 'Wx' libraries") + " !" ;
		wxString txt =  quote( m_namePlugin ) + _("can generate the translatable strings") + "...";
    // also to 'Code::Blocks' log
        InfoWindow::Display(title, txt, 5000);
	}

_printD("	<= End 'CreateForWx::detectLibProject()' => " + strBool(m_isWxProject) );

	return m_isWxProject;
}

// -----------------------------------------------------------------------------
// Allows to be called only with 'ProjectBuildTarget *' parameter
// which inherits from 'CompilerTargetBase' which inherits from 'CompileOptionsBase'
// The methods used are all methods of 'CompileOptionsBase'.
//
// Called by :
//     1. CreateForWx::detectLibProject(cbProject * _pProject, bool _report):2,
//
// Call to :
//		1. Pre::executeAndGetOutputAndError(const wxString& _command, bool _prepend_error) :1,
//
bool CreateForWx::pathWx(ProjectBuildTarget * _pTarget)
{
_printD("=> Begin 'CreateForWx::pathWx(...)'" );

	if(!_pTarget) return false;
	cbProject * pProject = _pTarget->GetParentProject();
    if(! pProject) return false;

    wxString nameTarget = _pTarget->GetTitle(), nameProject = pProject->GetTitle();
    m_Dirproject = pProject->GetBasePath();
//_printError(quote(nameTarget)) ;

// return true if good
	bool ok = false;
	wxString namevar = wxEmptyString;

// for 'Win'
	if (m_Win)
	{
	// include paths + librarie path
		wxString incdir = wxEmptyString;
        wxArrayString tabincdirs = pProject->GetIncludeDirs()
                     ,tablibsdirs = _pTarget->GetLibDirs();
        // add tabs to the first
        //=========================================================
        wxUint32 nincdir = Pre::addArrays(tabincdirs, tablibsdirs);
        //=========================================================
//_printError("nincdir = " + strInt(nincdir));
		if (nincdir)
		{
            wxString  t = "wx"	,t1 = "$(#" + t, t2 = "$(" + t, t3 = "$" + t;
			wxUint32 u =0;
			while (u < nincdir && !ok )
			{
				incdir = tabincdirs.Item(u++).Lower();
				ok = incdir.StartsWith(t1)
					|| incdir.StartsWith(t2)
					|| incdir.StartsWith(t3)
					|| incdir.StartsWith(t)
					;
			}
			if (ok)  	namevar = incdir.BeforeFirst(cSlash);
		}
    // libraries target + libraries project
		if (!ok)
		{
			wxArrayString tablinksdirs = _pTarget->GetLinkLibs()
                          , tablibs = pProject->GetLinkLibs();
            // add tabs free the second
            //==========================================================
            wxUint32 nlinksdirs = Pre::addArrays(tablinksdirs, tablibs);
            //==========================================================
			if (nlinksdirs)
			{
				wxString linksdir, t1 = "libwx", t2 = "wx";
				wxUint32 u=0;
				while (u < nlinksdirs && !ok )
				{
					linksdir = tablinksdirs.Item(u++).Lower();
					ok = linksdir.StartsWith(t1)
                        || linksdir.StartsWith(t2);
				}
				if (ok)  	namevar = linksdir.BeforeFirst(cSlash);
			}
		}
// inc or lib finded
		if (ok)
		{
//_printError("namevar =" + quote(namevar)) ;
        // replace macros
			m_pMam->ReplaceMacros(namevar, _pTarget);
			m_Wxpath = namevar;
//_printWarn("namevar =" + quote(namevar)) ;
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
            //==================================================================
            response = Pre::executeAndGetOutputAndError(nameopt, PREPEND_ERROR);
            //==================================================================
            // warning ...
            ok = ! response.Contains("Warning:");
            //ok = ! response.Contains(_("Warning:") );
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

_printD("	<= End 'CreateForWx::pathWx()' => " + strBool(ok) );

	return ok;
}

///-----------------------------------------------------------------------------
//
// Called by :
//     1. CreateForWx::initTailored(const wxString _type):1,
//
//
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
	if (m_Lin)
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
	initArrayBanned();

_printD("	<= End 'CreateForWx::initWx()'" );
}

///-----------------------------------------------------------------------------
//   Feed table for resource files and xml files and code
//
// Called by :
//     1. CreateForWx::initTailored(const wxString _type):1,
//
void  CreateForWx::initArrayBanned()
{
//1- tags xrc
	m_ArrayTagsRes = ::GetArrayFromString(m_tagsRes, cSpace, TRIM_SPACE);
	// sort
	m_ArrayTagsRes.Sort();
//	showArray(m_ArrayTagsRes, "m_ArrayTagsRes");

//2- tags xXml
	m_ArrayTagsXml = ::GetArrayFromString(m_tagsXml, cSpace, TRIM_SPACE);
	// sort
	m_ArrayTagsXml.Sort();
//	showArray(m_ArrayTagsXml, "m_ArrayTagsXml");

//3- banned strings
    m_ArrayBanned  = ::GetArrayFromString(m_bannedMarks, cSpace, TRIM_SPACE);
	m_ArrayBanned.Add("\\"); m_ArrayBanned.Add("\"\"");
	// sort
	m_ArrayBanned.Sort();
	m_sizeArrayBanned = m_ArrayBanned.GetCount();
//	showArray(m_ArrayBanned, "m_ArrayBanned");

//4- begin banned strings
	m_ArrayBeginBanned = ::GetArrayFromString(m_beginBannedMarks, cSpace, TRIM_SPACE);
	// sort
	m_ArrayBeginBanned.Sort();
	m_sizeArrayBeginBanned = m_ArrayBeginBanned.GetCount();
//	showArray(m_ArrayBeginBanned, "m_ArrayBeginBanned");


// add array
//	addArrays(m_ArrayBanned, m_ArrayBeginBanned);
//	m_sizeArrayBanned = m_ArrayBanned.GetCount();
//_printWarn("m_ArrayBanned => "+ iToStr(m_sizeArrayBanned));
}

///-----------------------------------------------------------------------------
// Common init to 'List' and 'Extract': return 'true' on right
//
// Called by :
//		1. Pre::Init():1,
// Call	to :
//		1. CreateForWx::searchExe():1,
//
bool CreateForWx::initTailored(const wxString /*_type*/)
{
_printD("=> Begin 'CreateForWx::initTailored()'");

    bool ok = ! m_Wxpath.IsEmpty();
//8-  is project wxWidgets ?
    if (ok)
    {
//9- init 'm_Rexe'
        //=======
        initWx();
        //=======

//10- find executable paths  ('poedit', 'wxrc', 'xgettext', 'msgmerge')
        //====================
        bool ok = searchExe();
        //====================
        if (ok)
        {
//11- array clean
            m_NameprjWS.Clear();
            m_PrjwithI18nWS.Clear();
            m_Fileswithstrings.Clear();
//12- display Wx
            // changed directory
            wxFileName::SetCwd(m_Dirproject);
            // messages
            Mes = Lf +  Line(140);
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

///-----------------------------------------------------------------------------
// Search all executable path
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
	wxUniChar excla = '!';
	wxString tbegin = wxString(excla, 60) + Lf
		     ,tfin  = wxString(excla, 60) + Lf
		     ,exe
		     ,newpath
		     ;
	m_Pexeishere = m_Gexeishere = m_Mexeishere = m_Uexeishere = false;
	m_PathPexe = m_PathGexe  = m_PathMexe = m_PathUexe = m_PathIts = wxEmptyString;
//7-1 search path 'poedit'
	// Win-7 (who contains 'xgettext' and 'msgmerge')
	// Linux /usr/bin/xgettext
	//==================================================
	wxString pathpoedit = Pre::findPathfromconf(m_Pexe);
	//==================================================
	if (pathpoedit.IsEmpty())
	{
		Mes = "!! " + _("Could not query the executable") + " 'EXTERN_TRANSLATOR' !!" + Lf;
		Mes += "!! " + _("You can't translate extracted string") + " !!" + Lf;
		Mes = tbegin + Mes + tfin;
		ShowError(Mes);
		_printError(Mes);

	// search another place for 'xgettext' and 'msgmerge'
		// 'LocateDataFile' give complete 'path + nameExe'
		// 1- search 'xgettext'
		m_PathGexe = m_pMconf->LocateDataFile(m_Gexe, sdAllKnown);
		if (! m_PathGexe.IsEmpty() )
		{
			exe = UnixFilename(m_PathGexe, wxPATH_NATIVE);
			m_Gexeishere = wxFileName::FileExists(exe);
		}
		// 2- search 'its'
		if (m_Gexeishere)
		{
		/// Win-7
		// 'its' rules "path-poedit\Poeditxxx\GettextTools\share\gettext\its\codeblocks.its"
			newpath = "GettextTools\\share\\gettext\\its\\";
			exe.Replace(m_Gexe, newpath, FIRST_TXT);
			m_PathIts = UnixFilename(exe, wxPATH_NATIVE);
			// directory in project
			m_PathLocalIts = UnixFilename(".\\its\\", wxPATH_NATIVE);
        /// Linux
        //  'its' rules "/usr/share/gettext/its/codeblocks.its"
		}
		else
		{
			// no path 'its' ??
		}
		// 3- search 'msgmerge'
		m_PathMexe = m_pMconf->LocateDataFile(m_Mexe, sdAllKnown);
		if (! m_PathMexe.IsEmpty() )
		{
			exe = UnixFilename(m_PathMexe, wxPATH_NATIVE);
			m_Mexeishere = wxFileName::FileExists(exe);
		}
		// 4- search 'msguniq'		m_Patuexe = m_pMconf->LocateDataFile(m_Uexe, sdAllKnown);
		if (! m_PathUexe.IsEmpty() )
		{
			exe = UnixFilename(m_PathUexe, wxPATH_NATIVE);
			m_Uexeishere = wxFileName::FileExists(exe);
		}

	}
// 'Poedit' was found  !
	else
	{
		//1- 'poedit'
		m_PathPexe = UnixFilename(pathpoedit, wxPATH_NATIVE);
		m_Pexeishere = true;
// search 'xgettext' and 'merge' and path 'its'
	// old path : "path-poedit\Poeditxxx\Gexe"
		//2- 'xgettext'
		m_PathGexe = m_PathPexe.Mid(0, m_PathPexe.Len());
		m_PathGexe.Replace(m_Pexe, m_Gexe, FIRST_TXT);
			//exe = UnixFilename(m_PathGexe, wxPATH_NATIVE);
		m_Gexeishere = wxFileName::FileExists(m_PathGexe);
		//3- 'msgmerge'
		m_PathMexe = m_PathPexe.Mid(0, m_PathPexe.Len());
		m_PathMexe.Replace(m_Pexe, m_Mexe, FIRST_TXT);
			//exe = UnixFilename(m_PathMexe, ::wxPATH_NATIVE);
		m_Mexeishere = wxFileName::FileExists(m_PathMexe);
		//4- 'msguniq'
		m_PathUexe = m_PathPexe.Mid(0, m_PathPexe.Len());
		m_PathUexe.Replace(m_Pexe, m_Uexe, FIRST_TXT);
			//exe = UnixFilename(m_PathUexe, ::wxPATH_NATIVE);
		m_Uexeishere = wxFileName::FileExists(m_PathUexe);
	// new path : "path-poedit\Poeditxxx\GettextTools\bin\Gexe"
		if (! m_Gexeishere )
		{
		// 'xgettext'
			newpath = "GettextTools\\bin\\" + m_Gexe;
			m_PathGexe.Replace(m_Gexe, newpath, FIRST_TXT);
			//exe = UnixFilename(m_PathGexe, wxPATH_NATIVE);
			m_Gexeishere = wxFileName::FileExists(m_PathGexe);
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
	// msgmerge
		if (! m_Mexeishere)
		{
		// 'msgmerge'
			m_PathMexe = m_PathGexe.Mid(0, m_PathGexe.Len());
			m_PathMexe.Replace(m_Gexe, m_Mexe, FIRST_TXT);
		//exe = UnixFilename(m_PathMexe, wxPATH_NATIVE);
			m_Mexeishere = wxFileName::FileExists(m_PathMexe);
		}
	// msguniq
		if (! m_Uexeishere)
		{
		// 'msguniq'
			m_PathUexe = m_PathGexe.Mid(0, m_PathGexe.Len());
			m_PathUexe.Replace(m_Gexe, m_Uexe, FIRST_TXT);
		//exe = UnixFilename(m_PathMexe, wxPATH_NATIVE);
			m_Uexeishere = wxFileName::FileExists(m_PathUexe);
		}
	}
	// 'xgettext'
	if (m_Gexeishere)
	{
	// uses "X:\path_gexe"
		m_PathGexe = dquoteNS(m_PathGexe);
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
		if (m_Lin)
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
		Mes += "!! m_PathGexe = " + dquote(m_PathGexe) + Lf;
		Mes += "!! " + _("Strings to be translated will not be extracted.") + Lf;
		ShowError(Mes);
		Mes = tbegin + Mes + tfin;
		_printError(Mes);

		return false;
	}
// 'msgmerge'
	if (m_Mexeishere)
	{
	// add  double quote
		m_PathMexe = dquoteNS(m_PathMexe);
	}
	else
	{
		Mes = "!! " + _("Could not query the executable 'msgmerge'") + " !!" + Lf;
		Mes += "!! m_PathMexe = " + dquote(m_PathMexe) + Lf;
		Mes += "!! " + _("We can not merge to old '*.po'") + " !!" + Lf;
		ShowError(Mes);
		Mes = tbegin + Mes + tfin;
		_printError(Mes);

		return false;
	}
// 'msguniq'
	if (m_Uexeishere)
	{
	// add  double quote
		m_PathUexe = dquoteNS(m_PathUexe);
	}
	else
	{
		Mes = "!! " + _("Could not query the executable 'msguniq'") + " !!" + Lf;
		Mes += "!! m_PathUexe = " + dquote(m_PathUexe) + Lf;
		Mes += "!! " + _("We can not unifie to old '*.po'") + " !!" + Lf;
		ShowError(Mes);
		Mes = tbegin + Mes + tfin;
		_printError(Mes);

		return false;
	}


//7-2 find 'wxrc'
	m_Rexeishere = false;
	//1- into '$(#cb_exe)\wxrc' or into PATH system
	m_PathRexe = m_pMconf->LocateDataFile(m_Rexe , sdAllKnown);
	if (! m_PathRexe.IsEmpty())
	{
		exe = UnixFilename(m_PathRexe, wxPATH_NATIVE);
		m_Rexeishere = wxFileName::FileExists(exe);
	}
	if (! m_Rexeishere)
	{
	//2- into 'Poedit' ?
		m_PathRexe = m_PathPexe.Mid(0, m_PathPexe.Len());
		// 1-1 old path : "path-poedit\Poeditxxx\Rexe"
		m_PathRexe.Replace(m_Pexe, m_Rexe, FIRST_TXT);
		//exe = UnixFilename(m_PathRexe, wxPATH_NATIVE);
		m_Rexeishere = wxFileName::FileExists(m_PathRexe);
		if (!m_Rexeishere)
		{
		// 2-2- new path : "path-poedit\Poeditxxx\GettextTools\bin\Rexe"
			newpath = "GettextTools\\bin\\" + m_Rexe;
			m_PathRexe.Replace(m_Rexe, newpath, false);
			exe = UnixFilename(m_PathRexe, wxPATH_NATIVE);
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
							m_PathRexe = m_Wxpath + "utils" + strSlash + "wxrc";
							m_PathRexe += strSlash + "mswu" + strSlash + m_Rexe;
							exe = UnixFilename(m_PathRexe, wxPATH_NATIVE);
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
		m_PathRexe = dquoteNS(m_PathRexe);
	}
	else
	{
	     Mes = "!! " + _("Could not query the executable 'wxrc'") + " !!"+ Lf;
	    if (m_Win)
        {
            Mes += "!! " + Lf;
            Mes += "!! " + _("COMPILE THE TARGET 'win_31x' which will copy 'wxrc' to the right place") + Lf;
        }
        else
        {
            Mes += "!! m_PathRexe = " + dquote(m_PathRexe) + Lf;
            Mes += "!!" + _("Files '*.xrc, *.wxs' will not be taken into account") ;
            Mes += Dot + " !!" + Lf;
        }

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

// -----------------------------------------------------------------------------
//	nbstr : number strings, if  < 0 =>  error
//
// Called by
//      1. Pre::findGoodfiles():1,
//		2. CreateForWx::listStringsXml(const wxString& _shortfile):1,
//
// Call to :
//    1. Pre::readFileContents(const wxString& _filename):1,
//
// -----------------------------------------------------------------------------
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
		mes = Lf + _("Check for the presence of this file, or remove it from the project") + Lf;
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
        //================================================
        wxString source = Pre::readFileContents(namefile);
        //================================================
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
		// search once tag
			wxString ext, tag ;
			ext = _file.AfterLast('.') ;
			wxUint16 nb;
//_printWarn("search marks :" + quote(ext)) ;
		// 'xml'
			if ( ext == EXT_XML)
			{
			// table contains all possible marks
				nb = 0;
				wxUint32 nbCells = m_ArrayTagsXml.GetCount();
				for (wxUint16 pos = 0; pos < nbCells; pos++)
				{
					tag = m_ArrayTagsXml.Item(pos);
					//==============================
					nb = Pre::freqStr(source, tag);
					//==============================
					nbStr += nb;
        //_printError(Tab + quote(tag) + "=> " + strInt(nb));
				}
			}
			else
		// 'res'
			if ( ext == EXT_XRC || ext == EXT_WXS)
			{
			// table contains all possible marks
				wxUint32 nbCells = m_ArrayTagsRes.GetCount();
				for (wxUint16 pos =0; pos < nbCells; pos++)
				{
					tag = m_ArrayTagsRes.Item(pos);
					//==============================
					nb = Pre::freqStr(source, tag);
					//==============================
					nbStr += nb;
        //_printError(Tab + quote(tag) + "=> " + strInt(nb));
				}
			}
		// 'code'
			else
			{
				tag = "_(";
			// search 'tag'
                //==================================
				nbStr += Pre::freqStr(source, tag);
				//==================================
			}
		//_printError("nbStr = " + strInt(nbStr));
		}
    }

_printD("	<= End 'CreateForWx::isReadableFile(...)' => " + strInt(nbStr) );

	return nbStr;
}
// -----------------------------------------------------------------------------
//
// Called by :
//		1. Pre::listGoodfiles(bool _verify):1,
// Call to :
//		1. Pre::GexewithError(wxString, wxString, bool):1,
// -----------------------------------------------------------------------------
wxInt32 CreateForWx::listStringsCode(const wxString& _shortfile)
{
_printD("=> Begin 'CreateForWx::listStringsCode(" + _shortfile + ")'");

	wxString longfile = _shortfile;
//1- absolute name inputfile
	if (m_Workspace) 	longfile.Prepend(m_Dirproject);
//2- build command : 'xgettext -C -k_ infile -o- ....'  => console
	wxString key = "-k" + m_Keyword;
	wxString command = m_PathGexe + " --c++ --from-code=UTF-8 " + key;
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
	// data sorted
	//command += " -s" ;
	// text width before new line
	//command += " -w 180";
	command += " -w 80";
	//command += " --no-wrap " ;


//3- command execute with error
	// list string into
	//=====================================================================
	wxInt32 nbstr = Pre::GexewithError(_shortfile, command, PREPEND_ERROR);
	//=====================================================================

_printD("	<= End 'CreateForWx::listStringsCode(...) => " + strInt(nbstr) + ")'");

	return nbstr;
}

// -----------------------------------------------------------------------------
//
// Called by :
//		1. Pre::listGoodfiles(bool _verify):1,
// Call to :
//		1. Pre::GexewithError(wxString, wxString, bool):1,
// -----------------------------------------------------------------------------
wxInt32 CreateForWx::listStringsRes(wxString& _shortfile)
{
_printD("=> Begin 'CreateForWx::listStringsRes(" + _shortfile + ")'");
/// extracts strings of '*.xrc' ou '*.wxs' => '*_xrc.str??' or '*_wxs.str??'
	//===========================================
	wxInt32 nbstr = extractStringsRes(_shortfile);
	//===========================================
	if (!nbstr)	return 0;
// 'm_nameFileTemp' contains the resource file name
//_printWarn("res =>" + quote(m_nameFileTemp));

	wxString longfile = m_nameFileTemp;
//1- absolute name inputfile
//	if (m_Workspace) 	longfile.Prepend(m_Dirproject);
//2- build command : 'xgettext -C -k_ infile -o- ....'  => console
	wxString key = " -k" + m_Keyword;
//_printError("key = " + quote(key) );
	wxString command = m_PathGexe + " --c++ --from-code=UTF-8" + key;
	// inputfile
	command += dquote(longfile) ;
	// output  to 'stdout'
	command += " -o-";
	// without header of the form 'msgid ""'
	command += " --omit-header";
	// text indent
	//command += " -i";
	// files sorted
	//command += " -F";
	// data sorted
	//command += " -s" ;
	// text width before new line
	//command += " -w 180";
	command += " -w 80";
	//command += " --no-wrap " ;

//_print("command :" + quote(command));
//3- command execute with error
	// list string into
	//=====================================================================
	nbstr = Pre::GexewithError(_shortfile, command, PREPEND_ERROR);
	//=====================================================================

_printD("	<= End 'CreateForWx::listStringsRes(...) => " + strInt(nbstr) + ")'");

	return nbstr;
}
// -----------------------------------------------------------------------------
// List all strings in files '*.wrc' and '*.wxs'
//
//  Command of 'wxrc'
//
//** Win **
//	wxrc [/h] [/v] [/e] [/c] [/p] [/g] [/n <str>] [/o <str>] [--validate]
//			[--validate-only] [--xrc-schema <str>] input file(s)..
//	/h, --help            show help message
//	/v, --verbose         be verbose
//	/e, --extra-cpp-code  output C++ header file with XRC derived classes
//	/c, --cpp-code        output C++ source rather than .rsc file
//	/p, --python-code     output wxPython source rather than .rsc file
//	/g, --gettext         output list of translatable strings (to stdout or file if -o used)
//	/n, --function=<str>  C++/Python function name (with -c or -p) [InitXmlResource]
//	/o, --output=<str>    output file [resource.xrs/cpp]
//	--validate            check XRC correctness (in addition to other processing)
//	--validate-only       check XRC correctness and do nothing else
//	--xrc-schema=<str>    RELAX NG schema file to validate against (optional)
//
//** Linux **
// Usage: wxrc [-h] [-v] [-e] [-c] [-p] [-g] [-n <str>] [-o <str>] [--validate]
//			[--validate-only] [--xrc-schema <str>] input file(s)...
//  -h, --help            show help message
//  -v, --verbose         be verbose
//  -e, --extra-cpp-code  output C++ header file with XRC derived classes
//  -c, --cpp-code        output C++ source rather than .rsc file
//  -p, --python-code     output wxPython source rather than .rsc file
//  -g, --gettext         output list of translatable strings (to stdout or file if -o used)
//  -n, --function=<str>  C++/Python function name (with -c or -p) [InitXmlResource]
//  -o, --output=<str>    output file [resource.xrs/cpp]
//  --validate            check XRC correctness (in addition to other processing)
//  --validate-only       check XRC correctness and do nothing else
//  --xrc-schema=<str>    RELAX NG schema file to validate against (optional)
//
// Called by :
//		1. Pre::listGoodfiles(bool _verify):1,
//
// Call to :
//		1. CreateForWx::expandName(wxString _file, wxInt32 _indexfree):1,
//		1. CreateForWx::RexewithError(wxString, wxString, bool):1,
//
wxInt32 CreateForWx::extractStringsRes(wxString& _file)
{
_printD("=> Begin 'CreateForWx::extractStringsRes(" + quoteNS(_file) + ")'");

	if (m_PathRexe.IsEmpty() )	return wxNOT_FOUND;
//1- path name output file
	// => 'xxxx_wxs.str???' or 'xxxx_wrc.str???' or ...
	// index free
	wxUint32 indexfree =  m_FileStrcreated.Count();
	// new name for output
	//==============================================
	wxString filemod = expandName(_file, indexfree);
	//==============================================
	m_nameFileTemp = m_Temp + filemod.AfterLast(cSlash);
//_printError("m_nameFileTem = " + quote(m_nameFileTemp) );
	if (m_Workspace)
	{
	// change 'm_DirProject' ->  'm_DirprojectLeader'
		m_nameFileTemp.Prepend(m_DirprojectLeader);
	}
//_printError("listStringsRes => m_DirprojectLeader = " + quote(m_DirprojectLeader) );

//2- build command for 'wxrc'
	wxString command = m_PathRexe;
	command += " -v" ;
	command += " -g" ;
	//command += "--validate";
	command += " -o" + dquote(m_nameFileTemp);
	command += dquote(_file);

//3- command execute with errors
    //============================================
	wxInt32 nbstr = RexewithError(_file, command);
	//============================================

//4- copy of temporary file : 'temp/dummy_xrc.strxxx' -> 'temp/dummy.xrc'
	if (nbstr)
	{
        //================================
		bool ok = copyRes(m_nameFileTemp);
		//================================
		if (!ok)
		{
			Mes = _("The copy of the") + quote(m_nameFileTemp) + _("file failed");
			_printError(Mes);
		}
	}

_printD("	<= End 'CreateForWx::extractStringsRes(...) => " + strInt(nbstr) + ")'");

	return nbstr;
}

// -----------------------------------------------------------------------------
// Display formatting for resources
//  modify
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
	//=====================================================================
    wxString result = Pre::executeAndGetOutputAndError(_command, _prepend);
    //=====================================================================
	if (result.IsEmpty()) 	return -2;	// ??

//2- if '-v' result error starts with  = "10:16:18: Error: ..\n"
	if (result.Find("Error:") != wxNOT_FOUND)
	{
		_printError("result: " + quoteNS(result)  );

		return wxNOT_FOUND;
	}

//3- if '-v' result starts with  = "processing name.xrc...\n"
	wxString tag =  "...\n";
	// no result withless "-v"
	if (result.EndsWith(tag))
	{
	// result withless 'tag' and withless  "processing "
		result.Replace("processing ", wxEmptyString);
		result.Replace(tag, wxEmptyString);
	}

//4- print 'm_nameFileTemp' into 'Fileswithstrings'
	wxInt32 nbstr = 0;
	// read source 'xxx_xrc.strnnn' => 'm_nameFileTemp'
    //======================================================
	wxString source = Pre::readFileContents(m_nameFileTemp);
    //======================================================
	if (!source.IsEmpty())
	{
		wxArrayString tabmes = ::GetArrayFromString(source, Lf, TRIM_SPACE);
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
	// return source contains all strings filtered or not
        //=================================================
		bool ok = CreateForWx::wxrcOut(source, _shortfile);
		//=================================================
	//	wxString mes = source;
    // mes contains strings filtered
	// numbers strings
		if (m_goodListing)      nbstr = m_nbListStrings - nbstrBefore;
		else                	nbstr = m_nbExtractStrings - nbstrBefore;
	// header
		++m_nbFileEligible;
		wxString header = Lf + Tab + "E"+ strIntZ(m_nbFileEligible, 4) + "-";
		header += quote(_shortfile) + " ==> " + strInt(nbstr) + Space + _("string(s)");
		_printWarn(header);
		m_Fileswithstrings.Add(header);
	// list numbers line
		m_Fileswithstrings.Add(source);
	// display 'tabmes' line by line
		wxString line;
		tabmes = ::GetArrayFromString(source, Lf, TRIM_SPACE);
		nbstrfile = tabmes.GetCount();
		for (wxInt32 n = 0 ; n < nbstrfile; n++)
		{
			line = tabmes.Item(n);
			line.Prepend(Tab + space(3)) ;
		// uses 'Collector::MesToLog(...)'
			if (tabmes.Item(n).Left(1) == '!')
				printWarn(line);
			else
				print(line);
		}

// 6- replace 'source' by 'mes'
        bool good = true;
        if (!ok)
        {
            good = ::cbSaveToFile(m_nameFileTemp, source);
            if (!good)
            {
                Mes = _("Error in saved")  + quote(m_nameFileTemp)  ;
               _printError(Mes);
            }
        }
// 7- last message
        Mes = Tab + "  -> " + _("creates temporary") + quote(m_nameFileTemp);
        if (!ok && good)
        {
            Mes += _("which has been filtered") + " (" + strInt(m_nbFilteredRes) ;
            Mes += Space + _("strings deleted") + ") ";
        }
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

// -----------------------------------------------------------------------------
//  Lists the strings found in the '*.xml' using the 'its' rules
//
// Called by :
//		1. Pre::listGoodfiles(bool _verify):1,
// Call to :
//		1. Pre::GexewithError(const wxString& _shortfile, const wxString& _command,
//							const bool& _prepend):1,
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
    // includes all the rules currently encountered ...
    rules = "codeblocks.its";
    // you have to check if the 'rulesIts' file is readable
	m_PathRulesIts = m_PathIts + rules;

//3- build command : 'xgettext ...'
	// for 'xgettext' and 'its' for *.xml
	wxString its = " --its=" + dquoteNS(m_PathRulesIts);
	wxString command = m_PathGexe;
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
	//command += " -F";
	// sorted data
	//command += " -s" ;
	// text width before new line
	//command += " -w 320 ";
	command += " -w 80 ";
	//command += " --no-wrap " ;
	// + 'itstool' comment
	command += "--itstool ";
	// path absolute 'its' file
	command += its;
//_printWarn(quote(command));
//2- command execute with errors
	// list string into
	//=====================================================================
	wxInt32 nbstr = Pre::GexewithError(_shortfile, command, PREPEND_ERROR);
	//=====================================================================
    if(!nbstr)
    {
        Mes = Tab + "!!" + quote(_shortfile)+ "!!";
        _printError(Tab + Line(Mes.Len()) );
        _printWarn(Mes);
        _print(Tab + space(2) +  "** " + _("This file has no translatable strings") + cDot);
        _printError(Tab + Line(Mes.Len()) );
    // a '*.xml' fil withless strings
        m_nbXmlWithlessString++;
    }
// all
    Mes = Tab + space(2) + "**" +  Space + _("Rules template used") + Space ;
    Mes += ":" + quote(m_PathRulesIts);
    _printWarn(Mes);

_printD("	<= End 'CreateForWx::listStringsXml(...) => " + strInt(nbstr) + ")'");

	return nbstr;
}

//------------------------------------------------------------------------------
// Initialyze 'm_FilesCreatingPOT' with a header.po
//
//  Call by :
//  1. Collector::OnMenuListPrj(wxCommandEvent& _pEvent):1,
//  2. Collector::OnMenuListWS(wxCommandEvent& _pEvent):1,
//
wxUint16 CreateForWx::iniHeaderPo()
{
// the header : Scol2 = ";;"
    wxString header;
    header << wxString("# SOME DESCRIPTIVE TITLE.") << Scol2 ;
    header << wxString("# Copyright (C) 2020-2022") << Scol2 ;
    header << wxString("# This file is distributed under the same license as the PACKAGE package.") << Scol2 ;
    header << wxString("# LETARTARE <https://forums.codeblocks.org>, 2022") << Scol2 ;
    header << wxString("# from plugin CodeBlocks::Collector-") << VERSION_COLLECT << (", 2022") << Scol2 ;
    header << Scol2 ;
  //  header << wxString("#, fuzzy") << Scol2 ;
    header << wxString("msgid \"\" ") << Scol2 ;
    header << wxString("msgstr \"\" ") << Scol2 ;
    header << Space << Scol2 ;
    header << dquoteNS("Project-Id-Version: PACKAGE VERSION \\n") << Scol2 ;
    header << dquoteNS("Report-Msgid-Bugs-To: \\n") << Scol2 ;
    header << dquoteNS("POT-Creation-Date: 2022-11-18 14:24+0200 \\n") << Scol2 ;
    header << dquoteNS("PO-Revision-Date: YEAR-MO-DA HO:MI+ZONE \\n") << Scol2 ;
    header << dquoteNS("Last-Translator: LETARTARE <https://forums.codeblocks.org> \\n") << Scol2 ;
    header << dquoteNS("Language-Team: LANGUAGE <LL@li.org> \\n") << Scol2 ;
    header << dquoteNS("Language:  \\n") << Scol2 ;
    header << dquoteNS("MIME-Version: 1.0 \\n") << Scol2 ;
    header << dquoteNS("Content-Type: text/plain - charset \\n") << Scol2 ;
    header << dquoteNS("Content-Transfer-Encoding: 8bit \\n")  << Scol2 ;
    header << dquoteNS("Plural-Forms: nplurals=2; plural=n != 1; \\n") << Scol2;

    m_sizeHeaderpo = header.Len() + 80;
    m_FilesCreatingPOT = ::GetArrayFromString(header, Scol2, TRIM_SPACE);
   // m_FilesCreatingPOT.Add(Lf);
    m_FilesCreatingPOT.Add(Space + Lf);
    wxUint16 nb = m_FilesCreatingPOT.Count();

    return nb;
}

// -----------------------------------------------------------------------------
// Creating a file '*.pot'
//
// Called by :
//		1. Pre::extraction(bool _prjfirst, cbProject * _prj):1,
// Call to :
//      1. Pre::saveArrayToDisk (const wxArrayString& _strarray, const wxChar _mode):1,
//		2. CreateForWx::Cleantemp():1,
//
bool CreateForWx::creatingPot(bool _prjfirst)
{
_printD("=> Begin 'CreateForWx::creatingPot(" + strBool(_prjfirst) + ")'" );

    if (m_PathGexe.empty())
    {
        Mes = " !!! 'xgettext' " + _("not defined") + " !!!" ;
        _printError(Mes);
        return false;
    }
    if (m_PathUexe.empty())
    {
        Mes = " !!! 'msguniq' " + _("not defined") + " !!!" ;
        _printError(Mes);
        return false;
    }

// local variables
	bool ok = true, good = false;
	wxString name, nameprj, shortfile, longfile, filein, pActualprj ;
	wxUint32 nbCells, index = 0, nfiles;
    nbCells = m_FileswithI18n.GetCount()   ;

	Mes = Tab + "--> " + _("Extract of") + Space + strInt(nbCells) + Space;
	Mes += _("file(s) by 'xgettext' and 'wxrc'");
	if(!m_Workspace)
        Mes +=  " --> " + dquoteNS(m_Dirlocale + m_Shortnamedup) ;
	_printWarn(Mes);
	m_Fileswithstrings.Add(Mes);

// globales variables
	m_DirlocaleLeader = m_DirprojectLeader + m_Dirlocale;
//_printError("creatingPot => m_DirprojectLeader = " + quote(m_DirprojectLeader) );
	Mes = Tab;
	if (m_Workspace)	Mes += _("Leader locale");
	else				Mes += _("Locale");
	Mes += Space + _("directory used") + " :" + quote(m_DirprojectLeader) +Lf;
	_printWarn(Mes);
	m_Fileswithstrings.Add(Mes);

//2- extract all elegible files from 'm_FileswithI18n'
	wxString dirproject, tempres, ext ;
	wxUint32 nbprjWS = 0;
	// range 'g_MaxProgress'
	BuildLogger::g_MaxProgress = nbCells + wxUint32(nbCells/5) ;
	BuildLogger::g_CurrentProgress = 0 ;
	if (nbCells > 0) nfiles = nbCells -1;
/// for debug
// _printError("CreatingPot : g_MaxProgress = " + strInt(BuildLogger::g_MaxProgress) + " files");
    // read all registered files in  'm_FileswithI18n'
	for (wxUint32 i=0; i < nbCells ; i++ )
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
			if (name.Last() == cSlash)
			{
			// project path
				dirproject = name;
				continue;
			}
		// its a file  : always relative path
			if (name.Last() != cSlash)
			{
			// file name : relative path
				shortfile = name;
				filein = shortfile;
			}
		}
//2-2 Workspace
		if (m_Workspace)
		{
			name =  m_FileswithI18n.Item(i);
		// it's a path : always absolute path
			if (name.Last() == cSlash)
			{
			// read name project
				nameprj = m_PrjwithI18nWS.Item(i);
			// project path
				dirproject = name;
				++nbprjWS;
				Mes = Lf + _("Project") + "-" + strIntZ(nbprjWS, 4) ;
				Mes += "/" + strIntZ(m_nbPrjextractWS, 4) + " :" + quote(nameprj) ;
				if (_prjfirst)	Mes += "-->";
                else			Mes += "-->>";
                Mes += quote(m_Shortnamedup);
				Mes += Lf + Tab + _("path") + " :" + quote(dirproject);
				_printWarn(Mes);
				continue;
			}
		// it's a file : always relative path
			if (name.Last() != cSlash)
			{
			// file name : relative path
				shortfile = name;
			}
		}

	// log message
		++index;
		Mes =  Tab + "F" + strIntZ(index, 4) + "/" + strIntZ(nfiles, 4) ;
		Mes +=  " :"  + quote(shortfile);
    // uses 'Collector::MesToLog(...)'
		print (Mes);
		m_Fileswithstrings.Add(Mes);
	} // end for
/// test
//showArray(m_FilesCreatingPOT, "m_FilesCreatingPOT");
// save to '*.dup' who is complete with perhaps some duplicates
    //=================================================
    ok = Pre::saveArrayToDisk(m_FilesCreatingPOT, 'P');
    //=================================================

//3- delete all temporary files
    //=========================================
	wxInt32 nbfiles = CreateForWx::Cleantemp();
	//=========================================
	good = nbfiles >= 0 ;

//4- 'Stop'
	if (!good)
		return false;

_printD("	<= End 'CreateForWx::creatingPot(...)' => " + strBool(ok && good) );

	return ok && good ;
}

// -----------------------------------------------------------------------------
// Modify ressource or xml file name-
//		=> 'xxxx_wxs.str???' or 'xxxx_wrc.str???' or ...
//		??? is an insertion index in 'm_FileStrCreated'
//
// Called by :
//		1. wxInt32 CreateForWx::listStringsRes(wxString& _file):1,
//
// Call to : none
//
wxString CreateForWx::expandName(wxString _file, wxInt32 _indexfree)
{
_printD("=> Begin 'CreateForWx::expandName(" + quote(_file) + ", " + strInt(_indexfree) + ")");
// _indexfree == 0  => name = 'm_NameProjectLeader'
	if (_indexfree == 0)
	{
	// already registered ?
		_indexfree = m_FileStrcreated.Index(_file);
		if (_indexfree == wxNOT_FOUND)
			return wxEmptyString;
	}
// new extension
	wxString ext(".str");
	ext += strInt(_indexfree);
// only name file witless directory
	_file = _file.AfterLast(cSlash);
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
// -----------------------------------------------------------------------------
// Copy a temporary file to orginal
//	'xxxx_wxs.str???' => 'xxxx.wxs'
//	'xxxx_wrc.str???' => 'xxxx.xrc'
//
// Called by :
//		1.  CreateForWx::listStringsRes(wxString& _file):1,
//
// Call to :
//      1. Pre::readFileContents(const wxString& _filename):1,
//
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
        //===================================================
		wxString stringsRes = Pre::readFileContents(nameres);
		//===================================================
		wxInt32 nb = stringsRes.Replace("\r", "", ALL_TXT) ;
		if (nb)
		{

			Mes = Tab + Space + iToStr(nb) + Space + _("deleted") + " '\\r' " ;
			Mes += _("in file") + quote(nameres);
			_printWarn(Mes);
		}
//3- save file
		ok = ::cbSaveToFile(nameres, stringsRes);
	}

	return ok;
}

// -----------------------------------------------------------------------------
// Display formatting for resources  ( 'wxrc' executable )
//  - '_txt' is filtered and returned
//
// Called by :
//		1. CreateForWx::RexewithError(const wxString& _shortfile,
//									  const wxString& _command,
//									  const bool& _prepend)
// Call to : none
//
/// only displays in the Collector log, not in the '*.list' file
//
bool  CreateForWx::wxrcOut(wxString& _txt, wxString _shortfile)
{
_printD("=> Begin 'CreateForWx::wxrcOut(" + quoteNS(_txt) + ", ...)'");

// Global variable to prevent errors in resources
	m_Strwitherror = false;
	wxString tn = CrLf,  result = wxEmptyString, mes = wxEmptyString, filter =  wxEmptyString;
	wxArrayString tabout = wxArrayString()
				 ,tabline = wxArrayString();
// error ?
	wxString line, tmp = wxEmptyString, bannedline;
	wxInt32 pos = _txt.Find("Error:" );
	if (pos != wxNOT_FOUND )
	{
	// error parsing
		m_Strwitherror = true;
		tabout = ::GetArrayFromString(_txt, Lf, NO_TRIM_SPACE);
    // first line if not verbose
		line = tabout.Item(0);
		line.Remove(0, pos);
		tmp = line.AfterFirst(cSpace);

		return "";
	}
// it's good
	// array from string
	tabout = ::GetArrayFromString(_txt, Lf, NO_TRIM_SPACE);
	wxUint32 nbl = 0 ; pos = 0;
	m_nbFilteredRes = 0;
	bool banned = false;
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
		// "1" => "00001" and "985" => "00985"
			strZ(tmp, 5);
			mes	=  Tab  + "  :L" + tmp + Space + "==>";
		}
		else
	// line with 'Keyword' => translatable string
		if (line.Find(m_Keyword) != wxNOT_FOUND )
		{
		// find string between two quotation tag : ATTENTION to  '_("\"no target\"")' !
			tmp = line.AfterFirst(cDquote).BeforeLast(cDquote);
			bannedline = tmp;
			banned = false;
		// filters not tranlatables
			// see 'wxString Pre::m_bannedMarks' and 'wxArrayString Pre::m_ArrayBanned'
			if (m_ArrayBanned.Index(bannedline) != wxNOT_FOUND
				||  bannedline.IsNumber() || bannedline.StartsWith("http:")
				)
			{
				filter += quoteNS(bannedline);
				m_nbFilteredRes++;
				m_ArrayFiltered.Add(bannedline);
//_printError("Res::filtered =>" + quote(bannedline) + "from " + quote(_shortfile) + mes );
				banned = true;
			}
		//	else
			{
			// find text "\r\n" not Eol
				pos = tmp.Find(tn);
				if (pos != wxNOT_FOUND)
				{
					tmp.Replace("\\r\\n", Eol + Tab + Tab, ALL_TXT);
					tmp.Replace(tn, Eol + Tab + Tab, ALL_TXT);
					tabline = ::GetArrayFromString(tmp, Lf, NO_TRIM_SPACE);
					for (wxUint32 k=0; k < tabline.GetCount(); k++)
					{
						mes += tabline.Item(k);
					}
				}
				else
				{
					if (banned) 	mes.Prepend("!! " + _("BANNED") + " !!") ;
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
//_printWarn(markNS(filter));
// total
	if (m_goodListing)		m_nbListStrings += nbl;
	else					m_nbExtractStrings += nbl;

	_txt = result;

_printD("	<= End 'CreateForWx::wxrcOut(...)'");

	return filter.IsEmpty();
}

// -----------------------------------------------------------------------------
// Delete all temporary file
//
// Called by :
//		1. Collector::OnMenuCleanTemp(wxCommandEvent& _pEvent):1,
//		2. CreateForWx::creatingPot(bool _prjfirst):1,
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
//_printError("CreateForWx::Cleantemp() => m_DirprojectLeader = " + quote(m_DirprojectLeader) );
	// get the directory of the leader project 'm_DirprojectLeader
	ok = ! m_DirlocaleLeader.IsEmpty();
	if (ok)
	{
    // control to pending messages in the event loop
		m_pM->Yield();

		m_nbFilesdeleted = 0;
		//==================================
		ok = recursRmDir(m_DirlocaleLeader);
		//==================================
		if (ok)
		{
			nfc = m_nbFilesdeleted;
			ok = nfc > 0;
			if (nfc)
			{
				Mes =  Tab + "***" + quote(strInt(nfc));
				Mes += _("files have been deleted in the temporary directory.");
				 _print(Mes);
                m_Fileswithstrings.Add(Mes);
			}
		}
	}

	if (!ok)
    {
        Mes = Tab + _("No temporary files to delete") + Dot;
        _print(Mes);
    }

//2- init
	m_FileStrcreated.Clear();

	// messages
	Mes =  "   <-- "  + _("end 'Clean temporary files'");
	_printWarn(Mes);
	//_print(Line(Mes.Len()*2));
    m_Fileswithstrings.Add(Mes);

_printD("	<= End 'CreateForWx::Cleantemp()' => " + strInt(nfc)  );

	return nfc;
}
///-----------------------------------------------------------------------------
