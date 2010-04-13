//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _CpGatewayManager_h_
#define _CpGatewayManager_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsServerTask.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class PtGatewayInterface;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class CpGatewayManager : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   CpGatewayManager();
     //:Default constructor

   CpGatewayManager(const CpGatewayManager& rCpGatewayManager);
     //:Copy constructor

   virtual
   ~CpGatewayManager();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   static CpGatewayManager* getCpGatewayManager();
    //: Singleton constructor/accessor
    // Note: this class does not need to be a singleton.  The only method that
    // assumes singleton is getCpGatewayManager.

   CpGatewayManager& operator=(const CpGatewayManager& rhs);
     //:Assignment operator

   OsStatus setGatewayInterface(const char* terminalName,
       PtGatewayInterface* pInterface);
   //: Add a PtGatewayInterface call back object for media setup on the named
   //: terminal.
   // Note this method does not validate the terminal name in any way.  One
   // validate the terminal name and be sure that it is registered with the
   // provider (i.e. proxy server) if calls are to work with the provider.
   //! param: (in) terminalName - the terminalName with which the interface is to work.
   //! param: (in) pInterface - interface object to be called for media setup information for calls on the geteway terminal.
   //! retcode: OS_SUCCESS - the interface was registered for the named terminal.

   void getNewCallId(UtlString& callId);
   //: Get a valid callId with which to create a new call
   //  The callId is needed to call methods (i.e. <I>dialUrl</I>) requiring
   // a callId to create a new call.

   OsStatus connect(const char* callId,
                    const char* fromAddressUrl,
                    const char* fromTerminalName,
                    const char* toAddressUrl);
   //: Create a call if one does not exist and setup a connection between
   //: the to and from addresses originating on the given terminal.
   //! param: (in) callId - must refer to an existing call or a valid callId from <I>getNewCallId</I>.  In the latter case a new call will be created.
   //! param: (in) fromAddressUrl - the originating terminal url.  Note: this should be validated if it is used in conjuction with the provider or proxy server.
   //! param: (in) fromTerminalname - this should be a valid gateway terminal having a registered (via <I>setGatewayInterface</I>) PtGatewayInterface.
   //! param: (in) toAddressUrl - destination url.  Note: in general this cannot be validated as the call may be to an address which is outside the domain of the provider.
   //! retcode: OS_SUCCESS - the callId is valid and an attempt will be made to connect the call.  This does not indicate that the connect was successful or completed.
   //! retcode: OS_INVALID_ARGUMENT - the call indicated by callId previously existed, but was in an invalid state to connect a call.

   OsStatus answer(const char* callId, const char* terminalName);
   //: Answer an incoming call on the given terminal.
   //! param: callId - identifies an existing incomming call.
   //! param: terminalName - this should be a valid gateway terminal having a registered (via <I>setGatewayInterface</I>) PtGatewayInterface having the address of the destination of the incoming call.
   //! retcode: OS_SUCCESS - the callId is valid and an attempt will be made to answer the call.  This does not indicate that the answer was successful or completed.
   //! retcode: OS_INVALID_ARGUMENT - the call indicated by callId or the terminalName indicated does not exist.

   OsStatus disconnectConnection(const char* callId,
                                 const char* connectionAddressUrl);
   //: Disconnects the indicated connection in the given call.
   //! param: (in) callId - the handle indicating which call contains the connection to disconnect.
   //! param: (in) connectionAddressUrl - indicates which leg of the call is to be disconnected.
   //! retcode: OS_SUCCESS - the callId and connectionAddressUrl are valid and an attempt will be made to disconnect the connection.  This does not indicate that the disconnect was successful or completed.
   //! retcode: OS_INVALID_ARGUMENT - the call indicated by callId or the connection indicated by connectionAddressUrl does not exist.

/* ============================ ACCESSORS ================================= */

   UtlBoolean gatewayInterfaceExists(const char* terminalName);
   //: Checks if the named terminal has a registered gateway interface.
   // Note: this does not validate that the terminal name is valid with respect to the provider or proxy.

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _CpGatewayManager_h_
