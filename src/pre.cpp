/***************************************************************
 * Name:      pre.cpp
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2020-05-10
 * Modified:  2023-01-17
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
	m_Win = true; m_Lin = m_Mac = false;
#elif defined(__WXGTK__)
	m_Lin = true ; m_Win = m_Mac = false;
#elif defined(__WXMAC__)
	m_Mac = true; m_Win = m_Lin = false;
#endif
    // language filled in 'CreateForWx::m_lang' and 'CreateForQt::m_lang'
	m_lang = wxLocale::GetLanguageCanonicalName(wxLocale::GetSystemLanguage());
//	_printError("Pre::m_lang =" + quote(Pre::m_lang));
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
// all managers
	m_pM = nullptr; m_pMprj = nullptr; m_pMam = nullptr; m_pMed = nullptr;
	m_pMconf = nullptr;
// others pointers
	m_pProjects = nullptr;  m_pProject = nullptr;
	m_pProjectleader = nullptr;
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
	if (m_Lin)
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
// show lang = "xx_XX"
    Mes += "-" + quoteNS(m_lang) ;

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
//  1. Collector::OnMenuListPrj(wxCommandEvent& _pEvent):1,
//  2. Collector::OnMenuExtractPrj(wxCommandEvent& _pEvent):1,
//  3. Collector::OnMenuListWS(wxCommandEvent& _pEvent):1,
//  4. Collector::OnMenuExtractWS(wxCommandEvent& _pEvent);1,
//
bool Pre::isInitialized()		{return m_Init;}

// Called by :
//  1. Collector::OnMenuListPrj(wxCommandEvent& _pEvent):1
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
//  1. Collector::OnMenuListPrj(wxCommandEvent& _pEvent):1,
//  2. Collector::OnMenuExtractPrj(wxCommandEvent& _pEvent):1,
//  3. Collector::OnMenuListWS(wxCommandEvent& _pEvent):1,
//  4. Collector::OnMenuExtractWS(wxCommandEvent& _pEvent):1,

bool Pre::hasNoproject()         {return m_nbProjects == 0 ;}

// Called by :
//	1. Collector::OnMenuListExtractPrj(wxCommandEvent& _pEvent):1
//	2. Collector::OnMenuListExtractWS(wxCommandEvent& _pEvent):1
//
bool Pre::isAllRight()  {return m_Init && ! m_Stop && ! m_Cancel && m_nbProjects;}

///-----------------------------------------------------------------------------
// Give the date followed by the time of construction of the plugin
//
// Called by :
//     1. AddOnForQt::OnAttach()
//
wxString Pre::GetDateBuildPlugin()
{
    wxString str = _("date error !");
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
		Mes = "!!" + _("The file") + quote(m_Nameconf)+ _("is empty") + " !!";
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
    //===================
	m_Init = Pre::Init();
	//===================
	if (! m_Init)		return false;
// initialze 'm_to' to actual time
	m_tL0 = elapsed(0);
// close the files 'project_name.list', 'project_name.dup' 'project_name.extr' , 'project_name.po'
    //========================
    Pre::closeAllExtraFiles();
    //========================
// lines header 'm_FilesCreatingPOT'
    m_nbHeader =  _nbstr;
// workspace or not
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
    // initialze 'm_tE0' to actual time
        m_tE0 = elapsed(0);
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
    //===================
    m_Init = Pre::Init();
    //===================
    if (!m_Init)	return false;

// initialze 'm_tL0' to actual time
    m_tL0 = elapsed(0);

// launch listingWS()
    //==========================
    bool ok  = Pre::listingWS();
    //==========================
    if (!ok)
    {
        /// TODO ...
    }

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
    // initialze 'm_tE0' to actual time
        //=================
        m_tE0 = elapsed(0);
        //=================
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
//1- message
	// work begin
	_print(Lf + Tab + Line(77));
	wxUint32 nprj =  m_pMprj->GetProjects()->Count();
	Mes = "===> " + _("begin 'Extract workspace' on") + Space + strInt(nprj);
	Mes += Space + _("projects") ;
    //=============================
    Mes += Lf + Tab + Pre::date() ;
    //=============================
	_print(Mes);
	m_Fileswithstrings.Add(Mes);
	// test project number
	if (hasNoproject())   return false;

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
	if (pPrj && index >= 0 && index < int(m_nbProjects))
	{
	// finded project
        //===============================
		ok = extraction(FIRST_PRJ, pPrj);
		//===============================
		if(!ok)
		{
		    // TODO ...
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

//1- clean 'm_FileStrcreated'
    //========================
	Pre::initFileStrcreated();
	//========================

//2- message
	// work begin
	wxUint32 nbPrj =  m_pMprj->GetProjects()->Count();
	if (!nbPrj)
    {
    // no project
      //  Mes = _("No project loaded") + " !!";
      //  _printError(Mes);
        return false;
    }
    _print(Tab + Line(77));
	Mes = "===> " + _("begin 'List workspace' on") + Space + strInt(nbPrj);
	Mes += Space + _("projects") ;
    //=============================
    Mes += Lf + Tab + Pre::date() ;
    //=============================
	_printWarn(Mes);
	m_Fileswithstrings.Add(Mes);

//3- begin workspace
	m_Workspace = m_State == fbListWS || m_State == fbExtractWS || m_State == fbListExtractWS;
	m_nbStringsWS = 0; m_nbStrWS = 0; m_nbXmlWithlessString = 0;
	// These two tables must remain synchronous in size
    m_FileswithI18n.Clear();
	m_PrjwithI18nWS.Clear();
	// projects name array clean
	m_NameprjWS.Clear();

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
		ok = releaseProject(pActualprj, NO_POT);
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
            Mes = space(3) + "## " + strInt(m_nbStrWS) + Space ;
            Mes += _("all detected code/res strings");
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
//_printError("Pre::listingWS() => m_DirprojectLeader = " + quote(m_DirprojectLeader) );

//8- duration and save list
        //=========================
		good = Pre::endaction('L');
		//=========================
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
//	4. CreateForWx::creatingPot(bool _prjfirst):1,
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
	// without modify 'Namepot' if exists
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
		wxInt32 choice = cbMessageBox(Mes, "!!!" + _("Warning") + " !!!"
									  ,wxICON_QUESTION | wxYES_NO);
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
		if (m_State != fbListExtractPrj)
		{
			_print(Line(110));
		}
		Mes = space(3) + _("Build file") + quote(m_Dirlocale + m_Shortnamedup);
		//=============================
        Mes += Lf + Tab + Pre::date() ;
        //=============================
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
//_printError("Pre::extraction(...) => m_DirprojectLeader = " + quote(m_DirprojectLeader) );
	// create *.pot, call inherited
        //=============================
		bool ok = creatingPot(_prjfirst);
		//=============================
		if (! ok)
		{
			if (! m_Stop)
			{
				Mes = "Pre::extraction(...) : ";
				Mes += _("Unable to extract file") + quote(m_Shortnamedup) + " !!";
				//==================
				Pre::ShowError(Mes);
				//==================
				_printError(Mes);
				m_Fileswithstrings.Add(Mes);
				//==================
				Pre::endaction('E');
				//==================
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
				Mes += quote(m_Shortnamedup) + "**" + Eol;
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

	bool good = true, del  = false;
//0- choice for delete
	Mes = Lf + "   --> " + _("begin") + Space + quote(_("Replaces unwanted characters")) ;
	_printWarn(Mes);
	m_Fileswithstrings.Add(Mes);
// TODO : go to 'Settings' ...
	if (m_Warnpot.IsEmpty())
	{
		Mes  = Tab + "1- " + _("Replace some macro") + "..."  + Lf;
		Mes += Tab + "2- " + _("Find and replace :") + Lf;
		Mes += Tab + "  2-1- '\\r' -> ''" + Lf;
		Mes += Tab + "  2-2- '$$...' -> '$...'" + Lf;
		Mes += Tab + "  2-3- '%%...' -> '%...'";
		_print(Mes);
		m_Fileswithstrings.Add(Mes);
		del = true;
	}
	else
	{
		Mes = Tab + "** "  +  _("Some selected strings contains the character") ;
		Mes += quote("\\r")  ;
		Mes +=  _("which is not tolerated by 'xgettext'") +  ", " ;
		Mes += _("also they will not be taken into account") ;
		Mes +=  "..."  ;
		_printWarn(Mes);
		m_Fileswithstrings.Add(Mes);
		del = true;
	}
// end
	Mes = "   <-- " + _("end") + Space + quote(_("Replaces unwanted characters")) ;
	_printWarn(Mes);
	m_Fileswithstrings.Add(Mes);

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
			Mes += quote(m_Shortnamedup);
			_print(Mes);
			//==================
			Pre::ShowError(Mes);
			//==================
			wxChar mode = 'E';
			if (m_goodListing) 	mode = 'L';
			//====================================================
			good = Pre::saveArrayToDisk(m_Fileswithstrings, mode);
			//====================================================
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
//_printError("m_Namepo :" + quote(m_Namepo)) ;
	wxString shortname = m_Namepo.AfterLast(cSlash).Prepend(m_Dirlocale);
//_printError("shortname :" + quote(shortname)) ;
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
	// Launching 'Translator' synchronous mode
		Mes = "   ==> " + _("Launching") + quote(EXTERN_TRANSLATOR);
		_printWarn(Mes);
		m_Fileswithstrings.Add(Mes);
	// synchronous
		//============================================
		wxInt32 ret = LaunchExternalTool(m_PathPexe);
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
    wxString longname = _namefile, shortname = longname.AfterLast(cSlash);
    // '*.mo' created ?
	bool ok = wxFileName::FileExists(longname);
	if (ok)
	{
	// display 'm_pCollectLog'
	  //  SwitchToLog();
		Mes = Tab + "**" + quote(EXTERN_TRANSLATOR) + _("has created") ;
		Mes += quote(shortname) + _("to directory") + " :" ;
		_printWarn(Mes) ;
		Mes =  Tab + quote(longname.BeforeLast(cSlash) + cSlash) ;
		_print(Mes) ;
		Mes = "   <== " + _("end") + quote(EXTERN_TRANSLATOR);
		_printWarn(Mes);

	// copy this file to another location ?
		wxString title = _("Copy the file") + quote(shortname) ;
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
                Mes =  Tab + quote(longname.BeforeLast(cSlash) + cSlash);
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

    //============================
	if (! Pre::Initbasic(_type) )	return false;
    // call inherited virtual
	if (! initTailored(_type) )	    return false;
	//=============================
	// actualize booleens
	m_Pexeishere = ! m_PathGexe.IsEmpty();
	m_Gexeishere = ! m_PathGexe.IsEmpty();
	m_Mexeishere = ! m_PathMexe.IsEmpty();
	m_Uexeishere = ! m_PathUexe.IsEmpty();

	m_Cancel = false; m_Stop = false;

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
		m_Uexe = "msguniq.exe";
	// configuration CodeBlocks file name
		m_Nameconf = "default.conf";
	}
	else
	if (m_Lin)
	{
		m_Eol = Lf;
		m_Pexe = "poedit"; m_Gexe = "xgettext";	m_Mexe = "msgmerge";
		m_Uexe = "msguniq";
		m_Nameconf = "default.conf" ;//  ???
	}
	else
	if (m_Mac) // TO VERIFY ...
	{
		m_Eol = Cr;
		m_Pexe = "poedit"; m_Gexe = "xgettext";	m_Mexe = "msgmerge";
		m_Uexe = "msguniq";
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
//_printError("Pre::Initbasic() => m_DirprojectLeader = " + quote(m_DirprojectLeader) );
	// targets associated with a file
	m_Targetsfile.Clear();
	// all projects
	m_pProjects = m_pMprj->GetProjects();
	if(m_pProjects && m_pProjects->IsEmpty())
        return false;

    m_nbProjects = m_pProjects->Count();
    if (hasNoproject())   return false;

//7- index leader project
    //================================================
    wxInt32 index = Pre::indexProject(m_NameprojectLeader);
    //================================================
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
        //====================================
		bool ok = Pre::createDir(m_Dirlocale);
		//====================================
		if (ok)
		{
		//9.2 'm_Temp''
            //==========================
			ok = Pre::createDir(m_Temp);
			//==========================
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
	// work begin
        wxString mes = Tab + _("Work path") + " :" + quote(m_Dirproject);
        _print(Tab + Line(mes.Len()));
		Mes = Tab + _("The active project") + Space + quoteNS(m_Nameproject);
		Mes += ", " + strInt(m_nbFilesprj) + Space + _("files");
        //=============================
        Mes += Lf + Tab + Pre::date() ;
        //=============================
		_printWarn(Mes);
		_print(mes);
		m_Fileswithstrings.Add(Mes + mes);

		m_DirprojectLeader = m_Dirproject.Mid(0, m_Dirproject.Len());
//_printError("Pre::listing() => m_DirprojectLeader = " + quote(m_DirprojectLeader) );
	}
//2- project leader  or  '_posWS == 0'
	bool leaderprj =_posWS == m_indexPrjLeader;
	if (leaderprj && m_Workspace)
	{
        Mes = Lf + Tab + _("'List from workspace' with KEYWORD") + " = ";
        Mes += quote(m_Keyword) + "..." + Lf ;
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
        _print(line); _print(Mes); _print(line);
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

	Mes = Lf + "---> " + _("begin 'List from project'") ;
	if (m_Workspace)   Mes += "-" + strInt(_posWS) + "'";
	else    Mes += Space + _("with KEYWORD") + " =" + quote(m_Keyword) + "...";

  	_printWarn(Mes);
	m_Fileswithstrings.Add(Mes);
// strings number estimation !
    Mes = "** " + _("Estimated number of strings to translate") + ", " ;
	Mes += _("wait a little bit")  + " ..." ;
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
	if (!m_Workspace)   m_FileswithI18n.Clear();

//5-1- find elegibles files and calculate the strings detected number
    //=====================================
	m_nbListStrings = Pre::findGoodfiles();
	//=====================================
	// actual time
	m_tI = elapsed(0) ;
	Mes =  "** ... " + strInt(m_nbListStrings) ;
    Mes += Tab + _("duration") + " = " + dtimeStr(m_tI - m_tL0);
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
	BuildLogger::g_MaxProgress = m_nbListStrings + wxUint32(m_nbListStrings/5) ;
	BuildLogger::g_CurrentProgress = 0 ;

/// debug
//_printError("Listing : g_MaxProgress = " + strInt(BuildLogger::g_MaxProgress) + " marks");
/*
// debug
if (m_Workspace)
{
_printWarn("m_indexFirstFile = " + iToStr(m_indexFirstFile) + " => " + m_FileswithI18n.Item(m_indexFirstFile));
_printWarn("m_indexLastFile = " + iToStr(m_indexLastFile) + " => " + m_FileswithI18n.Item(m_indexLastFile));
}
*/
	wxInt32 nbfiles = m_indexLastFile - m_indexFirstFile ;
    Mes = Lf + Tab  + "# " + _("There are") + Space + iToStr(nbfiles) + Space ;
    Mes += _("elected files") + " ..." ;
	_printWarn(Mes);
	m_Fileswithstrings.Add(Mes);

