/*************************************************************
 * Name:      collector.cpp
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2020-05-10
 * Modified:  2022-12-10
 * Copyright: LETARTARE
 * License:   GPL
 *************************************************************
 */

//------------------------------------------------------------------------------
#include <sdk.h> 	// Code::Blocks SDK
#ifdef __linux__
    #include <wx/filesys.h>
    #include <cbproject.h>
    #include <projectmanager.h>
    #include <macrosmanager.h>
    #include <configmanager.h>
    #include <cbeditor.h>
#endif
// for progress bar
#include "buildlogger.h"

#include "collector.h"
// two unidirectional aggregations with 'Collector'
// for Wx
#include "createforwx.h"
// for Qt
#include "createforqt.h"

// not place change !
#include "defines.h"
//------------------------------------------------------------------------------
/** \brief The static logger for 'Collector'
 */
BuildLogger* Collector::m_pCollectLog = nullptr ;

//------------------------------------------------------------------------------
// Register the plugin with 'Code::Blocks'.
// We are using an anonymous namespace so we don't litter the global one.
namespace
{
    /** \brief ATTENTION :
	 *   name to register this plugin into 'Code::Blocks'
	 *   THIS NAME MUST BE IDENTICAL TO
	 *	 $(name) IN PROJECT CUSTOM VARIABLE
	 *   AND IN 'manifest.xml' :: <Plugin name="Collector">
  	 */
  	wxString NamePlugin = "Collector";
	PluginRegistrant<Collector> reg(NamePlugin);
}

// ----------------------------------------------------------------------------
// menu entries constante identificator
// -----------------------------------------------------------------------------
	// menu bar
	const wxUint64 Collector::idMbarCollect 			= wxNewId();
	const wxUint64 Collector::idMbarCollectWx 			= wxNewId();
	const wxUint64 Collector::idMbarCollectQt 			= wxNewId();
	const wxUint64 Collector::idMbarListPrj 			= wxNewId();
	const wxUint64 Collector::idMbarListWS 				= wxNewId();
	const wxUint64 Collector::idMbarExtractPrj 			= wxNewId();
	const wxUint64 Collector::idMbarExtractWS 			= wxNewId();
	const wxUint64 Collector::idMbarListExtractPrj		= wxNewId();
	const wxUint64 Collector::idMbarListExtractWS	    = wxNewId();
	const wxUint64 Collector::idMbarWaitingForStart		= wxNewId();
	const wxUint64 Collector::idMbarStop	 			= wxNewId();
	const wxUint64 Collector::idMbarCleanTemp 			= wxNewId();
	const wxUint64 Collector::idMbarSetting	 			= wxNewId();
	// tool bar
	const wxUint64 Collector::idTbarCollectWx			= wxNewId();
	const wxUint64 Collector::idTbarCollectQt			= wxNewId();
	const wxUint64 Collector::idTbarListPrj				= wxNewId();
	const wxUint64 Collector::idTbarExtractPrj			= wxNewId();
	const wxUint64 Collector::idTbarListExtractPrj		= wxNewId();
	const wxUint64 Collector::idTbarListWS				= wxNewId();
	const wxUint64 Collector::idTbarExtractWS			= wxNewId();
	const wxUint64 Collector::idTbarListExtractWS	    = wxNewId();

	const wxUint64 Collector::idTbarWaitingForStart		= wxNewId();
	const wxUint64 Collector::idTbarStop	 			= wxNewId();
	const wxUint64 Collector::idTbarCleanTemp 			= wxNewId();

	const wxUint64 Collector::idTbarKey					= wxNewId();

// events handling
BEGIN_EVENT_TABLE(Collector, cbPlugin)
// add any events you want to handle here

    // project
	EVT_TOOL (idMbarListPrj, Collector::OnMenuToState)
	EVT_TOOL (idMbarExtractPrj, Collector::OnMenuToState)
	EVT_TOOL (idMbarListExtractPrj, Collector::OnMenuToState)
	// workspace
	EVT_TOOL (idMbarListWS, Collector::OnMenuToState)
	EVT_TOOL (idMbarExtractWS, Collector::OnMenuToState)
	EVT_TOOL (idMbarListExtractWS, Collector::OnMenuToState)
	// init
	EVT_TOOL (idMbarWaitingForStart, Collector::OnMenuWaitingForStart)
	EVT_TOOL (idMbarCleanTemp, Collector::OnMenuToState)
    // stopping others
	EVT_TOOL (idMbarStop, Collector::OnMenuStop)

// tool bar
	EVT_UPDATE_UI(idTbarListPrj,  Collector::OnUpdateUI)
	EVT_UPDATE_UI(idTbarExtractPrj,  Collector::OnUpdateUI)
	EVT_UPDATE_UI(idTbarListExtractPrj,  Collector::OnUpdateUI)
	EVT_UPDATE_UI(idTbarListWS,  Collector::OnUpdateUI)
	EVT_UPDATE_UI(idTbarExtractWS,  Collector::OnUpdateUI)
	EVT_UPDATE_UI(idTbarListExtractWS,  Collector::OnUpdateUI)
    EVT_UPDATE_UI(idTbarStop,  Collector::OnUpdateUI)

    EVT_TOOL (idTbarCollectWx, Collector::OnMenuChoiceLib)
	EVT_TOOL (idTbarCollectQt, Collector::OnMenuChoiceLib)

    // project
	EVT_TOOL (idTbarListPrj, Collector::OnMenuToState)
	EVT_TOOL (idTbarExtractPrj, Collector::OnMenuToState)
	EVT_TOOL (idTbarListExtractPrj, Collector::OnMenuToState)
	// workspace
	EVT_TOOL (idTbarListWS, Collector::OnMenuToState)
	EVT_TOOL (idTbarExtractWS, Collector::OnMenuToState)
	EVT_TOOL (idTbarListExtractWS, Collector::OnMenuToState)
	// init
	EVT_TOOL (idTbarWaitingForStart, Collector::OnMenuWaitingForStart)
	EVT_TOOL (idTbarCleanTemp, Collector::OnMenuToState)
    // stopping others
	EVT_TOOL (idTbarStop, Collector::OnMenuStop)

END_EVENT_TABLE()

// -----------------------------------------------------------------------------
//	Constructor load ressource 'Collector.zip'
//
Collector::Collector(): cbPlugin(), ColState()
{
	wxString zip = NamePlugin + ".zip";
	if(!Manager::LoadResource(zip))		NotifyMissingFile(zip);
}

// -----------------------------------------------------------------------------
// Create handlers event and creating the pre-builders
//
void Collector::OnAttach()
{
//1- manager
    m_pM = Manager::Get();
	if (!m_pM)  return ;
//2- projects manager
	m_pMprj = m_pM->GetProjectManager();
	if (!m_pMprj)  return ;

	m_pMplug = m_pM->GetPluginManager();
	if (!m_pMplug)  return ;

//3- plugin manually loaded
	cbEventFunctor<Collector, CodeBlocksEvent>* functorPluginLoaded =
		new cbEventFunctor<Collector, CodeBlocksEvent>(this, &Collector::OnPluginLoaded);
	m_pM->RegisterEventSink(cbEVT_PLUGIN_INSTALLED, functorPluginLoaded);
//4- handle project activate
	cbEventFunctor<Collector, CodeBlocksEvent>* functorActivateProject =
		new cbEventFunctor<Collector, CodeBlocksEvent>(this, &Collector::OnActivateProject);
	m_pM->RegisterEventSink(cbEVT_PROJECT_ACTIVATE, functorActivateProject);
//5- handle target selected
	cbEventFunctor<Collector, CodeBlocksEvent>* functorTargetSelected =
		new cbEventFunctor<Collector, CodeBlocksEvent>(this, &Collector::OnActivateTarget);
	m_pM->RegisterEventSink(cbEVT_BUILDTARGET_SELECTED, functorTargetSelected);
//6- handle modified edited file
    // delete see v-1.7.0
//7- bitmaps from *.zip
	LoadAllPNG();

//8- construct a new log
	m_pLogMan = m_pM->GetLogManager();
	if(m_pLogMan)
    {
    //8-1 add 'Collector' log
		m_pCollectLog = new BuildLogger(FIX_LOG_FONT);
        m_LogPageIndex = m_pLogMan->SetLog(m_pCollectLog);
		m_pLogMan->Slot(m_LogPageIndex).title = _(NamePlugin);
		// for 'Wx' => 'm_bmLogoWx', for 'Qt' => 'm_bmLogoQt' ...
        CodeBlocksLogEvent evtAdd1(cbEVT_ADD_LOG_WINDOW, m_pCollectLog,
									m_pLogMan->Slot(m_LogPageIndex).title, &m_bmLogoWx);

        m_pM->ProcessEvent(evtAdd1);

	//8-2 add bar progress to 'Collector' log
		m_pCollectLog->AddBuildProgressBar();
		if (m_pCollectLog->m_pProgress)
		{
		// initialisation progress bar
			BuildLogger::g_MaxProgress = 10000;
			BuildLogger::g_CurrentProgress = 0;
			m_pCollectLog->m_pProgress->SetRange(BuildLogger::g_MaxProgress);
			m_pCollectLog->m_pProgress->SetValue(BuildLogger::g_CurrentProgress);
		}

    //8-3 memorize last log
        CodeBlocksLogEvent evtGetActive(cbEVT_GET_ACTIVE_LOG_WINDOW);
        m_pM->ProcessEvent(evtGetActive);
        m_Lastlog   = evtGetActive.logger;
        m_LastIndex = evtGetActive.logIndex;
    // display 'm_pCollectLog'
        SwitchToLog();
    }

//9- construct the builder 'Wx'
	// construct a new 'm_pCreateWx' m_pProject == nullptr
	m_pCreateWx = new CreateForWx(NamePlugin, m_LogPageIndex);
	if (!m_pCreateWx)
	{
		Mes = _("Error to create") + Space + "m_pCreateWx";
		Mes += Lf + _("The plugin part 'Wx' is not operational") + " !!"; _printError(Mes);
		_PrintError(Mes);
	//  release plugin
	    OnRelease(false);
	}
	else
	{
        Mes = _("Create") + quote("m_pCreateWx");
		Mes += Lf + _("The plugin part 'Wx' is operational") ;
		//_print(Mes);
    }

//10- builder 'Qt'
	// construct a new 'm_pCreateQt' m_pProject == nullptr
	m_pCreateQt = new CreateForQt(NamePlugin, m_LogPageIndex);
	if (!m_pCreateQt)
		{
		Mes = _("Error to create") + quote("m_pCreateQt") ;
		Mes += Lf + _("The plugin part 'Qt' is not operational") + " !!"; _printError(Mes);
		_PrintError(Mes);
	//  release plugin
	    OnRelease(false);
	}
	else
	{
        Mes = _("Create") + quote("m_pCreateQt");
		Mes += Lf + _("The plugin part 'Qt' is operational") ;
		//_print(Mes);
    }


//11- user welcome message
	if (m_pCreateWx && m_pCreateQt)
	{
		Mes = m_pCreateWx->platForm();
		Mes += ", " + quoteNS("sdk-" + m_pCreateWx->GetVersionSDK());
		Mes += ", " + quoteNS(NamePlugin +  "-" + VERSION_COLLECT)  ;
        Mes += ", " + _("built the") + quote(m_pCreateWx->GetDateBuildPlugin()) + Lf;
		_printWarn(Mes);
	}
	else
	{
	//  release plugin
	    OnRelease(false);
	}

	Mes.Clear();
}

//-----------------------------------------------------------------------------
// Construct menu on 'menuBar'
//
//	Called by :
//	1. 'ProjectsManager'
// Call to : none
//
void Collector::BuildMenu(wxMenuBar* _pMenuBar)
{
_printD("=> Begin 'Collector::BuildMenu(...)");

	if (!IsAttached() || ! _pMenuBar)     	return;

// saved '_menuBar'
	m_pMenuBar =  _pMenuBar;
// locate "Settings" menu and insert before it
	wxString label = _("&Settings");
	wxInt32 menuposX = m_pMenuBar->FindMenu(label);
//_print("menuposX = " + strInt(menuposX));
	if (menuposX == wxNOT_FOUND )
	{
		_printError( quote(label) + _("cannot be found") + " !!!");
		return ;
	}
	// good place
	m_placeCollectX  = size_t(menuposX) ;
// get the  'Settings' menu
	wxMenu  * menu = m_pMenuBar->GetMenu(menuposX);
	if (menu)
	{
	// number menus
		size_t posi = m_pMenuBar->GetMenuCount();
	// whereis "&Collect"
		bool good = m_placeCollectX <= posi;
		if (!good)  return ;

	// new menu in menuBar before 'Settings'
		wxMenu * mCollect = new wxMenu();
		if (mCollect)
		{
		// a menu pointer
			m_pmCollect = mCollect;
		// insert before 'Settings'
			label = _("&Collect") ;
			 m_pMenuBar->Insert(m_placeCollectX, mCollect, label);
		// menu item
			wxMenuItem  * pItem ;
			// for wxWidgets
			label 	= _("Collect") + "-'Wx'";
			Mes 	= _("Collect for 'wxWidgets' project or workspace") ;
			pItem  	= new wxMenuItem(mCollect,  idMbarCollectWx , label, Mes);
			if (pItem)
			{
//_print("pItem: isListProject  :" + strInt(idMbarListPrj));
                pItem->SetBitmap(m_bmLogoWx);
				mCollect->Append(pItem);
			}
        /*
			// for Qt future
			label 	= _("Collect") + "-'Qt'";
			Mes 	= _("Collect for 'Qt' project or workspace") ;
			pItem  	= new wxMenuItem(mCollect,  idMbarCollectQt , label, Mes);
			if (pItem)
			{
//_print("pItem: isListProject  :" + strInt(idMbarListPrj));
                pItem->SetBitmap(m_bmLogoQt);
				mCollect->Append(pItem);
			}
			*/
			//  Separator
			mCollect->AppendSeparator() ;
		// 'Setting'
			//label 	= _("Setting") ;
			Mes 	= _("Setting 'key' for translate") + Space + "= '_'";
			pItem 	= new wxMenuItem(mCollect, idMbarSetting, Mes);
			if (pItem)
			{
				mCollect->Append(pItem);
			}
		//  Separator
			mCollect->AppendSeparator() ;
		// 'List'
			label 	= _("List from project");
			Mes 	= _("List all translatable strings found in the project") ;
			pItem  	= new wxMenuItem(mCollect,  idMbarListPrj , label, Mes);
			if (pItem)
			{
//_print("pItem: isListProject  :" + strInt(idMbarListPrj));
                pItem->SetBitmap(m_bmList);
				mCollect->Append(pItem);
			}
		// 'Extract'
			label 	= _("Extract from project") ;
			Mes 	= _("Extract all translatable strings found in the project") ;
			pItem 	= new wxMenuItem(mCollect,  idMbarExtractPrj , label, Mes);
			if (pItem)
			{
                pItem->SetBitmap(m_bmExtract);
				mCollect->Append(pItem);
			}
		// 'ListAndExtract'
			label 	= _("List and Extract from project") ;
			Mes 	= _("List and Extract all translatable strings found in the project") ;
			pItem  	= new wxMenuItem(mCollect,  idMbarListExtractPrj, label, Mes);
			if (pItem)
			{
//_print("pItem: isListProject  :" + strInt(idMbarListPrj));
				pItem->SetBitmap(m_bmExtractPlus);
				mCollect->Append(pItem);
			}
		//  Separator
			mCollect->AppendSeparator() ;
		// 'List worspace'
			label 	= _("List from workspace") ;
			Mes 	= _("List all translatable strings found in the workspace") ;
			pItem 	= new wxMenuItem(mCollect,  idMbarListWS , label, Mes);
			if (pItem)
			{
				pItem->SetBitmap(m_bmListPlus);
				mCollect->Append(pItem);
			}
		// 'Extract worspace'
			label 	= _("Extract from workspace") ;
			Mes 	= _("Extract all translatable strings found in the workspace") ;
			pItem 	= new wxMenuItem(mCollect,  idMbarExtractWS, label, Mes);
			if (pItem)
			{
//_print("pItem: idMbarExtractWS  :" + strInt(idMbarExtractWS));
				pItem->SetBitmap(m_bmExtractPlus);
				mCollect->Append(pItem);
			}
		// 'ListAndExtract workspace'
			label 	= _("List and Extract from workspace") ;
			Mes 	= _("List and Extract all translatable strings found in the workspace") ;
			pItem  	= new wxMenuItem(mCollect,  idMbarListExtractWS , label, Mes);
			if (pItem)
			{
//_print("pItem: isListProject  :" + strInt(idMbarListPrj));
				pItem->SetBitmap(m_bmExtractPlus);
				mCollect->Append(pItem);
			}
		//  Separator
			mCollect->AppendSeparator() ;
		// 'FirstState' state graph
			label 	= _("Init to first state") ;
			Mes 	= _("Resets the state of the state graph to the beginning") ;
			pItem  	= new wxMenuItem(mCollect,  idMbarWaitingForStart, label, Mes);
			if (pItem)
			{
//_print("pItem: isListProject  :" + strInt(idMbarListPrj));
				pItem->SetBitmap(m_bmInit);
				mCollect->Append(pItem);
			}
		// 'Clean temp'
			label  	= _("Clean temporary") ;
			Mes 	= _("Removes temporary files");
			pItem  	= new wxMenuItem(mCollect,  idMbarCleanTemp , label, Mes);
			if (pItem)
			{
				pItem->SetBitmap(m_bmTemp);
				mCollect->Append(pItem);
			}
		// 'Stop'
			label 	= _("Stop current action") ;
			Mes 	= _("Stops the action in progress");
			pItem 	= new wxMenuItem(mCollect, idMbarStop , label, Mes);
			if (pItem)
			{
				pItem->SetBitmap(m_bmStop);
				mCollect->Append(pItem);
			}
		}
	// first wait state
		actualizeGraphState(fbWaitingForStart);
	}

_printD("	<= End 'Collector::BuildMenu(...)");
}

