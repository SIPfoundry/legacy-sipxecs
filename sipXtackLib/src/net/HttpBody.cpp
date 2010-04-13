//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

#include <string.h>
#include <assert.h>

// APPLICATION INCLUDES
#include <os/OsSysLog.h>
#include <utl/UtlSList.h>
#include <utl/UtlSListIterator.h>
#include <utl/UtlDList.h>
#include <utl/UtlDListIterator.h>
#include <net/HttpBody.h>
#include <net/SdpBody.h>
#include <net/SmimeBody.h>
#include <net/MimeBodyPart.h>
#include <net/SipDialogEvent.h>
#include <net/NameValueTokenizer.h>
#include <net/HttpMessage.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// The number of hex chars to use for boundary strings.
#define BOUNDARY_STRING_LENGTH 8
// Mask to extract the low (BOUNDARY_STRING_LENGTH*4) bits of an unsigned int.
#define BOUNDARY_COUNTER_MASK 0xFFFFFFFF

// STATIC VARIABLE INITIALIZATIONS

unsigned HttpBody::boundaryCounter = 0;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
HttpBody::HttpBody(const char* bytes, ssize_t length, const char* contentType) :
   bodyLength(0),
   mBodyPartCount(0)
{
   if (contentType)
   {
      append(contentType);
      NameValueTokenizer::frontBackTrim(this, " \t");
      //osPrintf("Content type: \"%s\"\n", mBodyContentType.data());
      ssize_t boundaryIndex = index(MULTIPART_BOUNDARY_PARAMETER,
                                0, UtlString::ignoreCase);

      if(boundaryIndex >=0 &&
         index(CONTENT_TYPE_MULTIPART,
               0, UtlString::ignoreCase) == 0)
      {
         boundaryIndex += strlen(MULTIPART_BOUNDARY_PARAMETER);
         //osPrintf("Boundary start:=>%s\n",
         //    (mBodyContentType.data())[boundaryIndex]);

         // Allow white space before =
         ssize_t fieldLength = this->length();
         while(boundaryIndex < fieldLength &&
               (data()[boundaryIndex] == ' ' ||
                data()[boundaryIndex] == '\t'))
            boundaryIndex++;

         if(data()[boundaryIndex] == '=')
         {
            mMultipartBoundary.append(&data()[boundaryIndex + 1]);
            NameValueTokenizer::frontTrim(&mMultipartBoundary, " \t");
            ssize_t whiteSpaceIndex = mMultipartBoundary.first(' ');
            if(whiteSpaceIndex > 0) mMultipartBoundary.remove(whiteSpaceIndex);
            whiteSpaceIndex = mMultipartBoundary.first('\t');
            if(whiteSpaceIndex > 0) mMultipartBoundary.remove(whiteSpaceIndex);
            //osPrintf("HttpBody: boundary=\"%s\"\n", mMultipartBoundary.data());
         }
      }
   }

   if(bytes && length < 0) length = strlen(bytes);
   if(bytes && length > 0)
   {
      if (mBody.append(bytes, length).length() > 0) //append was successful
      {
         bodyLength = length;

         if(isMultipart())
         {
            for(unsigned int partIndex = 0;; partIndex++)
            {
               UtlString contentType;
               UtlString name;
               UtlString value;
               const char* partBytes;
               const char* parentBodyBytes;
               ssize_t partLength;
               ssize_t parentBodyLength;
               getBytes(&parentBodyBytes, &parentBodyLength);
               getMultipartBytes(partIndex, &partBytes, &partLength);
               //osPrintf("Body part 1 length: %d\n", firstPart.length());
               //osPrintf("++++ Multipart Body #1 ++++\n%s\n++++ End Multipart #1 ++++\n",
               //    firstPart.data());
               if(partLength <= 0) break;

               // Parse throught the header to the MIME part
               // The first blank line is the begining of the part body
               NameValueTokenizer parser(partBytes, partLength);
                 do
                 {
                     parser.getNextPair(HTTP_NAME_VALUE_DELIMITER,
                     &name, & value);
                     if(name.compareTo(HTTP_CONTENT_TYPE_FIELD) == 0)
                     {
                         contentType = name;
                     }
                 }
                 while(!name.isNull());

               // This is a bit of a temporary kludge
               //Prepend a HTTP header to make it look like a HTTP message
               //partBytes.insert(0, "GET / HTTP/1.0\n");
               //HttpMessage firstPartMessage(partBytes.data(), partBytes.length());
               //const HttpBody* partFileBody = firstPartMessage.getBody();
               //ssize_t bytesLeft = parser.getProcessedIndex() - partLength;

               if (partLength > 0)
               {
                  mBodyParts.append(new MimeBodyPart(this, partBytes, partBytes - parentBodyBytes,
                                                            partLength,contentType.data()));
                  // Save the number of body parts.
                  mBodyPartCount = partIndex + 1;
               }
            }
         }
      }
   }
}

// Construct a multipart HttpBody with zero parts.
HttpBodyMultipart::HttpBodyMultipart(const char* contentType) :
   HttpBody(NULL, -1, contentType)
{
   // Create the boundary.
   nextBoundary(mMultipartBoundary);
   // Write it into the body.
   mBody = "--";
   mBody.append(mMultipartBoundary);
   mBody.append("--\r\n");
   // Add the boundary parameter to the type.
   append(";" MULTIPART_BOUNDARY_PARAMETER "=\"");
   append(mMultipartBoundary);
   append("\"");
   // No need to check validity of the boundary string, as there is no
   // body content for it to appear in.
}

// Copy constructor
HttpBody::HttpBody(const HttpBody& rHttpBody) :
   UtlString(rHttpBody),
   bodyLength(rHttpBody.bodyLength),
   mBody(rHttpBody.mBody),
   mMultipartBoundary(rHttpBody.mMultipartBoundary),
   mBodyPartCount(rHttpBody.mBodyPartCount)
{
   UtlSListIterator iterator(rHttpBody.mBodyParts);
   MimeBodyPart* pMimeBodyPart;
   while ((pMimeBodyPart = dynamic_cast<MimeBodyPart*>(iterator())))
   {
       mBodyParts.append(new MimeBodyPart(*pMimeBodyPart));
   }
}

