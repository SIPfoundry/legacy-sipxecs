//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _SipResourceList_h_
#define _SipResourceList_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlHashMap.h>
#include <net/HttpBody.h>
#include <net/Url.h>
#include <net/SipDialogEvent.h>
#include <net/SipPresenceEvent.h>
#include <os/OsDateTime.h>
#include <os/OsBSem.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define RESOURCE_LIST_CONTENT_TYPE "application/rlmi+xml"
#define RESOURCE_LIST_XMLNS "urn:ietf:params:xml:ns:rlmi"

#define BEGIN_LIST "<list xmlns=\"urn:ietf:params:xml:ns:rlmi\""
#define END_LIST "</list>\n"

#define URI_EQUAL " uri="
#define FULL_STATE_EQUAL " fullState="

#define BEGIN_RESOURCE "<resource uri="
#define END_RESOURCE "</resource>\n"

#define BEGIN_NAME "<name>"
#define END_NAME "</name>\n"

#define BEGIN_INSTANCE "<instance id="

#define STATE_ACTIVE "active"
#define STATE_PENDING "pending"
#define STATE_TERMINATED "terminated"

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//! Container for resource element in the resource list
/**
 * This class contains all the contents presented in a resource element of the
 * resource list described in draft-ietf-simple-event-list-07.txt
 * (A Session Initiation Protocol Event Notification Extension for Resource Lists).
 * This class has the methods to construct and manipulate the resource and its
 * sub-elements.
 */

class Resource : public UtlContainable
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/**
 * @name ====================== Constructors and Destructors
 * @{
 */
   /// Constructor
   Resource(const char* uri);

   /// Copy constructor
   Resource(const Resource& rResource);

   /// Destructor
   ~Resource();

   virtual UtlContainableType getContainableType() const;

   virtual unsigned int hash() const;

   int compareTo(const UtlContainable *b) const;

///@}

/**
 * @name ====================== Resource Setting Interfaces
 *
 * These methods set/get the resource element and sub-elements.
 *
 * @{
 */

   void getResourceUri(UtlString& uri) const;

   void setName(const char* name);

   void getName(UtlString& name) const;

   void setInstance(const char* id,
                    const char* state);

   void getInstance(UtlString& id,
                    UtlString& state) const;

///@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   // Variables for resource element
   UtlString mUri;

   // Variables for name element
   UtlString mName;

   // Variables for instance element
   UtlString mId;
   UtlString mState;

   //Assignment operator
   Resource& operator=(const Resource& rhs);
};


//! Container for MIME type application/rlmi+xml.
/**
 * This class contains all the contents presented in a resource list
 * described in draft-ietf-simple-event-list-07.txt (A Session Initiation Protocol
 * (SIP) Event Notification Extension for Resource Lists). This class has the
 * methods to construct and manipulate the resources in a resource list.
 */
class SipResourceList : public HttpBody
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/**
 * @name ====================== Constructors and Destructors
 * @{
 */
   //! Construct an empty body of a resource list
   SipResourceList(const UtlBoolean state,
                   const char* uri,
                   const char* type);


   //! Construct from an existing resource list in the xml format
   SipResourceList(const char* bodyBytes, const char* type);

   //! Destructor that will free up the memory allocated for resource contents if it is not being deleted
   virtual
      ~SipResourceList();

///@}

/**
 * @name ====================== Resource List Serialization Interfaces
 *
 * @{
 */

   //! Build the body of this object
   void buildBody(int* version = NULL) const;
   /**< If version is non-NULL, then the body text will be built with
    *   the recorded version number (mVersion), and that version will
    *   be returned to the caller.
    *   If it is NULL, the body text will be built with the substitution
    *   placeholder, '&version;'.
    */

   //! Get the event type of this object
   void getEventType(UtlString& type) const;

   //! Get the string length of this object
   virtual ssize_t getLength() const;

   //! Get the resource list uri
   void getListUri(UtlString& uri) const;

   //! Get the serialized char representation of this resource list.
   /*! \param bytes - buffer space where the resource list is written, null
    *       terminated.
    *  \param length - the number of bytes written (not including the
    *       null terminator).
    */
   virtual void getBytes(const char** bytes,
                         ssize_t* length) const;

   //! Get the serialized string representation of this resource list.
   /*! \param bytes - buffer space where the resource list is written, null
    *       terminated.
    *  \param length - the number of bytes written (not including the
    *       null terminator).
    */
   virtual void getBytes(UtlString* bytes,
                         ssize_t* length) const;

///@}

/**
 * @name ====================== Resource List Setting Interfaces
 *
 * These methods set/get the resource element.
 *
 * @{
 */

   //! Insert a Resource object to the hash table.
   void insertResource(Resource* resource);

   //! Get the Resource object based on the resource Uri.
   Resource* getResource(UtlString& resourceUri);

   //! Remove the Resource object from the hash table.
   Resource* removeResource(Resource* resource);

   //! Insert a Event object to the hash table.
   void insertEvent(UtlContainable* event);

   //! Remove the Event object from the hash table.
   UtlContainable* removeEvent(UtlContainable* event);

   //! Check whether there is any resource or not
   UtlBoolean isEmpty();

///@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

   /// Parse an existing resource list from xml format into the internal representation.
   void parseBody(const char* bytes);


/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   //! Variables for list
   int mVersion;
   UtlString mFullState;
   UtlString mListUri;
   UtlString mEventType;

    //! reader/writer lock for synchronization
    OsBSem mLock;

   //! Variable for resource element
   UtlHashMap mResources;

   //! Variable for holding the event packages
   UtlHashMap mEvents;

   //! Disabled copy constructor
   SipResourceList(const SipResourceList& rSipResourceList);

   //! Disabled assignment operator
   SipResourceList& operator=(const SipResourceList& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipResourceList_h_
