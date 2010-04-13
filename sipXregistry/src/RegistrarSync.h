//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _REGISTRARSYNC_H_
#define _REGISTRARSYNC_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsServerTask.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipRegistrar;
class SipRegistrarServer;


/**
 * This thread is the XML-RPC client that sends updates to
 * each peer server.
 */
class RegistrarSync : public OsServerTask
{
public:
   /// constructor
   RegistrarSync(SipRegistrar& registrar);

   /// destructor
   virtual ~RegistrarSync();

   /// Signal that there may be updates ready to send.
   void sendUpdates();

   /// Send any updates that we can.
   UtlBoolean handleMessage(OsMsg& eventMessage);

protected:
   SipRegistrarServer& getRegistrarServer();

private:
   SipRegistrar& mRegistrar;

   /// There is no copy constructor.
   RegistrarSync(const RegistrarSync&);

   /// There is no assignment operator.
   RegistrarSync& operator=(const RegistrarSync&);
};

#endif // _REGISTRARSYNC_H_
