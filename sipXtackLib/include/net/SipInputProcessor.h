/**
 *
 * Copyright (c) eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License (LGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 *
 */



#ifndef _SipInputProcessor_h_
#define _SipInputProcessor_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <net/SipMessage.h>
#include <utl/UtlContainable.h>


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/** This class defines the interface that must be implemented by any
 *  class that wishes to see and modify incoming SIP messages.  More specifically,
 *  The SipUserAgent class offers ways for an application to see and modify all
 *  incoming SIP messages.  A class wishing to perform such actions must be derived
 *  from  SipInputProcessor, provide an implementation for its
 *  handleOutputMessage() method and register itself as an input processor to
 *  a SipUserAgent via the method SipUserAgent::addSipOutputProcessor()
 */
class SipInputProcessor : public UtlContainable
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   //! Constructor for class SipInputProcessor
   //!
   //! \param priority   - Value representing the priority of the
   //!                     registering processor.  Lower numnerical values
   //!                     indicate higher priority.  When multiple
   //!                     processors register with the same SipUserAgent,
   //!                     they will be called in order of priority from
   //!                     the highest priority to the lowest. Each
   //!                     processor can see the modifications made by
   //!                     the previous processor(s).  Order of call for
   //!                     processors registrered with the same priority
   //!                     level is non-deterministic.  The processor that
   //!                     has registered with the highest priority will
   //!                     be called first and therefore will not see the
   //!                     message modifications made by other processors.
   //!                     On the other hand, the processor that has
   //!                     registered with the lowest priority will
   //!                     be called last but will see the message
   //!                     modifications made by all other processors
   //!                     before it.
   //!
   SipInputProcessor( uint priority );

   virtual ~SipInputProcessor();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

	/** Method that gets called by the SIP stack when an object has registered
     *  to a SipUserAgent instance as an input processor and that user agent is
     *  generating an incoming SIP request or response. When an input processor
     *  receives the callback, the SIP stack has already figured out where
     *  to send the message however the message has not yet been sent.
     *  handleOutputMessage() constitutes an application's last chance to see or
     *  modify a SIP message before it goes on the wire however an application
     *  must *NOT* alter message headers that would normally have an effect
     *  on its routing e.g. adding or modifying the top Route header
     *  or changing the Request-URI host part.  Making such alterations
     *  would have no bearing on the actual routing of the message given that
     *  the stack has already determined the next hop by the time the callback
     *  is made which would result in massive confusion and troubleshooting
     *  nightmares.
     *
     *  Threading considerations: Calls to handleOutputMessage() are made in the
     *  context of the thread that requested the send() action on the
     *  SipUserAgent.  As such, to be safe, applications should be built to expect
     *  calls to  handleOutputMessage() on multiple threads and provide
     *  mutual exclusion in the implementation of that method.
     *
     * Blocking considerations:  Given that callbacks are made in the
     * the context of the thread that is executing send() operations
     * on the user agent, special care must be taken in the
     * implementation of the handleOutputMessage() to avoid blocking calls.
     *
     * \param message - reference to message that is about to be sent
     * \param address - address to which message will be sent
     * \param port - port to which message will be sent
     *
     */
   virtual void handleOutputMessage( SipMessage& message,
                                     const char* address,
                                     int port ) = 0;

   uint getPriority( void ) const;

   virtual unsigned hash() const;

   virtual UtlContainableType getContainableType() const;

   static const UtlContainableType TYPE;    /**< Class type used for runtime checking */

/* ============================ INQUIRY =================================== */

   virtual int compareTo(UtlContainable const *) const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   uint mPriority;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipOutputProcessor_h_
