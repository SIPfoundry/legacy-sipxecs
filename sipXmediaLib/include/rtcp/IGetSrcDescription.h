//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


//  Border Guard
#ifndef _IGetSrcDescription_h
#define _IGetSrcDescription_h

#include "rtcp/RtcpConfig.h"

// Include
#include "IBaseClass.h"

// Defines
#define CNAME_FIELD        0x1
#define NAME_FIELD         0x2
#define EMAIL_FIELD        0x4
#define PHONE_FIELD        0x8
#define LOCATION_FIELD     0x10
#define APPNAME_FIELD      0x20
#define NOTE_FIELD         0x40
#define PRIVATE_FIELD      0x80

#define CNAME_ID            1
#define NAME_ID             2
#define EMAIL_ID            3
#define PHONE_ID            4
#define LOCATION_ID         5
#define APPNAME_ID          6
#define NOTE_ID             7
#define PRIVATE_ID          8

#define MAX_ENTRYSIZE      255

//  Forward Declarations
interface ISDESReport;

/**
 *
 * Interface Name:  IGetSrcDescription
 *
 * Inheritance:  None
 *
 *
 * Description:  The IGetSrcDescription interface allows consumers to retrieve
 *               components of a source description including Name, Email,
 *               Phone, Location, Application Name, Notes, and Private fields.
 *
 * Notes:
 *
 */
interface IGetSrcDescription : public IBaseClass
 {

//  Public Methods

public:

/**
 *
 * Method Name:  GetChanges
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long
 * The aggregate changes that have occurred in the report since the last period
 *
 * Description: Retrieves the aggregate changes that have occurred in the
 *              report since the last period
 *
 * Usage Notes: Mask is an OR'ing of all changes for a period.
 */
    virtual unsigned long GetChanges(void)=0;

/**
 *
 * Method Name:  GetFieldChange
 *
 *
 * Input:       unsigned long  ulChangeMask
 *                A mask identifying the changed field in an SDES Report
 *
 * Outputs:     unsigned long *pulFieldType
 *                The Field Identifier present in change mask
 *              unsigned char *puchReportBuffer
 *                Character Buffer to store contents of field
 *
 * Returns:     unsigned long - The modified change mask
 *
 * Description: Gets a field from an SDES Report based upon the change mask
 *              passed.  A field present within the change mask shall have
 *              its ID and field contents loaded as output arguments to this
 *              call.  The change mask shall be modified to reflect the
 *              removal of the field change that is being returned.
 *
 * Usage Notes: This may be called multiple times to extract all the changes
 *              from an SDES report.  No more changes are available once the
 *              mask has a value of 0.
 */
    virtual unsigned long GetFieldChange(unsigned long ulChangeMask,
                                         unsigned long *pulFieldType,
                                         unsigned char *puchFieldBuffer) = 0;


/**
 *
 * Method Name:  GetAllComponents()
 *
 *
 * Inputs:      unsigned char          *puchName     - NAME field
 *              unsigned char          *puchEmail    - EMAIL field
 *              unsigned char          *puchPhone    - PHONE field
 *              unsigned char          *puchAppName  - APPLICATION NAME
 *              unsigned char          *puchLocation - LOCATION field
 *              unsigned char          *puchNotes    - NOTES field
 *              unsigned char          *puchPrivate  - PRIVATE field
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: Performs a bulk get upon all the constituent elements
 *              composing a Source Description.
 *
 * Usage Notes: The elements retrieved are NULL terminated.
 *
 */
    virtual void GetAllComponents(unsigned char *puchName,
                                  unsigned char *puchEmail,
                                  unsigned char *puchPhone,
                                  unsigned char *puchLocation,
                                  unsigned char *puchAppName,
                                  unsigned char *puchNotes,
                                  unsigned char *puchPrivate) = 0;

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
 */
    virtual unsigned long GetSSRC(void)=0;

/**
 *
 * Method Name:  GetName
 *
 *
 * Inputs:    None
 *
 * Outputs:   unsigned char *puchName
 *              Character buffer in which the NAME attribute shall be returned
 *
 * Returns:   unsigned long
 *              Length of the item being returned in the buffer
 *
 * Description: Retrieves the Name attribute stored within the object and
 *              returns its associated length.
 *
 * Usage Notes: All unsigned character strings returned are limited to 255
 *              bytes in length. Any wide unsigned character support for
 *              internationalized display is a responsibility assumed by
 *              the caller.
 */
    virtual unsigned long GetName(unsigned char *puchName) = 0;



/**
 *
 * Method Name:  GetEmail
 *
 *
 * Inputs:   None
 *
 * Outputs:  unsigned char *puchEmail
 *             Character buffer in which the EMAIL attribute shall be returned
 *
 * Returns:  unsigned long
 *             Length of the item being returned in the buffer
 *
 * Description: Retrieves the Email attribute stored within the object and
 *              returns its associated length.
 *
 * Usage Notes: All unsigned character strings returned are limited to 255
 *              bytes in length. Any wide unsigned character support for
 *              internationalized display is a responsibility assumed by
 *              the caller.
 */
    virtual unsigned long GetEmail(unsigned char *puchEmail) = 0;



/**
 *
 * Method Name:  GetPhone
 *
 *
 * Inputs:   None
 *
 * Outputs:  unsigned char *puchPhone
 *             Character buffer in which the PHONE attribute shall be returned
 *
 * Returns:  unsigned long
 *             Length of the item being returned in the buffer
 *
 * Description: Retrieves the Phone attribute stored within the object and
 *              returns its associated length.
 *
 * Usage Notes: All unsigned character strings returned are limited to 255
 *              bytes in length. Any wide unsigned character support for
 *              internationalized display is a responsibility assumed by
 *              the caller.
 */
    virtual unsigned long GetPhone(unsigned char *puchPhone) = 0;


/**
 *
 * Method Name:  GetAppName
 *
 *
 * Inputs:   None
 *
 * Outputs:  unsigned char *puchAppName
 *             Character buffer in which the APPLICATION NAME attribute shall
 *             be returned
 *
 * Returns:  unsigned long
 *             Length of the item being returned in the buffer
 *
 * Description: Retrieves the Application Name attribute stored within the
 *              object and returns its associated length.
 *
 * Usage Notes: All unsigned character strings returned are limited to 255
 *              bytes in length. Any wide unsigned character support for
 *              internationalized display is a responsibility assumed by
 *              the caller.
 */
    virtual unsigned long GetAppName(unsigned char *puchAppName) = 0;



/**
 *
 * Method Name:  GetLocation
 *
 *
 * Inputs:   None
 *
 * Outputs:  unsigned char *puchLocation
 *             Character buffer in which the LOCATION attribute shall be
 *             returned
 *
 * Returns:  unsigned long
 *             Length of the item being returned in the buffer
 *
 * Description: Retrieves the Location attribute stored within the object
 *              and returns its associated length.
 *
 * Usage Notes: All unsigned character strings returned are limited to 255
 *              bytes in length. Any wide unsigned character support for
 *              internationalized display is a responsibility assumed by
 *              the caller.
 */
    virtual unsigned long GetLocation(unsigned char *puchLocation) = 0;



/**
 *
 * Method Name:  GetNotes
 *
 *
 * Inputs:   None
 *
 * Outputs:  unsigned char *puchNotes
 *             Character buffer in which the Notes attribute shall be returned
 *
 * Returns:  unsigned long
 *             Length of the item being returned in the buffer
 *
 * Description: Retrieves the Notes attribute stored within the object and
 *              returns its associated length.
 *
 * Usage Notes: All unsigned character strings returned are limited to 255
 *              bytes in length. Any wide unsigned character support for
 *              internationalized display is a responsibility assumed by
 *              the caller.
 */
    virtual unsigned long GetNotes(unsigned char *puchNotes) = 0;

/**
 *
 * Method Name:  GetPrivate
 *
 *
 * Inputs:   None
 *
 * Outputs:  unsigned char *puchPrivate
 *             Character buffer in which the Private attribute shall be returned
 *
 * Returns:  unsigned long
 *             Length of the item being returned in the buffer
 *
 * Description: Retrieves the Private attribute stored within the object and
 *              returns its associated length.
 *
 * Usage Notes: All unsigned character strings returned are limited to 255
 *              bytes in length. Any wide unsigned character support for
 *              internationalized display is a responsibility assumed by
 *              the caller.
 */
    virtual unsigned long GetPrivate(unsigned char *puchNotes) = 0;

/**
 *
 * Method Name:  GetSDESInterface()
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     ISDESReport *  - Pointer to the SDES Report Interface
 *
 * Description: Returns the SDES Report interface.
 *
 * Usage Notes:
 *
 */
    virtual ISDESReport * GetSDESInterface(void) = 0;
};

#endif
