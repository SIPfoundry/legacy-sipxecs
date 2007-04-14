// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsSysLog.h>
#include <utl/XmlContent.h>
#include <net/SipDialogEvent.h>
#include <net/NameValueTokenizer.h>
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


// Destructor
Dialog::~Dialog()
{
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
     // Generate the initial report with version 1, so we can generate
     // the default report with version 0 in
     // DialogDefaultConstructor::generateDefaultContent (in
     // DialogEventPublisher.cpp).
   : mVersion(1),
     mDialogState(state),
     mEntity(entity),
     mLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
   remove(0);
   append(DIALOG_EVENT_CONTENT_TYPE);
}

SipDialogEvent::SipDialogEvent(const SipDialogEvent& dialogEvent)
   : mVersion(dialogEvent.mVersion),
     mDialogState(dialogEvent.mDialogState),
     mEntity(dialogEvent.mEntity),
     mLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
   // TODO: mDialogs list is not copied
   OsSysLog::add(FAC_SIP, PRI_ERR, "SipDialogEvent::SipDialogEvent not implemented");
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
                  
                  ucElement = subNode->FirstChild("target")->ToElement();
                  if (ucElement)
                  {
                     target = ucElement->Attribute("uri");
                     pDialog->setLocalTarget(target);
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
                  
                  ucElement = subNode->FirstChild("target")->ToElement();
                  if (ucElement)
                  {
                     target = ucElement->Attribute("uri");
                     pDialog->setRemoteTarget(target);      
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


UtlBoolean SipDialogEvent::isEmpty()
{
   return (mDialogs.isEmpty());
}


int SipDialogEvent::getLength() const
{
   int length;
   UtlString tempBody;

   getBytes(&tempBody, &length);

   return length;
}


UtlSListIterator* SipDialogEvent::getDialogIterator()
{
   return new UtlSListIterator(mDialogs);
}


void SipDialogEvent::buildBody(int& version) const
{
   UtlString dialogEvent;
   UtlString singleLine;
   char buffer[20];
   char durationBuffer[20];

   // Return the XML version.
   version = mVersion;

   // Construct the xml document of dialog event
   dialogEvent = UtlString(XML_VERSION_1_0_CRLF);

   // Dialog Information Structure
   dialogEvent.append(BEGIN_DIALOG_INFO);

   Url entityUri(mEntity);
   sprintf(buffer, "%d", mVersion);

   dialogEvent.append(VERSION_EQUAL);
   singleLine = DOUBLE_QUOTE + UtlString(buffer) + DOUBLE_QUOTE;
   dialogEvent += singleLine;
   
   dialogEvent.append(STATE_EQUAL);
   singleLine = DOUBLE_QUOTE + mDialogState + DOUBLE_QUOTE;
   dialogEvent += singleLine;

   dialogEvent.append(ENTITY_EQUAL);
   singleLine = DOUBLE_QUOTE + entityUri.toString() + DOUBLE_QUOTE;
   dialogEvent += singleLine;
   dialogEvent.append(END_LINE);
 
   // Take the lock (we will be modifying the state even though 'this'
   // is read-only).
   ((SipDialogEvent*)this)->mLock.acquire();

   // Dialog elements
   UtlSListIterator dialogIterator(mDialogs);
   Dialog* pDialog;
   while ((pDialog = (Dialog *) dialogIterator()))
   {
      UtlString id, callId, localTag, remoteTag, direction;
      pDialog->getDialog(id, callId, localTag, remoteTag, direction);

      dialogEvent.append(BEGIN_DIALOG);
      singleLine = DOUBLE_QUOTE + id + DOUBLE_QUOTE;
      dialogEvent += singleLine;
      if (!callId.isNull())
      {
         dialogEvent.append(CALL_ID_EQUAL);
         singleLine = DOUBLE_QUOTE + callId + DOUBLE_QUOTE;
         dialogEvent += singleLine;
      }

      if (!localTag.isNull())
      {
         dialogEvent.append(LOCAL_TAG_EQUAL);
         singleLine = DOUBLE_QUOTE + localTag + DOUBLE_QUOTE;
         dialogEvent += singleLine;
      }

      if (!remoteTag.isNull())
      {
         dialogEvent.append(REMOTE_TAG_EQUAL);
         singleLine = DOUBLE_QUOTE + remoteTag + DOUBLE_QUOTE;
         dialogEvent += singleLine;
      }
   
      if (!direction.isNull())
      {
         dialogEvent.append(DIRECTION_EQUAL);
         singleLine = DOUBLE_QUOTE + direction + DOUBLE_QUOTE;
         dialogEvent += singleLine;
      }
      dialogEvent.append(END_LINE);

      // State element
      UtlString state, event, code;
      pDialog->getState(state, event, code);

      dialogEvent.append(BEGIN_STATE);
      if (!event.isNull())
      {
         dialogEvent.append(EVENT_EQUAL);
         singleLine = DOUBLE_QUOTE + event + DOUBLE_QUOTE;
         dialogEvent += singleLine;
      }

      if (!code.isNull())
      {
         dialogEvent.append(CODE_EQUAL);
         singleLine = DOUBLE_QUOTE + code + DOUBLE_QUOTE;
         dialogEvent += singleLine;
      }

      // End of state element
      singleLine = END_BRACKET + state + END_STATE;
      dialogEvent += singleLine;

      // Duration element
      int duration = pDialog->getDuration();      
      if (duration !=0)
      {
         duration = OsDateTime::getSecsSinceEpoch() - pDialog->getDuration();
         sprintf(durationBuffer, "%d", duration);
         dialogEvent += BEGIN_DURATION + UtlString(durationBuffer) + END_DURATION;     
      }
      
      // Local element
      UtlString identity, displayName, target;
      pDialog->getLocalIdentity(identity, displayName);
      pDialog->getLocalTarget(target);

      dialogEvent.append(BEGIN_LOCAL);
      if (!identity.isNull())
      {
         dialogEvent.append(BEGIN_IDENTITY);
         if (!displayName.isNull())
         {
            NameValueTokenizer::frontBackTrim(&displayName, "\"");
            dialogEvent.append(DISPLAY_EQUAL);
            singleLine = DOUBLE_QUOTE + displayName + DOUBLE_QUOTE;
            dialogEvent += singleLine;
         }
         
         singleLine = END_BRACKET + identity + END_IDENTITY;
         dialogEvent += singleLine;
      }

      if (!target.isNull() && target.compareTo("sip:") != 0)
      {
         singleLine = BEGIN_TARTGET + target + END_TARGET;
         dialogEvent += singleLine;
      }

      // End of local element
      dialogEvent.append(END_LOCAL);

      // Remote element
      pDialog->getRemoteIdentity(identity, displayName);
      pDialog->getRemoteTarget(target);

      dialogEvent.append(BEGIN_REMOTE);
      if (!identity.isNull())
      {
         dialogEvent.append(BEGIN_IDENTITY);
         if (!displayName.isNull())
         {
            NameValueTokenizer::frontBackTrim(&displayName, "\"");
            dialogEvent.append(DISPLAY_EQUAL);
            singleLine = DOUBLE_QUOTE + displayName + DOUBLE_QUOTE;
            dialogEvent += singleLine;
         }
   
         singleLine = END_BRACKET + identity + END_IDENTITY;
         dialogEvent += singleLine;
      }
      
      if (!target.isNull() && target.compareTo("sip:") != 0)
      {
         singleLine = BEGIN_TARTGET + target + END_TARGET;
         dialogEvent += singleLine;
      }

      // End of remote element
      dialogEvent.append(END_REMOTE);  

      // End of dialog element
      dialogEvent.append(END_DIALOG);  
   }

   // End of dialog-info element
   dialogEvent.append(END_DIALOG_INFO);  
   
   // Update body text and version number (even though 'this' is read-only).
   ((SipDialogEvent*)this)->mBody = dialogEvent;
   ((SipDialogEvent*)this)->bodyLength = dialogEvent.length();
   ((SipDialogEvent*)this)->mVersion++;

   ((SipDialogEvent*)this)->mLock.release();

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipDialogEvent::buildBody Dialog content = \n%s", 
                 mBody.data());
}


void SipDialogEvent::getBytes(const char** bytes, int* length) const
{
   *bytes = mBody.data();
   *length = bodyLength;
}


void SipDialogEvent::getBytes(UtlString* bytes, int* length) const
{
   int dummy;
   buildBody(dummy);
   
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
