// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _REDIRECTPLUGIN_H_
#define _REDIRECTPLUGIN_H_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/Plugin.h"
#include "os/OsStatus.h"
#include "net/SipMessage.h"
#include "utl/UtlString.h"
#include "net/Url.h"
#include "os/OsConfigDb.h"
#include "net/SipUserAgent.h"
#include "utl/UtlContainableAtomic.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class SipRedirectorPrivateStorage;

/**
 * SIP Redirect Plugin Hook
 *
 * A RegisterPlugin is an action invoked by the SipRegistrarServer whenever a
 *   successful REGISTER request has been processed (that is, after it has
 *   effected its change on the registry database).  The plugin may then take
 *   any action based on the fact that the registration has occured.
 *
 * This class is the abstract base from which all RegisterPlugins must inherit.
 *
 * To configure a RegisterPlugin into the SipRegistrarServer, the registrar-config
 * file should have a directive configuring the plugin library:
 * @code
 * SIP_REGISTRAR_HOOK_LIBRARY.[instance] : [path to libexampleregplugin.so]
 * @endcode
 * Where [instance] is replaced by a unique plugin name, and the value
 * points to the libary that provides the plugin code.
 *
 * In addition to the class derived from this base, a RegisterPlugin library must
 * provide a factory routine named getRegisterPlugin with extern "C" linkage so
 * that the OsSharedLib mechanism can look it up in the dynamically loaded library
 * (looking up C++ symbols is problematic because of name mangling).
 * The factory routine looks like:
 * @code
 * class ExampleRegisterPlugin : public RegisterPlugin
 * {
 *    virtual void takeAction( const SipMessage&   registerMessage ///< the successful registration
 *                            ,const unsigned int  registrationDuration ///< the actual allowed
 *                                                                      /// registration time (note
 *                                                                      /// that this may be < the
 *                                                                      /// requested time).
 *                            ,SipUserAgent*       sipUserAgent     ///< to be used if the plugin
 *                                                                  /// wants to send any SIP msg
 *                            );
 *
 *    friend RegisterPlugin* getRegisterPlugin(const UtlString& name);
 * }
 *
 * extern "C" RegisterPlugin* getRegisterPlugin(const UtlString& instance)
 * {
 *   return new ExampleRegisterPlugin(instance);
 * }
 * @endcode
 *
 * @see Plugin
 */

/**
 * SipRedirector is a virtual class.  Instances of its subclasses are the
 * processing agents which construct and manipulate lists of contact points
 * for addresses of record.
 */

class RedirectPlugin : public Plugin
{
  public:

   /**
    * The type of the request sequence numbers.
    *
    * Must be a type for which '++' will increment with rollover when
    * the end of its range is reached.
    */
   typedef unsigned int RequestSeqNo;

   static const char* Prefix;
   ///< the configuration file prefix = "SIP_REDIRECT"

   static const char* Factory;
   ///< the factory method name = "getRedirectPlugin"

   /**
    * The destructor does almost nothing, as the primary finalization
    * work should be done by ::finalize().
    */
   virtual ~RedirectPlugin();

   /**
    * Initialization will be done by the PluginHooks object calling
    * the ::readConfig() method, then ::initialize() will be called.
    * All initialization should be done in one of these methods, rather
    * than the constructor, so that we have finer control over when
    * it happens.
    *
    * In ::readConfig(), configDb is the subset of the configuration
    * parameters tagged for this plugin.  In ::initialize(),
    * configDb is a UtlHashMap that gives the complete
    * configuration parameters. 
    *
    * pSipUserAgent is a pointer to the SipUserAgent to use for
    * communication.
    *
    * redirectorNo is the number this redirector is assigned.
    *
    * @return ::initialize() returns OS_SUCCESS if it has successfully
    * initialized and wishes to process requests, and OS_FAILED if it
    * does not wish to process requests.  If it has detected an error,
    * it must output an error message on its own, as OS_FAILED per se isn't
    * an error signal.  Regardless of the return value, ::finalize()
    * will be called before the destructor is called.
    */
   virtual OsStatus initialize(OsConfigDb& configDb,
                               SipUserAgent* pSipUserAgent,
                               int redirectorNo,
                               const UtlString& localDomainHost) = 0;

   virtual void readConfig(OsConfigDb& configDb);

   /**
    * All finalization should be done in ::finalize() in preference
    * to in the destructor, so that we have finer control over when
    * it happens.
    */
   virtual void finalize() = 0;

