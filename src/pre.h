/*************************************************************
 * Name:      pre.h
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2020-05-10
 * Modified:  2022-12-16
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
		bool hasNoproject();
		/** \brief For globals boolan
		 *  \return true if 'm_Init = true' and 'm_Stop = false' and 'm_Cancel = false'
		 */
        bool isAllRight();

        /** \brief close all extra files to editor '*.list', '*.extr', '*.po'
		 *  \return true if closed
		 */
        bool closeAllExtraFiles();

		/** \brief Indicates if target is command only
		 *	@param _nametarget : target name
		 *  @return true if it's command target
		 */
		bool isCommandTarget(const wxString& _nametarget) ;

	protected:

		/** \brief Indicates if target is command only or a virtual target
		 *	@param _nametarget : target name
		 *  @return true if it's command target or virtual target
		 */
        bool isCommandOrVirtualTarget(const wxString& _nametarget) ;
		/** \brief Indicates if target is command only
		 *	@param _pTarget : target pointer
		 *  @return true if it's command target
		 */
		bool isCommandTarget(const ProjectBuildTarget * _pTarget) ;
		/** \brief Indicates if target is command only or a virtual target
		 *	@param _pTarget : target pointer
		 *  @return true if it's command target or virtual target
		 */
        bool isCommandOrVirtualTarget(const ProjectBuildTarget * _pTarget) ;

		/**  \brief Give compileable targets list for project or virtual target
		 *   @return a table : of compileable target name
		 */
		wxArrayString compileableProjectTargets();

        /** \brief Indicates if target is virtual
		 *	@param _nametarget : target name
		 *  @return true if it's virtual target
		 */
		bool isVirtualTarget(const wxString& _nametarget) ;

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
		virtual bool creatingPot(bool _prjfirst) = 0;

// -----------------------------------------------------------------------------

