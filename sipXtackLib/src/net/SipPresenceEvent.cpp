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
#include <utl/UtlHashMapIterator.h>
#include <utl/XmlContent.h>
#include <net/SipPresenceEvent.h>
#include <net/NameValueTokenizer.h>
#include <xmlparser/tinyxml.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define DOUBLE_QUOTE "\""
#define END_BRACKET ">"
#define END_LINE ">\n"

// STATIC VARIABLE INITIALIZATIONS
const UtlContainableType Tuple::TYPE = "Tuple";


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
Tuple::Tuple(const char* tupleId)
{
   mId = tupleId;
}


// Destructor
Tuple::~Tuple()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
Tuple&
Tuple::operator=(const Tuple& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

// Copy constructor
Tuple::Tuple(const Tuple& rTuple)
{
   mId = rTuple.mId;
}

/* ============================ ACCESSORS ================================= */
void Tuple::setTupleId(const char* tupleId)
{
   mId = tupleId;
}


void Tuple::getTupleId(UtlString& tupleId) const
{
   tupleId = mId;
}


void Tuple::setStatus(const char* status)
{
   mStatus = status;
}


void Tuple::getStatus(UtlString& status) const
{
   status = mStatus;
}


void Tuple::setContact(const char* contactUrl,
                       const float priority)
{
   mContactUrl = contactUrl;
   mPriority = priority;
}


void Tuple::getContact(UtlString& contactUrl,
                       float& priority) const
{
   contactUrl = mContactUrl;
   priority = mPriority;
}


int Tuple::compareTo(const UtlContainable *b) const
{
   return mId.compareTo(((Tuple *)b)->mId);
}


unsigned int Tuple::hash() const
{
    return mId.hash();
}


const UtlContainableType Tuple::getContainableType() const
{
    return TYPE;
}


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipPresenceEvent::SipPresenceEvent(const char* entity, const char* bodyBytes)
   : mLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
   remove(0);
   append(PRESENCE_EVENT_CONTENT_TYPE);

   mEntity = entity;
   
   if (bodyBytes)
   {
      bodyLength = strlen(bodyBytes);
      parseBody(bodyBytes);
  
      ((SipPresenceEvent*) this)->mBody = bodyBytes;
   }
}


// Destructor
SipPresenceEvent::~SipPresenceEvent()
{
   // Clean up all the tuple elements
   if (!mTuples.isEmpty())
   {
      mTuples.destroyAll();
   }
}

/* ============================ MANIPULATORS ============================== */

void SipPresenceEvent::parseBody(const char* bodyBytes)
{
   if (bodyBytes)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipPresenceEvent::parseBody incoming package = '%s'", 
                    bodyBytes);
                    
      TiXmlDocument doc("PresenceEvent.xml");
      
      if (doc.Parse(bodyBytes))
      {
         TiXmlNode * rootNode = doc.FirstChild ("presence");
        
         TiXmlElement* ucElement = 0;

         if (rootNode != NULL)
         {
            ucElement = rootNode->ToElement();
            
            if (ucElement)
            {
               mEntity = ucElement->Attribute("entity");
            }
      
            // Parse each tuple
            for (TiXmlNode *groupNode = rootNode->FirstChild("tuple");
                 groupNode; 
                 groupNode = groupNode->NextSibling("tuple"))
            {
               UtlString tupleId;
               
               // Get the attributes in tuple
               ucElement = groupNode->ToElement();
               if (ucElement)
               {
                  tupleId = ucElement->Attribute("id");
               }
                  
               Tuple* pTuple = new Tuple(tupleId);
               
               // Get the status element
               UtlString status;
               status = ((groupNode->FirstChild("status"))->FirstChild("basic"))->FirstChild()->Value();
               pTuple->setStatus(status);

               // Get the contact element
               UtlString contact, priority;
               TiXmlNode *subNode = groupNode->FirstChild("contact");
               if (subNode)
               {
                  contact = subNode->FirstChild()->Value();
                  ucElement = subNode->ToElement();
               
                  if (ucElement)
                  {                  
                     priority = ucElement->Attribute("priority");
                  }
                  
               pTuple->setContact(contact, atof(priority));
               }
             
               // Insert it into the list
               insertTuple(pTuple);               
            }
         }
      }
   }
}


// Assignment operator
SipPresenceEvent&
SipPresenceEvent::operator=(const SipPresenceEvent& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/* ============================ ACCESSORS ================================= */
void SipPresenceEvent::insertTuple(Tuple* tuple)
{
   mLock.acquire();
   UtlContainable* result = mTuples.insert(tuple);

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipPresenceEvent::insertTuple Tuple = %p %s", 
                 tuple,
                 result ? "succeeded" : "failed");

   mLock.release();
}


Tuple* SipPresenceEvent::removeTuple(Tuple* tuple)
{
   mLock.acquire();
   UtlContainable *foundValue;
   foundValue = mTuples.remove(tuple);

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipPresenceEvent::removeTuple Tuple = %p", 
                 foundValue);                 

   mLock.release();
   return (Tuple *) foundValue;
}


Tuple* SipPresenceEvent::getTuple(UtlString& tupleId)
{
   mLock.acquire();
   UtlHashMapIterator tupleIterator(mTuples);
   Tuple* pTuple;
   UtlString foundValue;
   while ((pTuple = (Tuple *) tupleIterator()))
   {
      pTuple->getTupleId(foundValue);
      
      if (foundValue.compareTo(tupleId) == 0)
      {
         OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipPresenceEvent::getTuple found Tuple = %p for tupleId %s", 
                       pTuple, tupleId.data());                 
            
         mLock.release();
         return pTuple;
      }
   }     
          
   OsSysLog::add(FAC_SIP, PRI_WARNING, "SipPresenceEvent::getTuple could not find the Tuple for tupleId = '%s'", 
                 tupleId.data());                 
            
   mLock.release();
   return NULL;
}

UtlBoolean SipPresenceEvent::isEmpty()
{
   return (mTuples.isEmpty());
}

size_t SipPresenceEvent::getLength() const
{
   size_t length;
   UtlString tempBody;

   getBytes(&tempBody, &length);

   return length;
}

void SipPresenceEvent::buildBody(int& version) const
{
   UtlString PresenceEvent;
   UtlString singleLine;

   // Presence events have no version.
   version = 0;

   // Construct the xml document of Tuple event
   PresenceEvent = UtlString(XML_VERSION_1_0);

   // Presence Structure
   PresenceEvent.append(BEGIN_PRESENCE);
   PresenceEvent.append(PRESENTITY_EQUAL);
   singleLine = DOUBLE_QUOTE + mEntity + DOUBLE_QUOTE;
   PresenceEvent += singleLine;
   PresenceEvent.append(END_LINE);
    
   // Tuple elements
   ((SipPresenceEvent*)this)->mLock.acquire();
   UtlHashMapIterator tupleIterator(mTuples);
   Tuple* pTuple;
   while ((pTuple = (Tuple *) tupleIterator()))
   {
      
      UtlString tupleId;
      pTuple->getTupleId(tupleId);

      PresenceEvent.append(BEGIN_TUPLE);
      singleLine = DOUBLE_QUOTE + tupleId + DOUBLE_QUOTE;
      PresenceEvent += singleLine;
      PresenceEvent.append(END_LINE);
      
      // Status element
      UtlString status;
      pTuple->getStatus(status);
      PresenceEvent.append(BEGIN_STATUS);
      singleLine = BEGIN_BASIC + status + END_BASIC;
      PresenceEvent += singleLine;
      PresenceEvent.append(END_STATUS);
      
      // Contact element
      UtlString contact;
      float priority;
      pTuple->getContact(contact, priority);
      if (!contact.isNull())
      {
         singleLine = BEGIN_CONTACT + contact + END_CONTACT;
         PresenceEvent += singleLine;
      }

      // End of Tuple element
      PresenceEvent.append(END_TUPLE);
   }

   // End of presence structure
   PresenceEvent.append(END_PRESENCE);
   
   ((SipPresenceEvent*)this)->mLock.release();

   ((SipPresenceEvent*)this)->mBody = PresenceEvent;
   ((SipPresenceEvent*)this)->bodyLength = PresenceEvent.length();
   
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTupleEvent::getBytes Tuple content = \n%s", 
                 PresenceEvent.data());                 
}

void SipPresenceEvent::getBytes(const char** bytes, size_t* length) const
{
   UtlString tempBody;

   getBytes(&tempBody, length);
   ((SipPresenceEvent*)this)->mBody = tempBody.data();

   *bytes = mBody.data();
}

void SipPresenceEvent::getBytes(UtlString* bytes, size_t* length) const
{
   int dummy;
   buildBody(dummy);
   
   *bytes = ((SipPresenceEvent*)this)->mBody;
   *length = ((SipPresenceEvent*)this)->bodyLength;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

