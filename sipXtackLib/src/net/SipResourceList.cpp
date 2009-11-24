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
#include <net/SipResourceList.h>
#include <net/SipSubscribeServer.h>
#include <os/OsSysLog.h>
#include <utl/UtlHashMapIterator.h>
#include <utl/XmlContent.h>
#include <xmlparser/tinyxml.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const UtlContainableType TYPE = "ResourceList";

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
Resource::Resource(const char* uri)
{
   mUri = uri;
}


// Destructor
Resource::~Resource()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
Resource&
Resource::operator=(const Resource& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

// Copy constructor
Resource::Resource(const Resource& rResource)
{
   mUri = rResource.mUri;
}


/* ============================ ACCESSORS ================================= */
void Resource::getResourceUri(UtlString& uri) const
{
   uri = mUri;
}


void Resource::setName(const char* name)
{
   mName = name;
}


void Resource::getName(UtlString& name) const
{
   name = mName;
}


void Resource::setInstance(const char* id,
                           const char* state)
{
   mState = state;
   mId = id;
}


void Resource::getInstance(UtlString& id,
                           UtlString& state) const
{
   state = mState;
   id = mId;
}


int Resource::compareTo(const UtlContainable *b) const
{
   return mUri.compareTo(((Resource *)b)->mUri);
}


unsigned int Resource::hash() const
{
    return mUri.hash();
}


const UtlContainableType Resource::getContainableType() const
{
    return TYPE;
}


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipResourceList::SipResourceList(const UtlBoolean state,
                                 const char* uri,
                                 const char* type)
   : mLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
   remove(0);
   append(RESOURCE_LIST_CONTENT_TYPE);

   mVersion = 0;
   if (state)
   {
      mFullState = "true";
   }
   else
   {
      mFullState = "false";
   }

   mListUri = uri;
   mEventType = type;
}

SipResourceList::SipResourceList(const char* bodyBytes, const char* type)
   : mLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
   remove(0);
   append(RESOURCE_LIST_CONTENT_TYPE);

   if(bodyBytes)
   {
      bodyLength = strlen(bodyBytes);
      parseBody(bodyBytes);
   }

   ((SipResourceList*)this)->mBody = bodyBytes;
   mEventType = type;
}


// Destructor
SipResourceList::~SipResourceList()
{
   // Clean up all the resource elements
   if (!mResources.isEmpty())
   {
      mResources.destroyAll();
   }

   // Clean up all the event elements
   if (!mEvents.isEmpty())
   {
      mEvents.destroyAll();
   }
}

/* ============================ MANIPULATORS ============================== */

void SipResourceList::parseBody(const char* bodyBytes)
{
   if(bodyBytes)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipResourceList::parseBody incoming package = %s\n",
                    bodyBytes);

      TiXmlDocument doc("ResourceList.xml");

      if (doc.Parse(bodyBytes))
      {
	// TODO
      }
   }
}


// Assignment operator
SipResourceList&
SipResourceList::operator=(const SipResourceList& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/* ============================ ACCESSORS ================================= */
void SipResourceList::insertResource(Resource* resource)
{
   mLock.acquire();
   if (mResources.insert(resource) != NULL)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipResourceList::insertResource Resource = %p",
                    resource);
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_ERR, "SipResourceList::insertResource Resource = %p failed",
                    resource);
   }
   mLock.release();
}


Resource* SipResourceList::removeResource(Resource* resource)
{
   mLock.acquire();
   UtlContainable *foundValue;
   foundValue = mResources.remove(resource);

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipResourceList::removeResource Resource = %p",
                 foundValue);

   mLock.release();
    return (Resource *) foundValue;
}


Resource* SipResourceList::getResource(UtlString& resourceUri)
{
   mLock.acquire();
   UtlHashMapIterator resourceIterator(mResources);
   Resource* pResource;
   UtlString foundValue;
   while ((pResource = (Resource *) resourceIterator()))
   {
      pResource->getResourceUri(foundValue);

      if (foundValue.compareTo(resourceUri) == 0)
      {
         OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipResourceList::getResource found Resource = %p for resourceUri %s",
                       pResource, resourceUri.data());

         mLock.release();
         return pResource;
      }
   }

   OsSysLog::add(FAC_SIP, PRI_WARNING, "SipResourceList::getResource could not found the Resource for resourceUri = %s",
                 resourceUri.data());

   mLock.release();
   return NULL;
}

