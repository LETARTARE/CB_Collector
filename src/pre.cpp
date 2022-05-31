/***************************************************************
 * Name:      pre.cpp
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2020-05-10
 * Modified:  2022-05-31
 * Copyright: LETARTARE
 * License:   GPL
 **************************************************************/
//------------------------------------------------------------------------------
#include "pre.h"
//------------------------------------------------------------------------------
#include <sdk.h>
#include <cbplugin.h>      // sdk version
#ifdef __linux__
    #include <macrosmanager.h>
    #include <configmanager.h>
    #include <wx/datetime.h>
	#include <wx/dir.h>
	#include "projectfile.h"
	#include "cbeditor.h"
	#include <editormanager.h>
	#include <cbproject.h>
	#include <manager.h>
	#include <wx/menu.h>
#endif
//
#include "buildlogger.h"
// not place change !
#include "defines.h"
//------------------------------------------------------------------------------
/** \brief protect for 'm_Stop' access
 */
static wxMutex st_mutexStop;
//------------------------------------------------------------------------------

///-----------------------------------------------------------------------------
//	Constructor
//
// Called by :
//		1. CreateForWx::CreateForWx(wxString & _namePlugin, int _logIndex):1,
//		2. CreateForQt::CreateForQt(wxString & _namePlugin, int _logIndex):1,
//
// Call to :
//     1. ColState::Colstate():1,
//
Pre::Pre(const wxString & _nameplugin, int _logindex) :
      m_namePlugin(_nameplugin)
	  ,m_pM(Manager::Get())
	  ,m_pMprj(Manager::Get()->GetProjectManager())
	  ,m_pMam(Manager::Get()->GetMacrosManager())
	  ,m_pMed(Manager::Get()->GetEditorManager())
	  ,m_pMconf(Manager::Get()->GetConfigManager("..."))
	  ,m_LogPageIndex(_logindex)
{
#if   defined(__WXMSW__)
	m_Win = true; m_Linux = m_Mac = false;
#elif defined(__WXGTK__)
	m_Linux = true ; m_Win = m_Mac = false;
#elif defined(__WXMAC__)
	m_Mac = true; m_Win = m_Linux = false;
#endif
	// all projects in a table
	if (m_pMprj)
	{
		m_pProjects = m_pMprj->GetProjects();
		m_nbProjects = m_pProjects->Count();
	}
}
///-----------------------------------------------------------------------------
//	Destructor
//
// Called by
//		1. CreateForWx::~CreateForWx():1,
//		2. CreateForQt::~CreateForQt():1,
//
Pre::~Pre()
{
}
///-----------------------------------------------------------------------------
// Determines the system platform
//
// Called by :
//		1. Collector::OnAttach():1,
//
wxString Pre::platForm()
{
// choice platform
	if (! m_Win)
	{
		#undef SepD
		#define SepD 13
		#undef SizeSep
		#define SizeSep 1
	}
	Mes = _("Platform") ;
	if (m_Win)
	{
		Mes += " : 'Win";
		#undef Eol
		#define Eol CrLf
	}
	else
	if (m_Mac)
	{
		Mes += " : 'Mac";
		#undef Eol
		#define Eol Cr
	}
	else
	if (m_Linux)
	{
		Mes += " : 'Linux";
		#undef Eol
		#define Eol Lf
	}

#if defined(_LP64) || defined(_WIN64)
	Mes+="-64'";
#else
	Mes += "-32'";
#endif

	return Mes ;
}
///-----------------------------------------------------------------------------
//  Get version SDK
//
// Called by :
// 	1. Collector::OnAttach():1,
//
wxString Pre::GetVersionSDK()
{
	// from 'cbplugin.h'
	wxUint16 major 	= PLUGIN_SDK_VERSION_MAJOR
			,minor 	= PLUGIN_SDK_VERSION_MINOR
			,release= PLUGIN_SDK_VERSION_RELEASE;

    return  strInt(major) + Dot + strInt(minor) + Dot + strInt(release);
}

///-----------------------------------------------------------------------------
// Give internal plugin name
//
// Called by : none
//
wxString Pre::namePlugin()		{return m_namePlugin;}

///-----------------------------------------------------------------------------
// Give values globales variables
//
// Called by :
//     1. Collector::OnMenuListPrj(wxCommandEvent& _pEvent):1,
//     2. Collector::OnMenuExtractPrj(wxCommandEvent& _pEvent):1,
//		3. Collector::OnMenuListWS(wxCommandEvent& _pEvent):1,
//     4. Collector::OnMenuExtractWS(wxCommandEvent& _pEvent);1,
//
bool Pre::isInitialized()		{return m_Init;}

// Called by :
//	1. Collector::OnMenuListPrj(wxCommandEvent& _pEvent):1
//	2. Collector::OnMenuExtractPrj(wxCommandEvent& _pEvent):1
// 	3. Collector::OnMenuListWS(wxCommandEvent& _pEvent):1
//	4. Collector::OnMenuExtractWS(wxCommandEvent& _pEvent):1
//
bool Pre::isStopped()			{return m_Stop;}

// Called by :
//	1. Collector::OnMenuListPrj(wxCommandEvent& _pEvent):1
//	2. Collector::OnMenuExtractPrj(wxCommandEvent& _pEvent):1
// 	3. Collector::OnMenuListWS(wxCommandEvent& _pEvent):1
//	4. Collector::OnMenuExtractWS(wxCommandEvent& _pEvent):1
//
bool Pre::isCancelled()			{return m_Cancel;}

// Called by :
//	1. Collector::OnMenuListExtractPrj(wxCommandEvent& _pEvent):1
//	2. Collector::OnMenuListExtractWS(wxCommandEvent& _pEvent):1
//
bool Pre::isAllRight()		    {return m_Init && ! m_Stop && ! m_Cancel;}

///-----------------------------------------------------------------------------
// Give the date followed by the time of construction of the plugin
//
// Called by :
//     1. AddOnForQt::OnAttach()
//
wxString Pre::GetDateBuildPlugin()
{
    wxString str = _("date error !");
    // search lang = "xx_XX"
//     wxString lang =  wxLocale::GetLanguageInfo(wxLANGUAGE_DEFAULT)->GetLocaleName();
// _printWarn(quote(lang));

// path of '*.so' or 'lib*.so'
	if (! m_pMam)
		m_pMam = m_pM->GetMacrosManager();
	if (m_pMam)
	{
		wxString namePath = m_pMam->ReplaceMacros(_T("$PLUGINS") );
/// TO REVIEW ...
		#if __linux__ // to adjust
            //namePath += strSlash + "lib" + m_namePlugin + DOT_DYNAMICLIB_EXT;
            namePath += strSlash + m_namePlugin + DOT_DYNAMICLIB_EXT;
		#else
            namePath += strSlash + m_namePlugin + DOT_DYNAMICLIB_EXT;
        #endif
//_printWarn(namePath);
		if (::wxFileExists(namePath))
		{
			wxFileName pluginFile(namePath);
			if (pluginFile.FileExists())
			{
			// plugin date
				wxDateTime plugindate = pluginFile.GetModificationTime();
				str = plugindate.FormatDate() + "::" + plugindate.FormatTime();
			}
			else _printError(_("plugin 'namePath'") + " =" + quote(namePath));
		}
		else _printError(_("no exists 'namePath'") + " =" + quote(namePath));
	}

    return str;
}

// ----------------------------------------------------------------------------
// Locate and call a menu from string (e.g. "/Valgrind/Run Valgrind::MemCheck")
// it's a copy of 'CodeBlocks::sc_globals.cpp::CallMenu(const wxString& menuPath)'
// return menu identificateur or wxNOT_FOUND il failed
//
// Called by;
//	1. Pre::endextract():1,
//
// ----------------------------------------------------------------------------
wxInt32 Pre::CallMenu(const wxString& _menuPath)
{
_printD("=> Begin 'Pre::CallMenu(" + _menuPath + ")'" );

// this code is partially based on MenuItemsManager::CreateFromString()
	wxMenuBar* mbar;
	if(m_pM)
		mbar = m_pM->GetAppFrame()->GetMenuBar();
	if (!mbar) 	return   wxNOT_FOUND;
	// dummy value ??
	wxInt32 idMenu = 8 ;
// this code is partially based on 'MenuItemsManager::CreateFromString()'
    wxMenu* menu = nullptr;
    size_t pos = 0;
    while (true)
    {
	// ignore consecutive slashes of '_menuPath'
        while (pos < _menuPath.Length() && _menuPath.GetChar(pos) == '/')
			++pos;
	// find next slash
        size_t nextPos = pos;
        while (nextPos < _menuPath.Length() && _menuPath.GetChar(++nextPos) != '/')  ;
	// 'nextPos'
        wxString current = _menuPath.Mid(pos, nextPos - pos);
//_printWarn("... " + quote(current));
        if (current.IsEmpty())       break;
	// the last
        bool isLast = nextPos >= _menuPath.Length();
	// current holds the current search string
        if (!menu) // no menu yet? look in menubar
        {
            wxInt32 menuPos = mbar->FindMenu(current);
            if (menuPos == wxNOT_FOUND)		break; // failed
            else							menu = mbar->GetMenu(menuPos);
        }
        else
        {
            if (isLast)
            {
                wxInt32 id = menu->FindItem(current);
                if (id != wxNOT_FOUND)
                {
				// wx >= 3.0 : execute menu item
                    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, id);
                    mbar->GetEventHandler()->ProcessEvent(evt);
				// done
                    idMenu = id;
//_printWarn(quote(_menuPath) + "=> id : " + strInt(idMenu)  );
                }
                break;
            }
            wxInt32 existing = menu->FindItem(current);
            if (existing != wxNOT_FOUND)	menu = menu->GetMenuItems()[existing]->GetSubMenu();
            else						    break; // failed
        }
        // prepare for next loop
        pos = nextPos;
    }

_printD("    <= End 'Pre::CallMenu(...) => " + strInt(idMenu) );

    return idMenu;
}

// ----------------------------------------------------------------------------
// Called by :
//	1. CreateForWx::searchExe():1,
//
// Call to :
//	1. Pre::readFileContents(const wxString& _filename):1,
//	2. Pre::findblockExe(wxString _txt,wxString _block, wxString _buffer):2,
//
wxString Pre::findPathfromconf(const wxString _txt)
{
_printD("=> Begin 'Pre::findPathfromconf(" + _txt + ")'" );

// locate 'default.conf'
	wxString pathconf = m_pMconf->LocateDataFile(m_Nameconf, sdAllKnown);
	if (pathconf.IsEmpty())
	{
		Mes = _("file") + quote(m_Nameconf) + _("not found") + " !!";
		_printError(Mes);
		ShowError(Mes);

		return wxEmptyString;
	}
// load 'm_Nameconf'
    //==============================================
	wxString conf = Pre::readFileContents(pathconf);
	//==============================================
	if (conf.IsEmpty())
	{
		Mes = "!!" + quote(m_Nameconf)+ _("is empty") + " !!";
		_printError(Mes);
		ShowError(Mes);

		return wxEmptyString;
	}
// find 'Tools' :  begin "<tools>", end "</tools>"
	wxString block = "<tools>"
            //==========================================
		   ,path = Pre::findblockExe(_txt, block, conf);
		   //===========================================
	if (path.IsEmpty())
	{
// find 'Tools+' : begin "<ShellExtensions>", end "<ShellExtensions>"
		block = "<ShellExtensions>";
		//==========================================
		path = Pre::findblockExe(_txt, block, conf);
		//==========================================
	}

_printD("	<= End 'Pre::findPathfromconf(" + path + ")'" );

	return UnixFilename(path, wxPATH_NATIVE);
}

// -----------------------------------------------------------------------------
// List all translatable strings from project
//
// Called by :
//	1. Collector::OnMenuListPrj(wxCommandEvent& _pEvent):2,
//
// Call to :
//	1. Pre::Init():1,
//	2. Pre::listing(wxUint32 _pos):1,
//
bool Pre::ListProject(const wxString& _key, wxInt32& _nbstr)
{
_printD("=> Begin 'Pre::ListProject(...)'" );

	m_Init = Pre::Init();
	if (! m_Init)		return false;
    // close the files 'project_name.list', 'project_name.extr' , 'project_name.po'
    //===================
    closeAllExtraFiles();
    //===================

	m_Workspace = (m_State == fbListWS) || (m_State == fbExtractWS);
	m_Keyword = _key;
// list strings from the first project
    //==========================
	bool ok  =  Pre::listing(0);
	//==========================
	if (!ok)
	{
        Mes =  "'Pre::ListProject(...)'" + _("Error") + " ...";
        _printError(Mes);
	}

	_nbstr = m_nbListStrings;

_printD("    <= End 'Pre::ListProject()' => " + strBool(ok) );

	return ok;
}
// -----------------------------------------------------------------------------
// Extract all translatable strings from project
//
// Called by :
//	1. Collector::OnMenuExtractPrj(wxCommandEvent& _pEvent);2,
//
// Call to :
//	1. Pre::extraction(bool _prjfirst, cbProject *_pProject):1,
//
bool Pre::ExtractProject()
{
_printD("=> Begin Pre::ExtractProject()" );

// mandatory !
	bool ok = m_Init && m_goodListing ;
	if (ok)
	{
	// launch extraction
        //==========================================
		ok = Pre::extraction(FIRST_PRJ, m_pProject);
		//==========================================
		if(!ok)
		{
            Mes = "'Pre::ExtractProject()' => " + _("Error") + " ...";
            _printError(Mes);
		}
	}
	else
	{
		ok = false; m_Cancel = false;
		if (m_State != fbListExtractPrj)
		{
			Mes = _("Before, you must execute 'List project'") + " ...'" + Lf;
			Mes+= _("to define keyword") + " ...";
			ShowInfo(Mes);
		}
	}

_printD("	<= End Pre::ExtractProject() => " + strBool(ok) );

	return ok;
}

// -----------------------------------------------------------------------------
// List all translatable strings from workspace
//
// Called by :
//	1. Collector::OnMenuListWS(wxCommandEvent& _pEvent):1
//
// Call to :
//	1. Pre::Init():1,
//	2. Pre::listingWS():1,
//
bool Pre::ListWS(wxInt32 & _nbstr)
{
_printD("=> Begin 'Pre::ListWS()'");

// basic init : ... m_NameProjectLeader, m_indexProjectLeader, ...
    //==============
    m_Init = Init();
    //==============
    if (!m_Init)	return false;

// launch listingWS()
    //==========================
    bool ok  = Pre::listingWS();
    //==========================

    _nbstr = m_nbListStrings;

_printD("	<= End 'Pre::ListWS()' => ok : " + strBool(ok) );

	return ok;
}
// -----------------------------------------------------------------------------
// Extract all translatable strings from workspace
//
// Called by :
//	1. Collector::OnMenuExtractWS(wxCommandEvent& _pEvent);1,
//
// Call to :
//	1. Pre::extractionWS():1,
//	2. Pre::extraction(bool):1,
//
bool Pre::ExtractWS()
{
_printD("=> Begin 'Pre::ExtractWS()'" );
// mandatory
	bool ok = m_Init && m_goodListing;
	if (ok)
	{
	// launch extractionWS
        //=======================
		ok = Pre::extractionWS();
		//=======================
		if (!ok)
		{
            Mes = "Pre::extractionWS() => " + _("Error")  + " ...";
            _printError(Mes);
		}
	}
	else
	{
		ok = true; m_Cancel = false;
		Mes = _("Before, you must execute 'ExtractWS") + " ...'"  + Lf;
		Mes+= _("to define keyword") + " ...";
		ShowInfo(Mes);
	}

_printD("	<= End 'Pre::ExtractWS()' => " + strBool(ok) );

	return ok;
}
// -----------------------------------------------------------------------------
//
// Called by :
//	1. Collector::ExtractProject():1,
//
// Call to
//	1. Pre::findProject(const wxString& _name, wxInt32& _index):1
//	2. Pre::extraction(bool _prjfirst = false, cbProject *_pProject):1
//
bool Pre::extractionWS()
{
_printD("=> Begin 'Pre::extractionWS()'");

	bool ok = false ; m_Cancel = false;
    //========================================
    m_Datebegin = date().Mid(0, date().Len());
    //========================================
//1- message
	// work begin
	_print(Lf + Tab + Line(77));
	wxUint32 nprj =  m_pMprj->GetProjects()->Count();
	Mes = "===> " + _("begin 'Extract workspace' on") + Space + strInt(nprj);
	Mes += Space + _("projects") ;
    Mes += Lf + Tab + m_Datebegin ;
	_print(Mes);
	m_Fileswithstrings.Add(Mes);

//2- begin workspace
	m_Workspace = m_State == fbListWS || m_State == fbExtractWS || m_State == fbListExtractWS;
   // array clean
	wxString nameprj, namefile;
	//	find project
	wxInt32 index = -2;
	// leader project
	nameprj = m_NameprjWS[0];
	// find by name project
	//=============================================
	cbProject * pPrj = findProject(nameprj, index);
	//=============================================
	if (pPrj && index >=0 && index < int(m_nbProjects))
	{
	// finded project
        //===============================
		ok = extraction(FIRST_PRJ, pPrj);
		//===============================
		if(!ok)
		{
		}
	}
	else
	{
        ok = false;
	}

_printD("	<= End 'Pre::extractionWS()'");

	return ok;
}

// -----------------------------------------------------------------------------
//
// Called by :
//	1. Pre::ListWS():1,
//
// Call to :
//	1. Pre::initFileStrcreated():1,
//	2. Pre::listing(wxUint32 _pos):2,
//	3. Pre::releaseProject(cbProject * _pProject, bool _with = true):n,
//	4. Pre::endaction(wxChar _mode):1,
//
bool Pre::listingWS()
{
_printD("=> Begin 'Pre::ListingtWS()'");

//1- clean
    //========================
	Pre::initFileStrcreated();
	//========================
	// date : initialize 'm_begin'
    m_Datebegin = date().Mid(0, date().Len());

//2- message
	// work begin
	_print(Tab + Line(77));
	wxUint32 nbPrj =  m_pMprj->GetProjects()->Count();
	Mes = "===> " + _("begin 'List workspace' on") + Space + strInt(nbPrj);
	Mes += Space + _("projects") ;
    Mes += Lf + Tab + m_Datebegin ;
	_printWarn(Mes);
	m_Fileswithstrings.Add(Mes);

//3- begin workspace
	m_Workspace = m_State == fbListWS || m_State == fbExtractWS || m_State == fbListExtractWS;
	m_nbStringsWS = 0; m_nbStrWS = 0; m_nbXmlWithlessString = 0;

    m_FileswithI18n.Clear();
    // projects name array clean
	m_NameprjWS.Clear();
	m_PrjwithI18nWS.Clear();

	m_nbPrjextractWS = 0;
	cbProject * pActualprj;
	wxString nameprj;
	bool ok, good;
//4- IN FIRST leader project is project 'Wx' ? withless message:
	ok = ! m_Wxpath.IsEmpty();
	if (ok)
	{
	// save leader project name
		m_NameprjWS.Add(m_NameprojectLeader, 1);
	// find and list elected files LEADER PROJECT withless string display  'listing(0)'
        //=============================
		ok = listing(m_indexPrjLeader);
		//=============================
		if (ok)
		{
		// strings number
			m_nbStringsWS += m_nbListStrings;
			if (m_nbListStrings)
                m_nbPrjextractWS++;
			m_nbFilesWS += m_nbFilesprj;
			m_nbFileEligibleWS += m_nbFileEligible;
		}
		else
		{
			return false;
		}
	}
	if (ok) 	good = true;

//5- read workspace projects list
	for (wxUint32 pos = 0; pos < nbPrj; pos++)
	{
	// control to pending messages in the event loop
		m_pM->Yield();
	// analysis stopped manually
		if (m_Stop)
		{
			good = false ;
			break;
		}

	// get a project pointeur
		pActualprj = m_pMprj->GetProjects()->Item(pos);
		if (!pActualprj)
		{
		    good = false;
		    break;
		}
		nameprj = pActualprj->GetTitle();
	// skip leader project
		if (pos == m_indexPrjLeader)
        {
//_printError("m_indexPrjLeader : " +strInt(m_indexPrjLeader) + quote(nameprj)) ;
            continue;
        }
	// actual project not active
        //=====================================
		ok = releaseProject(pActualprj, false);
		//=====================================
		// it's not a Wx project
		if (!ok)
		{
			Mes = _("Project") + "-" + strInt(pos) + "==>" ;
			Mes += quote(nameprj) +_("is not a 'Wx' project !");
        // notify the user
			_printError(Line(Mes.Len())) ;
			_printWarn (Mes);
            _printError(Line(Mes.Len()));
			continue;
		}
	// is project 'Wx' ? withless message:
		ok = !  m_Wxpath.IsEmpty() || ! m_Qtpath.IsEmpty() ;
		if (ok)
		{
		// save name
			m_NameprjWS.Add(nameprj, 1);
//6- List elected files project with string display
            //=====================
			ok = Pre::listing(pos);
			//=====================
			if (!ok)
			{
				good = false;
				break;
			}
		// strings number
			m_nbStringsWS += m_nbListStrings;
			if (m_nbListStrings)    m_nbPrjextractWS++;
			m_nbFilesWS += m_nbFilesprj;
			m_nbFileEligibleWS += m_nbFileEligible;
        /// Experimental
			m_nbStrWS += m_nbStrPrj;
		}
	}
	// at least one project is good
	if (good)
	{
/// DEBUG
      //  if (!NO_CODE || !NO_RES || !NO_XML)
        {
            Mes = space(3) + "## " + strInt(m_nbStrWS) + Space + _("all detected code/res strings");
            _printError(Line(Mes.Len()));
            _printWarn(Mes);
            _printError(Line(Mes.Len()));
            m_Fileswithstrings.Add(Mes);
        }

//7- last active project with tree refresh
		m_pMprj->SetProject(m_pProjectleader, REFRESH);
		//==============================================
		Pre::releaseProject(m_pProjectleader, WITH_POT);
		//==============================================
		m_DirprojectLeader = m_Dirproject.Mid(0, m_Dirproject.Len());
//8- duration and save list
		good = Pre::endaction('L');
	}
	else
	{
	// not good in 'listing(...)
		Mes = Lf + "<=== " + _("end 'List workspace'") + " : " + _("STOPPED") + " !";
		Mes += Lf + m_Theend;
		_print(Mes);
		m_Fileswithstrings.Add(Mes);
	}

_printD("	<= End 'CreateForWx::ListingWS()' => " + strBool(good)  );

	return good;
}

// -----------------------------------------------------------------------------
//
// Called by :
//	1. Pre::ExtractProject():1
//
// Call to :
//	1. Pre::releaseProject(cbProject * _pProject, bool _with = true):1,
//	2. Pre::date():2,
//	3. Pre::bigproject(wxUint32 _nbfile):1,
//	4. CreateForWx::createPot(bool _prjfirst):1,
//	5. Pre::xgettextWarn(const wxString& _txt, const bool& _withpot):1,
//	6. Pre::endextract();1,
//
bool Pre::extraction(bool _prjfirst, cbProject * _pProject)
{
/// accept '_pProject == nullptr'

_printD("=> Begin 'Pre::extraction(" + strBool(_prjfirst) + ", ...)'" );

	bool good = false; m_Cancel = false;
	Mes = wxEmptyString;
	if (_pProject)
	{
	// launch from 'extractionWX'
		m_pProject = _pProject;
	// withless modify 'Namepot' if exists
        //======================================
		Pre::releaseProject(m_pProject, NO_POT);
		//======================================
	}
	good  = true;

//2- 'keyword' extract ?
	if (m_Keyword.Matches(wxEmptyString))
	{
// TO REVIEW for Qt
		m_Keyword = m_KeyWx;
	}
	else
	if  (!(m_Keyword.Matches(m_KeyWx) /*|| m_Keyword.Matches(m_KeywxT) */ ))
	{
		Mes = quote(m_Keyword);
		Mes += _("is not right keyword for extract into 'Wx'") + " !!!" + Eol;
		Mes += _("Would you continue ?");
		wxInt32 choice = cbMessageBox(Mes, "!!!" + _("Warning") + " !!!", wxICON_QUESTION | wxYES_NO);
		if (choice == wxID_NO)
		{
				m_Cancel = true;
				return false;
		}
	}
// message
	if (! m_Workspace)
	{
		m_Fileswithstrings.Clear();
		//========================================
		m_Datebegin = date().Mid(0, date().Len());
		//========================================
		if (m_State != fbListExtractPrj)
		{
			_print(Line(110));
		}
		Mes = space(3) + _("Build file") + quote(m_Dirlocale + m_Shortnamepot);
		Mes += Lf + Tab + m_Datebegin;
		_print(Mes);
		Mes =  "===> " + _("begin 'Extract from project' with KEYWORD") + " = ";
		Mes += quote(m_Keyword) + "...";
		_printWarn(Mes);
		m_Fileswithstrings.Add(Mes);
	}

  // 'xgettext' is here
	if (m_Gexeishere)
	{
//4- create files pot
	// path leader project
		if (m_Workspace) 	wxFileName::SetCwd(m_DirprojectLeader);
	// create *.pot, call inherited
        //=============================
		bool ok = createPot(_prjfirst);
		//=============================
		if (! ok)
		{
			if (! m_Stop)
			{
				Mes = "Pre::extraction(...) : ";
				Mes += _("Unable to extract file") + quote(m_Shortnamepot) + " !!";
				//=============
				ShowError(Mes);
				//=============
				_printError(Mes);
				m_Fileswithstrings.Add(Mes);
				Pre::endaction('E');
				_print(m_Theend);
			}
			return false;
		}
	// after last file  print warning '*.pot'
		else
		if (!m_Workspace)
		{
			if (! m_Warnpot.IsEmpty() )
			{
				Mes = Lf + Tab + "** " + _("Warnings inside");
				Mes += quote(m_Shortnamepot) + "**" + Eol;
			// formatting output file
                //============================================
				Mes += Pre::xgettextWarn(m_Warnpot, WITH_POT);
				//============================================
				m_Fileswithstrings.Add(Mes);
				_printWarn (Mes);
			}
		}
	}
	// no 'gexe' !!
	else
	{
		good = false;
	}
//5- end extract
	if (good)
	{
	// ... ,call 'Poedit'
        //=======================
		good = Pre::endextract();
		//=======================
		if (!good)
		{
            Mes = "'Pre::extraction()' => " + _("Error") + " ...";
            _printError(Mes);
		}
	}
//6- end
	_print(m_Theend);
	m_Fileswithstrings.Add(m_Theend);
	// last line
	_print(Line(140) );

_printD("	<= End 'Pre::extraction()' => " + strBool(good) );

	return good;
}
// -----------------------------------------------------------------------------
// Called by :
//		1. Pre::extraction(bool _prjfirst, cbProject * _pProject):1,
// Call to :
//	1. Pre::integrity(bool _replacecar):1,
//	2. Pre::openedit(wxString _file):2,
//	3. Pre::copyFileToDir(const wxString& _moname)
//	4. Pre::endaction(wxChar _mode):1,
//
bool Pre::endextract()
{
_printD("=> Begin 'Pre::endextract()'" );

	bool good = true;
	bool del  = false;
//0- choice for delete
	// TODO : go to 'Settings' ...
	if (m_Warnpot.IsEmpty())
	{
		Mes = "1- " + _("Replace some macro") + "..."  + Lf;
		Mes += "2- " + _("Find and replace :") + Lf;
		Mes += "  2-1- '\\r' -> ''" + Lf;
		Mes += "  2-2- '$$...' -> '$...'" + Lf;
		Mes += "  2-3- '%%...' -> '%...'" + Lf;
		Mes += Lf + _("For a big project, this may take several minutes") + " ..." + Lf;
		Mes += _("Do you want replace ?");
		wxString title = _("Check the integrity of") + quote(m_Shortnamepot);
		del = cbMessageBox(Mes, title, wxICON_QUESTION | wxYES_NO) == wxID_YES;
	}
	else
	{
		Mes = Tab + "** "  +  _("Some selected strings contains the character") ;
		Mes += quote("\\r")  ;
		Mes +=  _("which is not tolerated by 'xgettext'") +  ", " ;
		Mes += _("also they will not be taken into account") ;
		Mes +=  "..." ;
		_printWarn(Mes);
		m_Fileswithstrings.Add(Mes);
		del = true;
	}

	Mes = wxEmptyString;
	if (m_Gexeishere)
	{
//1- delete special string : "\r", "$$..." -> "$...",  ...
// 	and make a copy '*.pot' to '*.po' for 'Poedit'
        //=========================
		good = Pre::integrity(del);
		//=========================
		if (!good)
		{
			Mes = _("Error during verifing file integrity");
			Mes += quote(m_Shortnamepot);
			_print(Mes);
			//=============
			ShowError(Mes);
			//=============
			wxChar mode = 'E';
			if (m_goodListing) 	mode = 'L';
			//==============================================
			good = Pre::saveArray(m_Fileswithstrings, mode);
			//==============================================
		}
	} // end m_Gexeishere

//2- end action
    //=========================-
	good = Pre::endaction('E');
	//=========================
	if (!good)
	{
	// TODO ...
		Mes = "false = Pre::endaction('E')";
		_printError(Mes);
	}

	_print(space(3) + Line(80));
//3- open '*.po' to editor
	wxString shortname = m_Namepo.AfterLast(slash).Prepend(m_Dirlocale);
	if (wxFileName::FileExists(shortname) )
	{
        //=========================
		good = openedit(shortname);
		//=========================
		if (good)
		{
			Mes =  "   # " + _("Opening file") + quote(shortname);
			_print(Mes);
			m_Fileswithstrings.Add(Mes);
		}
		else
		{
			return  good;
		}
	}

//4- start 'Poedit' mandatory with local language !!
	// new strings to merge inside '*.po' ?
	if (m_nbFileEligible)
	{
	// execute 'Translator' asynchronous mode
		Mes = "   ==> " + _("Execute") + quote(EXTERN_TRANSLATOR);
		_printWarn(Mes);
	// synchronous
		//============================================
		wxInt32 ret  = LaunchExternalTool(m_Pathpexe);
		//============================================
		good = ret == 0;
		m_Fileswithstrings.Add(Mes);

    // copy of the '*.mo' in the right place
        wxString moName = m_Namepo.BeforeLast(cDot) + DOT_EXT_MO;
        //================================
        good = Pre::copyFileToDir(moName);
        //================================
        if (!good)
        {
            // TODO ...
        }
	}
	else
	{
		Mes = "	#" + _("No new string in this scan")  + " : ";
		Mes += _("The file") + quote(shortname) + _("will not be changed");
		Mes += Lf + Tab + quoteNS(m_Pexe) + Space + _("will not be called");
		_printWarn(Mes);
		m_Fileswithstrings.Add(Mes);
	}
	_print(space(3) + Line(80) + Lf);

_printD("	<= End 'Pre::endextract()' => " + strBool(good) );

	return good;
}
///-----------------------------------------------------------------------------
//  Copy a file to an another directory
//
// Called by :
//      1. Pre::endextract():1,
//
bool Pre::copyFileToDir(const wxString& _namefile)
{
    wxString longname = _namefile, shortname = longname.AfterLast(slash);
    // '*.mo' created ?
	bool ok = wxFileName::FileExists(longname);
	if (ok)
	{
	// display 'm_pCollectLog'
	  //  SwitchToLog();
		Mes = Tab + "**" + quote(EXTERN_TRANSLATOR) + _("has created") ;
		Mes += quote(shortname) + _("to directory") + " :" ;
		_printWarn(Mes) ;
		Mes =  Tab + quote(longname.BeforeLast(slash) + slash) ;
		_print(Mes) ;
		Mes = "   <== " + _("end") + quote(EXTERN_TRANSLATOR);
		_printWarn(Mes);

	// copy this file to another location ?
		wxString title = _("Copy le file") + quote(shortname) ;
		Mes = _("Do you want to copy this file to another location") + " ?";
		wxInt32 choice = cbMessageBox(Mes, title, wxICON_QUESTION | wxYES_NO);
		if (choice == wxID_YES)
		{
         // save '*.mo'
            //==========================
            bool ret = saveAs(longname);
            //==========================
            if (ret)
            {
                Mes =  Lf + Tab + "** " + _("File") + quote(shortname) ;
                Mes += _("is copied to directory") + " :";
                _printWarn(Mes);
                Mes =  Tab + quote(longname.BeforeLast(slash) + slash);
                _print(Mes) ;
            }
            else
            {
                Mes = "** " + _("The copy is not possible on the original") + " !!" ;
                _printWarn(Mes);
                Mes = Tab + _("File") + quote(shortname) ;
                Mes += _("already exist in the directory") + " !!";
                _print(Mes);
            }
        }
	}
	else
	{
		Mes = _("The file") + quote(longname) + _("no exists") + " !!!";
		_printError(Mes);
	}

    return ok;
}

