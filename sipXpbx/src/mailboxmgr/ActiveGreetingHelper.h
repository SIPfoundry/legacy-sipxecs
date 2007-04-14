// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef ACTIVEGREETINGHELPER_H
#define ACTIVEGREETINGHELPER_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS


// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 *  ActiveGreetingHelper
 *
 *  Provides methods for retrieving the URLs of the active greeting, 
 *  any recorded greeting and any recorded name.
 *
 *  @author Harippriya M Sivapatham
 *  @version 1.0
 */
class ActiveGreetingHelper
{
public:
    /**
     * Ctor
     */
    ActiveGreetingHelper();

    /**
     * Virtual Destructor
     */
    virtual ~ActiveGreetingHelper();

    /** Returns the URL of the active greeting that is played when 
     *  an incoming caller wants to leave a message for this user.
     *
     *  @param mailboxIdentity  Fully qualified mailbox URL of the user
     *  @param rGreetingUrl     Holds the fully qualified URL of the 
     *                          active greeting file - filled upon return.
     *  @param isFromWeb        Flag indicating the source of request.
     *                          This is used to determine the host of the mediaserver.
     *
     *  @return OS_SUCCESS      if URL was retrieved successfully.
     *          OS_FAILED       if there was an error retrieving the URL or
     *                          if the user had no active greeting.
     */
    OsStatus getActiveGreetingUrl ( const UtlString& mailboxIdentity, 
                                    UtlString& rGreetingUrl,
                                    const UtlBoolean& isFromWeb = FALSE) const;

    /** Returns the type of the active greeting
     *
     *  @param mailboxIdentity  Fully qualified mailbox URL of the user
     *  @param rGreetingType    Holds the type of the 
     *                          active greeting file - filled upon return.
     *
     *  @return OS_SUCCESS      if type was retrieved successfully.
     *          OS_FAILED       if there was an error
     */
    OsStatus getActiveGreetingType (    const UtlString& mailboxIdentity, 
                                        UtlString& rGreetingType ) const;


    /** 
     *  Returns the fully qualified URL of the specified greeting type.
     *  @param mailboxIdentity      Fully qualified mailbox id
     *  @param greetingType         Greeting type - "standard", "outofoffice", "extendedabsence".
     *  @param rGreetingUrl         Holds the URL - filled up on return
     *  @param isFromWeb            Flag indicating the source of request.
     *                              This is used to determine the host of the mediaserver.
     *  @param returnDefaultFileUrl Flag to indicate if the URL of the default system greeting
     *                              for the specified greeting type can be returned if 
     *                              user recorded greeting is not available.
     *
     *  @return OS_SUCCESS      if URL was retrieved successfully.
     *          OS_FAILED       if there was an error retrieving the URL or
     *                          if the user recorded and system greeting files were not available.
     */
    OsStatus getGreetingUrl (   const UtlString& mailboxIdentity,
                                const UtlString& greetingType,
                                UtlString& rGreetingUrl,
                                const UtlBoolean& isFromWeb = FALSE,
                                const UtlBoolean& returnDefaultFileUrl = TRUE) const;


    /** 
     *  Returns the fully qualified URL of the recorded name.
     *  @param mailboxIdentity  Fully qualified mailbox id
     *  @param rGreeting        Holds the URL - filled up on return
     *  @param isFromWeb        Flag indicating the source of request.
     *                          This is used to determine the host of the mediaserver.
     *
     *  @return OS_SUCCESS      if URL was retrieved successfully.
     *          OS_FAILED       if there was an error retrieving the URL or
     *                          if the user had not recorded their name.
     */    
    OsStatus getRecordedName(   const UtlString& mailboxIdentity, 
                                UtlString& rGreeting,
                                const UtlBoolean& isFromWeb = FALSE ) const;

protected:

private:

};

#endif //ACTIVEGREETINGHELPER_H

