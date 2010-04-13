//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#if !defined(AFX_SIPLINEEVENT_H__E2361046_5679_4988_A2CD_7564686A3EE4__INCLUDED_)
#define AFX_SIPLINEEVENT_H__E2361046_5679_4988_A2CD_7564686A3EE4__INCLUDED_

// APPLICATION INCLUDES
#include <net/SipLine.h>
#include <os/OsMsg.h>
#include <net/SipMessage.h>


class SipLineEvent : public OsMsg
{
public:
   enum EventSubTypes
   {
      SIP_LINE_EVENT_SUCCESS = 0,
      SIP_LINE_EVENT_FAILED,
      SIP_LINE_EVENT_NO_RESPONSE,
      SIP_LINE_EVENT_LINE_ADDED,
      SIP_LINE_EVENT_LINE_DELETED,
      SIP_LINE_EVENT_LINE_ENABLED,
      SIP_LINE_EVENT_LINE_DISABLED,
      SIP_LINE_EVENT_LINE_CHANGED,
      SIP_LINE_EVENT_OUTBOUND_CHANGED
   };


   SipLineEvent(SipLine* Line,
      int msgType = SipLineEvent::SIP_LINE_EVENT_FAILED,
      UtlString realm ="",
      UtlString scheme = HTTP_DIGEST_AUTHENTICATION,
      int SipReturnCode = SIP_OK_CODE,
      UtlString SipReturntext = SIP_OK_TEXT,
      void* observeData = NULL);

   virtual ~SipLineEvent();

   virtual OsMsg* createCopy(void) const;

   SipLineEvent& operator=(const SipLineEvent& rhs) ;

   int getSipReturnCode();

   UtlString& getSipReturnText();

   SipLine* getLine();

   UtlString& getRealm();

   UtlString& getScheme();


   void setMessageType(int status);

   int getMessageType() const;

   void setObserverData(void* pData);

   void* getObserverData();

private:

   UtlString mRealm;
   UtlString mScheme;
   UtlString mSipReturnText;
   int mSipReturnCode;
   int mMessageType;
   SipLine mLine;
   void* mObserverData;

};

#endif // !defined(AFX_SIPLINEEVENT_H__E2361046_5679_4988_A2CD_7564686A3EE4__INCLUDED_)