///----------------------------------------------------------------------------
//
// Called by :
// 	1. Pre::ListProject():1,
// 	2. Pre::ListWS():1,
//
// Call to :
// 	1. Pre::Initbasic():1,
// 	2. Pre::initTailored():1,  // call inherited
//
bool Pre::Init(const wxString _type)
{
_printD("=> Begin Pre::Init()");

    //==========================
	if (!Pre::Initbasic(_type) )	return false;
    // call inherited
	if (!initTailored(_type))	return false;
	//==========================
	// actualize booleens
	m_Pexeishere = ! m_Pathgexe.IsEmpty();
	m_Gexeishere = ! m_Pathgexe.IsEmpty();
	m_Mexeishere = ! m_Pathmexe.IsEmpty();

	m_Cancel = false; m_Stop = false;
// init 'm_begin' for duration in 'S'
    //==========
	beginTime();
	//==========

_printD("    <= End Pre::Init()");

	return true;
}
///-----------------------------------------------------------------------------
//
// Called by :
//  1. Pre::Init():1,
//
// Call to :
//  1. Pre::releaseProject(cbProject * _pProject, bool _with):1,
//
///-----------------------------------------------------------------------------
bool Pre::Initbasic(const wxString /*_type*/)
{
_printD("=> Begin 'Pre::Initbasic()'");

//0- global text
	m_Theend = _("End") + quote(m_Thename + "-" + VERSION_COLLECT);
//1- projects manager
	if (! m_pMprj)
	{
		Mes = _("Could not query the projects manager.");
		Mes += Lf + _("Cannot continue");
		ShowError(Mes);
		_printError(Mes); _print(m_Theend);
		return false;
	}
//2- editors manager
	if (!m_pMed)
	{
		Mes = _("Could not query the editors manager.");
		Mes += Lf + _("Cannot continue");
		ShowError(Mes);
		_printError(Mes); _print(m_Theend);
		return false;
	}
//3- configuration manager
	if (!m_pMconf)
	{
		Mes = _("Could not query the configuration manager.");
		Mes += Lf + _("Cannot continue");
		ShowError(Mes);
		_printError(Mes); _print(m_Theend);
		return false;
	}
//4- global variables
	if (m_Win)
	{
	// separators, end of line
		m_Eol = CrLf;
	// executable names
		m_Pexe = "Poedit.exe"; m_Gexe = "xgettext.exe";	m_Mexe = "msgmerge.exe";
	// configuration CodeBlocks file name
		m_Nameconf = "default.conf";
	}
	else
	if (m_Linux)
	{
		m_Eol = Lf;
		m_Pexe = "poedit"; m_Gexe = "xgettext";	m_Mexe = "msgmerge";
		m_Nameconf = "default.conf" ;//  ???
	}
	else
	if (m_Mac) // TO VERIFY ...
	{
		m_Eol = Cr;
		m_Pexe = "poedit"; m_Gexe = "xgettext";	m_Mexe = "msgmerge";
		m_Nameconf = "default.conf" ;//  ???
	}

//5- active project
	m_pProject = m_pMprj->GetActiveProject();
	if (! m_pProject)
	{
		Mes = _("Currently no project is loaded or activated.");
		Mes += Lf + _("Cannot continue");
		ShowError(Mes);
		_printError(Mes); _print(m_Theend);

		return false;
	}

//6- leader project
	m_pProjectleader = m_pProject;
	m_NameprojectLeader = m_pProject->GetTitle();
	m_DirprojectLeader = m_pProject->GetBasePath();
	// targets associated with a file
	m_Targetsfile.Clear();
	// all projects
	m_pProjects = m_pMprj->GetProjects();
	if(m_pProjects->IsEmpty())
        return false;

    m_nbProjects = m_pProjects->Count();
//_printWarn("m_nbProjects = " + strInt(m_nbProjects) );
//7- index leader project
    //===================================================
    wxInt32 index = indexProject(m_NameprojectLeader);
    //===================================================
    if (index >= 0 )    m_indexPrjLeader = index;
    else
        return false;

//8- actual project
    //==============================================
	if (! Pre::releaseProject(m_pProject, WITH_POT))		return false;
	//==============================================
//9- create directories into 'm_pProject'
		Mes = _("Could not create directory") + " :";
	//9.1 'm_Dirlocale'
        //===============================
		bool ok = createDir(m_Dirlocale);
		//===============================
		if (ok)
		{
		//9.2 'm_Temp''
            //=====================
			ok = createDir(m_Temp);
			//=====================
			if (!ok)
			{
				Mes += quote(m_Temp) + "!" ;
				_printError(Mes);
			}
		}
		else
		{
			Mes += quote(m_Dirlocale) + "!" ;
			_printError(Mes);
		}

_printD("	<= End 'Pre::Initbasic()' => " + strBool(ok));

	return ok;
}

