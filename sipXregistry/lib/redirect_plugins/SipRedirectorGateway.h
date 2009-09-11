//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SIPREDIRECTORGATEWAY_H
#define SIPREDIRECTORGATEWAY_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "registry/RedirectPlugin.h"
#include "digitmaps/UrlMapping.h"
#include "net/HttpServer.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipRedirectorGateway;

// Task type to write the mappings file to disk when needed.
class GatewayWriterTask : public OsTask {
  public:

   GatewayWriterTask(void* pArg);

   virtual int run(void* pArg);

  protected:
   SipRedirectorGateway* mpRedirector;
};

/**
 * SipRedirectorGateway is a class whose object maps dialing prefixes to
 * host addresses (presumably of SIP gateways).  It also runs an HTTP server
 * by which the mappings can be configured.
 */

class SipRedirectorGateway : public RedirectPlugin
{
  public:

   explicit SipRedirectorGateway(const UtlString& instanceName);

   ~SipRedirectorGateway();

   virtual void readConfig(OsConfigDb& configDb);

   /**
    * Requires the following parameters:
    *
    * MAPPING_FILE - full file name containing the mappings.
    * PREFIX - fixed part of routing prefix
    * DIGITS - number of digits in variable portion of routing prefix
    * PORT - HTTP listening port
    */
   virtual OsStatus initialize(OsConfigDb& configDb,
                               int redirectorNo,
                               const UtlString& localDomainHost);

   virtual void finalize();

   virtual RedirectPlugin::LookUpStatus lookUp(
      const SipMessage& message,
      const UtlString& requestString,
      const Url& requestUri,
      const UtlString& method,
      ContactList& contactList,
      RequestSeqNo requestSeqNo,
      int redirectorNo,
      SipRedirectorPrivateStorage*& privateStorage,
      ErrorDescriptor& errorDescriptor);

   virtual const UtlString& name( void ) const;

   /** Return TRUE if p is a valid prefix for this redirector. */
   UtlBoolean prefixIsValid(UtlString& p);

  protected:

   // String to use in place of class name in log messages:
   // "[instance] class".
   UtlString mLogName;

   // Dialing prefix - fixed portion of routing prefix.
   UtlString mPrefix;

   // Number of digits in variable portion of routing prefix.
   int mDigits;

   // File to read/write mappings to.
   UtlString mMappingFileName;

   // Return value for ::initialize().
   OsStatus mReturn;

   // Port for HTTP server.
   int mPort;

   // Semaphore to lock addess to the maps.
   OsBSem mMapLock;

   // Maps (full) user names into lists of contacts (each of which is ended
   // with newline).
   UtlHashMap mMapUserToContacts;
   // The reverse map from contact strings to user names.
   UtlHashMap mMapContactsToUser;

   // True if the maps have been modified but not written out.
   UtlBoolean mMapsModified;

   // Socket for the HTTP server
   OsServerSocket* mpSocket;

   // HTTP server for creating further mappings.
   HttpServer* mpServer;

   // Host name for this redirector.
   UtlString mDomainName;

   // Helper task to write the mappings back to disk periodically.
   GatewayWriterTask mWriterTask;

   void loadMappings(UtlString* file_name,
                     UtlHashMap* mapUserToContacts,
                     UtlHashMap* mapContactsToUser);
   void writeMappings(UtlString* file_name,
                      UtlHashMap* mapUserToContacts);
   // Add a mapping.  Returns the user assigned.
   UtlString* addMapping(const char* contacts,
                         int length);

   static void displayForm(const HttpRequestContext& requestContext,
                           const HttpMessage& request,
                           HttpMessage*& response);
   static void processForm(const HttpRequestContext& requestContext,
                           const HttpMessage& request,
                           HttpMessage*& response);
   UtlBoolean addMappings(const char* value,
                          int length,
                          UtlString*& user,
                          const char*& error_msg,
                          int& location);
};

#endif // SIPREDIRECTORGATEWAY_H
