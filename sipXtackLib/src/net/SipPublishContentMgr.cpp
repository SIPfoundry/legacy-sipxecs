//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <net/SipPublishContentMgr.h>
#include <utl/UtlHashBagIterator.h>
#include <utl/UtlString.h>
#include <utl/UtlSList.h>
#include <utl/UtlSListIterator.h>
#include <utl/UtlInt.h>
#include <net/HttpBody.h>
#include <os/OsSysLog.h>

// Private class to contain callback for eventTypeKey
class PublishCallbackContainer : public UtlString
{
public:
    PublishCallbackContainer();

    virtual ~PublishCallbackContainer();

    // Parent UtlString contains the SIP event Type token (not the eventTypeKey)
    void* mpApplicationData;
    SipPublishContentMgr::SipPublisherContentChangeCallback mpCallback;

private:
    //! DISALLOWED accidental copying
    PublishCallbackContainer(const PublishCallbackContainer& rPublishCallbackContainer);
    PublishCallbackContainer& operator=(const PublishCallbackContainer& rhs);
};

// Private class to contain event content for eventTypeKey
class PublishContentContainer : public UtlString
{
public:
    PublishContentContainer();

    virtual ~PublishContentContainer();

    // parent UtlString contains the resourceId and eventTypeKey

    // List of HttpBody's which are the content for this resourceId/eventTypeKey.
    // UtlString value of HttpBody is its MIME-type.
    UtlSList mEventContent;

    //! Dump the object's internal state.
    void dumpState();

private:
    //! DISALLOWED accendental copying
    PublishContentContainer(const PublishContentContainer& rPublishContentContainer);
    PublishContentContainer& operator=(const PublishContentContainer& rhs);

};

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */
PublishCallbackContainer::PublishCallbackContainer() :
   mpApplicationData(NULL),
   mpCallback(NULL)
{
}
PublishCallbackContainer::~PublishCallbackContainer()
{
}

PublishContentContainer::PublishContentContainer()
{
}
PublishContentContainer::~PublishContentContainer()
{
}

// Dump the object's internal state.
void PublishContentContainer::dumpState()
{
   // indented 8 and 10

   OsSysLog::add(FAC_RLS, PRI_INFO,
                 "\t        PublishContentContainer %p UtlString = '%s'",
                 this, data());

   int index = 0;
   UtlSListIterator content_itor(mEventContent);
   HttpBody* body;
   while ((body = dynamic_cast <HttpBody*> (content_itor())))
   {
      OsSysLog::add(FAC_RLS, PRI_INFO,
                    "\t          mEventContent[%d] = '%s':'%s'",
                    index, body->data(), body->getBytes());
      index++;
   }
}


// Constructor
SipPublishContentMgr::SipPublishContentMgr()
: mPublishMgrMutex(OsMutex::Q_FIFO)
{
}


// Copy constructor NOT IMPLEMENTED
SipPublishContentMgr::SipPublishContentMgr(const SipPublishContentMgr& rSipPublishContentMgr)
: mPublishMgrMutex(OsMutex::Q_FIFO)
{
}


// Destructor
SipPublishContentMgr::~SipPublishContentMgr()
{
   // Delete the stored information.
   mContentEntries.destroyAll();
   mPartialContentEntries.destroyAll();
   mDefaultContentEntries.destroyAll();
   mDefaultContentConstructors.destroyAll();
   mEventContentCallbacks.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipPublishContentMgr&
SipPublishContentMgr::operator=(const SipPublishContentMgr& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

void SipPublishContentMgr::publish(const char* resourceId,
                                   const char* eventTypeKey,
                                   const char* eventType,
                                   int numContentTypes,
                                   HttpBody* eventContent[],
                                   UtlBoolean noNotify,
                                   UtlBoolean fullState)
{
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipPublishContentMgr::publish resourceId '%s', eventTypeKey '%s', eventType '%s', numContentTypes %d, noNotify %d, fullState %d",
                  resourceId, eventTypeKey, eventType, numContentTypes,
                  noNotify, fullState);

    UtlBoolean resourceIdProvided = resourceId && *resourceId;

    // Construct the key to look up.
    UtlString key;
    if(resourceIdProvided)
    {
        key = resourceId;
    }

    if(eventTypeKey)
    {
       key.append(eventTypeKey);
    }

    lock();

    // Determine the type of storage we will be using
    UtlHashBag* pContent;
    if (resourceIdProvided)
    {
       if (fullState)
       {
          // Full dialog events
          pContent = &mContentEntries;
       }
       else
       {
          // Partial dialog events
          pContent = &mPartialContentEntries;
       }
    }
    else
    {
       // Default dialog events
       pContent = &mDefaultContentEntries;
    }

    // Look up the key in the specific or default entries, as appropriate.
    PublishContentContainer* container =
       dynamic_cast <PublishContentContainer*> (pContent->find(&key));

    // If not found, create a container.
    if(container == NULL)
    {
        container = new PublishContentContainer();
        *((UtlString*) container) = key;
	// Save the container in the appropriate hash.
        pContent->insert(container);
    }

    // The content for this event type already existed
    else
    {
        // Remove the old content
        container->mEventContent.destroyAll();
    }

    // Add the new content
    for (int index = 0; index < numContentTypes; index++)
    {
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
            "SipPublishContentMgr::publish eventContent[%d] = %p, key = '%s', content type = '%s', getBytes() = %p '%s'",
                      index,
                      eventContent[index],
                      key.data(),
                      eventContent[index]->data(),
                      eventContent[index]->getBytes(),
                      eventContent[index]->getBytes());
        container->mEventContent.append(eventContent[index]);
    }

    // Don't call the observers if noNotify is set or if this is default content.
    if (!noNotify && resourceIdProvided)
    {
       // Call the observer for the content change, if any.
       UtlString eventTypeString(eventType);
       PublishCallbackContainer* callbackContainer =
          dynamic_cast <PublishCallbackContainer*>
          (mEventContentCallbacks.find(&eventTypeString));
       if(callbackContainer && callbackContainer->mpCallback)
       {
          (callbackContainer->mpCallback)(callbackContainer->mpApplicationData,
                                          resourceId,
                                          eventTypeKey,
                                          eventTypeString);
       }
    }

    unlock();
}

void SipPublishContentMgr::publishDefault(const char* eventTypeKey,
                                          const char* eventType,
                                          int numContentTypes,
                                          HttpBody* eventContent[])
{
    publish(NULL,
            eventTypeKey,
            eventType,
            numContentTypes,
            eventContent);
}

void SipPublishContentMgr::publishDefault(const char* eventTypeKey,
                                          const char* eventType,
                                          SipPublishContentMgrDefaultConstructor*
                                          defaultConstructor)
{
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipPublishContentMgr::publishDefault eventTypeKey '%s', eventType '%s', defaultConstructor %p",
                  eventTypeKey, eventType, defaultConstructor);
    // Construct the key to look up.
    UtlString key;

    if(eventTypeKey)
    {
       key.append(eventTypeKey);
    }

    lock();

    // Add the default constructor.
    if (defaultConstructor)
    {
       // Remove any old value first.
       mDefaultContentConstructors.destroy(&key);
       UtlString* key_heap = new UtlString(key);
       mDefaultContentConstructors.insertKeyAndValue(key_heap,
                                                     defaultConstructor);
    }

    // Do not call the observer for the content change since this is default
    // content.

    unlock();
}

void SipPublishContentMgr::unpublish(const char* resourceId,
                                     const char* eventTypeKey,
                                     const char* eventType,
                                     UtlBoolean noNotify)
{
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipPublishContentMgr::unpublish resourceId '%s', eventTypeKey '%s', eventType '%s'",
                  resourceId, eventTypeKey, eventType);
    UtlBoolean resourceIdProvided = resourceId && *resourceId;

    // Construct the key to look up.
    UtlString key;
    if(resourceIdProvided)
    {
        key = resourceId;
    }

    if(eventTypeKey)
    {
       key.append(eventTypeKey);
    }

    lock();

    // Look up the key in the specific or default entries, as appropriate.
    PublishContentContainer* container;
    if (resourceIdProvided)
    {
       container = dynamic_cast <PublishContentContainer*>(mContentEntries.find(&key));
       if (container)
       {
           container->mEventContent.destroyAll();
           mContentEntries.destroy(container);
       }

       container = dynamic_cast <PublishContentContainer*>(mPartialContentEntries.find(&key));
       if (container)
       {
           container->mEventContent.destroyAll();
           mPartialContentEntries.destroy(container);
       }
    }
    else
    {
       container = dynamic_cast <PublishContentContainer*>(mDefaultContentEntries.find(&key));
       if (container)
       {
           container->mEventContent.destroyAll();
           mDefaultContentEntries.destroy(container);
       }

       // Remove any default constructor.
       mDefaultContentConstructors.destroy(&key);
    }

    // Don't call the observers if noNotify is set or if this is default content.
    if (!noNotify && resourceIdProvided)
    {
       UtlString eventTypeString(eventType);
       PublishCallbackContainer* callbackContainer =
          dynamic_cast <PublishCallbackContainer*>
          (mEventContentCallbacks.find(&eventTypeString));
       if(callbackContainer && callbackContainer->mpCallback)
       {
          (callbackContainer->mpCallback)(callbackContainer->mpApplicationData,
                                          resourceId,
                                          eventTypeKey,
                                          eventTypeString);
       }
    }

    unlock();
}

void SipPublishContentMgr::unpublishDefault(const char* eventTypeKey,
                                            const char* eventType)
{
    unpublish(NULL,
              eventTypeKey,
              eventType);
}

UtlBoolean SipPublishContentMgr::setContentChangeObserver(const char* eventType,
                                                          void* applicationData,
                                                          SipPublisherContentChangeCallback callbackFunction)
{
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipPublishContentMgr::setContentChangeObserver eventType '%s', callbackFunction %p",
                  eventType, callbackFunction);
    UtlBoolean callbackSet = FALSE;
    UtlString eventTypeString(eventType);

    lock();

    // eventTypeKey and eventType must be defined
    if(eventType == NULL ||
       *eventType == 0)
    {
        OsSysLog::add(FAC_SIP, PRI_ERR,
            "SipPublishContentMgr::setContentChangeObserver ignored, event type not set.");

    }

    // There should not be a callback set, need to unset first
    else if((mEventContentCallbacks.find(&eventTypeString)))
    {
        OsSysLog::add(FAC_SIP, PRI_ERR,
            "SipPublishContentMgr::setContentChangeObserver ignored, callback already exists for event: %s",
            eventType ? eventType : "<null>");
    }

    else
    {
        callbackSet = TRUE;
        PublishCallbackContainer* callbackEntry = new PublishCallbackContainer();
        *((UtlString*)callbackEntry) = eventType;
        callbackEntry->mpApplicationData = applicationData;
        callbackEntry->mpCallback = callbackFunction;
        mEventContentCallbacks.insert(callbackEntry);
    }

    unlock();

    return (callbackSet);
}



UtlBoolean SipPublishContentMgr::removeContentChangeObserver(const char* eventType,
                                                             void*& applicationData,
                                SipPublisherContentChangeCallback& callbackFunction)
{
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipPublishContentMgr::removeContentChangeObserver eventType '%s', callbackFunction %p",
                  eventType, &callbackFunction);
    UtlBoolean callbackRemoved = FALSE;
    UtlString eventTypeString(eventType);
    PublishCallbackContainer* callbackEntry = NULL;

    lock();

    // eventType must be defined
    if(eventType == NULL ||
       *eventType == 0)
    {
        OsSysLog::add(FAC_SIP, PRI_ERR,
            "SipPublishContentMgr::setContentChangeObserver ignored, event type not set.");

    }

    // There should not be a callback set, need to unset first
    else if((callbackEntry = (PublishCallbackContainer*)
        mEventContentCallbacks.remove(&eventTypeString)))
    {
        callbackRemoved = TRUE;
        callbackFunction = callbackEntry->mpCallback;
        applicationData = callbackEntry->mpApplicationData;
        delete callbackEntry;
        callbackEntry = NULL;
    }

    else
    {
        OsSysLog::add(FAC_SIP, PRI_ERR,
            "SipPublishContentMgr::setContentChangeObserver ignored, no callback exists for event: %s",
            eventType ? eventType : "<null>");
    }

    unlock();

    return (callbackRemoved);
}


/* ============================ ACCESSORS ================================= */