// ----------------------------------------------------------------------------
//  When called by 'Pre::listingWS()' : '_posWS' is project position in 'WS'
//
// Called by :
//		1. Pre::ListProject():1,
//		2. Pre::listingWS():n,
//
// Call to :
//		1. Pre::initFileStrcreated();1,
//		2. Pre::date():2,
//		3. Pre::bigproject(wxUint32):1,
//		4. Pre::findGoodfilesWx(bool):1,
//		5. Pre::endaction(wxChar):1,
//
bool Pre::listing(const wxUint32& _posWS)
{
_printD("=> Begin 'Pre::listing(" + strInt(_posWS) + ")'");

//0- table initialized
    //========================
	Pre::initFileStrcreated();
	//========================
	m_nbStrPrj = 0;
	wxUint16 ncline;
//1- message !Worspace
	if (!m_Workspace)
	{
    // 'xml' withless tanslatable strings
         m_nbXmlWithlessString = 0;
    // date, initialize 'm_begin'
        //========================================
        m_Datebegin = date().Mid(0, date().Len());
        //========================================
	// work begin
        wxString mes = Tab + _("Work path") + " :" + quote(m_Dirproject);
        _print(Tab + Line(mes.Len()));
		Mes = Tab + _("The active project") + Space + quoteNS(m_Nameproject);
		Mes += ", " + strInt(m_nbFilesprj) + Space + _("files");
        Mes += Lf + Tab + m_Datebegin ;
		_printWarn(Mes);
		_print(mes);
		m_Fileswithstrings.Add(Mes + mes);

		m_DirprojectLeader = m_Dirproject.Mid(0, m_Dirproject.Len());
	}
//2- project leader  or  '_posWS == 0'
	bool leaderprj =_posWS == m_indexPrjLeader;
	if (leaderprj && m_Workspace)
	{
        Mes = Lf + Tab + _("'List from workspace' with KEYWORD") + " = ";
        Mes += quote(m_Keyword) + "...";
        _print(Mes);
        m_Fileswithstrings.Add(Mes);
	}
//3- messages
	if (m_Workspace)
	{
   	// projects
        Mes = Space + _("Project") + "-" + strInt(_posWS) + quote(m_Nameproject);
		Mes += ", " + strInt(m_nbFilesprj) + Space + _("files") + Space ;
		if (leaderprj)
			Mes += " *** " + _("PROJECT LEADER") + " *** ";
        // formatting
        wxString line =  wxString('=', Mes.Length());
        _print(Lf + line); _print(Mes); _print(line);
    // add into array
		m_Fileswithstrings.Add(Mes);
		Mes = Tab + _("Project path") + " :" + quote(m_Dirproject);
		_print (Mes);
	// add into array
		m_Fileswithstrings.Add(Mes);
        // leader projects
		Mes = Tab + _("Path of the PROJECT LEADER");
		Mes += " :" + quote(m_DirprojectLeader);
		_printWarn(Mes) ;
	}

	Mes = "---> " + _("begin 'List from project") ;
	if (m_Workspace)
		Mes += "-" + strInt(_posWS) + "'";
	else
		Mes += Space + _("with KEYWORD") + " =" + quote(m_Keyword) + "...";

  	_printWarn(Mes);
	m_Fileswithstrings.Add(Mes);
	// strings number estimation !
    Mes = "** " + _("Estimated number of strings to translate") ;
	Mes += ", "  + _("wait a little bit")  + " ..." ;
	// aline '----   '
	ncline = Mes.Len();
	_printWarn(Line(ncline) );
	_print(Mes);
	m_Fileswithstrings.Add(Mes);

//4-  control to pending messages in the event loop
    // m_pM->Yield();

//5- find all files
//	1- create a files list to 'm_FileswithI18n' (for xgettext)
//	2- create a list of files created in 'Createdfilestr'
//		- '*.xrc' => create '*_xrc.strNNN', '*.wxs' => create '*_wxs.strNNN',
//		    with 'wxrc -g' (*.xrc, *.wxs -> *.strNNN)
//	    - '*.xml' (xgettext with 'its')
	m_goodListing = true;
	m_nbListStrings = 0;
	// only for one project
	if (!m_Workspace)  m_FileswithI18n.Clear();

//5-1- find elegibles files and calculate the strings detected number
    //=====================================
	m_nbListStrings = Pre::findGoodfiles();
	//=====================================
	Mes =  "** ... " + strInt(m_nbListStrings) ;
	Mes += Tab + _("duration") + " = " + elapsedTimeStr(m_begin);
    _print(Mes);
    // aline '----   '
    _printWarn(Line(ncline) );
    m_Fileswithstrings.Add(Mes);

	bool good = m_nbListStrings >= 0;
	if (!good)
	{
		Mes = "<--- " + _("end 'List project'")  + "  : ";
		Mes += _("on Error") + "...";
		_printError(Mes);
		m_goodListing = false;
		return false;
	}
// range of progress bar
	BuildLogger::g_MaxProgress = m_nbListStrings ;
	BuildLogger::g_CurrentProgress = 0 ;

/// debug
//_printError("Listing : g_MaxProgress = " + strInt(BuildLogger::g_MaxProgress) + " marks");

    Mes = Tab  + "# " + _("Elected files") + "..." ;
	_printWarn(Mes);
	m_Fileswithstrings.Add(Mes);

//5-2- find elegibles files
// debug
/*
if (m_Workspace)
{
_print("m_indexFirst = " + strInt(m_indexFirst) + " => " + m_FileswithI18n.Item(m_indexFirst));
_print("m_indexLast = " + strInt(m_indexLast) + " => " + m_FileswithI18n.Item(m_indexLast));
}
*/
    //=====================================
	m_nbListStrings = Pre::listGoodfiles();
	//=====================================
	good = m_nbListStrings >= 0;
	if (!good)
	{
		Mes = "<--- " + _("end 'List project'")  + "  : ";
		Mes += _("on Error") + "...";
		_printError(Mes);
		m_goodListing = false;
		return false;
	}
	else
	{
	/*
	// 'm_FileswithI18n' is filled
		Mes = Lf + Tab + "# " + strInt(m_nbListStrings) + Space ;
		Mes += _("collecting strings into") + Space;
		Mes += strInt(m_nbFileEligible) + Space + _("elegible(s) file(s)") ;
		_printWarn(Mes);
		m_Fileswithstrings.Add(Mes);
*/
        if (!NO_CODE || !NO_RES || !NO_XML)
        {
        /// Experimental : display the detected strings in project '_("'
            Mes = Tab + "# " + strInt(m_nbStrPrj) + Space + _("all detected code strings") ;
            _printError(Mes);
            m_Fileswithstrings.Add(Mes);
        }

		_printLn;
	// end listing message
		if (m_Workspace)
		{
			Mes = "<--- " + _("end 'List project");
			Mes += "-" + strInt(_posWS) + "'";
			_printWarn(Mes);
			m_Fileswithstrings.Add(Mes);
		}
	}
//6- duration and save list
	if (!m_Workspace )
	{
        //=========================
		good = Pre::endaction('L');
		//=========================
	}

_printD("	<= End 'Pre::listing()' => " + strBool(good) );

	return good;
}
// ----------------------------------------------------------------------------
// Called by :
//		1. Pre::listing(wxUint32 _pos):1,
//		2. Pre::endextract():1,
//		3. Pre::extraction(bool _prjfirst, cbProject * _pProject):1,
//		3. Pre::listingWS():1,
//
// Call to :
//		1. Pre::saveArray (const wxArrayString& _strarray,  wxChar _mode):1,
//		2. Pre::date():2,
//
bool Pre::endaction(const wxChar& _mode)
{
_printD("=> Begin 'Pre::endaction(" + quoteNS(strChar(_mode) ) + ")'" );

// ending messages
    if (m_Workspace)
    {
    // projects
        Mes = space(3) + "## "+ _("From") + Space + strInt(m_nbPrjextractWS) + Space ;
        Mes +=_("project(s) containing") + Space + strInt(m_nbFilesWS) + Space ;
        Mes += _("total files") ;
        _print(Lf + space(3) + Line(Mes.Len()) );
        _print(Mes);
        m_Fileswithstrings.Add(Mes);
    // extracted or detected strings
        Mes = Tab + " => " + strInt(m_nbStringsWS) + Space;
        if (_mode == 'L')   Mes += _("detected strings");
        else                Mes += _("extracted strings");
        Mes += ", " + _("inside") + Space + strInt(m_nbFileEligibleWS) + Space ;
        Mes += _("elected files") + ".";
         _printWarn(Mes);
        m_Fileswithstrings.Add(Mes);
    }
    else
    {
    // project
        Mes = space(3) + "## " + _("From") + Space + strInt(1) + Space ;
        Mes +=_("project containing") + Space + strInt(m_nbFilesprj) + Space ;
        Mes += _("total files") ;
        _print(space(3) + Line(Mes.Len()) );
        _print(Mes);
        m_Fileswithstrings.Add(Mes);
    // extracted or detected strings
        Mes = Tab + " => " + strInt(m_nbListStrings) + Space ;
        if (_mode == 'L')   Mes += _("detected strings");
        else                Mes += _("extracted strings");
        Mes += ", " + _("inside") + Space + strInt(m_nbFileEligible) + Space ;
        Mes += _("elected files") + ".";
		_printWarn(Mes);
		m_Fileswithstrings.Add(Mes);
    }
// *.po and taille
    if (_mode == 'E')
    {
        wxString namepo = m_Namepo.AfterLast(slash).Prepend(m_Dirlocale);
        Mes = Tab + " =>" + quote(namepo) + " (" + strInt(m_sizeFilePo) ;
        Mes += Space + _("bytes") + ")";
        _printWarn(Mes);
        m_Fileswithstrings.Add(Mes);
    }
// just for statistics on '*.xml' files
    if (m_nbXmlWithlessString)
    {
        Mes = Tab + "*** " + _("There are") + Space + strInt(m_nbXmlWithlessString);
        Mes += Space + _("files 'xml', who are not translatable strings") + "...";
        _printError(Lf +Tab + Line(Mes.Len()) );
        _printWarn(Mes);
        _printError(Tab + Line(Mes.Len()) );
    }

// file opened
    //==================================================
	bool ok = Pre::saveArray(m_Fileswithstrings, _mode);
	//==================================================
// end
	if (!m_Workspace)
	{
		if (_mode == 'E')
		{
			m_goodListing = false;
			Mes =  "<=== " + _("end 'Extract from project'");
		}
	// work end
		if (_mode == 'L')
		{
			Mes =  "<--- " + _("end 'List from project'") ;
		}
	}
	else
	{
		if (_mode == 'E')
		{
			m_goodListing = false;
			Mes =  "<=== " + _("end 'Extract from workspace'");
		}
	// work end
		if (_mode == 'L')
		{
			Mes =  "<=== " + _("end 'List from workspace'") ;
		}
	}
	Mes += " : " + _("all right.");
	_printWarn(Lf + Mes);
	m_Fileswithstrings.Add(Mes);

	Mes = Lf + Tab + _("duration") + " = " + elapsedTimeStr(m_begin) + Lf;
	_print(Mes);
	m_Fileswithstrings.Add(Mes);

_printD("	<= End 'Pre::endaction()' => " + strBool(ok));

	return ok;
}
// ----------------------------------------------------------------------------
//
// Called by :
//		1. Pre::listing(wxUint32 _pos):1,
//		2. Pre::listingWS():1,
//
// Call to :
//     1. Pre::deletefilesStr(bool _same):1,
// ----------------------------------------------------------------------------
wxUint32 Pre::initFileStrcreated()
{
_printD("=> Begin 'Pre::initFileStrcreated()'");

	wxUint32 nfc = m_FileStrcreated.GetCount();
	// is empty
	if (nfc == 0)
	{
	// box 0 for active project
		m_FileStrcreated.Add(m_NameprojectLeader, 1);
		nfc++;
	}
	else
	{
	// verify box 0 for active project
		bool same = m_FileStrcreated.Index(m_NameprojectLeader) == 0;
		// without temporary file
		if (nfc == 1)
		{
			// project has changed
			if (! same)
			{
			// update
				m_FileStrcreated.Clear();
				m_FileStrcreated.Add(m_NameprojectLeader, 1);
			}
		}
		else
		// with temporary file
		if (nfc > 1 )
		{
		// TO FINISH ...
		// delete old temporary file and name changed to actual project
		//	nfc = deletefilesStr(same);
		}
	}
_printD("	End Pre::initFileStrcreated() => " + strInt(nfc) );

// number file inside 'FileStrcreated'
	return nfc;
}

// ----------------------------------------------------------------------------
//	This method uses the current project 'm_pProject' and 'm_Nameproject'
//  called by 'Pre::listing(wxUint32 _pos)' with pos = project position in WS
//
// Called by :
//     1. Pre::listing(wxUint32 _posWS):1,
//
// Call to :
//	1. CreateForWx::isReadableFile(wxString _file, const bool _absolute):1,
//
// ----------------------------------------------------------------------------
wxInt32 Pre::findGoodfiles()
{
_printD("=> Begin 'Pre::findGoodfiles()'" );

    if (m_nbFilesprj < 1)  return 0;

// clean
    // all detected strings
    wxInt32 nbStr = 0, nbStrAll = 0;
	bool elegible = false, good = false;
// files
	ProjectFile * prjfile;
	wxString filein;
// number eligibles  files
	m_nbFileEligible = 0;
// the first project
    // directory
    m_FileswithI18n.Add(m_Dirproject);
    m_indexFirst = m_FileswithI18n.Index(m_Dirproject) + 1;
    // project name
	if (! m_Workspace)      m_PrjwithI18nWS.Add(m_Nameproject);

    wxInt32  index;
	m_Cancel = false;
	bool  okcode, okres, okxml;
	bool sameprj = false, finded = false;
	wxString ext, result;

	m_pM->Yield();

// all files of current project : !!! targets should be explored too !!!
    // for modulo calculate : it's a compromise
    wxUint32 val = m_nbFilesprj;
    if (val > 25)  val = m_nbFilesprj/25;
//_printError(strInt(val));

	for (wxUint32 nf = 0; nf < m_nbFilesprj; nf++)
	{
	// control to pending messages in the event loop
       // if (nf && nf%100 == 0)      m_pM->Yield();
       // if (nf && nf%val == 0)      m_pM->Yield();
	// analysis stopped manually
		if (m_Stop)
		{
			good = false;
			break;
		}
	//  not prjfile ?
		prjfile = m_pProject->GetFile(nf);
		if (!prjfile)	continue;

	// relative file name
		filein = prjfile->relativeFilename;
	//-----------------------------------------------------------------
		ext = filein.AfterLast(cDot);
	//1- *.cpp, *.h, *.cxx, *.script
		okcode = ext.Matches(EXT_H)  || ext.Matches(EXT_CPP);
		okcode = okcode || ext.Matches(EXT_CXX) || ext.Matches(EXT_HXX);
		okcode = okcode || ext.Matches(EXT_SCR) || ext.Matches(EXT_HPP);
	//2- '*.xml'  use 'xgettext' with '--its=xxxxx.its'
		okxml = ext.Matches(EXT_XML);
	//3- *.wks, *.wxrc :
		okres = ext.Matches(EXT_XRC) || ext.Matches(EXT_WXS);
	// elegible file
		elegible = okcode || okres || okxml;
		if (!elegible)  continue;
	/// DEBUG
		//if (okcode) continue;
		//if (okres) continue;
		//if (okxml) continue;
	/// <==
	// elegible good ?,
		// calculates the occurrences of marks in elegible file
		//==========================================
		nbStr = isReadableFile(filein, NO_ABSOLUTE);
		//==========================================
		good = nbStr > 0;
		if (!good)	continue;
	// all marks
        nbStrAll += nbStr;
	// update tables
		sameprj = false; finded = false;
		// project
		if(!m_Workspace)
		{
		// relative path
			m_FileswithI18n.Add(filein, 1);
			good = true;
		}
		// m_Workspace
		else
		{
        // index of the first from begin
			index = m_FileswithI18n.Index(filein);
			finded = index != wxNOT_FOUND ;
			sameprj = false;
		// file exists already
			if (finded) 	sameprj = m_PrjwithI18nWS.Item(index) == m_Nameproject;
		// not finded or not same project
			if (!finded || !sameprj)
			{
				m_FileswithI18n.Add(filein);
				m_PrjwithI18nWS.Add(m_Nameproject);
				good = true;
			}
		} // workspace
	} // end for nf

    m_indexLast = m_FileswithI18n.Index(m_FileswithI18n.Last(), true, true) ;
    m_nbStrPrj = nbStrAll;

_printD("	<= End 'Pre::findGoodfiles(...)'");
// all translatable strings detected
	return m_nbStrPrj ;
}

// ----------------------------------------------------------------------------
//	This method uses the current project 'm_pProject' and 'm_Nameproject'
//
// Called by :
//     1. Pre::listing(wxUint32 _pos):1,
//
// Call to :
//	2. CreateForWx::listStringsCode(const wxString& _shortfile)
//	3. CreateForWx::listStringsXml(const wxString& _shortfile)
//	4. CreateForWx::listStringsRes(const wxString& _shortfile)
// ----------------------------------------------------------------------------
wxInt32 Pre::listGoodfiles()
 {
_printD("=> Begin 'Pre::listGoodfiles()'" );

	m_Cancel = false;
	bool good, okcode, okres, okxml, elegible;
	wxString name, filein, ext, result;
	wxInt32 nbStr, nbStrCode = 0 , nbStrRes = 0, nbStrXml = 0;
// all files elegibles
	wxUint32 nbFiles = m_FileswithI18n.Count();
	if(!m_Workspace)
    {
        m_indexFirst = 0;
        m_indexLast = 0;
        if (nbFiles)    m_indexLast = nbFiles-1;
    }

	// only a directory !
    if (nbFiles < 1) return 0;

// begin : only the part used of current projct
	for (wxUint32 nf = m_indexFirst; nf <= m_indexLast  ; nf++)
	{
	// analysis stopped manually
		if (m_Stop)
		{
			good = false;
			break;
		}
    // control to pending messages in the event loop
		m_pM->Yield();
    // all names
        name =  m_FileswithI18n.Item(nf);
        // it's a directory => other project
        if (name.Last() == slash) continue ;
     // read file name
        filein = m_FileswithI18n.Item(nf);
	//-----------------------------------------------------------------
		ext = filein.AfterLast(cDot);
	//1- *.cpp, *.h, *.cxx, *.script
		okcode = ext.Matches(EXT_H)  || ext.Matches(EXT_CPP);
		okcode = okcode || ext.Matches(EXT_CXX) || ext.Matches(EXT_HXX);
		okcode = okcode || ext.Matches(EXT_SCR) || ext.Matches(EXT_HPP);
	//2- '*.xml'  use 'xgettext' with '--its=xxxxx.its'
		okxml = ext.Matches(EXT_XML);
	//3- *.wks, *.wxrc :
		okres = ext.Matches(EXT_XRC) || ext.Matches(EXT_WXS);
		// elegible file
		elegible = okcode || okres || okxml;
		if (!elegible)  continue;
	//--------------------------------------------------------------------
	//1- extensions *.h, *.cpp, *.hpp, *.cxx, *.hxx, *.script
		if (okcode)
		{
		/// ------------------------------
		///  for blocking => 'NO_CODE'
		/// ------------------------------
			if (!NO_CODE) continue;
		// use 'xgettext' : if error nbStr < 0, no string nbStr == 0
            //==============================
			nbStr = listStringsCode(filein);
			//==============================
			if (nbStr == 0) continue;
			good = nbStr > 0;
			if (good) nbStrCode += nbStr;
			else 	continue;
//_printError("Code:: nbStr = " + strInt(nbStr));
		}
		else
	//2- extensions *.wxrc, *.wks
		if (okres)
		{
		/// ------------------------------
		///  for blocking => 'NO_RES'
		/// ------------------------------
			if (!NO_RES) continue;
		// use 'wxrc' : if error nbStr < 0, no string nbStr == 0
			// name 'filein' not modify
			//=============================
			nbStr = listStringsRes(filein);
			//=============================
			if (nbStr == 0) continue;
			good = nbStr > 0;
			if (good) nbStrRes += nbStr;
			else 	continue;
//_printError("Res:: nbStr = " + strInt(nbStr));
		}
    //3- extensions *.xml
		if (okxml)
		{
		/// ------------------------------
		///  for blocking => 'NO_XML'
		/// ------------------------------
			if (!NO_XML) continue;
		// use 'xgettext' : if error nbStr < 0, no string nbStr == 0
            //=============================
			nbStr = listStringsXml(filein);
			//=============================
			if (nbStr == 0) continue;
			good = nbStr > 0;
			if (good) nbStrXml += nbStr;
			else 	continue;
//_printError("Xml:: nbStr = " + strInt(nbStr));
		}

	// all choice
		m_nbListStrings = nbStrCode + nbStrRes + nbStrXml ;
		if (m_nbListStrings < 0 )
		{
			// TO COMPLETE ...
			Mes = Tab + "** " + _("Error creating listing")  + " !!";
			_printError(Mes);
			return wxNOT_FOUND;
		}
	} // end for nf

_printD("	<= End 'Pre::listGoodfiles(...)'");

	return m_nbListStrings;
}