// Destructor
HttpBody::~HttpBody()
{
   mBodyParts.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
HttpBody&
HttpBody::operator=(const HttpBody& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

    mBody = rhs.mBody;

    bodyLength = rhs.bodyLength;

    // Set the content type
    remove(0);
    append(rhs);

    mMultipartBoundary = rhs.mMultipartBoundary;

    mBodyPartCount = rhs.mBodyPartCount;

   // Delete the old list.
   mBodyParts.destroyAll();

   // Copy the list from the RHS.
   UtlSListIterator rhsIterator(rhs.mBodyParts);
   MimeBodyPart* pMimeBodyPart;
   while ((pMimeBodyPart = dynamic_cast<MimeBodyPart*>(rhsIterator())))
   {
       mBodyParts.append(new MimeBodyPart(*pMimeBodyPart));
   }

   return *this;
}

// Generic copy
HttpBody* HttpBody::copy() const
{
  return new HttpBody(*this);
}

// Pseudo factory
HttpBody* HttpBody::createBody(const char* bodyBytes,
                               ssize_t bodyLength,
                               const char* contentType,
                               const char* contentEncoding)
{
    HttpBody* body = NULL;

    UtlString contentTypeString;
    if(contentType)
    {
        contentTypeString.append(contentType);
        contentTypeString.toLower();
    }
    if(contentType &&
       strcmp(contentTypeString.data(), SDP_CONTENT_TYPE) == 0)
    {
        body = new SdpBody(bodyBytes, bodyLength);
    }
    else if(contentType &&
            strcmp(contentTypeString.data(), CONTENT_SMIME_PKCS7) == 0)
    {
        body = new SmimeBody(bodyBytes, bodyLength, contentEncoding);
    }
    else if ((bodyLength  > 1) ||
             (bodyBytes[0] != '\n'))
    {
        body = new HttpBody(bodyBytes, bodyLength,
                            contentType);
    }

    return(body);
}

// Append a multipart body part to an existing multiparty body.
void HttpBody::appendBodyPart(const HttpBody& body,
                              const UtlDList& parameters)
{
   assert(isMultipart());

   // Construct a new MimeBodyPart for the new body part.
   MimeBodyPart* part = new MimeBodyPart(body, parameters);

   // Insert it as the last body part.
   mBodyParts.append(part);
   mBodyPartCount++;

   // Turn the final boundary into an intermediate boundary.
   mBody.remove(mBody.length() - 4);
   mBody.append("\r\n");

   // Insert the headers.
   ssize_t rawPartStart = mBody.length();
   UtlDListIterator iterator(*part->getParameters());
   NameValuePair* nvp;
   while ((nvp = (NameValuePair*) iterator()))
   {
      mBody.append(nvp->data());
      mBody.append(": ");
      mBody.append(nvp->getValue());
      mBody.append("\r\n");
   }
   mBody.append("\r\n");

   // Insert the body.
   ssize_t partStart = mBody.length();
   const char* bytes;
   ssize_t length;
   body.getBytes(&bytes, &length);
   mBody.append(bytes, length);
   ssize_t partEnd = mBody.length();

   // Update bodyLength.
   bodyLength = mBody.length();

   // Determine if we have to change the boundary string.
   bool change_boundary_string =
      mBody.index(mMultipartBoundary, partStart) != UTL_NOT_FOUND;

   // Add the final boundary.
   mBody.append("\r\n--");
   mBody.append(mMultipartBoundary);
   mBody.append("--\r\n");

   // Update the MimeBodyPart to know where it is contained in the HttpBody.
   part->attach(this,
                rawPartStart, partEnd - rawPartStart,
                partStart, partEnd - partStart);

   // If we have to change the boundary string.
   if (change_boundary_string)
   {
      // Find a new boundary string that isn't in the body.
      do {
         nextBoundary(mMultipartBoundary);
      } while (mBody.index(mMultipartBoundary) != UTL_NOT_FOUND);

      // Replace the old boundary string.
      UtlSListIterator iterator(mBodyParts);
      MimeBodyPart* part;
      while ((part = dynamic_cast<MimeBodyPart*>(iterator())))
      {
            // Replace the boundary string just before this part.
            mBody.replace(part->getRawStart() - (2 + BOUNDARY_STRING_LENGTH),
                          BOUNDARY_STRING_LENGTH,
                          mMultipartBoundary.data(),
                          BOUNDARY_STRING_LENGTH);
      }

      // Replace the boundary string in the final boundary.
      mBody.replace(mBody.length() - (4 + BOUNDARY_STRING_LENGTH),
                    BOUNDARY_STRING_LENGTH,
                    mMultipartBoundary.data(),
                    BOUNDARY_STRING_LENGTH);

      // Replace the boundary string in the Content-Type.
      ssize_t loc = this->index(";" MULTIPART_BOUNDARY_PARAMETER "=\"");
      this->replace(loc + sizeof (";" MULTIPART_BOUNDARY_PARAMETER "=\"") - 1,
                    BOUNDARY_STRING_LENGTH,
                    mMultipartBoundary.data(),
                    BOUNDARY_STRING_LENGTH);
   }
}

/* ============================ ACCESSORS ================================= */

ssize_t HttpBody::getLength() const
{
   return bodyLength;
}

void HttpBody::getBytes(const char** bytes, ssize_t* length) const
{
   *bytes = mBody.data();
   *length = mBody.length();
}

void HttpBody::getBytes(UtlString* bytes, ssize_t* length) const
{
   bytes->remove(0);
   const char* bytePtr;
   getBytes(&bytePtr, length);
   if (*length > 0)
   {
      //hint to the string to change the capacity to the new length.
      //if this fails, we may not have enough ram to complete this operation
      size_t newLength = (*length);
      if (bytes->capacity(newLength) >= newLength)
      {
         bytes->append(bytePtr, *length);
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "HttpBody::getBytes allocation failure to reserve %zu bytes", newLength);
      }
   }
}

