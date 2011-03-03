//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SIPREDIRECTORMPT_H
#define SIPREDIRECTORMPT_H

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

// Task type to write the mappings file to disk when needed.
class MPTWriterTask : public OsTask {
  public:

   MPTWriterTask(void* pArg);

   virtual int run(void* pArg);
};

/**
 * SipRedirectorMPT is a class whose object returns contacts for the
 * Multi-Party Test system.
 */

class SipRedirectorMPT : public RedirectPlugin
{
  public:

   explicit SipRedirectorMPT(const UtlString& instanceName);

   ~SipRedirectorMPT();

   virtual void readConfig(OsConfigDb& configDb);

   /**
    * Requires the following parameters:
    *
    * MAPPING_FILE - full file name containing the mappings.
    */
   virtual OsStatus initialize(const UtlHashMap& configParameters,
                               OsConfigDb& configDb,
                               SipUserAgent* pSipUserAgent,
                               int redirectorNo);

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

   // File to read/write mappings to.
   UtlString mMappingFileName;

   // Semaphore to lock addess to the maps.
   OsBSem mMapLock;

   // Maps (full) user names into lists of contacts (each of which is ended
   // with newline).
   UtlHashMap mMapUserToContacts;
   // The reverse map from contact strings to user names.
   UtlHashMap mMapContactsToUser;

   // True if the maps have been modified but not written out.
   UtlBoolean mMapsModified;

   // Socket for HTTP server
   OsServerSocket* mpSocket;

   // HTTP server for creating further mappings.
   HttpServer* mpServer;

   // File to read/write the mappings.
   UtlString configFileName;

   // Host name for this redirector.
   UtlString mDomainName;

   // Helper task to write the mappings back to disk periodically.
   MPTWriterTask writerTask;

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
   static UtlBoolean addMappings(const char* value,
                                 int length,
                                 UtlString*& user,
                                 const char*& error_msg,
                                 int& location);

  protected:

   // String to use in place of class name in log messages:
   // "[instance] class".
   UtlString mLogName;
};

#endif // SIPREDIRECTORMPT_H
