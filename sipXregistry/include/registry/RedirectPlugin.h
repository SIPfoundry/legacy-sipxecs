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
#include <vector>
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
#define REGISTRAR_ID_TOKEN "~~id~registrar"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class SipRedirectorPrivateStorage;
class ContactList;
class ErrorDescriptor;

/**
 * <b>SIP Redirect Plug-in overview</b>
 *
 *  A redirect plug-in is a logical entity that is, in its general form,
 *  responsible for mapping a Request URI to a set of one or more
 *  alternative locations at which the target of that URI can be found.
 *  A redirect plug-in is at liberty of choosing the redirection policies by
 *  which it establishes that mapping but it will generally be dictated
 *  by the nature of the specialized redirection function that it
 *  provides. For example, a redirector plug-in whose function it is to map an AOR to
 *  its registered Contacts will do so by consulting the location database.
 *
 * <b>Relationship with the SIP Redirect Server</b>
 *
 *  Each Redirect Plug-in is created by the SIP Redirect Server (see SipRedirectServer).
 *  For every request that the SIP Redirect Server processes it will
 *  call the plug-in's lookUp() or observe() method based on whether or not
 *  it wants the plug-in to provide Contacts for the request or merely observe it.
 *
 * <b>paragraph Redirect Plug-in requirements</b>
 *
 *  In order to create a redirect plug-in that will be called upon by the
 *  SipRedirectServer to play a part in the redirection effort, a certain
 *  set of requirements must be met.
 *   - Firstly, the redirect plug-in must be derived from the RedirectPlugin
 *     class and provide an implementation for its pure virtual methods and
 *     optionally its virtual methods.
 *
 *   - Secondly, to configure the redirect plug-in into the SipRedirectServer,
 *     the registrar-config file must have a directive configuring the plugin
 *     library:
 *     @code
 *         SIP_REDIRECT_HOOK_LIBRARY.[###-UNIQUE_NAME] : [path to libexampleregplugin.so]
 *     @endcode
 *     Where: ### is a three-digit value representing the ordinal number for the plug-in.
 *            UNIQUE_NAME is a unique name representing the plugin.
 *            path points to the library that provides the plugin code.
 *
 *   - Thirdly, the redirect plug-in must provide a factory routine named
 *     getRedirectPlugin with extern "C" linkage so that the OsSharedLib mechanism
 *     can look it up in the dynamically loaded library (looking up C++ symbols is
 *     problematic because of name mangling).
 *     The factory routine looks like:
 *     @code
 *         class ExampleRedirectPlugin : public RedirectPlugin
 *         {
 *            ...
 *             friend RedirectPlugin* getRedirectPlugin(const UtlString& instanceName);
 *             ...
 *         };
 *
 *         extern "C" RedirectPlugin* getRedirectPlugin(const UtlString& instanceName)
 *         {
 *            return new ExampleRegisterPlugin(instanceName);
 *         }
 *     @endcode
 *
 * @see Plugin
 * @see SipRedirectServer for more details on how and when the methods of RedirectPlugin are being called
 *      and to get additional information about the redirection process in general.
 *
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
    * The configDb is the subset of the configuration
    * parameters tagged for this plug-in ,i.e. of the form
    * SIP_REDIRECT.[###-UNIQUE_NAME.PARAM_NAME] : [PARAM_VALUE]
    * in the registrar-config configuration file.
    * configDb is a UtlHashMap that gives the complete
    * [PARAM_NAME]->[PARAM_VALUE] configuration pairs relevant to the
    * plug-in.
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
                               int redirectorNo,
                               const UtlString& localDomainHost) = 0;

   /*
    * Initialization will be done by the PluginHooks object calling
    * the ::readConfig() method, then ::initialize() will be called.
    * All initialization should be done in one of these methods, rather
    * than the constructor, so that we have finer control over when
    * it happens.
    *
    * The configDb is the subset of the configuration
    * parameters tagged for this plug-in ,i.e. of the form
    * SIP_REDIRECT.[###-UNIQUE_NAME.PARAM_NAME] : [PARAM_VALUE]
    * in the registrar-config configuration file.
    * configDb is a UtlHashMap that gives the complete
    * [PARAM_NAME]->[PARAM_VALUE] configuration pairs relevant to the
    * plug-in.
    */
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
      ERROR = 1,          ///< The redirector reported an error.  The passed
                          ///< ErrorDescriptor may be used to customize how that
                          ///< error will be communicated back to the originator
                          ///< via a SIP response.
      SEARCH_PENDING,     ///< The redirector needs the request to be suspended for
                          ///< asynchronous processing.
      SUCCESS             ///< the redirector successfully finished its processing.
   } LookUpStatus;

   /**
    * The SipRedirectServer calls this method to give a redirect plug-in a chance to
    * process the request and adjust the supplied Contact list to satisfy the
    * redirection policy it implements.
    *
    * After considering the request, if a redirect plug-in establishes that
    * the request is not interesting given the redirection policy it implements,
    * it must return SUCCESS and MUST NOT modify the supplied ContactList in ANY way.
    *
    * On the other hand, if the plug-in establishes that the request is one that
    * it should handle to affect its redirection, the plug-in must perform
    * all the necessary modifications tn the supplied ContactList to satisfy
    * the redirection policy it implements. These trasnformations include:
    *  - Addition of new contact(s);
    *  - Removal of one, many or all contacts;
    *  - Modification of contact(s) already contributed by plug-ins that were
    *    called before it;
    *  - Touching the ContactList.
    *
    * Once the ContactList modifications are completed, it must return SUCCES.
    *
    * When a redirect plug-in is asked to look-up a request, that plug-in, upon inspection
    * of the request, may determine that the request is interesting but contains errors
    * that prevent it from processing it.  Examples of such error conditions may
    * include missing vital headers, invalid user or domain, unsupported extension, ...
    * In such cases, the plug-in must return the ERROR status code to force
    * SipRedirectServer to return a final failure response to the requester.  The redirect
    * plug-in should also configure the supplied ErrorDescriptor to accurately represent
    * the error condition detected.
    *
    * If the redirect plug-in cannot complete the processing of the
    * request quickly, it must arrange for asynchronous processing that
    * will bring the redirector into a state where it can complete its processing
    * quickly and return SEARCH_PENDING.  Asynchronous processing then calls
    * RedirectPlugin::resumeRedirection() to indicate to the SipRedirectServer that
    * the request should be reprocessed.
    *
    * @see SiPRedirectServer for more details on how lookUp is called and redirection in general.
    *
    */
   virtual LookUpStatus lookUp(
      const SipMessage& message,      ///< the incoming SIP message
      const UtlString& requestString, /**< the request URI from the SIP message as a UtlString
                                       *   ONLY for use in debugging messages; all comparisons
                                       *   should be with requestUri */
      const Url& requestUri,          ///< the request URI from the SIP message as a Uri,
      const UtlString& method,        ///< Method of the request to redirect
      ContactList& contactList,       ///< Modifiable list containing the contact(s) that the request
                                      ///< should be redirected to.
      RequestSeqNo requestSeqNo,      ///< the request sequence number
      int redirectorNo,               ///< the identifier for this redirector
      class SipRedirectorPrivateStorage*& privateStorage, /**< the cell containing the pointer
                                                          * to the private storage object for
                                                          * this redirector, for this request. */
      ErrorDescriptor& errorDescriptor ///< the class that should be filled in by the redirector
                                       ///< to configure the SIP error reporting facilities when
                                       ///< returning ERROR
                               ) = 0;

   /**
    * The SipRedirectServer calls this method when the redirect plug-in does not have
    * a sufficiently high authority level to affect the redirection of the request
    * being presented.  This allows a plug-in to observe the request being redirected
    * and consult the ContactList in a read-only fashion.
    */
   virtual void observe(
      const SipMessage& message,      ///< the incoming SIP message
      const UtlString& requestString, /**< the request URI from the SIP message as a UtlString
                                       *   ONLY for use in debugging messages; all comparisons
                                       *   should be with requestUri */
      const Url& requestUri,          ///< the request URI from the SIP message as a Uri,
      const UtlString& method,        ///< Method of the request to observe
      const ContactList& contactList, ///< Read-only list of contacts to use for redirection
      RequestSeqNo requestSeqNo,      ///< the request sequence number
      int redirectorNo                ///< the identifier for this redirector
                               );

    /**
     *  Every redirector plug-in must implement this method and return
     *  a string representing its name.
     */
   virtual const UtlString& name( void ) const = 0;


   /**
    * Cancel processing of a request.
    *
    * If lookUp has ever returned SEARCH_PENDING for this request, the
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
    * invocation of lookUp that returns SEARCH_PENDING.
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

   friend class ContactListTest;

};

/**
 *  The ContactList is a class that is intented to be used by redirector plugins
 *  to manipulate the set of Contacts found for a given Request-URI look-up.
 *  The ContactList interface is very simple and offers 6 kinds of operations:
 *
 *  - add() operations    - allows a plug-in to add a new Contact entry to the list.
 *  - set() operations    - allows a plug-in to replace a Contact entry that is already
 *                          in the list.
 *  - get() operations    - allows a plug-in to get a Contact entry that is in the list.
 *  - remove() opetations - allows a plug-in to remove a specific Contact entry from the
 *                          list or all Contacts.
 *  - touch() opetation   - allows a plug-in to mark the list as having been modified
 *                          without altering any of its content.
 *  - entries() operation - allows a plug-in to get the number of Contacts contained in
 *                          the list.
 */
class ContactList
{
public:
   // ================================================================
   /** @name add methods
    *
    * allows a plug-in to add a new Contact entry to the list.
    */
   ///@{

   /**
    * method used to add a new Contact to the ContactList with the Contact being
    * specified as a Url object.
    *
    * @param contactUrl - Url containing the Contact to add to the list.
    * @param plugin     - Reference to the requesting plug-in - that information
    *                     is strictly used for logging purposes.
    * @return true if add succeeded; otherwise false.
    */
   bool add( const Url& contactUrl, const RedirectPlugin& plugin );
   /**
    * method used to add a new Contact to the ContactList with the Contact being
    * specified as a string.
    *
    * @param contact    - String containing the Contact to add to the list.
    * @param plugin     - Reference to the requesting plug-in - that information
    *                     is strictly used for logging purposes.
    * @return true if add succeeded; otherwise false.
    */
   bool add( const UtlString& contact, const RedirectPlugin& plugin );
   ///@}

   // ================================================================
   /** @name set methods
    *
    *  allows a plug-in to replace a Contact entry that is already
    *  in the list.
    */
   ///@{

   /**
    * method used to replace the content of an existing Contact in the list with the
    * replacing Contact specified as a Url object.
    *
    * @param index      - 0-based index of the Contact to replace in the list
    * @param contactUrl - Url containing the Contact to use for replacement
    * @param plugin     - Reference to the requesting plug-in - that information
    *                     is strictly used for logging purposes.
    * @return true if set succeeded; otherwise false.
    */
   bool set( size_t index, const Url& contactUrl, const RedirectPlugin& plugin );
   /**
    * method used to replace the content of an existing Contact in the list with the
    * replacing Contact specified as a string.
    *
    * @param index      - 0-based index of the Contact to replace in the list
    * @param contact    - string  containing the Contact to use for replacement
    * @param plugin     - Reference to the requesting plug-in - that information
    *                     is strictly used for logging purposes.
    * @return true if set succeeded; otherwise false.
    */
   bool set( size_t index, const UtlString& contact, const RedirectPlugin& plugin );
   ///@}

