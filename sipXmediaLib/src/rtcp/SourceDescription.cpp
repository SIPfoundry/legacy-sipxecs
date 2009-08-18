//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////



    // Includes
#include "rtcp/SourceDescription.h"

#ifdef __pingtel_on_posix__
#include <netinet/in.h>
#include <unistd.h>
#endif

#ifdef INCLUDE_RTCP /* [ */


    // Constants

    // SDES Fields Identifiers
const int TERMINATOR        = 0;
const int REPORT_CYCLES     = 3;


   // Static Variable Initialization
CSourceDescription *CSourceDescription::m_spoLocalSDES = NULL;

/**
 *
 * Method Name:  GetLocalSDES()
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  ISDESReport *piSDESReport
 *                             - Pointer to local Source Description Interface
 *
 * Description:  A static member function used to obtain an ISDESReport
 *               interface.
 *
 * Usage Notes:  This method shall cause the local Source Description Singleton
 *               object to be created if it has not already been instantiated.
 *
 */
CSourceDescription *CSourceDescription::GetLocalSDES(void)
{

    // If the Source Description object does not yet exist or hasn't been
    //  started, then acquire the lock to ensure that only one instance of
    //  the task is started
      // sLock.acquire();
    if (m_spoLocalSDES == NULL)
    {
        m_spoLocalSDES = new CSourceDescription(0,
            (unsigned char *)"Your Name Here",
            (unsigned char *)"caller@pingtel.com",
            (unsigned char *)"(781)938-5306",
            (unsigned char *)
                      "Suite 2200, 400 West Cummings Park, Woburn MA 01801",
            (unsigned char *)"Xpressa",
            (unsigned char *)"Insert User Profile Here",
            (unsigned char *)"-private data-");
        if(!m_spoLocalSDES)
        {
            osPrintf("**** FAILURE **** CSourceDescription::GetLocalSDES()"
                     " - Unable to Create Local Source Description Object\n");
            return(NULL);
        }
    }

    // Check whether the local Source Description object has been initialized.
    // Initialize it if it has not yet been done.
    if(!m_spoLocalSDES->IsInitialized())
    {
       if(!m_spoLocalSDES->Initialize())
       {
           osPrintf("**** FAILURE **** CSourceDescription::GetLocalSDES()"
                                     " - Unable to Initialize SDES object\n");
           m_spoLocalSDES->Release();
           m_spoLocalSDES = NULL;
           return(NULL);
       }

       return(m_spoLocalSDES);
    }

      // sLock.release();

    // Bump the reference count to this object
    m_spoLocalSDES->AddRef();

    return(m_spoLocalSDES);

}

/**
 *
 * Method Name:  CSourceDescription() - Constructor
 *
 *
 * Inputs:      unsigned long   ulSSRC       - SSRC ID
 *              unsigned char  *puchName     - NAME field
 *              unsigned char  *puchEmail    - EMAIL field
 *              unsigned char  *puchPhone    - PHONE field
 *              unsigned char  *puchAppName  - APPLICATION NAME
 *              unsigned char  *puchLocation - LOCATION field
 *              unsigned char  *puchNotes    - NOTES field
 *              unsigned char  *puchPrivate  - PRIVATE field
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: Performs CSourceDescription object initialization.  This
 *              constructor shall be called to form a Source Description
 *              identifying a local Pingtel phone set.
 *
 * Usage Notes: Performs default assignment of any arguments that may not be
 *              known at construction time.  The least likely known at
 *              construction time are placed at the end of the construction
 *              list.
 *
 */
CSourceDescription::CSourceDescription(unsigned long ulSSRC,
            unsigned char *puchName, unsigned char *puchEmail,
            unsigned char *puchPhone, unsigned char *puchLocation,
            unsigned char *puchAppName, unsigned char *puchNotes,
            unsigned char *puchPrivate)
               :CRTCPHeader(ulSSRC, etSDESReport),  // Base class construction
                m_bCNameSet(FALSE),
                m_ulContentMask(0),
                m_ulCNameLength(0),
                m_ulNameLength(0),
                m_ulEmailLength(0),
                m_ulPhoneLength(0),
                m_ulAppNameLength(0),
                m_ulLocationLength(0),
                m_ulNotesLength(0),
                m_ulPrivateLength(0)
{

    // Copy all Source Description elements provided at construction time into
    //  internal storage

    // Assign SSRC
    SetSSRC(ulSSRC);

    // Assign Field Information elements of the Source Description object
    SetAllComponents(puchName, puchEmail,
                puchPhone, puchLocation, puchAppName, puchNotes, puchPrivate);

}



/**
 *
 * Method Name:  CSourceDescription() - Constructor
 *
 *
 * Inputs:   bool bIncludeHeader
 *                             - TRUE => a header precedes the SDES field info
 *           unsigned char *puchSDESReport
 *                           - An SDES Report received from a call participant
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: Performs CSourceDescription object initialization.  This
 *              constructor shall be called to construct a Source Description
 *              object identifying a FE call participant.  The event interests
 *              register and notification interface passed shall be used to
 *              generate events and deliver them to their intended recipients.
 *
 * Usage Notes: Performs default assignment of any arguments that may not be
 *              known at construction time.
 *
 *              Uses wide character format for internationalization purposes.
 */
CSourceDescription::CSourceDescription(bool bIncludeHeader,
                                       unsigned char *puchSDESReport)
                   :CRTCPHeader(0, etSDESReport),  // Base class construction
                    m_ulContentMask(0),
                    m_ulChangeMask(0),
                    m_ulCNameLength(0),
                    m_ulNameLength(0),
                    m_ulEmailLength(0),
                    m_ulPhoneLength(0),
                    m_ulAppNameLength(0),
                    m_ulLocationLength(0),
                    m_ulNotesLength(0),
                    m_ulPrivateLength(0)
{

    // Check whether a source description report has been provided at
    //  construction time.  If so, delegate to the parsing method to
    //  extract all SDES contents.
    if(puchSDESReport != NULL)
        ParseSDESReport(bIncludeHeader, puchSDESReport);
}



/**
 *
 * Method Name: ~CSourceDescription() - Destructor
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: Shall deallocate and/or release all resources that were
 *              acquired over the course of runtime.
 *
 * Usage Notes:
 *
 *
 */
CSourceDescription::~CSourceDescription(void)
{
// Our reference count must have gone to 0 to get here.  We have not allocated
// any memory so we shall now go quietly into that good night!
}


/**
 *
 * Method Name:  SetAllComponents()
 *
 *
 * Inputs:      unsigned char *puchName      - NAME field
 *              unsigned char *puchEmail     - EMAIL field
 *              unsigned char *puchPhone     - PHONE field
 *              unsigned char *puchAppName   - APPLICATION NAME
 *              unsigned char *puchLocation  - LOCATION field
 *              unsigned char *puchNotes     - NOTES field
 *              unsigned char *puchPrivate   - PRIVATE field
 *
 * Outputs:     None
 *
 * Returns:     None
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
void CSourceDescription::SetAllComponents(unsigned char *puchName,
        unsigned char *puchEmail, unsigned char *puchPhone,
        unsigned char *puchLocation, unsigned char *puchAppName,
        unsigned char *puchNotes, unsigned char *puchPrivate)
{

    // Copy Name contents and store length
    if(puchName != NULL)
    {
        strcpy((char *)m_uchName, (char *)puchName);
        m_ulNameLength = strlen((char *)puchName);
    }

    // Copy Email contents and store length
    if(puchEmail != NULL)
    {
        strcpy((char *)m_uchEmail, (char *)puchEmail);
        m_ulEmailLength = strlen((char *)puchEmail);
    }

    // Copy Phone contents and store length
    if(puchPhone != NULL)
    {
        strcpy((char *)m_uchPhone, (char *)puchPhone);
        m_ulPhoneLength = strlen((char *)puchPhone);
    }

    // Copy Location contents and store length
    if(puchLocation != NULL)
    {
        strcpy((char *)m_uchLocation, (char *)puchLocation);
        m_ulLocationLength = strlen((char *)puchLocation);
    }

    // Copy AppName contents and store length
    if(puchAppName != NULL)
    {
        strcpy((char *)m_uchAppName, (char *)puchAppName);
        m_ulAppNameLength = strlen((char *)puchAppName);
    }

    // Copy Notes contents and store length
    if(puchNotes != NULL)
    {
        strcpy((char *)m_uchNotes, (char *)puchNotes);
        m_ulNotesLength = strlen((char *)puchNotes);
    }

    // Copy Private contents and store length
    if(puchNotes != NULL)
    {
        strcpy((char *)m_uchPrivate, (char *)puchPrivate);
        m_ulPrivateLength = strlen((char *)puchPrivate);
    }

}


/**
 *
 * Method Name:  GetAllComponents()
 *
 *
 * Inputs:      unsigned char *puchName      - NAME field
 *              unsigned char *puchEmail     - EMAIL field
 *              unsigned char *puchPhone     - PHONE field
 *              unsigned char *puchAppName   - APPLICATION NAME
 *              unsigned char *puchLocation  - LOCATION field
 *              unsigned char *puchNotes     - NOTES field
 *              unsigned char *puchPrivate   - PRIVATE field
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: Performs a bulk get upon all the constituent elements
 *              composing a Source Description.
 *
 * Usage Notes: The elements retrieved are NULL terminated so strings passed
 *              have to be at least 256 characters in length.
 *
 */
void CSourceDescription::GetAllComponents(unsigned char *puchName,
        unsigned char *puchEmail, unsigned char *puchPhone,
        unsigned char *puchLocation, unsigned char *puchAppName,
        unsigned char *puchNotes, unsigned char *puchPrivate)
{

    // Copy Name contents
    if(puchName != NULL)
        strcpy((char *)puchName, (char *)m_uchName);

    // Copy Email contents
    if(puchEmail != NULL)
        strcpy((char *)puchEmail, (char *)m_uchEmail);

    // Copy Phone contents
    if(puchPhone != NULL)
        strcpy((char *)puchPhone, (char *)m_uchPhone);

    // Copy Location contents
    if(puchLocation != NULL)
        strcpy((char *)puchLocation, (char *)m_uchLocation);

    // Copy AppName contents
    if(puchAppName != NULL)
        strcpy((char *)puchAppName, (char *)m_uchAppName);

    // Copy Notes contents
    if(puchNotes != NULL)
        strcpy((char *)puchNotes, (char *)m_uchNotes);

    // Copy Private contents
    if(puchPrivate != NULL)
        strcpy((char *)puchPrivate, (char *)m_uchPrivate);

}

/**
 *
 * Method Name:  SetSSRC
 *
 *
 * Inputs:      unsigned long   ulSSRC   - Source ID
 *
 * Outputs:     None
 *
 * Returns:     void
 *
 * Description: Stores the Source Identifier associated with an RTP connection.
 *
 * Usage Notes:
 *
 *
 *
 */
void CSourceDescription::SetSSRC(unsigned long ulSSRC)
{

    // Store the modified SSRC as an internal attribute
    CRTCPHeader::SetSSRC(ulSSRC);

}

/**
 *
 * Method Name:  SetCName
 *
 *
 * Inputs:   unsigned char *puchCName - CNAME Character String
 *           unsigned long  ulLength  - Optional Length of CNAME argument passed
 *
 * Outputs:  None
 *
 * Returns:  bool
 *
 * Description: Stores the CName field and length, either specified of derived,
 *              as attributed within the object.
 *
 * Usage Notes: The CNAME argument MUST be passed as a NULL terminated string
 *              or must contain a valid length argument.  All text strings
 *              passed shall be truncated beyond the length of 255 characters.
 *
 */
bool CSourceDescription::SetCName(unsigned char *puchCName,
                                  unsigned long ulLength)
{

    // Check whether a valid length was passed
    if(ulLength)
    {
        // Check for change
        if(strncmp((char *)puchCName, (char *)m_uchCName, ulLength) == 0)
            return(FALSE);

        // Make sure length is no greater than MAX_SOURCE_LENGTH
        m_ulCNameLength =
                ulLength < MAX_SOURCE_LENGTH ? ulLength : MAX_SOURCE_LENGTH-1;
        strncpy((char *)m_uchCName, (char *)puchCName, m_ulCNameLength);
        m_uchCName[m_ulCNameLength] = 0;
    }
    else
    {
        // Check for change
        if(strcmp((char *)puchCName, (char *)m_uchCName) == 0)
            return(FALSE);

        // Assume NULL termination and do a straight string copy
        strcpy((char *)m_uchCName, (char *)puchCName);
        m_ulCNameLength = strlen((char *)puchCName);
    }

    return(TRUE);
}


/**
 *
 * Method Name:  GetCName
 *
 *
 * Inputs:   None
 *
 * Outputs:  unsigned char *puchCName
 *                     - buffer in which the CNAME attribute shall be returned
 *
 * Returns:  unsigned long - Length of the item being returned in the buffer
 *
 * Description: Retrieves the CName attribute stored within the object and
 *              returns its associated length.
 *
 * Usage Notes: All character strings returned are limited to 255 bytes in
 *              length.  Any wide character support for internationalized
 *              display is a responsibility assumed by the caller.
 *
 *
 */
 unsigned long CSourceDescription::GetCName(unsigned char *puchCName)
 {

    // Copy the attribute contents into the output buffer passed
     if(puchCName != NULL)
        strcpy((char *)puchCName, (char *)m_uchCName);

    return(m_ulCNameLength);

 }

/**
 *
 * Method Name:  SetName
 *
 *
 * Inputs:      unsigned char  *puchName   - NAME Character String
 *              unsigned long   ulLength   - Length of NAME argument passed
 *
 * Outputs:     None
 *
 * Returns:     bool
 *
 * Description: Stores the Name field and length, either specified of derived,
 *              as attributed within the object.
 *
 * Usage Notes: The NAME argument MUST be passed as a NULL terminated string
 *              or must contain a valid length argument.  All text strings
 *              passed shall be truncated beyond the length of 255 characters.
 *
 */
bool CSourceDescription::SetName(unsigned char *puchName,
                                 unsigned long ulLength)
{

    // Check whether a valid length was passed
    if(ulLength)
    {
        // Check for change
        if(strncmp((char *)puchName, (char *)m_uchName, ulLength) == 0)
            return(FALSE);

        // Make sure length is no greater than MAX_LENGTH
        m_ulNameLength =
                ulLength < MAX_SOURCE_LENGTH ? ulLength : MAX_SOURCE_LENGTH-1;
        strncpy((char *)m_uchName, (char *)puchName, m_ulNameLength);
        m_uchName[m_ulNameLength] = 0;
    }
    else if(puchName != NULL)
    {
        // Check for change
        if(strcmp((char *)puchName, (char *)m_uchName) == 0)
            return(FALSE);

        // Assume NULL termination and do a straight string copy
        strcpy((char *)m_uchName, (char *)puchName);
        m_ulNameLength = strlen((char *)puchName);
    }

    return(TRUE);
}



/**
 *
 * Method Name:  GetName
 *
 *
 * Inputs:   None
 *
 * Outputs:  unsigned char *puchName
 *                      - buffer in which the NAME attribute shall be returned
 *
 * Returns:  unsigned long  - Length of the item being returned in the buffer
 *
 * Description: Retrieves the Name attribute stored within the object and
 *              returns its associated length.
 *
 * Usage Notes: All character strings returned are limited to 255 bytes in
 *              length.  Any wide character support for internationalized
 *              display is a responsibility assumed by the caller.
 *
 */
unsigned long CSourceDescription::GetName(unsigned char *puchName)
{

    // Copy the attribute contents into the output buffer passed
    if(puchName != NULL)
    {
        strcpy((char *)puchName, (char *)m_uchName);
        return(m_ulNameLength);
    }

    return(0);

}



/**
 *
 * Method Name:  SetEmail
 *
 *
 * Inputs:      unsigned char *puchEmail  - EMAIL character string
 *              unsigned long  ulLength   - Length of Email argument passed
 *
 * Outputs:     None
 *
 * Returns:     bool
 *
 * Description: Stores the Email field and length, either specified of derived,
 *              as attributed within the object.
 *
 * Usage Notes: The EMAIL argument MUST be passed as a NULL terminated string
 *              or must contain a valid length argument.  All text strings
 *              passed shall be truncated beyond the length of 255 characters.
 *
 *
 */
bool CSourceDescription::SetEmail(unsigned char *puchEmail,
                                  unsigned long ulLength)
{

    // Check whether a valid length was passed
    if(ulLength)
    {
        // Check for change
        if(strncmp((char *)puchEmail, (char *)m_uchEmail, ulLength) == 0)
            return(FALSE);

        // Make sure length is no greater than MAX_LENGTH
        m_ulEmailLength =
                ulLength < MAX_SOURCE_LENGTH ? ulLength : MAX_SOURCE_LENGTH-1;
        strncpy((char *)m_uchEmail, (char *)puchEmail, m_ulEmailLength);
        m_uchEmail[m_ulEmailLength] = 0;
    }
    else if(puchEmail != NULL)
    {
        // Check for change
        if(strcmp((char *)puchEmail, (char *)m_uchEmail) == 0)
            return(FALSE);

        // Assume NULL termination and do a straight string copy
        strcpy((char *)m_uchEmail, (char *)puchEmail);
        m_ulEmailLength = strlen((char *)puchEmail);
    }

    return(TRUE);
}


/**
 *
 * Method Name:  GetEmail
 *
 *
 * Inputs:   None
 *
 * Outputs:  unsigned char *puchEmail
 *                     - buffer in which the EMAIL attribute shall be returned
 *
 * Returns:  unsigned long - Length of the item being returned in the buffer
 *
 * Description: Retrieves the Email attribute stored within the object and
 *              returns its associated length.
 *
 * Usage Notes: All character strings returned are limited to 255 bytes in
 *              length.  Any wide character support for internationalized
 *              display is a responsibility assumed by the caller.
 *
 *
 */
unsigned long CSourceDescription::GetEmail(unsigned char *puchEmail)
{

    // Copy the attribute contents into the output buffer passed
    if(puchEmail != NULL)
    {
        strcpy((char *)puchEmail, (char *)m_uchEmail);
        return(m_ulEmailLength);
    }

    return(0);
}

/**
 *
 * Method Name:  SetPhone
 *
 *
 * Inputs:      unsigned char  *puchPhone   - PHONE character string
 *              unsigned long   ulLength    - Length of Phone argument passed
 *
 * Outputs:     None
 *
 * Returns:     bool
 *
 * Description: Stores the Phone field and length, either specified of derived,
 *              as attributed within the object.
 *
 * Usage Notes: The PHONE argument MUST be passed as a NULL terminated string
 *              or must contain a valid length argument.  All text strings
 *              passed shall be truncated beyond the length of 255 characters.
 *
 *
 */
bool CSourceDescription::SetPhone(unsigned char *puchPhone,
                                  unsigned long ulLength)
{

    // Check whether a valid length was passed
    if(ulLength)
    {
        // Check for change
        if(strncmp((char *)puchPhone, (char *)m_uchPhone, ulLength) == 0)
            return(FALSE);

        // Make sure length is no greater than MAX_LENGTH
        m_ulPhoneLength =
                ulLength < MAX_SOURCE_LENGTH ? ulLength : MAX_SOURCE_LENGTH-1;
        strncpy((char *)m_uchPhone, (char *)puchPhone, m_ulPhoneLength);
        m_uchPhone[m_ulPhoneLength] = 0;
    }
    else if(puchPhone != NULL)
    {
        // Check for change
        if(strcmp((char *)puchPhone, (char *)m_uchPhone) == 0)
            return(FALSE);

        // Assume NULL termination and do a straight string copy
        strcpy((char *)m_uchPhone, (char *)puchPhone);
        m_ulPhoneLength = strlen((char *)puchPhone);
    }

    return(TRUE);
}



/**
 *
 * Method Name:  GetPhone
 *
 *
 * Inputs:   None
 *
 * Outputs:  unsigned char *puchPhone
 *                     - buffer in which the PHONE attribute shall be returned
 *
 * Returns:  unsigned long - Length of the item being returned in the buffer
 *
 * Description: Retrieves the Phone attribute stored within the object and
 *              returns its associated length.
 *
 * Usage Notes: All character strings returned are limited to 255 bytes in
 *              length.  Any wide character support for internationalized
 *              display is a responsibility assumed by the caller.
 */
unsigned long CSourceDescription::GetPhone(unsigned char *puchPhone)
{

    // Copy the attribute contents into the output buffer passed
    if(puchPhone != NULL)
    {
        strcpy((char *)puchPhone, (char *)m_uchPhone);
        return(m_ulPhoneLength);
    }

    return(0);

}

/**
 *
 * Method Name:  SetAppName
 *
 *
 * Inputs:   unsigned char  *puchAppName - Application Name character string
 *           unsigned long   ulLength    - Length of APP NAME argument passed
 *
 * Outputs:  None
 *
 * Returns:  bool
 *
 * Description: Stores the App Name field and length, either specified of
 *              derived, as attributed within the object.
 *
 * Usage Notes: The APP NAME argument MUST be passed as a NULL terminated
 *              string or must contain a valid length argument.  All text
 *              strings passed shall be truncated beyond the length of 255
 *              characters.
 */
bool CSourceDescription::SetAppName(unsigned char *puchAppName,
                                    unsigned long ulLength)
{

    // Check whether a valid length was passed
    if(ulLength)
    {
        // Check for change
        if(strncmp((char *)puchAppName, (char *)m_uchAppName, ulLength) == 0)
            return(FALSE);

        // Make sure length is no greater than MAX_LENGTH
        m_ulAppNameLength =
                ulLength < MAX_SOURCE_LENGTH ? ulLength : MAX_SOURCE_LENGTH-1;
        strncpy((char *)m_uchAppName, (char *)puchAppName, m_ulAppNameLength);
        m_uchAppName[m_ulAppNameLength] = 0;
    }
    else if(puchAppName != NULL)
    {
        // Check for change
        if(strcmp((char *)puchAppName, (char *)m_uchAppName) == 0)
            return(FALSE);

        // Assume NULL termination and do a straight string copy
        strcpy((char *)m_uchAppName, (char *)puchAppName);
        m_ulAppNameLength = strlen((char *)puchAppName);
    }

    return(TRUE);
}


/**
 *
 * Method Name:  GetAppName
 *
 *
 * Inputs:   None
 *
 * Outputs:  unsigned char *puchAppName
 *          - buffer in which the APPLICATION NAME attribute shall be returned
 *
 * Returns:  unsigned long - Length of the item being returned in the buffer
 *
 * Description: Retrieves the Application Name attribute stored within the
 *              object and returns its associated length.
 *
 * Usage Notes: All character strings returned are limited to 255 bytes in
 *              length.  Any wide character support for internationalized
 *              display is a responsibility assumed by the caller.
 */
unsigned long CSourceDescription::GetAppName(unsigned char *puchAppName)
{

    // Copy the attribute contents into the output buffer passed
    if(puchAppName != NULL)
    {
        strcpy((char *)puchAppName, (char *)m_uchAppName);
        return(m_ulAppNameLength);
    }

    return(0);

}



/**
 *
 * Method Name:  SetLocation
 *
 *
 * Inputs:   unsigned char *puchLocation  - Location character string
 *           unsigned long  ulLength      - Length of Location argument passed
 *
 * Outputs:  None
 *
 * Returns:  bool
 *
 * Description: Stores the Location field and length, either specified of
 *              derived, as attributed within the object.
 *
 * Usage Notes: The LOCATION argument MUST be passed as a NULL terminated
 *              string or must contain a valid length argument.  All text
 *              strings passed shall be truncated beyond the length of 255
 *              characters.
 *
 *
 */
bool CSourceDescription::SetLocation(unsigned char *puchLocation,
                                     unsigned long ulLength)
{

    // Check whether a valid length was passed
    if(ulLength)
    {
        // Check for change
        if(strncmp((char *)puchLocation, (char *)m_uchLocation, ulLength) == 0)
            return(FALSE);

        // Make sure length is no greater than MAX_LENGTH
        m_ulLocationLength =
                ulLength < MAX_SOURCE_LENGTH ? ulLength : MAX_SOURCE_LENGTH-1;
        strncpy((char *)m_uchLocation,
                                    (char *)puchLocation, m_ulLocationLength);
        m_uchLocation[m_ulLocationLength] = 0;
    }
    else if(puchLocation != NULL)
    {
        // Check for change
        if(strcmp((char *)puchLocation, (char *)m_uchLocation) == 0)
            return(FALSE);

        // Assume NULL termination and do a straight string copy
        strcpy((char *)m_uchLocation, (char *)puchLocation);
        m_ulLocationLength = strlen((char *)puchLocation);
    }


    return(TRUE);
}


/**
 *
 * Method Name:  GetLocation
 *
 *
 * Inputs:   None
 *
 * Outputs:  unsigned char *puchLocation
 *                  - buffer in which the LOCATION attribute shall be returned
 *
 * Returns:  unsigned long - Length of the item being returned in the buffer
 *
 * Description: Retrieves the Location attribute stored within the object and
 *              returns its associated length.
 *
 * Usage Notes: All character strings returned are limited to 255 bytes in
 *              length.  Any wide character support for internationalized
 *              display is a responsibility assumed by the caller.
 */
unsigned long CSourceDescription::GetLocation(unsigned char *puchLocation)
{

    // Copy the attribute contents into the output buffer passed
    if(puchLocation != NULL)
    {
        strcpy((char *)puchLocation, (char *)m_uchLocation);
        return(m_ulLocationLength);
    }

    return(0);

}

/**
 *
 * Method Name:  SetNotes
 *
 *
 * Inputs:   unsigned char  *puchNotes  - Notes character string
 *           unsigned long   ulLength   - Length of Notes argument passed
 *
 * Outputs:  None
 *
 * Returns:  bool
 *
 * Description: Stores the Notes field and length, either specified of derived,
 *              as attributed within the object.
 *
 * Usage Notes: The NOTES argument MUST be passed as a NULL terminated string
 *              or must contain a valid length argument.  All text strings
 *              passed shall be truncated beyond the length of 255 characters.
 *
 *
 */
bool CSourceDescription::SetNotes(unsigned char *puchNotes,
                                  unsigned long ulLength)
{

    // Check whether a valid length was passed
    if(ulLength)
    {
        // Check for change
        if(strncmp((char *)puchNotes, (char *)m_uchNotes, ulLength) == 0)
            return(FALSE);

        // Make sure length is no greater than MAX_LENGTH
        m_ulNotesLength =
                ulLength < MAX_SOURCE_LENGTH ? ulLength : MAX_SOURCE_LENGTH-1;
        strncpy((char *)m_uchNotes, (char *)puchNotes, m_ulNotesLength);
        m_uchNotes[m_ulNotesLength] = 0;
    }
    else if(puchNotes != NULL)
    {
        // Check for change
        if(strcmp((char *)puchNotes, (char *)m_uchNotes) == 0)
            return(FALSE);

        // Assume NULL termination and do a straight string copy
        strcpy((char *)m_uchNotes, (char *)puchNotes);
        m_ulNotesLength = strlen((char *)puchNotes);
    }

    return(TRUE);
}


/**
 *
 * Method Name:  GetNotes
 *
 *
 * Inputs:   None
 *
 * Outputs:  unsigned char *puchNotes
 *                     - buffer in which the Notes attribute shall be returned
 *
 * Returns:  unsigned long - Length of the item being returned in the buffer
 *
 * Description: Retrieves the Notes attribute stored within the object and
 *              returns its associated length.
 *
 * Usage Notes: All character strings returned are limited to 255 bytes in
 *              length.  Any wide character support for internationalized
 *              display is a responsibility assumed by the caller.
 *
 *
 */
unsigned long CSourceDescription::GetNotes(unsigned char *puchNotes)
{

    // Copy the attribute contents into the output buffer passed
    if(puchNotes != NULL)
    {
        strcpy((char *)puchNotes, (char *)m_uchNotes);
        return(m_ulNotesLength);
    }

    return(0);

}


/**
 *
 * Method Name:  SetPrivate
 *
 *
 * Inputs:   unsigned char  *puchNotes   - Private character string
 *           unsigned long   ulLength    - Length of Private argument passed
 *
 * Outputs:  None
 *
 * Returns:  bool
 *
 * Description: Stores the Private field and length, either specified of
 *              derived, as attributed within the object.
 *
 * Usage Notes: The PRIVATE argument MUST be passed as a NULL terminated
 *              string or must contain a valid length argument.  All text
 *              strings passed shall be truncated beyond the length of 255
 *              characters.
 *
 *
 */
bool CSourceDescription::SetPrivate(unsigned char *puchPrivate,
                                    unsigned long ulLength)
{

    // Check whether a valid length was passed
    if(ulLength)
    {
        // Check for change
        if(strncmp((char *)puchPrivate, (char *)m_uchPrivate, ulLength) == 0)
            return(FALSE);

        // Make sure length is no greater than MAX_LENGTH
        m_ulPrivateLength =
                ulLength < MAX_SOURCE_LENGTH ? ulLength : MAX_SOURCE_LENGTH-1;
        strncpy((char *)m_uchPrivate, (char *)puchPrivate, m_ulPrivateLength);
        m_uchPrivate[m_ulPrivateLength] = 0;
    }
    else if(puchPrivate != NULL)
    {
        // Check for change
        if(strcmp((char *)puchPrivate, (char *)m_uchPrivate) == 0)
            return(FALSE);

        // Assume NULL termination and do a straight string copy
        strcpy((char *)m_uchPrivate, (char *)puchPrivate);
        m_ulPrivateLength = strlen((char *)puchPrivate);
    }

    return(TRUE);
}

/**
 *
 * Method Name:  GetPrivate
 *
 *
 * Inputs:   None
 *
 * Outputs:  unsigned char *puchPrivate
 *                   - buffer in which the Private attribute shall be returned
 *
 * Returns:  unsigned long - Length of the item being returned in the buffer
 *
 * Description: Retrieves the Private attribute stored within the object and
 *              returns its associated length.
 *
 * Usage Notes: All character strings returned are limited to 255 bytes in
 *              length.  Any wide character support for internationalized
 *              display is a responsibility assumed by the caller.
 *
 *
 */
unsigned long CSourceDescription::GetPrivate(unsigned char *puchPrivate)
{

    // Copy the attribute contents into the output buffer passed
    if(puchPrivate != NULL)
    {
        strcpy((char *)puchPrivate, (char *)m_uchPrivate);
        return(m_ulPrivateLength);
    }

    return(0);

}


/**
 *
 * Method Name:  FormatSDESReport
 *
 *
 * Inputs:   boolean        bIncludeHeader - TRUE if header should be included
 *           long           lContentMask   - Content Mask
 *           unsigned long  ulBufferSize   - length allocated for the buffer
 *
 * Outputs:  unsigned char *puchReportBuffer
 *                      - Buffer used to store the contents of the SDES Report
 *
 * Returns:  unsigned long - number of octets written into the buffer.
 *
 * Description: Constructs an SDES report using the buffer passed in by the
 *              caller.  The Source Description object shall use the period
 *              count passed to determine which information should be used
 *              to populate an SDES report.
 *
 * Usage Notes: The header of the RTCP Report shall be formatted by delegating
 *              to the base class.
 *
 *
 */
unsigned long CSourceDescription::FormatSDESReport(bool bIncludeHeader,
                long lContentMask, unsigned char *puchReportBuffer,
                unsigned long ulBufferSize)
{
    unsigned char   *puchPayloadBuffer;
    unsigned long    ulReportLength;

    // Check whether the CNAME has been calculate yet.
    // If not, we shall do this as our first act.
    if(!m_bCNameSet)
    {
        FormulateCName();
    }

    // Check to see whether there is a header present or not.
    if(bIncludeHeader)
    {
        // Let's offset into the Formatting buffer enough to start depositing
        //  payload
        puchPayloadBuffer = puchReportBuffer + GetHeaderLength();
    }
    else
    {
        // No header. Deposit payload at the beginning of the buffer
        puchPayloadBuffer = puchReportBuffer;

        // Let's load the SSRC into the SDES Report
        *((unsigned long *)puchPayloadBuffer) = htonl(GetSSRC());
        puchPayloadBuffer += sizeof(unsigned long);
    }


    // Let's load the field information based upon the content mask
    puchPayloadBuffer += LoadFieldInfo(puchPayloadBuffer, lContentMask);

    // Let's load padding onto the end of the packet to insure 4 byte alignment
    bool bPadded;
    puchPayloadBuffer += TerminateNPad(puchPayloadBuffer, &bPadded);

    // Set the report length
    ulReportLength = puchPayloadBuffer - puchReportBuffer;

    // Let's check to see whether we need to prepend a header to this Receiver
    //  Report.  If so, let's call the RTCP Header base class's formatter.
    if(bIncludeHeader)
    {
        // Let's slap a header on this report
        FormatRTCPHeader((unsigned char *)
                  puchReportBuffer,   // RTCP Report Buffer
                  bPadded,            // Padding Flag
                  1,                  // Receiver Count set to 1 for now
                  ulReportLength);    // Report Length
    }

    return(ulReportLength);

}


/**
 *
 * Method Name:  ParseSDESReport
 *
 *
 * Inputs:   bool bIncludeHeader - TRUE if RTCP Header preceeds report
 *           unsigned char *puchReportBuffer
 *               - Character Buffer containing the contents of the SDES Report
 *
 * Outputs:  None
 *
 * Returns:  unsigned long - Number of octets processed
 *
 * Description: Extracts the contents of an SDES report using the buffer passed
 *              in by the caller.  The Source Description object shall store
 *              the content and length of data fields extracted from the SDES
 *              Report.
 *
 * Usage Notes: The header of the RTCP Report shall be parsed by delegating to
 *              the base class.
 *
 *
 */
unsigned long CSourceDescription::ParseSDESReport(
                          bool bIncludeHeader, unsigned char *puchReportBuffer)
{

    unsigned char    *puchPayloadBuffer = puchReportBuffer;
    unsigned long     ulFieldLength = 0;

    // Determine whether the header has been included.  If so, let's
    // extract the header information
    if(bIncludeHeader)
    {
        // Check whether the RTCP Header has been correctly
        //  formed (Version, etc...).
        if(!ParseRTCPHeader(puchReportBuffer))
            return(GetReportLength());

        // Good header.  Let's bump the payload pointer and continue.
        puchPayloadBuffer += GetHeaderLength();
    }
    else
    {
        SetSSRC(ntohl(*((long *)puchPayloadBuffer)));
        puchPayloadBuffer += sizeof(long);
    }

    // Let's parse the constituent components of the SDES list until a list
    //  terminator is encountered.  Also, be sure to reset the change mask so
    //  we know the differences between the last report and this report.
    m_ulContentMask = 0;
    m_ulChangeMask = 0;

    while((ulFieldLength = ExtractFieldInfo(puchPayloadBuffer)))
        puchPayloadBuffer += ulFieldLength;

    // Bump the pointer to account for the Terminator
    puchPayloadBuffer++;

    // Let's process any padding that might be present to align the
    //  payload on a 32 bit boundary.
    puchPayloadBuffer += ExtractPadding(puchPayloadBuffer);

    return(puchPayloadBuffer - puchReportBuffer);

}




/**
 *
 * Method Name:  FormulateCName
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     bool  - TRUE indicates successful formulation of the CNAME.
 *                      (actually, always returns TRUE!)
 *
 * Description: Formulates the CNAME attribute by concatenating the NAME field
 *              with the IP address of the Pingtel Network Phone.  The
 *              resultant string shall take the following form
 *              'name@128.224.120.2' or 'name@host.pingtel.com'.
 *
 * Usage Notes: This method shall be called once at object initialization.  The
 *              CNAME attribute formed will be used in all successive calls and
 *              may only be changed when a system reset occurs.
 *
 *
 */
bool CSourceDescription::FormulateCName(void)
{
    unsigned char uchHostName[MAX_ENTRYSIZE];

    // Let's get the hostname of this system so that we can form the CNAME
    if(gethostname((char *)uchHostName, MAX_ENTRYSIZE-1) != 0)
    {
        // Unable to retrieve hostname. Let's use a default hostname
        strcpy((char *)uchHostName, (char *)"unknownhost.unknowndomain.com");
    }

    // We will form the CNAME by taking the current SDES Name and concatenating
    //  the Host Name.  If the name is not set, we will use a default name
    if(m_ulNameLength == 0)
    {
        // Use a default name
        sprintf((char *)m_uchCName, "UnknownUser@%s", uchHostName);
    }

    // A valid Name component is present.  Let's use it instead
    else
    {
        // Form a CName with the SDES Name
        strcpy((char *)m_uchCName, (char *)m_uchName);
    }

    // Let's complete the CName by concatenating the host name to the end
    sprintf((char *)m_uchCName,
                             "%s@%s", (char *)m_uchName, (char *)uchHostName);

    // Let's store the length of this CName
    m_ulCNameLength = strlen((char *)m_uchCName);

    // Set the flag so this doesn't get recalculated
    m_bCNameSet = TRUE;

    return(TRUE);
}


/**
 *
 * Method Name:  ExtractFieldInfo
 *
 *
 * Inputs:   unsigned char *puchReportBuffer
 *                                         - Buffer containing the SDES Report
 *
 * Outputs:  None
 *
 * Returns:  unsigned long - Number of octets processed
 *
 * Description: Extracts the field information contents of an SDES report using
 *              the buffer passed in by the caller. Each field entry shall
 *              contain an octet field type, an octet field length, and
 *              'length' octets of character data not to exceed 255.
 *
 * Usage Notes:
 *
 *
 */
