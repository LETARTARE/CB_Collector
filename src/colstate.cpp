/*************************************************************
 * Name:      colstate.cpp
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2022-03-06
 * Modified:  2022-03-17
 * Copyright: LETARTARE
 * License:   GPL
 *************************************************************
 */

#include "colstate.h"
#include "defines.h"        // for 'quoteNS(wxString)'
///-----------------------------------------------------------------------------
// init static 'm_State'
ColState::colState ColState::m_State = fbWaitingForStart;
///-----------------------------------------------------------------------------
//	Constructor
//
// Called by :
//     1. Collector::Collector():1,
//     2. Pre::Pre(const wxString & _nameplugin, int _logindex):1,
//
ColState::ColState()
{
}
//-----------------------------------------------------------------------------
//	Destructor
//
// Called by :
//
ColState::~ColState()
{
    //dtor
}

///-----------------------------------------------------------------------------
// Write a string for a state of 'colState'
//
// Called by :
//     1. Collector::OnMenuWaitingForStart(wxCommandEvent& _pEvent):2,
//
wxString ColState::strState(const colState& _state)
{
    wxString str = "None";
    switch (_state)
    {
    	case fbNone             : str = "None" ; break;
        case fbStop             : str = "Stop" ; break;
        case fbCleanTemp        : str = "CLeanTemp" ; break;
        case fbWaitingForStart  : str = "WaitListAll" ; break;
        // project
        case fbListPrj          : str = "ListPrj" ; break;
        case fbWaitForExtractPrj: str = "WaitExtractPrj" ; break;
        case fbExtractPrj       : str = "ExtractPrj" ; break;
        case fbListExtractPrj   : str = "ListExtractPrj" ; break;
        // workspace
        case fbListWS           : str = "ListWS" ; break;
        case fbWaitForExtractWS : str = "WaitExtractWS" ; break;
        case fbExtractWS        : str = "ExtractWS" ; break;
        case fbListExtractWS    : str = "ListExtractWS" ; break;
    }

    return quoteNS(str);
}