   // ================================================================
   /** @name get methods
    *
    * allows a plug-in to get a Contact entry that is in the list.
    */
   ///@{

   /**
    * method used to retrieve a specific Contact entry from the Contact list.
    *
    * @param index      - 0-based index of the Contact to retrieve
    * @param contactUrl - Url object that will receive the retrieved Contact
    * @param plugin     - Reference to the requesting plug-in - that information
    *                     is strictly used for logging purposes.
    * @return true if get succeeded; otherwise false.
    */
   bool get( size_t index, Url& contactUrl ) const;
   /**
    * method used to retrieve a specific Contact entry from the Contact list.
    *
    * @param index      - 0-based index of the Contact to retrieve
    * @param contact    - UtlString object that will receive the retrieved Contact
    * @param plugin     - Reference to the requesting plug-in - that information
    *                     is strictly used for logging purposes.
    * @return true if get succeeded; otherwise false.
    */
   bool get( size_t index, UtlString& contact ) const;

   ///@}

   // ================================================================
   /** @name remove methods
    *
    * allows a plug-in to remove a specific Contact entry from the
    * list or all Contacts.
    */
   ///@{
   /**
    * method used to remove a specific Contact entry from the Contact list.
    *
    * @param index      - 0-based index of the Contact to remove
    * @param plugin     - Reference to the requesting plug-in - that information
    *                     is strictly used for logging purposes.
    * @return true is removal succeeded; otherwise false.
    */
   bool remove( size_t index, const RedirectPlugin& plugin );
   /**
    * method used to remove all the Contact entries contained in the Contact list.
    *
    * @param plugin     - Reference to the requesting plug-in - that information
    *                     is strictly used for logging purposes.
    * @return true if removal succeeded; otherwise false.
    */
   bool removeAll( const RedirectPlugin& plugin );
   ///@}