//-----------------------------------------------------------------------------
// Receive an event menu item or bouton tool for actualize state machine
// then execute 'List' or 'Extract' or ...
//
//	Called by :
// 1. Collector::OnMenuListPrj(wxCommandEvent& _pEvent):1,
// 2. Collector::OnMenuExtractPrj(wxCommandEvent& _pEvent):1,
// 3. Collector::OnMenuListExtractPrj(wxCommandEvent& _pEvent):1,
//
// 4. Collector::OnMenuListWS(wxCommandEvent& _pEvent):1,
// 5. Collector::OnMenuExtractWS(wxCommandEvent& _pEvent):1,
// 6. Collector::OnMenuListExtractWS(wxCommandEvent& _pEvent):1,
//
// Call to :
// 1. Collector::callActions(wxCommandEvent& _pEvent):1,
//
void Collector::OnMenuToState(wxCommandEvent& _pEvent)
{
_printD("=> Begin 'Collector::OnMenuToState(...)'");

    colState State = fbNone;
// origin
	wxUint64 id = _pEvent.GetId();
// only good id's
	bool goodId = true;

	if (id == idMbarStop || id == idTbarStop)
        State = fbStop;
    else
    if (id == idMbarCleanTemp  || id == idTbarCleanTemp)
        State = fbCleanTemp;
    else
    if (id == idMbarListPrj  || id == idTbarListPrj)
        State = fbListPrj;
    else
    // is a macro state : 'fbListPrj' followed by 'fbExtractPrj'
    if (id == idMbarListExtractPrj  || id == idTbarListExtractPrj)
        State = fbListExtractPrj;
    else
    if (id == idMbarExtractPrj  || id == idTbarExtractPrj)
        State = fbExtractPrj;
    else
    if (id == idMbarListWS  || id == idTbarListWS)
        State = fbListWS;
    else
    if (id == idMbarExtractWS  || id == idTbarExtractWS)
        State = fbExtractWS;
    else
    if (id == idMbarListExtractWS || id == idTbarListExtractWS)
    // is a macro state : 'fbListWS' followed by 'fbExtractWS'
        State = fbListExtractWS;
    else
        goodId = false;

    if (goodId)
    {
    // update state graph
        actualizeGraphState(State);
    // actions according state machine
        callActions(_pEvent);
    }

_printD("    <= End 'Collector::OnMenuToState(...)'");

}
//------------------------------------------------------------------------------
// Define the grap state and Enable the menu and tools  items of 'Collect'
//
//	Called by :
// 	1. Collector::OnActivateProject(CodeBlocksEvent& _pEvent):2
// 	2. Collector::OnMenuExtractWS(wxCommandEvent& _pEvent):2,
//	3. Collector::OnMenuExtractWS(wxCommandEvent& _pEvent):1,
//	4. Collector::OnMenuListWS(wxCommandEvent& _pEvent):2,
//	5. Collector::OnMenuCleanTemp(wxCommandEvent& _pEvent):1,
//	6. Collector::OnMenuListPrj(wxCommandEvent& _pEvent):2,
//	7. Collector::OnMenuWaitingForStart(wxCommandEvent& _pEvent):1,
//	8. Collector::OnMenuStop(wxCommandEvent& _pEvent):2,

// Call to  :
//	1. ColState::strState(const wxString & _state):1,
//	2. Pre::setStop(const bool& _abort):1,
//
ColState::colState Collector::actualizeGraphState(const colState& _state)
{
_printD("=> Begin 'Collector::actualizeGraphState(" + strState(_state) + ")'" );

// 'Wx' or 'Qt' only
	bool valid =  m_isWxProject ||  m_isQtProject;
// menus and tools
	if ( m_pmCollect && m_pToolBar)
	{
// menus bar "&Collect"
		// TO REVIEW ...
		m_pmCollect->Enable(idMbarCollectWx, m_isWxProject);
	//	m_pmCollect->Enable(idMbarCollectQt, m_isQtProject);

	// all disable
		m_pmCollect->Enable(idMbarListPrj,  false);
		m_pmCollect->Enable(idMbarExtractPrj,  false);
		m_pmCollect->Enable(idMbarListExtractPrj, false);

		m_pmCollect->Enable(idMbarListWS,  false);
		m_pmCollect->Enable(idMbarExtractWS,  false);
		m_pmCollect->Enable(idMbarListExtractWS, false);

		m_pmCollect->Enable(idMbarSetting, false);
		m_pmCollect->Enable(idMbarStop, false);
		m_pmCollect->Enable(idMbarCleanTemp, false);
		m_pmCollect->Enable(idMbarWaitingForStart, false);

// tool bar	"Collect"
		m_pToolBar->EnableTool(idTbarCollectWx, m_isWxProject);
	//	m_pToolBar->EnableTool(idTbarCollectQt, m_isQtProject);
	// all disable
		m_pToolBar->EnableTool(idTbarListPrj, false);
		m_pToolBar->EnableTool(idTbarExtractPrj, false);
		m_pToolBar->EnableTool(idTbarListExtractPrj, false);

		m_pToolBar->EnableTool(idTbarListWS, false);
		m_pToolBar->EnableTool(idTbarExtractWS, false);
		m_pToolBar->EnableTool(idTbarListExtractWS, false);

		m_pToolBar->EnableTool(idTbarCleanTemp, false);
		m_pToolBar->EnableTool(idTbarStop, false);
		m_pToolBar->EnableTool(idTbarWaitingForStart, false);

		m_pTbarCboKeys->Clear();
		if(m_isWxProject) m_pTbarCboKeys->Insert("_", 0);
		//if(m_isQtProject) m_pTbarCboKeys->Insert("tr", 0);
		m_pTbarCboKeys->SetSelection(0);

		m_pToolBar->Enable(valid);

// state
    // variable of mother class
        m_State = _state;
		switch (m_State)
		{
			case fbWaitingForStart:
			// menu bar
				m_pmCollect->Enable(idMbarListPrj,  true);
				m_pmCollect->Enable(idMbarListExtractPrj, true);

				m_pmCollect->Enable(idMbarListWS, true);
				m_pmCollect->Enable(idMbarListExtractWS, true);

				m_pmCollect->Enable(idMbarCleanTemp,  true);
				m_pmCollect->Enable(idMbarWaitingForStart, true);
			// tool bar
				m_pToolBar->EnableTool(idTbarListPrj, true);
				m_pToolBar->EnableTool(idTbarListExtractPrj, true);

				m_pToolBar->EnableTool(idTbarListWS, true);
				m_pToolBar->EnableTool(idTbarListExtractWS, true);

				m_pToolBar->EnableTool(idTbarCleanTemp, true);
				m_pToolBar->EnableTool(idTbarWaitingForStart, true);
				break;
            case fbStop:
				m_pmCollect->Enable(idMbarStop,  true);
				m_pToolBar->EnableTool(idTbarStop, true);
            // for agregates
				m_pCreateWx->setStop(true);
				m_pCreateQt->setStop(true);
				break;
            case fbCleanTemp:
				m_pmCollect->Enable(idMbarStop,  true);
				m_pToolBar->EnableTool(idTbarStop, true);
				break;
        // list and extract project
			case fbListPrj:
				m_pmCollect->Enable(idMbarStop,  true);
				m_pToolBar->EnableTool(idTbarStop, true);
				break;
            case fbWaitForExtractPrj:
				m_pmCollect->Enable(idMbarExtractPrj,  true);
				m_pToolBar->EnableTool(idTbarExtractPrj, true);
				m_pmCollect->Enable(idMbarWaitingForStart, true);
				m_pToolBar->EnableTool(idTbarWaitingForStart, true);
				break;
			case fbExtractPrj:
				m_pmCollect->Enable(idMbarStop,  true);
				m_pToolBar->EnableTool(idTbarStop, true);
				break;
			case fbListExtractPrj:
                m_pmCollect->Enable(idMbarStop,  true);
				m_pToolBar->EnableTool(idTbarStop, true);
				break;
        // list and extract workspace
			case fbListWS:
				m_pmCollect->Enable(idMbarStop,  true);
				m_pToolBar->EnableTool(idTbarStop, true);
				break;
            case fbWaitForExtractWS:
				m_pmCollect->Enable(idMbarExtractWS,  true);
				m_pToolBar->EnableTool(idTbarExtractWS, true);
				m_pmCollect->Enable(idMbarWaitingForStart, true);
				m_pToolBar->EnableTool(idTbarWaitingForStart, true);
				break;
            case fbExtractWS:
				m_pmCollect->Enable(idMbarStop,  true);
				m_pToolBar->EnableTool(idTbarStop, true);
				break;
            case fbListExtractWS:
                m_pmCollect->Enable(idMbarStop,  true);
                m_pToolBar->EnableTool(idTbarStop, true);
				break;

			case fbNone:
				break;
		}
	}

_printD("	<= End 'Collector::actualizeGraphState(...) => " + strState(m_State) );

	return m_State;
}

///-----------------------------------------------------------------------------
// Execute an action acccording state machine
//
//	Called by :
// 1. Collector::OnMenuToState(wxCommandEvent& _pEvent):1,
//
// Call to :
// 1. Collector::OnMenuStop(wxCommandEvent& _pEvent):1,
// 2. Collector::OnMenuCleanTemp(wxCommandEvent& _pEvent):1,
// 3. Collector::OnMenuListPrj(wxCommandEvent& _pEvent):1,
//
// 4. Collector::OnMenuExtractProject(wxCommandEvent& _pEvent):1,
// 5. Collector::OnMenuListWS(wxCommandEvent& _pEvent):1,
// 6. Collector::OnMenuExtractWS(wxCommandEvent& _pEvent):1,
//
bool  Collector::callActions(wxCommandEvent& _pEvent)
{
_printD("=> Begin 'Collector::CallActions(...)'");

    bool ok = false;

    switch (m_State)
    {
    	case fbNone  			: break;
        case fbWaitingForStart  : break;
        // OnMenuStop(...) is called directly
        case fbStop             : /*OnMenuStop(_pEvent);*/ break;
        case fbCleanTemp        : OnMenuCleanTemp(_pEvent) ; break;
        // projects
        case fbListPrj          : OnMenuListPrj(_pEvent); break;
        case fbWaitForExtractPrj: break;
        case fbExtractPrj       : OnMenuExtractPrj(_pEvent); break;
        case fbListExtractPrj   : OnMenuListExtractPrj(_pEvent); break;
        // workspace
        case fbListWS           : OnMenuListWS(_pEvent); break;
        case fbWaitForExtractWS : break;
        case fbExtractWS        : OnMenuExtractWS(_pEvent); break;
        case fbListExtractWS    : OnMenuListExtractWS(_pEvent); break;
    }

_printD("    <= End 'Collector::CallActions(...)'");

    return ok;
}

