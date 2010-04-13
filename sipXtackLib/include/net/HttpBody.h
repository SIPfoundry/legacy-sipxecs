//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _HttpBody_h_
#define _HttpBody_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <utl/UtlSList.h>
#include <utl/UtlDList.h>

// DEFINES
#define CONTENT_TYPE_TEXT_PLAIN "text/plain"
#define CONTENT_TYPE_TEXT_HTML "text/html"
#define CONTENT_SMIME_PKCS7 "application/pkcs7-mime"
#define CONTENT_TYPE_MULTIPART "multipart/"
#define CONTENT_TYPE_MULTIPART_RELATED "multipart/related"

#define MULTIPART_BOUNDARY_PARAMETER "boundary"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MimeBodyPart;

//! class to contain an HTTP body
/*! This is the base class and container for all HTTP (SIP, etc.)
 * message bodies.  This includes multipart MIME bodies, single
 * part MIME and specific MIME types.  The HttpBody is essentially
 * a container for a blob.
 */
class HttpBody : public UtlString
{
   friend class HttpBodyMultipart;

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   HttpBody(const char* bytes = NULL,
            ssize_t length = -1,
            const char* contentType = NULL);
   //: Construct an HttpBody from a bunch of bytes

   HttpBody(const char* contentType);
   //: Construct a multipart HttpBody with zero parts.
   // contentType should have no "boundary" parameter.

   HttpBody(const HttpBody& rHttpBody);
     //:Copy constructor

   virtual HttpBody* copy() const;

   virtual
   ~HttpBody();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   HttpBody& operator=(const HttpBody& rhs);
   //:Assignment operator

   //! Pseudo body factory
   static HttpBody* createBody(const char* bodyBytes,
                               ssize_t bodyBytesLength,
                               const char* contentType,
                               const char* contentEncoding);

   //! Append a multipart body part to an existing multiparty body.
   void appendBodyPart(const HttpBody& body,
                       const UtlDList& parameters);

/* ============================ ACCESSORS ================================= */

   virtual ssize_t getLength() const;

   // Note: for convenience, bytes is null terminated
   // However depending upon the content type, the body may
   // contain more than one null character.
   // *bytes != NULL, even if *length == 0.
   virtual void getBytes(const char** bytes, ssize_t* length) const;
   virtual void getBytes(UtlString* bytes, ssize_t* length) const;
   virtual const char* getBytes() const;

   UtlBoolean getMultipartBytes(int partIndex,
                                const char** bytes,
                                ssize_t* length) const;

   const MimeBodyPart* getMultipart(int partIndex) const;

   int getMultipartCount() const;

   //! Get the multipart boundary string.
   // Valid while HttpBody exists and is not modified.
   const char* getMultipartBoundary() const;

   //! Get the content type string.
   // Valid while HttpBody exists and is not modified.
   const char* getContentType() const;

/* ============================ INQUIRY =================================== */

   UtlBoolean isMultipart() const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   ssize_t bodyLength;
   UtlString mBody;
   UtlString mMultipartBoundary;
   int mBodyPartCount;          // Only significant if ::isMultipart() == true.
   UtlSList mBodyParts;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   // The counter for generating boundary values.
   static unsigned boundaryCounter;
   // Generate the next boundary value.
   static void nextBoundary(UtlString& boundary);
};


// Carrier class to provide a constructor that would otherwise conflict
// with an existing constructor for HttpBody.
class HttpBodyMultipart : public HttpBody
{
  public:

   HttpBodyMultipart(const char* contentType);
   //: Construct a multipart HttpBody with zero parts.
   // contentType should have no "boundary" parameter.
};


/* ============================ INLINE METHODS ============================ */

#endif  // _HttpBody_h_
