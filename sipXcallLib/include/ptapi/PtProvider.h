//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtProvider_h_
#define _PtProvider_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsBSem.h"
#include "os/OsProtectEventMgr.h"
#include "ptapi/PtDefs.h"
#include "tao/TaoDefs.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class PtAddress;
class PtCall;
class PtProviderListener;
class PtTerminal;
class TaoClientTask;
class TaoServerTask;
class TaoReference;
class TaoObjectMap;
class CpCallManager;

//:A PtProvider represents the software entity that interfaces with an
//:instance of the "Pingtel Server" to monitor and control a group of
//:Pingtel phones.
//
// <H3>Obtaining access to a Provider (Pingtel Server)</H3>
// A PtProvider object is created and returned by the static
// PtProvider::getProvider() method.  This method sets up any needed
// communication paths between the application and the provider.
//
// <H3>Listeners and Events</H3>
// Each time a state change occurs on a provider, the application is notified
// via an event. These events are reported via PtProviderListener objects.
// Applications instantiate objects which are derived from the
// PtProviderListener class and use the PtProvider.addProviderListener()
// method to begin the delivery of events. Nearly all provider
// events reported via a PtProviderListener are instances of either the
// PtProviderEvent class or its descendants.  The only exceptions are the
// PROVIDER_ADDRESS_ADDED and PROVIDER_ADDRESS_REMOVED events (which are
// instances of the PtAddressEvent class) and the PROVIDER_TERMINAL_ADDED
// and PROVIDER_TERMINAL_REMOVED events (which are instances of the
// PtTerminalEvent class).<br>
// <br>
// Applications may query the reported event object for
// the specific state change via the PtEvent.getId() method. When the
// provider changes state, a PtProviderEvent is reported via the
// PtProviderListener with one of the following event IDs:
// PROVIDER_IN_SERVICE, PROVIDER_OUT_OF_SERVICE, or PROVIDER_SHUTDOWN.  A
// PtProviderEvent with an event ID of PROVIDER_EVENT_TRANSMISSION_ENDED is
// delivered to all PtProviderListeners when the provider becomes
// unobservable and is the final event delivered to the listener.
//
// <H3>Call Objects and Providers</H3>
// The PtProvider maintains knowledge of the calls currently associated with
// it. Applications may obtain an array of these calls via the
// PtProvider.getCalls() method. A PtProvider may have calls associated with
// it that were created before it came into existence. The provider maintains
// references to all calls until they move into the PtCall::INVALID state.<br>
// <br>
// Applications may create new calls using the PtProvider.createCall() method.
// A new call is returned in the PtCall::IDLE state. Applications may then
// use this idle call to place new phone calls. Once created, this new
// call object is returned via the PtProvider.getCalls() method.
//
// <H3>The Provider's domain</H3>
// The term "provider's domain" refers to the collection of PtAddress and
// PtTerminal objects which are local to the provider, and typically, can be
// controlled by the provider.
//
// <H3>Address and Terminal Objects</H3>
// A PtAddress object represents what we commonly think of as a "telephone
// number." For calls to IP telephony endpoints, the "telephone number" may
// actually be a URL or IP address identifying the endpoint.  Regardless, it
// represents a logical endpoint of a phone call.  A PtTerminal
// represents a physical endpoint connected to the telephone or IP network.
// An example of a terminal is a telephone that uses the Pingtel Telephony API.
// PtAddresses and PtTerminals are in a many-to-many relationship. A
// PtAddress may be associated with multiple PtTerminals, and PtTerminals
// may be associated with multiple PtAddresses. See the specifications for
// the PtAddress and PtTerminal classes for more information.<br>
// <br>
// Unlike PtCall objects, applications may not create PtTerminal or PtAddress
// objects. The PtProvider begins with knowledge of certain PtTerminal and
// PtAddress objects within its local domain. This list is quasi-static,
// changing only as logical and physical endpoints are added and removed in
// the phone system. The currently known addresses and terminals in the
// provider's domain are reported by the PtProvider.getAddresses() and
// PtProvider.getTerminals() methods, respectively.<br>
// <br>
// PtAddress and PtTerminal objects may also be created sometime during the
// operation of the PtProvider when the PtProvider learns of addresses and
// terminals that are outside of its domain.  For example, if the PtProvider's
// domain is a PBX, the PtProvider will know about all PtAddresses and
// PtTerminals in the PBX.  Any PtAddresses and PtTerminals it subsequently
// learns about that are outside the PBX are referred to as remote. These
// remote address and terminal objects are not reported by the
// PtProvider.getTerminals() and PtProvider.getAddresses() methods.