//-----------------------------------------------------------------------------
// Execute an menu item or bouton tool for library choice 'Wx' or ''Qt
//
//	Called by :
// 	1. Collector::idMbarCollectWx
//	2. Collector::idTbarCollectWx
//	3. Collector::idMbarCollectQt
//	4. Collector::idTbarCollectQt
//
// Call to : none
//
void Collector::OnMenuChoiceLib(wxCommandEvent& _pEvent)
{
_printD("=> Begin 'Collector::OnMenuChoiceLib(...)'");
// origin
	wxUint64 id = _pEvent.GetId();
//	Mes = "Id : " + strInt(id); _printWarn(Mes);
// no menu '&Collect' nor tool bar
	if (!m_pmCollect || !m_pToolBar) return;

// 'Wx'
	if (m_isWxProject && id == idMbarCollectWx)
	{
	// menus bar
		m_pmCollect->Enable(idMbarCollectQt, false);
		m_pmCollect->Enable(idMbarCollectWx, true);
	// tools bar
		m_pToolBar->EnableTool(idTbarCollectQt, false);
		m_pToolBar->EnableTool(idTbarCollectWx, true);
	}
	else
// 'Qt'
	if (m_isQtProject && id == idMbarCollectQt)
	{
	// menus bar
		m_pmCollect->Enable(idMbarCollectWx, false);
		m_pmCollect->Enable(idMbarCollectQt, true);
	// tools bar
		m_pToolBar->EnableTool(idTbarCollectWx, false);
		m_pToolBar->EnableTool(idTbarCollectQt, true);
	}

_printD("    <= End 'Collector::OnMenuChoiceLib(...)'");
}

//-----------------------------------------------------------------------------
// Execute 'Stop' item menu bar or tool bar
//
// Called by :
// 	1. Collector::idMbarStop:1,
// 	2. Collector::idTbarStop:1,

// Call to
//	1. Collector::SwitchToLog():1,
//  2. CreateForWx::SetStop(bool _stop):1,
//  3. CreateForQt::SetStop(bool _stop):1,
//  4. Collector::actualizeGraphState(const colState& _state):2,

void Collector::OnMenuStop(wxCommandEvent& _pEvent)
{
_printD("=> Begin 'Collector::OnMenuStop(...)' with 'checked':" + strBool(_pEvent.IsChecked()));

// origin
	wxUint64 id = _pEvent.GetId();
	if (id == idMbarStop || id == idTbarStop)
	{
	// display 'Collector' log
		SwitchToLog();
		if (m_State != fbStop)
		{
		// abort files creating
			if (m_pCreateWx && m_isWxProject) 		m_pCreateWx->setStop(true);
			if (m_pCreateQt && m_isQtProject)		m_pCreateQt->setStop(true);
		// enable menus 'Stop'
			actualizeGraphState(fbStop);
		}
		else
		{
		// enable menus wait
			actualizeGraphState(fbWaitingForStart);
		}
	}
_printD("	=> End 'Collector::OnMenuStop(...)'");
}

//-----------------------------------------------------------------------------
// Execute an item
//
//	Called by :
//		1. Collector::idMbarWaitingForStart:1,
//		2. Collector::idTbarWaitingForStart:1,

// Call to :
//		1. Collector::actualizeGraphState(const colState& _state):1,
//		2. ColState::strState(const colState& _state):1,
//
void Collector::OnMenuWaitingForStart(wxCommandEvent& _pEvent)
{
_printD("=> Begin 'Collector::OnMenuWaitingForStart(...)'");

    Mes = _("The current state machine is") + " :" + strState(m_State);
    wxUint8 nc = Mes.Len();
    _print(Lf+ Line(nc));
	_printWarn(Mes);

	wxUint64 id = _pEvent.GetId();
	// only good id's
    if (id == idMbarWaitingForStart || id == idTbarWaitingForStart )
	{
	// modify state machine
		actualizeGraphState(fbWaitingForStart);
    // progress bar to 0
        BuildLogger::g_CurrentProgress = 0;
        if (m_pCollectLog->m_pProgress)
            m_pCollectLog->m_pProgress->SetValue(BuildLogger::g_CurrentProgress);
    // close extra files    '*.list', '*.extr', '*.po'
        bool ok = m_pCreateWx->closeAllExtraFiles();
        if (ok)
        {
            Mes = "** " + _("Extra file(s) are closed") + ".";
            _print( Mes);
        }
    }

	Mes = _("The new state machine is") + " : " + strState(m_State) ;
	_printWarn(Mes);
    _print(Line(nc));

_printD("    <= End 'Collector::OnMenuWaitingForStart(...)'");
}

///-----------------------------------------------------------------------------
// List all translatable strings of the project
//
//	Called by :
//	1. Collector::callActions(wxCommandEvent& _pEvent):1,
//	2. Collector::OnMenuListExtractPrj(wxCommandEvent& _pEvent):1,

// Call to
//	1. Collector::SwitchToLog():1,
//	2. Collector::actualizeGraphState(const colState& _state):3,
//	3. CreateForWx::ListProject():1,
//	4. Pre::isCancelled():1,
//	5. Pre::isStopped():1,
//	6. CreateForQt::ListProject(const wxString& _key):1,
//
void Collector::OnMenuListPrj(wxCommandEvent& _pEvent)
{
_printD("=> Begin 'Collector::OnMenuListPrj(...)' with 'checked':" + strBool(_pEvent.IsChecked()));

	bool ok = false; wxInt32 nbstr = 0;
// identifiers
	wxUint64 id = _pEvent.GetId();
	// only good id's
    if (id == idMbarListPrj || id == idTbarListPrj )
	{
	// display 'Collector' log
		SwitchToLog();
	// Wx
		if (m_isWxProject)
		{
		// List chains
            wxString key = "_";
            if (m_pTbarCboKeys)
				key = m_pTbarCboKeys->GetStringSelection();
		// initialisation progress bar
			BuildLogger::g_CurrentProgress = 0;
        // init po header => line number header
            //==================================
			nbstr =  m_pCreateWx->iniHeaderPo();
            //==================================
			//========================================
			ok = m_pCreateWx->ListProject(key, nbstr);
			//========================================
			if (!ok)
			{
				Mes = "---> ";
				if (!m_pCreateWx->isInitialized())
				{
					Mes += _("Could not initialize the project.");
					Mes += Lf + _("Cannot continue in") + Space;
				}
				else
				if (m_pCreateWx->isCancelled())
				{
					Mes +=  _("You have") + quote(_("CANCELLED")) ;
				}
				else
				if (m_pCreateWx->isStopped())
				{
					Mes +=  _("You have") + quote(_("STOPPED")) ;
				}
				else
                if (m_pCreateWx->hasNoproject())
				{
                    Mes += _("No project loaded") + " !!" + quote(_("STOPPED"));
				}
				else
				{
					Mes += "Collector::OnMenuListPrj(...) => " ;
					Mes +=  _("An error unknown has") + quote(_("ABORTED")) ;
				}

				Mes += _("the current operation in") + Space ;
				Mes += _("'List from project'") + " ! <---" ;
				m_pCreateWx->ShowInfo(Mes);
				_printWarn(Mes);
			}
			else
			{
                if (nbstr)
                {
                // 'LisPrj'
                    if (m_State == fbListPrj)
                    {
                        Mes = ">>> " ;
                        Mes += _("YOU CAN NOW USE the menu");
                        Mes +=  quote(_("Extract from project")) ;
                        Mes +=  " <<<";
                        _printWarn(Mes);
                    }
                // 'LisPrj' followed by 'ExtractPrj'
                    else
                    {
                        // ...
                    }
                }
                else
                {
                    // 'LisPrj'
                    if (m_State == fbListPrj)
                    {
                        Mes = ">>> !! " ;
                        Mes += _("Your project has no strings to translate") ;
                        Mes +=  " !! <<<";
                        _printWarn(Mes);
                    }
                // 'LisPrj' followed by 'ExtractPrj'
                    else
                    {
                        // ...
                    }
                }
            }
		}

///-----------------------------------------------------------------------------
	// Qt
		if (m_isQtProject)
		{
			Mes = _("The listing of the 'Qt' part of")  + quote(m_pProject->GetTitle());
			Mes += _("is not yet operational") + " ...";
			_printError(Line(Mes.Len()) );
			_printWarn(Mes);
			_printError(Line(Mes.Len()) );
		/*
		// List chains
            wxString key = "tr";
            if (m_pTbarCboKeys)
				key = m_pTbarCboKeys->GetStringSelection();
//_printError("key : " + quote(key));
			ok = m_pCreateQt->ListProject();
			if (!ok)
			{
				Mes = _("Could not list the 'Qt' project.");
				Mes += Lf + _("Cannot continue");
				m_pCreateQt->ShowError(Mes);
				_printError(Mes);
			}
			*/
		}
///-----------------------------------------------------------------------------
	// Extract project
		if (ok && nbstr)    actualizeGraphState(fbWaitForExtractPrj);
	// first state
		else 		        actualizeGraphState(fbWaitingForStart);
	}

_printD("	<= End 'Collector::OnMenuListPrj(...)'" );
}

///-----------------------------------------------------------------------------
// Extract all translatable strings of the project
//
//	Called by :
// 1. Collector::callActions(wxCommandEvent& _pEvent):1,
//	2. Collector::OnMenuListExtractPrj(wxCommandEvent& _pEvent):1,

