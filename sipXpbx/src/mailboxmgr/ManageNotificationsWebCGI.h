//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef ManageNotificationsWebCGI_H
#define ManageNotificationsWebCGI_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "mailboxmgr/CGICommand.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlHashMap;

/**
 * ManageNotificationsWebCGI Class
 *
 * @author Harippriya M Sivapatham
 * @version 1.0
 */
class ManageNotificationsWebCGI : public CGICommand
{
public:
    /**
     * Ctor
     */
    ManageNotificationsWebCGI ( const UtlString& mailboxIdentity,
                                const UtlString& action,
                                const UtlString& status,
                                const UtlString& contactAddress,
                                const UtlString& contactType,
                                const UtlString& newContactAddress,
                                const UtlString& newContactType,
                                const UtlString& sendAttachments
                              );

    /**
     * Virtual Dtor
     */
    virtual ~ManageNotificationsWebCGI();

    /** This does the work */
    virtual OsStatus execute ( UtlString* out = NULL );

    /**
     *  Code for displaying the main webpage listing all the contact addresses,
     *  their type, link for editing/deleting existing contacts and
     *  adding new contacts.
     *
     */
    OsStatus getManageNotificationsUI(UtlString* out) ;

    /**
     *  Constructs the HTML code for displaying an individual row on the main page
     *  for the specified contact.
     *
     *  @param  contactAddress  Content for the field 'Contact'
     *  @param  contactType     Content for the field 'Type'.
     *  @param  htmlCode        Html code for displaying the row. Filled on return
     *
     *  @return OS_SUCCESS or OS_FAILED
     */
    OsStatus getNotificationsUIHtmlCode(  const UtlString& contactAddress,
                                          UtlHashMap* contactDetailsHashDict,
                                          UtlString& htmlCode) const ;

    /**
     *  Constructs the HTML page for displaying a form for adding new / editing
     *  existing contacts.
     *
     *  @param  out     Contains the HTML code. Filled on return.
     *  @return OS_SUCCESS or OS_FAILED
     */
    OsStatus getAddEditNotificationsUI(UtlString* out) const;

    /**
     *  Calls the appropriate method in MailboxManager for
     *  adding / editing / deleting a contact.
     *  Constructs HTML code for redirecting to the appropriate page
     *  and sends appropriate status based on the outcome of the operation requested.
     *
     *  @param  out     Contains the HTML code. Filled on return.
     *  @return OS_SUCCESS or OS_FAILED
     */
    OsStatus addEditDeleteNotification(UtlString* out) const ;

    /** Utility method for validating the contact address
     *  before adding it / replacing the existing address with it.
     *  Validates the address stored in the variable m_newContactAddress
     *
     *  @return     OS_SUCCESS      Address is valid
     *  @return     OS_INVALID      Address is invalid
     */
    OsStatus validateContactAddress() const ;

protected:

private:
        UtlString m_mailboxIdentity;
    const UtlString m_action ;
    const UtlString m_status ;
    const UtlString m_contactAddress ;
    const UtlString m_contactType ;
    const UtlString m_newContactAddress ;
    const UtlString m_newContactType ;
    const UtlString m_sendAttachments ;

    UtlString m_cgiUrl ;
};

#endif //ManageNotificationsWebCGI_H
