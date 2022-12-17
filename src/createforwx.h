/***************************************************************
 * Name:      createforwx.h
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2020-05-10
 * Modified:  2022-12-10
 * Copyright: LETARTARE
 * License:   GPL
 **************************************************************/
#ifndef _CREATEFORWX_H
#define _CREATEFORWX_H
//------------------------------------------------------------------------------
#include "pre.h"
//------------------------------------------------------------------------------
/**	\class CreateForWx
 *	@brief The class is used to build 'Wx I18n' files ...
 *
 */
class CreateForWx  : public Pre
{
	public :

		/** \brief Constructor
         *  @param _namePlugin : The plugin name
         *  @param _logIndex : The active log
         */
		CreateForWx(const wxString _namePlugin, const int _logIndex);

		/** \brief Destructor
         */
		virtual ~CreateForWx();

		/** \brief Detects if the current target uses 'Wx' libraries,
		 *  \note  Overloading a pure virtual method
         *  @param _pProject : the active project
         *  @param _report : display report if true.
         *  @return true : if used
         */
		bool detectLibProject(cbProject * _pProject, bool _report = true) override;

		/** \brief	Delete all temporary file
		 *  \return number deleted
         */
		wxUint32 Cleantemp();

	private:

		/** \brief Call a menu
		 *  \param _mItem : example => "&Tools\Poedit"
		 */
		wxInt32 menuCall(const wxString& _mItem) ;

		/** \brief Specific initialisation by inherited type
		 *  \note  Overloading a pure virtual method
		 *  \param _type : in ["_WX", "_QT", "_WX","_WX_QT"]
		 *  \return true if good
		 */
		bool initTailored(const wxString _type  = wxEmptyString ) override ;
		/** \brief Initialize various 'Wx' specific variables
		 */
		void initWx();
		/** \brief Initialize specific variables 'm_TabmarkersRes', 'm_TabmarkersXml'
		 */
		void initArrayBanned();

	   /** \brief Search all 'exe' paths
	    *  \return true if all finded
         */
		bool searchExe();

		/** \brief Search the 'Wx' path and feed 'm_Wxpath'
		 *  \param _pTarget : target*
		 *  \return  true if path finded
         */
		bool pathWx(ProjectBuildTarget * _pTarget);

		/** \brief	Tests readability for chain collection
		 *  \note  Overloading a pure virtual method
		 *  \param _file : the file to check
		 *  \param _absolute : path file is absolute
		 *  \return true if readable
		 */
		wxInt32 isReadableFile(const wxString& _file,
							const bool _absolute = false) override;

		/** \brief List the strings into _shortfile with 'xgettext'
		 *  \note  Overloading a pure virtual method
		 *  \param _shortfile : the file with strings or not
		 *  \return number finded strings  or -1 if error
		 */
		wxInt32  listStringsCode(const wxString& _shortfile) override;
		/** \brief List the strings into _shortfile with 'xgettext' and 'its'
		 *  \note  Overloading a pure virtual method
		 *  \param _shortfile : the file with strings or not
		 *  \return number finded strings  or -1 if error
		 */
		wxInt32  listStringsXml(const wxString& _shortfile) override;
		/** \brief Extract the label of *.xrc and create a temporary file  with string(s)
		 *  \note  Overloading a pure virtual method
		 *  \param _file : 'namefile '*.xrc'
		 *  \return number strings, if  < 0 =>  error
		 */
		wxInt32  listStringsRes(wxString& _file) override;
		/** \brief For the '*.xrc' file  return '*_xrc.str???'
		 *  \param _file : '*.xrc'
		 *  \param _indexfree : 'xxx' where xxx is index in 'm_FileStrCreated'
		 *  \return the new file name '*_xrc.str'
		 */
		wxString expandName(wxString _file, wxInt32 _indexfree = 0);
		/** \brief For the '*_xrc.strxxx' file  copy to '*.xrc'
		 *  \param _file : '*_xrc.strxxx' or '*_wks.strxxx'
		 *  \return true if correct
		 */
		bool copyRes(wxString _file);

		/** \brief  Display formatting for resouces wxrc and wks
		 *  \note  Overloading a pure virtual method
		 *  \param _txt : raw text  => _txt filtered or not
		 * \param _shortfile :  it's a name file just to didplay
		 *  \return false if _txt is filtered
		 */
		bool wxrcOut(wxString& _txt, wxString _shortfile = "");
		/** \brief Execute 'wxrc'
		 *  \param _shortfile :  it's a file for extract strings
		 *  \param _command : command line fot 'xgettext'
		 *  \param _prepend :  allows you to retrieve errors or messages at runtime.
		 *  \return if good : number chains to tranlate, else number advices
		 */
		wxInt32 RexewithError(const wxString& _shortfile, const wxString& _command,
							  const bool& _prepend = true);

		/** \brief Create the file '*.pot' containing all translatable string(s)
		 *  \note  Overloading a pure virtual method
		 *  \param  _prjfirst : true if it's the first file
		 *  \return true if good
		 */
		bool creatingPot(bool _prjfirst) override ;

    public:
		/** \brief Create the heder for file '*.pot'
		 *  \return number lins created
		 */
		wxUint16 iniHeaderPo();

};

#endif // _CREATEFORWX_H
