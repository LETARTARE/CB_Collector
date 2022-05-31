/***************************************************************
 * Name:      collector.h
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2020-05-10
 * Modified:  2022-05-31
 * Copyright: LETARTARE
 * License:   GPL
 **************************************************************/
//------------------------------------------------------------------------------
#ifndef _COLLECTOR_H_
#define _COLLECTOR_H_
//------------------------------------------------------------------------------
#include "colstate.h"   // for 'ColState'
//------------------------------------------------------------------------------
#include <wx/wxprec.h>
// For compilers that support precompilation, includes <wx/wx.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
// for class cbPlugin
#include <cbplugin.h>
#include <wx/event.h>
#include "buildlogger.h"
//------------------------------------------------------------------------------
//class BuildLogger;
class CreateForWx;
class CreateForQt;
//------------------------------------------------------------------------------

/** \class  Collector
 *  @brief Collector plugin main class
*/
class Collector : public cbPlugin, public ColState
{
  public:
    /** \brief Constructor
     */
    Collector();

    /** \brief Destructor
      */
    virtual ~Collector() {}

	/** \brief Log tab in the message pane
     */
	static BuildLogger* m_pCollectLog;
    /** \brief Append log message to 'Collector log'
     *  @param  _text : message to display
     *  @param  _lv : level -> Logger::caption, info, warning, success, error,
     *               critical, failure, pagetitle, spacer, asterisk
     */
	static void MesToLog(wxString _text, const Logger::level _lv = Logger::info);

 private:

    /** \brief Builder for 'Wx'
      */
    CreateForWx * m_pCreateWx = nullptr;
    /** \brief Builder for 'Qt'
      */
    CreateForQt * m_pCreateQt = nullptr;

///-----------------------------------------------------------------------------

    /** \brief  menu bar items constant identificators
     */
    static const wxUint64 idMbarCollect;
    static const wxUint64 idMbarCollectWx;
    static const wxUint64 idMbarCollectQt;

    static const wxUint64 idMbarListPrj;
    static const wxUint64 idMbarListWS;
    static const wxUint64 idMbarExtractPrj;
    static const wxUint64 idMbarExtractWS;
    static const wxUint64 idMbarListExtractPrj;
    static const wxUint64 idMbarListExtractWS;

    static const wxUint64 idMbarWaitingForStart;
    static const wxUint64 idMbarStop;
    static const wxUint64 idMbarCleanTemp;
    // setting
    static const wxUint64 idMbarSetting;

    /** \brief  tools bar entries constant identificators
     */
    static const wxUint64 idTbarCollectWx;
    static const wxUint64 idTbarCollectQt;
    static const wxUint64 idTbarListPrj;

    static const wxUint64 idTbarExtractPrj;
    static const wxUint64 idTbarListExtractPrj;
    static const wxUint64 idTbarListWS;
    static const wxUint64 idTbarExtractWS;
    static const wxUint64 idTbarListExtractWS;

    static const wxUint64 idTbarWaitingForStart;
    static const wxUint64 idTbarStop;
    static const wxUint64 idTbarCleanTemp;
    // settings
    static const wxUint64 idTbarKey;

    /** \brief  event table
     */
    DECLARE_EVENT_TABLE();
// ----------------------------------------------------------------------------

  public:

    /** \brief Invoke configuration dialog.
     * \note NOT USED
	 */
    virtual int Configure()
		{return 0;}

    /** \brief Return the plugin's configuration priority.
      * This is a number (default is 50) that is used to sort plugins
      * in configuration dialogs. Lower numbers mean the plugin's
      * configuration is put higher in the list.
      */
    virtual int GetConfigurationPriority() const
		{ return 50; }

    /** \brief Return the configuration group for this plugin. Default is cgUnknown.
      * Notice that you can logically OR more than one configuration groups,
      * so you could set it, for example, as "cgCompiler | cgContribPlugin".
      * \note NOT USED
      */
    virtual int GetConfigurationGroup() const
		{ return cgContribPlugin; }

    /** \brief This method is called by Code::Blocks and is used by the plugin
      * to add any menu items it needs on Code::Blocks's menu bar.\n
      * It is a pure virtual method that needs to be implemented by all
      * plugins. If the plugin does not need to add items on the menu,
      * just do nothing ;)
      * @param _pMenuBar the wxMenuBar to create items in
      */
    virtual void BuildMenu(wxMenuBar* _pMenuBar);

