// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _EmailReporter_h_
#define _EmailReporter_h_

// SYSTEM INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "FailureReporterBase.h"

// APPLICATION INCLUDES

// DEFINES

//if you really have to contact more than fifty people, then something is really
//wrong...
#define MAX_CONTACTS 50  
#define MAX_MESSAGES 1000

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class EmailReporter : public FailureReporterBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   EmailReporter();
     //:Default constructor

   EmailReporter(const EmailReporter& rEmailReporter);
     //:Copy constructor

   virtual
   ~EmailReporter();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   EmailReporter& operator=(const EmailReporter& rhs);
     //:Assignment operator
   
   virtual OsStatus report(UtlString &rProcessAlias,UtlString &rMessage);
   //: Queues the report information to the specified targte
   
   virtual OsStatus send();  
   //: Sends any queued reports to target

   OsStatus addContact(UtlString &rEmailAddress);
   //: Add one contact to the reporter list

   void setEmailExecuteCommand(UtlString &rCommand);
   //: Sets the external command to execute which runs the email
   //: Program.
   
   void flush();
   //: forces the queue to be cleared

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    UtlString *mContacts[MAX_CONTACTS];
    //array of contacts to send mail to

    int mNumContacts;
    //:number of contacts we loaded

    UtlString mEmailCommandStr;
    //external command to execute to run email smtp client
    
    UtlString *mFailureMessage[MAX_MESSAGES];
    //: Place to store messages to be sent

    int mNumMessages;
    //: Total Messages stored thus far.
};

/* ============================ INLINE METHODS ============================ */

#endif  // _EmailReporter_h_