UtlBoolean SipPublishContentMgr::getContent(const char* resourceId,
                                            const char* eventTypeKey,
                                            const char* eventType,
                                            const char* acceptHeaderValue,
                                            HttpBody*& content,
                                            UtlBoolean& isDefaultContent,
                                            UtlBoolean fullState)
{
    UtlBoolean foundContent = FALSE;
    UtlString key(resourceId);

    key.append(eventTypeKey);
    PublishContentContainer* container = NULL;
    isDefaultContent = FALSE;

    // Turn acceptHeaderValue into a HashBag of its components.
    // acceptedTypesGiven = FALSE if there are no components.
    UtlHashBag contentTypes;
    UtlBoolean acceptedTypesGiven =
       buildContentTypesContainer(acceptHeaderValue, contentTypes);

    lock();

    UtlHashBag* pContent;
    if (fullState)
    {
       // Full content (this is the usual case)
       pContent = &mContentEntries;
    }
    else
    {
       // Partial content (used for partial dialog events)
       pContent = &mPartialContentEntries;
    }

    // See if resource-specific content exists
    container =
        dynamic_cast <PublishContentContainer*> (pContent->find(&key));

    // There is no resource-specific content.  Check if the default
    // constructor exists.
    if (container == NULL)
    {
       // Construct the key for the default data.
       UtlString default_key(eventTypeKey);

       // Look up the constructor.

       SipPublishContentMgrDefaultConstructor* constructor =
          dynamic_cast <SipPublishContentMgrDefaultConstructor*>
          (mDefaultContentConstructors.findValue(&default_key));
       // If it exists, call it to publish content for this resource/event.
       if (constructor)
       {
          constructor->generateDefaultContent(this, resourceId,
                                              eventTypeKey, eventType);
       }

       // See if resource specific content exists now.
       container =
          dynamic_cast <PublishContentContainer*> (pContent->find(&key));

       // If content was found, still mark it as default content.
       if (container)
       {
               isDefaultContent = TRUE;
       }
       // If still no content was found, check if the default exists.
       else
       {
           container =
              dynamic_cast <PublishContentContainer*>
              (mDefaultContentEntries.find(&default_key));
           if(container)
           {
               isDefaultContent = TRUE;
           }
       }
    }

    // Within the container, find the content of the right MIME type.
    if (container)
    {
        if (acceptedTypesGiven)
        {
           UtlString base_mime_type;

           // Search for the first content in the container whose
           // MIME type is in the acceptable list.
           UtlSListIterator contentIterator(container->mEventContent);
           HttpBody* bodyPtr;
           while (!foundContent &&
                  (bodyPtr = dynamic_cast <HttpBody*> (contentIterator())))
           {
              // Test if ';' is present in *bodyPtr's MIME type.
              // (Remember that an HttpBody considered as a UtlString has
              // the value of its content type.)
              ssize_t i = bodyPtr->index(';');

              // The 'if expression' is TRUE if the MIME type of *bodyPtr
              // is found in in contentTypes.  This is messy, because
              // *bodyPtr's MIME type may have parameters, which have
              // to be removed before searching contentTypes.
              if (contentTypes.find(i == UTL_NOT_FOUND ?
                                    bodyPtr :
                                    ( base_mime_type.remove(0),
                                      base_mime_type.append(*bodyPtr, 0, i),
                                      &base_mime_type)))
              {
                 content = bodyPtr->copy();
                 foundContent = TRUE;
              }
           }
           if (!foundContent)
           {
              // No content was found that matched the required MIME types.
              OsSysLog::add(FAC_SIP, PRI_WARNING,
                            "SipPublishContentMgr::getContent no content found for key '%s', acceptHeaderValue '%s', resourceId '%s', eventTypeKey ='%s', eventType '%s'",
                            key.data(),
                            acceptHeaderValue ? acceptHeaderValue : "[none]",
                            resourceId ? resourceId : "[none]",
                            eventTypeKey, eventType);
           }
        }
        else
        {
           // No MIME types were specified, take the first content in the list.
           HttpBody* bodyPtr =
              dynamic_cast <HttpBody*> (container->mEventContent.first());
           if (bodyPtr)
           {
              content = bodyPtr->copy();
              foundContent = TRUE;
           }
           else
           {
              // No content was found (at all).
              OsSysLog::add(FAC_SIP, PRI_WARNING,
                            "SipPublishContentMgr::getContent no content found for key '%s', acceptHeaderValue '%s', resourceId '%s', eventTypeKey ='%s', eventType '%s'",
                            key.data(),
                            acceptHeaderValue ? acceptHeaderValue : "[none]",
                            resourceId ? resourceId : "[none]",
                            eventTypeKey, eventType);
           }
        }
    }
    else
    {
       // No container found for this resource and event.
       OsSysLog::add(FAC_SIP, PRI_WARNING,
                     "SipPublishContentMgr::getContent no container found for key '%s', acceptHeaderValue '%s', resourceId '%s', eventTypeKey ='%s', eventType '%s', fullState = %d",
                     key.data(),
                     acceptHeaderValue ? acceptHeaderValue : "[none]",
                     resourceId ? resourceId : "[none]",
                     eventTypeKey, eventType,
                     fullState);
    }

    unlock();

    contentTypes.destroyAll();
    return (foundContent);
}


