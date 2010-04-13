//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _MsgQueue_h
#define _MsgQueue_h

#include "rtcp/RtcpConfig.h"

// Include
#ifdef WIN32
#include "TLinkedList.h"
#pragma warning(disable:4275)
#elif defined(_VXWORKS)
#include <msgQLib.h>
#elif defined(__pingtel_on_posix__)
#include "os/OsMsgQ.h"
#else
#error Unsupported target platform.
#endif


#include "Message.h"


// Constant Message Type
const unsigned long NOFLUSH=0;
const unsigned long FLUSH=1;
const unsigned long ONE_SECOND=1000;


/*|><|************************************************************************
*
* Class Name:   CMsgQueue
*
* Inheritance:
*
* Methods:
*
* Attributes:
*
* Description:  CMsgQueue is a thread safe class which shall provide
*               general purpose message delivery, storage, retrieval and
*               processing services.  At the heart of the class shall be
*               a FIFO linked list for storing messages, an event object
*               and an apartment style thread.  The apartment thread
*               shall block on an event object waiting for incoming
*               messages to be posted to the linked list.  When a
*               message is posted to the list, the event object shall be
*               set causing the thread to wake up.  Upon waking up, the
*               thread shall take the first message off the linked list
*               and call a general purpose processing method defined as
*               a pure virtual.  The processing method shall accept a
*               message type and an opaque pointer as arguments.  It
*               shall be the responsibility of the user to implement
*               this method and the message processing logic associated
*               with their object.
*
*               The CMsgQueue thread shall continue removing messages
*               from the linked list FIFO until none remain.  When the
*               linked list is emptied, the event object shall be
*               cleared and the CMsgQueue thread shall block waiting for
*               the next message   to be delivered.
*
*               The CMessageQueue shall implement a set of private
*               services for performing internal management of the
*               linked list, event object and thread in addition to
*               public services for posting, retreiving and flushing
*               messages.
*
*
*
*
************************************************************************|<>|*/

class CMsgQueue
#ifdef WIN32
      : public CTLinkedList<CMessage *>   // Inherit Linked List Services
#endif
{

  public:   // Public Member Functions

/*|><|************************************************************************
*
* Method Name:  CMsgQueue - Constructor
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      None
*
* Description:  The CMsgQueue constructor shall perform some very basic
*               setup which shall include the initialization of internal
*               head, tail and entry counter data members.  All other
*               failure prone setup shall be performed in the
*               Initialize() method.
*
* Usage Notes:
*
*
************************************************************************|<>|*/
      CMsgQueue();


/*|><|************************************************************************
*
* Method Name:  CMsgQueue - Destructor
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      None
*
* Description:  The destructor for the CMsgQueue object.  This method
*               shall have the responsibility of suspending message
*               processing and flushing all messages held within the
*               message queue.  The flush shall return all allocated
*               memory back to the memory pool through using the
*               services provide in CMemoryPool.  The destructor shall
*               also arrange for the graceful termination of the
*               apartment thread through signalling the terminate event.
*
* Usage Notes:
*
*
************************************************************************|<>|*/
      virtual ~CMsgQueue();


/*|><|************************************************************************
*
* Method Name:  Initialize
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      bool
*
* Description:  The Initialize method shall perform the fault suceptible
*               initialization associated with this class.  The
*               initialization sequence shall include:
*
*               - Creation of the Message and Terminate Event objects.
*               _ Creation of the Apartment thread
*
*               All of the above initialization steps must succeed for
*               Initialize to return success.
*
* Usage Notes:
*
*
************************************************************************|<>|*/
      virtual bool Initialize();

/*|><|************************************************************************
*
* Method Name:  Shutdown
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      bool
*
* Description:  The Shutdown method shall trigger all MsgQueue processing to
*               cease.  This is done as part of a two phase termination
*               consisting of succession and destruction.
*
* Usage Notes:
*
*
************************************************************************|<>|*/
      virtual bool Shutdown();


/*|><|************************************************************************
*
* Method Name:  Post
*
*
* Inputs:       CMessage *
*
* Outputs:      None
*
* Returns:      bool
*
* Description:  The Post method provides a general purpose
*               facility to post a message of a particular type with
*               optional arguments to the CMsgQueue linked list.
*
*               Post Message shall set the message event object to inform
*               the apartment thread of the arrival of a new message.
*
* Usage Notes:
*
*
************************************************************************|<>|*/
      bool Post(CMessage *pMessage);


  protected:    // Protected Member Functions

/*|><|************************************************************************
*
* Method Name:  ProcessMessage
*
*
* Inputs:       CMessage *
*
* Outputs:      None
*
* Returns:      bool
*
* Description:  This is a pure virtual function provide by CMsgQueue as
*               a means for dispatching messages removed from the queue
*               for processing.  It shall be the responsibility of the
*               derived class to implement this method.
*
* Usage Notes:
*
*
************************************************************************|<>|*/
      virtual bool ProcessMessage(CMessage *)=0;



/*|><|************************************************************************
*
* Method Name:  TerminateProcessing
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      bool
*
* Description:  Terminates thread processing for this object.
*
* Usage Notes:
*
*
************************************************************************|<>|*/
      void TerminateProcessing(unsigned long dwMode=NOFLUSH);

/*|><|************************************************************************
*
* Method Name:  FlushMessages
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      bool
*
* Description:  Removes all messages from the CMsgQueue and releases the
*               memory associated with each entry.
*
* Usage Notes:
*
*
************************************************************************|<>|*/
      void FlushMessages(void);

  private:

#if defined(_WIN32)
/*|><|************************************************************************
*
* Method Name:  CreateMessageEvent
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      bool
*
* Description:  This method shall create a manually resetable event object to
*               be used for informing the MsgQueue object of the arrival of a
*               new message.
*
* Usage Notes:
*
*
************************************************************************|<>|*/
      bool CreateMessageEvent();

/*|><|************************************************************************
*
* Method Name:  DestroyMessageEvent
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      None
*
* Description:  This method shall destroy a manually resetable event object
*               used for informing the MsgQueue object of the arrival of a new
*               message.
*
* Usage Notes:
*
*
************************************************************************|<>|*/
      void DestroyMessageEvent();

/*|><|************************************************************************
*
* Method Name:  CreateThreadEvents
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      bool
*
* Description:  This method shall create a manually resetable event object to
*               be used for informing the parent of thread state changes.
*
* Usage Notes:
*
*
************************************************************************|<>|*/
      bool CreateThreadEvents();

/*|><|************************************************************************
*
* Method Name:  DestroyThreadEvents
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      None
*
* Description:  This method shall destroy a manually resetable event object
*               used for informing the parent of thread state changes.
*
* Usage Notes:
*
*
************************************************************************|<>|*/
      void DestroyThreadEvents();

/*|><|************************************************************************
*
* Method Name:  CreateMessageThread
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      bool
*
* Description:  This is a private method which shall create an apartment style
*               processing thread for the purpose dispatching and processing
*               messages that have been posted to a client's message list.
*
* Usage Notes:
*
*
************************************************************************|<>|*/
      bool CreateMessageThread();

/*|><|************************************************************************
*
* Method Name:  InitMessageThread
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      None
*
* Description:  This is a private method which shall initialize the message
*               thread.  Initialization shall involve the creation and setting
*               of a thread event used to signal the main thread of state
*               change.  Upon successful event creation, the init method shall
*               call the MessageLoop() which shall pend on and process incoming
*               messages.  A failed initialization shall result in thread
*               termination.
*
*
* Usage Notes:
*
*
************************************************************************|<>|*/
      static unsigned int _stdcall InitMessageThread(void * vpArgument);


/*|><|************************************************************************
*
* Method Name:  TerminateMessaging
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      bool
*
* Description:  Terminates any further message processing activity
*               for this object.
*
* Usage Notes:
*
*
************************************************************************|<>|*/
      void TerminateMessaging(unsigned long dwMode);



/*|><|************************************************************************
*
* Method Name:  TerminateMessageThread
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      None
*
* Description:  This is a private method which shall shall terminate the
*               message thread.  In so doing, it shall destroy both message and
*               thread events used to signal state change as well as remove all
*               messages which may remain on the message queue.
*
*
* Usage Notes:
*
*
************************************************************************|<>|*/
      void TerminateMessageThread();


#elif defined(_VXWORKS)

/**
 *
 * Method Name:  CreateMessageTask()
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     bool
 *
 * Description:
 *
 * Usage Notes:
 *
 */
bool CMsgQueue::CreateMessageTask();


/*|><|************************************************************************
*
* Method Name:  InitMessageThread
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      None
*
* Description:  This is a private method which shall initialize the message
*               thread.  Initialization shall involve the creation and setting
*               of a thread event used to signal the main thread of state
*               change.  Upon successful event creation, the init method shall
*               call the MessageLoop() which shall pend on and process
*               incoming messages.  A failed initialization shall result in
*               thread termination.
*
*
* Usage Notes:
*
*
************************************************************************|<>|*/
      static int InitMessageThread(int argument1, int arg2, int arg3,
                   int arg4, int arg5, int arg6, int arg7, int arg8,
                   int arg9, int arg10);

#elif defined(__pingtel_on_posix__)
/* nothing needed? */
#else
#error Unsupported target platform.
#endif

/*|><|************************************************************************
*
* Method Name:  MessageLoop
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      None
*
* Description:  This is a private method which shall pend on incoming messages
*               delivered to the inheriting object.  Signals from either the
*               Message Event or Thread event shall cause the MessageLoop to
*               leave its wait state and begin processing.  If a message event
*               is detected, the MessageLoop shall iterate through each message
*               on the list and dispatch those messages to the client defined
*               ProcessMessage() method.  If a terminate event is detected, the
*               message processing loop shall be exited and the thread shall be
*               gracefully terminated.
*
*
* Usage Notes:
*
*
************************************************************************|<>|*/
      void MessageLoop();



  private:

#if defined(_WIN32)
/*|><|************************************************************************
*
* Attribute Name:   m_hMessageThread
*
* Type:             HANDLE
*
* Description:      The thread handle is used to store the handle generated
*                   in response to a successful thread creation request.
*
************************************************************************|<>|*/
      HANDLE m_hMessageThread;

/*|><|************************************************************************
*
* Attribute Name:   m_hMessageEvent
*
* Type:             HANDLE
*
* Description:      The message event is a manually resetable event which
*                   shall be used to signal the addition of a new message in
*                   the Message Queue.
*
************************************************************************|<>|*/
      HANDLE m_hMessageEvent;

/*|><|************************************************************************
*
* Attribute Name:   m_hThreadEvent
*
* Type:             HANDLE
*
* Description:      The thread event shall be used by used to signal thread
*               creation and shall trigger the onset of thread termination.
*
************************************************************************|<>|*/
      HANDLE m_hThreadEvent;

/*|><|************************************************************************
*
* Attribute Name:   m_hTerminateEvent
*
* Type:             HANDLE
*
* Description:  The terminate event shall be used to signal the  gracefully
*               termination of the apartment thread to the waiting destructor.
*
************************************************************************|<>|*/
      HANDLE m_hTerminateEvent;

#elif defined(_VXWORKS)

/*|><|************************************************************************
*
* Attribute Name:   m_ulMsgQID
*
* Type:             MSG_Q_ID
*
* Description:      The message queue handle used to identify the vxWorks FIFO
*                   on which events messages are pushed and popped.
*
************************************************************************|<>|*/
      MSG_Q_ID    m_ulMsgQID;

/*|><|************************************************************************
*
* Attribute Name:   m_iMsgTaskID
*
* Type:         int
*
* Description:      The task ID for the thread used to process messages that
*           are pending on the FIFO.
*
************************************************************************|<>|*/
      int        m_iMsgTaskID;

#elif defined(__pingtel_on_posix__)
      OsMsgQ * m_pMsgQ;
      static void * InitMessageThread(void * argument1);
#else
#error Unsupported target platform.
#endif

};

#endif
