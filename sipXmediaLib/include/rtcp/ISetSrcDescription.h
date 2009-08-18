//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


//  Border Guard
#ifndef _ISetSrcDescription_h
#define _ISetSrcDescription_h

#include "rtcp/RtcpConfig.h"

//  Includes
#include "IBaseClass.h"

/**
 *
 * Interface Name:  ISetSrcDescription
 *
 * Inheritance:     None
 *
 *
 * Description:  The ISetSrcDescription interface allows consumers to modify
 *               components of a source description including Name, Email,
 *               Phone, Location, Application Name, Notes, and Private fields.
 *
 * Notes:
 *
 */
interface ISetSrcDescription : public IBaseClass
 {

//  Public Methods

public:

/**
 *
 * Method Name:  SetSrcComponents()
 *
 *
 * Inputs:   unsigned char *puchName      - NAME field
 *           unsigned char *puchEmail     - EMAIL field
 *           unsigned char *puchPhone     - PHONE field
 *           unsigned char *puchAppName   - APPLICATION NAME
 *           unsigned char *puchLocation  - LOCATION field
 *           unsigned char *puchNotes     - NOTES field
 *           unsigned char *puchPrivate   - PRIVATE field
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: Performs a bulk set upon all the constituent elements composing
 *              a Source Description.
 *
 * Usage Notes: Performs default assignment of any arguments that may not be
 *              provided at assignment time.  The least likely known are placed
 *              at the end of the assignment list.
 *
 *              All elements passed must be NULL terminated.
 *
 */
    void SetAllComponents(unsigned char *puchName=NULL,
                          unsigned char *puchEmail=NULL,
                          unsigned char *puchPhone=NULL,
                          unsigned char *puchLocation=NULL,
                          unsigned char *puchAppName=NULL,
                          unsigned char *puchNotes=NULL,
                          unsigned char *puchPrivate=NULL);


/**
 *
 * Method Name:  SetName
 *
 *
 * Inputs:   unsigned char   *puchName  - NAME Character String
 *           unsigned long   ulLength   -
 *                                 Optional Length of NAME argument passed
 *
 * Outputs:  None
 *
 * Returns:  bool            bChanged   -
 *                                 Flag identifying whether item has changed
 *
 * Description: Stores the Name field and length, either specified of derived,
 *              as attribute data within the object.
 *
 * Usage Notes: The NAME argument MUST be passed as a NULL terminated string or
 *              must contain a valid length argument. All text strings passed
 *              shall be truncated beyond the length of 255 characters.
 *
 *
 *
 */
    virtual bool SetName(unsigned char *puchCName, unsigned long ulLength) = 0;


/**
 *
 * Method Name:  SetEmail
 *
 *
 * Inputs:   unsigned char  *puchEmail - EMAIL character string
 *           unsigned long   ulLength  -
 *                                Optional Length of Email argument passed
 *
 * Outputs:  None
 *
 * Returns:  bool   bChanged  - Flag identifying whether item has changed
 *
 * Description: Stores the Email field and length, either specified of derived,
 *              as attribute data within the object.
 *
 * Usage Notes: The EMAIL argument MUST be passed as a NULL terminated string
 *              or must contain a valid length argument. All text strings
 *              passed shall be truncated beyond the length of 255 characters.
 *
 *
 */
    virtual bool SetEmail(unsigned char *puchEmail,
                                     unsigned long ulLength=0) = 0;


/**
 *
 * Method Name:  SetPhone
 *
 *
 * Inputs:   unsigned char  *puchPhone - PHONE character string
 *           unsigned long   ulLength  -
 *                               Optional Length of Phone argument passed
 *
 * Outputs:  None
 *
 * Returns:  bool   bChanged  - Flag identifying whether item has changed
 *
 * Description: Stores the Phone field and length, either specified of derived,
 *              as attribute data within the object.
 *
 * Usage Notes: The PHONE argument MUST be passed as a NULL terminated string
 *              or must contain a valid length argument. All text strings
 *              passed shall be truncated beyond the length of 255 characters.
 *
 *
 */
    virtual bool SetPhone(unsigned char *puchPhone, unsigned long ulLength)=0;


/**
 *
 * Method Name:  SetAppName
 *
 *
 * Inputs:   unsigned char  *puchAppName - Application Name character string
 *           unsigned long   ulLength    -
 *                                 Optional Length of APP NAME argument passed
 *
 * Outputs:  None
 *
 * Returns:  bool   bChanged   - Flag identify whether item has changed
 *
 * Description: Stores the App Name field and length, either specified of
 *              derived, as attribute data within the object.
 *
 * Usage Notes: The APP NAME argument MUST be passed as a NULL terminated
 *              string or must contain a valid length argument. All text
 *              strings passed shall be truncated beyond the length of 255
 *              characters.
 *
 *
 */
    virtual bool SetAppName(unsigned char *puchAppName,
                                         unsigned long ulLength) = 0;



/**
 *
 * Method Name:  SetLocation
 *
 *
 * Inputs:   unsigned char  *puchLocation  - Location character string
 *           unsigned long   ulLength      -
 *                              Optional Length of Location argument passed
 *
 * Outputs:  None
 *
 * Returns:  bool   bChanged   - Flag identifying whether item has changed
 *
 * Description: Stores the Location field and length, either specified of
 *              derived, as attribute data within the object.
 *
 * Usage Notes: The LOCATION argument MUST be passed as a NULL terminated
 *              string or must contain a valid length argument. All text
 *              strings passed shall be truncated beyond the length of 255
 *              characters.
 *
 *
 */
    virtual bool SetLocation(unsigned char *puchLocation,
                                            unsigned long ulLength) = 0;


/**
 *
 * Method Name:  SetNotes
 *
 *
 * Inputs:   unsigned char  *puchNotes - Notes character string
 *           unsigned long   ulLength  -
 *                               Optional Length of Notes argument passed
 *
 * Outputs:  None
 *
 * Returns:  bool   bChanged   - Flag identifying whether item has changed
 *
 * Description: Stores the Notes field and length, either specified of
 *              derived, as attribute data within the object.
 *
 * Usage Notes: The NOTES argument MUST be passed as a NULL terminated string
 *              or must contain a valid length argument. All text strings
 *              passed shall be truncated beyond the length of 255 characters.
 *
 *
 */
    virtual bool SetNotes(unsigned char *puchNotes, unsigned long ulLength)=0;


/**
 *
 * Method Name:  SetPrivate
 *
 *
 * Inputs:   unsigned char  *puchNotes  - Private character string
 *           unsigned long   ulLength   -
 *                               Optional Length of Private argument passed
 *
 * Outputs:  None
 *
 * Returns:  bool   bChanged   - Flag identifying whether item has changed
 *
 * Description: Stores the Private field and length, either specified of
 *              derived, as attribute data within the object.
 *
 * Usage Notes: The PRIVATE argument MUST be passed as a NULL terminated string
 *              or must contain a valid length argument. All text strings
 *              passed shall be truncated beyond the length of 255 characters.
 *
 *
 */
    virtual bool SetPrivate(unsigned char *puchPrivate,
                                             unsigned long ulLength) = 0;

};

#endif
