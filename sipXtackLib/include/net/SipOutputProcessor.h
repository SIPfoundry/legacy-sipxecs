//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _SipOutputProcessor_h_
#define _SipOutputProcessor_h_

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
 *  class that wishes to see and modify outgoing SIP messages.  More specifically,
 *  The SipUserAgent class offers ways for an application to see and modify all
 *  outgoing SIP messages.  A class wishing to perform such actions must be derived
 *  from  SipOutputProcessor, provide an implementation for its  
 *  handleOutputMessage() method and register itself as an output processor to
 *  a SipUserAgent via the method SipUserAgent::addSipOutputProcessor()
 */
class SipOutputProcessor : public UtlContainable
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   //! Constructor for class SipOutputProcessor
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
   SipOutputProcessor( uint priority );
   
   virtual ~SipOutputProcessor();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

	/** Method that gets called by the SIP stack when an object has registered 
     *  to a SipUserAgent instance as an output processor and that user agent is 
     *  generating an outgoing SIP request or response. When an output processor
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