void SipPublishContentMgr::getStats(int& numDefaultContent,
                                    int& numDefaultConstructor,
                                    int& numResourceSpecificContent,
                                    int& numCallbacksRegistered)
{
    lock();
    numDefaultContent = mDefaultContentEntries.entries();
    numDefaultConstructor = mDefaultContentConstructors.entries();
    numResourceSpecificContent = mContentEntries.entries();
    numCallbacksRegistered = mEventContentCallbacks.entries();
    unlock();
}

UtlBoolean SipPublishContentMgr::getPublished(const char* resourceId,
                                              const char* eventTypeKey,
                                              int maxContentTypes,
                                              int& numContentTypes,
                                              HttpBody* eventContent[],
                                              SipPublishContentMgrDefaultConstructor**
                                              pDefaultConstructor)
{
    UtlBoolean contentReturned = FALSE;

    UtlBoolean resourceIdProvided = resourceId && *resourceId;

    // Construct the key to look up.
    UtlString key;
    if(resourceIdProvided)
    {
        key = resourceId;
    }

    if(eventTypeKey)
    {
       key.append(eventTypeKey);
    }

    lock();

    // Look up the key in the specific or default entries, as appropriate.
    PublishContentContainer* container =
       dynamic_cast <PublishContentContainer*> ((resourceIdProvided ?
                                                 mContentEntries :
                                                 mDefaultContentEntries).find(&key));

    // If not found, return zero versions.
    if (container == NULL)
    {
       contentReturned = TRUE;
       numContentTypes = 0;
    }

    // The content for this event type exists.
    else
    {
        int num = container->mEventContent.entries();

        if(num <= maxContentTypes)
        {
           contentReturned = TRUE;
           numContentTypes = num;
           // Copy the contents into the array.
           for (int index = 0; index < num; index++)
           {
              eventContent[index] =
                 new HttpBody(*dynamic_cast <HttpBody*>
                               (container->mEventContent.at(index)));
           }
        }
    }

    // Return the default constructor, if any.
    if (pDefaultConstructor)
    {
       UtlContainable* defaultConstructor =
          mDefaultContentConstructors.findValue(&key);
       *pDefaultConstructor =
          // Is there a default constructor?
          defaultConstructor ?
          // If so, make a copy of the constructor and return pointer to it.
          (dynamic_cast <SipPublishContentMgrDefaultConstructor*>
           (defaultConstructor))->copy() :
          // Otherwise, return NULL.
          NULL;
    }

    unlock();

    return contentReturned;
}

/* ============================ INQUIRY =================================== */

// Dump the object's internal state.
void SipPublishContentMgr::dumpState()
{
   lock();

   // indented 4 and 6

   OsSysLog::add(FAC_RLS, PRI_INFO,
                 "\t    SipPublishContentMgr %p",
                 this);

   UtlHashBagIterator itor(mContentEntries);
   PublishContentContainer* container;
   while ((container = dynamic_cast <PublishContentContainer*> (itor())))
   {
      OsSysLog::add(FAC_RLS, PRI_INFO,
                    "\t      mContentEntries{'%s'}",
                    container->data());
      container->dumpState();
   }

   unlock();
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean
SipPublishContentMgr::buildContentTypesContainer(const char* acceptHeaderValue,
                                                 UtlHashBag& contentTypes)
{
   // Set to TRUE when we see an element.
   UtlBoolean containsMimetypes = FALSE;

   if (acceptHeaderValue)
   {
      // Parse acceptHeaderValue into its parts.
      const char* p = acceptHeaderValue;
      while (*p != '\0')
      {
         // Skip any leading separator characters.
         size_t i = strspn(p, " \t,");
         if (*p != '\0')
         {
            p += i;
            // Identify the base MIME type (without any parameters).
            i = strcspn(p, " \t,;");
            if (i > 0)
            {
               // Create a string containing the MIME type and put it in the
               // bag of MIME types.
               UtlString* value = new UtlString(p, i);
               contentTypes.insert(value);
               containsMimetypes = TRUE;
               p += i;
               // Skip until a comma or end-of-string.
               p += strcspn(p, ",");
            }
         }
      }
   }

   return containsMimetypes;
}

void SipPublishContentMgr::lock()
{
    mPublishMgrMutex.acquire();
}

void SipPublishContentMgr::unlock()
{
    mPublishMgrMutex.release();
}

/* ============================ FUNCTIONS ================================= */

// Support functions for SipPublishContentMgrDefaultConstructor.

// None.
