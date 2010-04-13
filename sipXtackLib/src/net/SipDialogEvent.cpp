//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <net/NameValueTokenizer.h>
#include <net/SipDialogEvent.h>
#include <net/SipSubscribeServer.h>
#include <os/OsSysLog.h>
#include <utl/UtlDListIterator.h>
#include <utl/XmlContent.h>
#include <xmlparser/tinyxml.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS
const UtlContainableType Dialog::TYPE = "Dialog";


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
Dialog::Dialog(const char* dialogId,
               const char* callId,
               const char* localTag,
               const char* remoteTag,
               const char* direction)
{
   mId = dialogId;
   mCallId = callId;
   mLocalTag = localTag;
   mRemoteTag = remoteTag;
   mDirection = direction;
   setIdentifier();
}


Dialog::Dialog(const Dialog& dialog)
   : mId(dialog.mId),
   mCallId(dialog.mCallId),
   mLocalTag(dialog.mLocalTag),
   mRemoteTag(dialog.mRemoteTag),
   mDirection(dialog.mDirection),
   mIdentifier(dialog.mIdentifier),
   mState(dialog.mState),
   mEvent(dialog.mEvent),
   mCode(dialog.mCode),
   mDuration(dialog.mDuration),
   mNewCallId(dialog.mNewCallId),
   mNewLocalTag(dialog.mNewLocalTag),
   mNewRemoteTag(dialog.mNewRemoteTag),
   mReferredBy(dialog.mReferredBy),
   mDisplay(dialog.mDisplay),
   mLocalIdentity(dialog.mLocalIdentity),
   mLocalDisplay(dialog.mLocalDisplay),
   mLocalTarget(dialog.mLocalTarget),
   mLocalSessionDescription(dialog.mLocalSessionDescription),
   mRemoteIdentity(dialog.mRemoteIdentity),
   mRemoteDisplay(dialog.mRemoteDisplay),
   mRemoteTarget(dialog.mRemoteTarget),
   mRemoteSessionDescription(dialog.mRemoteSessionDescription)
{
   UtlDListIterator iterator(dialog.mLocalParameters);
   NameValuePairInsensitive* pNameValuePair;
   while ((pNameValuePair = dynamic_cast<NameValuePairInsensitive*>(iterator())))
   {
      mLocalParameters.append(new NameValuePairInsensitive(*pNameValuePair));
   }
   UtlDListIterator iterator2(dialog.mRemoteParameters);
   while ((pNameValuePair = dynamic_cast<NameValuePairInsensitive*>(iterator2())))
   {
      mRemoteParameters.append(new NameValuePairInsensitive(*pNameValuePair));
   }
}