// Call to :
//	    1. Collector::actualizeGraphState(const colState& _state):3,
//	2. CreateForWx::ExtractProject():1,
//	3. Pre::isCancelled():1,
//	4. Pre::isStopped():1,
//	5. CreateForQt::ExtractProject():1,
//
void Collector::OnMenuExtractPrj(wxCommandEvent& _pEvent)
{
_printD("=> Begin 'Collector::OnMenuExractPrj(...)' with 'checked':" + strBool(_pEvent.IsChecked()));

	bool ok = false;
	wxUint64 id = _pEvent.GetId();
// only good id's
	if(id == idMbarExtractPrj || id == idTbarExtractPrj)
	{
	// display 'Collector' log
		SwitchToLog();
	// 'Wx'
		if (m_isWxProject)
		{
		// initialisation progress bar
			BuildLogger::g_CurrentProgress = 0;
		// Extract
            //=================================
			ok = m_pCreateWx->ExtractProject();
			//=================================
			if (!ok)
			{
				Mes = "---> ";
				if (!m_pCreateWx->isInitialized())
				{
					Mes += _("Could not initialize the project.");
					Mes += Lf + _("Cannot continue in") + Space;
				}
				else
				if (m_pCreateWx->isCancelled())
				{
					Mes += _("You have") + quote(_("CANCELLED")) ;
				}
				else
				if (m_pCreateWx->isStopped())
				{
					Mes += _("You have") + quote(_("STOPPED")) ;
				}
				else
                if (m_pCreateWx->hasNoproject())
				{
                    Mes += _("No project loaded") + " !!" + quote(_("STOPPED"));
				}
				else
				{
					Mes += "Collector::OnMenuExtractPrj(...) => " ;
					Mes +=  _("An error unknown has") + quote(_("ABORTED")) ;
				}

				Mes += _("the current operation in") + Space ;
				Mes += _(quote("Extract from project")) + " ! <---" ;
				m_pCreateWx->ShowInfo(Mes);
				_printWarn(Mes);
			}
			else
			{
				// end ... ??
			}
		}
///-----------------------------------------------------------------------------
	// 'Qt'
		if (m_isQtProject)
		{
            m_State = fbExtractPrj;
            Mes = _("The extraction of the 'Qt' part of")  + quote(m_pProject->GetTitle());
			Mes += _("is not yet operational") + " ...";
			_printError(Line(Mes.Len()) );
			_printWarn(Mes);
			_printError(Line(Mes.Len()) );
		/*	TODO ...
		// in waiting
            m_State = fbActionExtract;
			ok = m_pCreateQt->ExtractProject(listAndExtract);
			if (!ok)
			{
				Mes = _("Could not extract the 'Qt' project.");
				Mes += Lf + _("Cannot continue");
				m_pCreateQt->ShowError(Mes);
				_printError(Mes);
			}
		*/
		}
///-----------------------------------------------------------------------------
	// enable menu item : first state
		actualizeGraphState(fbWaitingForStart);

		SwitchToLog() ;
	}

_printD("	<= End 'Collector::OnMenuExractProject(...)'" );
}

///-----------------------------------------------------------------------------
// Called by an item of menu bar or tool bar
//
//	Called by :
// 	1. Collector::callActions(wxCommandEvent& _pEvent):1,
//
// Call to :
// 	1. CreateForWx::ListProject(const wxString& _key):1,
// 	2. CreateForWx::isAllRight():1,
// 	3. CreateForWx::ExtractProject():1,
// 	4. CreateForQt::ListProject(const wxString& _key):1,
// 	5. CreateForQt::isAllRight():1,
// 	6. CreateForQt::ExtractProject():1,

void Collector::OnMenuListExtractPrj(wxCommandEvent& _pEvent)
{
_printD("=> Begin 'Collector::OnMenuListExtractPrj(...) : 'checked':" + strBool(_pEvent.IsChecked()));

	wxUint64 id = _pEvent.GetId();
// only good id's
	if(id == idMbarListExtractPrj || id == idTbarListExtractPrj)
	{
    //1- List
		_pEvent.SetId(idTbarListPrj);
		//=====================
		OnMenuListPrj(_pEvent);
		//=====================
    //2- Extract
        // the base project 'Wx'
		if (m_isWxProject)
		{
		    bool ok =  m_pCreateWx->isAllRight();
            if (ok)
            {
                _pEvent.SetId(idTbarExtractPrj);
                //========================
                OnMenuExtractPrj(_pEvent);
                //========================
            }
            else
            {
               Mes = "m_pCreateWx->isAllRight() => " + strBool(ok)  + " !!";
               _printError(Mes);
            }
        }

        // the base project 'Qt'
		if (m_isQtProject)
		{
            if (m_pCreateQt->isAllRight())
            {
                _pEvent.SetId(idTbarExtractPrj);
                //========================
                OnMenuExtractPrj(_pEvent);
                //========================
            }
        }
	}

_printD("	<= End 'Collector::OnMenuListExtractProject(...)'" );
}

///-----------------------------------------------------------------------------
// List workspace
//
// Called by :
//  1. Collector::callActions(wxCommandEvent& _pEvent):1,
//	2. Collector::OnMenuListExtractWS(wxCommandEvent& _pEvent):1,

// Call to
//	1. Collector::actualizeGraphState(const colState& _state):3,
//	2. CreateForWx::ListWS():1,
//	3. Pre::isInitialized():1,
// 	4. Pre::isCancelled():1,
//	5. Pre::isStopped():1,
//	6. CreateForQt::ListWS():1,
//
void Collector::OnMenuListWS(wxCommandEvent& _pEvent)
{
_printD("=> Begin 'Collector::OnMenuListWS(...)' with 'checked':" + strBool(_pEvent.IsChecked()));

	bool ok = false; wxInt32 nbstr;
	wxUint64 id = _pEvent.GetId();
// only good id's
	if(id == idMbarListWS || id == idTbarListWS)
	{
	// display 'm_Collector' log
		SwitchToLog();
	// the base project 'Wx'
		if (m_isWxProject)
		{
        // init po header => line number header
            //==================================
			nbstr =  m_pCreateWx->iniHeaderPo();
            //==================================
		// initialisation progress bar
			BuildLogger::g_CurrentProgress = 0;
			//==============================
			ok = m_pCreateWx->ListWS(nbstr);
			//==============================
			if (!ok)
			{
				Mes = "---> ";
				if (!m_pCreateWx->isInitialized())
				{
					Mes += _("Could not initialize the worspace project.");
					Mes += Lf + _("Cannot continue in") + Space;
				}
				else
				if (m_pCreateWx->isCancelled())
				{
					 Mes+= _("You have") + quote(_("CANCELLED")) ;
				}
				else
				if (m_pCreateWx->isStopped())
				{
					Mes += _("You have") + quote(_("STOPPED")) ;
				}
				else
                if (m_pCreateWx->hasNoproject())
				{
                    Mes += _("No project loaded") + " !!" + quote(_("STOPPED"));
				}
				else
				{
					Mes += "Collector::OnMenuListWS(...) => " ;
					Mes += _("An error unknown has") + quote(_("ABORTED")) ;
				}

				Mes += _("the current operation in") + Space ;
				Mes += quote(_("List from workspace")) + " ! <---" ;
				m_pCreateWx->ShowInfo(Mes);
				_printWarn(Mes);
			}
		// ok is true
			else
			{
                if (nbstr)
                {
                // 'ListWS'
                    if (m_State == fbListWS)
                    {
                        Mes = Eol +  ">>> " ;
						Mes += _("YOU CAN NOW USE the menu");
                        Mes +=  quote(_("Extract from workspace")) ;
                        Mes +=  " <<<";
                        _printWarn(Mes);
                    }
                // 'ListWS' followed by 'ExtractWS'
                    else
                    {
                    }
                }
                else
                {
                // 'LisWS'
                    if (m_State == fbListWS)
                    {
                        Mes = ">>> !! " ;
                        Mes += _("Your workspace has no strings to translate") ;
                        Mes +=  " !! <<<";
                        _printWarn(Mes);
                    }
                // 'LisWS' followed by 'ExtractWS'
                    else
                    {
                        // ...
                    }
                }
            }
		}
///-----------------------------------------------------------------------------
	// the base project 'Qt'
		if (m_isQtProject)
		{

			Mes = _("The listage of the 'Qt' workspace is not yet operational") + " ...";
			_printError(Line(Mes.Len()) );
			_printWarn(Mes);
			_printError(Line(Mes.Len()) );
		/*
            m_State = fbListWS;
			ok = m_pCreateQt->ListWS();
			if (!ok)
			{
				Mes = _("Could not list the worspace project.");
				Mes += Lf + _("Cannot continue.");
				m_pCreateQt->ShowError(Mes);
				_printError(Mes);
			}
		*/
		}
///-----------------------------------------------------------------------------
	// enable menu item
		if (ok&& nbstr)		actualizeGraphState(fbWaitForExtractWS);
		// first state
		else 		        actualizeGraphState(fbWaitingForStart);
	}

_printD("	<= End 'Collector::OnMenuListWS(...)' ");
}

///-----------------------------------------------------------------------------
// Called by an item of menu bar or tool bar
//
//	Called by :
//  1. Collector::callActions(wxCommandEvent& _pEvent):1,

