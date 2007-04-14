//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef EditMessageWebCGI_H
#define EditMessageWebCGI_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "mailboxmgr/CGICommand.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 *  EditMessageWebCGI Class
 *
 *  CGI used for editing information associated with a message.
 *  Currently, it is invoked only from the WebUI and the information
 *  that is editable is the message subject.
 *  This CGI will be extended in the future, for example, to change the priority
 *  of the message and to be accessible from the TUI.
 *
 *  @author Harippriya M Sivapatham
 *  @version 1.0
 */
class EditMessageWebCGI : public CGICommand
{
public:
    /**
     * Ctor
     */
    EditMessageWebCGI(const UtlString& mailboxIdentity,
                      const UtlString& folderName,
                      const UtlString& messageId,
                      const UtlString& subject,
                      const UtlString& nextblockhandle,
                      const UtlBoolean& formSubmitted = FALSE) ;

    /**
     * Virtual Dtor
     */
    virtual ~EditMessageWebCGI();

    /** This does the work */
    virtual OsStatus execute ( UtlString* out = NULL );


protected:

private:
    /** Fully qualified mailbox identity */
        UtlString m_mailboxIdentity;

    /** Folder in which the message to be edited resides */
    const UtlString m_folderName ;

    /** Id of the message. */
    const UtlString m_messageId ;

    /** Updated message subject. */
    const UtlString m_msgSubject ;

    /** This CGI performs two functions and the flag m_formSubmitted is used to
        differentiate between the two:
        1. Display the form for editing a message --> m_formSubmitted = FALSE
        2. Edit the message by updating the message descriptor --> m_formSubmitted = TRUE
    */
    const UtlBoolean m_formSubmitted ;

    /** This variable is used to make sure that at the end of the edit,
        the right webpage is displayed (if there are more than one webpages used
        to display the contents of the single folder)
    */
    const UtlString m_nextBlockHandle ;

};

#endif //EditMessageWebCGI_H
