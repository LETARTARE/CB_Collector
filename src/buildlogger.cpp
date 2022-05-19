/***************************************************************
 * Name:      buildlogger.cpp
 * Purpose:   Buid a progress bar to 'Collector' log
 * Author:    LETARTARE
 * Created:   2022-04-30
 * Modified:  2022-05-04
 * Copyright: LETARTARE
 * License:   GPL
 **************************************************************/

#include "buildlogger.h"
/// ----------------------------------------------------------------------------
//  Variables globals statics
//
// Modified by:
//	1. Collector::OnAttach():1,
//	2. Collector::OnMenuListPrj(wxCommandEvent& _pEvent):1,
//	3. Collector::OnMenuExtractPrj(wxCommandEvent& _pEvent):1,
//	4. Collector::OnMenuListWS(wxCommandEvent& _pEvent):1,
//	5. Collector::OnMenuExtractWS(wxCommandEvent& _pEvent):1,
//	6. Collector::MesToLog(const wxString& _Text, const Logger::level _lv):1,
//
wxUint32 BuildLogger::g_CurrentProgress = 0, BuildLogger::g_MaxProgress = 0;
/// ----------------------------------------------------------------------------

#include <wx/sizer.h>
/// ----------------------------------------------------------------------------
//  Constructor
//
// Called by:
//	1.   Collector::OnAttach():1,
//
BuildLogger::BuildLogger(bool _fixedPitchFont )
	: TextCtrlLogger(_fixedPitchFont)
{
}
/// ----------------------------------------------------------------------------
//  Destructor
//
// Called by:
//	1. void OnRelease(bool _appShutDown):1,
//
BuildLogger::~BuildLogger()
{
	//dtor
}
/// -----------------------------------------------------------------------------
// Update values to 'Collector::m_pCollectLog'
//
// Called by:
// 	1. LogManager::NotifyUpdate():1,
//
void BuildLogger::UpdateSettings()
{
	TextCtrlLogger::UpdateSettings();
	style[caption].SetAlignment(wxTEXT_ALIGNMENT_DEFAULT);
	style[caption].SetFont(style[error].GetFont());
	style[error].SetFont(style[info].GetFont());
}
/// ----------------------------------------------------------------------------
//    Create a 'wxPanel' who is sized to 'wxSizer'
//
// Called by :  'main.cpp'
//	1. MainFrame::SetupGUILogging(int uiSize16):1,
//
wxWindow* BuildLogger::CreateControl(wxWindow* _parent)
{
	m_pPanel = new wxPanel(_parent);

	TextCtrlLogger::CreateControl(m_pPanel);
	control->SetId(idBuildLog);

	m_pSizer = new wxBoxSizer(wxVERTICAL);
	m_pSizer->Add(control, 1, wxEXPAND, 0);
	m_pPanel->SetSizer(m_pSizer);

	return m_pPanel;
}

/// -----------------------------------------------------------------------------
//   Add a progress bar to 'Collector::m_pCollectLog'
//
// Called by:
//	1. Collector::OnAttach():1,
//
void BuildLogger::AddBuildProgressBar()
{
	if (!m_pProgress)
	{
		m_pProgress = new wxGauge(m_pPanel, -1, 0, wxDefaultPosition, wxSize(-1, 12));
		m_pSizer->Add(m_pProgress, 0, wxEXPAND);
		m_pSizer->Layout();
	}
}

/*
// not used here
/// -----------------------------------------------------------------------------
// Remove a progress bar to 'Collector::m_pCollectLog'
//
// Called by:
//
void BuildLogger::RemoveBuildProgressBar()
{
	if (m_pProgress)
	{
		m_pSizer->Detach(m_pProgress);
		m_pProgress->Destroy();
		m_pProgress = 0;
		m_pSizer->Layout();
	}
}
*/
