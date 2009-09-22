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
                ssize_t parentBodyStartIndex = 0,
                ssize_t rawBodyLength = 0,
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
               ssize_t rawPartStart, ssize_t rawPartLength,
               ssize_t partStart, ssize_t partLength);

/* ============================ ACCESSORS ================================= */

   // Get the various indexes from the object.
   ssize_t getRawStart() const;
   ssize_t getRawLength() const;
   ssize_t getStart() const;
   ssize_t getLength() const;

   virtual void getBytes(const char** bytes, ssize_t* length) const;

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
    ssize_t mParentBodyRawStartIndex;
    ssize_t mRawBodyLength;
    ssize_t mParentBodyStartIndex;
    ssize_t mBodyLength;

};

/* ============================ INLINE METHODS ============================ */

inline ssize_t MimeBodyPart::getRawStart() const
{
   return mParentBodyRawStartIndex;
}

inline ssize_t MimeBodyPart::getRawLength() const
{
   return mRawBodyLength;
}

inline ssize_t MimeBodyPart::getStart() const
{
   return mParentBodyStartIndex;
}

inline ssize_t MimeBodyPart::getLength() const
{
   return mBodyLength;
}

inline UtlDList* MimeBodyPart::getParameters()
{
   return &mNameValues;
}

#endif  // _MimeBodyPart_h_
