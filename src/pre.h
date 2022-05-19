/*************************************************************
 * Name:      pre.h
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2020-05-10
 * Modified:  2022-05-18
 * Copyright: LETARTARE
 * License:   GPL
 *************************************************************
 */

//------------------------------------------------------------------------------
#ifndef _PRE_H_
#define _PRE_H_
//------------------------------------------------------------------------------
#include "colstate.h"   // for 'ColState'

// for linux
#ifdef __linux__
    #include "cbproject.h"
#endif
#include "projectmanager.h"
//------------------------------------------------------------------------------
#include <wx/filefn.h>
#include <wx/thread.h>
//------------------------------------------------------------------------------
class Collector;
class cbProject;
class CompileTargetBase;
class ProjectBuildTarget;
class MacrosManager;
class EditorManager;
class ConfigManager;
class ProjectManager;
class Manager;
class ProjectFile;
class TextCtrlLogger;
class wxMenuBar;
//------------------------------------------------------------------------------

/**	\class Pre
 *	\brief This class supports inherited classes to collect strings to be translated.
 *  \brief It cannot be instantiated because it has pure virtual methods
 *  \brief that will have to be defined in the inheriting classes.
 *  \brief This class inherit of 'ColState'
 */
class Pre : public ColState
{
	public:

		/** \brief Constructor
         *  @param _nameplugin  The plugin name.
         *  @param _logindex The active log page.
         */
		Pre(const wxString & _nameplugin, int _logindex);
		/**  \brief Destructor
		  */
		virtual ~Pre();

		/** \brief Adjusts variable depending on the platform
		 *  \return a formatted text
		 */
		wxString platForm() ;
		/** \brief Plugin name
		 *  @return plugin name
		 */
		wxString namePlugin();
		/** \brief Get the date followed by the time of construction of the plugin
		 *  @return date and time
		 */
		wxString GetDateBuildPlugin();
		/** \brief Get SDK version from 'cbplugin.h'
		 *  @return version du SDK ex: "1.19.0" for Code::Blocks 13.12
		 */
		wxString GetVersionSDK();

		/** \brief Call to 'cbMessageBox(_mes, "", wxICON_INFORMATION)'
		 *  \param _mes : message to be seen
		 */
		void ShowInfo(const wxString& _mes);
		/** \brief Call to 'cbMessageBox(_mes, "", wxICON_ERROR)'
		 *  \param _mes : message to be seen
		 */
		void ShowError(const wxString& _mes);

		/** \brief Locate and call a menu from string (e.g. "/Valgrind/Run Valgrind::MemCheck")
		 *  \note it's a copy of 'CodeBlocks::sc_globals.cpp::CallMenu(const wxString& menuPath)'		 *
		 *  \param _menuPath : Menu item to be called
		 *  \return menu identificateur or  wxNOT_FOUND il failed
		 */
		wxInt32 CallMenu(const wxString& _menuPath);

		/** \brief Save a file to disk with a file dialog
         *  @param _namefile : file name
         *	@return true if all right
         */
         bool saveAs(wxString & _namefile);

		/** \brief Set stop complement file creating
		 *  @param _abort : stop current action
		 */
		void setStop(const bool& _abort) ;
		/** \brief Set key for translate
		 *  @param _key : "_", ...
		 */
		void setKey(const wxString& _key = "_") ;

		/** \brief For globals boolan
		 *  \return true if is correct
		 */
		bool isInitialized();
		bool isStopped();
		bool isCancelled();
		/** \brief For globals boolan
		 *  \return true if 'm_Init = true' and 'm_Stop = false' and 'm_Cancel = false'
		 */
        bool isAllRight();
        /** \brief Give the number of strings to extract
		 *  \return number strings
		 */
		wxInt32 nbStringsToExtract();

		/** \brief Indicates if target is command only
		 *	@param _nametarget : target name
		 *  @return true if it's command target
		 */
		bool isCommandTarget(const wxString& _nametarget) ;
		/** \brief Indicates if target is command only
		 *	@param _pBuildTarget : target pointer
		 *  @return true if it's command target
		 */
		bool isCommandTarget(const ProjectBuildTarget * _pBuildTarget) ;

///-----------------------------------------------------------------------------
		/** \brief Collect strings to be translated
		 *  @param  _key : word key for translation
		 *  @param  _nbstr : all strings to extract
		 *  \return true is good
		 */
		bool ListProject(const wxString& _key, wxInt32 & _nbstr);
		/** \brief Extract strings to be translated
		 *  \return true is good
		 */
		bool ExtractProject();
		/** \brief List strings to be translated in workspace
		 *  @param  _nbstr : all strings to extract
		 *  \return true is good
		 */
		bool ListWS(wxInt32 & _nbstr);
		/** \brief Extract strings to be translated in workspace
		 *  \return true is good
		 */
		bool ExtractWS();
///-----------------------------------------------------------------------------

	protected:

		/** \brief Search libraries in project or target
		 *  @param _type : in ["_WX", "_QT", "_WX","_WX_QT"]
		 *  @param _pContainer : 'cbProject * Project' or 'ProjectBuildTarget * buildtarget'
		 *	 both inherited of 'CompileTargetBase'
		 *  @return true if finded
		*/
		bool hasLib(const wxString _type, CompileTargetBase * _pContainer) ;

		/**  \brief Give compileable targets list for project or virtual target
		 *   @return a table : of compileable target name
		 */
		wxArrayString compileableProjectTargets();

	protected:
	// ----------------------------- pure virtual ------------------------------

		/** \brief Detects the current libraries for project
		 *  \note  it's a virtual pure
		 *  @param _pProject : the active project
		 *  @param _report : display report if true.
		 *  @return true : if used
		 */
		virtual bool detectLibProject(cbProject * _pProject, bool _report = true) =0;

		/** \brief specific initialisation by inherited type
		 *  \note  it's a virtual pure
		 *  \param _type : in ["_WX", "_QT", "_WX","_WX_QT"]
		 *  \return true if good
		 */
		virtual bool initTailored(const wxString _type  = wxEmptyString ) = 0;
		/** \brief Tests readability for chain collection
		 *  \note  it's a virtual pure
		 *  \param _file : file name
		 *  \param _absolute : path file is absolute
		 *  \return number strings, if  < 0 =>  error
		 */
		virtual wxInt32 isReadableFile(const wxString& _file,
									const bool _absolute = false) = 0;

		/** \brief Display the strings to be extracted of code file
		 *  \note  it's a virtual pure
		 *  \param _shortfile : short file name
		 *  \return  number of strings extracted
		 */
		virtual wxInt32 listStringsCode(const wxString& _shortfile) = 0;

		/** \brief Display the strings to be extracted of resource file
		 *  \brief and Create à new file name temporary
		 *  \note  it's a virtual pure
		 *  \param  _file : resource file name
		 *  \return  number of strings extracted
		 */
		virtual wxInt32 listStringsRes(wxString& _file) = 0;
		/** \brief For the '*.xrc' file  return '*_xrc.str???' (??? in [1..n]
		 *  \note  it's a virtual pure
		 *  \param _file : '*.xrc'
		 *  \param _indexfree : 'xxx' where xxx is index in 'm_FileStrCreated'
		 *  \return the new file name '*_xrc.str???'
		 */
		virtual wxString expandName(wxString _file, wxInt32 _indexfree = 0) = 0;

		/** \brief Display the strings to be extracted of xml file
		 *  \note  it's a virtual pure
		 *  \param  _shortfile : short file name
		 *  \return   number of strings extracted
		 */
		virtual wxInt32 listStringsXml(const wxString& _shortfile) = 0;

		/** \brief Creating the 'name_project.pot' file
		 *  \note  it's a virtual pure
		 *  \param _prjfirst = true => plural projects, false => one project
		 *  \return true if good
		 */
		virtual bool createPot(bool _prjfirst) = 0;

// -----------------------------------------------------------------------------

	protected:
		/** \brief Global initialization
		 *  \param _type : in ["_WX", "_QT", "_WX","_WX_QT"]
		 *  \return	true if good
		 */
		bool Init(const wxString _type = wxEmptyString );
		bool Initbasic(const wxString _type  = wxEmptyString );
		/** \brief Release all variables
		 *  \param _pProject : project file
		 *  \param _withpot : with a 'name_project.pot'
		 *  \return	true if good
		 */
		bool releaseProject(cbProject * _pProject, bool _withpot = true);

		/** \brief Initialization of a table for temporary file
		 *  \return number strings created
		 */
		wxUint32 initFileStrcreated();

		/** \brief Retrieve a project by name
		 *  \param  _name : project name
		 *  \param  _index : return project index
		 *  \return project pointeur
		 */
		cbProject* findProject(const wxString& _name, wxInt32& _index);
		/** \brief Give a project index
		 *  \param  _name : project name
		 *  \return  return project index
		 */
		wxInt32 indexProject(const wxString& _name);

		/** \brief Search and memorize strings to be extracted
		 *  \param _posWS : workspace project position
		 *  \return true if good
		 */
		bool listing(const wxUint32& _posWS);
		/** \brief Extract the strings translatable from a project
		 *  \param _prjfirst : true if the first project
		 *  \param _pProject : project treated
		 *  \return	true if good
		 */
		bool extraction(bool _prjfirst = false, cbProject *_pProject = nullptr);

		/** \brief Search and memorize strings to be extracted from workspace
		 *  \return	true if good
		 */
		bool listingWS();
		/** \brief Extract the strings translatable from worspace
		 *  \return	true if good
		 */
		bool extractionWS();

		/** \brief Search the elegible files for extracting
		 *  \return numbers good files
		 */
		wxInt32 findGoodfiles();
		/** \brief List translatables strings from elegible files
		 *  \return numbers good files
		 */
		wxInt32 listGoodfiles();

		/** \brief Execute 'xgettext' for 'listing()' or 'extract()'
		 *  \param _shortfile :  it's a file for extract strings
		 *  \param _command : command line fot 'xgettext'
		 *  \param _prepend :  allows you to retrieve errors or messages at runtime.
		 *  \return if good : number chains to tranlate, else number advices
		 */
		wxInt32 GexewithError(const wxString& _shortfile, const wxString& _command,
							  const bool& _prepend = true);
		/**  \brief Launch a command line
		 *	 @param _command : command line
		 *	 @param _prepend_error : prepend error to return
		 *   @return empty string if it's correct else error message
		 */
		wxString executeAndGetOutputAndError(const wxString& _command,
											const bool& _prepend_error = true) ;
		/** \brief From a text, extract the different strings to be translated
		 *  \param _txt : one line of text
		 *  \return a string formatted in successive lines
		 */
		wxString xgettextOut(const wxString& _txt);
		/** \brief Retrieves error or warning messages when executing 'xgettext'.
		 *  \param _txt : text provided by 'xgettext'
		 *  \param _withpot : indicates if a '*.pot' file has been generated.
		 *  \return a readable message formatted
		 */
		wxString xgettextWarn(const wxString& _txt, const bool& _withpot = false);

		/** \brief Verify integity file
		 *  \param _replacecar : true if the user agrees to delete characters
		 *  \return true if good
		 */
		bool integrity(const bool& _replacecar);
		/** \brief Refreshes '_file' by replacing 'old' text with the '_new'
		 *  \param _source :  file to update
		 *  \param _old : old text
		 *  \param _new : new text
		 *  \return true if good
		 */
		bool changeContents(wxString& _source, const wxString& _old, wxString& _new);
		/** \brief Merge new '_new' file with '_old' file and update '_old' file
		 *  \param _oldpo : old file *.po with translation
		 *  \param _newpot : new file *.pot with new strings finded
		 *  \return number new strings or  wxNOT_FOUND  if error
		 */
		wxInt32 mergeFiles(wxString& _oldpo, wxString& _newpot);

		/** \brief Ending extracting
		 *  \return	true if good
		 */
		bool endextract();

		/** \brief Display messages of ending listing or extracting
		 *   \brief and the ending date
		 *  \param _mode : 'E' for extracting, 'L' for listing
		 *  \return true if good
		 */
		bool endaction(const wxChar& _mode);

// -----------------------------------------------------------------------------
		/** \brief Returns the number of occurrences of _str in the _txt
		 *  \param _souce :  text code
		 *  \param _str : to find
		 *  \return the number of occurrences
		 */
		wxUint32 freqStr(const wxString& _souce, const wxString& _str);

// -----------------------------------------------------------------------------
	   /** \brief Recursively deletes non-empty directories
		 *	 @param _rmDir : directory name
		 *   @return true if it's
		 */
		bool recursRmDir(wxString _rmDir);
		/** \brief Number of files deleted in temporary directory
		 */
		wxUint32 m_nbFilesdeleted = 0;
// -----------------------------------------------------------------------------

// ----------------------------- variables -------------------------------------
	protected:

