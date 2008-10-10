// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _WatchDog_h_
#define _WatchDog_h_

// SYSTEM INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "utl/UtlSList.h"

// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class XmlRpcDispatch;


//:Class to provide interface to process mgmt xmlrpc
// All that is left of the old WatchDog.  Could be renamed.
class WatchDog
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   WatchDog( const int port,UtlSList& allowedPeers);
   /**<
    * Default constructor (takes list of peers from which to accept requests)
    * This object become the owner of the new'd memory in the 'allowedPeers'
    * list.  It will delete them upon destruction.
    */

   virtual
   ~WatchDog();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   WatchDog& operator=(const WatchDog& rhs);
     //:Assignment operator
   
/* ============================ ACCESSORS ================================= */

   /// Get the XML-RPC dispatcher
   XmlRpcDispatch* getXmlRpcDispatch();

   /// Whether or not the specified peer is allowed to make XML-RPC Process Manamgement requests.
   bool isAllowedPeer(const UtlString& peer) const;
   /**<
    * The mAllowedPeers list is currently only modified once (during the thread
    * initialization.)  After that all access is thread-safe by virtue of being 
    * read-only.  i.e. Calling this method does not lock the semephore.
    * 
    * \param peer The name of the peer making the request.
    * \return True if the specified peer is allowed, false otherwise.
    * \see mAllowedPeers
    */

   /// Server for the XML-RPC requests.
   void startRpcServer();


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsBSem mLock; // Semaphore to enable thread-safe support.

   int             mXmlRpcPort;
   XmlRpcDispatch* mpXmlRpcDispatch;

   UtlSList  mAllowedPeers;  // The list of peers allowed to make XML-RPC Process 
                             // Management requests.

   WatchDog(const WatchDog& rWatchDog);
     //:Copy constructor (not implemented for this class)


};

/* ============================ INLINE METHODS ============================ */

#endif  // _WatchDog_h_