// -----------------------------------------------------------------------------
// Returns the number of occurrences of _str in the _txt
//
// Called by :
//		1. CreateForWx::isReadableFile(const wxString& _file, const bool _absolute):3,

// Call to : none
//
wxUint32 Pre::freqStr(const wxString& _source, const wxString& _str)
{
	wxString temp = _source;
	wxUint32 nbStr = 0;
	if (_str.empty())
		return nbStr  ;

	nbStr = temp.Replace(_str, _str);

// return occurence of '_str' in '_source'
	return nbStr;
}

// ----------------------------------------------------------------------------
// Called by :
//		1. CreateForWx::listStringsCode(wxString):1,
//		2. CreateForWx::listStringsXmlIts(const wxString& _shortfile, wxString _rulesIts)
//		3. CreateForWx::createPot(bool):1,

//		4. CreateForQt::listStringsCode(wxString _shortfile)
//		5.
//		6. CreateForQt::createPot(bool _prjfirst)

// Call to :
//		1. Pre::executeAndGetOutputAndError(const wxString& _command, const bool& _prepend_error):1,
//		2. Pre::xgettextWarn(const wxString& _txt, const bool& _withpot):3,
//		3. Pre::xgettextOut(wxString ):2,
//
wxInt32  Pre::GexewithError(const wxString& _shortfile, const wxString& _command,
							const bool& _prepend)
{
_printD("=> Begin 'Pre::GexewithError(" + _shortfile + ", " + strBool(_prepend) +  ")''");

// command, error alone, error in first :: see 'src\msw\utilsexec.cpp'
	bool  with_string = false;;
	//====================================================================
	wxString result = Pre::executeAndGetOutputAndError(_command, PREPEND);
	//====================================================================
// no chains
	if (result.IsEmpty() && _prepend)		return 0;

//_printError("result = " + quote(result) );
	wxUint32 nstrBefore,  nbStr =0 ;
	bool with_warn = false;
	wxInt32 pos, posw;
	wxString mes, tmp, header;
	wxArrayString tabmes;
// --------------------
// A- for 'liststring()'
// --------------------
	if (_prepend)
	{
	// not empty ?
	//	if (! result.IsEmpty())
		{
		// memorize strings count
            if (m_goodListing)      nstrBefore = m_nbListStrings;
            else                	nstrBefore = m_nbExtractStrings;
        // search 'xgettext:'
			pos = result.Find(m_Gexe + ":");
		// it's an error  message
			// for xml : "StartTag: invalid element name" (malformed)
			if (pos !=  wxNOT_FOUND)
			{
				result = result.BeforeLast('\n');
				mes = _("Error") + " : " ;
				if (result.Contains("StartTag:")) mes += _("'Xml' malformed") ;
				mes += Eol + "=>" + quote(result) + "<=";
				m_Fileswithstrings.Add(mes);
				_printError(mes);
				return  wxNOT_FOUND;
			}
		// -----------------------------------------
		// no error but possibly warning and string
		// -----------------------------------------
		// string ?
			pos = result.Find("#");
			with_string = pos !=  wxNOT_FOUND;
		// warning ?
			posw = result.Find("warning:");
			with_warn = posw !=  wxNOT_FOUND;
		// ----------------------------------------

	// --------------
	// 1- string only
	// --------------
			if (!with_warn && with_string)
			{
			// 1.1
				if (m_Formatxgettext)
				{
				// all strings to extracts
                    //=============================
					mes = Pre::xgettextOut(result);
					//=============================
				}
			//1.2- bumbers strings
                if (m_goodListing)      nbStr = m_nbListStrings - nstrBefore;
                else                	nbStr = m_nbExtractStrings - nstrBefore;
			// header
				++m_nbFileEligible;
				_printLn;
				header = Tab + "E" + strIntZ(m_nbFileEligible, 4) + "-" ;
				header += quote(_shortfile) + " ==> " + strInt(nbStr) + Space + _("string(s)");
				_printWarn(header);
				m_Fileswithstrings.Add(header);
			// list numbers line
				m_Fileswithstrings.Add(mes);
			// display is realized by 'xgettextOut'
				tabmes = GetArrayFromString(mes, Lf, true);
				nbStr = tabmes.GetCount();
				for (wxUint32 n = 0 ; n < nbStr; n++)
				{
                // uses 'Collector::MesToLog(...)'
					print(Tab + space(3) + tabmes.Item(n));
				}
			}
			else
	// -------------------
	// 2- warning + string
	// -------------------
			if (with_warn && with_string)
			{
	// NOT USED ???
			// get warning before the first '#'
				mes = result.Mid(0, pos);
				// formatting
				//===================================
				mes = Pre::xgettextWarn(mes, NO_POT);
				//===================================
				mes = Tab + "    * " + _("Warning(s)") + " : " + Eol + mes;
				m_Fileswithstrings.Add(mes);
				_printWarn(mes);
			// delete warning inside 'result'
				result.Remove(0, pos);
			// output file
				mes = Tab + "    ** " + _("translatable strings") + " : " + Eol;
				if (m_Formatxgettext)
				{
				// _print file name if elegible file
                    //==============================
					mes += Pre::xgettextOut(result);
					//==============================
                    //_printWarn(mes);
				}
				m_Fileswithstrings.Add(mes);
			// print 'Collector log'
                if (m_goodListing)  	nbStr = m_nbListStrings - nstrBefore;
                else                	nbStr = m_nbExtractStrings - nstrBefore;
				Mes += _("string(s)") + " ...";
				_print(Mes);
			// display is realized by 'xgettextOut'
				tabmes = GetArrayFromString(mes, Lf, true);
				nbStr = tabmes.GetCount();
				for (wxUint32 n = 0 ; n < nbStr; n++)
				{
					_print(Tab + space(3) + tabmes.Item(n));
				}
			}
			else
	// -------------------
	// 3- warning only
	// -------------------
			if (with_warn && ! with_string)
			{
			// get total warning
				mes = result;
			// formatting
                //===================================
				mes = Pre::xgettextWarn(mes, NO_POT);
				//===================================
				mes = Tab + "    * " + _("Warning(s)") + " : " + Eol + mes;
				m_Fileswithstrings.Add(mes);
				_print(mes);
			}
	// ------------------------
	// 4- impossible : see away
	// ------------------------
		} // result is no empty
	} // end _prepend

// ------------------------------------
// B- for 'createPot()'  extract = true
// ------------------------------------
	if ( ! _prepend)
	{
	// result contains only warning
		m_Warnpot = result.Mid(0, result.Len());
	// warning inside Namepot ?
		posw = result.Find("warning:" );
		with_warn = posw !=  wxNOT_FOUND;
		if (with_warn)
		{
		// find first name of 'file' inside 'file'
			pos = result.Find(_shortfile);
			if (pos !=  wxNOT_FOUND)
			{
			// memorize warning *.pot
				m_Warnpot = result.Mid(0, pos);
			// ignore warning to *.pot
				mes = result.Remove(0, pos);
			// print
				Mes = Tab + "    * " + _("Warning(s) inside") ;
				Mes += quote(_shortfile) + Eol;
				_print (Mes);
			// formatting output file
                //===================================
				mes = Pre::xgettextWarn(mes, NO_POT);
				//===================================
				Mes += mes;
				m_Fileswithstrings.Add(Mes);
				nbStr =  wxNOT_FOUND;
			} // end pos
		} // end with_warn
		else
		{
		// it's an error  message : NO TRANSLATE HERE
			mes = "No such file or directory";
			pos = result.Find(mes);
			if (pos !=  wxNOT_FOUND)
			{
				mes = _(mes) + " :" + Eol ;
				mes += markNS(result.BeforeLast('\n'));
				m_Fileswithstrings.Add(mes);
				_printError(mes);
				return  wxNOT_FOUND;
			}
			else
			{
			// an other error ...
				mes = markNS(result);
			///	m_Fileswithstrings.Add(mes);
			///	_printWarn(mes);
			}
		}
	}  // end extract (! _prepend)

_printD("	<= End 'Pre::GexewithError(...) => " + strInt(nbStr) );

	return nbStr ;
}

// ----------------------------------------------------------------------------
// Called by :
//		1. Pre::extraction(bool _prjfirst, cbProject * _pProject):1,
//		2. Pre::GexewithError(wxString _shortfile, wxString _command, bool _prepend):3,
//
// Call to :
//
wxString Pre::xgettextWarn(const wxString& _txt, const bool& _withpot)
{
_printD("=> Begin 'Pre::xgettextWarn(" + _txt + ", " + strBool(_withpot) + ")'");

// mandatory 'Lf' not Eol !
	wxArrayString tabwarn = GetArrayFromString(_txt, Lf, false);
	wxString line, linebefore = wxEmptyString, tmp = wxEmptyString;
	wxInt32 pos, nl = tabwarn.GetCount();
	for (wxInt32 u=0; u < nl; u++)
	{
		line = tabwarn.Item(u);
		if (linebefore.Matches(line))  	continue;
	// memorize line -> linebefore
		linebefore = line.Mid(0, line.Len());
	// if absolute path + name  delete "D:"
		pos = line.Find(":\\") ;
		if (pos !=  wxNOT_FOUND)
			line.Remove(0, pos + 1);
	// analyse file
		if (_withpot == false && (line.Find(m_Shortnamepot) ==  wxNOT_FOUND) )
		{
		// find first ':'
			if (line.Find(":") !=  wxNOT_FOUND)
			{
			// number line
				tmp += Tab + "    L" + line.AfterFirst(':') + Space;
				if (nl > 0 && u < nl - 1)
				{
					tmp += Eol;
				}
			}
		}
		else
	// analyse *.pot
		if (_withpot && (line.Find(m_Shortnamepot) != wxNOT_FOUND) )
		{
		// find first ':'
			if (line.Find(":") != wxNOT_FOUND)
			{
			// number line
				tmp += Tab + "    L" + line.AfterFirst(':') + Space;
				if (nl > 0 && u < nl-1)
				{
					tmp += Eol;
				}
			}
		}
		else
		{
			tmp += wxEmptyString;
		}
	}

_printD("	<= End 'Pre::xgettextWarn(...) => " + tmp + ")'");

	return tmp;
}

// ----------------------------------------------------------------------------
// Called by :
//	1. Pre::GexewithError(wxString _shortfile, wxString _command, bool _prepend):2,
//
// Call to : none
//
wxString Pre::xgettextOut(const wxString& _txt)
{
_printD("=> Begin 'Pre::xgettextOut(" + markNS(_txt) + "')");
// --------------------------------------------------------
// One PO file entry has the following schematic structure:
//
// white-space
// #  translator-comments
// #. extracted-comments
// #: reference...
// #, flag...
// #| msgid previous-untranslated-string
// msgid untranslated-string
// msgstr translated-string
// ---------------------------------------------------------

// extract lines
	wxArrayString tabout = GetArrayFromString(_txt, Lf, false);
	wxString line, lineplus, result = wxEmptyString, tmp, strNbline ;
	wxChar ca, cb;
	bool  plural = false , several = false;
	wxUint32 nbl = 0, nbLines = tabout.GetCount();
	m_Strwitherror = false;
// all lines
	for (wxUint32 u=0; u < nbLines ; u++)
	{
		line = tabout.Item(u);
		if (line.Matches(wxEmptyString) )	continue;

	// new line '#'
		ca = line.GetChar(0);
		if (ca == '#')
		{
			cb = line.GetChar(1);
		// references
			if (cb == ':')
			{
			/// -----------------------------------------------------
			/// #: src\pre.cpp:1025 src\pre.cpp:1034 src\pre.cpp:1043
			///	#: src\pre.cpp:1078
			///	#, flag...
			///	????
			///	msgid "Text to recovered"
			/// msgstr ""
			/// ------------------------------------------------------
			/// => :L1025:L1034:L1043:L1078  => "Text to recovered"
			// retrieve the different line numbers
				do
				{
					strNbline = line.AfterLast(':');
					tmp.Prepend(":L" + strZ(strNbline, 5)) ;
					line = line.BeforeLast(' ');
				}
				while (line != "#:") ;
			}
			else
		// cb in [' ', '.', ',', '|']
			/// ' ' == translator comments
			/// '.' == automatic comments
			/// ',' == flags : fuzzy, c-format, ...
			/// '|' ==  msgid previous-untranslated-string
			if (cb == ',' || cb == ' ' || cb == '.' || cb == '|')
			{
				continue ;
			}
		}
		else
	// new line 'msgid + txt'
		// a message
		// 	'msgid  ".............\n"'
		//
		if (line.Contains("msgid"))
		{
			tmp.Prepend(Tab + "  "); tmp += " ==> ";
			// after first '"' and before last '"'
			lineplus = line.AfterFirst('"').BeforeLast('"');
		/// lineplus == 'msgid text'
			several = lineplus.Matches("") ;
		// only one line
			if (!several) 	tmp += dquoteNS(lineplus);
            else            tmp += cDquote;
		// display one line
		//_print(tmp);
		}
		else
		/// msgid ""
		///	cSpace"............"
		/// cSpace"............"
		/// msgstr ""  // always empty
	// new line with only message
		// not tested !!
		if (ca == '"')
		{
	//_printWarn("several : " + quote(line));
		// recovered message to translate
			tmp += line.AfterFirst('"').BeforeLast('"');
		}
		else
	// new line 'msgstr'  => memorize 'tmp'
		if (line.Contains("msgstr")/* && first */)
		{
            if (several)    tmp += cDquote;
            result +=  tmp + Lf;
			tmp.Clear();
			nbl++;
			several = false ;
		}
		else
// TODO : msgid_plural ...
		if (line.Find("msgid_plural") != wxNOT_FOUND)
		{
			line.Replace("msgid_plural   ", " =plural=>", FIRST_TXT);
			result +=  line;
			plural = true;  // after = true;
			nbl++;
		}
		else
// TODO : ???
		if (plural)
		{
			wxUint8 Nf = 2 ; // for french ?
			wxString temp = "msgtr[" + strInt(Nf) + "]";
			if (line.Find(temp) != - wxNOT_FOUND)
			{
				result += "<=plural=" + Eol;
				plural = false; //	after = false;
			}
		}
	}
// delete last Lf
	result.EndsWith(Lf, &result);
// total strings
    if (m_goodListing)		m_nbListStrings += nbl;
	else					m_nbExtractStrings += nbl;

_printD("	<= End 'Pre::xgettextOut(...)");

	return result;
}

//-----------------------------------------------------------------------------
// Execute tool program (always asynchronous).
//
// Called by :
//      1. Pre::endextract():1,
//
// Call to :
//      1. wxString Pre::executeAsyncAndGetOutput(const wxString& _command,
//                                                const bool& _prepend_error):1,
//
wxInt32 Pre::LaunchExternalTool(const wxString& _toolexe)
{
    wxInt32 ret = 0;
// executable
    wxString command = dquoteNS(_toolexe) ;
// file name
    wxString file = "$ACTIVE_EDITOR_FILENAME";
    m_pMam->ReplaceMacros(file);

    command += Space + dquoteNS(file);
//_print("Command = " + quote(command) );
// launch '
    //======================================================================
    // asynchronous and no hided
    wxString result = Pre::executeAsyncAndGetOutput(command, PREPEND);
    //======================================================================
    if (!result.IsEmpty())
    {
        // Warning : Gdk-WARNING, Gtk-CRITICAL => assertion ...
        //_printWarn(markNS(result));
    }

    return ret;
}

///-----------------------------------------------------------------------------
// Execute another program (always synchronous).
//
// Called by :
//	1. CreateForWx::pathWx(CompileTargetBase * _pcontainer):1,
//	2. Pre::mergeFiles(wxString _new, wxString _old):1,
//	3. Pre::GexewithError(wxString _shortfile, wxString _command, bool _prepend):1,
//	4. CreateForWx::RexewithError(const wxString& _shortfile, const wxString& _command,
//								  const bool& _prepend):1,
//
wxString Pre::executeAndGetOutputAndError(const wxString& _command, const bool& _prepend_error)
{
_printD("=> Begin 'Pre::executeAndGetOutputAndError(...)'");

	wxArrayString output;
	wxArrayString error;
	//====================================================
	wxExecute(_command, output, error, wxEXEC_NODISABLE );
	//====================================================

	wxString str_out;

	if ( _prepend_error && !error.IsEmpty())
		str_out += GetStringFromArray(error, Eol);

	if (!output.IsEmpty())
		str_out += GetStringFromArray(output, Eol);

	if (!_prepend_error && !error.IsEmpty())
		str_out += GetStringFromArray(error,  Eol);

_printD("	<= End 'Pre::executeAndGetOutputAndError(...)' =>" + Eol + quote(str_out) );

	return  str_out;
}

///-----------------------------------------------------------------------------
// Execute another program (always asynchronous and no hided).
//
//  call by :
//
//      1. wxInt32 Pre::LaunchExternalTool(const wxString& _toolexe):1,
//
wxString Pre::executeAsyncAndGetOutput(const wxString& _command, const bool& _prepend_error)
{
_printD("=> Begin 'Pre::executeAndGetOutputAndError(...)'");

	wxArrayString output;
	wxArrayString error;
	//===============================================================
	wxExecute(_command, output, error, wxEXEC_ASYNC | wxEXEC_NOHIDE);
	//===============================================================

	wxString str_out;

	if ( _prepend_error && !error.IsEmpty())
		str_out += GetStringFromArray(error, Eol);

	if (!output.IsEmpty())
		str_out += GetStringFromArray(output, Eol);

	if (!_prepend_error && !error.IsEmpty())
		str_out += GetStringFromArray(error,  Eol);

_printD("	<= End 'Pre::executeAndGetOutputAndError(...)' =>" + Eol + quote(str_out) );

	return  str_out;
}

///-------------------------------------------------------------------------
// release project actual 'Project'  : with -> Namepot
//
// Called by  :
//		1. Pre::Initbasic():1,
//		2. Pre::listingWS():2,
//		3. Pre::extractionWS():2, Pre::extraction(...):1,
// Call to :
//		1. CreateForWx::detectLibProject(cbProject * _pProject, bool _report)
///-------------------------------------------------------------------------
bool Pre::releaseProject(cbProject * _pProject, bool _withpot)
{
	if (!_pProject) return false;

_printD("=> Begin 'Pre::releaseProject(_pProject, " + strBool(_withpot) + ")'");

//1- actual project
	m_pProject = _pProject;
	// name
	m_Nameproject = m_pProject->GetTitle();
	// the current path is the actual project
	m_Dirproject = m_pProject->GetBasePath();
// verify project type : no report
    //=======================================
	bool isWx = detectLibProject(m_pProject);
	//=======================================
	if(!isWx)
	{
		return false;
	}
	// absolutly
	wxFileName::SetCwd(m_Dirproject);
//2- targets number
	m_nbTargets = m_pProject->GetBuildTargetsCount();
	if (m_nbTargets == 0)
	{
		Mes = _("Not target to this project") + "  !";
		Mes += Lf + _("Cannot continue");
		ShowError(Mes);
		_printError(Mes); _print(m_Theend);

		return false;
	}
//3- all project files
	m_nbFilesprj = m_pProject->GetFilesCount();
//_printWarn("releaseProject(...) => m_nbFilesprj = " + strInt(m_nbFilesprj)) ;
	if (m_nbFilesprj < 1)
	{
		Mes = _("The active project contains no file.");
		Mes += Lf + _("Cannot continue");
		ShowError(Mes);
		_printError(Mes); _print(m_Theend);

		return false;
	}
	bool ok = true;
// with Namepot
	if (_withpot)
	{
	//4- file project.pot
		m_Dirpot = m_DirprojectLeader + m_Dirlocale;
		if (! wxFileName::DirExists(m_Dirpot))
		{
		//  linux : 0755, user : 'rwx', others : 'r-x'
            //============================
			bool ok = CreateDir(m_Dirpot);
			//============================
			if (!ok)
			{
				Mes = _("Can't create directory");
				Mes += quote(m_Dirpot) + "!!";
				ShowError(Mes);
				_printError(Mes); _print(m_Theend);

				return false;
			}
		}
	// name file pot
		m_Shortnamepot = m_Nameproject;
		if (m_Workspace)		m_Shortnamepot += "_workspace.pot";
		else					m_Shortnamepot += ".pot";
		m_Namepot = m_Dirpot + m_Shortnamepot;
    // create a native '*.pot'
		wxString firstline = "# " + m_Nameproject;
		//=========================================================
		ok = cbSaveToFile(m_Dirlocale + m_Shortnamepot, firstline);
		//=========================================================
		if (!ok)
        {
            Mes = " ** " + _("Error call to save the first") ;
            Mes += " 'cbSaveToFile(" + m_Dirlocale + m_Shortnamepot + ")'" ;
			_printError(Mes);
            ShowError(Mes);
            _print(m_Theend);
        }
        else
        {
            Mes = _("Create a native file '.pot' (the first line))") + " :" ;
            Mes += quote(m_Dirlocale + m_Shortnamepot);
           // _print(Mes);
        }
	}

_printD("	<= End 'Pre::releaseProject((...)' => " + strBool(ok) );

	return ok;
}
// ----------------------------------------------------------------------------
// Retrieve a project by name : uses 'm_pProjects' table and 'm_nbProjects'
//
// Called by :
//		1. Pre::extractionWS():1,
//
// Call to : none
//
cbProject* Pre::findProject(const wxString& _name, wxInt32& _index)
{
_printD("=> Begin 'Pre::findProject(" + _name + ", ...)'" );

    if (!m_nbProjects)  return nullptr;

	cbProject* pPrj = nullptr;
	_index = -1;
	for (wxUint32 index = 0; index < m_nbProjects; index++)
	{
		pPrj = m_pProjects->Item(index);
		if (pPrj)
        {
            if (pPrj->GetTitle() == _name)
            {
                _index = index;
                break;
            }
        }
        else
            break;
	}
_printD("	<= End 'Pre::findProject(" + _name + ", " + strInt(_index) + ")'" );

	return pPrj;
}
// ----------------------------------------------------------------------------
// Retrieve a project index by name : uses 'm_pProjects' table and 'm_nbProjects'
//
// Called by :
//		1. Pre::Initbasic(const wxString /*_type*/):1,
//
// Call to : none
//
wxInt32 Pre::indexProject(const wxString& _name)
{
_printD("=> Begin 'Pre::findIndexProject(" + _name + ")'" );

    if (!m_nbProjects)  return -1;

    wxInt32 pos = -1;
    cbProject* pPrj = nullptr;
//_printError("name = " + quote(_name) + ", m_nbProjects = " + strInt(m_nbProjects) );
    for (wxUint32 index = 0; index < m_nbProjects; index++)
	{
//_print("index = " + strInt(index) );
		pPrj = m_pProjects->Item(index);
		if (pPrj)
        {
            if (pPrj->GetTitle() == _name)
            {
                pos = index;
                break;
            }
        }
        else
        {
            Mes = "Pre::indexProject(" + _name + ") " ;
            Mes += _("has finded a null project") + " !!!";
            _printError(Mes);
            pos = -1;
            break;
        }
	}

_printD("	<= End 'Pre::findProject(" + _name + ", " + strInt(pos) + ")'" );

    return pos;
}
// ----------------------------------------------------------------------------
// Find  the path of an 'exe' into 'default.conf'
//
// Called by :
//	1. Pre::findPathfromconf(wxString _txt):2,
//
// Call to :
//
wxString Pre::findblockExe(const wxString _txt, wxString _block, wxString _buffer)
{
_printD("=> Begin 'Pre::findblockExe(" + _txt + ", ...)'" );

	wxString mes = _("not found");
	wxInt32 posb = _buffer.Find(_block);
	if (posb ==  wxNOT_FOUND)
	{
		Mes = quote(_block) + mes + " !!";
		_print(Mes);

		return wxEmptyString;
	}
// modify block : the first "<" => "</"
	_block.Replace("<", "</", FIRST_TXT );
	wxInt32 pose = _buffer.Find(_block);
	if (pose ==  wxNOT_FOUND)
	{
		Mes = quote(_block) + mes + " !!";
		_print(Mes);

		return wxEmptyString;
	}
	wxString tool = _buffer.Mid(posb, pose-posb);
// find end 'poedit' or ??
	pose = tool.Find(_txt);
	if (pose ==-1)
	{
		Mes = quote(_txt) + mes + " !!";
		_print(Mes);

		return wxEmptyString;
	}
	pose += _txt.Len();
	// delete the rest
	tool.Remove(pose, tool.Len() - pose );
// find begin path
	tool = tool.AfterLast('[');
	if (tool.IsEmpty())
	{
		Mes = _("path") + quote(_txt) + mes + " !!";
		_print(Mes);

		return wxEmptyString;
	}
// delete all '"'
	tool.Replace(Dquote, wxEmptyString, ALL_TXT);

_printD("	<= End 'Pre::findblockExe(" + _txt + ", ...)'" );

	return  tool;
}
// ----------------------------------------------------------------------------
//	Verify file integrity '*.pot'
// Called by :
//	1. Pre::endextract():1,
//
// Call to :
//	1. Pre::readFileContents(const wxString& _filename):1,
//	2. Pre::changeContents(wxString _file, wxString _old, wxString _new):5,
//	3. Pre::writeFileContents(const wxString& _filename, const wxString& _contents):1,
//
bool Pre::integrity(const bool& _replacecar)
{
_printD("=> Begin 'Pre::integrity(" + strBool(_replacecar)	+ ")'" );

//1- read source "*.pot"
	bool ok = wxFileName::FileExists(m_Namepot);
	if (!ok)
	{
		Mes =  Tab + _("Could'not locate file");
		Mes +=  quote(m_Shortnamepot) + "!!";
		_print(Mes);
		return false;
	}
	//=================================================
	wxString source = Pre::readFileContents(m_Namepot);
	//=================================================
	if (source.IsEmpty())
	{
		Mes =  Tab + _("Empty file") + quote(m_Shortnamepot) + "!!" ;
		_print(Mes);
		return false;
	}

	m_sizeFilePo = source.Len();

	Mes = Lf + "   --> " +  _("Verify file integrity");
	Mes += quote(m_Shortnamepot);
	Mes += " (" + strInt(m_sizeFilePo) + Space + _("bytes") + ")";
	_printWarn(Mes);
	m_Fileswithstrings.Add(Mes);

//2- header *.pot
	Mes = Tab + "1- " + _("Adjust and complete the header") + " ...";
	_print(Mes);
	m_Fileswithstrings.Add(Mes);

	// modify the header
	wxString oldtxt, newtxt;
	//2-1- descriptive
	oldtxt = "SOME DESCRIPTIVE TITLE"; newtxt = "$(PROJECT_NAME)";
    //================================================
	ok = Pre::changeContents (source, oldtxt, newtxt);
	//================================================
	if (!ok)
	{
		_printError("not ok 1 => Pre::changeContents(...)" );
		return false;
	}
	//2-2- year copyright
	oldtxt = "YEAR THE PACKAGE'S COPYRIGHT HOLDER";
	newtxt = m_pMam->ReplaceMacros("$TDAY").Mid(0, 4);
	//================================================
	ok = Pre::changeContents (source, oldtxt , newtxt );
	//================================================
	if (!ok)
	{
		_printError("not ok 2 => Pre::changeContents(...)" );
		return false;
	}
	//2-3- package name
	oldtxt = "PACKAGE "; newtxt = "$(PROJECT_NAME)";
	//================================================
	ok = Pre::changeContents  (source, oldtxt, newtxt);
	//================================================
	if (!ok)
	{
		_printError("not ok 3 => Pre::changeContents(...)" );
		return false;
	}
	//2-4- PACKAGE VERSION
	oldtxt = "PACKAGE VERSION"; newtxt = "$(PROJECT_FILENAME)";
	//================================================
	ok = Pre::changeContents  (source, oldtxt, newtxt);
	//================================================
	if (!ok)
	{
		_printError("not ok 4 => Pre::changeContents(...)" );
		return false;
	}
	//2-5- Last Translator
	oldtxt = "FULL NAME"; newtxt = "$PLUGINS";
	//===================================================
	//ok = Pre::changeContents  (source, oldtxt, newtxt);
	//===================================================
	//if (!ok)	return false;
	//2-6- LANGUAGE	"LL"
	oldtxt = "LANGUAGE"; newtxt = "$(LANGUAGE)";
	//================================================
	ok = Pre::changeContents  (source, oldtxt, newtxt);
	//================================================
	if (!ok)
	{
		_printError("not ok 5 => Pre::changeContents(...)" );
		return false;
	}
	//2-7- Language
	oldtxt = "Language: "; newtxt = "Language: $(LANGUAGE)";
	//=================================================
	ok = Pre::changeContents  (source, oldtxt, newtxt);
	//=================================================
	if (!ok)
	{
		_printError("not ok 6 => Pre::changeContents(...)" );
		return false;
	}
	//2-8- CHARSET : here $(ENCODING) -> "windows-1252"  !!!
	oldtxt = "CHARSET"; newtxt = "UTF-8";
	//===================================================
	//ok = Pre::changeContents  (source, oldtxt, newtxt);
	//===================================================
	if (!ok)
	{
		_printError("not ok 7 => Pre::changeContents(...)" );
		return false;
	}

//3- fix '*.pot'
	if (_replacecar)
	{
	//3-1 find "\r" and delete into valid line
		wxString wait = wxEmptyString;
		if (m_sizeFilePo >= BIGFILE)
			wait += Tab + _("PLEASE WAIT") + " ...";
		else
			wait += " ...";
		// search "\r"
		wxString rc = "\\r";
		wxChar di = '#';
		wxString line;
		Mes = Tab + "2- " + _("Search strings to delete") + " : ";
		Mes += Quote + rc + Quote + wait;
		_print(Mes);
		m_Fileswithstrings.Add(Mes);
		// an array from 'source'
		wxArrayString asource = GetArrayFromString(source, Lf, true);
		source = wxEmptyString;
		wxUint32 nr = 0;
		wxUint32 nl = asource.GetCount();
		for (wxUint32 u = 0 ; u < nl; u++)
		{
        // control to pending messages in the event loop
          //  m_pM->Yield();
			line = asource.Item(u);
			if (line.IsEmpty())		continue;
			ok = (line.GetChar(0) != di) && (line.Find(rc) !=  wxNOT_FOUND);
			if (ok)
			{
        // replace all 'rc' inside 'line'
				nr += line.Replace(rc, wxEmptyString, ALL_TXT);
			}
        // build new 'source'
			source += line + Eol;
		}
		// result
		if (nr)
		{
			Mes = Tab + "   ... " + _("deleting") + Space + strInt(nr) + Space ;
			Mes += _("character(s)");
		}
		else
		{
			Mes = Tab + "   ... " + _("nothing");
		}
		_print(Mes);
		m_Fileswithstrings.Add(Mes);

	//3-2 find "$$..." et replace by "$..."
		wxString dollardou = "\"$$", dollar = "\"$";
		wxString centdou   = "\"%%", cent = "\"%";
		Mes = Tab + "3- " + _("Search strings to replace") + " :";
		Mes += quote(dollardou) + ", " + quote(centdou) + " ...";
		_print(Mes);
		m_Fileswithstrings.Add(Mes);
		// replace	all
		nr = source.Replace(dollardou, dollar, ALL_TXT);
		if (nr)
		{
			Mes = Tab + Tab + _("finded") + Space + strInt(nr) + Space ;
			Mes +=  _("string(s)") + dquote("$$...");
			Mes +=  _("and replaced by") + dquote("$...");
		}
		else
		{
			Mes = Tab + "   ... " + _("nothing");
		}
		_print(Mes);
		m_Fileswithstrings.Add(Mes);

	//3-3 find "%%..." et replace by "%..."
		// replace	all
		nr = source.Replace(centdou, cent, ALL_TXT);
		if (nr)
		{
			Mes = Tab + Tab + _("finded") + Space + strInt(nr) + Space ;
			Mes += _("string(s)") + dquote("%%...");
			Mes += _("and replaced by") + dquote("%...");
		}
		else
		{
			Mes = Tab + "   ... " + _("nothing");
		}
		_print(Mes);
		m_Fileswithstrings.Add(Mes);
		// control to pending messages in the event loop
		// m_pM->Yield();
	}
	else
	{
		Mes =  Tab + "2- " + _("No replace");
		_print(Mes);
		m_Fileswithstrings.Add(Mes);
	}

//4- write new 'project.pot'
	if (_replacecar) 	Mes = "4- ";
	else				Mes = "3- ";
	Mes = Tab + Mes + _("Write") + " '*.pot'";
	_print(Mes);
	m_Fileswithstrings.Add(Mes);
	// control to pending messages in the event loop
  //  m_pM->Yield();
	// write file *.pot
	//===================================
	ok = cbSaveToFile(m_Namepot, source);
	//===================================
	if (!ok)
	{
		Mes =  Tab + _("Could'not write file");
		Mes + quote(m_Shortnamepot) + " !!";
		_printError(Mes);

		return false;
	}
	Mes = Tab + space(3) + _("Write to") + quote(m_Shortnamepot);
	_print (Mes);
	m_Fileswithstrings.Add(Mes);
	// m_Namepo = *.po
		m_Namepo = m_Namepot.BeforeLast(cDot) + ".po";
	// 'm_Namepo' exists already ?
	ok = wxFileName::FileExists(m_Namepo);
	// control to pending messages in the event loop
   // m_pM->Yield();
	if (!ok)
	{
//5- duplicate 'm_Namepot' to 'm_Namepo' no overwrite
		ok = wxCopyFile(m_Namepot, m_Namepo, false);
		if (!ok)
		{
			Mes =  Tab + _("Not created file") + quote(m_Namepo);
			_print(Mes);
			return false;
		}
		Mes = Tab + space(3) + _("Copy to") + quote(m_Namepo.AfterLast(strSlash.GetChar(0)));
		Mes += "(" + _("for") +  " 'Poedit')";
		_print (Mes);
		m_Fileswithstrings.Add(Mes);
	}
	else
	{
//6- make a backup 'Namepo.cop'
		wxString backup = m_Namepo.BeforeLast(cDot) + ".cop";
		ok = wxCopyFile(m_Namepo, backup, true);
		if (!ok)
		{
			Mes =  Tab + _("Not created file") + quote(backup);
			_print(Mes);
			return false;
		}
		Mes = Tab + space(3) + _("Backup") +  " (*.po) " + _("to");
		Mes += quote(backup.AfterLast(strSlash.GetChar(0)));
		_print (Mes);
		m_Fileswithstrings.Add(Mes);

//7- merge 'Namepot' inside 'Namepo'
		if (m_Mexeishere)
		{
		// control to pending messages in the event loop
          //  m_pM->Yield();
		// short name
			wxString shortpo = m_Namepo.AfterLast(slash)
					 ,shortpot = m_Namepot.AfterLast(slash) ;
		// merge 'shortpot' with  'shortpo"
            //==================================================
			wxInt32 nbdots = Pre::mergeFiles(shortpo, shortpot);
			//==================================================
			ok  = nbdots >= 0;
			if (!ok)
			{
				Mes =  Tab + _("Not merge files") + quote(m_Namepot);
				Mes += _("inside") + quote(shortpo);
				_print(Mes);
				return false;
			}
        // good
			Mes = Tab ;
			if (_replacecar) 	Mes += "5- ";
			else				Mes += "4- ";
			Mes +=  _("Merge") + quote(shortpot);
			Mes += "(+ " + strInt(m_nbNewstrings) + Space + _("new strings")  + ") " ;
			Mes += _("inside")  + quote(shortpo) ;
			_print (Mes);
			m_Fileswithstrings.Add(Mes);
			m_nbNewstrings = nbdots;
		}
	}

//8-  end
    Mes = "   <-- " +  _("end verify file integrity") + Lf;
    _printWarn(Mes);

	Mes = wxEmptyString;

_printD("	<= End 'Pre::integrity()'");

	return ok;
}
// ----------------------------------------------------------------------------
//
// Called by :
//     1. Pre::integrity(bool _replacecar):5,
//
// Call to : none
// ----------------------------------------------------------------------------
bool Pre::changeContents(wxString& _source, const wxString& _old, wxString& _new)
{
// TO REVIEW ...just header
    if (_source.IsEmpty())
    {
        Mes = "Pre::changeContents : " + _("source is empty") + " !! ";
        _printError(Mes) ;
        return false;
    }
	wxString source = _source.Mid(0, 582);
//_printD("=> Begin 'Pre::changeContents(...) => source :"  + markNS(_source));

	bool ok = false;
	wxInt32 pos = source.Find(_old);
	if (pos !=  wxNOT_FOUND)
	{
		m_pMam->ReplaceMacros(_new);
    // replace the first
		pos = _source.Replace(_old, _new, FIRST_TXT);
		ok = pos > 0;
		if (pos == 0)
		{
		// replace number
			Mes =  _("can't replace") + quote(_old);
			Mes +=  _("inside") + quote(m_Shortnamepot) + " !!";
			_print(Mes);
		}
	}

_printD("	<= End 'Pre::changeContents(...)' => " + strBool(ok) );

	return ok;
}
// ----------------------------------------------------------------------------
//
// Called by :
//     1. Pre::integrity(bool _replacecar):1,
// Call to :
//     1. Pre::executeAndGetOutputAndError(const wxString& _command, bool _prepend_error):1,
// ----------------------------------------------------------------------------
wxInt32 Pre::mergeFiles(wxString& _oldpo, wxString& _newpot)
{
_printD("=> Begin 'Pre::mergeFiles(" +  _oldpo + ", " + _newpot + ")");

// relative paths
	_oldpo.Prepend(m_Dirlocale); _newpot.Prepend(m_Dirlocale);
	// number new strings to translate : FALSE !!!
	wxUint32 nbdots = 0;
	// 'm_Pathmexe' contains already double quote
	wxString command = m_Pathmexe;
	// update
	command += " -U";
	// suffix
	command += " --suffix=.dou";
	// delete progress dot =>  nbdots == 0
	//command += " -q";
	// no fuzzy match
	command += " -N";
	//  keep the previous "msgid" strings of translated messages
	command += " --previous";
	// with color if exists
	//command += " --color ";
	// enclose filenames in quotation marks
	command += dquote(_oldpo) +  dquote(_newpot);
//_printError(quoteNS(command));
	//====================================================================
	wxString result = executeAndGetOutputAndError(command, PREPEND_ERROR);
	//====================================================================
//_printWarn(quote(result));
	if (!result.IsEmpty())
	{
	// error : ATTENTION 'msmerge' messages uses the local language !!
		if (result.Find(_("done")) ==  wxNOT_FOUND )		return 0;
		// delete all progress  indicator :  cDot == '.'
		nbdots = result.Replace(cDot, wxEmptyString);
		result.Replace("done", wxEmptyString);
	}

_printD("	<= End 'Pre::mergeFiles(...)'");

	return nbdots;
}

///-----------------------------------------------------------------------------
// Set stop current action
//
// Called by :
//		1. Collector::actualizeGraphState(const colState& _state):2
//		2. Collector::OnMenuStop(wxCommandEvent& _pEvent):2,
//
void Pre::setStop(const bool& _abort)
{
_printD("=> Begin Pre::setStop(" + strBool(_abort) + ")'" );
// before using the code we must acquire the mutex
	wxMutexLocker lock(st_mutexStop);
	m_Stop = _abort;

_printD("   <= End Pre::setStop(" + strBool(_abort) + ")'" );
}

///-----------------------------------------------------------------------------
// Set key word
//
// Called by : none
//
void Pre::setKey(const wxString& _key) 		{m_Keyword = _key;}

///-----------------------------------------------------------------------------
// Gives if a target is a command target
//
//	Called by :
//		1. Pre::isCommandTarget(const wxString& _nametarget):1,
//		2. CreateForQt::isGoodTargetQt(const ProjectBuildTarget * _pBuildTarget):1,
//
bool Pre::isCommandTarget(const ProjectBuildTarget * _pTarget)
{
	if (!_pTarget)		return false;

	return _pTarget->GetTargetType() == ::ttCommandsOnly;
}

///-----------------------------------------------------------------------------
// Gives if a target is a command target
//
//	Called by :
//		1. Collector::OnActivateTarget(CodeBlocksEvent& _pEvent):1,
//		2. Pre::findGoodfiles(bool _verify):1,
//  	1. Pre::detectQtTarget(const wxString& _nametarget, bool _report):1,
//
//  Call to :
//      1. Pre::isCommandTarget(const ProjectBuildTarget * _pTarget):1,
//
bool Pre::isCommandTarget(const wxString& _nametarget)
{
    if(!m_pProject) return false;

	return isCommandTarget(m_pProject->GetBuildTarget(_nametarget) );
}
/*
///-----------------------------------------------------------------------------
// Gives if a target is a virtual target
//
//	Called by :
//
bool Pre::isVirtualTarget(const ProjectBuildTarget * _pTarget)
{
    if(! _pTarget) return false;

    const cbProject * pProject = _pTarget->GetParentProject();
    if(! pProject) return false;

    return pProject->HasVirtualBuildTarget(_pTarget->GetTitle())  ;
}
*/
///-----------------------------------------------------------------------------
// Gives if a target is a virtual target
//
//	Called by :
//       1. Pre::isCommandOrVirtualTarget(const wxString& _nametarget):1,
//
bool Pre::isVirtualTarget(const wxString& _nametarget)
{
    if(! m_pProject) return false;

    return m_pProject->HasVirtualBuildTarget(_nametarget)  ;
}

///-----------------------------------------------------------------------------
// Gives if a target is a command or virtual target
//
//	Called by :
//      1.  CreateForWx::detectLibProject(cbProject * _pProject, bool _report):1,
//      2.  Pre::isCommandOrVirtualTarget(const ProjectBuildTarget * _pTarget):1,
//
//  Call to :
//      1. Pre::isCommandTarget(const wxString& _nametarget):1,
//      2. Pre::isVirtualTarget(const wxString& _nametarget):1,
//
bool Pre::isCommandOrVirtualTarget(const wxString& _nametarget)
{
    return isCommandTarget(_nametarget) || isVirtualTarget(_nametarget)  ;
}
///-----------------------------------------------------------------------------
// Gives if a target is a command or virtual target
//
//	Called by :
//      1. CreateForWx::detectLibProject(cbProject * _pProject, bool _report):1,
//      2. Pre::compileableProjectTargets():1,
//
//  Call to :
//      1. Pre::isCommandOrVirtualTarget(const wxString& _nametarget):1,
//
bool Pre::isCommandOrVirtualTarget(const ProjectBuildTarget * _pTarget)
{
    if(! _pTarget) return false;

    return isCommandOrVirtualTarget(_pTarget->GetTitle());
}

///-----------------------------------------------------------------------------
// Search all not compileable target for project
//	or Search all not compileable target for virtual project
//
// Called by  :
//		1.CreateForQt::detectLibProject(cbProject * _pProject, bool _report):1,
//
wxArrayString Pre::compileableProjectTargets()
{
	wxArrayString compilTargets;
	ProjectBuildTarget * pBuildTarget ;
	wxUint16 ntarget = m_pProject->GetBuildTargetsCount();
	while (ntarget)
	{
		ntarget--;
		pBuildTarget = m_pProject->GetBuildTarget(ntarget);
    // virtual target ?
		if(!pBuildTarget) continue;
    // real command target ?
        //=========================================================
		if (Pre::isCommandOrVirtualTarget(pBuildTarget) ) continue;
		//=========================================================
    // a compileable target
		compilTargets.Add(pBuildTarget->GetTitle());
	}

	return compilTargets;
}