		/** \brief platforms
		 */
		bool m_Win = false
		/** \brief platforms Linux
		 */
			 ,m_Linux = false
		/** \brief platforms Mac
		 */
			 ,m_Mac = false
			 ;

		/** \brief array of all projects
	     */
		ProjectsArray * m_pProjects = nullptr;
		/** \brief number projects
	     */
		wxUint32 m_nbProjects = 0;

	// active project  and leader project
		/**  \brief
		 */
		cbProject * m_pProjectleader = nullptr;
		/**  \brief
		 */
		wxUint32
			/**  \brief
			 */
            m_indexPrjLeader = 0, m_indexFirst = 0, m_indexLast = 0
			/**  \brief
			 */
            , m_nbTargets = 0
			/**  \brief
			 */
			 ,m_nbFileEligible = 0
			/**  \brief
			 */
			 ,m_nbOldStringsPo = 0
			/**  \brief
			 */
			,m_nbStringsWS = 0, m_nbPrjextractWS = 0
			/**  \brief
			 */
			,m_nbNewstrings = 0
			/**  \brief
			 */
			,m_nbFilesprj = 0, m_Bignumber = 100
		;
		wxInt32
            /**  \brief
			 */
             m_nbListStrings = 0, m_nbExtractStrings = 0
        ;
		/**  \brief
		 */
		wxInt32 m_nbStrPrj = 0, m_nbStrWS = 0;
		/**  \brief
		 */
		bool m_VerifyCommandsOnly = true
			/**  \brief
			 */
			,m_Init = false, m_Stop = false, m_Cancel = false, m_AllOk = false
			/**  \brief
			 */
			,m_Workspace = false, m_goodListing  = false
			/**  \brief
			 */
			,m_Strcreated = false

			/**  \brief executables are here ?
			 */
			,m_Pexeishere = false
			,m_Gexeishere  = false
			,m_Mexeishere  = false
			/**  \brief
			 */
			,m_Withstrings = false
			,m_Formatxgettext = true
			,m_Strwitherror = false
		;

	// the tables
		wxArrayString
			/** \brief temporary files created
			 */
			m_FileStrcreated
			/** \brief  projects names 'Wx'
			 */
			,m_NameprjWS
			/** \brief project files with text i18n
			 */
			,m_FileswithI18n
			/** \brief projects into workspace withfiles text i18n
			 */
			,m_PrjwithI18nWS
			/** \brief workspace with text i18n
			 */
			,m_FileswithI18nWS
			/** \brief targets of 'ProjectFile
			 */
			,m_Targetsfile
			/** \brief list files and extracted strings for editing
			 */
			,m_Fileswithstrings
		;
	// path file ...
		wxString
			/**  \brief  text
			 */
			m_Thename  = "CollectorPlugin", m_Theend = wxEmptyString
			/**  \brief
			 */
			,m_Nameconf =  wxEmptyString
			/**  \brief
			 */
			,m_Pexe = "Poedit", m_Pathpexe = wxEmptyString
			/**  \brief
			 */
			,m_Gexe = "xgettext", m_Pathgexe = wxEmptyString
			,m_PathIts = wxEmptyString, m_PathLocalIts = wxEmptyString
			,m_PathRulesIts = wxEmptyString
			,m_Mexe ="msgmerge", m_Pathmexe = wxEmptyString
			/**  \brief
			 */
			,m_Rexe = "wxrc", m_Pathrexe = wxEmptyString
			,EXT_WXS = "wxs"
			/**  \brief
			 */
			,m_Exttmp = wxEmptyString, m_Warnpot =  wxEmptyString
			,m_Wxpath = wxEmptyString, m_Qtpath = wxEmptyString
			/**  \brief project directory absolute path
			 */
			,m_Dirproject = wxEmptyString
			/**  \brief project name,
			 */
			,m_Nameproject = wxEmptyString
			/** \brief Contains active target name
			 */
			,m_Nameactivetarget = wxEmptyString
			/**  \brief project leader directory absolute path
			 */
			,m_NameprojectLeader = wxEmptyString
			,m_DirprojectLeader = wxEmptyString
			/**  \brief for file name *.pot
			 */
			 // 'strSlash' not define here !
			,m_Dirlocale =  "trlocale" + wxString(wxFILE_SEP_PATH)
			,m_DirlocaleLeader = wxEmptyString
			,m_Namepot 	= wxEmptyString, m_Shortnamepot = wxEmptyString
			,m_Dirpot	= wxEmptyString, m_Namepo = wxEmptyString
			,m_nameFileTemp = wxEmptyString, m_Inputfile  = wxEmptyString
			,m_Temp = m_Dirlocale + "temp" + wxString(wxFILE_SEP_PATH)

			/**  \brief extract keyword default
			 */
			,m_KeyWx	= "_"
			,m_KeyQt	= "tr"
			,m_Keyword 	= m_KeyWx

			/**  \brief strings begin and end
			 */
			,m_Datebegin = wxEmptyString,  m_Dateend = wxEmptyString
			,m_Eol
		;
		/** \brief
		 */
		bool m_isWxProject = false
			,m_isQtProject = false
			;

// ---------------------------- utility ----------------------------------------

	protected :

		/** \brief Give a date
		 *  @return date
		 */
		wxString date();
		/** \brief Startup duration
		 *  @param _namefunction : used function name
         */
		void beginDuration(const wxString & _namefunction = wxEmptyString);
		/** \brief End duration
		 *  @param _namefunction : used function name
         */
		void endDuration(const wxString & _namefunction = wxEmptyString);
		/** \brief Give a duration
		 *  @return duration 'mS'
		 */
		wxString duration();

		/** \brief Startup time
		 *  @return clock()/1000
         */
		wxUint64 beginTime();
		/** \brief End time
		 *  @return  duration from 'beginTime()'
         */
		wxUint64 endTime();
		/** \brief Give a duration
		 *  @return duration 'mS'
		 */
		wxUint64 elapsedTime(const wxUint64 _begin);
        /** \brief Give a string duration
		 *  @return duration 'minutes', 'seconds'
		 */
		wxString elapsedTimeStr(const wxUint64 _begin);

		/** \brief Find the path of an 'exe' into 'default.conf'
		 *  \param _txt : the executable to be found
		 *  \param _block :  the block to be found
		 *  \param _buffer : source text
		 *  \return the finded block of text
		*/
		wxString findblockExe(const wxString _txt, wxString _block, wxString _buffer);

		/** \brief  Look for the '_txt' in the configuration file of 'CB'.
		 *  \param _txt : text to find
		 *  \return the finded text
		*/
		wxString findPathfromconf(const wxString _txt);

		/** \brief Display a file to editor
		 *  \param _file : watch file
		 *  \return true if opened
		 */
		bool openedit(const wxString _file);

		/** \brief Create a directory
		 *	@param _dir : dirrectory name
		 *  @return true if it's correct
		 */
		bool createDir (const wxString& _dir) ;

		/**  \brief Read contents file
		 *	 @param _filename : file name
		 *   @return contents file
		 */
		wxString readFileContents(const wxString& _filename) ;

		/** \brief Save an array to disk and open the file in 'Editor'
         *  @param _strarray : array name to save
         *	@param _mode : 'L' => 'List...', 'E' => 'Extract...'
         *	@return true if all right
         */
		bool saveArray (const wxArrayString& _strarray, const wxChar _mode)  ;

		/** \brief give a string of table 'title' for debug
		 *	@param _title : title name
		 *	@param _strarray : array name
		 *  @return one string of array
		 */
		wxString allStrTable(const wxString& _title, const wxArrayString& _strarray);

	protected:

		/** \brief name of plugin
		 */
		wxString m_namePlugin = wxEmptyString;

		/**  \brief manager
		 */
		Manager * m_pM = nullptr;
		/**  \brief manager
		 */
		ProjectManager * m_pMprj = nullptr;
		/**  \brief macros manager
		 */
		MacrosManager  * m_pMam = nullptr;
		/**  \brief editor manager
		 */
		EditorManager  * m_pMed = nullptr;
		/**  \brief config manager
		 */
		ConfigManager  * m_pMconf = nullptr;

		/** \brief the current project
		 */
		cbProject * m_pProject = nullptr;

		/**  \brief all messages to logs
		 */
		wxString Mes = wxEmptyString;

		/** \brief log page index
		 */
		wxUint32	m_LogPageIndex = 0;

		/**  \brief calculate duration  mS
		 */
		clock_t m_start, m_begin, m_end;
};

#endif // _CBPRE_H_
