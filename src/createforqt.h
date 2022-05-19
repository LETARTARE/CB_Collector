/***************************************************************
 * Name:      createforqt.h
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2020-05-10
 * Modified:  2022-04-05
 * Copyright: LETARTARE
 * License:   GPL
 **************************************************************/
#ifndef _CREATEFORQT_H
#define _CREATEFORQT_H
//------------------------------------------------------------------------------
#include "pre.h"
//------------------------------------------------------------------------------
/**	\class CreateForQt
 *	@brief The class is used to build 'Qt'  I18n files
 *
 */
class CreateForQt  : public Pre
{
	public :

		/** \brief Constructor
         *  @param _namePlugin : The plugin name
         *  @param _logIndex : The active log
         */
		CreateForQt(const wxString & _namePlugin, const int _logIndex);

		/** \brief Destructor
         */
		virtual ~CreateForQt();

		/** \brief Detects if the current target uses 'Qt' libraries,
		 *  \note  Overloading a pure virtual method
         *  @param _pProject : the active project
         *  @param _report : display report if true.
         *  @return true : if used
         */
		bool detectLibProject(cbProject * _pProject, bool _report = false) override;

		/** \brief	Delete all temporary file
		 *  @return number deleted files
         */
		wxUint32 Cleantemp();

	private:
		/** \brief specific initialisation by inherited type
		 *  \note  Overloading a pure virtual method
		 *  \param _type : in ["_WX", "_QT", "_WX","_WX_QT"]
		 *  @return true if good
		 */
		bool initTailored(const wxString _type  = wxEmptyString ) override ;

		/** \brief Find container path
		 *	@param _pContainer : 'cbProject*' or 'ProjectBuildTarget*'
		 * 	 	both inherited of 'CompileTargetBase'
		 *  @return path
		 */
		wxString pathlibQt(CompileTargetBase * _pContainer) ;

		/** \brief Is whether the 'Qt' executables exist
		 *	@param _pBuildTarget : 'CompileTargetBase*' of used target
		 *  @return true if they exist all
		 */
		bool findTargetQtexe(CompileTargetBase * _pBuildTarget) ;

		/** \brief	Search if the file has translatable strings
		 *  \note  Overloading a pure virtual method
		 *  \param _file : the file to check
		 *  \param _absolute : path file is absolute
		 *  \return true if elegible file
         */
	//	bool isReadableFile(const wxString& _file,
	//						const bool _absolute = false) override;
		wxInt32 isReadableFile(const wxString& _file,
							const bool _absolute = false) override;
		/** \brief List the strings into _shortfile with 'xgettext'
		 *  \note  Overloading a pure virtual method
		 *  \param _shortfile : the file with strings or not
		 *  \return true if readable
         */
		wxInt32 listStringsCode(const wxString& _shortfile) override;
		/** \brief List the strings into _shortfile with 'wxrc'
		 *  \note  Overloading a pure virtual method
		 *  \return number finded strings  or -1 if error
         */
		wxInt32 listStringsRes(wxString& /*_shortfile*/) override { return 0;	}
		/** \brief List the strings into _shortfile with 'xgettext'
		 *  \note  Overloading a pure virtual method
		 *  \return number finded strings  or -1 if error
         */
		wxInt32 listStringsXml(const wxString& /*_shortfile*/) override { return 0;	}

		/** \brief NOT USE HERE but mandatory (virtual pure)
		 *  \note  Overloading a pure virtual method
		 *  \param _file :
		 *  \return the new file name
         */
		wxString expandName(wxString _file, wxInt32 /*_indexfree = 0*/) override {return _file;}

		/** \brief Generate the name of the complement file
		 *	@param _file : file name creator
		 *  @return file name complement
		 */
		wxString nameCreated(const wxString& _file);

		/** \brief Search 'Qt' libraries in project or target
         *  @param _pContainer: 'cbProject * Project' or 'ProjectBuildTarget * buildtarget'
         *		both inherited of 'CompileTargetBase'
		 *  @return true if finded
         */
		bool hasLibQt(CompileTargetBase * _pContainer) ;

		/** \brief Create the file '*.pot' containing all translatable string(s)
		 *  \note  Overloading a pure virtual method
		 *  \param  _prjfirst : true if it's the first file
		 *  \return true if good
		 */
		bool createPot(bool _prjfirst) override ;

	private:

		wxString
		/** \brief Contains all defines 'Qt' for executables
		 */
			m_DefinesQt,
		/** Contains all path include 'Qt' for executables
		 */
			m_IncPathQt,
		/** \brief Contains file name
		 */
			m_filename
			;

		/** \brief executables are here ?
		 */
		bool
			/** \brief 'lupdate'
			 */
			m_LUexeishere = false
			/** \brief 'lrelease'
			 */
			,m_LRexeishere  = false
			/** \brief 'llinguist'
			 */
			,m_LLexeishere  = false
		    ;
	wxString
		/** \brief executable name files 'Qt' : 'lupdate'
		 */
			m_LUexe = wxEmptyString
		/** \brief executable name files 'Qt' : 'lrelease'
		 */
			,m_LRexe = wxEmptyString

		/** \brief executable name files 'Qt' : 'linguist'
		 */
			,m_LLexe = wxEmptyString

		/** \brief files prefix for 'moc'
		 */
			,m_Moc = "moc"
        /** \brief files prefix for 'uic'
		 */
			, m_UI = "ui"
		/** \brief files prefix for 'rcc'
		 */
			,m_Qrc = "qrc"
		/** \brief files extension for 'lrelease'
		 */
			,m_Lm = "qm"
		;
};

#endif // _CREATEFORQT_H
