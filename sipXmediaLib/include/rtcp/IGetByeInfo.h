//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


//  Border Guard
#ifndef _IGetByeInfo_h
#define _IGetByeInfo_h

#include "rtcp/RtcpConfig.h"

// Include
#include "IBaseClass.h"

/**
 *
 * Interface Name:  IGetByeInfo
 *
 * Inheritance:     None
 *
 *
 * Description:  The IGetByeInfo interface allows consumers to extract
 *               information from an RTCP Bye packet.  This data will allow
 *               a consumer to retrieve the reason for termination and the
 *               SSRC and CSRCS affected by the termination.
 *
 * Notes:
 *
 */
interface IGetByeInfo : public IBaseClass
 {

//  Public Methods

public:

/**
 *
 * Method Name:  GetSSRC
 *
 *
 * Inputs:       None
 *
 *
 * Outputs:      None
 *
 * Returns:     unsigned long - The SSRC of the Bye Report
 *
 * Description: Returns the SSRC Associated with the Bye Report.
 *
 * Usage Notes:
 *
 *
 */
    virtual unsigned long GetSSRC(void)=0;


/**
 *
 * Method Name:  GetReason
 *
 *
 * Inputs:      None
 *
 * Outputs: unsigned char *puchReason
 *           Character buffer in which the Reason attribute shall be returned
 *
 * Returns: unsigned long
 *           Length of the item being returned in the unsigned character buffer
 *
 * Description: Retrieves the Reason attribute stored within the object and
 *              returns its associated length.
 *
 * Usage Notes: All unsigned character strings returned are limited to 255
 *              bytes in length. Any wide unsigned character support for
 *              internationalized display is a responsibility assumed by
 *              the caller.
 */
    virtual unsigned long GetReason(unsigned char *puchReason)=0;

/**
 *
 * Method Name:  GetCSRC
 *
 *
 * Inputs:  bool bNBO
 *           Flag identifying whether data should be represented in NBO format

 *
 * Outputs: unsigned long *paulCSRC
 *           Contributing Source Identifier(s) Array pointer
 *
 * Returns: unsigned long - Number of Contributing Source Identifier(s)
 *
 * Description: Returns the contributing source values associated with the
 *              RTP packet.
 *
 * Usage Notes:
 *
 *
 */
    virtual unsigned long GetCSRC(unsigned long *paulCSRC, bool bNBO=FALSE)=0;

/**
 *
 * Method Name:  GetByeInterface()
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     IByeReport *  - Pointer to the Bye Report Interface
 *
 * Description: Returns the Bye Report interface.
 *
 * Usage Notes:
 *
 */
    virtual IByeReport * GetByeInterface(void) = 0;

};

#endif
