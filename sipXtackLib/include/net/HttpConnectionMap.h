//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _HttpConnectionMap_h_
#define _HttpConnectionMap_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <net/HttpServer.h>
#include <net/HttpBody.h>
#include <net/NameValuePair.h>
#include <os/OsSocket.h>
#include <os/OsConnectionSocket.h>
#include <os/OsTimeLog.h>
#include <os/OsMsgQ.h>
#include <utl/UtlHashMap.h>
#include <os/OsBSem.h>

// DEFINES

class HttpConnectionMapEntry : public UtlString
{
public:
    /// Constructor
    HttpConnectionMapEntry(const UtlString& name);

    /// Destructor
    virtual ~HttpConnectionMapEntry();

    OsConnectionSocket* mpSocket; //< pointer to a connection socket
    OsBSem              mLock;    //< protects access to the connection
    bool                mbInUse;  //< true if entry is in use, false if not
    static int          count;    //< used to udentify the entry
};

class HttpConnectionMap : public UtlHashMap
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /// Get pointer to singleton instance of the connection map
   static HttpConnectionMap* getHttpConnectionMap();

   /**<
    * @returns
    * - pointer to the instance of the connectiomn map
    */

   /// Release instance of connection map
   void releaseHttpConnectionMap();

   /// Clear all entries in map. Close all sockets and delete them.
   void clearHttpConnectionMap();

   /// Return a map entry for an existing connection or NULL. Locks the connection if non-NULL
   HttpConnectionMapEntry* getPersistentConnection(const Url& url, OsConnectionSocket*& socket);

   /**<
    * @returns
    * - pointer to a connection map entry and a connection socket. If no entry exists for a
    *   given URI one will be created and th socket pointer will be set to NULL.
    * - NULL if the connection does not exist
    */

/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    //! Constructor
    HttpConnectionMap();

    //! Destructor
    virtual ~HttpConnectionMap();

    /// Translate Url into key string that will be used for all further access
    void getPersistentUriKey(const Url& url, UtlString& key);

    static HttpConnectionMap* pInstance; ///< pointer to the instance
    static OsBSem mLock;                 ///< protects access to map

    /// no copy constructor
    HttpConnectionMap(const HttpConnectionMap& nocopy);

    /// no assignment operator
    HttpConnectionMap& operator=(const HttpConnectionMap& noassignment);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _HttpConnectionMap_h_