// Call to
//	1. Collector::actualizeGraphState(const colState& _state):3,
//	2. Pre::ExtractWS():1,
// 	3. Pre::isCancelled():1,
//	4. Pre::isStopped():1,
//	5. CreateForQt::ExtractWS():1,
//
void Collector::OnMenuExtractWS(wxCommandEvent& _pEvent)
{
_printD("=> Begin 'Collector::OnMenuExractWS(...)' with 'checked':" + strBool(_pEvent.IsChecked()));

	bool ok = false;
	wxUint64 id = _pEvent.GetId();
// only good id's
	if(id == idMbarExtractWS || id == idTbarExtractWS)
	{
	// display 'm_Collector' log
		SwitchToLog();
	// enable menu item
		actualizeGraphState(fbExtractWS);
	// the base project 'Wx'
		if (m_isWxProject)
		{
		// initialisation progress bar
			BuildLogger::g_CurrentProgress = 0;
		//
			m_State = fbExtractWS ;
            //============================
			ok = m_pCreateWx->ExtractWS();
			//============================
			if (!ok)
			{
				Mes = "---> ";
				if (!m_pCreateWx->isInitialized())
				{
					Mes += _("Could not initialize the worspace project.");
					Mes += Lf + _("Cannot continue in");
				}
				else
				if (m_pCreateWx->isCancelled())
				{
					 Mes += _("You have") + quote(_("CANCELLED")) ;
				}
				else
				if (m_pCreateWx->isStopped())
				{
					Mes += _("You have") + quote(_("STOPPED")) ;
				}
				else
                if (m_pCreateWx->hasNoproject())
				{
                    Mes += _("No project loaded") + " !!" + quote(_("STOPPED"));
				}
				else
				{
					Mes += "Collector::OnMenuExtractWS(...) => " ;
					Mes += _("An error unknown has") + quote(_("ABORTED")) ;
				}

				Mes += _("the current operation in") + Space ;
				Mes += _(quote("Extract from workspace")) + " ! <---" ;
				m_pCreateWx->ShowInfo(Mes);
				_printWarn(Mes);
			}
			else
			{
				// ...
			}
		}
///-----------------------------------------------------------------------------
	// the base project 'Qt'
		if (m_isQtProject)
		{
			Mes = _("The extraction of the 'Qt' workspace is not yet operational") +" ...";
			_printError(Line(Mes.Len()) );
			_printWarn(Mes);
			_printError(Line(Mes.Len()) );
		/*
             m_State = fbExtractWS ;
			ok = m_pCreateQt->ExtractWS();
			if (!ok)
			{
				Mes = _("Could not extract the worspace project") + Dot;
				Mes += Lf + _("Cannot continue");
				m_pCreateQt->ShowError(Mes);
				_printError(Mes);
			}
		*/
		}
///-----------------------------------------------------------------------------
	// enable menu item : first state
		actualizeGraphState(fbWaitingForStart);

		SwitchToLog() ;
	}

_printD("	<= End 'Collector::OnMenuExractWS(...)' ");
}

///-----------------------------------------------------------------------------
// Called by an item of menu bar or tool bar
//
//	Called by :
//	1. Collector::callActions(wxCommandEvent& _pEvent):1,

// Call to
//	1. Collector::OnMenuListWS(wxCommandEvent& _pEvent):1,
//  2. CreateForWx::isAllRight():1,
//	3. Collector::OnMenuExtractWS(wxCommandEvent& _pEvent):1,

void Collector::OnMenuListExtractWS(wxCommandEvent& _pEvent)
{
_printD("=> Begin 'Collector::OnMenuListExtractWS('checked':" + strBool(_pEvent.IsChecked()));

	wxUint64 id = _pEvent.GetId();
// only good id's
	if(id == idMbarListExtractWS || id == idTbarListExtractWS)
	{
    //1- List
        _pEvent.SetId(idTbarListWS);
        //====================
        OnMenuListWS(_pEvent);
        //====================
	//2- Extract
        // the base project 'Wx'
		if (m_isWxProject)
		{
            if (m_pCreateWx->isAllRight())
            {
                _pEvent.SetId(idTbarExtractWS);
                //=======================
                OnMenuExtractWS(_pEvent);
                //=======================
            }
        }

        // the base project 'Qt'
		if (m_isQtProject)
		{
            if (m_pCreateQt->isAllRight())
            {
                _pEvent.SetId(idTbarExtractWS);
                //=======================
                OnMenuExtractWS(_pEvent);
                //=======================
            }
        }
	}

_printD("	<= End 'Collector::OnMenuListExtractWS(...)'" );
}

///-----------------------------------------------------------------------------
// Called by an item of menu bar or tool bar
//
// Called by :
// 	1. Collector::callActions(wxCommandEvent& _pEvent):1,
// 	2. Collector::idMbarCleanTemp
// 	3. Collector::idTbarCleanTemp
//
// Call to :
// 	1. Collector::SwitchToLog():1,
//	2. Collector::actualizeGraphState(const colState& _state):3,
//	3. CreateForWx::Cleantemp():1,
//	4. CreateForQt::Cleantemp():1,
//
void Collector::OnMenuCleanTemp(wxCommandEvent& _pEvent)
{
_printD("=> Begin 'Collector::OnMenuCleanTemp(...)'" );

	wxUint64 id = _pEvent.GetId();
// only good id's
	if(id == idMbarCleanTemp || id == idTbarCleanTemp)
	{
	// display 'm_Collector' log
		SwitchToLog();
	// enable menu item 'Stop'
		actualizeGraphState(fbCleanTemp);
	// the base project 'Wx'
		if (m_isWxProject)
		{
            //=======================
            m_pCreateWx->Cleantemp();
            //=======================
		}

	// the base project 'Qt'
		if (m_isQtProject)
		{
			Mes = _("The 'CleanTmp' of the 'Qt' is not yet operational") + " ...";
			_printError(Line(Mes.Len()) );
			_printWarn(Mes);
			_printError(Line(Mes.Len()) );
			/*
			ok = m_pCreateQt->Cleantemp();
			if (!ok)
			{
				Mes = _("Could not clean the temporary strings") + Dot;
				Mes += Lf + _("Cannot continue");
				m_pCreateQt->ShowError(Mes);
				_printError(Mes);
			}
			*/
		}

	// enable menu item : first state
		actualizeGraphState(fbWaitingForStart);
	}

_printD("	<= End 'Collector::OnMenuCleanTemp(...)'" );
}

// ----------------------------------------------------------------------------
//	 Detects the project library type
//
// Called by :
//	1. Collector::OnActivateProject(CodeBlocksEvent& _pEvent):1,
//	2. Collector::OnActivateTarget(CodeBlocksEvent& _pEvent):1,
//	3. Collector::SwitchToLog():1,
//
// Call to :
//	1. CreateForWx::detectLibProject(cbProject * _pProject, bool _report):1,
//	2. CreateForQt::detectLibProject(cbProject * _pProject, bool _report):1,
//
wxString Collector::detectTypeProject(cbProject * _pProject, const bool _report)
{
_printD("=> Begin 'Collector::detectTypeProject(..., " + strBool(_report) + ")'" );

// display 'm_Collector' log
	SwitchToLog();

	m_typeProject = wxEmptyString;
	if (! _pProject)	return m_typeProject ;

	m_isWxProject = m_isQtProject = false;

// 'Wx' libraries ?
    //================================================================
	m_isWxProject = m_pCreateWx->detectLibProject(_pProject, _report);
	//================================================================
	if (m_isWxProject)		m_typeProject = "_WX";

// 'Qt' libraries ?
    //================================================================
    m_isQtProject = m_pCreateQt->detectLibProject(_pProject, _report);
    //================================================================
	if (m_isQtProject)		m_typeProject = "_QT";

// 'Wx' and 'Qt' : differents targets !!
	if (m_isWxProject && m_isQtProject)		m_typeProject = "_WX_QT";

_printD("	<= End 'Collector::detectTypeProject(...)' => " + quote(m_typeProject) );

	return m_typeProject;
}

///-----------------------------------------------------------------------------
// Validate messages to 'Collector log'
//		- called ONLY when the plugin is manually loaded
//
// Called by :  cbEVT_PLUGIN_INSTALLED
//	1.  PluginManager::InstallPlugin(const wxString& pluginName, bool forAllUsers, bool askForConfirmation):1,
//
// Call to : build an event 'cbEVT_PROJECT_ACTIVATE'
//	1. Collector::'OnActivateProject(CodeBlocksEvent& _pEvent):1,

//
void Collector::OnPluginLoaded(CodeBlocksEvent& _pEvent)
{
_printD("=>	Begin 'Collector::OnPluginLoaded(...)");
// just for debug
/*
	Mes = "Collector::OnPluginLoaded(...) -> ";
	Mes +=  " 'Collector' is manually loaded";
	//Mes += Space + "-> m_initDone = " + strBool(m_initDone) ;
	_printWarn(Mes) ;
**/
// the active project
	m_pProject = m_pMprj->GetActiveProject();
	if (m_pProject)
	{
	// pseudo event : call 'OnActivateProject(CodeBlocksEvent& _pEvent)'
		m_pseudoEvent = true;
		CodeBlocksEvent evt(cbEVT_PROJECT_ACTIVATE, 0, m_pProject, 0, this);
		//=====================
		OnActivateProject(evt);
		//=====================
	}

	Mes.Clear();
// The event processing system continues searching
	_pEvent.Skip();

_printD("	<=	End 'Collector::OnPluginLoaded(...)");
}

///-----------------------------------------------------------------------------
//	Delete the pre-builders and do de-initialization for plugin
//
// Called by : 'cbEVT_PLUGIN_RELEASED'
//	1. cbPlugin::Release(bool appShutDown):1,
//
void Collector::OnRelease(bool _appShutDown)
{
//1- delete builders
	if (m_pCreateWx)
	{
		delete m_pCreateWx;
		m_pCreateWx = nullptr;
	}
	if (m_pCreateQt)
	{
		delete m_pCreateQt;
		m_pCreateQt = nullptr;
	}

//2-  delete log
	if(m_pLogMan)
    {
        if(m_pCollectLog)
        {
            CodeBlocksLogEvent evt(cbEVT_REMOVE_LOG_WINDOW, m_pCollectLog);
            m_pM->ProcessEvent(evt);
			m_pCollectLog = nullptr;
        }
    }

//3- others pointers
	m_pProject = nullptr;
	m_pMenuBar = nullptr;
	m_pToolBar = nullptr;
	m_pmCollect = nullptr;

//4- do de-initialization for your plugin
    // if appShutDown is true, the plugin is unloaded because Code::Blocks is being shut down,
    // which means you must not use any of the SDK Managers
    // NOTE: after this function, the inherited member variable
    // m_IsAttached will be FALSE...
    if(!_appShutDown)
		m_pM->RemoveAllEventSinksFor(this);
}

