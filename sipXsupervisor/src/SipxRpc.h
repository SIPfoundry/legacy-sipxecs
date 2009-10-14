//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _SipxRpc_h_
#define _SipxRpc_h_

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


///Class to provide interface to supervisor xmlrpc (including process management
///and file replication)
class SipxRpc
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   SipxRpc(XmlRpcDispatch* dispatcher, UtlSList& allowedPeers);
   /**<
    * Default constructor (takes list of peers from which to accept requests).
    * This object becomes the owner of the new'd memory in the 'allowedPeers'
    * list.  It will delete them upon destruction.
    */

   ///Destructor
   virtual
   ~SipxRpc();

/* ============================ MANIPULATORS ============================== */

   ///Assignment operator
   SipxRpc& operator=(const SipxRpc& rhs);

/* ============================ ACCESSORS ================================= */

   /// Get the XML-RPC dispatcher
   XmlRpcDispatch* getXmlRpcDispatch();

   /// Whether or not the specified peer is allowed to make XML-RPC Process Management requests.
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

   /// Whether or not an HTTP request is from some allowed peer, and if so which one.
   bool isAllowedPeer(const HttpRequestContext& context, ///< the request to be checked
                      UtlString& peer                    ///< if allowed, the name of the peer
                      ) const;
   /**<
    * @returns true if the request is from some allowed peer, and sets peer to that name.
    * Otherwise, peer is set to the empty string.
    */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   XmlRpcDispatch* mpXmlRpcDispatch;

   UtlSList  mAllowedPeers;  /// The list of peers allowed to make XML-RPC Process
                             /// Management requests.

   ///Copy constructor (not implemented for this class)
   SipxRpc(const SipxRpc& nocopy);


};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipxRpc_h_