//5-2- find elegibles files
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
//		1. Pre::saveArrayToDisk (const wxArrayString& _strarray,  wxChar _mode):1,
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
        _print(space(3) + Line(Mes.Len()) );
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
// '*.po' and size  => display size '*.po'
    if (_mode == 'E')
    {
        wxString namepo = m_Namepo.AfterLast(cSlash).Prepend(m_Dirlocale);
		Mes = Tab + _("to the file") + quote(namepo);
        Mes += " (" + strInt(m_sizeFilePo) + Space + _("bytes") + ")";
        _printWarn(Mes);
        m_Fileswithstrings.Add(Mes);
    }
// Experimental
// just for statistics on '*.xml' files
    if (m_nbXmlWithlessString)
    {
        Mes = Tab + "*** " + _("There exists") + Space + strInt(m_nbXmlWithlessString);
        Mes += Space + _("files 'xml', who are not translatable strings") + "...";
        _printError(Lf +Tab + Line(Mes.Len()) );
        _printWarn(Mes);
        _printError(Tab + Line(Mes.Len()) );
    }

// file opened
    //========================================================
	bool ok = Pre::saveArrayToDisk(m_Fileswithstrings, _mode);
	//========================================================

// end messages
    if (_mode == 'E')
    {
        m_goodListing = false;
    // actual time - old time
        m_dtE = elapsed(0) - m_tE0;
        Mes = Lf + Tab + _("duration") + " = " + dtimeStr(m_dtE);
        _print(Mes);
        m_Fileswithstrings.Add(Mes);

        Mes = Lf + "<=== ";
        if (!m_Workspace)    Mes += _("end 'Extract from project'");
        else                 Mes += _("end 'Extract from workspace'");
    }
    else