///-----------------------------------------------------------------------------
//  Create directory
//
//  Called by :
//		1. Pre::Initbasic(const wxString _type):2,
//
bool Pre::createDir (const wxString& _dir)
{
_printD("=> Begin 'Pre::createDir(" + quote(_dir) + ")" );

	bool ok = true  ;
	if (! wxDirExists(_dir))
	{
	// Linux -> access = 0x0777
		ok = wxMkdir(_dir) ;
		if (!ok)
		{
			Mes = _("Can't create directory")  ;
			Mes += quote( _dir) + "!!";
			_printError(Mes);
			cbMessageBox(Mes, "Pre::createDir()", wxICON_ERROR) ;
		}
	}

	Mes.Clear();

_printD(" <= End 'Pre::createDir(...) => " + strBool(ok) );

	return ok ;
}

///-----------------------------------------------------------------------------
// Recursively deletes all directories and files
//
// Called by :
//		1. wxUint32 CreateForWx::Cleantemp():1
//		2. bool Pre::recursRmDir(wxString _rmDir):1, (recursive)
//
bool Pre::recursRmDir(wxString _rmDir)
{
_printD("=> Begin 'Pre::recursRmDir(" + _rmDir + ")");
// For safety, you don't empty a complete disk ('..' or '.')
    if (_rmDir.length() <= 3)
		return false;

// We make sure that the directory to be deleted exists
    if(!wxDir::Exists(_rmDir))
		return false;

// We make sure that the name of the folder to be deleted ends with a separator
	// if (_rmDir.Last() != wxFILE_SEP_PATH )
	// patch find in "https://github.com/vslavik/poedit"
	if (_rmDir.empty() || (_rmDir.Last() != wxFILE_SEP_PATH) )
		_rmDir += wxFILE_SEP_PATH;

// Creating the 'wxDir*' object
    wxDir* pDir = new wxDir(_rmDir);
    if (!pDir)
		return false;

// Retrieving the first item in the folder to be deleted
    wxString item;
    bool ok = pDir->GetFirst(&item);
    if (ok)
    {
        do
        {
        // If the item found is a directory
            if (wxDirExists(_rmDir + item))
            /// recursive call ...
                //=========================
                recursRmDir(_rmDir + item);
                //=========================
            else
        // Otherwise, we delete the file
			if(!wxRemoveFile(_rmDir + item))
			{
            //Error during deletion (access rights ?)
				Mes =_( "Could not remove item") + quote(_rmDir + item);
				_printError(Mes);
			}
			else
				m_nbFilesdeleted++;
        }
        while (pDir->GetNext(&item));
    }
// From now on, the folder is empty, so we can delete it
    // But first you have to destroy the wxDir variable that always points to it
    delete pDir;
// You can now delete the folder passed as a parameter
    ok = wxFileName::Rmdir(_rmDir);
    if (!ok)
    {
	// Error during deletion (access rights ?)
		Mes = _("Could not remove directory") + quote(_rmDir);
		_printError(Mes);
    }
    Mes.Clear();

_printD("	<= End 'Pre::recursRmDir(...)");

    return ok;
}
///-----------------------------------------------------------------------------
//	Read content file
//
// Called by  :
//		1. CreateForWx::searchExe():1,
//		2. CreateForWx::isReadableFile(const wxString& _file, const bool _absolute):1,
//		3. CreateForWx::RexewithError(const wxString& _shortfile,
//								const wxString& _command, const bool& _prepend):1,
//		4. Pre::findPathfromconf(const wxString _txt):1,
//		5. Pre::integrity(const bool& _replacecar):1,
//
wxString Pre::readFileContents(const wxString& _filename)
{
	wxFileName filename(m_pMam->ReplaceMacros(_filename));
	NormalizePath(filename, wxEmptyString);
	wxFile file(filename.GetFullPath());

	return cbReadFileContents(file);
}

///-----------------------------------------------------------------------------
// Save a 'wxArrayString' to disk and read the file in 'Editor'
//
// Called by  :
//		1. Pre::endextract():1,
//		2. Pre::endaction(wxChar _mode):1,
//
bool Pre::saveArray (const wxArrayString& _strarray, const wxChar _mode)
{
_printD("=> Begin saveArray(..., " + wxString(_mode) +  +")'" );

// print array
	wxUint32 n = _strarray.GetCount();
	bool ok = n > 0;
	if (ok)
	{
	// name file
		wxString ext;
		if (_mode == 'L') 		ext = "list";
		else
		if (_mode == 'E')		ext = "extr";
		else					return false;
	// file to disk
		wxString namefile = m_Dirpot + m_NameprojectLeader;
		if (m_Workspace)	namefile += "_workspace";
		namefile += Dot + ext;
		wxString shortname = namefile.AfterLast(slash).Prepend(m_Dirlocale);
    // exists already ?
		ok = wxFileName::FileExists(namefile);
		if (ok)		wxRemoveFile(namefile);
	// data to file
		wxString mes, temp = GetStringFromArray(_strarray, Eol, false );
		//================================
		ok = cbSaveToFile(namefile, temp);
		//================================
		if (!ok)
		{
			mes =  Tab + _("Could not write file");
			mes += quote(shortname) + " !!";
			_printError(mes);
			return false;
		}
		mes = Lf + Tab + "# " + _("Writing file") + quote(shortname);
		_print(mes);

	// open file into editor
        //===========================
		ok = Pre::openedit(namefile);
		//===========================
		if (!ok)
		{
			mes =  Tab + _("Couldn't open 'Editor' with");
			mes += quote(shortname) + " !!";
			_printError(mes);
		}
		else
		{
			Mes = Tab + "# " + _("Opening file") + quote(shortname);
			_print(Mes);
			m_Fileswithstrings.Add(Mes);
		}
	}

_printD("	<= End 'saveArray(...)' => " + strBool(ok) )	;

	return ok;
}

///-----------------------------------------------------------------------------
// Add a '_array'  to  '_receivearray' and free '_array
//
// Called by  :
//  1. CreateForWx::pathWx(ProjectBuildTarget * _pTarget):1,
//
void Pre::addArrays(wxArrayString& _receive, wxArrayString& _array)
{
    for (wxUint32 cell = 0 ;cell < _array.GetCount() ; cell++)
    {
       _receive.Add(_array.Item(cell)) ;
    }
    // free _array
    _array.Clear();
}
///-----------------------------------------------------------------------------
// Display a file in  'Editor'
//
// Called by  :
//		1. Pre::endextract():2,
//		2. Pre::saveArray (const wxArrayString& _strarray,  wxChar _mode):1,
//
bool Pre::openedit(const wxString _file)
{
_printD("=> Begin 'openedit(" + _file + ")'" );

	bool ok = wxFileName::FileExists(_file);
	if (ok)
	{
		cbEditor*  ed_open = m_pMed->IsBuiltinOpen(_file);
	// editor use '_file'
		if (ed_open)
		{
			ed_open->Close();
		}
	// open '_file'
		ed_open = m_pMed->Open(_file);
		if (ed_open )
		{
			ok = true;
		}
		else
		{
			Mes = _("Couldn't open") + quote(_file) + " !!";
			ShowError(Mes);
			_printError (Mes);
			ok  = false;
		}
	}

_printD("	<= End 'openedit(...)' => " + strBool(ok) );

	return ok;
}

///-----------------------------------------------------------------------------
// Close a file in 'Editor'
//
// Called by  :
//      1. Pre::closeAllExtraFiles():3,
//
bool Pre::closedit(const wxString _file)
{
_printD("=> Begin 'closedit(" + _file + ")'" );

    bool ok = false ;
    cbEditor*  ed_open = m_pMed->IsBuiltinOpen(_file);
// editor use '_file'
    if (ed_open)
    {
        ok = ed_open->Close();
    }

_printD("	<= End 'closedit(...)' => " + strBool(ok) );

	return ok;
}
///-----------------------------------------------------------------------------
// Close all extra files in 'Editor'
//
// Called by  :
//      1. Collector::OnMenuWaitingForStart(wxCommandEvent& _pEvent):1,
//      2. Pre::ListProject(const wxString& _key, wxInt32& _nbstr):1,
//      3. Pre::ListWS(wxInt32 & _nbstr):1,
//
bool Pre::closeAllExtraFiles()
{
_printD("=> Begin 'Pre::closeAllExtraFiles()'");

    wxString fullpath = m_Dirproject + m_Dirlocale + m_Nameproject;
    if (m_Workspace)   fullpath += "_workspace";

    //======================================
    bool oklist = closedit(fullpath + ".list") ;

    bool okextr = closedit(fullpath + ".extr");

    bool okpo   = closedit(fullpath+ ".po");
    //======================================

_printD("    <= End 'Pre::closeAllExtraFiles()'");

    return oklist || okextr || okpo;
}

///-----------------------------------------------------------------------------
// Save an editor file to disk to an other directory
//
// Called by  :
//		1. Collector::OnFilePoModified(CodeBlocksEvent& _pEvent):1,
//
bool Pre::saveAs(wxString & _namefile)
{
 	bool ok = false;
    wxFileDialog saveFile(nullptr,
						_("Select directory to save"),
						"",
						_namefile,
						"*.mo",
						wxFD_SAVE | wxFD_OVERWRITE_PROMPT | wxFD_CHANGE_DIR
						);
    PlaceWindow(&saveFile);
	wxInt32 ret =  saveFile.ShowModal();
    if (ret == wxID_OK)   // wxID_OK = 5100
	{
	// return new long name
		wxString newfile = saveFile.GetPath();
	// copy the _namefile  with overwrite
		ok = wxCopyFile(_namefile, newfile, true);
		if (ok)
        {
			_namefile = newfile;
        }
        else
        {

        }
	}
    else
    {
        ok = false ;
    }

 	return ok;
}

// ----------------------------------------------------------------------------
//  Called by : many ...
//
void Pre::ShowInfo(const wxString& _mes)
{
	cbMessageBox(_mes, wxEmptyString, wxICON_INFORMATION);
}
// ----------------------------------------------------------------------------
//  Called by : many ...
//
void Pre::ShowError(const wxString& _mes)
{
	cbMessageBox(_mes, wxEmptyString, wxICON_ERROR);
}

///-----------------------------------------------------------------------------
// Get date
//
// Called by  :
//  1. Pre::Init():1 ,
//	1. Pre::listing(...):2
//	2. Collector::OnSaveFileEditor(CodeBlocksEvent& _pEvent):1,
//	3. Pre::extraction(bool _prjfirst, cbProject * _pProject):
//
wxString Pre::date()
{
	wxDateTime now = wxDateTime::Now();
	wxString str = now.FormatDate() + "-" + now.FormatTime();
// for duration in 'S'
    //====================
	m_begin = beginTime();
	//====================

    return str ;
}
///-----------------------------------------------------------------------------
// Execution time
//
//
wxString Pre::duration()
{
    clock_t dure = clock() - m_start;
    if (m_Linux)     dure /= 1000;

    return wxString::Format("%ld ms", dure);
}
///-----------------------------------------------------------------------------
//  Begin duration for Debug
//
void Pre::beginDuration(const wxString & _namefunction)
{
// date
	Mes = Lf + "==> ";
	Mes += _("Start of") + quote(_namefunction) + ": " + date();
	_printWarn(Mes);
	m_start = clock();
	Mes.Clear();
}
///-----------------------------------------------------------------------------
//  End duration for Debug
//
void Pre::endDuration(const wxString & _namefunction)
{
	Mes = "<== " ;
	Mes += _("End of") + quote( _namefunction  ) + ": " + date();
	Mes += " -> " + duration();
	_printWarn(Mes);
	Mes.Clear();
}
///-----------------------------------------------------------------------------
//  Begin time 'S'
//
// Called by :
//  1.  Pre::Init(const wxString _type):1,
//
wxUint64 Pre::beginTime()
{
	m_begin = clock()/1000;
	if (m_Linux)     m_begin /= 1000;

	return m_begin;
}
///-----------------------------------------------------------------------------
//  End time 'S'
//
wxUint64 Pre::endTime()
{
	m_end = clock()/1000;
	if (m_Linux)     m_end /= 1000;

	return m_end;
}
///-----------------------------------------------------------------------------
//  Elapsed time 'S'
//
// Call by :
//      1. wxString Pre::elapsedTimeStr(const wxUint64 _begin):1,
//
wxUint64 Pre::elapsedTime(const wxUint64 _begin)
{
	if (_begin)
	{
		m_begin = _begin;
	}
	m_end = clock()/ 1000;
	if (m_Linux)     m_end /= 1000;

	wxUint64 elapsed = m_end - m_begin;

	return elapsed ;
}
///-----------------------------------------------------------------------------
//  Elapsed time 'S' in string
//
//  Called by :
//      1. bool Pre::listing(const wxUint32& _posWS):1,
//      2. bool Pre::endaction(const wxChar& _mode):1,
//
//  Call to :
//      1. wxUint64 Pre::elapsedTime(const wxUint64 _begin):1,
//
wxString Pre::elapsedTimeStr(const wxUint64 _begin)
{
    //======================================================================
	wxUint64 dure = elapsedTime(_begin), minu = dure / 60, sec = dure % 60 ;
	//======================================================================

	wxString str = strInt(minu) + Space + _("minute(s)") + ", " ;
	str += strInt(sec) + Space +_("second(s)");

	return str;
}

///-----------------------------------------------------------------------------
// Give a string from a 'wxArrayString' for debug
//
// Called by  :  for debug a table
//
wxString Pre::allStrTable(const wxString& _title, const wxArrayString& _strarray)
{
_printD("=> Begin 'Pre::allStrTable(...)");

	wxString mes = "--------- debug --------------";
	mes += Lf + quoteNS( m_Dirproject) ;
	mes += Lf + "=>" + quote( _title ) ;
	wxUint16 nl = _strarray.GetCount();
	mes += " : " + strInt(nl) + Space + _("lines") ;
	for (wxUint16 u = 0; u < nl; u++)
	{
	// read strarray line from  '1' ... 'nl'
		mes += Lf + strInt(u+1) + "- " + _strarray.Item(u) ;
	}
	mes +=  Lf + "--------- end  debug ------------" ;

_printD("	<= End 'Pre::allStrTable(...)");

	return mes;
}
// ----------------------------------------------------------------------------
