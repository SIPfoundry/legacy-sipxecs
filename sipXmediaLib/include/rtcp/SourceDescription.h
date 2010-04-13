//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


//  Border Guard
#ifndef _SourceDescription_h
#define _SourceDescription_h

#include "rtcp/RtcpConfig.h"

//  Includes
#include "BaseClass.h"
#include "RTCPHeader.h"
#include "ISDESReport.h"
#include "ISetSrcDescription.h"
#include "IGetSrcDescription.h"


/**
 *
 * Class Name:  CSourceDescription
 *
 * Inheritance: CBaseClass          - Base Class Implementation
 *              CBaseRTCPClass      - RTCP Reporting Base Class Implementation
 *
 *
 * Interfaces:  ISDESReport,        - SDES Reporting interface
 *              ISetSrcDescription  - Set Source Description Interface
 *              IGetSrcDescription  - Get Source Description Interface
 *
 * Description: The CSourceDescription Class manages source description
 *              information passed by called participants through RTCP Source
 *              Description (SDES) Reports while in a VOIP call.
 *
 * Notes:       CSourceDescription is derived from CBaseClass which provides
 *              basic Initialization and reference counting support.
 *
 */
class CSourceDescription:
          public CBaseClass,        // Inherits the CBaseClass implementation
          public CRTCPHeader,       // Inherits the CRTCPHeader implementation
          public ISDESReport,       // Interface exposed for SDES Reporting
          public ISetSrcDescription,
                        // Interface exposed for modifying Source Description
          public IGetSrcDescription
                        // Interface exposed for retrieving Source Description
 {

//  Public Methods
public:

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
    CSourceDescription(unsigned long ulSSRC=0,
                       unsigned char *puchName=NULL,
                       unsigned char *puchEmail=NULL,
                       unsigned char *puchPhone=NULL,
                       unsigned char *puchLocation=NULL,
                       unsigned char *puchAppName=NULL,
                       unsigned char *puchNotes=NULL,
                       unsigned char *puchPrivate=NULL);

/**
 *
 * Method Name:  CSourceDescription() - Constructor
 *
 *
 * Inputs:      bool bHeader
 *                      - TRUE indicates a header precedes the SDES field info
 *              unsigned char *puchSDESReport
 *                           - An SDES Report received from a call participant
 *
 * Outputs:     None
 *
 * Returns:     None
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
    CSourceDescription(bool bHeader, unsigned char *puchSDESReport=NULL);


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
 * Description: Shall deallocate and/or release all resources which was
 *              acquired over the course of runtime.
 *
 * Usage Notes:
 *
 *
 */
    ~CSourceDescription(void);


/**
 *
 * Method Name:  GetLocalSDES()
 *
 * Inputs:       None
 *
 * Outputs:      None
 *
 * Returns:      CSourceDescription *poSourceDescription
 *                       - Pointer to Local Source Description Interface
 *
 * Description:  A static member function used ot obtain an Source Description
 *               interface.
 *
 * Usage Notes:  This method shall cause the local Source Description Singleton
 *               object to be created if it has not already been instantiated.
 *
 */
    static CSourceDescription *GetLocalSDES(void);
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
    void SetAllComponents(unsigned char *puchName=NULL,
                          unsigned char *puchEmail=NULL,
                          unsigned char *puchPhone=NULL,
                          unsigned char *puchLocation=NULL,
                          unsigned char *puchAppName=NULL,
                          unsigned char *puchNotes=NULL,
                          unsigned char *puchPrivate=NULL);

/**
 *
 * Method Name:  GetAllComponents()
 *
 *
 * Inputs:      unsigned char          *puchName         - NAME field
 *              unsigned char          *puchEmail        - EMAIL field
 *              unsigned char          *puchPhone        - PHONE field
 *              unsigned char          *puchAppName      - APPLICATION NAME
 *              unsigned char          *puchLocation     - LOCATION field
 *              unsigned char          *puchNotes        - NOTES field
 *              unsigned char          *puchPrivate      - PRIVATE field
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: Performs a bulk get upon all the constituent elements composing
 *              a Source Description.
 *
 * Usage Notes: The elements retrieved are NULL terminated.
 *
 */
    void GetAllComponents(unsigned char *puchName,
                          unsigned char *puchEmail,
                          unsigned char *puchPhone,
                          unsigned char *puchLocation,
                          unsigned char *puchAppName,
                          unsigned char *puchNotes,
                          unsigned char *puchPrivate);

/**
 *
 * Method Name:  GetFieldChange
 *
 *
 * Input:       unsigned long  ulChangeMask
 *                    - A mask identifying the changed field in an SDES Report
 *
 * Outputs:     unsigned long *pulFieldType
 *                               - The Field Identifier present in change mask
 *              unsigned char *puchReportBuffer
 *                               - Character Buffer to store contents of field
 *
 * Returns:     unsigned long - The modified change mask
 *
 * Description: Gets a field from an SDES Report based upon the change mask
 *              passed.  A field present within the change mask shall have its
 *              ID and field contents loaded as output arguments to this call.
 *              The change mask shall be modified to reflect the removal of
 *              the field change that is being returned.
 *
 * Usage Notes: This may be called multiple times to extract all the changes
 *              from an SDES report.  No more changes are available once the
 *              mask has a value of 0.
 *
 *
 */
    unsigned long GetFieldChange(unsigned long ulChangeMask,
                                 unsigned long *pulFieldType,
                                 unsigned char *puchFieldBuffer);


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
    virtual unsigned long GetSSRC(void);

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
    virtual void SetSSRC(unsigned long ulSSRC);


/**
 *
 * Method Name:  SetName
 *
 *
 * Inputs:   unsigned char *puchName - NAME Character String
 *           unsigned long  ulLength - Optional Length of NAME argument passed
 *
 * Outputs:  None
 *
 * Returns:  bool
 *
 * Description: Stores the Name field and length, either specified of derived,
 *              as attributed within the object.
 *
 * Usage Notes: The NAME argument MUST be passed as a NULL terminated string or
 *              must contain a valid length argument. All text strings passed
 *              shall be truncated beyond the length of 255 characters.
 *
 *
 *
 */
    bool SetName(unsigned char *puchCName, unsigned long ulLength=0);


/**
 *
 * Method Name:  GetName
 *
 *
 * Inputs:      None
 *
 * Outputs:     unsigned char *puchName
 *                      - buffer in which the NAME attribute shall be returned
 *
 * Returns:     unsigned long
 *                      - Length of the item being returned in the buffer
 *
 * Description: Retrieves the Name attribute stored within the object and
 *              returns its associated length.
 *
 * Usage Notes: All character strings returned are limited to 255 bytes in
 *              length.  Any wide character support for internationalized
 *              display is a responsibility assumed by the caller.
 *
 *
 */
    unsigned long GetName(unsigned char *puchName);


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
 *              or must contain a valid length argument. All text strings
 *              passed shall be truncated beyond the length of 255 characters.
 *
 *
 */
    bool SetEmail(unsigned char *puchEmail, unsigned long ulLength=0);


/**
 *
 * Method Name:  GetEmail
 *
 *
 * Inputs:      None
 *
 * Outputs:     unsigned char *puchEmail
 *                     - buffer in which the EMAIL attribute shall be returned
 *
 * Returns:     unsigned long - Length of the item returned in the buffer
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
    unsigned long GetEmail(unsigned char *puchEmail);


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
    bool SetPhone(unsigned char *puchPhone, unsigned long ulLength=0);


/**
 *
 * Method Name:  GetPhone
 *
 *
 * Inputs:      None
 *
 * Outputs:     unsigned char *puchPhone
 *                     - buffer in which the PHONE attribute shall be returned
 *
 * Returns:     unsigned long - Length of the item returned in the buffer
 *
 * Description: Retrieves the Phone attribute stored within the object and
 *              returns its associated length.
 *
 * Usage Notes: All character strings returned are limited to 255 bytes in
 *              length.  Any wide character support for internationalized
 *              display is a responsibility assumed by the caller.
 *
 *
 */
    unsigned long GetPhone(unsigned char *puchPhone);


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
 *
 *
 */
    bool SetAppName(unsigned char *puchAppName, unsigned long ulLength=0);


/**
 *
 * Method Name:  GetAppName
 *
 *
 * Inputs:      None
 *
 * Outputs:     unsigned char *puchAppName
 *          - buffer in which the APPLICATION NAME attribute shall be returned
 *
 * Returns:     unsigned long - Length of the item returned in the buffer
 *
 * Description: Retrieves the Application Name attribute stored within the
 *              object and returns its associated length.
 *
 * Usage Notes: All character strings returned are limited to 255 bytes in
 *              length.  Any wide character support for internationalized
 *              display is a responsibility assumed by the caller.
 *
 *
 */
    unsigned long GetAppName(unsigned char *puchAppName);



/**
 *
 * Method Name:  SetLocation
 *
 *
 * Inputs:   unsigned char  *puchLocation - LOCATION character string
 *           unsigned long   ulLength     - Length of LOCATION argument passed
 *
 * Outputs:  None
 *
 * Returns:  bool
 *
 * Description: Stores the Location field and length, either specified of
 *              derived, as attributed within the object.
 *
 * Usage Notes: The LOCATION argument MUST be passed as a NULL terminated
 *              string or must contain a valid length argument. All text
 *              strings passed shall be truncated beyond the length of 255
 *              characters.
 *
 *
 */
    bool SetLocation(unsigned char *puchLocation, unsigned long ulLength=0);


/**
 *
 * Method Name:  GetLocation
 *
 *
 * Inputs:      None
 *
 * Outputs:     unsigned char *puchLocation
 *                  - buffer in which the LOCATION attribute shall be returned
 *
 * Returns:     unsigned long - Length of the item returned in the buffer
 *
 * Description: Retrieves the Location attribute stored within the object and
 *              returns its associated length.
 *
 * Usage Notes: All character strings returned are limited to 255 bytes in
 *              length.  Any wide character support for internationalized
 *              display is a responsibility assumed by the caller.
 *
 *
 */
    unsigned long GetLocation(unsigned char *puchLocation);


/**
 *
 * Method Name:  SetNotes
 *
 *
 * Inputs:      unsigned char  *puchNotes  - NOTES character string
 *              unsigned long   ulLength   - Length of NOTES argument passed
 *
 * Outputs:     None
 *
 * Returns:     bool
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
    bool SetNotes(unsigned char *puchNotes, unsigned long ulLength=0);


/**
 *
 * Method Name:  GetNotes
 *
 *
 * Inputs:      None
 *
 * Outputs:     unsigned char *puchNotes
 *                     - buffer in which the NOTES attribute shall be returned
 *
 * Returns:     unsigned long - Length of the item returned in the buffer
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
    unsigned long GetNotes(unsigned char *puchNotes);

/**
 *
 * Method Name:  SetPrivate
 *
 *
 * Inputs:      unsigned char *puchNotes  - PRIVATE character string
 *              unsigned long  ulLength   - Length of PRIVATE argument passed
 *
 * Outputs:     None
 *
 * Returns:     bool
 *
 * Description: Stores the Private field and length, either specified of
 *              derived, as attributed within the object.
 *
 * Usage Notes: The PRIVATE argument MUST be passed as a NULL terminated string
 *              or must contain a valid length argument.  All text strings
 *              passed shall be truncated beyond the length of 255 characters.
 *
 *
 */
    bool SetPrivate(unsigned char *puchPrivate, unsigned long ulLength=0);


/**
 *
 * Method Name:  GetPrivate
 *
 *
 * Inputs:      None
 *
 * Outputs:     unsigned char *puchPrivate
 *                   - buffer in which the PRIVATE attribute shall be returned
 *
 * Returns:     unsigned long - Length of the item returned in the buffer
 *
 * Description: Retrieves the PRIVATE attribute stored within the object and
 *              returns its associated length.
 *
 * Usage Notes: All character strings returned are limited to 255 bytes in
 *              length.  Any wide character support for internationalized
 *              display is a responsibility assumed by the caller.
 *
 *
 */
    unsigned long GetPrivate(unsigned char *puchNotes);

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
    ISDESReport * GetSDESInterface(void);

/**
 *
 * Method Name:  GetAccessInterface()
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     IGetSrcDescription *
 *                              - Pointer to the Get Src Description Interface
 *
 * Description: Returns a pointer to the IGetSrcDescription interface used to
 *              view the contents of an SDES Report.
 *
 * Usage Notes:
 *
 */
    IGetSrcDescription *GetAccessInterface(void);

/**
 *
 * Method Name:  GetChanges
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long - The aggregate changes that have occurred in
 *                              the report since the last period
 *
 * Description: Retrieves the aggregate changes that have occurred in the
 *              report since the last period
 *
 * Usage Notes: Mask is an OR'ing of all changes for a period.
 *
 *
 */
    unsigned long GetChanges(void);


/**
 *
 * Method Name:  FormatSDESReport
 *
 *
 * Inputs:   boolean        bHeader
 *                             - TRUE indicates a header should be included
 *           long           lContentMask     - Content Mask
 *           unsigned long  ulBufferSize     - length of the buffer
 *
 * Outputs:  unsigned char *puchReportBuffer
 *                      - Buffer to receive the contents of the SDES Report
 *
 * Returns:  unsigned long  - number of octets written into the buffer.
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
unsigned long FormatSDESReport(bool bHeader,
                               long lContentMask,
                               unsigned char *puchReportBuffer,
                               unsigned long ulBufferSize);


/**
 *
 * Method Name:  ParseSDESReport
 *
 *
 * Inputs:   bool bHeader - TRUE indicates an RTCP Header preceeds the report
 *           unsigned char *puchReportBuffer
 *                                        - Buffer containing the SDES Report
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
    unsigned long ParseSDESReport(bool bHeader,
                                  unsigned char *puchReportBuffer);



   private:     // Private Methods

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
 * Usage Notes: This method shall be called once at object initialization.
 *              The CNAME attribute formed will be used in all successive
 *              calls and may only be changed when a system reset occurs.
 *
 *
 */
    bool FormulateCName(void);


/**
 *
 * Method Name:  SetCName
 *
 *
 * Inputs:      unsigned char  *puchCName  - CNAME Character String
 *              unsigned long   ulLength   - Length of CNAME argument passed
 *
 * Outputs:     None
 *
 * Returns:     bool
 *
 * Description: Stores the CName field and length, either specified of derived,
 *              as attributed within the object.
 *
 * Usage Notes: The CNAME argument MUST be passed as a NULL terminated string
 *              or must contain a valid length argument. All text strings
 *              passed shall be truncated beyond the length of 255 characters.
 *
 *
 *
 */
    bool SetCName(unsigned char *puchCName, unsigned long ulLength=0);


/**
 *
 * Method Name:  GetCName
 *
 *
 * Inputs:      None
 *
 * Outputs:     unsigned char *puchCName
 *                     - buffer in which the CNAME attribute shall be returned
 *
 * Returns:     unsigned long - Length of the item returned in the buffer
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
    unsigned long GetCName(unsigned char *puchCName);

/**
 *
 * Method Name:  ExtractFieldInfo
 *
 *
 * Inputs:      unsigned char *puchReportBuffer
 *                         - Buffer containing the contents of the SDES Report
 *
 * Outputs:     None
 *
 * Returns:     unsigned long - Number of octets processed
 *
 * Description: Extracts the field information contents of an SDES report using
 *              the buffer passed in by the caller.  Each field entry shall
 *              contain an octet field type, an octet field length, and
 *              'length' octets of character data not to exceed 255.
 *
 * Usage Notes:
 *
 *
 */
    unsigned long ExtractFieldInfo(unsigned char *puchReportBuffer);


/**
 *
 * Method Name:  ExtractPadding
 *
 *
 * Inputs:      unsigned char *puchReportBuffer
 *                         - Buffer containing the contents of the SDES Report
 *
 * Outputs:     None
 *
 * Returns:     unsigned long - Number of octet processed
 *
 * Description: Extracts the padding that might be present at the end of a list
 *              of field data contained within an SDES report.
 *
 * Usage Notes:
 *
 *
 */
    unsigned long ExtractPadding(unsigned char *puchReportBuffer);

/**
 *
 * Method Name: LoadFieldInfo
 *
 *
 * Inputs:      unsigned char *puchReportBuffer
 *                             - Buffer containing the contents of SDES Report
 *              long lContentMask
 *                - Content mask used to determine what to include in a report
 *
 * Outputs:     None
 *
 * Returns:     unsigned long - Number of octets processed
 *
 * Description: Loads the field information contents of an SDES report into the
 *              buffer passed by the caller.  Each field entry shall contain an
 *              octet field type, an octet field length, and 'length' octets of
 *              character data not to exceed 255.  The CNAME field will always
 *              be passed as part of the report along with another field
 *              element determined from the period count.
 *
 * Usage Notes:
 *
 *
 */
    unsigned long LoadFieldInfo(unsigned char *puchReportBuffer,
                                long lContentMask);

/**
 *
 * Method Name: LoadFieldChanges
 *
 *
 * Inputs:   unsigned char *puchReportBuffer
 *                                         - Buffer containing the SDES Report
 *
 * Outputs:  None
 *
 * Returns:  unsigned long - Number of octets processed
 *
 * Description: Loads the field information contents of an SDES report into the
 *              buffer passed by the caller.  Each field entry shall contain an
 *              octet field type, an octet field length, and 'length' octets of
 *              character data not to exceed 255.  The field contents will be
 *              determined by the change mask set at the last SDES report
 *              reception.
 *
 * Usage Notes:
 *
 *
 */
    unsigned long LoadFieldChanges(unsigned char *puchReportBuffer);


/**
 *
 * Method Name:  TerminateNPad
 *
 *
 * Inputs:   unsigned char *puchReportBuffer
 *                            - Buffer containing the SDES Report
 *
 * Outputs:  bool *pbPadded   - Flag specifying whether padding was added
 *
 * Returns:  unsigned long    - Number of octets processed
 *
 * Description: Add a terminating NULL octet and pad out to a 4 byte boundary.
 *
 * Usage Notes:
 *
 *
 */
    unsigned long TerminateNPad(unsigned char *puchReportBuffer,
                                bool *pbPadded);

/**
 *
 * Macro Name:  DECLARE_IBASE_M
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: This implements the IBaseClass functions used and exposed by
 *              derived classes.
 *
 * Usage Notes:
 *
 *
 */
DECLARE_IBASE_M


private:        // Private Data Members

/**
 *
 * Attribute Name:  m_spoLocalSDES
 *
 * Type:            static CSourceDescription *
 *
 * Description:  This static member points to the RTC Manager Singleton object.
 *
 */
      static CSourceDescription *m_spoLocalSDES;

/**
 *
 * Attribute Name:  m_bCNameSet
 *
 * Type:            bool
 *
 * Description:  This member indicates whether the CNAME has been calculated
 *               since the instantiation of this object.
 *
 */
      bool          m_bCNameSet;

/**
 *
 * Attribute Name:  m_ulContentMask
 *
 * Type:            unsigned long
 *
 * Description:  This member holds a mask of the contents of a Source
 *               Description report.
 *
 */
      unsigned long m_ulContentMask;


/**
 *
 * Attribute Name:  m_ulChangeMask
 *
 * Type:            unsigned long
 *
 * Description:     This member shall store the changes the occur between
 *                  Source Description report repections.
 *   WHAT?????
 *
 */
      unsigned long m_ulChangeMask;

/**
 *
 * Attribute Name:  m_ulCNameLength
 *
 * Type:            unsigned long
 *
 * Description:  This member holds the length of the CNAME Attribute.
 *
 */
      unsigned long m_ulCNameLength;


/**
 *
 * Attribute Name:  m_uchCName
 *
 * Type:            Character Array
 *
 * Description:  This member holds the CNAME field of a Source Description.
 *
 */
      unsigned char m_uchCName[MAX_SOURCE_LENGTH];

/**
 *
 * Attribute Name:  m_ulNameLength
 *
 * Type:            unsigned long
 *
 * Description:  This member holds the length of the NAME Attribute.
 *
 */
      unsigned long m_ulNameLength;

/**
 *
 * Attribute Name:  m_uchName
 *
 * Type:            Character Array
 *
 * Description:  This member holds the NAME field of a Source Description.
 *
 */
      unsigned char m_uchName[MAX_SOURCE_LENGTH];


/**
 *
 * Attribute Name:  m_ulEmailLength
 *
 * Type:            unsigned long
 *
 * Description:  This member holds the length of the EMAIL Attribute.
 *
 */
      unsigned long  m_ulEmailLength;

/**
 *
 * Attribute Name:  m_uchEmail
 *
 * Type:            Character Array
 *
 * Description:  This member holds the EMAIL field of a Source Description.
 *
 */
      unsigned char m_uchEmail[MAX_SOURCE_LENGTH];


/**
 *
 * Attribute Name:  m_ulPhoneLength
 *
 * Type:            unsigned long
 *
 * Description:  This member holds the length of the PHONE Attribute.
 *
 */
      unsigned long  m_ulPhoneLength;

/**
 *
 * Attribute Name:  m_uchPhone
 *
 * Type:            Character Array
 *
 * Description:  This member holds the PHONE field of a Source Description.
 *
 */
      unsigned char m_uchPhone[MAX_SOURCE_LENGTH];

/**
 *
 * Attribute Name:  m_ulAppNameLength
 *
 * Type:            unsigned long
 *
 * Description:  This member holds the length of the APP NAME Attribute.
 *
 */
      unsigned long  m_ulAppNameLength;

/**
 *
 * Attribute Name:  m_uchAppName
 *
 * Type:            Character Array
 *
 * Description:  This member holds the APP NAME field of a Source Description.
 *
 */
      unsigned char m_uchAppName[MAX_SOURCE_LENGTH];

/**
 *
 * Attribute Name:  m_ulLocationLength
 *
 * Type:            unsigned long
 *
 * Description:  This member holds the length of the LOCATION Attribute.
 *
 */
      unsigned long  m_ulLocationLength;

/**
 *
 * Attribute Name:  m_uchLocation
 *
 * Type:            Character Array
 *
 * Description:  This member holds the LOCATION field of a Source Description.
 *
 */
      unsigned char m_uchLocation[MAX_SOURCE_LENGTH];

/**
 *
 * Attribute Name:  m_ulNotesLength
 *
 * Type:            unsigned long
 *
 * Description:  This member holds the length of the NOTES Attribute.
 *
 */
      unsigned long  m_ulNotesLength;

/**
 *
 * Attribute Name:  m_uchNotes
 *
 * Type:            Character Array
 *
 * Description:  This member holds the NOTES field of a Source Description.
 *
 */
      unsigned char m_uchNotes[MAX_SOURCE_LENGTH];

/**
 *
 * Attribute Name:  m_ulPrivateLength
 *
 * Type:            unsigned long
 *
 * Description:  This member holds the length of the PRIVATE Attribute.
 *
 */
      unsigned long  m_ulPrivateLength;

/**
 *
 * Attribute Name:  m_uchPrivate
 *
 * Type:            Character Array
 *
 * Description:  This member holds the PRIVATE field of a Source Description.
 *
 */
      unsigned char m_uchPrivate[MAX_SOURCE_LENGTH];

};

/**
 *
 * Method Name:  GetChanges
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long - The aggregate changes that have occurred in
 *                              the report since the last period
 *
 * Description: Retrieves the aggregate changes that have occurred in the
 *              report since the last period
 *
 * Usage Notes: Mask is an OR'ing of all changes for a period.
 *
 *
 */
inline unsigned long CSourceDescription::GetChanges(void)
{

    return(m_ulChangeMask);

}

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
inline ISDESReport * CSourceDescription::GetSDESInterface(void)
{
    ((ISDESReport *)this)->AddRef();
    return((ISDESReport *)this);

}

/**
 *
 * Method Name:  GetAccessInterface()
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     IGetSrcDescription *
 *                              - Pointer to the Get Src Description Interface
 *
 * Description: Returns a pointer to the IGetSrcDescription interface used to
 *              view the contents of an SDES Report.
 *
 * Usage Notes:
 *
 */
inline IGetSrcDescription *CSourceDescription::GetAccessInterface(void)
{

    ((IGetSrcDescription *)this)->AddRef();
    return((IGetSrcDescription *)this);

}


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
inline unsigned long CSourceDescription::GetSSRC(void)
{

    return(CRTCPHeader::GetSSRC());

}

#endif
