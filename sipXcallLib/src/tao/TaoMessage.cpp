//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifdef TEST
#include "utl/UtlMemCheck.h"
#endif

#include "net/HttpMessage.h"
#include "tao/TaoMessage.h"

#ifdef TAOMSG_DEBUG
unsigned int TaoMessage::mMsgCnt = 0;
unsigned int getTaoMsgCnt()
{
        return TaoMessage::mMsgCnt;
}
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
TaoMessage::TaoMessage()
        : OsMsg(OsMsg::TAO_MSG, UNSPECIFIED)
{
        mMessageQueueHandle = 0;
        mBodyLength = 0;

#ifdef USE_HTTPMSG
        mHttpMsg.setContentType(TAO_MESSAGE_CONTENT_TYPE);
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

        sprintf(buff, "%d", OsMsg::TAO_MSG);
        mHttpMsg.addHeaderField(TAO_MESSAGE_TYPE_FIELD, buff);
        mHttpMsg.addHeaderField(TAO_MESSAGE_SUBTYPE_FIELD, "0"/*UNSPECIFIED*/);
#endif

#ifdef TAOMSG_DEBUG
        mHttpMsg.logTimeEvent("DEFAULT CONSTRUCTOR ");
        mMsgCnt++;
#endif

        mbDirty = TRUE ;
}

TaoMessage::TaoMessage(TaoListenerEventMessage& rEventMessage, TaoObjHandle hSocket)
        : OsMsg(OsMsg::TAO_MSG, UNSPECIFIED)
{
        mMessageQueueHandle = 0;

        mSocketHandle = hSocket;
        setMsgSubType(EVENT);

        intptr_t         intData;
   char buff[MAXIMUM_INTEGER_STRING_LENGTH];

        intData = rEventMessage.getEventId();
        mTaoObjHandle = (TaoObjHandle) intData;
        sprintf(buff, "%" PRIdPTR, intData);
        mArgList = buff;
        mArgCnt = 1;

#ifdef USE_HTTPMSG
        createHTTPMsg((TaoMessage&) *this);
#endif

#ifdef TAOMSG_DEBUG
        mMsgCnt++;
        char tmp[64];

        sprintf(tmp, "%d %d %d %ld %ld ", getMsgSubType(), mCmd, mMsgID, mSocketHandle, mTaoObjHandle);
        UtlString stringData = UtlString("TAO EVENT: ") + UtlString(tmp) + mArgList;
        mHttpMsg.logTimeEvent(stringData.data());
#endif

   mbDirty = TRUE ;
}

TaoMessage::TaoMessage(const TaoMessage& rTaoMessage)
        : OsMsg((OsMsg&) rTaoMessage)
{
        setMsgSubType(rTaoMessage.getMsgSubType());

        mTaoObjHandle = rTaoMessage.mTaoObjHandle;

        mSocketHandle = rTaoMessage.mSocketHandle;

        mCmd = rTaoMessage.mCmd;

        mMsgID   = rTaoMessage.mMsgID;

        mArgCnt  = rTaoMessage.mArgCnt;

        mArgList = rTaoMessage.mArgList;
        mMessageQueueHandle = rTaoMessage.mMessageQueueHandle;

   mbDirty = TRUE ;
        mBodyLength = 0 ;

#ifdef USE_HTTPMSG
        createHTTPMsg((TaoMessage&) rTaoMessage);
#endif

#ifdef TAOMSG_DEBUG
        mMsgCnt++;
        char tmp[64];

        sprintf(tmp, "%d %d %d %ld %ld ", getMsgSubType(), mCmd, mMsgID, mSocketHandle, mTaoObjHandle);
        UtlString stringData = UtlString("COPY CONSTRUCTOR: ") + UtlString(tmp) + mArgList;
        mHttpMsg.logTimeEvent(stringData.data());
#endif
}

TaoMessage::TaoMessage(OsMsg& rTaoMessage)
        :OsMsg((OsMsg&) rTaoMessage)
{
        mMessageQueueHandle = 0;
        mTaoObjHandle = 0; // NULL
        mSocketHandle = 0; // NULL

#ifdef USE_HTTPMSG
        createHTTPMsg((TaoMessage&) rTaoMessage);
#endif

#ifdef TAOMSG_DEBUG
        mMsgCnt++;
        char tmp[64];

        sprintf(tmp, "%d %d %d %ld %ld ", getMsgSubType(), mCmd, mMsgID, mSocketHandle, mTaoObjHandle);
        UtlString stringData = UtlString("COPY CONSTRUCTOR OS: ") + UtlString(tmp) + mArgList;
        mHttpMsg.logTimeEvent(stringData.data());
#endif

   mbDirty = TRUE ;
}

TaoMessage::TaoMessage(const unsigned char msgSubType,
                                           const unsigned char cmd,
                                           const int msgId,
                                           TaoObjHandle handle,
                                           TaoObjHandle socket,
                                           const int argCnt,
                                           const UtlString& argList)
                                : OsMsg(OsMsg::TAO_MSG, msgSubType)             // we only set & use the subtype
{
        mMessageQueueHandle = 0;
        mCmd = cmd;
        mSocketHandle = socket;
        mTaoObjHandle = handle;
        setMsgSubType(msgSubType);
        mMsgID   = msgId;
        mArgCnt  = argCnt;

        if ((argCnt > 0) && (argList.length() != 0))
        {
                mArgList = argList;
        }

#ifdef USE_HTTPMSG
        createHTTPMsg((TaoMessage&) *this);
#endif

#ifdef TAOMSG_DEBUG
        mMsgCnt++;
        char tmp[64];

        sprintf(tmp, "%d %d %d %ld %ld ", getMsgSubType(), mCmd, mMsgID, mSocketHandle, mTaoObjHandle);
        UtlString stringData = UtlString("CONSTRUCTOR 7: ") + UtlString(tmp) + mArgList;
        mHttpMsg.logTimeEvent(stringData.data());
#endif

        mbDirty = TRUE ;
}

TaoMessage::TaoMessage(const UtlString& msgString)
        : OsMsg(OsMsg::TAO_MSG, UNSPECIFIED)
{
        mMessageQueueHandle = 0;

#ifdef USE_HTTPMSG
        mHttpMsg.parseMessage(msgString.data(), msgString.length());
        UtlString tmp;

        mHttpMsg.getContentType(&tmp);
        if (tmp == TAO_MESSAGE_CONTENT_TYPE)
        {
                tmp = mHttpMsg.getHeaderValue(0, TAO_MESSAGE_ID_FIELD);
                mMsgID = atoi(tmp.data());

                tmp = mHttpMsg.getHeaderValue(0, TAO_MESSAGE_SUBTYPE_FIELD);
                setMsgSubType(atoi(tmp.data()));

                tmp = mHttpMsg.getHeaderValue(0, TAO_MESSAGE_HANDLE_FIELD);
                mTaoObjHandle = atol(tmp.data());

                tmp = mHttpMsg.getHeaderValue(0, TAO_MESSAGE_CMD_FIELD);
                mCmd = atoi(tmp.data());

                tmp = mHttpMsg.getHeaderValue(0, TAO_MESSAGE_SOCKET_FIELD);
                mSocketHandle = atol(tmp.data());

                tmp = mHttpMsg.getHeaderValue(0, TAO_MESSAGE_QUEUEHANDLE_FIELD);
                mMessageQueueHandle = atol(tmp.data());

                tmp = mHttpMsg.getHeaderValue(0, TAO_MESSAGE_ARGCNT_FIELD);
                mArgCnt = atoi(tmp.data());

                mArgList = mHttpMsg.getHeaderValue(0, TAO_MESSAGE_ARGLIST_FIELD);

        }
        else
        {
                setMsgSubType(UNSPECIFIED);
        }
#endif

   mBodyLength = msgString.length();
   mBody = msgString;
        deSerialize();

#ifdef TAOMSG_DEBUG
        mMsgCnt++;
        char tmps[64];

        sprintf(tmps, "%d %d %d %ld %ld ", getMsgSubType(), mCmd, mMsgID, mSocketHandle, mTaoObjHandle);
        UtlString stringData = UtlString("CONSTRUCTOR str: ") + UtlString(tmps) + mArgList;
        mHttpMsg.logTimeEvent(stringData.data());
#endif
}

TaoMessage::~TaoMessage()
{
#ifdef TAOMSG_DEBUG
        mHttpMsg.logTimeEvent("DESTRUCTOR: ");
        mHttpMsg.dumpTimeLog();
        mMsgCnt--;
#endif
}

void TaoMessage::createHTTPMsg(TaoMessage& rTaoMessage)
{
#ifdef USE_HTTPMSG
        mHttpMsg.setFirstHeaderLine("GET /PATH HTTP/1.0");

        mHttpMsg.setContentType(TAO_MESSAGE_CONTENT_TYPE);
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

        sprintf(buff, "%d", OsMsg::TAO_MSG);
        mHttpMsg.addHeaderField(TAO_MESSAGE_TYPE_FIELD, buff);

        sprintf(buff, "%d", rTaoMessage.getMsgSubType());
        mHttpMsg.addHeaderField(TAO_MESSAGE_SUBTYPE_FIELD, buff);

        sprintf(buff, "%d", rTaoMessage.getMsgID());
        mHttpMsg.addHeaderField(TAO_MESSAGE_ID_FIELD, buff);

        sprintf(buff, "%d", rTaoMessage.getCmd());
        mHttpMsg.addHeaderField(TAO_MESSAGE_CMD_FIELD, buff);

        sprintf(buff, "%ld", rTaoMessage.getTaoObjHandle());
        mHttpMsg.addHeaderField(TAO_MESSAGE_HANDLE_FIELD, buff);

        sprintf(buff, "%ld", rTaoMessage.getSocket());
        mHttpMsg.addHeaderField(TAO_MESSAGE_SOCKET_FIELD, buff);

        sprintf(buff, "%ld", rTaoMessage.getMsgQueueHandle());
        mHttpMsg.addHeaderField(TAO_MESSAGE_QUEUEHANDLE_FIELD, buff);

        sprintf(buff, "%d", rTaoMessage.getArgCnt());
        mHttpMsg.addHeaderField(TAO_MESSAGE_ARGCNT_FIELD, buff);

        mHttpMsg.addHeaderField(TAO_MESSAGE_ARGLIST_FIELD, rTaoMessage.getArgList());
#endif
}

OsMsg* TaoMessage::createCopy(void) const
{
        return (new TaoMessage(*this));
}

//////////////////////////////////////////////////////////////////////
// MANIPULATORS
//////////////////////////////////////////////////////////////////////

// Assignment operator
TaoMessage&
TaoMessage::operator=(const TaoMessage& rhs)
{
        if (this == &rhs)            // handle the assignment to self case
          return *this;

        mTaoObjHandle       = rhs.mTaoObjHandle;
        mSocketHandle       = rhs.mSocketHandle;
        mMsgID              = rhs.mMsgID;
        mArgCnt             = rhs.mArgCnt;
        mArgList            = rhs.mArgList;
        mMessageQueueHandle = rhs.mMessageQueueHandle;
        mCmd                = rhs.mCmd;

        mbDirty = TRUE ;

#ifdef USE_HTTPMSG
        createHTTPMsg((TaoMessage&)rhs);
#endif

#ifdef TAOMSG_DEBUG
        char tmp[64];

        sprintf(tmp, "%d %d %d %ld %ld ", getMsgSubType(), mCmd, mMsgID, mSocketHandle, mTaoObjHandle);
        UtlString stringData = UtlString("ASSIGN: ") + UtlString(tmp) + mArgList;
        mHttpMsg.logTimeEvent(stringData.data());
#endif
        return *this;
}

void TaoMessage::setMsgQueueHandle(TaoObjHandle handle)
{
#ifdef USE_HTTPMSG
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

        sprintf(buff, "%ld", handle);
        mHttpMsg.setHeaderValue(TAO_MESSAGE_QUEUEHANDLE_FIELD, buff);
#endif

        mMessageQueueHandle = handle;
        mbDirty = TRUE ;
}

void TaoMessage::setSocket(TaoObjHandle handle)
{
#ifdef USE_HTTPMSG
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

        sprintf(buff, "%ld", handle);
        mHttpMsg.setHeaderValue(TAO_MESSAGE_SOCKET_FIELD, buff);
#endif

        mSocketHandle = handle;
        mbDirty = TRUE ;
}

void TaoMessage::setMsgID(unsigned int id)
{
#ifdef USE_HTTPMSG
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

        sprintf(buff, "%d", id);
        mHttpMsg.setHeaderValue(TAO_MESSAGE_ID_FIELD, buff);
#endif

        mMsgID = id;
        mbDirty = TRUE ;
}

void TaoMessage::setCmd(TaoMsgCmds cmd)
{
#ifdef USE_HTTPMSG
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

        memset(buff, 0, sizeof(buff));
        sprintf(buff, "%d", (int)cmd);
        mHttpMsg.setHeaderValue(TAO_MESSAGE_CMD_FIELD, buff);
#endif

        mCmd = cmd;
        mbDirty = TRUE ;
}

void TaoMessage::setArgCnt(int cnt)
{
#ifdef USE_HTTPMSG
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

        sprintf(buff, "%d", cnt);
        mHttpMsg.setHeaderValue(TAO_MESSAGE_ARGCNT_FIELD, buff);
#endif

        mArgCnt = cnt;
        mbDirty = TRUE ;
}

void TaoMessage::setArgList(UtlString& argList)
{
#ifdef USE_HTTPMSG
        mHttpMsg.setHeaderValue(TAO_MESSAGE_ARGCNT_FIELD, argList.data());
#endif

        mArgList = argList;
        mbDirty = TRUE ;
}

void TaoMessage::setArgList(const char* szArgList)
{
#ifdef USE_HTTPMSG
        mHttpMsg.setHeaderValue(TAO_MESSAGE_ARGCNT_FIELD, szArgList);
#endif

        mArgList = szArgList;
        mbDirty = TRUE ;
}


void TaoMessage::setObjHandle(TaoObjHandle handle)
{
#ifdef USE_HTTPMSG
   char buff[MAXIMUM_INTEGER_STRING_LENGTH];

        sprintf(buff, "%ld", handle);
        mHttpMsg.setHeaderValue(TAO_MESSAGE_HANDLE_FIELD, buff);
#endif

        mTaoObjHandle = handle;
        mbDirty = TRUE ;
}

//////////////////////////////////////////////////////////////////////
// Functions
//////////////////////////////////////////////////////////////////////
void TaoMessage::getBytes(UtlString* bytes, ssize_t* length)
{
   if (mbDirty)
      serialize() ;

#ifdef USE_HTTPMSG
        mHttpMsg.getBytes(bytes, length);
#else
        *length = mBodyLength;
        *bytes = mBody;
#endif
}

void TaoMessage::serialize()
{
        char buf[128];
        int subType = getMsgSubType();

        memset(buf, 0, 128 * sizeof(char));

        sprintf(buf, "st=%d id=%d cmd=%d oh=%ld sh=%ld qh=%ld ac=%d ",
                subType,
                mMsgID,
                mCmd,
                mTaoObjHandle,
                mSocketHandle,
                mMessageQueueHandle,
                mArgCnt);

        mBody = (const char*) buf;
        mBody.append(mArgList);
        mBodyLength = mBody.length();

   mbDirty = FALSE ;
}

void TaoMessage::deSerialize()
{
        int subType;
        char* pArg;

        pArg = new char[mBody.length()];

        sscanf(mBody.data(), "st=%d id=%d cmd=%c oh=%ld sh=%ld qh=%ld ac=%d %s",
                &subType,
                &mMsgID,
                &mCmd,
                &mTaoObjHandle,
                &mSocketHandle,
                &mMessageQueueHandle,
                &mArgCnt,
                pArg);

        setMsgSubType(subType);

        mArgList = (const char*) pArg ;
        delete pArg;

   mbDirty = FALSE ;
}