// work end
    if (_mode == 'L')
    {
    // actual time
        m_dtL = elapsed(0) - m_tL0;
        Mes = Lf + Tab + _("duration") + " = " + dtimeStr(m_dtL);
        _print(Mes);
        m_Fileswithstrings.Add(Mes);

        Mes = Lf + "<--- " ;
        if (!m_Workspace)   Mes += _("end 'List from project'") ;
        else                Mes += _("end 'List from workspace'") ;
    }

	Mes += " : " + _("all right") + Dot + Lf;
	_printWarn(Mes);
	m_Fileswithstrings.Add(Mes);

	if (_mode == 'E')
    {
    // total duration : List and Extract
        Mes =  _("Total duration") + " : " + dtimeStr(m_dtL + m_dtE) + Lf;
        _print(Mes);
        m_Fileswithstrings.Add(Mes);
    }

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
	// cell 0 for active project
		m_FileStrcreated.Add(m_NameprojectLeader, 1);
		nfc++;
	}
	else
	{
	// verify cell 0 for active project
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
    // the directory of the current project
    m_FileswithI18n.Add(m_Dirproject);
//_printError(m_FileswithI18n.Last());
    // the first file of the current project
    m_indexFirstFile = m_FileswithI18n.Index(m_Dirproject) + 1;
    // project name
	//if (! m_Workspace)      m_PrjwithI18nWS.Add(m_Nameproject);
	if (m_Workspace)
	{
        m_PrjwithI18nWS.Add(m_Nameproject);
//_printWarn(iToStr(m_PrjwithI18nWS.GetCount()) + " projects I18n " + m_PrjwithI18nWS.Last());
    }

    wxInt32  index, nbmax;
	m_Cancel = false;
	bool  okcode, okres, okxml;
	bool sameprj = false, finded = false;
	wxString ext, result;
// control to pending messages in the event loop
	//m_pM->Yield();

// all files of current project : !!! targets should be explored too !!!
    // for modulo calculate : it's a compromise
    wxUint32 val = m_nbFilesprj;
    if (val > 25)  val = m_nbFilesprj/25;
//_printError(strInt(val));

	for (wxUint32 nf = 0; nf < m_nbFilesprj; nf++)
	{
	/// Mandatory to avoid slowing down the estimation !!
	// control to pending messages in the event loop
      //  m_pM->Yield();
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
	/// for DEBUG
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
//_printError("nbStr = " + iToStr(nbStr) + " in " + quote(filein) );
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
        // index of the first from begin ?
			index = m_FileswithI18n.Index(filein);
			finded = index != -1 ;
			sameprj = false;
		// file exists already
			if (finded)
			{
			// preventing the indexes of the two tables from becoming out of sync
				nbmax = m_PrjwithI18nWS.GetCount();
				Mes = "Pre::findGoodfiles() => m_FileswithI18n::index = " + iToStr(index);
				Mes += ", m_PrjwithI18nWS::max = " + iToStr(nbmax) ;
				wxASSERT_MSG( index < nbmax , Mes);
			// assert  before
                sameprj = m_PrjwithI18nWS.Item(index) == m_Nameproject;
            }
		// not finded or not same project
			if (!finded || !sameprj)
			{
				m_FileswithI18n.Add(filein);
				m_PrjwithI18nWS.Add(m_Nameproject);
				good = true;
			}
		} // workspace
	} // end for nf

    m_indexLastFile = m_FileswithI18n.Index(m_FileswithI18n.Last(), true, true) ;
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
        m_indexFirstFile = 0;
        m_indexLastFile = 0;
        if (nbFiles)    m_indexLastFile = nbFiles-1;
    }

	// only a directory !
    if (nbFiles < 1) return 0;

// begin : only the part used of current projct
	for (wxUint32 nf = m_indexFirstFile; nf <= m_indexLastFile  ; nf++)
	{
    // control to pending messages in the event loop
		m_pM->Yield();
	// analysis stopped manually
		if (m_Stop)
		{
			good = false;
			break;
		}
    // all names
        name =  m_FileswithI18n.Item(nf);
        // it's a directory => other project
        if (name.Last() == cSlash) continue ;
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
	//1- extensions '*.h, *.cpp, *.hpp, *.cxx, *.hxx, *.script' : code C++
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
		// debug
		//	listFiltered("okcode");
		}
		else
	//2- extensions '*.wxrc, *.wks'
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
		// debug
		//	listFiltered("okres");
		}
    //3- extensions '*.xml'
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
//_printError("Xml:: nbStr = " + strInt(nbStr))
		// debug
		//	listFiltered("okxml");
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

// -----------------------------------------------------------------------------
// Execute 'xgettext' on a file and retrieve errors and result
//
// Called by :
//		1. CreateForWx::listStringsCode(wxString):1,
//		2. CreateForWx::listStringsXmlIts(const wxString& _shortfile, wxString _rulesIts)

//		//4. CreateForQt::listStringsCode(wxString _shortfile)
//		//6. CreateForQt::creatingPot(bool _prjfirst)

// Call to :
//		1. Pre::executeAndGetOutputAndError(const wxString& _command, const bool& _prepend_error):1,
//		2. Pre::xgettextWarn(const wxString& _txt, const bool& _withpot):3,
//		3. Pre::xgettextOut(wxString ):2,
//
wxInt32  Pre::GexewithError(const wxString& _shortfile, const wxString& _command,
							const bool& _prepend)
{
_printD("=> Begin 'Pre::GexewithError(" + _shortfile + ", " + strBool(_prepend) +  ")'");

// command, error alone, error in first :: see 'src\msw\utilsexec.cpp'
	//====================================================================
	wxString result = Pre::executeAndGetOutputAndError(_command, PREPEND);
	//====================================================================
// no chains
	if (result.IsEmpty() && _prepend)		return 0;

//_printError("result = " + quote(result) );
	wxUint32 nstrBefore,  nbStr = 0 ;
	wxInt32 pos, posw;
	bool with_warn = false, with_string = false;;;
	wxString msg, tmp, header, txt;
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
			// for xml :
			//	1- "StartTag: invalid element name" (malformed)
			//	2- "Premature end of data" => 'cannot be read'
			if (pos !=  wxNOT_FOUND)
			{
				result = result.BeforeLast('\n');
				msg = _("Error") + " : " ;
				if (result.Contains("StartTag:")) msg += _("'Xml' malformed") ;
				if (result.Contains("Premature end of data")) msg += _("Xml cannot be read");
				msg += Eol + "=>" + quote(result) + "<=";
				m_Fileswithstrings.Add(msg);
				_printError(msg);
				return  wxNOT_FOUND;
			}
		// ---------------------------------------------
        // a line contains '« \r »' : warning and string
        //----------------------------------------------
        /// message filtering like ( both lines are on one line )
// 'cpp_test/cctest_frame.cpp:327:
//  AVERTISSEMENT : un message à traduire ne doit pas contenir de séquence d'échappement « \r »'
/// suivi de
// #: cpp_test/cctest_frame.cpp:107
// msgid "CC Testing"
// msgstr ""
/// message filtering like ( both lines are on one line )
// 'cpp_test/cctest_frame.cpp:327:
//  AVERTISSEMENT : un message à traduire ne doit pas contenir de séquence d'échappement « \r »'
/// suivi de
// #: cpp_test/cctest_frame.cpp:107
// msgid "CC Testing"
// msgstr ""
			posw = result.Find("#:");
			// it's an warning  message for text contains '\r'
			if (posw !=  wxNOT_FOUND)
			{
// txt = result.BeforeFirst('#');
//_printError("contains '#:' in posw = " + iToStr(posw) + Lf + markNS(txt) );
            // delete before '#:'
                result = result.Mid(posw) ;
                with_string = true;
//_printWarn(result);
			}
			else
            {
        // -----------------------------------------
		// no error but possibly warning and string
		// -----------------------------------------
		// string ?
                pos = result.Find("#");
                with_string = pos !=  wxNOT_FOUND;
            // warning ?
                posw = result.Find("warning:") ;
                with_warn = posw !=  wxNOT_FOUND;
            }
		// ----------------------------------------

	// --------------
	// 1- string only
	// --------------
			if (!with_warn && with_string)
			{
//_printWarn("	1 => !with_warn && with_string ...");
			// 1.1
				if (m_Formatxgettext)
				{
				// all strings to extracts
                    //=============================
					msg = Pre::xgettextOut(result);
					//=============================
//_printError(quote(msg));
				}
			//1.2- numbers strings
                if (m_goodListing)      nbStr = m_nbListStrings - nstrBefore;
                else                	nbStr = m_nbExtractStrings - nstrBefore;
			// header
				++m_nbFileEligible;
				header = Lf + Tab + "E" + strIntZ(m_nbFileEligible, 4) + "-" ;
				header += quote(_shortfile) + " ==> " + strInt(nbStr) + Space + _("string(s)");
				_printWarn(header);
				m_Fileswithstrings.Add(header);
			// list numbers line
				m_Fileswithstrings.Add(msg);
			// display 'tabmes' line by line
				wxString line;
				tabmes = ::GetArrayFromString(msg, Lf, TRIM_SPACE);
				nbStr = tabmes.GetCount();
				for (wxUint32 n = 0 ; n < nbStr; n++)
				{
					line = tabmes.Item(n);
					line.Prepend(Tab + space(3)) ;
				// uses 'Collector::MesToLog(...)'
					if (tabmes.Item(n).Left(1) == '!')	printWarn(line);
					else								print(line);
				}
			}
			else
	// -------------------
	// 2- warning + string
	// -------------------
			if (with_warn && with_string)
			{
//_printWarn("	2 => with_warn && with_string ...");
			// get warning before the first '#'
				msg = result.Mid(0, pos);
//_printError(markNS(msg));
				// formatting
				//===================================
				msg = Pre::xgettextWarn(msg, NO_POT);
				//===================================
				msg.Prepend(Tab + "    * " + _("Warning(s)") + " : " + Eol) ;
				m_Fileswithstrings.Add(msg);
				_printWarn(msg);
			// delete warning inside 'result'
				result.Remove(0, pos);
			// output file
				msg = Tab + "    ** " + _("translatable strings") + " : " + Eol;
				if (m_Formatxgettext)
				{
				// _print file name if elegible file
                    //==============================
					msg += Pre::xgettextOut(result);
					//==============================
                    //_printWarn(msg);
				}
				m_Fileswithstrings.Add(msg);
			// print 'Collector log'
                if (m_goodListing)  	nbStr = m_nbListStrings - nstrBefore;
                else                	nbStr = m_nbExtractStrings - nstrBefore;
				Mes += _("string(s)") + " ...";
				_print(Mes);
			// display is realized by 'xgettextOut'
				tabmes = ::GetArrayFromString(msg, Lf, TRIM_SPACE);
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
//_printWarn("	3 => with_warn && !with_string ...");
			// get total warning
				msg = result;
			// formatting
                //===================================
				msg = Pre::xgettextWarn(msg, NO_POT);
				//===================================
				msg.Prepend(Tab + "    * " + _("Warning(s)") + " : " + Eol);
				m_Fileswithstrings.Add(msg);
				_print(msg);
			}
	// ------------------------
	// 4- impossible : see away
	// ------------------------
			else
			{
//_printWarn("	4 => !with_warn && !with_string ...");
			}
		} // result is no empty
	} // end _prepend

// ------------------------------------
// B- for 'creatingPot()'  extract = true
// ------------------------------------
	if (! _prepend)
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
				msg = result.Remove(0, pos);
			// print
				Mes = Tab + "    * " + _("Warning(s) inside") ;
				Mes += quote(_shortfile) + Eol;
				_print (Mes);
			// formatting output file
                //===================================
				msg = Pre::xgettextWarn(msg, NO_POT);
				//===================================
				Mes += msg;
				m_Fileswithstrings.Add(Mes);
				nbStr =  wxNOT_FOUND;
			} // end pos
		} // end with_warn
		else
		{
		// it's an error  message : NO TRANSLATE HERE
			msg = _("No such file or directory");
			pos = result.Find(msg);
			if (pos !=  wxNOT_FOUND)
			{
				msg +=  " :" + Eol ;
				msg += markNS(result.BeforeLast('\n'));
				m_Fileswithstrings.Add(msg);
				_printError(msg);
				return  wxNOT_FOUND;
			}
			else
			{
			// an other error ...
				msg = markNS(result);
			///	m_Fileswithstrings.Add(msg);
				_printWarn(msg);
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
wxString Pre::xgettextWarn(wxString & _txt, const bool& _withpot)
{
_printD("=> Begin 'Pre::xgettextWarn(" + _txt + ", " + strBool(_withpot) + ")'");

// mandatory 'Lf' not Eol !
	wxArrayString tabwarn = ::GetArrayFromString(_txt, Lf, NO_TRIM_SPACE);
	wxString line, linebefore = wxEmptyString, tmp = wxEmptyString;
	wxInt32 pos, nl = tabwarn.GetCount();
	for (wxInt32 u = 0; u < nl; u++)
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
		if (_withpot == false && (line.Find(m_Shortnamedup) ==  wxNOT_FOUND) )
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
	// analyse '*.dup'
		if (_withpot && (line.Find(m_Shortnamedup) != wxNOT_FOUND) )
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

_print("	<= End 'Pre::xgettextWarn(...) => " + tmp + ")'");

	return tmp;
}

// -----------------------------------------------------------------------------
// Generate out file for '*.po' : remove banned strings
//
// Called by :
//	1. Pre::GexewithError(wxString _shortfile, wxString _command, bool _prepend):2,
//
// Call to : none
//
wxString Pre::xgettextOut(wxString& _txt)
{
_printErrorD("=> Begin 'Pre::xgettextOut(" + markNS(_txt) + "')");
// --------------------------------------------------------
// One PO file entry has the following schematic structure:
//
// white-space or 'Lf'
// #  translator-comments
// #. extracted-comments
// #, flag...
// #| msgid previous-untranslated-string
// #: reference...
// #: reference...
// msgid untranslated-string
// msgstr translated-string
// ---------------------------------------------------------

// extract lines
	wxArrayString tabout = ::GetArrayFromString(_txt, Lf, NO_TRIM_SPACE);
	wxString line, lineplus, linepot, result = wxEmptyString, tmp, strNbline, refbanned;
	wxChar ca, cb;
	bool  endheader = false,
		  plural = false , several = false, banned = false;
	wxUint32 nbl = 0, nbLines = tabout.GetCount(), nbr
			 ,nbrefbanned = 0, nbbeginblock =0, nbendblock = 0;
	m_ArrayFiltered.Clear();
	m_Strwitherror = false;
// all lines of 'tabout'
	for (wxUint32 u = 0; u < nbLines ; u++)
	{
		linepot = line = tabout.Item(u);
//_printWarn(iToStr(u) + " : " + quote(line) );
		if (line.IsEmpty())
            continue;
	// no space (...) on left
		line.Trim(FROM_LEFT);
/// TO VERIFY
// delete all '\r'
//nbr = line.Replace("\\r", "", ALL_TXT);
//if (nbr)   _printError(iToStr(nbr) + " in " + quote(line) );
	/// "# " => translator comments
		if (line.StartsWith("# ") )
			continue;

		ca = line.GetChar(0);
	/// '#'
		if (ca == '#')
		{
		/// ---------------- begin block ---------------------------
		///	#. traductor comment
		/// #: files references
		/// #: src\pre.cpp:1025 src\pre.cpp:1034 src\pre.cpp:1043
		///	#: src\pre.cpp:1078
		///	#, flag ...
		///	#|	msgid previous-untranslated-string
		///	msgid "Text to recovered"
		/// msgstr ""
		/// space // is a separator of blocks
		/// ------------------End block -----------------------------
		// second char in // cb in [':', '.', ',', '|']
			cb = line.GetChar(1);
			/// '.' == automatic comments (ex  (itstool) path: .
			/// ',' == flags : fuzzy, c-format, ...
			/// '|' ==  msgid previous-untranslated-string			...
			if (cb == ','  || cb == '|'	|| cb == '.')
                continue;

		// references
			if (cb == ':')
			{
			// the first ':' => begin first block or end of header
				if (!endheader)
				{
					endheader = true;
					nbbeginblock = u;
				}
			/// #: files references
			/// => :L1025:L1034:L1043:L1078  => "Text to recovered"
			// retrieve the different line numbers
				do
				{
					strNbline = line.AfterLast(':');
					tmp.Prepend(":L" + strZ(strNbline, 5)) ;
					line = line.BeforeLast(' ');
				}
				while (line != "#:") ;
				nbrefbanned = u;
			}
		}
		else
	/// new line 'msgid + txt'
		// 	'msgid  ".............\n"'
		if (line.StartsWith("msgid"))
		{
		// used 'tmp' for log message
			tmp.Prepend(Tab + "  "); tmp += " ==> ";
			// after first '"' and before last '"'
			lineplus = line.AfterFirst('"').BeforeLast('"');
		// lineplus == 'msgid text'
			several = lineplus.IsEmpty() ;
		// only one line
			if (!several) 	tmp += dquoteNS(lineplus);
			else            tmp += cDquote;
		// banned lines ?
			if (filterBeginBanned(lineplus))
			{
//_printError("Code::banned =>" + quote(lineplus));
				banned = true;
			// add to temp for '*.list' : prevent user
				tmp.Prepend("!! " + _("BANNED") + " !!") ;
			}
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
		if (line.StartsWith("msgstr"))
		{
			if (several)    tmp += cDquote;
			result +=  tmp + Lf;
			tmp.Clear();
			nbl++;
			several = false ;
		}
		else
// TOVERIFY : msgid_plural ...
		if (line.Find("msgid_plural") != wxNOT_FOUND)
		{
			line.Replace("msgid_plural   ", " =plural=>", FIRST_TXT);
			result +=  line;
			plural = true;
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
				plural = false;
			}
		}
		else
		if (line.StartsWith("msgtxt"))
		{
			/// TODO ...

		}
	// save linepot
		if (banned)
		{
		/// modify the last reference message => obsolete messages marked
			if (linepot.StartsWith("msgid") )
			{
			/// modify the last reference message : '#:' => '#.'
				m_FilesCreatingPOT.Last().Replace(T_MES, T_FLAG);
			/// modify msgid message : => '#.'
				linepot.Prepend(T_FLAG);
			}
			if (linepot.StartsWith("msgstr") )
			{
			/// modify msgstr message : => '#.'
				linepot.Prepend(T_FLAG);
			/// end banned
				banned = false;
			}
		}
		m_FilesCreatingPOT.Add(linepot);
	// separator block
		if (linepot.Contains("msgstr"))		m_FilesCreatingPOT.Add(Lf);
//_printError(quote(linepot));
	}
// total strings
    if (m_goodListing)		m_nbListStrings += nbl;
	else					m_nbExtractStrings += nbl;

_printD("	<= End 'Pre::xgettextOut(...)");

	return result;
}
//------------------------------------------------------------------------------
//	TEST : search the word banned in a string
//
// Called by :
//		1. Pre::xgettextOut(wxString& _txt);1,
//
//#include <wx/arrstr.h>

bool Pre::filterBeginBanned(wxString _line)
{
//_print("code:filterBeginBanned(...) =>" + quote(_line) );
	bool banned = true, beginBanned;
// separate the line with space
	wxString line, rest, word = _line.BeforeFirst(cSpace, &rest);
//_print("first word = " + quote(word));
	banned = beginBanned = m_ArrayBeginBanned.Index(word) != wxNOT_FOUND ;
	// all banned
	while (!rest.IsEmpty() && banned)
	{
		line = rest;
//_printWarn("rest:" + quote(rest) );
		word = line.BeforeFirst(cSpace, &rest);
//_printWarn( "word:" + quote(word) + "rest:" + quote(rest) );
	// is number or find word
		banned = word.IsNumber() || m_ArrayBeginBanned.Index(word) != wxNOT_FOUND ;
//_printError("banned = " + bToStr(banned));
	}
	//
	banned = beginBanned && banned ;

	return banned;
}

//------------------------------------------------------------------------------
//	TEST : search the string filtered and show it
//		for 'code', 'xml', 'resources'
//
// Called by :
//		1. Pre::listGoodfiles():3,
//
void Pre::listFiltered(wxString _txt)
{
    wxUint32 nb = m_ArrayFiltered.Count();
    if  (nb)
    {
	_printError(Dline(4) + " DEBUG " + quote(_txt) + Dline(4));
		_printError("Strings filtered : " + strInt(nb));
		wxString mes;
        for (wxUint32 cell = 0; cell < nb; cell++)
        {
            mes = m_ArrayFiltered.Item(cell);
            _printError(Tab + quoteNS(mes));
        }
	_printError(Dline(23));
    }
}

//------------------------------------------------------------------------------
//	TEST : search the banned string and show it
//		for 'code', 'xml', 'resources'
//
// Called by : none
//
void Pre::listBanned(wxString _txt)
{
    wxUint32 nb = m_ArrayBanned.Count();
    if  (nb)
    {
	_printError(Dline(4) + " DEBUG " + quote(_txt) + Dline(4));
		_printError("Strings banned : " + strInt(nb));
		wxString mes;
        for (wxUint32 cell = 0; cell < nb; cell++)
        {
            mes = m_ArrayBanned.Item(cell);
            _printError(Tab + quoteNS(mes));
        }
	_printError(Dline(23));
    }
}
//------------------------------------------------------------------------------
//	TEST : search the begin banned string and show it
//		for 'code', 'xml', 'resources'
//
// Called by : none
//
void Pre::listBeginBanned(wxString _txt)
{
    wxUint32 nb = m_ArrayBeginBanned.Count();
    if  (nb)
    {
	_printError(Dline(4) + " DEBUG " + quote(_txt) + Dline(4));
		_printError("Strings begin banned : " + strInt(nb));
		wxString mes;
        for (wxUint32 cell = 0; cell < nb; cell++)
        {
            mes = m_ArrayBeginBanned.Item(cell);
            _printError(Tab + quoteNS(mes));
        }
	_printError(Dline(23));
    }
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
    //================================================================
    // asynchronous and no hided
    wxString result = Pre::executeAsyncAndGetOutput(command, PREPEND);
    //================================================================
    if (!result.IsEmpty())
    {
        // Warning : Gdk-WARNING, Gtk-CRITICAL => assertion ...
        //_printWarn(markNS(result));
    }

    return ret;
}
//------------------------------------------------------------------------------
// Execute another program (always asynchronous and no hided).
//
//  call by :
//      1. wxInt32 Pre::LaunchExternalTool(const wxString& _toolexe):1,
//
wxString Pre::executeAsyncAndGetOutput(const wxString& _command, const bool& _prepend_error)
{
_printD("=> Begin 'Pre::executeAndGetOutputAndError(...)'");

	wxArrayString output;
	wxArrayString error;
	//=================================================================
	::wxExecute(_command, output, error, wxEXEC_ASYNC | wxEXEC_NOHIDE);
	//=================================================================

	wxString str_out;

	if ( _prepend_error && !error.IsEmpty())
		str_out += ::GetStringFromArray(error, Eol);

	if (!output.IsEmpty())
		str_out += ::GetStringFromArray(output, Eol);

	if (!_prepend_error && !error.IsEmpty())
		str_out += ::GetStringFromArray(error,  Eol);

_printD("	<= End 'Pre::executeAndGetOutputAndError(...)' =>" + Eol + quote(str_out) );

	return  str_out;
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
	//======================================================
	::wxExecute(_command, output, error, wxEXEC_NODISABLE );
	//======================================================

	wxString str_out;

	if ( _prepend_error && !error.IsEmpty())
		str_out += ::GetStringFromArray(error, Eol);
//_printError("error => " +quote(str_out));

	if (!output.IsEmpty())
		str_out += ::GetStringFromArray(output, Eol);
//_printError("str_out => " +quote(str_out));

	if (!_prepend_error && !error.IsEmpty())
		str_out += ::GetStringFromArray(error,  Eol);
//_printError("error2 => " +quote(str_out));

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
//_printError("Pre::releaseProject() => m_DirprojectLeader = " + quote(m_DirprojectLeader) );
		if (! wxFileName::DirExists(m_Dirpot))
		{
		//  linux : 0755, user : 'rwx', others : 'r-x'
            //============================
			bool ok = ::CreateDir(m_Dirpot);
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
	// name file 'pot' or 'dup'
		m_Shortnamedup =  m_Nameproject;
		if (m_Workspace)	m_Shortnamedup += PROLONG_WS ;
		m_Shortnamedup += DOT_EXT_DUP;
		m_Namedup = m_Dirpot + m_Shortnamedup;
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

    if (hasNoproject())  return nullptr;

	cbProject* pPrj = nullptr;
	_index = wxNOT_FOUND;
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

    if (hasNoproject())  return -1;

    wxInt32 pos = wxNOT_FOUND;
    cbProject* pPrj = nullptr;
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
	if (pose == wxNOT_FOUND)
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
//
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

//1- read "*.dup" => source
	bool ok = wxFileName::FileExists(m_Namedup);
	if (!ok)
	{
		Mes =  Tab + _("Could'not locate file");
		Mes +=  quote(m_Namedup) + "!!";
		_print(Mes);

		return false;
	}
	// read contents of file 'm_Namedup'
	//=================================================
	wxString source = Pre::readFileContents(m_Namedup);
	//=================================================
	if (source.IsEmpty())
	{
		Mes =  Tab + _("Empty file") + quote(m_Shortnamedup) + "!!" ;
		_print(Mes);

		return false;
	}

	m_sizeFilePo = source.Len();

	Mes = Lf + "   --> " +  _("Verify file integrity");
	Mes += quote(m_Shortnamedup);
	Mes += " (" + strInt(m_sizeFilePo) + Space + _("bytes") ;
	Mes += ", " + _("of which header") + " : " + strInt(m_sizeHeaderpo) ;
	Mes += Space + _("bytes") + ")" ;
	_printWarn(Mes);
	m_Fileswithstrings.Add(Mes);

//2- complete header *.dup or *.pot
    //=============================
    ok = Pre::adjustHeader(source);
    //=============================
    if (!ok)
	{
        return false;
	}

//3- fix '*.dup'
	if (_replacecar)
	{
		wxString rc = "\\r";
		wxUint32 nr = 0;
//3-1 find "\r" and delete into valid line
	/// **********************
	/// No longer seems useful
	/// **********************
	//  nr = deleteCr(source);
		Mes = Tab + "2- " + _("Search strings to delete") + " : ";
		Mes += Quote + rc + Quote + Space  + "(" + _("no longer used") + ")" + Lf;
		// result
		if (nr)
		{
			Mes += Tab + "   ... " + _("deleting") + Space + strInt(nr) + Space ;
			Mes += _("character(s)");
		}
		else
		{
			Mes += Tab + "   ... " + _("nothing");
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
	}
	else
	{
		Mes =  Tab + "2- " + _("No replace");
		_print(Mes);
		m_Fileswithstrings.Add(Mes);
	}

/// ======================= files '*.pot' and '*.po =========================

// name 'dup'
	m_Shortnamedup = m_Namedup.AfterLast(cSlash);
// name 'uni'
	m_Nameuni = m_Namedup.BeforeLast(cDot) + ".uni";
	m_Shortnameuni = m_Shortnamedup.BeforeLast(cDot) + ".uni";
// name 'pot'
	m_Namepot = m_Namedup.BeforeLast(cDot) + ".pot";
	m_Shortnamepot = m_Shortnamedup.BeforeLast(cDot) + ".pot";

//_printError("m_Namedup :" + quote(m_Namedup) );

//4- write new '*.dup' for replace old '*.dup'
	Mes = Tab;
	if (_replacecar) 	Mes += "4- ";
	else				Mes += "3- ";
	Mes += _("Write") + " '*.dup' => " + _("disk") + " :";
	_print(Mes);
	m_Fileswithstrings.Add(Mes);

	// write to file '*.dup'
	ok = ::cbSaveToFile(m_Namedup, source);
	if (!ok)
	{
		Mes =  Tab + _("Could'not write file");
		Mes + quote(m_Shortnamedup) + " !!";
		_printError(Mes);

		return false;
	}
	Mes = Tab + space(3) + _("Write modifications to") + quote(m_Shortnamedup);
	_print (Mes);
	m_Fileswithstrings.Add(Mes);

//5- unifie duplicate messages in '*.dup' to '*.pot' with 'msguniq'

	// for 'msguniq' or 'msgmerge'
	wxString result;
	wxInt32 nbdots = 0;
	// for test '*.pot'
	bool potishere = false ;
	if (m_Uexeishere && m_Mexeishere)
	{
		Mes = Tab;
		if (_replacecar) 	Mes += "5- ";
		else				Mes += "4- ";
		Mes += _("Unifie duplicate messages") + " '*.dup' => '*.uni' :" ;
		_print(Mes);
		m_Fileswithstrings.Add(Mes);

	// create '*.uni' withless duplicate
/// m_Workspace : take into account  'm_DirlocaleLeader'
		// ===========================================================
		result = Pre::noDuplicateMsgid(m_Namedup, m_Nameuni);
		// ===========================================================
		if (result.IsEmpty())	Mes = _("There were duplicate strings") + ", ";
		else					Mes = _("None") + Space;
		Mes += _("unified translatable strings to") + quote(m_Shortnameuni);
		Mes.Prepend(Tab + space(3));
		_print(Mes);
		m_Fileswithstrings.Add(Mes);

//5- create '*.pot'
		Mes = Tab ;
		if (_replacecar) 	Mes += "6- ";
		else				Mes += "7- ";
	// 'm_Namepot' exists already ?
		potishere = wxFileName::FileExists(m_Namepot);
		if (!potishere)
		{
			Mes += _("Copy") + " '*.uni' => '*.pot' :"  ;
			_print(Mes);
			m_Fileswithstrings.Add(Mes);

		// copy uni => pot
			ok = wxCopyFile(m_Nameuni, m_Namepot, true);
			if (!ok)
			{
				Mes =  Tab + _("Not copy file") + quote(m_Shortnamepot);
				_print(Mes);

				return false;
			}
			Mes = Tab + space(3) + _("Copy") +  quote(m_Shortnameuni);
			Mes += _("to the file") + quote(m_Shortnamepot);
			_print (Mes);
			m_Fileswithstrings.Add(Mes);
		}
		else
		{
			Mes += _("Merge") + " '*.uni' => '*.pot' :"  ;
			_print(Mes);
			m_Fileswithstrings.Add(Mes);

		// merge '*.uni' and '*.pot'
			//===============================================================
			result = Pre::mergeFiles(m_Shortnamepot, m_Shortnameuni, nbdots);
			//===============================================================
//_printWarn(Tab + quote(result));
			ok  = nbdots >= 0;
			if (!ok)
			{
				Mes =  Tab + _("Not merge files") + quote(m_Shortnameuni);
				Mes += _("inside") + quote(m_Shortnamepot);
				_print(Mes);
			}
		// good
			//if (result.StartsWith(_("done")) )
			{
			// good no verbose
				Mes =  Tab + space(3) +  _("Merge") + quote(m_Shortnameuni);
				Mes += _("inside")  + quote(m_Shortnamepot) ;
				_print (Mes);
				m_Fileswithstrings.Add(Mes);
			// good no verbose : treated strings
				if (result.StartsWith('P') )
				{
					Mes = Tab + Tab + result;
					_printWarn(Mes);
					m_Fileswithstrings.Add(Mes);
				}
			}
		}
	}
	else
	{
		Mes = _("The file") + quote(m_Uexe) + _("or perhaps") + quote(m_Mexe) + _("no exists");
		_printWarn(Mes);
		m_Fileswithstrings.Add(Mes);

		return false;
	}

//6- make a copy '*.pot' => '*.po'
	Mes = Tab ;
	if (_replacecar) 	Mes += "7- ";
	else				Mes += "8- ";
	// name po
	m_Namepo = m_Namepot.BeforeLast(cDot) + ".po";
	m_Shortnamepo = m_Shortnamepot.BeforeLast(cDot) + ".po";
	ok = wxFileName::FileExists(m_Namepo);
	if (!ok)
	{
		Mes += _("Copy") + " '*.pot' => '*.po' :"  ;
		_print(Mes);
		m_Fileswithstrings.Add(Mes);
	// copy
		ok = wxCopyFile(m_Namepot, m_Namepo, true);
		if (!ok)
		{
			Mes =  Tab + _("Not created file") + quote(m_Namepo);
			_print(Mes);

			return false;
		}
		Mes = Tab + space(3) + _("Copy") +  quote(m_Shortnamepot)+ _("to the file");
		Mes += quote(m_Shortnamepo);
		_print (Mes);
		m_Fileswithstrings.Add(Mes);
	}
	else
	{
//7- merge '*.po' and '*.pot'
		//==============================================================
		nbdots = 0 ;
		result = Pre::mergeFiles(m_Shortnamepo, m_Shortnamepot, nbdots);
		//==============================================================
		ok  = nbdots >= 0;
		if (!ok)
		{
			Mes += _("Not merge files") + quote(m_Shortnamepot);
			Mes += _("inside") + quote(m_Shortnamepo);
			_print(Mes);
		}
	// good
		Mes += _("Merge") + " '*.pot' => '*.po' :"  ;
		_print(Mes);
		m_Fileswithstrings.Add(Mes);

		Mes =  Tab + space(3) +  _("Merge") + quote(m_Shortnamepot);
		Mes += _("inside")  + quote(m_Shortnamepo) ;
		_print (Mes);
		m_Fileswithstrings.Add(Mes);
	// good no verbose : treated strings
		if (result.StartsWith('P') )
		{
			Mes = Tab + Tab + result;
			_printWarn(Mes);
			m_Fileswithstrings.Add(Mes);
		}
	}

//8-  end
    Mes = "   <-- " +  _("end verify file integrity") + Lf;
    _printWarn(Mes);

	Mes = wxEmptyString;

_printD("	<= End 'Pre::integrity()'");

	return ok;
}

// -----------------------------------------------------------------------------
//
// Called by :
//  1. Pre::integrity(bool _replacecar):1,
//
// Call to :
//	1. Pre::changeContents(wxString _file, wxString _old, wxString _new):6,

bool Pre::adjustHeader(wxString & _source)
{
    bool ok = false;
    // header *.dup or *.pot
	Mes = Tab + "1- " + _("Adjust and complete the header") + " ...";
	_print(Mes);
	m_Fileswithstrings.Add(Mes);

    // modify the header
	wxString oldtxt, newtxt;

//1- descriptive
	oldtxt = "SOME DESCRIPTIVE TITLE"; newtxt = "$(PROJECT_NAME)";
    //================================================
	ok = Pre::changeContents (_source, oldtxt, newtxt);
	//================================================
	if (!ok)
	{
		_printError("not ok 1 => Pre::changeContents(...)" );
		return false;
	}
//2- year copyright
	oldtxt = "YEAR-MO-DA HO:MI+ZONE" ;
	newtxt = GetDateBuildPlugin();
	//================================================
	ok = Pre::changeContents (_source, oldtxt , newtxt );
	//================================================
	if (!ok)
	{
		_printError("not ok 2 => Pre::changeContents(...)" );
		return false;
	}
//3- package name
	oldtxt = "PACKAGE "; newtxt = "$(PROJECT_NAME)";
	//================================================
	ok = Pre::changeContents  (_source, oldtxt, newtxt);
	//================================================
	if (!ok)
	{
		_printError("not ok 3 => Pre::changeContents(...)" );
		return false;
	}
//4- PACKAGE VERSION
	oldtxt = "PACKAGE VERSION"; newtxt = "$(PROJECT_FILENAME)";
	//================================================
	ok = Pre::changeContents  (_source, oldtxt, newtxt);
	//================================================
	if (!ok)
	{
		_printError("not ok 4 => Pre::changeContents(...)" );
		return false;
	}
//5- Last Translator
	//oldtxt = "FULL NAME"; newtxt = "$PLUGINS";
	//===================================================
	//ok = Pre::changeContents  (source, oldtxt, newtxt);
	//===================================================
	//if (!ok)
	//{
	//	_printError("not ok 5 => Pre::changeContents(...)" );
	//	return false;
	//}

//6- LANGUAGE	"LL"  here ==> 'fr_FR'
	oldtxt = "LANGUAGE"; newtxt = "$(LANGUAGE)";
	//================================================
	ok = Pre::changeContents  (_source, oldtxt, newtxt);
	//================================================
	if (!ok)
	{
		_printError("not ok 6 => Pre::changeContents(...)" );
		return false;
	}
//7- Language  here => 'm_locale.GetCanonicalName()'
	if (m_lang.IsEmpty())
		m_lang = wxLocale::GetLanguageCanonicalName(wxLocale::GetSystemLanguage());
//_printError("m_lang =" +quote(m_lang) );
    if (!m_lang.IsEmpty())
    {
        oldtxt = "Language:"; newtxt = "Language: " + m_lang;
        //=================================================
        ok = Pre::changeContents  (_source, oldtxt, newtxt);
        //=================================================
	}
	else
	{
        _printWarn("*** m_lang =" + quote(m_lang));
        ok = false;
	}
    // all errors
	if (!ok)
	{
		_printError("not ok 7 => Pre::changeContents(...)" );

		return false;
	}

//8- charset : here $(ENCODING) -> "windows-1252"  !!!
	//oldtxt = "charset"; newtxt = "charset=$(ENCODING)" ;
	oldtxt = "charset"; newtxt = "charset=UTF-8";
	//=================================================
	ok = Pre::changeContents  (_source, oldtxt, newtxt);
	//=================================================
	if (!ok)
	{
		_printError("not ok 8 => Pre::changeContents(...)" );
		return false;
	}

    return ok;
}
// -----------------------------------------------------------------------------
//
// Called by :
//     1. Pre::integrity(bool _replacecar):1,
//
wxUint32 Pre::deleteCr(wxString & _source)
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
// an array from 'source'
	wxArrayString asource = ::GetArrayFromString(_source, Lf, TRIM_SPACE);
	wxUint32 nlmax = asource.GetCount();
	_source = wxEmptyString;
	bool ok;
	wxUint32 nr = 0;
	for (wxUint32 u = 0 ; u < nlmax; u++)
	{
	// control to pending messages in the event loop
	  //  m_pM->Yield();
		line = asource.Item(u);
		if (line.IsEmpty())		continue;
		ok = (line.GetChar(0) != di) && (line.Find(rc) !=  wxNOT_FOUND);
		if (ok)
		{
		// ATTENTION : "\\return" => "\eturn" !!!
			if (!line.Contains("\\return") )
			{
		_printError("Pre::deleteCr() => line = " + quoteNS(line) );
			// replace all 'rc' inside 'line'
				nr += line.Replace(rc, wxEmptyString, ALL_TXT);
			}
		}
	// build new 'source'
		_source += line + Eol;
	}

	return nr ;
}

// -----------------------------------------------------------------------------
//
// Called by :
//     1. Pre::integrity(bool _replacecar):5,
//
// Call to : none
// -----------------------------------------------------------------------------
bool Pre::changeContents(wxString& _source, const wxString& _old, wxString& _new)
{
_printD("=> Begin 'Pre::changeContents(...)'");

    if (_source.IsEmpty())
    {
        Mes = "Pre::changeContents : " + _("source is empty") + " !! ";
        _printError(Mes) ;
        return false;
    }
// just header
	wxString source = _source.Mid(0, m_sizeHeaderpo);
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
			Mes +=  _("inside") + quote(m_Shortnamedup) + " !!";
			_print(Mes);
		}
	}
	else
    {
        _printWarn(source);
    }

_printD("	<= End 'Pre::changeContents(...)' => " + strBool(ok) );

	return ok;
}
// -----------------------------------------------------------------------------
//
// Called by :
//     1. Pre::integrity(bool _replacecar):1,
// Call to :
//     1. Pre::executeAndGetOutputAndError(const wxString& _command, bool _prepend_error):1,
// -----------------------------------------------------------------------------
wxString Pre::mergeFiles(wxString _old, wxString _new, wxInt32 &  _nbdots)
{
_printWarnD("=> Begin 'Pre::mergeFiles(" +  _old + ", " + _new + ")");
	m_DirlocaleLeader = m_DirprojectLeader + m_Dirlocale;
// absolute paths
	_old.Prepend(m_DirlocaleLeader); _new.Prepend(m_DirlocaleLeader);
	wxUint32 nbdots = 0;
	// 'm_PathMexe' contains already double quote
	wxString command = m_PathMexe, verbose = " -v" , result;
	// verbose
	command += verbose;
	// directory
//	command += " -D" + dquote(m_DirprojectLeader) ;
	// enclose filenames in quotation marks
	command += dquote(_old) +  dquote(_new) ;
	// update _oldpo
	command += " -U";
	// make a backup with new suffix
	command += " --suffix=.dou";
	// delete progress dot =>  nbdots == 0
	//command += " -q";
	// no fuzzy match
	command += " -N";
	// preserve location #: xxxxx
	command += " --add-location"; // or " -n"
	//  keep the previous "msgid" strings of translated messages marked with ‘#|’
	command += " --previous";
	// with color if exists
	//command += " --color ";
	// large = 80 car
	command += " -w80";
	//===========================================================
	result = executeAndGetOutputAndError(command, PREPEND_ERROR);
	//===========================================================
//_printError("result not treated :" + Lf + markNS(result));
	if (!result.IsEmpty())
	{
	// delete all progress  indicator :  cDot == '.'
		nbdots = result.Replace(Dot, wxEmptyString, ALL_TXT);
		/// not verbose => nbdots > 0
		/// verbose if nbdots == 0 => error
	// ATTENTION 'msgmerge' messages uses the local language !!
	// no verbose : " -v" => find 'done'
    // not correct !!
	//	if (result.Find(_("done")) !=  wxNOT_FOUND )
		if (! command.Contains(verbose) )
		{
		// begin '.'
			if (result.StartsWith(cDot))
			{
				result.Replace(_("done"), wxEmptyString);
				result = result.AfterFirst(cSpace);
			}
		// begin : 'Win' "msgmerge.exe : xxxxxx", 'Lin' "msgmerge : xxxxxx"
			else
			{
				_printWarn(_("The command") + " = " + Lf + markNS(command));
				_printError(_("Generate error") + " : " + Lf + markNS(result));
				nbdots = 0;
			}
		}
	// verbose
		/// first line 	"........\n"
		/// second line " Read %ld old + %ld reference, merged %ld, fuzzied %ld
		///				  missing %ld, obosolete %ld\n"
		else
		{
			result.Trim(FROM_LEFT);
			result.Prepend(_("Processed chains") + " : " );
		}
	}
	_nbdots = nbdots

_printWarnD("	<= End 'Pre::mergeFiles(...)'");

	return result.Trim(FROM_RIGHT);
}

// ----------------------------------------------------------------------------
// Unifies duplicates in the strings to be translated :
//	=> a single string for the several same strings
////
// Called by :
//     1. Pre::integrity(const bool& _replacecar):1,
// Call to :
//		1. Pre::executeAndGetOutputAndError(const wxString& _command,
//											const bool& _prepend_error):1,
//
wxString Pre::noDuplicateMsgid(wxString& _inputdup, wxString& _outputpo)
{
_printErrorD("	===> Begin Pre::noDuplicateMsgid(...)" );
// absolute paths with Dquote
	wxString command = m_PathUexe + Space;
	// directory
	//command += " -D xxxxx "
	// input file
	command += dquoteNS(_inputdup);
	// output file
	command += " -o" + dquoteNS(_outputpo);
_printWarnD("Pre::noDuplicateMsgid => command = " + Lf + markNS(command) );

	// execute 'm_PathUexe'
	//====================================================================
	wxString result = executeAndGetOutputAndError(command, PREPEND_ERROR);
	//====================================================================
	if (!result.IsEmpty())
	{
    //_printWarn(_("ATTENTION 'msguniq' messages uses the local language") + " !!" );
		if (result.Contains("msguniq") )
		{
			Mes = _("Generate error") + " :" + Lf;
			result.Prepend(Mes);
			_printError(quote(result));
		}
	}
_printErrorD("		<== End Pre::noDuplicateMsgid(...)" );

	return result;
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
	if (! ::wxDirExists(_dir))
	{
	// Linux -> access = 0x0777
		ok = ::wxMkdir(_dir) ;
		if (!ok)
		{
			Mes = _("Can't create directory")  ;
			Mes += quote( _dir) + "!!";
			_printError(Mes);
			::cbMessageBox(Mes, "Pre::createDir()", wxICON_ERROR) ;
		}
	}

	Mes.Clear();

_printD(" <= End 'Pre::createDir(...) => " + strBool(ok) );

	return ok ;
}

///-----------------------------------------------------------------------------
// Recursively deletes all files and directories
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
    if(!::wxDir::Exists(_rmDir))
		return false;

// We make sure that the name of the folder to be deleted ends with a separator
	// if (_rmDir.Last() != wxFILE_SEP_PATH )
	// patch find in "https://github.com/vslavik/poedit"
	if (_rmDir.empty() || (_rmDir.Last() != wxFILE_SEP_PATH) )
		_rmDir += wxFILE_SEP_PATH;

// Creating the 'wxDir*' object
    wxDir* pDir = new ::wxDir(_rmDir);
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
            if (::wxDirExists(_rmDir + item))
            {
            /// recursive call ...
                //=========================
                recursRmDir(_rmDir + item);
                //=========================
            }
            else
        // Otherwise, we delete the file
			if(!::wxRemoveFile(_rmDir + item))
			{
            //Error during deletion (access rights ?)
				Mes =_( "Could not remove item") + quote(_rmDir + item);
				_printError(Mes);
			}
			else
			{
				m_nbFilesdeleted++;
				Mes = Tab + "DF" + strIntZ(m_nbFilesdeleted, 5) + " :";
				Mes += quote(item) ;
                _print(Mes);
                 m_pM->Yield();
            }
        }
        while (pDir->GetNext(&item));
    }