///-----------------------------------------------------------------------------
// Switch to a log
//
// Called by :
//	1. OnAttach():1,
//	2. ...
//
void Collector::SwitchToLog(const int _indexLog)
{
// display a log
	CodeBlocksLogEvent evtSwitch(cbEVT_SWITCH_TO_LOG_WINDOW, _indexLog);
	m_pM->ProcessEvent(evtSwitch);
}
void Collector::SwitchToLog()
{
// display the 'Collector log'
	if(m_pCollectLog)
	{
		CodeBlocksLogEvent evtSwitch(cbEVT_SWITCH_TO_LOG_WINDOW, m_pCollectLog);
		m_pM->ProcessEvent(evtSwitch);
	}
}
///-----------------------------------------------------------------------------
// Append text to log and drives the progress bar
//
// Called by :
//	1. all 'printxxx(wxString)' not '_printxxx(...)'
//
void Collector::MesToLog(wxString _text, const Logger::level _lv)
{
    if(! m_pCollectLog)  return;
// add 'g_CurrentProgress' value string to head for verify
    if (PRINT_DISPLAY)
        _text.Prepend(strInt(++BuildLogger::g_CurrentProgress)) ;
    else
        ++BuildLogger::g_CurrentProgress;

    m_pCollectLog->Append(_text, _lv);

	if (! m_pCollectLog->m_pProgress)   return;

// progressing
//	if (BuildLogger::g_CurrentProgress < BuildLogger::g_MaxProgress)
	{
		m_pCollectLog->m_pProgress->SetRange(BuildLogger::g_MaxProgress);
		m_pCollectLog->m_pProgress->SetValue(BuildLogger::g_CurrentProgress);
	}
}

///-----------------------------------------------------------------------------
// Activate a project
//		called before a 'cbEVT_PROJECT_NEW' !!
//     called when workspace is closed !!
//
// Called by :
//		1. event 'cbEVT_PROJECT_ACTIVATE'
//
// Calls to :
// 	1. Collector::detectTypeProject(cbProject * _pProject, bool _report)
// 	2. Collector::SwitchToLog():1,
// 	3. Collector:: actualizeGraphState(colState _state):2,
//
void Collector::OnActivateProject(CodeBlocksEvent& _pEvent)
{
_printD("=> Begin 'Collector::OnActivateProject(...)");

// project manager is busy
	if (m_pMprj && m_pMprj->IsBusy())
    {
        _pEvent.Skip(); return;
    }

	SwitchToLog();
// the active project
	cbProject *pPrj = _pEvent.GetProject();
	if(!pPrj)
	{
		Mes = Space + "Collector::OnActivateProject() : ";
		Mes += _("no project supplied") + " !!"; _printError(Mes);
		_pEvent.Skip(!m_pseudoEvent);
		m_pseudoEvent = false;
		return;
	}

// actualize the project here
	m_pProject = pPrj;
	m_Nameactivetarget = m_pProject->GetActiveBuildTarget();
// DEBUG
//* *********************************************************
//	m_pCreateWx->beginDuration("OnActivateProject(...)");
//* *********************************************************
	Mes = _("This active project")  + Space ;
	Mes += quoteNS(m_pProject->GetTitle()  + "::" + m_Nameactivetarget)  ;
	Mes +=  Space ;
// detect project type
    //=========================================================
	wxString type = detectTypeProject(m_pProject, WITH_REPORT);
	//=========================================================
	if (type == wxEmptyString)
	{
		Mes +=  _("uses an unsupported library")   + " !!";
		_printWarn(Mes);

		actualizeGraphState(fbNone);
		_pEvent.Skip() ; return;
	}
	else
	if (type == "_WX" )
	{
		Mes += _("uses 'Wx'") ;
		m_isWxProject = true;
	}
	else
	if (type == "_QT" )
	{
		Mes += _("uses 'Qt'") ;
		m_isQtProject = true;
	}
	else
	if (type == "_WX_QT")
	{
		Mes += _("uses in different targets 'Wx' and 'Qt'") ;
		m_isWxProject = true; m_isQtProject = true;
	}
	_printWarn(Mes);

// menus enabled and state machine init
	actualizeGraphState(fbWaitingForStart);

// DEBUG
//* *******************************************************
//	m_pCreateWx->endDuration("OnActivateProject(...)");
//* *******************************************************
	Mes.Clear();

_printD("	<= End ' Collector::OnActivateProject(...)");
}

///-----------------------------------------------------------------------------
// Activate a target
//		called after a project is closed !!
//	    called before a project is opened
//
// Called by :
//	1. event 'cbEVT_BUILDTARGET_SELECTED'
//
// Calls to :
// 	1. Pre::isCommandTarget(const wxString& _nametarget):1,
// 	2. Collector::detectTypeProject(cbProject * _pProject, bool _report):1,
// 	3. Collector:: actualizeGraphState(colState _state):2,
//
void Collector::OnActivateTarget(CodeBlocksEvent& _pEvent)
{
_printD("=> Begin 'Collector::OnActivatetarget(...)");

// project manager is busy
	if (m_pMprj && !m_pMprj->IsBusy())
	{
//_printError("OnActivateProject:: m_pMprj->IsBusy() is 'true' !!!");
		_pEvent.Skip(); return;
	}

// a target can exist : nor a 'Q't nor 'Wx' project
	if (!m_isQtProject && !m_isWxProject)
	{
		_pEvent.Skip(); return;
	}
/* for debug
	if (m_isWxProject)
		_print("m_isWxProject = " + strBool(m_isWxProject) );
	else
	if (m_isQtProject)
		_print("m_isQtProject = " + strBool(m_isQtProject) );
*/
// the active project
	cbProject *pPrj = _pEvent.GetProject();
	if(!pPrj)
	{
        Mes = Space + "Collector::OnActivateTarget() : ";
		Mes += _("no project supplied") + " !!"; _printError(Mes);
		_pEvent.Skip();
		return;
	}
//  it's not the current project :  is it possible ??
	if ( m_pProject != pPrj)
	{
// just for debug
	//	Mes += quote(prj->GetTitle()) + " : " ;
	//	Mes += _("event project is not the current project !!"); 	_printWarn(Mes);
// <=
		_pEvent.Skip();  return;
	}

// DEBUG
//* *********************************************************
//	m_pCreateWx->beginDuration("OnActivateTarget(...)");
//* *********************************************************

// the active target
	wxString nametarget  =  _pEvent.GetBuildTargetName() ;
	if (nametarget.IsEmpty() )
	{
	// test if the project is open
		if (m_pMprj->IsProjectStillOpen(m_pProject))
		{
			Mes = Space + _("no target supplied") + " !!"; _printError(Mes);
		}
		_pEvent.Skip(); 	return;
	}
// is a command target
    //===========================================
	if (m_pCreateWx->isCommandTarget(nametarget))
	//===========================================
	{
		Mes =  Tab + quote("::" + nametarget);
		Mes += Tab + _("is a command target") + " !!" ; _printWarn(Mes);
		_pEvent.Skip();
		return;
	}
//  new target ?
	if (nametarget != m_Nameactivetarget)
	{
		m_Nameactivetarget = nametarget;
    // check it's libraries
		Mes = _("This active project")  + Space ;
		Mes += quoteNS(m_pProject->GetTitle()  + "::" + m_Nameactivetarget)  ;
		Mes +=  Space ;
	// detect project type
        //=========================================================
		wxString type = detectTypeProject(m_pProject, WITH_REPORT);
		//=========================================================
		if (type == wxEmptyString)
		{
			Mes +=  _("uses an unsupported library")   + " !!";
			_printWarn(Mes);
			actualizeGraphState(fbNone);
			_pEvent.Skip() ; return;
		}
		else
		if (type == "_WX" )
		{
			Mes += _("uses 'Wx'") ;
			m_isWxProject = true;
		}
		else
		if (type == "_QT" )
		{
			Mes += _("uses 'Qt'") ;
			m_isQtProject = true;
		}
		else
		if (type == "_WX_QT")
		{
			Mes += _("uses in different targets 'Wx' and 'Qt'") ;
			m_isWxProject = true; m_isQtProject = true;
		}
		_printWarn(Mes);

	// menu and state
		actualizeGraphState(fbWaitingForStart);
	}

	Mes.Clear();
// DEBUG
//* *******************************************************
//	m_pCreateWx->endDuration("OnActivateTarget(...)");
//* *******************************************************
_printD("	<= End ' Collector::OnActivatetarget(...)");

// The event processing system continues searching
	_pEvent.Skip();
}

