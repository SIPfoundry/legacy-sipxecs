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
#include <utl/UtlHashBag.h>
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
#define END_PRESENCE "</presence>\n"

#define PRESENTITY_EQUAL " entity="

#define BEGIN_TUPLE "<tuple id="
#define END_TUPLE "</tuple>\n"

#define BEGIN_STATUS "<status>\n"
#define END_STATUS "</status>\n"

#define BEGIN_BASIC "<basic>"
#define END_BASIC "</basic>\n"

#define BEGIN_CONTACT "<contact>"
#define END_CONTACT "</contact>\n"


#define STATUS_OPEN "open"
#define STATUS_CLOSED "closed"

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//! Container for tuple element in the presence event package
/**
 * This class contains all the contents presented in a tuple element of the
 * Presence Information Data Format described in RFC 3863. This class has the
 * methods to construct and manipulate the tuple and its sub-elements.
 *
 * The UtlString superclass of Tuple is the 'id' datum of the Tuple.
 * Tuple inherits its hash() and compareTo() from UtlString.
 */

class Tuple : public UtlString
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

   void setContact(const char* uri, const float priority);

   void getContact(UtlString& uri, float& priority) const;

///@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   // Variables for status element
   UtlString mStatus;

   // Variables for contact element
   UtlString mContactUri;
   float mPriority;

   // Assignment operator
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
   void buildBody() const;

   //! Get the string length of this object
   //  Calls buildBody() to build the body.
   virtual ssize_t getLength() const;

   //! Get the serialized char representation of this presence event.
   /*! Calls buildBody() to build the body.
    *  \param bytes - buffer space where the presence event is written, null
    *       terminated.
    *  \param length - the number of bytes written (not including the
    *       null terminator).
    */
   virtual void getBytes(const char** bytes,
                         ssize_t* length) const;

   //! Get the serialized string representation of this presence event.
   /*! Calls buildBody() to build the body.
    *  \param bytes - buffer space where the presence event is written, null
    *       terminated.
    *  \param length - the number of bytes written (not including the
    *       null terminator).
    */
   virtual void getBytes(UtlString* bytes,
                         ssize_t* length) const;

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

   /** Make the hash bag of Tuples visible.
    *  Beware that this operation does not lock the SipPresenceEvent,
    *  so the caller must ensure no other operations are done on it
    *  as long as the hash bag is visible.
    */
   const UtlHashBag& getTuples();

   //! Get a single tuple.  Returns NULL if SipPresenceEvent has no Tuples.
   //  Only useful if SipPresenceEvent has no more than 1 Tuple.
   Tuple* getTuple();

///@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

   /// Parse an existing tuple event package from xml format into the internal representation.
   void parseBody(const char* bytes);


/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   //! The URI of the entity described by this presence event.
   UtlString mEntity;

   //! The tuples of presence information.  (Tuples are indexed by their id's.)
   UtlHashBag mTuples;

   //! Reader/writer lock for synchronization
   mutable OsBSem mLock;

   //! Disabled copy constructor
   SipPresenceEvent(const SipPresenceEvent& rSipPresenceEvent);

   //! Disabled assignment operator
   SipPresenceEvent& operator=(const SipPresenceEvent& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipPresenceEvent_h_