// From now on, the folder is empty, so we can delete it
    // But first you have to destroy the wxDir variable that always points to it
    delete pDir;
// You can now delete the folder passed as a parameter
    ok = ::wxFileName::Rmdir(_rmDir);
    if (!ok)
    {
	// Error during deletion (access rights ?)
		Mes = _("Could not remove directory") + quote(_rmDir);
		_printError(Mes);
    }
    else
    {
        Mes = Tab + "DD" + + " :" + quote(_rmDir) ;
        _print(Mes);
        m_pM->Yield();
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

	return ::cbReadFileContents(file);
}

///-----------------------------------------------------------------------------
// Save a 'wxArrayString' to disk and read the file in 'Editor'
//
// Called by  :
//		1. CreateForWx::creatingPot(bool _prjfirst):1,
//		2. Pre::endaction(wxChar _mode):1,
//
bool Pre::saveArrayToDisk (const wxArrayString& _strarray, const wxChar _mode)
{
_printErrorD("=> Begin saveArrayToDisk(..., " + wxString(_mode) +  +")'" );

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
		else
		if (_mode == 'P')       ext = "dup";
        else return false;
	// file to disk
		wxString namefile = m_Dirpot + m_NameprojectLeader;
		if (m_Workspace)	namefile += PROLONG_WS;
		namefile += Dot + ext;
		wxString shortname = namefile.AfterLast(cSlash).Prepend(m_Dirlocale);
    // exists already ?
		ok = wxFileName::FileExists(namefile);
		if (ok)		wxRemoveFile(namefile);
	// data to file
		wxString mes, out, line ;
		size_t nbmax = _strarray.GetCount() - 1;
        if (_mode == 'L' ||  _mode == 'E' )
        {
            out = ::GetStringFromArray(_strarray, Lf);
        }
        else
        if (_mode == 'P')
        {
/// debug
	//showArray(m_FilesCreatingPOT, "m_FilesCreatingPOT");
			wxString  sep = Space;
            for (wxUint32 i = 0; i <= nbmax; ++i)
            {
                line =  _strarray[i];
                if (line.StartsWith("msgstr"))
                {
      //  _printWarn(iToStr(i) + "- " + quoteNS(line));
                    line << (Lf + sep);
                }

                if (!line.Contains(Lf) )
                {
                    line << Lf;
        //_printError(iToStr(i) + "- " + quoteNS(line));
                }

                out << line ; //<< sep;
            }
          //  _printError(out);
        }
        // save file
		ok = ::cbSaveToFile(namefile, out);
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

_printErrorD("	<= End 'saveArrayToDisk(...)' => " + strBool(ok) )	;

	return ok;
}

