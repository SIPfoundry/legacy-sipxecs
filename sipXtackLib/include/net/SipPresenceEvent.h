// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _SipPresenceEvent_h_
#define _SipPresenceEvent_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlHashMap.h>
#include <net/HttpBody.h>
#include <net/Url.h>
#include <os/OsDateTime.h>
#include <os/OsBSem.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

#define PRESENCE_EVENT_CONTENT_TYPE "application/pidf+xml"
#define PRESENCE_EVENT_TYPE "presence"

#define BEGIN_PRESENCE "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\""
#define END_PRESENCE "</presence>\r\n"

#define PRESENTITY_EQUAL " entity="

#define BEGIN_TUPLE "<tuple id="
#define END_TUPLE "</tuple>\r\n"

#define BEGIN_STATUS "<status>\r\n"
#define END_STATUS "</status>\r\n"

#define BEGIN_BASIC "<basic>"
#define END_BASIC "</basic>\r\n"

#define BEGIN_CONTACT "<contact>"
#define END_CONTACT "</contact>\r\n"


#define STATUS_OPEN "open"
#define STATUS_CLOSED "closed"

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//! Container for tuple element in the presence event package
/**
 * This class contains all the contents presented in a tuple element of the
 * presence information data format described in RFC 3863. This class has the
 * methods to construct and manipulate the tuple and its sub-elements.
 */

class Tuple : public UtlContainable
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/** 
 * @name ====================== Constructors and Destructors
 * @{
 */
   /// Constructor
   Tuple(const char* tupleId);

   /// Copy constructor
   Tuple(const Tuple& rTuple);

   /// Destructor
   ~Tuple();

   virtual UtlContainableType getContainableType() const;

   static const UtlContainableType TYPE;

   virtual unsigned int hash() const;

   int compareTo(const UtlContainable *b) const;

///@}
   
/**
 * @name ====================== Tuple Setting Interfaces
 *
 * These methods set/get the tuple element and sub-elements.
 *
 * @{
 */

   void setTupleId(const char* tupleId);

   void getTupleId(UtlString& tupleId) const;

   void setStatus(const char* status);

   void getStatus(UtlString& state) const;

   void setContact(const char* url, const float priority);

   void getContact(UtlString& url, float& priority) const;

///@}
   
/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:
   
/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   // Variables for tuple element
   UtlString mId;

   // Variables for status element
   UtlString mStatus;

   // Variables for contact element
   UtlString mContactUrl;
   float mPriority;

   //Assignment operator
   Tuple& operator=(const Tuple& rhs);
};


//! Container for MIME type application/pidf+xml.
/**
 * This class contains all the contents presented in a presence event package
 * described in RFC 3863. This class has the methods to construct and
 * manipulate the presence events in a presence event package.
 */
class SipPresenceEvent : public HttpBody
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/** 
 * @name ====================== Constructors and Destructors
 * @{
 */
   //! Construct an empty package or one from an existing presence event package in the xml format
   SipPresenceEvent(const char* entity, const char* bodyBytes = NULL);

   //! Destructor that will free up the memory allocated for presence contents if it is not being deleted
   virtual
      ~SipPresenceEvent();

///@}
   
/**
 * @name ====================== Presence Event Serialization Interfaces
 *
 * @{
 */

   //! Build the body of this object
   void buildBody(int& version) const;

   //! Get the string length of this object
   virtual int getLength() const;

   //! Get the serialized char representation of this presence event.
   /*! \param bytes - buffer space where the presence event is written, null
    *       terminated.
    *  \param length - the number of bytes written (not including the
    *       null terminator).
    */
   virtual void getBytes(const char** bytes,
                         int* length) const;

   //! Get the serialized string representation of this presence event.
   /*! \param bytes - buffer space where the presence event is written, null
    *       terminated.
    *  \param length - the number of bytes written (not including the
    *       null terminator).
    */
   virtual void getBytes(UtlString* bytes,
                         int* length) const;

///@}

/**
 * @name ====================== Tuple Setting Interfaces
 *
 * These methods set/get the tuple element.
 *
 * @{
 */

   //! Insert a Tuple object to the hash table.
   void insertTuple(Tuple* tuple);

   //! Get the Tuple object based on the tupleId.
   Tuple* getTuple(UtlString& tupleId);
   
   //! Remove the Tuple object.
   Tuple* removeTuple(Tuple* tuple);
   
   //! Check whether there is any Tuple or not
   UtlBoolean isEmpty();

///@}
   
/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:
   
   /// Parse an existing tuple event package from xml format into the internal representation.
   void parseBody(const char* bytes);


/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   //! Variables for presence
   UtlString mEntity;

   //! Variables for tuple element
   UtlHashMap mTuples;

    //! reader/writer lock for synchronization
    OsBSem mLock;

   //! Disabled copy constructor
   SipPresenceEvent(const SipPresenceEvent& rSipPresenceEvent);

   //! Disabled assignment operator
   SipPresenceEvent& operator=(const SipPresenceEvent& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipPresenceEvent_h_