    /** \brief This method is called by 'Code::Blocks' and is used by the plugin
      * to add any toolbar items it needs on Code::Blocks's toolbar.\n
      * It is a pure virtual method that needs to be implemented by all
      * plugins. If the plugin does not need to add items on the toolbar,
      * just do nothing ;)
      * @param _pToolBar the wxToolBar to create items on
      * @return The plugin should return true if it needed the toolbar, false if not
      */
    virtual bool BuildToolBar(wxToolBar* _pToolBar) ;

  protected:
    /** \brief Any descendent plugin should override this virtual method and
      * perform any necessary initialization. This method is called by
      * Code::Blocks (PluginManager actually) when the plugin has been
      * loaded and should attach in Code::Blocks. When Code::Blocks
      * starts up, it finds and <em>loads</em> all plugins but <em>does
      * not</em> activate (attaches) them. It then activates all plugins
      * that the user has selected to be activated on start-up.\n
      * This means that a plugin might be loaded but <b>not</b> activated...\n
      * Think of this method as the actual constructor...
      */
    virtual void OnAttach();

    /** \brief Any descendent plugin should override this virtual method and
      * perform any necessary de-initialization. This method is called by
      * Code::Blocks (PluginManager actually) when the plugin has been
      * loaded, attached and should de-attach from Code::Blocks.\n
      * Think of this method as the actual destructor...
      * @param _appShutDown If true, the application is shutting down. In this
      *         case *don't* use Manager::Get()->Get...() functions or the
      *         behaviour is undefined...
      */
    virtual void OnRelease(bool _appShutDown);

  /* -------------------- personal methods --------------------------------- */

  private:
/* ------------------------ Collector -----------------------------------------*/

    /** \brief Only one method for all events
     *  \param _pEvent : '_pEvent.m_id' contains the identifier of the calling item
     */
    void OnMenuToState(wxCommandEvent& _pEvent);
    /** \brief Enable the menu items of '&Collect' and tools bar 'Collector'
	 * \param _state : enable only one state in state graph
     */
    ColState::colState actualizeGraphState(const colState& _state);
    /** \brief Call all 'OnMenu...()'
     *  \param _pEvent : used for call to 'OnMenu...(_pEvent)'
     */
    bool callActions(wxCommandEvent& _pEvent);

// ----------------------------------------------------------------------------

	/** \brief Choice for 'Wx' or 'Qt'
     *  \param _pEvent : unused actually
     */
    void OnMenuChoiceLib(wxCommandEvent& _pEvent) ;
    /** \brief Places the state graph on hold for selection
     *  \param _pEvent :  with checked = true
     */
    void OnMenuWaitingForStart(wxCommandEvent& _pEvent) ;

    /** \brief Places the status graph in the listing project report
     *  \param _pEvent : with checked = true
     */
    void OnMenuListPrj(wxCommandEvent& _pEvent) ;
    /** \brief Places the status graph in the extracting project report
     *  \param _pEvent :  with checked = true
     */
    void OnMenuExtractPrj(wxCommandEvent& _pEvent) ;
    /** \brief Places the status grap in the listing and extract report
     *  \param _pEvent :  with checked = true
     */
    void OnMenuListExtractPrj(wxCommandEvent& _pEvent) ;

    /** \brief Places the status graph in the listing workspace report
     *  \param _pEvent :  with checked = true
     */
    void OnMenuListWS(wxCommandEvent& _pEvent) ;
    /** \brief Places the status graph in the extracting workspace report
     *  \param _pEvent : with checked = true
     */
    void OnMenuExtractWS(wxCommandEvent& _pEvent) ;
	/** \brief Places the status grap in the listing and extract workspace report
	 *  \param _pEvent :  with checked = true
	 */
    void OnMenuListExtractWS(wxCommandEvent& _pEvent) ;

    /** \brief Places the status graph in the cleaning report and Delete the temporary directory
     *  \param _pEvent :  with checked = true
     */
    void OnMenuCleanTemp(wxCommandEvent& _pEvent) ;

    /** \brief  Places the status graph in stop report and Stop current action
     *  \param _pEvent  : with checked == true
     */
    void OnMenuStop(wxCommandEvent& _pEvent) ;