// Destructor
Dialog::~Dialog()
{
   mLocalParameters.destroyAll();
   mRemoteParameters.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

void Dialog::setIdentifier()
{
   // Compose a unique identifier for the dialog by concatenating the
   // call-id, to-tag, and from-tag.
   mIdentifier.append(mCallId);
   mIdentifier.append("\001");
   mIdentifier.append(mLocalTag);
   mIdentifier.append("\001");
   mIdentifier.append(mRemoteTag);
}

/* ============================ ACCESSORS ================================= */

void Dialog::getBytes(UtlString& b, ssize_t& l)
{
   b.remove(0);
   b.append(BEGIN_DIALOG);
   UtlString singleLine;
   singleLine = DOUBLE_QUOTE;
   XmlEscape(singleLine, mId);
   singleLine += DOUBLE_QUOTE;
   b += singleLine;
   if (!mCallId.isNull())
   {
      b.append(CALL_ID_EQUAL);
      singleLine = DOUBLE_QUOTE;
      XmlEscape(singleLine, mCallId);
      singleLine += DOUBLE_QUOTE;
      b += singleLine;
   }

   if (!mLocalTag.isNull())
   {
      b.append(LOCAL_TAG_EQUAL);
      // mLocalTag is a token
      singleLine = DOUBLE_QUOTE + mLocalTag + DOUBLE_QUOTE;
      b += singleLine;
   }

   if (!mRemoteTag.isNull())
   {
      b.append(REMOTE_TAG_EQUAL);
      // mRemoteTag is a token
      singleLine = DOUBLE_QUOTE + mRemoteTag + DOUBLE_QUOTE;
      b += singleLine;
   }

   if (!mDirection.isNull())
   {
      b.append(DIRECTION_EQUAL);
      // mDirection is a token
      singleLine = DOUBLE_QUOTE + mDirection + DOUBLE_QUOTE;
      b += singleLine;
   }
   b.append(END_LINE);

   // State element
   b.append(BEGIN_STATE);
   if (!mEvent.isNull())
   {
      b.append(EVENT_EQUAL);
      // mEvent is a token
      singleLine = DOUBLE_QUOTE + mEvent + DOUBLE_QUOTE;
      b += singleLine;
   }

   if (!mCode.isNull())
   {
      b.append(CODE_EQUAL);
      // mCode is a token
      singleLine = DOUBLE_QUOTE + mCode + DOUBLE_QUOTE;
      b += singleLine;
   }

   // End of state element
   singleLine = END_BRACKET + mState + END_STATE;
   b += singleLine;

   // Duration element
   if (mDuration !=0)
   {
      b += BEGIN_DURATION;
      b.appendNumber((Int64) OsDateTime::getSecsSinceEpoch() - mDuration);
      b += END_DURATION;
   }

   // Local element
   b.append(BEGIN_LOCAL);
   if (!mLocalIdentity.isNull())
   {
      b.append(BEGIN_IDENTITY);
      if (!mLocalDisplay.isNull())
      {
         UtlString displayName = mLocalDisplay;
         NameValueTokenizer::frontBackTrim(&displayName, "\"");
         b.append(DISPLAY_EQUAL);
         singleLine = DOUBLE_QUOTE;
         XmlEscape(singleLine, displayName);
         singleLine += DOUBLE_QUOTE;
         b += singleLine;
      }

      singleLine = END_BRACKET;
      XmlEscape(singleLine, mLocalIdentity);
      singleLine += END_IDENTITY;
      b += singleLine;
   }

   if (!mLocalTarget.isNull() && mLocalTarget.compareTo("sip:") != 0)
   {
      singleLine = BEGIN_TARGET;
      XmlEscape(singleLine, mLocalTarget);
      singleLine += DOUBLE_QUOTE END_LINE;
      b += singleLine;
      // add optional parameters
      UtlDListIterator* iterator = getLocalParameterIterator();
      NameValuePairInsensitive* nvp;
      while ((nvp = (NameValuePairInsensitive*) (*iterator)()))
      {
         singleLine = BEGIN_DIALOG_PARAM;
         singleLine += PNAME;
         XmlEscape(singleLine, nvp->data());
         singleLine += PVALUE;
         XmlEscape(singleLine, nvp->getValue());
         singleLine += END_DIALOG_PARAM;
         b += singleLine;
      }
      delete iterator;

      singleLine = END_TARGET;
      b += singleLine;
   }

   // End of local element
   b.append(END_LOCAL);

   // Remote element
   b.append(BEGIN_REMOTE);
   if (!mRemoteIdentity.isNull())
   {
      b.append(BEGIN_IDENTITY);
      if (!mRemoteDisplay.isNull())
      {
         UtlString displayName = mRemoteDisplay;
         NameValueTokenizer::frontBackTrim(&displayName, "\"");
         b.append(DISPLAY_EQUAL);
         singleLine = DOUBLE_QUOTE;
         XmlEscape(singleLine, displayName);
         singleLine += DOUBLE_QUOTE;
         b += singleLine;
      }

      singleLine = END_BRACKET;
      XmlEscape(singleLine, mRemoteIdentity);
      singleLine += END_IDENTITY;
      b += singleLine;
   }

   if (!mRemoteTarget.isNull() && mRemoteTarget.compareTo("sip:") != 0)
   {
      singleLine = BEGIN_TARGET;
      XmlEscape(singleLine, mRemoteTarget);
      singleLine += DOUBLE_QUOTE END_LINE;
      b += singleLine;
      // add optional parameters
      UtlDListIterator* iterator = getRemoteParameterIterator();
      NameValuePairInsensitive* nvp;
      while ((nvp = (NameValuePairInsensitive*) (*iterator)()))
      {
         singleLine = BEGIN_DIALOG_PARAM;
         singleLine += PNAME;
         XmlEscape(singleLine, nvp->data());
         singleLine += PVALUE;
         XmlEscape(singleLine, nvp->getValue());
         singleLine += END_DIALOG_PARAM;
         b += singleLine;
      }
      delete iterator;

      singleLine = END_TARGET;
      b += singleLine;
   }

   // End of remote element
   b.append(END_REMOTE);

   // End of dialog element
   b.append(END_DIALOG);

   l = b.length();
}

void Dialog::getDialog(UtlString& dialogId,
                       UtlString& callId,
                       UtlString& localTag,
                       UtlString& remoteTag,
                       UtlString& direction) const
{
   dialogId = mId;
   callId = mCallId;
   localTag = mLocalTag;
   remoteTag = mRemoteTag;
   direction = mDirection;
}


void Dialog::getCallId(UtlString& callId) const
{
   callId = mCallId;
}


void Dialog::setDialogId(const char* dialogId)
{
   mId = dialogId;
   setIdentifier();
}


void Dialog::getDialogId(UtlString& dialogId) const
{
   dialogId = mId;
}


void Dialog::setState(const char* state,
                      const char* event,
                      const char* code)
{
   mState = state;
   mEvent = event;
   mCode = code;
}


void Dialog::setTags(const char* localTag,
                     const char* remoteTag)
{
   mLocalTag = localTag;
   mRemoteTag = remoteTag;
   setIdentifier();
}


void Dialog::getState(UtlString& state,
                      UtlString& event,
                      UtlString& code) const
{
   state = mState;
   event = mEvent;
   code = mCode;
}


void Dialog::setDuration(const unsigned long duration)
{
   mDuration = duration;
}


unsigned long Dialog::getDuration() const
{
   return mDuration;
}


void Dialog::setReplaces(const char* callId,
                         const char* localTag,
                         const char* remoteTag)
{
   mNewCallId = callId;
   mNewLocalTag = localTag;
   mNewRemoteTag = remoteTag;
}


void Dialog::getReplaces(UtlString& callId,
                         UtlString& localTag,
                         UtlString& remoteTag) const
{
   callId = mNewCallId;
   localTag = mNewLocalTag;
   remoteTag = mNewRemoteTag;
}


void Dialog::setReferredBy(const char* url,
                           const char* display)
{
   mReferredBy = url;
   mDisplay = display;
}


void Dialog::getReferredBy(UtlString& url,
                           UtlString& display) const
{
   url = mReferredBy;
   display = mDisplay;
}


void Dialog::setLocalIdentity(const char* identity,
                              const char* display)
{
   mLocalIdentity = identity;
   mLocalDisplay = display;
}


void Dialog::getLocalIdentity(UtlString& identity,
                              UtlString& display) const
{
   identity = mLocalIdentity;
   display = mLocalDisplay;
}


void Dialog::setRemoteIdentity(const char* identity,
                               const char* display)
{
   mRemoteIdentity = identity;
   mRemoteDisplay = display;
}


void Dialog::getRemoteIdentity(UtlString& identity,
                               UtlString& display) const
{
   identity = mRemoteIdentity;
   display = mRemoteDisplay;
}


void Dialog::setLocalTarget(const char* url)
{
   mLocalTarget = url;
}


void Dialog::getLocalTarget(UtlString& url) const
{
   url = mLocalTarget;
}


void Dialog::setRemoteTarget(const char* url)
{
   mRemoteTarget = url;
}


void Dialog::getRemoteTarget(UtlString& url) const
{
   url = mRemoteTarget;
}


int Dialog::compareTo(const UtlContainable *b) const
{
   return mIdentifier.compareTo(((Dialog *) b)->mIdentifier);
}


void Dialog::addLocalParameter(NameValuePairInsensitive* nvp)
{
   mLocalParameters.append(nvp);
}

UtlDListIterator* Dialog::getLocalParameterIterator()
{
   return new UtlDListIterator(mLocalParameters);
}


bool Dialog::setLocalParameter(const char* pname, const UtlString& pvalue)
{
   UtlBoolean found = FALSE;

   UtlDListIterator paramIterator(mLocalParameters);
   NameValuePairInsensitive* param= NULL;

   UtlString paramName;

   while (!found && (param = dynamic_cast<NameValuePairInsensitive*>(paramIterator())))
   {
      paramName = param->data();
      if (paramName.compareTo(pname, UtlString::ignoreCase) == 0)
      {
         found = TRUE;
         param->setValue(pvalue);
      }
   }
   return(found);
}

bool Dialog::getLocalParameter(const char* pname, UtlString& pvalue)
{
   UtlBoolean found = FALSE;
   pvalue = "";

   UtlDListIterator paramIterator(mLocalParameters);
   NameValuePairInsensitive* param= NULL;

   UtlString paramName;

   while (!found && (param = dynamic_cast<NameValuePairInsensitive*>(paramIterator())))
   {
      paramName = param->data();
      if (paramName.compareTo(pname, UtlString::ignoreCase) == 0)
      {
         found = TRUE;
         pvalue = param->getValue();
      }
   }
   return(found);
}

void Dialog::addRemoteParameter(NameValuePairInsensitive* nvp)
{
   mRemoteParameters.append(nvp);
}

UtlDListIterator* Dialog::getRemoteParameterIterator()
{
   return new UtlDListIterator(mRemoteParameters);
}

bool Dialog::setRemoteParameter(const char* pname, const UtlString& pvalue)
{
   UtlBoolean found = FALSE;

   UtlDListIterator paramIterator(mRemoteParameters);
   NameValuePairInsensitive* param= NULL;

   UtlString paramName;

   while (!found && (param = dynamic_cast<NameValuePairInsensitive*>(paramIterator())))
   {
      paramName = param->data();
      if (paramName.compareTo(pname, UtlString::ignoreCase) == 0)
      {
         found = TRUE;
         param->setValue(pvalue);
      }
   }
   return(found);
}

bool Dialog::getRemoteParameter(const char* pname, UtlString& pvalue)
{
   UtlBoolean found = FALSE;
   pvalue = "";

   UtlDListIterator paramIterator(mRemoteParameters);
   NameValuePairInsensitive* param= NULL;

   UtlString paramName;

   while (!found && (param = dynamic_cast<NameValuePairInsensitive*>(paramIterator())))
   {
      paramName = param->data();
      if (paramName.compareTo(pname, UtlString::ignoreCase) == 0)
      {
         found = TRUE;
         pvalue = param->getValue();
      }
   }
   return(found);
}

unsigned int Dialog::hash() const
{
   return mIdentifier.hash();
}


const UtlContainableType Dialog::getContainableType() const
{
    return TYPE;
}

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipDialogEvent::SipDialogEvent(const char* state, const char* entity)
   : mDialogState(state),
     mEntity(entity),
     mLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
   remove(0);
   append(DIALOG_EVENT_CONTENT_TYPE);
}