class PtProvider
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum ProviderState
   {
      IN_SERVICE,
      OUT_OF_SERVICE,
      SHUTDOWN
   };
   //!enumcode: IN_SERVICE - the provider is currently available for use
   //!enumcode: OUT_OF_SERVICE - the provider is currently not available for use
   //!enumcode: SHUTDOWN - the provider is permanently no longer available for use

/* ============================ CREATORS ================================== */

   static PtStatus getProvider(const char* login, const char* password,
                               const char* server, const char* options,
                                                           CpCallManager* pCallMgr, PtProvider*& rpProvider);

 //  static int getProvider(const char* login, const char* password,
   //                            const char* server, const char* options,
     //                          PtProvider& rProvider);
     //:If successful, sets <i>rpProvider</i> to point to a PtProvider object.
     //!param: (in) login - user identifier (used for authentication)
     //!param: (in) password - password (used for authentication)
     //!param: (in) server - Pingtel server expressed as "host:port"
     //!param: (in) options - (not presently used)
     //!param: (in) pCallManager - Pointer to the call manager
     //!param: (out) rProvider - set to refer to a PtProvider object

     //!retcode: PT_SUCCESS - rProvider reference to a PtProvider object
     //!retcode: PT_PROVIDER_UNAVAILABLE - the Pingtel server did not respond
     //!retcode: PT_HOST_NOT_FOUND - Invalid host or port for server
     //!retcode: PT_AUTH_FAILED - Invalid login name or password

/* ============================ MANIPULATORS ============================== */

   virtual PtStatus addProviderListener(PtProviderListener& rListener);
     //:Adds a listener to the provider.
     //!param: (in) rListener - the listener to add to this provider
     //!retcode: PT_SUCCESS - success
     //!retcode: PT_EXISTS - <i>rListener</i> is already registered
     //!retcode: PT_PROVIDER_UNAVAILABLE - the provider is not available

   virtual PtStatus createCall(PtCall& rCall);
     //:If successful, sets <i>rpCall</i> to refer to a new PtCall object.
     // The new PtCall object will be in the PtCall::IDLE state and have no
     // connections.
     //!param: (out) rCall - reference to the newly created call
     //!retcode: PT_SUCCESS - success
     //!retcode: PT_RESOURCE_UNAVAILABLE - insufficient resources
     //!retcode: PT_PROVIDER_UNAVAILABLE - the provider is not available

   virtual PtStatus removeProviderListener(PtProviderListener& rListener);
     //:Removes the specified listener from the provider.
     //!param: (in) rListener - the listener to remove from this provider
     //!retcode: PT_SUCCESS - success
     //!retcode: PT_NOT_FOUND - <i>rListener</i> not registered
     //!retcode: PT_PROVIDER_UNAVAILABLE - the provider is not available

   virtual PtStatus shutdown(void);
     //:Instructs the provider to shut itself down and perform all necessary
     //:cleanup.
     //!retcode: PT_SUCCESS - success
     //!retcode: PT_PROVIDER_UNAVAILABLE - the provider is not available

