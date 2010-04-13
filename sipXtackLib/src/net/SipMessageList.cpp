//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <net/SipMessageList.h>
#include <os/OsSysLog.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
//#define TEST_PRINT
/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipMessageList::SipMessageList()
{
}

// Copy constructor
SipMessageList::SipMessageList(const SipMessageList& rSipMessageList)
{
}

// Destructor
SipMessageList::~SipMessageList()
{
    while (SipMessage* pMsg = (SipMessage*) messageList.pop())
    {
        delete pMsg;
    }
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipMessageList&
SipMessageList::operator=(const SipMessageList& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/* ============================ ACCESSORS ================================= */

SipMessage* SipMessageList::getResponseFor(SipMessage* request)
{
    int iteratorHandle  = messageList.getIteratorHandle();
    SipMessage* message;

    while ((message = (SipMessage*) messageList.next(iteratorHandle)))
    {
        if(message->isResponseTo(request))
        {
            break;
        }
    }
    messageList.releaseIteratorHandle(iteratorHandle);
    return(message);
}

SipMessage* SipMessageList::getRequestFor(SipMessage* response)
{
    int iteratorHandle = messageList.getIteratorHandle();
    SipMessage* message;

    while ((message = (SipMessage*) messageList.next(iteratorHandle)))
    {
        if(response->isResponseTo(message))
        {
            break;
        }
    }
    messageList.releaseIteratorHandle(iteratorHandle);
    return(message);
}

SipMessage* SipMessageList::getDuplicate(SipMessage* message,
                                         UtlBoolean responseCodesMustMatch)
{
    int iteratorHandle = messageList.getIteratorHandle();
    SipMessage* listMessage;

    while ((listMessage = (SipMessage*) messageList.next(iteratorHandle)))
    {
        if(listMessage->isSameMessage(message, responseCodesMustMatch))
        {
            break;
        }
    }

    messageList.releaseIteratorHandle(iteratorHandle);
    return(listMessage);
}

SipMessage* SipMessageList::getAckFor(SipMessage* inviteResponse)
{
    int iteratorHandle = messageList.getIteratorHandle();
    SipMessage* listMessage;

    while ((listMessage = (SipMessage*) messageList.next(iteratorHandle)))
    {
        if(listMessage->isAckFor(inviteResponse))
        {
            break;
        }
    }

    messageList.releaseIteratorHandle(iteratorHandle);
    return(listMessage);
}
//SDUA
SipMessage* SipMessageList::getInviteFor(SipMessage* cancelRequest)
{
    int iteratorHandle = messageList.getIteratorHandle();
    SipMessage* listMessage;

    while ((listMessage = (SipMessage*) messageList.next(iteratorHandle)))
    {
        if(listMessage->isInviteFor(cancelRequest))
        {
            break;
        }
    }

    messageList.releaseIteratorHandle(iteratorHandle);
    return(listMessage);
}

int
SipMessageList::getListSize()
{
    return messageList.getCount();
}

int
SipMessageList::getIterator()
{
    return messageList.getIteratorHandle();
}

SipMessage* SipMessageList::getSipMessageForIndex( int iteratorHandle )
{
    return (SipMessage*) messageList.next(iteratorHandle);
}

void
SipMessageList::releaseIterator(int iteratorHandle)
{
    messageList.releaseIteratorHandle(iteratorHandle);
}

SipMessage*
SipMessageList::isSameFrom( const Url& fromUrl )
{
    int iteratorHandle = messageList.getIteratorHandle();
    SipMessage* listMessage = NULL;

    // @JC More efficient this was in the main loop
    int newPort;
    UtlString newAddress, newProtocol, newUser;
    SipMessage::parseAddressFromUri(
        fromUrl.toString(), &newAddress, &newPort, &newProtocol, &newUser);

    // Iterate through the list of SipMessages, extracting the from field
    // fields from each of the messages and compare them to the
    // one passed in, if same returns pointer to match in the list
    while ((listMessage = (SipMessage*) messageList.next(iteratorHandle)))
    {
        UtlString from;
        listMessage->getFromField(&from);
        if(!from.isNull())
        {
            int port;
            UtlString address, protocol, user;
            SipMessage::parseAddressFromUri(
                from, &address, &port, &protocol, &user);

            if ( address.compareTo(newAddress) == 0
                && protocol.compareTo(newProtocol) == 0
                && user.compareTo(newUser) == 0
                && port == newPort)
            {
                break;
            }

        }
    }

    messageList.releaseIteratorHandle( iteratorHandle );

    return(listMessage);
}

SipMessage*
SipMessageList::isSameTo( const Url& toUrl )
{
    int iteratorHandle = messageList.getIteratorHandle();
    SipMessage* listMessage = NULL;

    UtlString newTo = toUrl.toString();

    // @JC Moved here from inner loop
    int newPort;
    UtlString newAddress, newProtocol, newUser;
    SipMessage::parseAddressFromUri(
        newTo, &newAddress, &newPort, &newProtocol, &newUser);

    // Iterate through the list of SipMessages, extracting the 'to' field
    // fields from each of the messages and compare them to the
    // one passed in, if same returns pointer to match in the list
    while ((listMessage = (SipMessage*) messageList.next(iteratorHandle)))
    {
        UtlString to;
        listMessage->getToField(&to);
        if(!to.isNull())
        {
            int port;
            UtlString address, protocol, user;
            SipMessage::parseAddressFromUri(
                to, &address , &port, &protocol, &user);
            if ( address.compareTo(newAddress) == 0
                && protocol.compareTo(newProtocol) == 0
                && user.compareTo(newUser) == 0
                && port == newPort)
            {
                break;
            }
        }
    }

    messageList.releaseIteratorHandle(iteratorHandle);

    return(listMessage);
}

SipMessage*
SipMessageList::isSameCallId( const UtlString& newCallId )
{
    int iteratorHandle = messageList.getIteratorHandle();
    SipMessage* listMessage = NULL;

    while ((listMessage = (SipMessage*) messageList.next(iteratorHandle)))
    {
        UtlString callId;
        listMessage->getCallIdField(&callId);
        if(!callId.isNull())
        {
            if ( newCallId.compareTo(callId) == 0)
            {
                break;
            }
        }
    }

    messageList.releaseIteratorHandle(iteratorHandle);

    return(listMessage);
}

UtlBoolean
SipMessageList::remove( SipMessage* message )
{
    int iteratorHandle = messageList.getIteratorHandle();
    SipMessage* listMessage;


    while ((listMessage = (SipMessage*) messageList.next(iteratorHandle)))
    {
      #ifdef TEST_PRINT
         UtlString listMessageStr;
         ssize_t listMessageLen;
         listMessage->getBytes(&listMessageStr, &listMessageLen);
         osPrintf("\n-------\nSipMessageList::remove List message %s \n++++++\n",listMessageStr.data());

         UtlString MessageStr;
         ssize_t MessageLen;
         message->getBytes(&MessageStr, &MessageLen);
         osPrintf("\nSipMessageList::remove parameter Message %s \n-------\n",MessageStr.data());
      #endif

        if(listMessage == message)
        {
#ifdef TEST_PRINT
            osPrintf("SipMessageList::remove removing message\n");
#endif
            messageList.remove(iteratorHandle);

            break;
        }
    }

    messageList.releaseIteratorHandle(iteratorHandle);
    return(listMessage != NULL);
}

void SipMessageList::add(SipMessage* message)
{
    messageList.push(message);
}

void SipMessageList::remove(int iteratorHandle)
{
    messageList.remove(iteratorHandle);
}

void SipMessageList::removeOldMessages(long oldTime, UtlBoolean deleteMessages)
{
    int iteratorHandle = messageList.getIteratorHandle();
    SipMessage* message;
//    osPrintf("**********************************\n");
//    osPrintf("SipMessageList::removeOldMessages\n");
//    osPrintf("**********************************\n");
    while ((message = (SipMessage*) messageList.next(iteratorHandle)))
    {
        if(message->getTransportTime() < oldTime)
        {
#ifdef TEST_PRINT
            osPrintf("SipMessageList::removeOldMessages removing message dated: %d before: %d\n",
                message->getTransportTime(), oldTime);
#endif
            messageList.remove(iteratorHandle);
            if(deleteMessages)
            {
//                osPrintf("**********************************\n");
//                osPrintf("SipMessageList::removeOldMessages REMOVED: %X\n",message);
//                osPrintf("**********************************\n");
                delete message;
                message = NULL;
            }
        }
    }

    messageList.releaseIteratorHandle(iteratorHandle);
}

void SipMessageList::toString(UtlString& listDumpString)
{
    int iteratorHandle = messageList.getIteratorHandle();

    listDumpString.remove(0);

    SipMessage* listMessage;
    UtlString listMessageBytes;
    ssize_t msgLen;

    while ((listMessage = (SipMessage*) messageList.next(iteratorHandle)))
    {
        listMessageBytes.remove(0);
        listMessage->getBytes(&listMessageBytes, &msgLen);
        listDumpString.append("?????????????????????????????\n");
        listDumpString.append(listMessageBytes);
    }
    listDumpString.append("End ?????????????????????????????\n");
    messageList.releaseIteratorHandle(iteratorHandle);
    listMessageBytes.remove(0);
}


#define DBG_TBL_MAX_URI   30
#define DBG_TBL_MAX_FIELD 10
#define DBG_TBL_FORMAT_STR "%-30s %-30s %-10s %-10d %s\n"
void SipMessageList::printDebugTable()
{
    SipMessage* pMsg;

    int iteratorHandle = messageList.getIteratorHandle();

    OsSysLog::add(FAC_REFRESH_MGR, PRI_DEBUG,
                  "\nDump of SipMessageList (instance %8p)\n", this);
    OsSysLog::add(FAC_REFRESH_MGR, PRI_DEBUG,
                  "To                             CallId                         Method     CSeq       Request\n");
    OsSysLog::add(FAC_REFRESH_MGR, PRI_DEBUG,
                  "------------------------------ ------------------------------ ---------- ---------- ----------\n");

    while ((pMsg = (SipMessage*) messageList.next(iteratorHandle)))
        {
        UtlString  toURI;
        UtlString  callId;
        int       iCseq;
        UtlString  method;
        UtlBoolean bIsResponse;

        pMsg->getToUri(&toURI);
        pMsg->getCallIdField(&callId);
        pMsg->getCSeqField(&iCseq, &method);
        bIsResponse = pMsg->isResponse();

        // Shorten fields if necessary
        if (toURI.length() > DBG_TBL_MAX_URI)
            toURI.remove(DBG_TBL_MAX_URI);

        if (callId.length() > DBG_TBL_MAX_URI)
            callId.remove(DBG_TBL_MAX_URI);

        if (method.length() > DBG_TBL_MAX_FIELD)
            method.remove(DBG_TBL_MAX_FIELD);

        OsSysLog::add(FAC_REFRESH_MGR, PRI_DEBUG,
                      DBG_TBL_FORMAT_STR,
                      toURI.data(), callId.data(),
                      method.data(), iCseq, bIsResponse ? "FALSE" : "TRUE");
    }

    messageList.releaseIteratorHandle(iteratorHandle);
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
