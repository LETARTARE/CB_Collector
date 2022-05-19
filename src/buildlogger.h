/***************************************************************
 * Name:      buildlogger.h
 * Purpose:    Buid a progress bar to 'Collector' log
 * Author:    LETARTARE
 * Created:   2022-04-30
 * Modified:  2022-05-04
 * Copyright: LETARTARE
 * License:   GPL
 **************************************************************/


#ifndef BUILDLOGGER_H
#define BUILDLOGGER_H
//==============================================================================
#include <sdk.h> 		// Code::Blocks SDK
#include <loggers.h>
#ifdef __linux__
    #include <wx/gauge.h>
    #include <wx/panel.h>
    #include <wx/sizer.h>
#endif
//==============================================================================
/** \brief  for 'BuildLogger::CreateControl(...)
 */
const int idBuildLog = wxNewId();

//====================== A descendant of 'TextCtrlLogger' ======================

/** \brief Create a 'Collector' log descendant of 'TextCtrlLogger'.
 *  \brief with a panel and a progress bar to 'Collector' log
 *  \brief
 */
class BuildLogger : public TextCtrlLogger
{
	private :
		/** \brief  A panel for 'wxGauge'
		 */
		wxPanel* m_pPanel = nullptr;
		/** \brief  A sizer for 'wxPanel'
		*/
		wxBoxSizer* m_pSizer = nullptr;

	public:
		/** \brief  A progress bar
		 */
		wxGauge* m_pProgress = nullptr;
		/** \brief
		 */
		static wxUint32 g_CurrentProgress, g_MaxProgress;
		/** \brief  Consructor
		 *  \param 	_fixedPitchFont : size font
		 */
		BuildLogger(bool _fixedPitchFont = false) ;
		/** \brief  Destructor
		 */
		virtual ~BuildLogger();

		/** \brief  Update values
		 */
		void UpdateSettings() override ;

		/** \brief  Create a 'wxPanel' and a 'wxSizer'
		 */
		wxWindow* CreateControl(wxWindow* _parent) override;

		/** \brief  Add a progress bar
		 */
		void AddBuildProgressBar();

	// not used here
		/** \brief delete a progress bar
		 */
	   // void RemoveBuildProgressBar() ;
};
//==============================================================================

#endif // BUILDLOGGER_H