unsigned long
         CSourceDescription::ExtractFieldInfo(unsigned char *puchReportBuffer)
{
    bool bChanged;
    unsigned char *puchPayloadBuffer = puchReportBuffer;

    // Get the Field type and length located
    //  within the first 2 octets of the SDES chunk
    unsigned long ulFieldType   = (unsigned long)*puchPayloadBuffer++;
    unsigned long ulFieldLength = (unsigned long)*puchPayloadBuffer++;

    // Evaluate the field type to determine how we
    //  will process and store the information
    switch(ulFieldType)
    {
        // A Null terminator octer has been found.  We will return control to
        //  the caller so that it may strip off any padding.
        case TERMINATOR:
            return(0);

        // CNAME_ID field type found
        case CNAME_ID:
            m_bCNameSet = TRUE;
            bChanged = SetCName(puchPayloadBuffer, ulFieldLength);
            break;

        // NAME_ID field type found
        case NAME_ID:
            bChanged = SetName(puchPayloadBuffer, ulFieldLength);
            break;

        // EMAIL_ID field type found
        case EMAIL_ID:
            bChanged = SetEmail(puchPayloadBuffer, ulFieldLength);
            break;

        // PHONE_ID field type found
        case PHONE_ID:
            bChanged = SetPhone(puchPayloadBuffer, ulFieldLength);
            break;

        // LOCATION_ID field type found
        case LOCATION_ID:
            bChanged = SetLocation(puchPayloadBuffer, ulFieldLength);
            break;

        // APPNAME_ID field type found
        case APPNAME_ID:
            bChanged = SetAppName(puchPayloadBuffer, ulFieldLength);
            break;

        // NOTE_ID field type found
        case NOTE_ID:
            bChanged = SetNotes(puchPayloadBuffer, ulFieldLength);
            break;

        // PRIVATE_ID field type found
        case PRIVATE_ID:
            bChanged = SetPrivate(puchPayloadBuffer, ulFieldLength);
            break;

        default:
            return(puchPayloadBuffer + ulFieldLength - puchReportBuffer);

    }

    // Set the field content mask
    m_ulContentMask += (1 << (ulFieldType - 1));

    // set the changed mask as well if the field
    //  changed since the last report received.
    if(bChanged)
            m_ulChangeMask += (1 << (ulFieldType - 1));

    // Return the amount of data processed
    return(puchPayloadBuffer + ulFieldLength - puchReportBuffer);

}

/**
 *
 * Method Name:  GetFieldChange
 *
 *
 * Input:    unsigned long  ulChangeMask
 *                    - A mask identifying the changed field in an SDES Report
 *
 * Outputs:  unsigned long *pulFieldType
 *                               - The Field Identifier present in change mask
 *           unsigned char *puchReportBuffer
 *                               - Buffer to store contents of field
 *
 * Returns:  unsigned long - The modified change mask
 *
 * Description: Gets a field from an SDES Report based upon the change mask
 *              passed.  A field present within the change mask shall have its
 *              ID and field contents loaded as output arguments to this call.
 *              The change mask shall be modified to reflect the removal of the
 *              field change that is being returned.
 *
 * Usage Notes: This may be called multiple times to extract all the changes
 *              from an SDES report.  No more changes are available once the
 *              mask has a value of 0.
 *
 *
 */
unsigned long CSourceDescription::GetFieldChange(unsigned long ulChangeMask,
                 unsigned long *pulFieldType, unsigned char *puchFieldBuffer)
{

    // Loop through the list of field types to determine which
    //  are set within the change mask
    for(unsigned long ulField = 1; ulField <= PRIVATE_ID; ulField++)
    {
        // Check whether this Field bit is set within the change mask
        if(ulChangeMask & (1 << (ulField - 1)))
        {
            // One's compliment  this field from the mask
            ulChangeMask &= ~(1 << (ulField - 1));

            // Now load the arguments passed with
            //  the field type and the field buffer
            *pulFieldType = ulField;

            // Evaluate the field type to determine what to pass back
            switch(ulField)
            {

                // CNAME_ID field type found
                case CNAME_ID:
                    GetCName(puchFieldBuffer);
                    break;

                // NAME_ID field type found
                case NAME_ID:
                    GetName(puchFieldBuffer);
                    break;

                // EMAIL_ID field type found
                case EMAIL_ID:
                    GetEmail(puchFieldBuffer);
                    break;

                // PHONE_ID field type found
                case PHONE_ID:
                    GetPhone(puchFieldBuffer);
                    break;

                // LOCATION_ID field type found
                case LOCATION_ID:
                    GetLocation(puchFieldBuffer);
                    break;

                // APPNAME_ID field type found
                case APPNAME_ID:
                    GetAppName(puchFieldBuffer);
                    break;

                // NOTE_ID field type found
                case NOTE_ID:
                    GetNotes(puchFieldBuffer);
                    break;

                // PRIVATE_ID field type found
                case PRIVATE_ID:
                    GetPrivate(puchFieldBuffer);
                    break;
            }

            break;
        }

    }

    // Return the modified change mask
    return(ulChangeMask);

}


/**
 *
 * Method Name:  ExtractPadding
 *
 *
 * Inputs:   unsigned char *puchReportBuffer - Buffer containing the SDES Report
 *
 * Outputs:  None
 *
 * Returns:  unsigned long - Number of octet processed
 *
 * Description: Extracts the padding that might be present at the end of a
 *              list of field data contained within an SDES report.
 *
 * Usage Notes:
 *
 *
 */
unsigned long
           CSourceDescription::ExtractPadding(unsigned char *puchReportBuffer)
{
    unsigned char *puchPayloadBuffer = puchReportBuffer;

    // The last entry at the end of the list will be
    //  padded out to a 4 byte boundary
    while(((unsigned long)puchPayloadBuffer) % 4)
        puchPayloadBuffer++;

    return(puchPayloadBuffer - puchReportBuffer);

}


/**
 *
 * Method Name: LoadFieldInfo
 *
 *
 * Inputs:   unsigned char *puchReportBuffer
 *                         - Buffer containing the SDES Report
 *           long lContentMask
 *                         - The content mask used to populate the SDES Report
 *
 * Outputs:  None
 *
 * Returns:  unsigned long - Number of octets processed
 *
 * Description: Loads the field information contents of an SDES report into the
 *              buffer passed by the caller. Each field entry shall contain an
 *              octet field type, an octet field length, and 'length' octets of
 *              character data not to exceed 255.  The CNAME field will always
 *              be passed as part of the report along with another field
 *              element determined from the period count.
 *
 * Usage Notes:
 *
 *
 */
unsigned long CSourceDescription::LoadFieldInfo(unsigned char *puchReportBuffer,
                                                long          lContentMask)
{

    unsigned char   uchFieldBuffer[MAX_SOURCE_LENGTH];
    unsigned char   *puchPayloadBuffer = puchReportBuffer;
    unsigned long   ulEntryLength = 0;

    // Check the content mask to determine whether we are only interested in
    //  the changes to the SDES report since the last received.
    if(lContentMask == SDES_CHANGES)
        LoadFieldChanges(puchReportBuffer);


    // Load the CNAME field into the report buffer
    *puchPayloadBuffer++ = (unsigned char)CNAME_ID;
    ulEntryLength = GetCName(uchFieldBuffer);
    *puchPayloadBuffer++ = (unsigned char)ulEntryLength;
    strncpy((char *)puchPayloadBuffer, (char *)uchFieldBuffer, ulEntryLength);
    puchPayloadBuffer += ulEntryLength;
    m_ulChangeMask = (1 << (CNAME_ID - 1));

    // Let's use the content mask to determine whether we should add additional
    //  fields of information with the CNAME and which field it should be.
    if(lContentMask % REPORT_CYCLES == 0)
    {
        // Let's send a piece of information to accompany the CNAME field
        unsigned long ulItem =
                        (((lContentMask - REPORT_CYCLES)/ REPORT_CYCLES) %
                                                         (PRIVATE_ID - 1)) + 2;

        m_ulChangeMask += (1 << (ulItem - 1));

        // Add the field Type to the buffer
        *puchPayloadBuffer++ = (unsigned char)ulItem;

        // Evaluate the item and add the corresponding field
        //  information to the payload buffer
        switch(ulItem)
        {

            // NAME_ID field type found
            case NAME_ID:
                ulEntryLength = GetName(uchFieldBuffer);
                break;

            // EMAIL_ID field type found
            case EMAIL_ID:
                ulEntryLength = GetEmail(uchFieldBuffer);
                break;

            // PHONE_ID field type found
            case PHONE_ID:
                ulEntryLength = GetPhone(uchFieldBuffer);
                break;

            // LOCATION_ID field type found
            case LOCATION_ID:
                ulEntryLength = GetLocation(uchFieldBuffer);
                break;

            // APPNAME_ID field type found
            case APPNAME_ID:
                ulEntryLength = GetAppName(uchFieldBuffer);
                break;

            // NOTE_ID field type found
            case NOTE_ID:
                ulEntryLength = GetNotes(uchFieldBuffer);
                break;

            // PRIVATE_ID field type found
            case PRIVATE_ID:
                ulEntryLength = GetPrivate(uchFieldBuffer);
                break;

            default:
                break;

        }

        // Load the field item in payload buffer
        *puchPayloadBuffer++ = (unsigned char)ulEntryLength;
        strncpy((char *)puchPayloadBuffer,
                                       (char *)uchFieldBuffer, ulEntryLength);
        puchPayloadBuffer += ulEntryLength;

    }


    // Return the amount of data processed
    return(puchPayloadBuffer - puchReportBuffer);

}

/**
 *
 * Method Name: LoadFieldChanges
 *
 *
 * Inputs:   unsigned char *puchReportBuffer - Buffer containing the SDES Report
 *
 * Outputs:  None
 *
 * Returns:  unsigned long - Number of octets processed
 *
 * Description: Loads the field information contents of an SDES report into the
 *              buffer passed by the caller. Each field entry shall contain an
 *              octet field type, an octet field length, and 'length' octets of
 *              character data not to exceed 255.  The field contents will be
 *              determined by the change mask set at the last SDES report
 *              reception.
 *
 * Usage Notes:
 *
 *
 */
unsigned long
         CSourceDescription::LoadFieldChanges(unsigned char *puchReportBuffer)
{

    unsigned char   uchFieldBuffer[MAX_SOURCE_LENGTH];
    unsigned char   *puchPayloadBuffer = puchReportBuffer;
    unsigned long   ulEntryLength = 0;


    // Load the CNAME field into the report buffer
    *puchPayloadBuffer++ = (unsigned char)CNAME_ID;
    ulEntryLength = GetCName(uchFieldBuffer);
    *puchPayloadBuffer++ = (unsigned char)ulEntryLength;
    strncpy((char *)puchPayloadBuffer, (char *)uchFieldBuffer, ulEntryLength);
    puchPayloadBuffer += ulEntryLength;

    // Let's use the content mask to determine whether we should add
    //  additional fields of information with the CNAME and which field
    //  it should be.
    for(int iCount = 0; iCount <= PRIVATE_ID && m_ulContentMask; iCount++)
    {
        // Let's see if the field element is part of the change mask.  If not
        //  present, loop again until the field possibilities are exhausted.
        if(!(m_ulContentMask & (1 << iCount)))
            continue;

        // Add the field Type to the buffer
        *puchPayloadBuffer++ = (unsigned char)iCount+1;

        // Evaluate the item and add the corresponding field
        //  information to the payload buffer
        switch(iCount+1)
        {

            // NAME_ID field type found
            case NAME_ID:
                ulEntryLength = GetName(uchFieldBuffer);
                break;

            // EMAIL_ID field type found
            case EMAIL_ID:
                ulEntryLength = GetEmail(uchFieldBuffer);
                break;

            // PHONE_ID field type found
            case PHONE_ID:
                ulEntryLength = GetPhone(uchFieldBuffer);
                break;

            // LOCATION_ID field type found
            case LOCATION_ID:
                ulEntryLength = GetLocation(uchFieldBuffer);
                break;

            // APPNAME_ID field type found
            case APPNAME_ID:
                ulEntryLength = GetAppName(uchFieldBuffer);
                break;

            // NOTE_ID field type found
            case NOTE_ID:
                ulEntryLength = GetNotes(uchFieldBuffer);
                break;

            // PRIVATE_ID field type found
            case PRIVATE_ID:
                ulEntryLength = GetPrivate(uchFieldBuffer);
                break;

            default:
                break;

        }

        // Load the field item in payload buffer
        *puchPayloadBuffer++ = (unsigned char)ulEntryLength;
        strncpy((char *)puchPayloadBuffer,
                                       (char *)uchFieldBuffer, ulEntryLength);
        puchPayloadBuffer += ulEntryLength;

    }


    // Return the amount of data processed
    return(puchPayloadBuffer - puchReportBuffer);

}

/**
 *
 * Method Name:  TerminateNPad
 *
 *
 * Inputs:   unsigned char *puchReportBuffer - Buffer containing the SDES Report
 *
 * Outputs:  bool *pbPadded - TRUE => padding was added
 *
 * Returns:  unsigned long  - Number of octets processed
 *
 * Description: Add a terminating NULL octet and pad out to a 4 byte boundary.
 *
 * Usage Notes:
 *
 *
 */
unsigned long
    CSourceDescription::TerminateNPad(unsigned char *puchReportBuffer,
                                      bool *pbPadded)
{
    unsigned char *puchPayloadBuffer = puchReportBuffer;
    unsigned char numPadBytes;

    // Add the terminating NULL octet
    *puchPayloadBuffer++ = 0;

    // Add padding as needed to get us aligned on a 4 byte boundary
    numPadBytes = (unsigned char) ((4 - (((unsigned long)puchPayloadBuffer) % 4)) % 4);
    switch (numPadBytes) {
    case 3:
        *puchPayloadBuffer++ = 0;
    case 2:
        *puchPayloadBuffer++ = 0;
    case 1:
        *puchPayloadBuffer++ = numPadBytes;
        *pbPadded = TRUE;
        break;
    case 0:
        *pbPadded = FALSE;
        break;
    }

    return(puchPayloadBuffer - puchReportBuffer);

}
#endif /* INCLUDE_RTCP ] */
