//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _SipPublishServerEventStateCompositor_h_
#define _SipPublishServerEventStateCompositor_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES

#include <os/OsDefs.h>
#include <os/OsMutex.h>
#include <utl/UtlDefs.h>
#include <utl/UtlHashMap.h>


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// FORWARD DECLARATIONS
class SipMessage;
class UtlString;

// TYPEDEFS

//! Class for specializing the handling of SIP Events in SipPublishServer
/*! This class provides the default behavior for SIP event packages
 *  handled by SipPublishServer.  Event packages which wish to change
 *  or extend the default behavior should derived from this class and
 *  override the behavior of methods implemented by this class.
 *
 * \par
 */
class SipPublishServerEventStateCompositor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:



/* ============================ CREATORS ================================== */

    //! Default Dialog constructor
    SipPublishServerEventStateCompositor();


    //! Destructor
    virtual
    ~SipPublishServerEventStateCompositor();


/* ============================ MANIPULATORS ============================== */

    //! Determine what the resourceId ad eventTypeKey should be for this PUBLISH request
    /*! The default behavior is to use the identify from the request URI
     *  as the resourceId and the event type token from the Event header
     *  as the eventTypeKey.  Some event packages may wish to override
     *  this (e.g. the host part of the resourceID, which will usually be an
     *  IP address may make sense in some cases to be substituted with the 
     *  domain name.  In some event packages, the content of the event state
     *  information will vary based upon some Event header parameters, in
     *  which cases it may make sense to include that event header parameter
     *  in a consistant order and format in the eventTypeKey.)
     */
    virtual UtlBoolean getKeys(const SipMessage& publishRequest,
                               UtlString& resourceId,
                               UtlString& eventTypeKey);

    //! Determine if the given PUBLISH request is authenticated
    /*! Default behavior is to not require any authentication.
     */
    virtual UtlBoolean isAuthenticated(const SipMessage& publishRequest,
                                       const UtlString& resourceId,
                                       const UtlString& eventTypeKey,
                                       SipMessage& publishResponse);

    //! Determine if the given PUBLISH request is authorized
    /*! Default behavior is to allow any request to subscribe
     */
    virtual UtlBoolean isAuthorized(const SipMessage& publishRequest,
                                   const UtlString& resourceId,
                                   const UtlString& eventTypeKey,
                                   SipMessage& publishResponse);


/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    //! Copy constructor NOT ALLOWED
    SipPublishServerEventStateCompositor(const SipPublishServerEventStateCompositor& rSipPublishServerEventStateCompositor);

    //! Assignment operator NOT ALLOWED
    SipPublishServerEventStateCompositor& operator=(const SipPublishServerEventStateCompositor& rhs);


};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipPublishServerEventStateCompositor_h_
