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
    PublishCallbackContainer(UtlString& eventTypeKey,
                             SipPublishContentMgr::SipPublisherContentChangeCallback callbackFunction,
                             void* applicationData);

    virtual ~PublishCallbackContainer();

    // Parent UtlString contains the SIP event Type token (not the eventTypeKey)
    SipPublishContentMgr::SipPublisherContentChangeCallback mpCallback;
    void* mpApplicationData;

private:
    //! DISALLOWED accidental copying
    PublishCallbackContainer(const PublishCallbackContainer& rPublishCallbackContainer);
    PublishCallbackContainer& operator=(const PublishCallbackContainer& rhs);
};

// Private class to contain event content for resourceId/eventTypeKey
class PublishContentContainer : public UtlString
{
public:
    PublishContentContainer(UtlString key);

    virtual ~PublishContentContainer();

    // The parent UtlString is "resourceId\001eventTypeKey", which is the
    // "key" string.

    // List of HttpBody's which are the content for this resourceId/eventTypeKey.
    // UtlString value of HttpBody is its MIME-type.
    // Will always contain 1 or more elements.
    UtlSList mEventContent;

    //! Dump the object's internal state.
    void dumpState();

   // No ::getContainableType() is declared, so that PublishContentContainer's
   // compare as their base UtlString's.

private:
    //! DISALLOWED accendental copying
    PublishContentContainer(const PublishContentContainer& rPublishContentContainer);
    PublishContentContainer& operator=(const PublishContentContainer& rhs);

};

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// Used to separate the resourceId from the eventTypeKey in
// PublishContentContainer value.
#define CONTENT_KEY_SEPARATOR "\001"

// STATIC VARIABLE INITIALIZATIONS

// Special value for the acceptHeaderValue argument to ::getContent()
// that indicates all MIME types are acceptable.
// Since UtlString::findToken() treats whitespace as insignificant,
// the value of " " cannot duplicate a legitimate value.
const UtlString SipPublishContentMgr::acceptAllTypes(" ");

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */
PublishCallbackContainer::PublishCallbackContainer(UtlString& eventTypeKey,
                                                   SipPublishContentMgr::SipPublisherContentChangeCallback callbackFunction,
                                                   void* applicationData) :
   UtlString(eventTypeKey),
   mpCallback(callbackFunction),
   mpApplicationData(applicationData)
{
}

PublishCallbackContainer::~PublishCallbackContainer()
{
}

PublishContentContainer::PublishContentContainer(UtlString key) :
   UtlString(key)
{
}

PublishContentContainer::~PublishContentContainer()
{
   // Delete the stored information.
   mEventContent.destroyAll();
}

// Dump the object's internal state.
void PublishContentContainer::dumpState()
{
   // indented 8 and 10

   UtlString key(*this);
   key.replace(sizeof (CONTENT_KEY_SEPARATOR) - 1,
               key.index(CONTENT_KEY_SEPARATOR),
               " ");
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
//SipPublishContentMgr::SipPublishContentMgr(const SipPublishContentMgr& rSipPublishContentMgr)


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

// Assignment operator NOT IMPLEMENTED
//SipPublishContentMgr&
//SipPublishContentMgr::operator=(const SipPublishContentMgr& rhs)

void SipPublishContentMgr::revised_publish(const char* resourceId,
                                   const char* eventTypeKey,
                                   const char* eventType,
                                   int numContentTypes,
                                   HttpBody* eventContent[],
                                   UtlBoolean fullState,
                                   UtlBoolean noNotify)
{
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipPublishContentMgr::publish resourceId '%s', eventTypeKey '%s', eventType '%s', numContentTypes %d, noNotify %d, fullState %d",
                  resourceId ? resourceId : "(null)",
                  eventTypeKey, eventType, numContentTypes, noNotify, fullState);

    if (numContentTypes < 1)
    {
       OsSysLog::add(FAC_SIP, PRI_ERR,
                     "SipPublishContentMgr::publish "
                     "No content bodies supplied for resourceId '%s', "
                     "eventTypeKey '%s', fullState %d",
                     resourceId? resourceId : "(null)",
                     eventTypeKey, fullState);
       
    }

    // Construct the key to look up.
    UtlString key;
    key.append(resourceId);
    key.append(CONTENT_KEY_SEPARATOR);
    key.append(eventTypeKey);

    lock();

    // Determine the storage we will be using.
    UtlHashBag* pContent;
    // resourceId can be NULL if we are called from ::publishDefault()
    if (resourceId)
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
       if (fullState)
       {
          pContent = &mDefaultContentEntries;
       }
       else
       {
          pContent = &mDefaultPartialContentEntries;
       }
    }

    // Look up the key in the specific or default entries, as appropriate.
    PublishContentContainer* container =
       dynamic_cast <PublishContentContainer*> (pContent->find(&key));

    // If not found, create a container.
    if (container == NULL)
    {
        container = new PublishContentContainer(key);
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
    if (!noNotify && resourceId)
    {
       // Call the observer for the content change, if any.
       UtlString eventTypeString(eventType);
       PublishCallbackContainer* callbackContainer =
          dynamic_cast <PublishCallbackContainer*>
          (mEventContentCallbacks.find(&eventTypeString));
       if (callbackContainer)
       {
          (callbackContainer->mpCallback)(callbackContainer->mpApplicationData,
                                          resourceId,
                                          eventTypeKey,
                                          eventTypeString,
                                          NULL);
       }
    }

    unlock();
}

void SipPublishContentMgr::revised_publishDefault(const char* eventTypeKey,
                                          const char* eventType,
                                          int numContentTypes,
                                          HttpBody* eventContent[],
                                          UtlBoolean fullState)
{
    revised_publish(NULL,
            eventTypeKey,
            eventType,
            numContentTypes,
            eventContent,
            fullState);
}

void SipPublishContentMgr::revised_publishDefault(const char* eventTypeKey,
                                          const char* eventType,
                                          SipPublishContentMgrDefaultConstructor*
                                          defaultConstructor,
                                          UtlBoolean fullState)
{
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipPublishContentMgr::publishDefault eventTypeKey '%s', eventType '%s', fullState %d, defaultConstructor %p",
                  eventTypeKey, eventType, fullState, defaultConstructor);

    // Construct the key to look up.
    UtlString key;
    key.append(CONTENT_KEY_SEPARATOR);
    key.append(eventTypeKey);

    lock();

    // Determine the storage we will be using.
    UtlHashMap* pContent;

    if (fullState)
    {
       pContent = &mDefaultContentConstructors;
    }
    else
    {
       pContent = &mDefaultPartialContentConstructors;
    }

    // Remove any old value first.
    pContent->destroy(&key);

    // Add the default constructor.
    if (defaultConstructor)
    {
       UtlString* key_heap = new UtlString(key);
       pContent->insertKeyAndValue(key_heap, defaultConstructor);
    }

    // Do not call the observer for the content change since this is default
    // content.

    unlock();
}

void SipPublishContentMgr::revised_unpublish(const char* resourceId,
                                     const char* eventTypeKey,
                                     const char* eventType,
                                     const char* reason)
{
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipPublishContentMgr::unpublish resourceId '%s', eventTypeKey '%s', eventType '%s', reason '%s'",
                  resourceId ? resourceId : "(null)",
                  eventTypeKey, eventType,
                  reason ? reason : "(null)");

    // Construct the key to look up.
    UtlString key;
    key.append(resourceId);
    key.append(CONTENT_KEY_SEPARATOR);
    key.append(eventTypeKey);

    lock();

    // Look up the key in the specific or default entries, as appropriate.
    if (resourceId)
    {
       PublishContentContainer* content =
          dynamic_cast <PublishContentContainer*> (mContentEntries.remove(&key));
       PublishContentContainer* partialContent =
          dynamic_cast <PublishContentContainer*> (mPartialContentEntries.remove(&key));

       // Call the default content constructors, if available.
       
       // Construct the key for the default data.
       UtlString default_key;
       default_key.append(CONTENT_KEY_SEPARATOR);
       default_key.append(eventTypeKey);

       // If it exists, call it to publish full content for this resource/event.
       SipPublishContentMgrDefaultConstructor* constructor =
          dynamic_cast <SipPublishContentMgrDefaultConstructor*>
          (mDefaultContentConstructors.findValue(&default_key));
       if (constructor)
       {
          constructor->generateDefaultContent(this, resourceId,
                                              eventTypeKey, eventType);
       }

       // If it exists, call it to publish partial content for this resource/event.
       constructor =
          dynamic_cast <SipPublishContentMgrDefaultConstructor*>
          (mDefaultPartialContentConstructors.findValue(&default_key));
       if (constructor)
       {
          constructor->generateDefaultContent(this, resourceId,
                                              eventTypeKey, eventType);
       }

       // Is there a callback for this eventType?  (Probably so.)
       UtlString eventTypeString(eventType);
       PublishCallbackContainer* callbackContainer =
          dynamic_cast <PublishCallbackContainer*>
          (mEventContentCallbacks.find(&eventTypeString));
       if (callbackContainer)
       {
          // See if resource-specific (full) content exists now.
          PublishContentContainer* newContent =
             dynamic_cast <PublishContentContainer*> (mContentEntries.find(&key));

          // If no (full) content was generated, check if (fixed) default content exists.
          PublishContentContainer* defaultContent;
          if (!newContent)
          {
             defaultContent =
                dynamic_cast <PublishContentContainer*> (mDefaultContentEntries.find(&default_key));
          }

          // Now, newContent is non-NULL if default content exists for
          // 'key', which means that the resource still has
          // publishable content.
          if (newContent || defaultContent)
          {
             // Replace the new content for the resource with the previous content.
             if (newContent)
             {
                mContentEntries.removeReference(newContent);
             }
             PublishContentContainer* newPartialContent =
                dynamic_cast <PublishContentContainer*> (mPartialContentEntries.remove(&key));

             // Insert the previous content for the resource.
             if (content)
             {
                mContentEntries.insert(content);
             }
             if (partialContent)
             {
                mPartialContentEntries.insert(partialContent);
             }

             // Do a "publish" callback.
             (callbackContainer->mpCallback)(callbackContainer->mpApplicationData,
                                             resourceId,
                                             eventTypeKey,
                                             eventTypeString,
                                             NULL);

             // Remove the previous content again.
             // Insert the previous content for the resource.
             if (content)
             {
                mContentEntries.removeReference(content);
             }
             if (partialContent)
             {
                mPartialContentEntries.removeReference(partialContent);
             }

             // Insert the new content again.
             if (newContent)
             {
                mContentEntries.insert(newContent);
             }
             if (newPartialContent)
             {
                mContentEntries.insert(newPartialContent);
             }
          }
          else
          {
             // No publishable content, so do an "unpublish" callback.
             (callbackContainer->mpCallback)(callbackContainer->mpApplicationData,
                                             resourceId,
                                             eventTypeKey,
                                             eventTypeString,
                                             reason);
          }
       }

       // Delete the old content containers.
       if (content)
       {
          delete content;
       }
       if (partialContent)
       {
          delete partialContent;
       }
    }
    else
    {
       // Remove default content.
       mDefaultContentEntries.destroy(&key);
       // Remove any default constructor.
       mDefaultContentConstructors.destroy(&key);
    }

    unlock();
}

void SipPublishContentMgr::revised_unpublishDefault(const char* eventTypeKey,
                                            const char* eventType,
                                            UtlBoolean fullState)
{
    revised_unpublish(NULL,
              eventTypeKey,
              eventType,
              NULL);
}

void SipPublishContentMgr::revised_setContentChangeObserver(const char* eventType,
                                                            SipPublisherContentChangeCallback callbackFunction,
                                                            void* applicationData)
{
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipPublishContentMgr::setContentChangeObserver eventType '%s', callbackFunction %p, applicationData = %p",
                  eventType, callbackFunction, applicationData);
    UtlString eventTypeString(eventType);

    lock();

    // eventType must be defined
    if (!eventTypeString.isNull())
    {
       // Delete any existing callback.
       mEventContentCallbacks.destroy(&eventTypeString);
       // Insert the new callback, if it is not NULL.
       if (callbackFunction)
       {
          mEventContentCallbacks.insert(
             new PublishCallbackContainer(eventTypeString,
                                          callbackFunction,
                                          applicationData));
       }
    }
    else
    {
       OsSysLog::add(FAC_SIP, PRI_ERR,
                     "SipPublishContentMgr::setContentChangeObserver "
                     "ignored, eventType is null.");
    }

    unlock();

    return;
}

