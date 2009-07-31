//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PtCallCapabilities_h_
#define _PtCallCapabilities_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class PtCall;
class PtTerminalConnection;

//:The PtCallCapabilities object represents the initial capabilities interface for the PtCall.
// This object supports basic queries for the core classes.
// <p>
// Applications obtain the static PtCall capabilities via the PtProvider.getCallCapabilities()
// method, and the dynamic capabilities via the PtCall.getCapabilities() method. This Class
// is used to represent both static and dynamic capabilities.
// <p>
// Any object which extends the PtCall class should also extend this class to provide additional
// capability queries for that particular package.
// <p>

class PtCallCapabilities
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   PtCallCapabilities();
     //:Default constructor

   PtCallCapabilities(const PtCallCapabilities& rPtCallCapabilities);
     //:Copy constructor

   virtual
   ~PtCallCapabilities();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PtCallCapabilities& operator=(const PtCallCapabilities& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

        UtlBoolean canConnect();
     //:Returns true if the application can invoke Call.connect(), false otherwise.
     //!retcode: True if the application can perform a connect
         //!retcode: false otherwise.



        UtlBoolean isObservable();
     //:Returns true if this Call can be observed, false otherwise.
     //!retcode: True if this Call can be observed
         //!retcode: false otherwise.


        UtlBoolean canDrop();
     //:Returns true if the application can invoke the drop feature, false otherwise.
     //!retcode: True if the application can invoke the drop feature
         //!retcode: false otherwise.




        UtlBoolean canOffHook();
     //:Returns true if the application can invoke the off hook feature, false otherwise.
     //!retcode: true if the application can invoke the off hook feature
         //!retcode: false otherwise.



        UtlBoolean canSetConferenceController();
     //:Returns true if the application can set the conference controller, false otherwise.
     //!retcode: true if the application can set the conference controller
         //!retcode: false otherwise.



        UtlBoolean canSetTransferController();
     //:Returns true if the application can set the transfer controller, false otherwise.
     //!retcode: true if the application can set the transfer controller
         //!retcode: false otherwise.



        UtlBoolean canSetTransferEnable();
     //:Returns true if the application can invoke the set transferring enabling feature, false
     // otherwise. The value returned by this method is independent of the ability of the application
     // to invoke the transfer feature.
         // <p>
     // Applications are not required to inform the implementation of the purpose of the
     // consultation call and may rely upon the default value returned by the
     // CallControlCall.getTransferEnable() method.
     //!retcode: True if the application can invoke the set transferring enabling feature,
         //!retcode: false otherwise.



        UtlBoolean canSetConferenceEnable();
     //:Returns true if the application can invoke the set conferencing enabling feature, false
     // otherwise. The value returned by this method is independent of the ability of the application
     // to invoke the conference feature.
         // <p>
     // Applications are not required to inform the implementation of the purpose of the
     // consultation call and may rely upon the default value returned by the
     // CallControlCall.getConferenceEnable() method.
     //!retcode: True if the application can invoke the set conferencing enabling feature,
         //!retcode: false otherwise.



        UtlBoolean canTransfer(PtCall call);
     //:Returns true if the application can invoke the overloaded transfer feature which takes a Call
     // as an argument, false otherwise.
         // <p>
     // The argument provided is for typing purposes only. The particular instance of the object
     // given is ignored and not used to determine the capability outcome is any way.
         // <p>
     //!param: (in) call - This argument is used for typing information to determine the overloaded version of the transfer() method.
     //!retcode: True if the application can invoke the transfer feature which takes a Call as an argument,
         //!retcode: false otherwise.



        UtlBoolean canTransfer(UtlString destination);
     //:Returns true if the application can invoke the overloaded transfer feature which takes a
     // destination string as an argument, false otherwise.
         // <p>
     // The argument provided is for typing purposes only. The particular instance of the object
     // given is ignored and not used to determine the capability outcome is any way.
         // <p>
     //!param: (in) destination - This argument is used for typing information to determine the overloaded version of the transfer() method.
     //!retcode: True if the application can invoke the transfer feature which takes a destination string as an argument,
                  //!retcode: false otherwise.


        UtlBoolean canConference();
     //:Returns true if the application can invoke the conference feature, false otherwise.
     //!retcode: True if the application can invoke the conference feature,
         //!retcode: false otherwise.



        UtlBoolean canAddParty();
     //:Returns true if the application can invoke the add party feature, false otherwise.
     //!retcode: True if the application can invoke the add party feature,
         //!retcode: false otherwise.



        UtlBoolean canConsult(PtTerminalConnection tc, UtlString destination);
     //:Returns true if the application can invoke the overloaded consult feature which takes a
     // TerminalConnection and string as arguments, false otherwise.
         // <p>
     // The arguments provided are for typing purposes only. The particular instances of the
     // objects given are ignored and not used to determine the capability outcome is any way.
     //!param: (in) tc - This argument is used for typing information to determine the overloaded version of the consult() method.
     //!param: (in) destination - This argument is used for typing information to destination the overloaded version of the consult() method.
     //!retcode: True if the application can invoke the consult feature which takes a PtTerminalConnection and a string as arguments.
     //!retcode: false otherwise



        UtlBoolean canConsult(PtTerminalConnection tc);
     //:Returns true if the application can invoke the overloaded consult feature which takes a
     // TerminalConnection as an argument, false otherwise.
         // <p>
     // The arguments provided are for typing purposes only. The particular instances of the
     // objects given are ignored and not used to determine the capability outcome is any way.
         //!param: (in) tc - This argument is used for typing information to determine the overloaded version of the consult() method.
     //!retcode: True if the application can invoke the consult feature which takes a PtTerminalConnection as an argument.
     //!retcode: false otherwise.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtCallCapabilities_h_
