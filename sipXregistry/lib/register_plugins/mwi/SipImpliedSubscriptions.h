//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef SIPIMPLSUB_H
#define SIPIMPLSUB_H

/**
 * SIP Registrar Implied Subscriptions
 *
 * The SipImpliedSubscriptions::takeAction method is invoked by
 *   the Registrar whenever a REGISTER request succeeds.  This object determines
 *   whether or not the register needs to generate any SUBSCRIBE requests on behalf
 *   of the originator of the REGISTER, and if so, creates and sends those SUBSCRIBE
 *   requests.
 */

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsBSem.h"
#include "utl/UtlString.h"
#include "sipXecsService/SipNonceDb.h"
#include "net/SipUserAgent.h"
#include "registry/RegisterPlugin.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class ImpliedSubscriptionUserAgent;

/// Plugin::Factory method to get the instance of this plugin.
extern "C" RegisterPlugin* getRegisterPlugin(const UtlString& name);

/// Create message waiting subscriptions for specified User Agents
/**
 * This plugin creates a SUBSCRIBE request for message waiting indications
 * on behalf of any REGISTER requests whose UserAgent header matches one
 * of the configured UA regular expressions.  This supports phones that will
 * accept NOTIFY messages for this but do not send the SUBSCRIBE request.
 */
class SipImpliedSubscriptions : public RegisterPlugin
{
public:

    /** dtor */
    virtual ~SipImpliedSubscriptions();

    virtual void readConfig( OsConfigDb& configDb );

    virtual void takeAction( const SipMessage&   registerMessage  ///< the successful registration
                            ,const unsigned int  registrationDuration /**< the actual allowed
                                                                       * registration time (note
                                                                       * that this may be < the
                                                                       * requested time). */
                            ,SipUserAgent*       sipUserAgent     /**< to be used if the plugin
                                                                   *   wants to send any SIP msg */
                            );

protected:
    /*
     * The needsImpliedSubscription and buildSubscribeRequest methods
     * are broken out separately and are virtual to facilitate unit testing.
     */
    friend class SipImpliedSubscriptionsTest;

    /// Is the UserAgent for this registration configured for implied subscriptions?
    virtual
    bool needsImpliedSubscription( const SipMessage& registerMessage );

    /// Create a subscription request based on this registration.
    virtual
    void buildSubscribeRequest( const SipMessage& registerMessage ///< the registration
                               ,int duration                      ///< seconds for the subscription
                               ,SipMessage& subscribeRequest      ///< returned request
                               ,UtlString&  callId   ///< callid from the request
                               ,UtlString&  fromTag  ///< From tag parameter from the request
                               ,UtlString&  fromUri  ///< identity from the request
                               );
    /**<
     * The callId, fromTag, and fromUri are the same as those in the subscribeRequest,
     * but are passed separately so that they can be used in authentication.
     */

    /// attempt to add xxx-authorization header to the subscribe request.
    void addAuthorization( const SipMessage& registerMessage ///< to get the user and realm from
                           ,SipMessage& subscribeRequest      ///< the request to authenticate
                           ,UtlString&  callId                ///< callid from the request
                           ,UtlString&  fromTag               ///< From tag parameter from the request
                           ,UtlString&  fromUri               ///< identity from the request
                           );

private:
    friend class ImpliedSubscriptionUserAgent;

    static OsBSem*                  mpSingletonLock;
    static SipImpliedSubscriptions* mpSingleton;

    // String to use in place of class name in log messages:
    // "[instance] class".
    UtlString mLogName;

    /**
     * Constructor is hidden so that only the factory can instantiate it.
     */
    SipImpliedSubscriptions(const UtlString& name);
    friend RegisterPlugin* getRegisterPlugin(const UtlString& name);

/**  ConfigPrefix -
 *     The configuration may contain any number of directives
 *     that begin with this prefix - the value of each is a regular
 *     expression that matches a User-Agent header value.  When a matching
 *     REGISTER request is received,
 */
    static const char ConfigPrefix[];

};

#endif // SIPIMPLSUB_H
