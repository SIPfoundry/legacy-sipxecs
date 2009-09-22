//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////



// MailAttachment class definition for Mailer

#include "net/NetBase64Codec.h"
#include "net/MailAttachment.h"

MailAttachment::MailAttachment(const MailAttachment &original)
{
    // Copy Constructor:
    m_Filename = original.m_Filename;
    m_MIMEtype = original.m_MIMEtype;
    size_t base64Len = strlen(original.m_Base64)+1;
    m_Base64 = new char[base64Len];
    if (m_Base64)
    {
        memcpy(m_Base64, original.m_Base64, base64Len);
    }
    else
    {
        m_Base64 = NULL;
    }
}

bool MailAttachment::Load(const UtlString &filename)
{
    bool successful = false;

    // Save the filename (sans-path) and generate a MIME content type based on
    // the extension.  (Note that this code only supports a few major MIME
    // types and is intended for demonstration only.)
    m_Filename = filename;
    UtlString ext = m_Filename(m_Filename.length()-3, 3);
    ext.toLower();
    if (ext == "gif") m_MIMEtype = "image/gif";
    else if (ext == "jpg") m_MIMEtype = "image/jpeg";
    else if (ext == "zip") m_MIMEtype = "application/zip";
    else if (ext == "wav") m_MIMEtype = "audio/x-wav";
    else if (ext == "htm") m_MIMEtype = "text/html";
    else if (ext == "txt") m_MIMEtype = "text/plain";
    else m_MIMEtype = "application/octet-stream";

    // Open the file for reading
    OsFile file(filename.data());
    if (file.open() == OS_SUCCESS)
    {
        // Save the file size
        size_t fileSize;
        file.getLength(fileSize);

        // Create a buffer for the file contents
        unsigned char *buffer = new unsigned char[fileSize];
        if (buffer != NULL)
        {
            // Allocate a buffer for the base64 encoding of the file contents
            size_t nBytesForEncoding = (fileSize/3+1)*4;
            size_t nBytesForCRLF = (nBytesForEncoding/76+1)*2;
            size_t nBytesForTerminatingNull = 1;
            m_Base64 = new char[nBytesForEncoding+nBytesForCRLF+nBytesForTerminatingNull];
            if (m_Base64 != NULL)
            {
                // Read the file contents into the buffer
                size_t bytesRead;
                if ( file.read(buffer, fileSize,bytesRead) == OS_SUCCESS )
                {
                    if (bytesRead == fileSize)
                    {
                        // Perform Base64 encoding on the buffer
                        Base64Encode(buffer, fileSize);
                        successful = true;
                    }
                }
                // Deallocate the base64 buffer if not successful
                delete [] m_Base64;
                m_Base64 = NULL;
            }
            delete [] buffer;
            buffer = NULL;
        }
        file.close();
    }

    return successful;
}

bool MailAttachment::Load( const unsigned char *data, const size_t& rDatalength, const UtlString &rFilename )
{
    bool successful = false;

    // Save the filename (sans-path) and generate a MIME content type based on
    // the extension.  (Note that this code only supports a few major MIME
    // types and is intended for demonstration only.)
    m_Filename = rFilename;
    UtlString ext = m_Filename(m_Filename.length()-3, 3);
    ext.toLower();
    if (ext == "gif") m_MIMEtype = "image/gif";
    else if (ext == "jpg") m_MIMEtype = "image/jpeg";
    else if (ext == "zip") m_MIMEtype = "application/zip";
    else if (ext == "wav") m_MIMEtype = "audio/x-wav";
    else if (ext == "htm") m_MIMEtype = "text/html";
    else if (ext == "txt") m_MIMEtype = "text/plain";
    else m_MIMEtype = "application/octet-stream";

    if (data != NULL)
    {
        // Allocate a buffer for the base64 encoding of the file contents
        unsigned long nBytesForEncoding = (rDatalength/3+1)*4;
        unsigned long nBytesForCRLF = (nBytesForEncoding/76+1)*2;
        unsigned long nBytesForTerminatingNull = 1;
        m_Base64 = new char[nBytesForEncoding+nBytesForCRLF+nBytesForTerminatingNull];
        if (m_Base64 != NULL)
        {
            // Perform Base64 encoding on the buffer
            Base64Encode(data, rDatalength);
            successful = true;
        }
    }
    return successful;
}

MailAttachment::~MailAttachment()
{
    // Deallocate the base64 buffer if it exists
    if (m_Base64)
    {
        delete [] m_Base64;
        m_Base64 = NULL;
    }
}

void MailAttachment::Base64Encode(const unsigned char *buffer, unsigned long  buflen)
{

    m_Base64[0] = '\0';

    // Three bytes (24 bits) from the file
    unsigned char rawByte[3];

    // The 4-byte encoding of these 24 bits
    unsigned char encodedByte[4];

    // Number of bytes currently in the base64 encoding
    unsigned int nBytes = 0;

    // Byte index of start of line in the base64 encoding
    unsigned int iLineStart = 0;

    // The base64 character set
    unsigned char base64CharSet[] =
    { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
      'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
      'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
      'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };

    // Read 24 bits (3 bytes) at a time from the buffer
    for (unsigned long i = 0; i < buflen; i += 3)
    {
        // Read a 24 bit quantum into rawByte as three bytes
        rawByte[0] = buffer[i];
        if (i+1 < buflen) rawByte[1] = buffer[i+1]; else rawByte[1] = '\0';
        if (i+2 < buflen) rawByte[2] = buffer[i+2]; else rawByte[2] = '\0';

        // Splice the quantum into four groups of 6 bits,
        // left-padding these with zeros to make them 8 bits wide
        encodedByte[0] = (rawByte[0] & 0xFC) >> 2;
        encodedByte[1] = ((rawByte[0] & 0x03) << 4) | (rawByte[1] >> 4);
        encodedByte[2] = ((rawByte[1] & 0x0F) << 2) | (rawByte[2] >> 6);
        encodedByte[3] = (rawByte[2] & 0x3F);

        // These values are indices into the base64 character set, so
        // replace them with their respective characters
        encodedByte[0] = base64CharSet[encodedByte[0]];
        encodedByte[1] = base64CharSet[encodedByte[1]];
        encodedByte[2] = base64CharSet[encodedByte[2]];
        encodedByte[3] = base64CharSet[encodedByte[3]];

        // If the quantum was only 1 byte: replace last 2 bytes of encoding with ==
        if (i+1 >= buflen)
        {
            encodedByte[2] = '=';
            encodedByte[3] = '=';
        }

        // If the quantum was only 2 bytes: replace last byte of encoding with =
        if (i+2 >= buflen) encodedByte[3] = '=';

        // Save these to the base64 buffer
        m_Base64[nBytes++] = encodedByte[0];
        m_Base64[nBytes++] = encodedByte[1];
        m_Base64[nBytes++] = encodedByte[2];
        m_Base64[nBytes++] = encodedByte[3];

        // Lines can be a maximum of 76 characters long
        if ((nBytes-iLineStart) % 76 == 0)
        {
            m_Base64[nBytes++] = '\r';
            m_Base64[nBytes++] = '\n';
            iLineStart = nBytes;
        }
    }

    m_Base64[nBytes++] = '\0';

// DWW Found out eudora and other mail programs require \r\n at 76 chars
// something our NetBase64encode does not do!
//      UtlString encodedData;
//    NetBase64Codec::encode(buflen, buffer, encodedData);
//    memcpy(m_Base64, encodedData.data(), encodedData.length());
}
