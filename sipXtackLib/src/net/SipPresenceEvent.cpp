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
#include <utl/UtlHashBagIterator.h>
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
Tuple::Tuple(const char* tupleId) :
   UtlString(tupleId)
{
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

   static_cast <UtlString&> (*this) = static_cast <const UtlString&> (rhs);
   this->mStatus = rhs.mStatus;
   this->mContactUri = rhs.mContactUri;
   this->mPriority = rhs.mPriority;

   return *this;
}

// Copy constructor
Tuple::Tuple(const Tuple& rTuple) :
   UtlString(static_cast <const UtlString&> (rTuple)),
   mStatus(rTuple.mStatus),
   mContactUri(rTuple.mContactUri),
   mPriority(rTuple.mPriority)
{
}

/* ============================ ACCESSORS ================================= */

void Tuple::setTupleId(const char* tupleId)
{
   static_cast <UtlString&> (*this) = tupleId;
}


void Tuple::getTupleId(UtlString& tupleId) const
{
   tupleId = static_cast <const UtlString&> (*this);
}


void Tuple::setStatus(const char* status)
{
   mStatus = status;
}


void Tuple::getStatus(UtlString& status) const
{
   status = mStatus;
}


void Tuple::setContact(const char* contactUri,
                       const float priority)
{
   mContactUri = contactUri;
   mPriority = priority;
}


void Tuple::getContact(UtlString& contactUri,
                       float& priority) const
{
   contactUri = mContactUri;
   priority = mPriority;
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

      mBody = bodyBytes;
   }
}


// Destructor
SipPresenceEvent::~SipPresenceEvent()
{
   // Clean up all the tuple elements
   mTuples.destroyAll();
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

   Tuple *foundValue = dynamic_cast <Tuple*> (mTuples.remove(tuple));

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipPresenceEvent::removeTuple Tuple = %p, returned %p",
                 tuple, foundValue);

   mLock.release();
   return foundValue;
}


Tuple* SipPresenceEvent::getTuple(UtlString& tupleId)
{
   mLock.acquire();

   // We are cheating a bit in the find() here, because we search
   // for a Tuple but give a UtlString as the argument.
   Tuple* pTuple = dynamic_cast <Tuple*> (mTuples.find(&tupleId));

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipPresenceEvent::getTuple seach for '%s', found %p",
                 tupleId.data(), pTuple);

   mLock.release();
   return pTuple;
}

const UtlHashBag& SipPresenceEvent::getTuples()
{
   return mTuples;
}

Tuple* SipPresenceEvent::getTuple()
{
   mLock.acquire();

   UtlHashBagIterator itor(mTuples);

   Tuple* tuple = dynamic_cast <Tuple*> (itor());

   mLock.release();
   return tuple;
}

ssize_t SipPresenceEvent::getLength() const
{
   ssize_t length;
   UtlString tempBody;

   getBytes(&tempBody, &length);

   return length;
}

void SipPresenceEvent::buildBody() const
{
   mLock.acquire();

   // Pretend that mBody (from the base class HttpBody) is mutable.
   UtlString& mBodyMutable(const_cast <UtlString&> (mBody));
   UtlString singleLine;

   // Presence events have no version.

   // Construct the xml document of Tuple event
   mBodyMutable = XML_VERSION_1_0;

   // Presence Structure
   mBodyMutable.append(BEGIN_PRESENCE);
   mBodyMutable.append(PRESENTITY_EQUAL);
   singleLine = DOUBLE_QUOTE + mEntity + DOUBLE_QUOTE;
   mBodyMutable += singleLine;
   mBodyMutable.append(END_LINE);

   // Tuple elements
   UtlHashBagIterator tupleIterator(const_cast <UtlHashBag&> (mTuples));
   Tuple* pTuple;
   while ((pTuple = (Tuple *) tupleIterator()))
   {
      UtlString tupleId;
      pTuple->getTupleId(tupleId);

      mBodyMutable.append(BEGIN_TUPLE);
      singleLine = DOUBLE_QUOTE + tupleId + DOUBLE_QUOTE;
      mBodyMutable += singleLine;
      mBodyMutable.append(END_LINE);

      // Status element
      UtlString status;
      pTuple->getStatus(status);
      mBodyMutable.append(BEGIN_STATUS);
      singleLine = BEGIN_BASIC + status + END_BASIC;
      mBodyMutable += singleLine;
      mBodyMutable.append(END_STATUS);

      // Contact element
      UtlString contact;
      float priority;
      pTuple->getContact(contact, priority);
      if (!contact.isNull())
      {
         singleLine = BEGIN_CONTACT + contact + END_CONTACT;
         mBodyMutable += singleLine;
      }

      // End of Tuple element
      mBodyMutable.append(END_TUPLE);
   }

   // End of presence structure
   mBodyMutable.append(END_PRESENCE);

   // Pretend bodyLength (from base class HttpBody) is mutable.
   // @TODO@ This is just to adjust bodyLength to match UtlString::length().
   // This seems redundant.  I suspect bodyLength is redundant for
   // UtlString::length() and should be removed.
   const_cast <ssize_t&> (bodyLength) = mBodyMutable.length();

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipTupleEvent::buildBody Tuple mBodyMutable = '%s'",
                 mBodyMutable.data());

   mLock.release();
}

void SipPresenceEvent::getBytes(const char** bytes, ssize_t* length) const
{
   UtlString tempBody;

   getBytes(&tempBody, length);

   *bytes = mBody.data();
}

void SipPresenceEvent::getBytes(UtlString* bytes, ssize_t* length) const
{
   buildBody();

   *bytes = mBody;
   *length = bodyLength;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */
