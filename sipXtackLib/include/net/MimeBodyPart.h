//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _MimeBodyPart_h_
#define _MimeBodyPart_h_

// SYSTEM INCLUDES
//#include <...>
#include "utl/UtlDList.h"

// APPLICATION INCLUDES
#include <net/HttpBody.h>


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:One part of a multipart Mime body
// This is a child part of a multipart MIME body
class MimeBodyPart : public HttpBody
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   MimeBodyPart(const HttpBody* parent = NULL,
                const char* bytes = 0,
                size_t parentBodyStartIndex = 0,
                size_t rawBodyLength = 0,
                const char* contentType = NULL);

   
   //! Construct a MimeBodyPart from an HttpBody and a list of parameters.
   MimeBodyPart(const HttpBody& httpBody,
                //< Provides the bytes of the body.
                const UtlDList& parameters
                //< Provides the parameters.
      );
   /**< Does not attach the MimeBodyPart to a parent, or set the members
    *   showing its location in the parent.  For that you need attach().
    */

   MimeBodyPart(const MimeBodyPart& rMimeBodyPart);
     //:Copy constructor

   virtual MimeBodyPart* copy() const;

   virtual
   ~MimeBodyPart();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   MimeBodyPart& operator=(const MimeBodyPart& rhs);
     //:Assignment operator

   /** Update the members that locate this MimeBodyPart within its parent
    *  HttpBody.
    */
   void attach(HttpBody* parent,
               size_t rawPartStart, size_t rawPartLength,
               size_t partStart, size_t partLength);

/* ============================ ACCESSORS ================================= */

   // Get the various indexes from the object.
   size_t getRawStart() const;
   size_t getRawLength() const;
   size_t getStart() const;
   size_t getLength() const;

   virtual void getBytes(const char** bytes, size_t* length) const;

   UtlBoolean getPartHeaderValue(const char* headerName,
                                 UtlString& headerValue) const;

   UtlDList* getParameters();

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    UtlDList mNameValues;
    const HttpBody* mpParentBody;
    size_t mParentBodyRawStartIndex;
    size_t mRawBodyLength;
    size_t mParentBodyStartIndex;
    size_t mBodyLength;

};

/* ============================ INLINE METHODS ============================ */

inline size_t MimeBodyPart::getRawStart() const
{
   return mParentBodyRawStartIndex;
}

inline size_t MimeBodyPart::getRawLength() const
{
   return mRawBodyLength;
}

inline size_t MimeBodyPart::getStart() const
{
   return mParentBodyStartIndex;
}

inline size_t MimeBodyPart::getLength() const
{
   return mBodyLength;
}

inline UtlDList* MimeBodyPart::getParameters()
{
   return &mNameValues;
}

#endif  // _MimeBodyPart_h_
