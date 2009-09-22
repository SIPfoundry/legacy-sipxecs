//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _SipClientWriteBuffer_h_
#define _SipClientWriteBuffer_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <net/SipClient.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//: SipClient with additional machinery to buffer SipMessage's that
//: are awaiting writing out.
class SipClientWriteBuffer : public SipClient
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   SipClientWriteBuffer(OsSocket* socket,
                        SipProtocolServerBase* pSipServer,
                        SipUserAgentBase* sipUA,
                        const char* taskNameString,
                        UtlBoolean bIsSharedSocket);
     //:Default constructor

   virtual ~SipClientWriteBuffer();

/* ============================ MANIPULATORS ============================== */

   /// Handles an incoming message (from the message queue).
   virtual UtlBoolean handleMessage(OsMsg& rMsg);

   /// Not used in SipClientWriteBuffer.
   virtual void sendMessage(const SipMessage& message,
                            const char* address,
                            int port
                            ///< port to send to; must not be PORT_NONE
      );

   /// Insert a message into the buffer.
   virtual void insertMessage(SipMessage* message);

   /// Insert a keep alive message into the buffer.
   virtual void insertMessage(UtlString* keepAlive);

   /// Write as much of the buffered messages as can be written.
   virtual void writeMore();

   /// Empty the buffer, if requested, return all messages in the queue to the SipUserAgent
   /// as transport errors.
   virtual void emptyBuffer(bool reportError);

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

    virtual UtlContainableType getContainableType(void) const
    {
       return SipClientWriteBuffer::TYPE;
    };

    /** Class type used for runtime checking */
    static const UtlContainableType TYPE;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    /// The queue of messages waiting to be written.
    UtlSList mWriteBuffer;
    // mWriteQueued == !mWriteBuffer.isEmpty()

    /// The text of the first message in mWriteBuffer.
    UtlString mWriteString;
    // If mWriteBuffer is not empty, this is the text of the first message
    // in mWriteBuffer.  Otherwise, it is null.

    /// The number of bytes of mWriteString which have already been written.
    unsigned int mWritePointer;
    // 0 <= mWritePonter <= mWriteString.length()

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    SipClientWriteBuffer(const SipClientWriteBuffer& rSipClientWriteBuffer);
     //:disable Copy constructor

    SipClientWriteBuffer& operator=(const SipClientWriteBuffer& rhs);
     //:disable Assignment operator
};

#endif  // _SipClientWriteBuffer_h_