SipDialogEvent::SipDialogEvent(const SipDialogEvent& dialogEvent)
   : HttpBody(dialogEvent),
     mVersion(dialogEvent.mVersion),
     mDialogState(dialogEvent.mDialogState),
     mEntity(dialogEvent.mEntity),
     mLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
   mDialogs.removeAll();
   UtlSListIterator iterator(dialogEvent.mDialogs);
   Dialog* pDialog;
   while ((pDialog = dynamic_cast<Dialog*>(iterator())))
   {
      mDialogs.append(new Dialog(*pDialog));
   }
}

SipDialogEvent* SipDialogEvent::copy() const
{
  return new SipDialogEvent(*this);
}


SipDialogEvent::SipDialogEvent(const char* bodyBytes)
   : mLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
   remove(0);
   append(DIALOG_EVENT_CONTENT_TYPE);

   if(bodyBytes)
   {
      bodyLength = strlen(bodyBytes);
      parseBody(bodyBytes);
   }

   mBody = bodyBytes;
}


// Destructor
SipDialogEvent::~SipDialogEvent()
{
   // Clean up all the dialog elements
   mDialogs.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

void SipDialogEvent::parseBody(const char* bodyBytes)
{
   bool foundDialogs = false;

   if(bodyBytes)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogEvent::parseBody incoming package = %s\n",
                    bodyBytes);

      TiXmlDocument doc("dialogEvent.xml");

      doc.Parse(bodyBytes);
      if (!doc.Error())
      {
         TiXmlNode * rootNode = doc.FirstChild ("dialog-info");

         TiXmlElement* ucElement = 0;

         if (rootNode != NULL)
         {
            ucElement = rootNode->ToElement();

            if (ucElement)
            {
               mVersion = atoi(ucElement->Attribute("version"));
               mDialogState = ucElement->Attribute("state");
               mEntity = ucElement->Attribute("entity");
            }

            // Parse each dialog
            for (TiXmlNode *groupNode = rootNode->FirstChild("dialog");
                 groupNode;
                 groupNode = groupNode->NextSibling("dialog"))
            {
               UtlString dialogId, callId, localTag, remoteTag, direction;

               foundDialogs = true;

               // Get the attributes in dialog
               ucElement = groupNode->ToElement();
               if (ucElement)
               {
                  dialogId = ucElement->Attribute("id");
                  callId = ucElement->Attribute("call-id");
                  localTag = ucElement->Attribute("local-tag");
                  remoteTag = ucElement->Attribute("remote-tag");
                  direction = ucElement->Attribute("direction");
               }

               Dialog* pDialog = new Dialog(dialogId, callId, localTag, remoteTag, direction);

               // Get the state element
               UtlString state, event, code;
               state = (groupNode->FirstChild("state"))->FirstChild()->Value();

               ucElement = groupNode->FirstChild("state")->ToElement();
               if (ucElement)
               {
                  event = ucElement->Attribute("event");
                  code = ucElement->Attribute("code");
                  pDialog->setState(state, event, code);
               }

               // Get the duration element
               UtlString duration;
               TiXmlNode *subNode = groupNode->FirstChild("duration");
               if (subNode)
               {
                  duration = subNode->FirstChild()->Value();
                  pDialog->setDuration((unsigned long)atoi(duration.data()));
               }
               else
               {
                  pDialog->setDuration(0);
               }

               // Get the local element
               UtlString identity, display, target;
               subNode = groupNode->FirstChild("local");
               if (subNode)
               {
                  TiXmlNode *subNode1 = subNode->FirstChild("identity");
                  if (subNode1)
                  {
                     identity = subNode1->FirstChild()->Value();
                     ucElement = subNode1->ToElement();
                     if (ucElement)
                     {
                        display = ucElement->Attribute("display");
                        pDialog->setLocalIdentity(identity, display);
                     }
                  }

                  subNode1 = subNode->FirstChild("target");
                  ucElement = subNode1->ToElement();
                  if (ucElement)
                  {
                     target = ucElement->Attribute("uri");
                     pDialog->setLocalTarget(target);

                     // parse optional param elements
                     for (TiXmlNode *paramNode = 0;
                       (paramNode = subNode1->IterateChildren("param", paramNode)); )
                     {
                        TiXmlElement *paramElement = paramNode->ToElement();
                        const char* pname = paramElement->Attribute("pname");
                        const char* pvalue = paramElement->Attribute("pval");
                        pDialog->addLocalParameter(new NameValuePairInsensitive(pname, pvalue));
                     }

                  }
               }

               // Get the remote element
               subNode = groupNode->FirstChild("remote");
               if (subNode)
               {
                  TiXmlNode *subNode1 = subNode->FirstChild("identity");
                  if (subNode1)
                  {
                     identity = subNode1->FirstChild()->Value();
                     ucElement = subNode1->ToElement();
                     if (ucElement)
                     {
                        display = ucElement->Attribute("display");
                        pDialog->setRemoteIdentity(identity, display);
                     }
                  }

                  subNode1 = subNode->FirstChild("target");
                  ucElement = subNode1->ToElement();
                  if (ucElement)
                  {
                     target = ucElement->Attribute("uri");
                     pDialog->setRemoteTarget(target);

                     // parse optional param elements
                     for (TiXmlNode *paramNode = 0;
                       (paramNode = subNode1->IterateChildren("param", paramNode)); )
                     {
                        TiXmlElement *paramElement = paramNode->ToElement();
                        const char* pname = paramElement->Attribute("pname");
                        const char* pvalue = paramElement->Attribute("pval");
                        pDialog->addRemoteParameter(new NameValuePairInsensitive(pname, pvalue));
                     }
                  }
               }

               // Insert it into the list
               insertDialog(pDialog);
            }
            if (foundDialogs == false)
            {
               OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogEvent::parseBody no dialogs found");
            }
         }
         else
         {
            OsSysLog::add(FAC_SIP, PRI_ERR, "SipDialogEvent::parseBody <dialog-info> not found");
         }
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_ERR, "SipDialogEvent::parseBody xml parsing error");
      }
   }
}

// Assignment operator
SipDialogEvent&
SipDialogEvent::operator=(const SipDialogEvent& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   // TODO: need to add code to copy members here
   OsSysLog::add(FAC_SIP, PRI_ERR,
       "SipDialogEvent::operator= not implemented");

   return *this;
}

void SipDialogEvent::insertDialog(Dialog* dialog)
{
   mLock.acquire();
   if (mDialogs.insert(dialog) != NULL)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogEvent::insertDialog Dialog = %p",
                    dialog);
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_ERR, "SipDialogEvent::insertDialog Dialog = %p failed",
                    dialog);
   }
   mLock.release();
}


Dialog* SipDialogEvent::removeDialog(Dialog* dialog)
{
   mLock.acquire();
   UtlContainable *foundValue;
   foundValue = mDialogs.remove(dialog);

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogEvent::removeDialog Dialog = %p",
                 foundValue);

   mLock.release();
   return (Dialog *) foundValue;
}


Dialog* SipDialogEvent::getDialog(UtlString& callId,
                                  UtlString& localTag,
                                  UtlString& remoteTag)
{
   mLock.acquire();
   UtlSListIterator dialogIterator(mDialogs);
   Dialog* pDialog;
   UtlString foundDialogId, foundCallId, foundLocalTag, foundRemoteTag,
      foundDirection;
   while ((pDialog = (Dialog *) dialogIterator()))
   {
      pDialog->getDialog(foundDialogId,
                         foundCallId,
                         foundLocalTag,
                         foundRemoteTag,
                         foundDirection);

      if (foundCallId.compareTo(callId) == 0 &&
          foundLocalTag.compareTo(localTag) == 0 &&
          (foundRemoteTag.isNull() ||
           foundRemoteTag.compareTo(remoteTag) == 0))
      {
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipDialogEvent::getDialog found Dialog = %p for callId = '%s', local tag = '%s', remote tag = '%s'",
                       pDialog,
                       callId.data(), localTag.data(), remoteTag ? remoteTag.data() : "(none)");

         mLock.release();
         return pDialog;
      }
   }

   OsSysLog::add(FAC_SIP, PRI_WARNING,
                 "SipDialogEvent::getDialog could not find the Dialog for callId = '%s', local tag = '%s', remote tag = '%s'",
                 callId.data(),
                 localTag ? localTag.data() : "(none)",
                 remoteTag ? remoteTag.data() : "(none)");
   mLock.release();
   return NULL;
}


Dialog* SipDialogEvent::getDialogByCallId(UtlString& callId)
{
   mLock.acquire();
   UtlSListIterator dialogIterator(mDialogs);
   Dialog* pDialog;
   UtlString foundDialogId, foundCallId, foundLocalTag, foundRemoteTag,
      foundDirection;
   while ((pDialog = (Dialog *) dialogIterator()))
   {
      pDialog->getDialog(foundDialogId,
                         foundCallId,
                         foundLocalTag,
                         foundRemoteTag,
                         foundDirection);

      if (foundCallId.compareTo(callId) == 0)
      {
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipDialogEvent::getDialog found Dialog = %p for callId = '%s'",
                       pDialog, callId.data());

         mLock.release();
         return pDialog;
      }
   }

   OsSysLog::add(FAC_SIP, PRI_WARNING,
                 "SipDialogEvent::getDialog could not find the Dialog for callId = '%s'",
                 callId.data());
   mLock.release();
   return NULL;
}


Dialog* SipDialogEvent::getDialogByDialogId(UtlString& dialogId)
{
   mLock.acquire();
   UtlSListIterator dialogIterator(mDialogs);
   Dialog* pDialog;
   UtlString foundDialogId, foundCallId, foundLocalTag, foundRemoteTag,
      foundDirection;
   while ((pDialog = (Dialog *) dialogIterator()))
   {
      pDialog->getDialog(foundDialogId,
                         foundCallId,
                         foundLocalTag,
                         foundRemoteTag,
                         foundDirection);

      if (foundDialogId.compareTo(dialogId) == 0)
      {
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipDialogEvent::getDialog found Dialog = %p for dialogId = '%s'",
                       pDialog, dialogId.data());

         mLock.release();
         return pDialog;
      }
   }

   OsSysLog::add(FAC_SIP, PRI_WARNING,
                 "SipDialogEvent::getDialog could not find the Dialog for dialogId = '%s'",
                 dialogId.data());
   mLock.release();
   return NULL;
}


UtlBoolean SipDialogEvent::isEmpty()
{
   return (mDialogs.isEmpty());
}


ssize_t SipDialogEvent::buildBodyGetLength() const
{
   ssize_t length;
   UtlString tempBody;

   buildBodyGetBytes(&tempBody, &length);

   return length;
}


UtlSListIterator* SipDialogEvent::getDialogIterator()
{
   return new UtlSListIterator(mDialogs);
}


void SipDialogEvent::buildBody(int* version) const
{
   UtlString dialogEvent;
   UtlString singleLine;

   // Construct the xml document of dialog event
   dialogEvent = UtlString(XML_VERSION_1_0);

   // Dialog Information Structure
   dialogEvent.append(BEGIN_DIALOG_INFO);

   if (version)
   {
      // Generate the body with the recorded version.
      char buffer[20];
      sprintf(buffer, "%d", mVersion);
      dialogEvent.append(VERSION_EQUAL);
      singleLine = DOUBLE_QUOTE + UtlString(buffer) + DOUBLE_QUOTE;
      dialogEvent += singleLine;
      // Return the XML version.
      *version = mVersion;
   }
   else
   {
      // Generate the body with the substitution placeholder.
      dialogEvent.append(VERSION_EQUAL
                         DOUBLE_QUOTE VERSION_PLACEHOLDER DOUBLE_QUOTE);
   }

   dialogEvent.append(STATE_EQUAL);
   singleLine = DOUBLE_QUOTE + mDialogState + DOUBLE_QUOTE;
   dialogEvent += singleLine;

   dialogEvent.append(ENTITY_EQUAL);
   singleLine = DOUBLE_QUOTE;
   XmlEscape(singleLine, mEntity);
   singleLine += DOUBLE_QUOTE;
   dialogEvent += singleLine;
   dialogEvent.append(END_LINE);

   // Take the lock (we will be modifying the state even though 'this'
   // is read-only).
   (const_cast <SipDialogEvent*> (this))->mLock.acquire();

   // Dialog elements
   UtlSListIterator dialogIterator(mDialogs);
   Dialog* pDialog;
   while ((pDialog = (Dialog *) dialogIterator()))
   {
      UtlString b;
      ssize_t l;
      pDialog->getBytes(b, l);
      dialogEvent.append(b);
   }

   // End of dialog-info element
   dialogEvent.append(END_DIALOG_INFO);

   // Update body text (even though 'this' is read-only).
   (const_cast <SipDialogEvent*> (this))->mBody = dialogEvent;
   (const_cast <SipDialogEvent*> (this))->bodyLength = dialogEvent.length();
   // mVersion is not updated, as that is used only to record
   // the version of parsed events.

   (const_cast <SipDialogEvent*> (this))->mLock.release();

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogEvent::buildBody Dialog content = \n%s",
                 mBody.data());
}


void SipDialogEvent::getBytes(const char** bytes, ssize_t* length) const
{
   *bytes = mBody.data();
   *length = bodyLength;
}


void SipDialogEvent::getBytes(UtlString* bytes, ssize_t* length) const
{
   *bytes = mBody;
   *length = bodyLength;
}


void SipDialogEvent::buildBodyGetBytes(UtlString* bytes, ssize_t* length) const
{
   int dummy;
   buildBody(&dummy);

   *bytes = mBody;
   *length = bodyLength;
}


void SipDialogEvent::setEntity(const char* entity)
{
   mEntity = entity;
}


void SipDialogEvent::getEntity(UtlString& entity) const
{
   entity = mEntity;
}


void SipDialogEvent::setState(const char* state)
{
   mDialogState = state;
}


void SipDialogEvent::getState(UtlString& state) const
{
   state = mDialogState;
}

int SipDialogEvent::getVersion() const
{
   return mVersion;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */
