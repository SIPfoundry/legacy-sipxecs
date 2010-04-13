//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

//  Border Guard
#ifndef _IRTCPRegister_h
#define _IRTCPRegister_h

#include "rtcp/RtcpConfig.h"

// Include
#include "IBaseClass.h"
#include "IRTCPNotify.h"



/**
 *
 * Interface Name:  IRTCPRegister
 *
 * Inheritance:     None
 *
 *
 * Description:     The IRTCPRegister interface allows consumers to register
 *                  an interest in receiving notifications regarding the
 *                  manifestation of certain events.  A callback interface is
 *                  provided with this interest to allow event notifications
 *                  to be delivered.  The interface also provides an
 *                  unregister method.
 *
 * Notes:
 *
 */
interface IRTCPRegister : public IBaseClass
 {

//  Public Methods
public:
/**
 *
 * Method Name: Advise()
 *
 *
 * Inputs:      IRTCPNotify *piRTCPNotify       - RTCP Event Notify Interface
 *
 * Outputs:     None
 *
 * Returns:     bool
 *
 * Description: The Advise() method shall be used by a caller to register
 *              interest in receiving events in addition to identifying an
 *              interface on which to receive notifications when these event
 *              criteria are met.
 *
 * Usage Notes: The Advise method can be used to register a new interest or
 *              modify and existing interest.  The interface pointer is used
 *              as the primary key for storing the registered interest.
 *
 */
    virtual bool Advise(IRTCPNotify *piRTCPNotify) = 0;

/**
 *
 * Method Name:  Unadvise()
 *
 *
 * Inputs:      IRTCPNotify *piRTCPNotify       - RTCP Event Notify Interface
 *
 * Outputs:     None
 *
 * Returns:     bool
 *
 * Description: The Unadvise() method allows a caller to remove a previously
 *              registered interest in receiving event.
 *
 * Usage Notes:
 *
 *
 */
    virtual bool Unadvise(IRTCPNotify *piRTCPNotify) = 0;

};

#endif