//	protected:
    public:
		/** \brief Global initialization
		 *  \param _type : in ["_WX", "_QT", "_WX","_WX_QT"]
		 *  \return	true if good
		 */
		bool Init(const wxString _type = wxEmptyString );

	protected :
		/** \brief Basic initialization
		 *  \param _type : in ["_WX", "_QT", "_WX","_WX_QT"]
		 *  \return	true if good
		 */
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
		 *  \return  return project index  ( error < 0 )
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
		/**  \brief Launch a command line mode synchronous
		 *	 @param _command : command line
		 *	 @param _prepend_error : prepend error to return
		 *   @return empty string if it's correct else error message
		 */
		wxString executeAndGetOutputAndError(const wxString& _command,
											const bool& _prepend_error = true) ;
		/**  \brief Launch a command line mode asynchronous
		 *	 @param _command : command line
		 *	 @param _prepend_error : prepend error to return
		 *   @return empty string if it's correct else error message
		 */
        wxString executeAsyncAndGetOutput(const wxString& _command,
                                    const bool& _prepend_error = true) ;

        /** \brief Launch  an external executable tool in asynchronous mode
         *	\param _toolexe : executable name
		 *  \return error code, 0 if good
		 */
        wxInt32 LaunchExternalTool(const wxString& _toolexe);
		/** \brief From a text, extract the different strings to be translated
		 *  \param _txt : one line of text
		 *  \return a string formatted in successive lines
		 */
		wxString xgettextOut(wxString& _txt);

		/** \brief With 'xgettextOut(wxString& _txt)' filters strings
		 *  \param _txt : "okcode", "okres", "okxml"
		 */
		void listFiltered(wxString _txt = "");

		/** \brief With 'xgettextOut(wxString& _txt)' banned strings
		 *  \param _txt : "okcode", "okres", "okxml"
		 */
		void listBanned(wxString _txt = "");
		void listBeginBanned(wxString _txt = "");
		/** \brief With 'xgettextOut(wxString& _txt)' banned strings
		 *  \brief separates a line into elements ( separated by 'Space' )
		 *  \param _txt : "okcode", "okres", "okxml"
		 */
		bool filterBeginBanned(wxString _txt);

		/** \brief Retrieves error or warning messages when executing 'xgettext'.
		 *  \param _txt : text provided by 'xgettext'
		 *  \param _withpot : indicates if a '*.pot' file has been generated.
		 *  \return a readable message formatted
		 */
		wxString xgettextWarn(wxString & _txt, const bool& _withpot = false);

		/** \brief Verify integrity file
		 *  \param _replacecar : true if the user agrees to delete characters
		 *  \return true if good
		 */
		bool integrity(const bool& _replacecar);
		/** \brief Delete '\\r' in translate strings
		 *  \param _source : translate strings
		 *  \return number deleted characters
		 */
		wxUint32 deleteCr(wxString & _source);

		/** \brief Refreshes '_file' by replacing 'old' text with the '_new'
		 *  \param _source :  file to update
		 *  \param _old : old text
		 *  \param _new : new text
		 *  \return true if good
		 */
		bool changeContents(wxString& _source, const wxString& _old, wxString& _new);

		/** \brief Find and Unifies duplicate translations
		 *  \brief  : a single string for the several same strings
		 *  \param _inputdup : input file with duplicates messages
		 *  \param _outputpo : new file *.po with unified duplicates
		 *  \return error messages
		 */
		wxString noDuplicateMsgid(wxString& _inputdup, wxString& _outputpo);

		/** \brief Merge new '_new' file with '_old' file and update '_old' file
		 *  \param _old : old file *.po with translation to update
		 *  \param _new : new file *.pot with unified strings
		 *  \param _nbdots 	: number new strings ??? TO VERIFY
		 *  \return result messages
		 */
		wxString mergeFiles(wxString _old, wxString _new, wxInt32 &  _nbdots);

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
		/** \brief Returns the number of occurrences of '_str' in the '_txt'
		 *  \param _souce :  text code
		 *  \param _str : to find
		 *  \return the number of occurrences
		 */
		wxUint32 freqStr(const wxString& _souce, const wxString& _str);

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
        /** \brief for localization
		 */
        wxLocale m_locale;
        wxString m_lang = "";

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
			/**  \brief various index
			 */
            m_indexPrjLeader = 0, m_indexFirstFile = 0, m_indexLastFile = 0
			/**  \brief
			 */
            , m_nbTargets = 0
			/**  \brief
			 */
			 ,m_nbFileEligible = 0 , m_nbFileEligibleWS = 0
			/**  \brief
			 */
			 ,m_nbOldStringsPo = 0, m_nbXmlWithlessString = 0
			/**  \brief
			 */
			,m_nbStringsWS = 0, m_nbPrjextractWS = 0
			/**  \brief
			 */
			,m_nbFilteredRes = 0, m_nbFilteredXml = 0
			/**  \brief
			 */
			,m_nbFilesprj = 0, m_nbFilesWS = 0, m_Bignumber = 100
			/** \brief
			 */
			,m_sizeFilePo = 0
		;
		wxInt32
            /** \brief
			 */
            m_nbListStrings = 0, m_nbExtractStrings = 0
            /** \brief
             */
            , m_sizeHeaderpo = 0

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
			,m_Uexeishere  = false
			,m_Rexeishere  = false
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
			/** \brief project files with text i18n workspace or not
			 */
			,m_FileswithI18n
			/** \brief projects into workspace withfiles text i18n
			 */
			,m_PrjwithI18nWS
			/** \brief targets of 'ProjectFile
			 */
			,m_Targetsfile
			/** \brief list files and extracted strings for editing
			 */
			,m_Fileswithstrings
			/** \brief futur *.pot filtered
			 */
			,m_FilesCreatingPOT
			,m_SaveCreatingPOT
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
			,m_Pexe = "Poedit", m_PathPexe = wxEmptyString
			/**  \brief
			 */
			,m_Gexe = "xgettext", m_PathGexe = wxEmptyString
			,m_PathIts = wxEmptyString, m_PathLocalIts = wxEmptyString
			,m_PathRulesIts = wxEmptyString
			,m_Mexe ="msgmerge", m_PathMexe = wxEmptyString
			,m_Uexe ="msguniq", m_PathUexe = wxEmptyString
			/**  \brief
			 */
			,m_Rexe = "wxrc", m_PathRexe = wxEmptyString
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
			/**  \brief for file name '*.pot'
			 */
			 // 'strSlash' not define here !
			,m_Dirlocale =  "trlocale" + wxString(wxFILE_SEP_PATH)
			,m_DirlocaleLeader
			,m_Namepot, m_Shortnamepot
			,m_Namepo, m_Shortnamepo
			,m_Dirpot
			,m_Namedup, m_Shortnamedup
			,m_Nameuni, m_Shortnameuni
			,m_nameFileTemp, m_Inputfile
			,m_Temp = m_Dirlocale + "temp" + wxString(wxFILE_SEP_PATH)

			/**  \brief extract keyword default
			 */
			,m_KeyWx	= "_"
			,m_KeyQt	= "tr"
			,m_Keyword 	= m_KeyWx
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
		/** \brief Give a duration (milliseconds)
		 *  @param  _last : preceding time
		 *  @return duration 'mS' : if (_last == 0 => actual time
		 */
		wxUint64 elapsed(const wxUint64 _last);
        /** \brief Give a string duration
		 *  @return duration 'minutes', 'seconds', 'mS'
		 */
		wxString dtimeStr(const wxUint64 _time);

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
		/** \brief close a file to editor
		 *  \param _file : watch file
		 *  \return true if closed
		 */
		bool closedit(const wxString _file);

		/** \brief Copy a file to an another directory
		 *  \return	true if good
		 */
		bool copyFileToDir(const wxString& _namefile);

		/** \brief Create a directory
		 *	@param _dir : dirrectory name
		 *  @return true if it's correct
		 */
		bool createDir (const wxString& _dir) ;

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
		bool saveArrayToDisk (const wxArrayString& _strarray, const wxChar _mode)  ;
        /** \brief Add an array to an other
         *  @param _receive : array to receiving
         *	@param _array : array name to adding
         *  \return cells numbers of _receive
         */
		wxUint32 addArrays(wxArrayString& _receive, wxArrayString& _array);

		/** \brief give a string of table 'title' for debug
		 *	@param _title : title name
		 *	@param _strarray : array name
		 *  @return one string of array
		 */
		wxString allStrTable(const wxString& _title, const wxArrayString& _strarray);

		/** \brief Show an array to log
		 *	@param _title : title name
		 *	@param _strarray : array name
		 * \return true if good
		 */
		bool showArray(const wxArrayString& _strarray, const wxString& _title = "");

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
		clock_t m_tL0 = 0, m_tE0 = 0, m_tI = 0, m_dtL = 0 , m_dtE = 0;


#include "marksbanned.h"	// all banned strings

		/** \brief  tags finded into '*.wxs' or '*.xrc'
		 *  \note  separator : ' '
		 */
/*
		wxString
			m_tagsRes = "<label <item <help <caption <note <message "
						"<title <hint <tooltip "
*/
		/** \brief  tags finded into '*.xml'
		 */
/*
			,m_tagsXml = "title description name summary developer_name "
						 "checkMessage caption category type error <comment <p <li "
			/// "thanksto "
*/
        /** \brief  banned strings finded into all
         */
/*
			,m_bannedMarks =
				"- + ... * ? m_  ll xx aaa eee nnn xxx vvv www "
				"-fpic -fno-rtti c,cpp,cxx,cc,c++ Cygwin GCC "
				"GNU Linux Allman (ANSI) Java K&&R Stroustrup VTK "
				"Horstmann Ratliff Whitesmith 1TBS Google Mozilla "
				"Pico Lisp GDB CDB AT&T Intel TCP UDP 115200 192.168.0.1 "
				"wizard.xrc atxxxxx mxxxx avrxxxx wxXXXX "
				"include lib obj cflags ldflags func addr "
				"CR LF AUTO Windows >Mac (CR) ASCII (ISO-8859-1) ID: "
*/
		 /** \brief  begin banned strings finded into all files for 'C::B'
         */
/*
			,m_beginBannedMarks =
				"- + ... * ? m_  ll xx aaa eee nnn xxx vvv www \"\" "
				"-fpic -fno-rtti c,cpp,cxx,cc,c++ Cygwin GDB CDB AT&T Intel "
				"/dev/ttyS0 192.168.0.1 YAML LaTeX POD Verb "
				"GCC ASM SQL Smalltalk Ruby Python XBase Powershell Postscript Perl "
				"Pascal Objective C NSIS Matlab Markdown Mak Lua Lisp JavaScript "
				"Java Intel HEX HTML/PHP/ASP/JS Haskell Fortran Fortran77 D CUDA "
				"CSS CoffeeScript CMake nVidia cg Caml BibTeX Bash Autotools "
				"AngelScript Ada Motorola 68k TreeCtrl1 "
				"-fno-omit-frame-pointer UUID C/C++ ID: C++ 0x0 LF CR "
				"M3000 AVR ARM cortex- Motorola S-Record Squirrel "
				"AT90S2313 AT90S232 AT90S2333 AT90S2343 AT90S4414 AT90S4433 "
				"AT90S4434 AT90S8515 AT90C8534 AT90s8535 AT90usb82 AT90usb162 "
				"AT90PWM1 AT90PWM2 AT90PWM2B AT90PWM3 AT90PWM3b AT90PWM81 AT90K "
				"AT90PWM161 AT90PWM216 AT90PWM319 AT90CAN32 AT90CAN64 AT90SCR100 "
				"AT90USB646 AT90USB647 "
				"ATA5831 ATA8210 AT90CAN128 AT90USB1286 AT90USB1287 "
				"ATA6617c ATA664251 AT90S2323 "
				"ATtiny Attiny ATtiny13 ATtiny13a ATtiny2313 ATtiny22 ATtiny26 "
				"ATtiny24 ATtiny24a ATtiny25 ATtiny261 ATtiny4313 ATtiny43u "
				"ATtiny44 ATtiny44a ATtiny441 ATtiny45 ATtiny461 ATtiny461a "
				"ATtiny48 ATtiny828 ATtiny84 ATtiny84a ATtiny841 ATtiny85 ATtiny861 "
				"ATtiny861a ATtiny87 ATtiny88 "
				"Attiny202 Attiny204 Attiny212 Attiny214 Attiny402 Attiny404 "
				"Attiny406 Attiny412 Attiny414 Attiny416 Attiny417 Attiny804 "
				"Attiny806 Attiny807 Attiny814 Attiny816 Attiny817 "
				"Attiny1604 Attiny1606 Attiny1607 Attiny1614 Attiny1616 Attiny1617 "
				"ATtiny4 ATtiny5 ATtiny9 ATtiny10 ATtiny11 ATtiny12 ATtiny15 ATtiny20 "
				"ATtiny28 ATtiny40 Attiny3214 Attiny3216 Attiny3217 attiny167 attiny1634 "
				"atmega8u2 atmega16u2 atmega32u2 "
				"ATmega16hvb ATmega16hvbrevb "
				"ATmega16 ATmega16a ATmega16hva ATmega16hva2 ATmega16vbrevb "
				"ATmega16m1 ATmega16u4 ATmega161 ATmega162 ATmega163 ATmega164A "
				"ATmega164P ATmega164PA ATmega165 ATmega165A ATmega165P ATmega165PA "
				"ATmega168 ATmega168A ATmega168P ATmega168PA ATmega168PB "
				"ATmega169 ATmega169P ATmega169PA ATmega32 ATmega32a ATmega32c1 "
				"ATmega32hvb ATmega32hvbrevb ATmega32m1 ATmega32u4 ATmega32u6 "
				"ATmega48 ATmega48a ATmega48p ATmega48pa ATmega48pb ATmega8 "
				"ATmega8a ATmega8hva ATmega88 ATmega88a ATmega88pa ATmega88pb "
				"ATmega128 ATmega128a ATmega128RFA1 ATmega1280 ATmega1281 ATmega1284 "
				"ATmega1284P ATmega1284RFR2 "
				"ATmega256RFR2 ATmega2560 ATmega2561 ATmega2564RFR2 "
				"ATmega323 ATmega323A ATmega324P ATmega324PA ATmega325 ATmega325A "
				"ATmega325P ATmega325PA ATmega328 ATmega328P ATmega328PB ATmega329 "
				"ATmega329A ATmega329P ATmega329PA ATmega3250 ATmega3250A ATmega3250P "
				"ATmega3250PA ATmega3290 ATmega3290P ATmega3290PA ATmega406 ATmega64 "
				"ATmega64A ATmega64C1 ATmega64HVE ATmega64m1 ATmega64rf2 ATmega640 "
				"ATmega644 ATmega644A ATmega644P ATmega644PA ATmega644RF2 ATmega645 "
				"ATmega645A ATmega645P ATmega649 ATmega649A ATmega649P ATmega6450 "
				"ATmega6450A ATmega6450P ATmega6490 ATmega6490A ATmega6490P "
				"Atmega808 Atmega809 Atmega1608 Atmega1609 Atmega4808 Atmega4809 "
				"ATmega8515 ATmega8535 ATmega103 Atmega3208 Atmega3209 "
				"AT86RF401 ATA5272 ATA6616C AT43USB355 AT43USB320 AT76C711 "
				"ATA6285 ATA6286 ATA6289 ATA6612c ATA5795 ATA5790 ATA5790N "
				"ATA5791 ATA6613C ATA6614q ATA5782 ATA5702M322 ata5505 "
				"AVRXmega ATXmega8E5 ATXmega16A4 ATXmega16A4U ATXmega16C4 ATXmega16D4 "
				"ATXmega16E5 ATXmega32A4U ATXmega32C3 ATXmega32C4 ATXmega32D3 ATXmega32D4 "
				"ATXmega32E5 ATXmega32A4 ATXmega64A3 ATXmega64A3U ATXmega64B1 ATXmega64B3 "
				"ATXmega64C3 ATXmega64D3 ATXmega64D4 ATXmega64A1 ATXmega64A1U ATXmega128A3 "
				"ATXmega128A3U ATXmega128B1 ATXmega128b3 ATXmega128C3 ATXmega128D3 "
				"ATXmega128D4 ATXmega192A3 ATXmega192A3U ATXmega256A3BU ATXmega256A3U "
				"ATXmega256C3 ATXmega256D3 ATXmega384C3 ATXmega384D3 ATXmega128A1 "
				"ATXmega128A1U ATXmega128A4U ATXmega192C3 ATXmega192D3 ATXmega256A3 "
				"ATXmega256A3B "
				"AVR64DA28 AVR64DA32 AVR64DA48 AVR64DA64 AVR64DB28 AVR64DB32 AVR64DB64 "
				"AVR32DA28 AVR32DA32 AVR32DA48 AVR32DB32 AVR32DB48 AVR128DA28 AVR128DA32 "
				"AVR128DA48 AVR128DA64 AVR128DB28 AVR128DB32 AVR128DB48 AVR128DB64 AVR64DB48 "
				"abcmini alf arduino atisp avr109 avr910 avr911 avrisp avrisp2 avrispmkII "
				"avrispv2 avrispmkII avrispv2 bascom blaster bsd buspirate butterfly "
				"c2n232i dapa dasa dasa3 dragon_dw dragon_hvsp dragon_isp dragon_jtag "
				"dragon_pdi dragon_pp dt006 ere-isp-avr frank-stk200 futurlec jtag1 "
				"jtag1slow jtag2 jtag2avr32 jtag2dw jtag2fast jtag2isp jtag2pdi jtag2slow "
				"jtagmkI jtagmkII tagmkII_avr32 mib510 pavr picoweb pony-stk200 ponyser "
				"siprog sp12 stk200 stk500 stk500hvsp stk500pp stk500v1 stk500v2 "
				"stk600 stk600hvsp stk600pp usbasp usbtiny xil --mkI --mkII --dragon "
				"MSP430  MSP1  MSP2  MSP3  MSP4  MSP5  MSP6 E423 E425 E427 "
				"E4232 E4242 E4252 E4272 W423 W425 W427 G4250 G4260 G4270 "
				"G437 G438 G439 G4616 G4617 G4618 G4619 CC430 "
				"Cortex-A15 Cortex-A9 Cortex-A8 Cortex-A7 Cortex-A5 Cortex-R7 Cortex-R5 "
				"Cortex-R4 Cortex-M4 Cortex-M3 Cortex-M1 Cortex-M0+ Cortex-M0 "
				"ARM946E-S ARM966E-S ARM968E-S ARM926EJ-S ARM940T ARM920T ARM922T "
				"ARM9TDMI ARM720T ARM7TDMI ARM7TDMI-S ARM9E ARM7EJ-S "
				"XScale CPU Mode Arm Thumb "
				"-mno-apcs-frame -mno-apcs-float -mno-apcs-frame -mno-apcs-reentrant "
				"-mno-apcs-stack-check -msched-prolog -mno-thumb-interwork -mno-long-calls "
				"-mno-single-pic-base -mpic-register=R9 -mpic-register=R10 -mtpcs-frame "
				"-mno-cirrus-fix-invalid-insns  -mcirrus-fix-invalid-insns -mtpcs-leaf-frame "
				"-mcaller-super-interworking -mcallee-super-interworking "
				"-mabi=apcs-gnu -mabi=atpcs -mabi=aapcs -mabi=aapcs-linux -mabi=iwmmxt "
				"-mapcs-stack-check -mapcs-reentrant -mapcs-float "
				"-mwords-little-endian -msoft-float -mhard-float -mfloat-abi=softfp "
				"-mlong-calls -mnop-fun-dllimport -mpoke-function-name "
				"-fpic -ffunction-sections -funwind-tables "
				"-fstack-protector -no-canonical-prefixes -fno-exceptions -fno-rtti "
				"-fno-strict-aliasing Armeabi-v7a "
				"-mabi -msched -mapcs -mno -msoft -mhard -mfloat -mfpe -mlong "
				"-mpoke -mpic -mcirrus -mtpcs -mcaller -mtpcs -mcaller "
				"at91sam7s32;at91sam7s64;at91sam7s128;at91sam7s256 "
				"lpc2132;lpc2134;lpc2136;lpc2138 %02hd/%02hd/%d %02hd:%02hd:%02hd "
				"Intel i386 i486 Pentium (MMX) PRO "
				"AMD K6 K6-2 K6-3 K6-6 K8 Athlon Thunderbird Opteron Athlon64 Athlon-FX "
				"(MMX, 3DNow!) 3DNow!, SSE re-gf re-iar DWARF 2 ABI "
				"VFPv2 VFPv3 VFPv3_D16 VFPv3_D16_FP16 VFPv4 VFPv4_sp VFP9-S "
				"PowerPC  e300v1  e300c1  e300c2 e300c3 e300c4 e500v1 e500v2 e600 Zen 5565 gekko "
				"wxAdvanced wxAUI wxGL wxHTML wxMedia wxNet 'wxPropertyGrid wxQA "
				"wxRibbon wxRichText wxSTC wxWebView wxXML wxXRC wxWidgets "
				"EasyRun TC1796 TriBoard TC1130 TC1161 TC1162 TC1762 TC1766 TC1792 "
				"TC1796 TC1797 EasyKit TC1767 phyCORE TC1130 amd intel amd16 intel16 "
				"Intel Pentium 3 (MMX, SSE) 4 SSE, SSE2) Prescott  SSE2, SSE3) "
				"EM 1 2 HRule Blockquote Prechar "
				"CommentStream HereString HereCharacter CommentDockKeyword "
				"Stdout Stdin Stderr Nil "
				"TriCore TC1130 TC1161 TC1162 TC1762 TC1765 TC1766 TC1767 TC1775 "
				"TC1792 TC1796 TC1797 TC1920 "
				"RegSubst LonQuote Immeval LongQuote Cmdlet "
				"HereDelim HereQ Rawstring Preproc Preproc If def Page Ex Strikeout "
*/			;
		/** \brief  Initialized by 'initArrayBanned()'
		 */
		wxArrayString
		/** \brief  Tags for file resources
		 */
					m_ArrayTagsRes
		/** \brief  Tags for file xml
		 */
					,m_ArrayTagsXml
		/** \brief All banned strings
		 */
					,m_ArrayBanned
		/** \brief Only begin of banned for code, xml ('xgettext')
		 */
					,m_ArrayBeginBanned
		/** \brief All filtered strings for resources ('wxrc')
		 */
					,m_ArrayFiltered
					;
		/** \brief
		 */
		wxUint32 m_sizeArrayBanned,
				m_sizeArrayBeginBanned
				;

		/** \brief Valid 'poedit'
		 */
		bool m_withPoedit = false;

		/** \brief
		 */
		size_t m_nbHeader = 0;
};

#endif // _CBPRE_H_