///-----------------------------------------------------------------------------
//	Extract image ressource from 'Collector.zip'
//
// Called by :
//	1. Collector::LoadAllPNG():16,
//
wxBitmap Collector::LoadPNG(const wxString & _name)
{
_printD("=> Begin Collector::LoadPNG(" + quoteNS(_name) + ")" );

    wxFileSystem filesystem;
    wxString filename = ConfigManager::ReadDataPath();
    filename += cSlash + NamePlugin + ".zip#zip:" + _name;
    wxFSFile *file = filesystem.OpenFile(filename, wxFS_READ | wxFS_SEEKABLE);
    wxString Mes ;
    if (file == nullptr)
    {
        Mes = _("File not found") + " :" + quote(filename) ;
    // to 'Code::Blocks' log !
        _PrintError(Mes);
        return wxBitmap();
    }

    wxImage img;
    img.LoadFile(*file->GetStream(), wxBITMAP_TYPE_PNG);
    img.ConvertAlphaToMask();
    wxBitmap bmp(img);
    if (!bmp.IsOk())
    {
        wxBell();
        Mes = _("File not loaded correctly") + " :" + quote(filename) ;
        _PrintError(Mes);
        // to 'Code::Blocks' log
        return wxBitmap();
    }

_printD("   <= End Collector::LoadPNG(...)");

    return bmp;
}
///-----------------------------------------------------------------------------
//	Extract all images PNG from 'Collector.zip'
//
// Called by :
//	1. Collector::OnAttach():1,
//
bool Collector::LoadAllPNG()
{
_printD("=> Begin Collector::LoadPAllNG()" );

	bool ok = true;
// it's 'wxWidgets' logo
	m_bmLogoWx    = LoadPNG(_T("wxlogo.png"));
		if (!m_bmLogoWx.IsOk()) _printError("Error with 'm_bmLogoWx'");
// it's 'Qt' logo
	m_bmLogoQt    = LoadPNG(_T("qtlogo.png"));
		if (!m_bmLogoQt.IsOk()) _printError("Error with 'm_bmLogoQt'");

	m_bmList   = LoadPNG(_T("list.png"));
		if (!m_bmList.IsOk()) _printError("Error with 'm_bmList'");
	m_bmListPlus   = LoadPNG(_T("list+.png"));
		if (!m_bmListPlus.IsOk()) _printError("Error with 'm_bmListPlus'");

	m_bmExtract = LoadPNG(_T("extract.png"));
		if (!m_bmExtract.IsOk()) _printError("Error with 'm_bmExtract'");
	m_bmExtractPlus     = LoadPNG(_T("extract+.png"));
		if (!m_bmExtractPlus.IsOk()) _printError("Error with 'm_bmExtractPlus'");

	m_bmStop    	= LoadPNG(_T("stop.png"));
		if (!m_bmStop.IsOk()) _printError("Error with 'm_bmStop'");
	m_bmStopOff 	= LoadPNG(_T("stopoff.png"));
		if (!m_bmStopOff.IsOk()) _printError("Error with 'm_bmStopOff'");

	m_bmInit     	= LoadPNG(_T("initon.png"));
		if (!m_bmInit.IsOk()) _printError("Error with 'm_bmInit'");
	m_bmInitOff     = LoadPNG(_T("initoff.png"));
		if (!m_bmInitOff.IsOk()) _printError("Error with 'm_bmInitOff'");

	m_bmTemp     	= LoadPNG(_T("deltemp.png"));
		if (!m_bmTemp.IsOk()) _printError("Error with 'm_bmTemp'");
	m_bmTempOff     = LoadPNG(_T("deltempoff.png"));
		if (!m_bmTempOff.IsOk()) _printError("Error with 'm_bmtempOff'");

_printD("   <= End Collector::LoadAllPNG()");

	return ok;
}

// -----------------------------------------------------------------------------
// Update the state of all menu items, recursively, by generating
//	'wxEVT_UPDATE_UI' events for them
//
// Called by : receive enough
//
// Call to :
//
void Collector::OnUpdateUI(wxUpdateUIEvent& _pEvent)
{
_printD("=> Begin Collector::OnUpdateUI(...)");

	if (m_pMprj->GetProjects()->GetCount() == 0)
    {
        if (m_pToolBar)
            m_pToolBar->Enable(false);
        if (m_pMenuBar)
		{

			m_pMenuBar->FindItem(idMbarCollect)->Enable(false);
			m_pMenuBar->FindItem(idMbarWaitingForStart)->Enable(false);

			m_pMenuBar->FindItem(idMbarListPrj)->Enable(false);
			m_pMenuBar->FindItem(idMbarExtractPrj)->Enable(false);
			m_pMenuBar->FindItem(idMbarListExtractPrj)->Enable(false);

			m_pMenuBar->FindItem(idMbarListWS)->Enable(false);
			m_pMenuBar->FindItem(idMbarExtractWS)->Enable(false);
			m_pMenuBar->FindItem(idMbarListExtractWS)->Enable(false);

			m_pMenuBar->FindItem(idMbarCleanTemp)->Enable(false);
			m_pMenuBar->FindItem(idMbarStop)->Enable(false);
			m_pMenuBar->FindItem(idMbarSetting)->Enable(false);
		}
    }

    _pEvent.Skip();

_printD("	<= End Collector::OnUpdateUI(...)");
}

// ----------------------------------------------------------------------------
//	Buttons, ... in toolbar
//
//	Called by : Code::Bock "toolBar->..." at begin
//
bool Collector::BuildToolBar(wxToolBar* _pToolBar)
{
_printD("=> Begin 'Collector::BuildToolBar(...)'");

	if (!IsAttached() || !_pToolBar)    return false;

// future tool bar
	m_pToolBar = _pToolBar;
/*  TODO ...
// size *..png : 16, 20, 24, 28, 32, 40, 48, 56, 64
	const int toolbarSize = m_pM->GetImageSize(Manager::UIComponent::Toolbars);
// zip path
    const wxString prefix = ConfigManager::GetDataFolder()
                          + wxString::Format(wxFILE_SEP_PATH + NamePlugin + _T(".zip#zip:"),
                                             toolbarSize, toolbarSize);
//_printWarn("toolbarSize = " + strInt(toolbarSize) ) ;
// factor scale tool bar
//	const double scaleFactor = cbGetContentScaleFactor(*m_pToolBar);
//_printWarn("scaleFactor = " + strInt(scaleFactor));
*/
// text
	wxString label;
// two tools exclusives 'Wx' and 'Qt'
	label = _("Collect") + "-Wx" ; Mes = _("Tools for collect strings in sources 'wxWidgets'");
	m_pToolBar->AddRadioTool(idTbarCollectWx, Mes, m_bmLogoWx, wxNullBitmap, label);

//	label = _("Collect") + "-Qt" ; Mes = _("Tools for collect strings in sources 'Qt'");
//	m_pToolBar->AddRadioTool(idTbarCollectQt, Mes, m_bmLogoQt, wxNullBitmap, label );

	m_pToolBar->AddSeparator();
// combobox
	m_pTbarCboKeys = new wxComboBox(m_pToolBar, idTbarKey,
                                      _T("_"), wxDefaultPosition, wxSize(50,-1), 0, nullptr,
                                      wxCB_DROPDOWN | wxCB_READONLY);
	if (m_pTbarCboKeys)
	{
		m_pTbarCboKeys->SetToolTip(_("Key for translate"));

        label = _("Key"); Mes = _("Key for 'Collector'");
		m_pToolBar->AddControl(m_pTbarCboKeys);
	}
//	else
//		_printError("m_pTbarCboKeys is null !!");
	m_pToolBar->AddSeparator();

	label = _("List from project");
	Mes = _("List all translatable strings found in the project") ;;
	m_pToolBar->AddTool(idTbarListPrj, Mes, m_bmList, wxNullBitmap,
						wxITEM_NORMAL, label
						);
	label = _("Extract from project");
	Mes =  _("Extract all translatable strings found in the project") ;;
	m_pToolBar->AddTool(idTbarExtractPrj, Mes, m_bmExtract, wxNullBitmap,
						wxITEM_NORMAL, label
						);
	label = _("List and Extract from project");
	Mes =  _("List and Extract all translatable strings found in the project") ;;
	m_pToolBar->AddTool(idTbarListExtractPrj, Mes, m_bmListPlus, wxNullBitmap,
						wxITEM_NORMAL, label
						);

	m_pToolBar->AddSeparator();

	label = _("List from workspace");
	Mes = _("List all translatable strings found in the workspace") ;;
	m_pToolBar->AddTool(idTbarListWS, Mes, m_bmList, wxNullBitmap,
						wxITEM_NORMAL, label
						);
	label = _("Extract from workspace");
	Mes =  _("Extract all translatable strings found in the workspace") ;;
	m_pToolBar->AddTool(idTbarExtractWS, Mes, m_bmExtract, wxNullBitmap,
						wxITEM_NORMAL, label
						);
	label = _("List and Extract from workspace");
	Mes =  _("List and Extract all translatable strings found in the workspace") ;;
	m_pToolBar->AddTool(idTbarListExtractWS, Mes, m_bmListPlus, wxNullBitmap,
						wxITEM_NORMAL, label
						);
	m_pToolBar->AddSeparator();

	label = _("Stop");
	Mes = _("Stop process collect for 'Collect'");
	m_pToolBar->AddTool(idTbarStop, Mes, m_bmStop, m_bmStopOff,
						wxITEM_NORMAL, label
						);

	label = _("Clean temp");
	Mes = _("Removes temporary files");
	m_pToolBar->AddTool(idTbarCleanTemp, Mes, m_bmTemp, m_bmTempOff,
						wxITEM_NORMAL, label
						);

	label = _("Init graph");
	Mes = _("Resets the state of the state graph to the beginning");
	m_pToolBar->AddTool(idTbarWaitingForStart, Mes, m_bmInit, wxNullBitmap,
						wxITEM_NORMAL, label
						);

	m_pToolBar->Realize();
	m_pToolBar->SetInitialSize();

	m_pToolBar->Enable(true);
	m_pToolBar->EnableTool(idTbarCollectWx, true);
	// not operational !
	m_pToolBar->EnableTool(idTbarCollectQt, false);

	m_pToolBar->EnableTool(idTbarExtractPrj, false);
	m_pToolBar->EnableTool(idTbarExtractWS, false);
	m_pToolBar->EnableTool(idTbarStop, false);

	if (m_pTbarCboKeys)
	{
		// keys word after enabled
		if (m_pToolBar->GetToolState(idTbarCollectWx) )
		{
			m_pTbarCboKeys->Insert("_", 0);
		}
		else
		if (m_pToolBar->GetToolState(idTbarCollectQt) )
		{
			//m_pTbarCboKeys->Insert("QObject::tr", 0);
			//m_pTbarCboKeys->Insert("tr", 0);
		}
		// the last insert
		m_pTbarCboKeys->SetSelection(0);
	}

_printD("    <= End 'Collector::BuildToolBar(...)'");

	return true ;
}

///-----------------------------------------------------------------------------