/* ============================ ACCESSORS ================================= */

   virtual PtStatus getAddress(const char* phoneURL, PtAddress& rAddress);
     //:Sets <i>rpAddress</i> to refer to the PtAddress corresponding to the
     //:specified phone number or NULL if there is no corresponding address.
     //!param: (in) phoneURL - a URL representing the phone number to look up
     //!param: (out) rAddress - reference to the address object corresponding to phoneURL
     //!retcode: PT_SUCCESS - success
     //!retcode: PT_INVALID_ARGUMENT - <i>phoneURL</i> was not recognized
     //!retcode: PT_PROVIDER_UNAVAILABLE - the provider is not available

//   PtStatus getAddresses(PtAddress& arAddresses[], int size, int& nItems);
      virtual PtStatus getAddresses(PtAddress arAddresses[], int size, int& nItems);
          //: TRICKY: array of reference are illegal, avoid using reference here.
     //:Returns an array of references to PtAddress objects that are known to
     //:the provider and within its domain.
     // The caller provides an array that can hold up to <i>size</i>
     // PtAddress references.  This method will fill in the <i>addresses</i>
     // array with up to <i>size</i> references.  The actual number of references
     // filled in is passed back via the <i>nItems</i> argument.
     //!param: (out) arAddresses - the array of PtAddress references
     //!param: (in) size - the number of elements in the <i>addresses</i> array
     //!param: (out) nItems - the number of items assigned
     //!retcode: PT_SUCCESS - success
     //!retcode: PT_MORE_DATA - there are more than <i>size</i> addresses
     //!retcode: PT_PROVIDER_UNAVAILABLE - the provider is not available

   //PtStatus getCalls(PtCall& arCalls[], int size, int& nItems);
   virtual PtStatus getCalls(PtCall arCalls[], int size, int& nItems);
   //:WARNING: array of reference are illegal try to avoid it.
     //:Returns an array of references to PtCall objects that are known to the
     //:provider.
     // The caller provides an array that can hold up to <i>size</i>
     // PtCall references.  This method will fill in the <i>calls</i> array with
     // up to <i>size</i> references.  The actual number of references filled in
     // is passed back via the <i>nItems</i> argument.
     //!param: (out) arCalls - the array of PtCall pointers
     //!param: (in) size - the number of elements in the <i>calls</i> array
     //!param: (out) nItems - the number of items assigned
     //!retcode: PT_SUCCESS - success
     //!retcode: PT_MORE_DATA - there are more than <i>size</i> calls
     //!retcode: PT_PROVIDER_UNAVAILABLE - the provider is not available

  virtual PtStatus getProviderListeners(PtProviderListener* listeners[],
                                int size, int& nItems);
     //:Returns an array of pointers to PtProviderListener objects that have
     //:been registered with this provider.
     // The caller provides an array that can hold up to <i>size</i>
     // PtProviderListener pointers.  This method will fill in the
     // <i>listeners</i> array with up to <i>size</i> pointers.  The actual
     // number of pointers filled in is passed back via the <i>nItems</i>
     // argument.
     //!param: (out) listeners - the array of PtProviderListener pointers
     //!param: (in) size - the number of elements in the <i>pListeners</i> array
     //!param: (out) nItems - the number of items assigned
     //!retcode: PT_SUCCESS - success
     //!retcode: PT_MORE_DATA - there are more than <i>size</i> listeners
     //!retcode: PT_PROVIDER_UNAVAILABLE - the provider is not available

   virtual int getState(void);
     //:Returns the current state of the provider, either IN_SERVICE,
     //:OUT_OF_SERVICE or SHUTDOWN.
     //!retcode: IN_SERVICE - the provider is available for use
     //!retcode: OUT_OF_SERVICE - the provider is temporarily unavailable
     //!retcode: SHUTDOWN - the provider is permanently unavailable

   virtual PtStatus getTerminal(const char* name, PtTerminal& rTerminal);
     //:Assigns to <i>rTerminal</i> the PtTerminal corresponding to the
     //:specified name.
     //!param: (in) name - the name to look up (typically the phone extension)
     //!param: (out) rpTerminal - the terminal object for name
     //!retcode: PT_SUCCESS - success
     //!retcode: PT_INVALID_ARGUMENT - name was not recognized
     //!retcode: PT_PROVIDER_UNAVAILABLE - the provider is not available
// PtStatus getTerminals(PtTerminals& arTerms[], int size, int& nItems);
   virtual PtStatus getTerminals(PtTerminal arTerms[], int size, int& nItems);
   //:WARNING: avoid using array of reference.
     //:Returns an array of references to PtTerminal objects that are known to
     //:the provider and within its domain.
     // The caller provides an array that can hold up to <i>size</i>
     // PtTerminal references.  This method will fill in the <i>terms</i> array
     // with up to <i>size</i> references.  The actual number of references
     // filled in is passed back via the <i>nItems</i> argument.
     //!param: (out) arTerms - the array of PtTerminal references
     //!param: (in) size - the number of elements in the <i>terms</i> array
     //!param: (out) nItems - the number of items assigned
     //!retcode: PT_SUCCESS - success
     //!retcode: PT_MORE_DATA - there are more than <i>size</i> terminals
     //!retcode: PT_PROVIDER_UNAVAILABLE - the provider is not available

   virtual PtStatus numAddresses(int& count);
     //:Returns the number of addresses currently known to this provider and
     //:within its domain.
     //!param: (out) count - the number of addresses known to this provider
     //!retcode: PT_SUCCESS - success
     //!retcode: PT_PROVIDER_UNAVAILABLE - the provider is not available

   virtual PtStatus numCalls(int& count);
     //:Returns the number of calls currently known to this provider.
     //!param: (out) count - the number of calls known to this provider
     //!retcode: PT_SUCCESS - success
     //!retcode: PT_PROVIDER_UNAVAILABLE - the provider is not available

   virtual PtStatus numProviderListeners(int& count);
     //:Returns the number of PtProviderListeners currently registered with
     //:this provider.
     //!param: (out) count - the number of listeners known to this provider
     //!retcode: PT_SUCCESS - success
     //!retcode: PT_PROVIDER_UNAVAILABLE - the provider is not available

   virtual PtStatus numTerminals(int& count);
     //:Returns the number of terminals currently known to this provider and
     //:within its domain.
     //!param: (out) count - the number of terminals known to this provider
     //!retcode: PT_SUCCESS - success
     //!retcode: PT_PROVIDER_UNAVAILABLE - the provider is not available

/* ============================ INQUIRY =================================== */

friend class PtTerminal;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    static OsBSem           semInit ;
      //: Binary Semaphore used to guard initialiation and tear down
    static PtProvider* spInstance;       // pointer to the single instance of
        static TaoClientTask            *mpClient;

        static TaoReference             *mpTransactionCnt;

        static TaoObjectMap             *mpCalls;
        static TaoReference             *mpCallCnt;

        static TaoObjectMap             *mpAddresses;
        static TaoReference             *mpAddressCnt;

        static unsigned int             mRef;


        UtlString                       mLogin;
        UtlString                       mPass;

        ProviderState           mState;

        OsTime          mTimeOut;

        void initialize();

        PtStatus createProvider(const char* login, const char* password,
                        const char* server, const char* options,
         CpCallManager* pCallMgr = NULL);

        PtProvider(const char* login, const char* password,
                        const char* server, const char* options,
                        CpCallManager* pCallMgr = NULL);

        PtProvider(const PtProvider& rPtProvider);
         //:Copy constructor (not implemented for this class)

        PtProvider& operator=(const PtProvider& rhs);
         //:Assignment operator (not implemented for this class)

        PtProvider(UtlString& rLogin, UtlString& rPass);

        PtProvider();
     //:Default constructor

   virtual
   ~PtProvider();
     //:Destructor

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
        OsProtectEventMgr *mpEventMgr;
        static UtlBoolean mbInvalidIP;
                                                                        //  the Pinger class
        static OsBSem  sLock;            // semaphore used to ensure that there
                                                                        //  is only one instance of this class

        static UtlBoolean isLocal(const char* host);



};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtProvider_h_