    /** \brief This method called by ??
	 *  @param _pEvent contains the event which call this method
	 */
    void OnUpdateUI(wxUpdateUIEvent& _pEvent);

// ----------------------------------------------------------------------------
  protected:

    /** \brief This method called by plugin is manually loaded
      * @param _pEvent contains the event which call this method
      */
    void OnPluginLoaded(CodeBlocksEvent& _pEvent);

    /** \brief This method called by project activate allows detect project using the
      * 'Wx' or 'Qt' libraries
      * @param _pEvent contains the event which call this method
      */
    void OnActivateProject(CodeBlocksEvent& _pEvent);
    /** \brief This method called by target activate allows detect target using the
      * 'Wx' or 'Qt' libraries
      * @param _pEvent Contains the event which call this method
      */
    void OnActivateTarget(CodeBlocksEvent& _pEvent);

  private:

    /** \brief Global used to record different messages for the construction report
     */
    wxString Mes = wxEmptyString;

    /** \brief Actual active project
     */
    cbProject* m_pProject = nullptr;
    /**  \brief Project type and presence
     */
    bool m_isQtProject = false, m_isQtActiveTarget = false
		,m_isWxProject = false, m_isWxXActiveTarget = false
		;
    /**  \brief Project type in ["", "_WX", "_QT", "_WX_QT"]
     */
    wxString m_typeProject = wxEmptyString ;
    /** \brief A pseudo event
     */
    bool m_pseudoEvent = false;

	/** \brief The name active target
     */
    wxString m_Nameactivetarget = wxEmptyString ;

   /** \brief Detects project type : 'Wx' or 'Qt'
     *  @param _pProject : the active project.
     *  @param _report : display report if true.
     *  @return type : '_WX' or '_QT' or ""
     */
    wxString detectTypeProject(cbProject * _pProject, const bool _report = false);

    /** \brief Show a log
     *  @param _indexLog : page index
     */
    void SwitchToLog(const int _indexLog);
    void SwitchToLog();

    /** \brief Extract one bitmap from *.zip
     *  @param _name : bitmap name
     *  @return a bitmap
     */
	wxBitmap LoadPNG(const wxString & _name);
	/** \brief Extract all bitmaps from *.zip
     *  @return a boolean
     */
	bool LoadAllPNG();

  private:

    /** \brief Index of our log tab (can this change during run time ??)
     */
    size_t m_LogPageIndex = 0,
    /**  Last log index
     */
        m_LastIndex = 0 ;

    /**  \brief last log
     */
    Logger * m_Lastlog = nullptr;

	/**  \brief Main manager
     */
    Manager* m_pM = nullptr;

	/**  \brief Project manager
	 */
    ProjectManager * m_pMprj = nullptr;
    /**  \brief Log manager
     */
    LogManager* m_pLogMan = nullptr;
	/**  \brief Plugin manager
     */
    PluginManager * m_pMplug = nullptr;

    /** \brief read the bar menu
     */
    wxMenuBar* m_pMenuBar = nullptr;
    /** \brief read the bar menu
     */
    wxToolBar* m_pToolBar = nullptr;
	/** \brief items menu '&Collect'
     */
    wxMenu * m_pmCollect = nullptr;
    /** \brief
     */
    wxComboBox*  m_pTbarCboKeys = nullptr;

    /** \brief "&Collect" menu position in menu bar
     */
    size_t  m_placeCollectX = wxNOT_FOUND;
    /** \brief "&Tools" menu position in menu bar
     */
    size_t  m_placeToolsX = wxNOT_FOUND;

    /** \brief bitmaps for menu and tool items
     */
    wxBitmap m_bmLogoWx = wxBitmap(), m_bmLogoQt = wxBitmap()
		,m_bmList = wxBitmap(), m_bmListOff = wxBitmap()
		,m_bmListPlus = wxBitmap(), m_bmListPlusOff = wxBitmap()
		,m_bmExtract = wxBitmap(), m_bmExtractOff = wxBitmap()
		,m_bmExtractPlus = wxBitmap(), m_bmExtractWSOff = wxBitmap()
        ,m_bmStop = wxBitmap(), m_bmStopOff = wxBitmap()
        ,m_bmInit = wxBitmap(), m_bmInitOff = wxBitmap()
        ,m_bmTemp = wxBitmap(), m_bmTempOff = wxBitmap()
        ;
};

#endif // _COLLECTOR_H_
//-------------------------------------------------