const char* HttpBody::getBytes() const
{
   return mBody.data();
}

const char* HttpBody::getMultipartBoundary() const
{
   return mMultipartBoundary.data();
}

const char* HttpBody::getContentType() const
{
   return data();
}

UtlBoolean HttpBody::getMultipartBytes(int partIndex,
                                       const char** bytes,
                                       ssize_t* length) const
{
    UtlBoolean partFound = FALSE;
    if(!mMultipartBoundary.isNull())
    {
        ssize_t byteIndex = -1;
        int partNum = -1;
        ssize_t partStartIndex = -1;
        ssize_t partEndIndex = -1;
        do
        {
            byteIndex = mBody.index(mMultipartBoundary.data(), byteIndex + 1);
            if(byteIndex >= 0)
            {
                partNum++;
                if(partNum == partIndex)
                {
                    partStartIndex = byteIndex + mMultipartBoundary.length();
                    if((mBody.data())[partStartIndex] == '\r') partStartIndex++;
                    if((mBody.data())[partStartIndex] == '\n') partStartIndex++;
                }
                else if(partNum == partIndex + 1)
                {
                    partEndIndex = byteIndex - 3;
                    //osPrintf("Part End Index: %d\n", partEndIndex);
                    //osPrintf("End of file: %c %d\n", mBody.data()[partEndIndex],
                    //    (int) ((mBody.data())[partEndIndex]));
                    if(((mBody.data())[partEndIndex]) == '\n') partEndIndex--;
                    //osPrintf("End of file: %c %d\n", mBody.data()[partEndIndex],
                    //    (int) ((mBody.data())[partEndIndex]));
                    if(((mBody.data())[partEndIndex]) == '\r') partEndIndex--;
                    //osPrintf("End of file: %c %d\n", mBody.data()[partEndIndex],
                    //    (int) ((mBody.data())[partEndIndex]));
                }
            }
        }
        while(partNum <= partIndex && byteIndex >= 0);
        if(partStartIndex >= 0 && partEndIndex > 0)
        {
            *bytes = &(mBody.data()[partStartIndex]);
            *length = partEndIndex - partStartIndex + 1;
            partFound = TRUE;
        }
        else
        {
            *bytes = NULL;
            *length = 0;
        }
    }
    return(partFound);
}

const MimeBodyPart* HttpBody::getMultipart(int index) const
{
    const MimeBodyPart* bodyPart = NULL;

    if(index >= 0 && isMultipart())
    {
        bodyPart = dynamic_cast<MimeBodyPart*>(mBodyParts.at(index));
    }
    return(bodyPart);
}

int HttpBody::getMultipartCount() const
{
   return mBodyPartCount;
}

/* ============================ INQUIRY =================================== */
UtlBoolean HttpBody::isMultipart() const
{
    return(!mMultipartBoundary.isNull());
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void HttpBody::nextBoundary(UtlString& boundary)
{
   boundaryCounter += 0x54637281; // This constant is arbitrary, but it must
                                  // be odd.
   char buffer[BOUNDARY_STRING_LENGTH + 1];
   // Need to trim boundary counter to the needed length, as
   // "unsigned" may be longer.
   sprintf(buffer, "%0*x",
           BOUNDARY_STRING_LENGTH, boundaryCounter & BOUNDARY_COUNTER_MASK);
   boundary = buffer;
}