///-----------------------------------------------------------------------------
// Add a '_array' to '_receive'
//
// Called by  :
//  1. CreateForWx::pathWx(ProjectBuildTarget * _pTarget):2,
//
wxUint32 Pre::addArrays(wxArrayString& _receive, wxArrayString& _array)
{
    for (wxUint32 cell = 0 ;cell < _array.GetCount() ; cell++)
    {
       _receive.Add(_array.Item(cell)) ;
    }

    return _receive.GetCount();
}
///-----------------------------------------------------------------------------
// Display a file in  'Editor'
//
// Called by  :
//		1. Pre::endextract():2,
//		2. Pre::saveArrayToDisk (const wxArrayString& _strarray,  wxChar _mode):1,
//
bool Pre::openedit(const wxString _file)
{
_printD("=> Begin 'openedit(" + _file + ")'" );

	bool ok = ::wxFileName::FileExists(_file);
	if (ok)
	{
		::cbEditor*  ed_open = m_pMed->IsBuiltinOpen(_file);
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
    ::cbEditor*  ed_open = m_pMed->IsBuiltinOpen(_file);
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
     if (m_Workspace)   fullpath += PROLONG_WS;

    //======================================
    bool oklist = closedit(fullpath + ".list") ;

    bool okextr = closedit(fullpath + ".extr");

 //   bool okpot  = closedit(fullpath + ".pot");
    bool okdup   = closedit(fullpath + ".dup");

    bool okuni   = closedit(fullpath + ".uni");

    bool okpo   = closedit(fullpath + ".po");
    //======================================

_printD("    <= End 'Pre::closeAllExtraFiles()'");

    return oklist || okextr || okdup || okpo || okuni;
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
    ::PlaceWindow(&saveFile);
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
            /// TODO ...
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
	::cbMessageBox(_mes, wxEmptyString, wxICON_INFORMATION);
}
// ----------------------------------------------------------------------------
//  Called by : many ...
//
void Pre::ShowError(const wxString& _mes)
{
	::cbMessageBox(_mes, wxEmptyString, wxICON_ERROR);
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
    return str ;
}

///-----------------------------------------------------------------------------
//  Begin duration for Debug
//
void Pre::beginDuration(const wxString & _namefunction)
{
// date
	Mes = Lf + "==> ";
	//===============================================================
	Mes += _("Start of") + quote(_namefunction) + ": " + Pre::date();
	//===============================================================
	_printWarn(Mes);
	m_tI = elapsed(0);
	Mes.Clear();
}
///-----------------------------------------------------------------------------
//  End duration for Debug
//
void Pre::endDuration(const wxString & _namefunction)
{
	Mes = "<== " ;
	//================================================================
	Mes += _("End of") + quote( _namefunction  ) + ": " + Pre::date();
	//================================================================
	Mes += " -> " + dtimeStr(elapsed(0) - m_tI);
	_printWarn(Mes);
	Mes.Clear();
}

///-----------------------------------------------------------------------------
//  Elapsed time in milliseconds
//
// Call by :
//      1.  wxString Pre::elapsedStr(const wxUint64 _begin):1,
//
wxUint64 Pre::elapsed(const wxUint64 _last)
{
// actual in 'mS'
    wxUint64 actual = clock(), ret = 0 ;
    if (m_Lin)    actual /= 1000;
//_printWarn("actual : " + strInt(actual) + ", _last : " + strInt(_last));
    if (_last <= actual)
        ret = actual - _last;

    return ret;
}

///-----------------------------------------------------------------------------
//  Elapsed time en mS to strings
//
// Call by :
//      1. Pre::listing(const wxUint32& _posWS):1,
//      2. Pre::endaction(const wxChar& _mode):4,
//      3. Pre::endDuration(const wxString & _namefunction):1,
//
// Call to : none
//
wxString Pre::dtimeStr(const wxUint64 _time)
{
// in 'mS'
    wxUint64 S = _time/1000, ms = _time%1000, minu = S/60, sec = S%60
            ;
	wxString str = strInt(minu) + Space + _("minute(s)") + ", " ;
	str += strInt(sec) + Space +_("second(s)") + ", ";
	str += strInt(ms) + Space +_("mS")  ;

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
	size_t nl = _strarray.GetCount();
	mes += " : " + strInt(nl) + Space + _("lines") ;
	for (size_t u = 0; u < nl; u++)
	{
	// read strarray line from  '1' ... 'nl'
		mes += Lf + strInt(u+1) + "- " + _strarray.Item(u) ;
	}
	mes +=  Lf + "--------- end  debug ------------" ;

_printD("	<= End 'Pre::allStrTable(...)");

	return mes;
}

///-----------------------------------------------------------------------------
// Show a 'wxArrayString' for debug to log
//
// Called by  :  for debug a table
//
bool Pre::showArray(const wxArrayString& _strarray, const wxString& _title)
{
_printD("=> Begin 'Pre::showArray(...)");
	wxString mes = "Array string : " + quote(_title) ;
	bool good = true;
	size_t nl = _strarray.GetCount();
    _printWarn(mes);
	for (size_t u = 0; u < nl; u++)
	{
	// control to pending messages in the event loop
		m_pM->Yield();
    // analysis stopped manually
		if (m_Stop)
		{
			good = false ;
			break;
		}
	// read strarray line from  '0' ... 'nl-1'
		mes = Tab + strInt(u) + "- " + quoteNS(_strarray.Item(u)) ;
		_printError(mes);
	}

_printD("	<= End 'Pre::showArray(...)");
    return good;
}

///-----------------------------------------------------------------------------
