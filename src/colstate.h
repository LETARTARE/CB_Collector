/*************************************************************
 * Name:      colstate.h
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2022-03-06
 * Modified:  2022-03-14
 * Copyright: LETARTARE
 * License:   GPL
 *************************************************************
 */

//------------------------------------------------------------------------------
#ifndef _COLSTATE_H
#define _COLSTATE_H
//------------------------------------------------------------------------------

#include "wx/string.h"

//------------------------------------------------------------------------------
/**	\class ColState
 *	\brief This classe share data between classe 'Pre' and 'Collector'
 */
class ColState
{
    protected:
    /** \brief Defines the current state of the 'Collector'.
     *  \brief to use with state graph
     */
    enum colState
    {
        fbNone = 10,            // no state
        fbWaitingForStart,      // wait state  for list
        fbStop,                 // stop state
        fbCleanTemp,            // clean temporary chains
        // project
        fbListPrj,              // project list
        fbWaitForExtractPrj,    // wait state  for  project extract
        fbExtractPrj,           // project extract
        fbListExtractPrj,       // macro state = 'fbListPrj' followed by 'fbExtractPrj'
        // workspace
        fbListWS,               // workspace list
        fbWaitForExtractWS,     // wait state  for workspace extract
        fbExtractWS,            // workspace extract
        fbListExtractWS         // macro state = 'fbListWS' followed by 'fbExtractWS'
    };

    public:
        /** \brief Constructor
         */
        ColState();

        /** \brief Destructor
         */
        virtual ~ColState();

    protected:

        /** \brief State of the state graph
         */
        static colState m_State;

        /** \brief Returns a string corresponding to the state of the 'Collector'
		 *   state graph
         *  \param _state : state of the state graph
         */
        static wxString strState(const colState& _state);
};

#endif // _COLSTATE_H
