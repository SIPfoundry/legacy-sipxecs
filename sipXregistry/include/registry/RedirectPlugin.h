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
class ErrorDescriptor;

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
      LOOKUP_ERROR,         /**< the redirector reported an error.  The ErrorDescriptor
                             *   can contain more information about the error condition. */
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
    * If it returns LOOKUP_ERROR, the redirector should most likely have
    * logged message(s) at ERR level giving the details of the problem.
    * It should also fill in the supplied ErrorDesriptor to configure the 
    * proper SIP error reporting facilities to be used by SipRedirectServer
    * when crafting the SIP failure response.  Redirectors that return
    * LOOKUP_SUCCESS *MUST NOT* modify the supplied ErrorDesriptor.
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
      class SipRedirectorPrivateStorage*& privateStorage, /**< the cell containing the pointer
                                                          * to the private storage object for
                                                          * this redirector, for this request. */
      ErrorDescriptor& errorDescriptor ///< the class that should be filled in by the redirector
                                       ///< to configure the SIP error reporting facilities when
                                       ///< returning LOOKUP_ERROR
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

/** 
 *  ErrorDescriptor is a simple class that offers methods that let
 *  a redirector reporting an error customize the error reporting facilities
 *  to be used by the RedirectServer when constructing a failure SIP response.
 *  More specifically, it allows for the following:
 *  #1 Specify the status code and reason phrase to use in the status line
 *     of the response.
 *     Example:   SIP/2.0 400 Bad request because of missing so and so
 *                        ^^^ ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 *         status code ____|                 |__________ reason phrase
 *     Note: If no status code and reason phrase are provided, 403 Forbidden is used.
 * 
 *  #2 Optionally specify the warning code and warning text to use in the Warning header of 
 *     the response.
 *     Example:  Warning: 399 somedomain "Internal data structure corrupted"
 *                        ^^^            ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 *        warning code ____|                           |______ warning text
 *     Note: if no warning code and warning text are provided, no Warning header 
 *           will be included in the response.
 *  #3 Optionally Specify whether or not a copy of the offending request should be included
 *     in the failure response as a sipfrag body.
 *  #4 Optionally Specify the content for the following fields :
 *      - Accept
 *      - Accept-Encoding
 *      - Accept-Language
 *      - Allow
 *      - Require
 *      - Retry-After
 *      - Unsupported
 */
class ErrorDescriptor      
{
public:
   ErrorDescriptor();
   ~ErrorDescriptor();
   
   /// ///////// ///
   ///  SETTERS  ///
   /// ///////// ///

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
   
   /// ///////// ///
   ///  GETTERS  ///
   /// ///////// ///
   
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

private:
   void setOptionalFieldValue( const UtlString& fieldName, const UtlString& fieldValue );
   bool getOptinalFieldValue ( const UtlString& fieldName,       UtlString& fieldValue ) const;
   
   int        mStatusCode;   // status code of the error response to send
   UtlString  mReasonPhrase; // phrase that will appear in response's status line
   
   int        mWarningCode;  // Warning code to use in Warning header
   UtlString  mWarningText;  // Warning text to use in Warning header
   
   bool       mAppendRequestToResponse; // flag that indicates whether the request
                                        // that forced the error is to be copied
                                        // in the response as a SIPFRAG body.
   
   UtlHashMap mOptionalFieldsValues; // holds values for optional Retry-After,
                                     // Require, Unsupported, Allow, Accept
                                     // Accept-Encoding and Accept-Language fields
};


#endif // _REDIRECTPLUGIN_H_
