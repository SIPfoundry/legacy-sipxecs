//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _HttpConnection_h_
#define _HttpConnection_h_

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
#include <os/OsTask.h>

// DEFINES

class HttpServer;

class HttpConnection : public OsTask, public UtlString
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    //! Constructor
    HttpConnection(OsConnectionSocket* requestSocket,
                   HttpServer* httpServer);

    //! Destructor
    virtual ~HttpConnection();

/* ============================ MANIPULATORS ============================== */
    virtual int run(void* runArg);

/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */
    bool toBeDeleted() {return mbToBeDeleted;}
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    OsConnectionSocket* mpRequestSocket;  ///< pointer to request socket
    HttpServer*         mpHttpServer;     ///< pointer HTTP server for callbacks
    bool                mbToBeDeleted;    ///< Indicator if connection can be deleted

    /// no copy constructor
    HttpConnection(const HttpConnection& nocopy);

    /// no assignment operator
    HttpConnection& operator=(const HttpConnection& noassignment);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _HttpConnection_h_