   typedef enum LookUpStatus
   {
      // Start numbering status values from 1 so 0 is invalid.
      LOOKUP_SUCCESS = 1,   ///< the redirector successfully finished its processing.
      LOOKUP_ERROR_REQUEST, /**< the request was invalid in some way
                             * (e.g., a call retrieve from an invalid orbit number)
                             * and the request should get a 403 Forbidden response. */
      LOOKUP_ERROR_SERVER,  /**< the redirector encountered an internal
                             * error and the request should get a 500 response */
      LOOKUP_SUSPEND        /**< the redirector needs the request to be suspended
                             * for asynchronous processing. */
   } LookUpStatus;

   /// Look up redirections and add them to the response.
   /**
    * @return the LookUpStatus indication showing the outcome of processing
    *
    * LOOKUP_SUCCESS is used in all non-error situations, even if the
    * redirector added no contacts.  Generation of 404 responses is done
    * by SipRedirectServer after all redirectors have executed.
    * If it returns LOOKUP_ERROR_*, the redirector should most likely have
    * logged message(s) at ERR level giving the details of the problem.
    *
    * The SipRedirectServer will be holding mMutex while lookUp is called.
    * See ../../doc/Redirection.txt for more details on how lookUp is called.
    */
   virtual LookUpStatus lookUp(
      const SipMessage& message,      ///< the incoming SIP message
      const UtlString& requestString, /**< the request URI from the SIP message as a UtlString
                                       *   ONLY for use in debugging messages; all comparisons
                                       *   should be with requestUri */
      const Url& requestUri,          ///< the request URI from the SIP message as a Uri,
      const UtlString& method,
      SipMessage& response,           ///< the response SIP message that we are building.
      RequestSeqNo requestSeqNo,      ///< the request sequence number
      int redirectorNo,               ///< the identifier for this redirector
      class SipRedirectorPrivateStorage*& privateStorage /**< the cell containing the pointer
                                                          * to the private storage object for
                                                          * this redirector, for this request. */
                               ) = 0;

   /**
    * Cancel processing of a request.
    *
    * If lookUp has ever returned LOOKUP_SUSPEND for this request, the
    * redirect server guarantees to call cancel after the moment when
    * lookUp will never be called again for the request.
    *
    * The private storage for the redirector that is managed by
    * SipRedirectServer will be deleted after cancel() is called.
    *
    * request is the sequence number of the request to be canceled.
    *
    * The SipRedirectServer will be holding mMutex while cancel() is called.
    *
    * This call must not block.
    */
   virtual void cancel(RequestSeqNo request);

   /**
    * Add a contact to the redirection list.
    *
    * response is the response SIP message we are constructing, for
    * use in debugging messages.
    *
    * contact is the contact URI to be added to the redirection list.
    *
    * label is a string that described this redirector, for use in
    * debugging messages.
    *
    * requestString is the request URI as a UtlString 
    */
   static void addContact(SipMessage& response,
                          const UtlString& requestString,
                          const Url& contact,
                          const char* label);

   /**
    * Remove all contacts from the redirection list.
    */
   static void removeAllContacts(SipMessage& response);

  protected:

   /// Constructor is protected so that it is only callable from subclasses.
   explicit RedirectPlugin(const UtlString& instanceName) :
      Plugin(instanceName)
      {
      };
    
   /**
    * Declare that a redirector is ready to reprocess a request.
    *
    * This method may be called from any context and does not block,
    * because all it does is queue a message on the redirect server's
    * queue.  It should not be called more than once following each
    * invocation of lookUp that returns LOOKUP_SUSPEND.
    *
    * request is the sequence number of the request (obtained from the
    * invocation of lookUp).
    *
    * redirector is the identifier of this redirector (ditto)
    */
   static void resumeRedirection(RequestSeqNo request,
                                 int redirector);
  private:

   /// There is no copy constructor.
   RedirectPlugin(const RedirectPlugin&);

   /// There is no assignment operator.
   RedirectPlugin& operator=(const RedirectPlugin&);
    
};

/**
 * SipRedirectorPrivateStorage is a virtual class.  Instances of its
 * subclasses can be saved by redirectors on the master list of
 * suspended requests.  Subclasses must implement getContainableType.
 *
 * Its hash and comparison functions are inherited from UtlContainableAtomic,
 * because these objects are treated as atomic.
 */

class SipRedirectorPrivateStorage : public UtlContainableAtomic
{
  public:
   
   virtual ~SipRedirectorPrivateStorage();

   virtual const char* const getContainableType() const = 0;

   // TYPE is not defined because this class is abstract.
};

#endif // _REDIRECTPLUGIN_H_