   // ================================================================
   /** @name utility methods
    */
   ///@{
   /**
    * method used to mark the list as having been modified without actually
    * altering its content.
    *
    * @param plugin     - Reference to the requesting plug-in - that information
    *                     is strictly used for logging purposes.
    * @return true if removal succeeded; otherwise false.
    */
   void touch( const RedirectPlugin& plugin );

   /**
    * method used to get the number of Contacts contained in Contact list
    *
    * @return the list size.
    */
   size_t entries( void ) const;
   ///@}

private:
   ContactList( const UtlString& requestString /* for logging purposes */ );

   void resetWasModifiedFlag( void );
   bool wasListModified( void ) const;

   UtlString               mRequestString;
   bool                    mbListWasModified;
   std::vector<UtlString>  mContactList;

   friend class SipRedirectServer;
   friend class ContactListTest;
   friend class SipRedirectorTimeOfDayTest;
   friend class SipRedirectorPresenceRoutingTest;
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

/**
 *  ErrorDescriptor is a simple class that offers methods that let
 *  a redirector reporting an error customize the error reporting facilities
 *  to be used by the RedirectServer when constructing a failure SIP response.
 *  More specifically, it allows for the following:
 *
 *  -# Specify the status code and reason phrase to use in the status line
 *     of the response. If no status code and reason phrase are provided, 403 Forbidden is used.
 *     Example:
 *     @verbatim
                  SIP/2.0 400 Bad request because of missing so and so
                          ^^^ ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
           status code ____|                 |__________ reason phrase
       @endverbatim
 *
 *  -# Optionally specify the warning code and warning text to use in the Warning header of
 *     the response.  If no warning code and warning text are provided, no Warning header
 *     is included in the response.
 *     Example:
 *     @verbatim
                 Warning: 399 somedomain "Internal data structure corrupted"
                          ^^^            ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
          warning code ____|                           |______ warning text
       @endverbatim
 *
 *  -# Optionally Specify whether or not a copy of the offending request should be included
 *     in the failure response as a sipfrag body.
 *  -# Optionally Specify the content for the following fields :
 *      - Accept
 *      - Accept-Encoding
 *      - Accept-Language
 *      - Allow
 *      - Require
 *      - Retry-After
 *      - Unsupported
 *
 * @nosubgrouping
 */
class ErrorDescriptor
{
public:
   ErrorDescriptor();
   ~ErrorDescriptor();

   // ================================================================
   /** @name Setters
    *
    * The setting methods provide ways to specify the various elements of an error response.
    */
   ///@{

   /**
    *  Used to specify the SIP status code and reason phrase that describe
    *  best the error encountered.  These values will be used in the status
    *  line of the failure response that the Redirect Server will generate.
    *  If SetStatusLineData is not called by the redirector that reports the
    *  error, a status code of 403 and a reason phrase of "Forbidden" will
    *  be used.
    *
    *  @param statusCode: status code of the error response to send. See
    *                     .../sipXtackLib/include/net/SipMessage.h for
    *                     status code definitions.
    *                     Only 4xx, 5xx or 6xx status codes are accepted.
    *  @param reasonPhrase: phrase that will appear in response's status line.
    *                       see .../sipXtackLib/include/net/SipMessage.h for
    *                       default reason phrases definitions.
    *  @return true if operation succeeded, otherwise false.
    */
   bool setStatusLineData( const int statusCode, const UtlString& reasonPhrase );

   /**
    *  Used to specify that a Warning header should be added to the failure response
    *  generated by the Redirect Server.  If setWarningData is not called by the
    *  redirector that reports the error, the failure response generated by the
    *  Redirect Server will not contain a Warning header.
    *
    *  @param warningCode: Warning code to use in Warning header
    *                      Only 3xx warning codes are accepted.
    *                      Warning code selection should follow the
    *                      guidelines provided in section 20.43 of RFC 3261.
    *                      see .../sipXtackLib/include/net/SipMessage.h for
    *                      warning code definitions.
    *  @param warningText: Warning text to use in Warning header
    *                      see .../sipXtackLib/include/net/SipMessage.h for
    *                      default warning text definitions.
    *
    *  @return true if operation succeeded, otherwise false.
    */
   bool setWarningData( const int warningCode, const UtlString& warningText );

   /**
    *  Requests that the SIP request for which the redirector is returning
    *  a faan eror be included in the response as a sipfrag body.
    *
    *  By default, the failure response does not include the request.
    */
   void appendRequestToResponse( void );

   /**
    *  Requests that the SIP request for which the redirector is returning
    *  an error not be included in the response as a sipfrag body.
    *
    *  By default, the failure response does not include the request so this
    *  method has a net effect only if appendRequestToResponse() was previously
    *  called.
    */
   void dontAppendRequestToResponse( void );

   /**
    *   Setters for optional fields that can be included in the failure response.
    *   By default, if a a value is not set, the field will not be present in the
    *   response.  Setting the value of a given field overwrites its  previous value.
    */
   void setAcceptFieldValue        ( const UtlString& fieldValue );
   void setAcceptEncodingFieldValue( const UtlString& fieldValue );
   void setAcceptLanguageFieldValue( const UtlString& fieldValue );
   void setAllowFieldValue         ( const UtlString& fieldValue );
   void setRequireFieldValue       ( const UtlString& fieldValue );
   void setRetryAfterFieldValue    ( const UtlString& fieldValue );
   void setUnsupportedFieldValue   ( const UtlString& fieldValue );

   ///@}


   // ================================================================
   /** @name Getters
    *
    * The getting methods provide ways to read the various elements of an error response.
    */
   ///@{

   /**
    *  Used to obtain the status line data information set in the ErrorDescriptor
    *
    *  @param statusCode: variable that will receive status code
    *  @param reasonPhrase: variable that will receive reason phrase
    */
   void getStatusLineData( int& statusCode, UtlString& reasonPhrase ) const;

   /**
    *  Used to obtain the warning information set in the ErrorDescriptor
    *
    *  @param warningCode: variable that will receive warning code if set
    *  @param warningText: variable that will receive warning text if set
    *
    *  @return: true if warning data was set, otherwise false
    */
   bool getWarningData( int& warningCode, UtlString& warningText ) const;

   /**
    *  Mehtod used to find out if any warning data has been supplied
    *  via a call to setWarningData()
    *
    *  @return: true if warning data was set, otherwise false
    */
    bool isWarningDataSet( void ) const;

   /**
    *  Indicates whether or not the request should be appended to the response
    *  as a sipfrag body.
    */
   bool shouldRequestBeAppendedToResponse( void ) const;

   /**
    *  Getters for optional fields that can be included in the failure response.
    *  @param fieldValue: string that will receive field value
    *
    *  @return: true if a value was set for the field; otherwise false
    *
    */
   bool getAcceptFieldValue        ( UtlString& fieldValue ) const;
   bool getAcceptEncodingFieldValue( UtlString& fieldValue ) const;
   bool getAcceptLanguageFieldValue( UtlString& fieldValue ) const;
   bool getAllowFieldValue         ( UtlString& fieldValue ) const;
   bool getRequireFieldValue       ( UtlString& fieldValue ) const;
   bool getRetryAfterFieldValue    ( UtlString& fieldValue ) const;
   bool getUnsupportedFieldValue   ( UtlString& fieldValue ) const;

   ///@}

private:
   void setOptionalFieldValue( const UtlString& fieldName, const UtlString& fieldValue );
   bool getOptinalFieldValue ( const UtlString& fieldName,       UtlString& fieldValue ) const;

   int        mStatusCode;   ///< status code of the error response to send
   UtlString  mReasonPhrase; ///< phrase that will appear in response's status line

   int        mWarningCode;  ///< Warning code to use in Warning header
   UtlString  mWarningText;  ///< Warning text to use in Warning header

   bool       mAppendRequestToResponse; /**< flag that indicates whether the request
                                         *   that forced the error is to be copied
                                         *   in the response as a SIPFRAG body. */

   UtlHashMap mOptionalFieldsValues; /**< holds values for optional Retry-After,
                                      *   Require, Unsupported, Allow, Accept
                                      *   Accept-Encoding and Accept-Language fields */
};


#endif // _REDIRECTPLUGIN_H_