void SipPublishContentMgr::getContentChangeObserver(const char* eventType,
                                                    SipPublisherContentChangeCallback& callbackFunction,
                                                    void*& applicationData)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipPublishContentMgr::getContentChangeObserver eventType '%s'",
                 eventType);
   UtlString eventTypeString(eventType);

   lock();

   PublishCallbackContainer* callbackEntry = 
      dynamic_cast <PublishCallbackContainer*>
      (mEventContentCallbacks.find(&eventTypeString));
   if (callbackEntry)
   {
      callbackFunction = callbackEntry->mpCallback;
      applicationData = callbackEntry->mpApplicationData;
   }
   else
   {
      callbackFunction = NULL;
      applicationData = NULL;
   }

   unlock();

   return;
}


/* ============================ ACCESSORS ================================= */

UtlBoolean SipPublishContentMgr::revised_getContent(const char* resourceId,
                                            const char* eventTypeKey,
                                            const char* eventType,
                                            UtlBoolean fullState,
                                            const UtlString& acceptHeaderValue,
                                            HttpBody*& content,
                                            UtlBoolean& isDefaultContent,
                                            UtlString* availableMediaTypes)
{
    UtlBoolean foundContent = FALSE;
    PublishContentContainer* container = NULL;
    isDefaultContent = FALSE;

    UtlString key;
    key.append(resourceId);
    key.append(CONTENT_KEY_SEPARATOR);
    key.append(eventTypeKey);

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
       UtlString default_key;
       default_key.append(CONTENT_KEY_SEPARATOR);
       default_key.append(eventTypeKey);

       // Look up the constructor.

       UtlHashMap* pDefaultConstructors;
       if (fullState)
       {
          // Full content (this is the usual case)
          pDefaultConstructors = &mDefaultContentConstructors;
       }
       else
       {
          // Partial content (used for partial dialog events)
          pDefaultConstructors = &mDefaultPartialContentConstructors;
       }
       SipPublishContentMgrDefaultConstructor* constructor =
          dynamic_cast <SipPublishContentMgrDefaultConstructor*>
          (pDefaultConstructors->findValue(&default_key));
       // If it exists, call it to publish content for this resource/event.
       if (constructor)
       {
          constructor->generateDefaultContent(this, resourceId,
                                              eventTypeKey, eventType);
       }

       // See if resource-specific content exists now.
       container =
          dynamic_cast <PublishContentContainer*> (pContent->find(&key));

       // If content was found, still mark it as default content.
       if (container)
       {
               isDefaultContent = TRUE;
       }
       // If still no content was found, check if (fixed) default content exists.
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

    // Within the container, choose the correct content.
    if (container)
    {
        if (acceptHeaderValue.compareTo(acceptAllTypes) != 0)
        {
           // Search for the first content in the container whose
           // MIME type is in the acceptable list.
           UtlSListIterator contentIterator(container->mEventContent);
           HttpBody* bodyPtr;
           while (!foundContent &&
                  (bodyPtr = dynamic_cast <HttpBody*> (contentIterator())))
           {
              if (acceptHeaderValue.findToken(bodyPtr->getContentType(), ",", ";"))
              {
                 content = bodyPtr->copy();
                 foundContent = TRUE;
              }
           }
           if (!foundContent)
           {
              // No content was found that matched the allowed MIME types.
              OsSysLog::add(FAC_SIP, PRI_WARNING,
                            "SipPublishContentMgr::getContent no acceptable content found for key '%s', acceptHeaderValue '%s', resourceId '%s', eventTypeKey ='%s', eventType '%s'",
                            key.data(),
                            acceptHeaderValue.data(),
                            resourceId ? resourceId : "[none]",
                            eventTypeKey, eventType);
              if (availableMediaTypes)
              {
                 // Construct the list of available MIME types.
                 availableMediaTypes->remove(0);
                 contentIterator.reset();
                 while ((bodyPtr = dynamic_cast <HttpBody*> (contentIterator())))
                 {
                    if (!availableMediaTypes->isNull())
                    {
                       availableMediaTypes->append(',');
                    }
                    availableMediaTypes->append(static_cast <UtlString&> (*bodyPtr));
                 }
              }
           }
        }
        else
        {
           // No MIME types were specified, take the first content in the list.
           // (which should exist)
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
              // Set *availableMediaTypes.
              if (availableMediaTypes)
              {
                 availableMediaTypes->remove(0);
              }
              OsSysLog::add(FAC_SIP, PRI_WARNING,
                            "SipPublishContentMgr::getContent no content found for key '%s', resourceId '%s', eventTypeKey ='%s', eventType '%s' - publish() must have been called with numContentTypes==0",
                            key.data(),
                            resourceId ? resourceId : "[none]",
                            eventTypeKey, eventType);
           }
        }
    }
    else
    {
       // No container found for this resource and event.
       // Set *availableMediaTypes.
       if (availableMediaTypes)
       {
          availableMediaTypes->remove(0);
       }
       OsSysLog::add(FAC_SIP, PRI_WARNING,
                     "SipPublishContentMgr::getContent no container found for key '%s', acceptHeaderValue '%s', resourceId '%s', eventTypeKey ='%s', eventType '%s', fullState = %d",
                     key.data(),
                     acceptHeaderValue.data(),
                     resourceId ? resourceId : "[none]",
                     eventTypeKey, eventType,
                     fullState);
    }

    unlock();

    return foundContent;
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

void SipPublishContentMgr::revised_getPublished(const char* resourceId,
                                        const char* eventTypeKey,
                                        UtlBoolean fullState,
                                        int& numContentTypes,
                                        HttpBody**& eventContent,
                                        SipPublishContentMgrDefaultConstructor**
                                        pDefaultConstructor)
{
    // Construct the key to look up.
    UtlString key;
    key.append(resourceId);
    key.append(CONTENT_KEY_SEPARATOR);
    key.append(eventTypeKey);

    lock();

    // Look up the key in the specific or default entries, as appropriate.
    UtlHashBag* bag = 
       resourceId ?
       (fullState ? &mContentEntries : &mPartialContentEntries) :
       (fullState ? &mDefaultContentEntries : &mDefaultPartialContentEntries);
    PublishContentContainer* container =
       dynamic_cast <PublishContentContainer*> (bag->find(&key));

    // If not found, return zero versions.
    if (container == NULL)
    {
       numContentTypes = 0;
       eventContent = new HttpBody*[0];
    }
    // Content for this event type exists.
    else
    {
        int num = container->mEventContent.entries();
        numContentTypes = num;

        HttpBody** contentCopies = new HttpBody*[num];
        eventContent = contentCopies;

        // Copy the contents into the array.
        for (int index = 0; index < num; index++)
        {
           eventContent[index] =
              new HttpBody(*dynamic_cast <HttpBody*>
                            (container->mEventContent.at(index)));
        }
    }

    // Return the default constructor, if any.
    if (pDefaultConstructor && !resourceId)
    {
       UtlContainable* defaultConstructor =
          (fullState ?
           mDefaultContentConstructors :
           mDefaultPartialContentConstructors).findValue(&key);
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

    return;
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

   dumpStateBag(mContentEntries, "mContentEntries");
   dumpStateBag(mPartialContentEntries, "mPartialContentEntries");
   dumpStateBag(mDefaultContentEntries, "mDefaultContentEntries");
   dumpStateBag(mDefaultPartialContentEntries, "mDefaultPartialContentEntries");

   unlock();
}

void SipPublishContentMgr::dumpStateBag(UtlHashBag& bag,
                                        const char* name)
{
   UtlHashBagIterator itor(bag);
   PublishContentContainer* container;
   while ((container = dynamic_cast <PublishContentContainer*> (itor())))
   {
      OsSysLog::add(FAC_RLS, PRI_INFO,
                    "\t      %s{'%s'}",
                    name, container->data());
      container->dumpState();
   }
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

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