UtlBoolean SipResourceList::isEmpty()
{
   return (mResources.isEmpty());
}

void SipResourceList::insertEvent(UtlContainable* event)
{
   mLock.acquire();
   if (mEvents.insert(event) != NULL)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipResourceList::insertEvent Event = %p",
                    event);
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_ERR, "SipResourceList::insertEvent Event = %p failed",
                    event);
   }
   mLock.release();
}


UtlContainable* SipResourceList::removeEvent(UtlContainable* event)
{
   mLock.acquire();
   UtlContainable *foundValue;
   foundValue = mEvents.remove(event);

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipResourceList::removeEvent Event = %p",
                 foundValue);

   mLock.release();
   return foundValue;
}


void SipResourceList::getEventType(UtlString& type) const
{
   type = mEventType;
}


void SipResourceList::getListUri(UtlString& uri) const
{
   uri = mListUri;
}


ssize_t SipResourceList::getLength() const
{
   ssize_t length;
   UtlString tempBody;

   getBytes(&tempBody, &length);

   return length;
}

void SipResourceList::buildBody(int* version) const
{
   UtlString resourceList;
   UtlString singleLine;

   // Construct the xml document of resource list
   resourceList = UtlString(XML_VERSION_1_0);

   //  Information Structure
   Url listUri(mListUri);
   resourceList.append(BEGIN_LIST);

   resourceList.append(URI_EQUAL);
   singleLine = DOUBLE_QUOTE + listUri.toString() + DOUBLE_QUOTE;
   resourceList += singleLine;

   if (version)
   {
      // Generate the body with the recorded version.
      char buffer[20];
      sprintf(buffer, "%d", mVersion);
      resourceList.append(VERSION_EQUAL);
      singleLine = DOUBLE_QUOTE + UtlString(buffer) + DOUBLE_QUOTE;
      resourceList += singleLine;
      // Return the XML version.
      *version = mVersion;
   }
   else
   {
      resourceList.append(VERSION_EQUAL
                          DOUBLE_QUOTE VERSION_PLACEHOLDER DOUBLE_QUOTE);
   }

   resourceList.append(FULL_STATE_EQUAL);
   singleLine = DOUBLE_QUOTE + mFullState + DOUBLE_QUOTE;
   resourceList += singleLine;
   resourceList.append(END_LINE);

   // Resource elements
   ((SipResourceList*)this)->mLock.acquire();
   UtlHashMapIterator resourceIterator(mResources);
   Resource* pResource;
   while ((pResource = (Resource *) resourceIterator()))
   {
      UtlString uriStr;
      pResource->getResourceUri(uriStr);

      Url uri(uriStr);
      resourceList.append(BEGIN_RESOURCE);
      singleLine = DOUBLE_QUOTE + uri.toString() + DOUBLE_QUOTE;
      resourceList += singleLine;
      resourceList.append(END_LINE);

      // Name element
      UtlString name;
      pResource->getName(name);
      if (!name.isNull())
      {
         singleLine = BEGIN_CONTACT + name + END_CONTACT;
         resourceList += singleLine;
      }

      UtlString id, state;
      pResource->getInstance(id, state);
      resourceList.append(BEGIN_INSTANCE);
      singleLine = DOUBLE_QUOTE + id + DOUBLE_QUOTE;
      resourceList += singleLine;
      resourceList.append(STATE_EQUAL);
      singleLine = DOUBLE_QUOTE + state + DOUBLE_QUOTE;
      resourceList += singleLine;
      resourceList.append(END_LINE);

      // End of resource element
      resourceList.append(END_RESOURCE);
   }

   // End of list element
   resourceList.append(END_LIST);

   ((SipResourceList*)this)->mLock.release();

   ((SipResourceList*)this)->mBody = resourceList;
   ((SipResourceList*)this)->bodyLength = resourceList.length();

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipResourceList::getBytes Resource list content = \n%s",
                 resourceList.data());
   // mVersion is not updated, as that is used only to record
   // the version of parsed events.
}

void SipResourceList::getBytes(const char** bytes, ssize_t* length) const
{
   UtlString tempBody;

   getBytes(&tempBody, length);
   ((SipResourceList*)this)->mBody = tempBody.data();

   *bytes = mBody.data();
}

void SipResourceList::getBytes(UtlString* bytes, ssize_t* length) const
{
   int dummy;
   buildBody(&dummy);

   *bytes = ((SipResourceList*)this)->mBody;
   *length = ((SipResourceList*)this)->bodyLength;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */
