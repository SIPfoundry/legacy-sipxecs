//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// Written by Soma Easwaramoorthy.  Licensed to Pingtel Corp.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

/* ====================================================================
 * The Vovida Software License, Version 1.0
 *
 * Copyright (c) 2000 Vovida Networks, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The names "VOCAL", "Vovida Open Communication Application Library",
 *    and "Vovida Open Communication Application Library (VOCAL)" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact vocal@vovida.org.
 *
 * 4. Products derived from this software may not be called "VOCAL", nor
 *    may "VOCAL" appear in their name, without prior written
 *    permission of Vovida Networks, Inc.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL VOVIDA
 * NETWORKS, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT DAMAGES
 * IN EXCESS OF $1,000, NOR FOR ANY INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * ====================================================================
 *
 * This software consists of voluntary contributions made by Vovida
 * Networks, Inc. and many individuals on behalf of Vovida Networks,
 * Inc.  For more information on Vovida Networks, Inc., please see
 * <http://www.vovida.org/>.
 *
 */


/*-----------------------------------------------------------------------------
 * File: OsStunQueryAgent.cpp
 * Module: STUN
 * Description:
 *  This file contains the definition of the functions of the following classes
 *   1. StunMessage
 *   2. OsStunQueryAgent
 *-----------------------------------------------------------------------------
 */

/*-----------------------------------------------------------------------------
 * Module history
 *
 * Date			Description
 * ----			-----------
 * 26 Feb 05	Initial version of the file using vovida
 *-----------------------------------------------------------------------------
 */

/*-----------------------------------------------------------------------------
 * File inclusions:
 *  This section includes the other files that are needed by this file.
 *-----------------------------------------------------------------------------
 */
#include <cassert>

#ifdef WIN32
#include <winsock.h>
#else
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif /* WIN32 */

#include <os/OsTask.h>
#include <os/OsStunQueryAgent.h>
#include <os/OsStunDatagramSocket.h>


/*-----------------------------------------------------------------------------
 * Function: parseMessage
 * Decription:
 *  This function parses the STUN message (passed as char array) that is
 *  recieved from a server and fills the approrpriate member variable
 * Arguments:
 *  buf - The buffer (character array) recieved from the server
 *  bufLen - The length of the buffer passed
 * Default Arguments:
 *  None
 * Arguments modified:
 *  None
 * Returns:
 *  Boolean status of the success of parsing
 *-----------------------------------------------------------------------------
 */
bool StunMessage::parseMessage(char* buf, UINT bufLen) {
    /* Zerofy all the bits of the message */
    memset(this, 0, sizeof(StunMessage));

    if (sizeof(StunMsgHdr) > bufLen) { /* Message so small to be a STUN message */
        return false;
    }

    /* Extract the STUN message header */
    memcpy(&this->msgHdr, buf, sizeof(StunMsgHdr));
    /* Network to host byte order */
    this->msgHdr.msgType = ntohs(this->msgHdr.msgType);
    this->msgHdr.msgLength = ntohs(this->msgHdr.msgLength);

    if (this->msgHdr.msgLength + sizeof(StunMsgHdr) != bufLen) {
        /* Corrupted or partial message */
        return false;
    }

    /* Extract the STUN message body */
    char* body = buf + sizeof(StunMsgHdr);
    unsigned int size = this->msgHdr.msgLength;

    while ( size > 0 ) {
        /* Extract type and length of the attribute */
        StunAtrHdr* attr = reinterpret_cast<StunAtrHdr*>(body);
        /* Network to host byte order */
        unsigned int attrLen = ntohs(attr->length);
        int atrType = ntohs(attr->type);

        if ( attrLen+4 > size ) { /* Corrupted or partial message */
            return false;
        }

        /* Skip the type and length of the attribute */
        body += 4;
        size -= 4;

        switch ( atrType ) {
        case MappedAddress:
            /* Mapped address */
            this->hasMappedAddress = true;
            if ( parseAtrAddress(  body,  attrLen,  this->mappedAddress )== false ) {
                /* Invalid address */
                return false;
            }
            break;

        case ResponseAddress:
            /* Response address */
            this->hasResponseAddress = true;
            if ( parseAtrAddress(  body,  attrLen,  this->responseAddress )== false ) {
                /* Invalid address */
                return false;
            }
            break;

        case ChangeRequest:
            /* Change request */
            this->hasChangeRequest = true;
            if (parseAtrChangeRequest( body, attrLen, this->changeRequest) == false) {
                /* Invalid change request */
                return false;
            }
            break;

        case SourceAddress:
            /* Source address */
            this->hasSourceAddress = true;
            if ( parseAtrAddress(  body,  attrLen,  this->sourceAddress )== false ) {
                /* Invalid address */
                return false;
            }
            break;

        case ChangedAddress:
            /* Changed address */
            this->hasChangedAddress = true;
            if ( parseAtrAddress(  body,  attrLen,  this->changedAddress )== false ) {
                /* Invalid address */
                return false;
            }
            break;

        case ErrorCode:
            /* Error code */
            this->hasErrorCode = true;
            if (parseAtrError(body, attrLen, this->errorCode) == false) {
                /* Invalid error code */
                return false;
            }

            break;

        case UnknownAttribute:
            /* Unknown attribute */
            this->hasUnknownAttributes = true;
            if (parseAtrUnknown(body, attrLen, this->unknownAttributes) == false) {
                /* Invalid unknown attribute */
                return false;
            }
            break;

        case ReflectedFrom:
            /* Reflected from */
            this->hasReflectedFrom = true;
            if ( parseAtrAddress(  body,  attrLen,  this->reflectedFrom ) == false ) {
                /* Invalid address */
                return false;
            }
            break;

        case XorMappedAddress:
            /* XOR mapped address */
            this->hasXorMappedAddress = true;
            if ( parseAtrAddress(  body,  attrLen,  this->xorMappedAddress ) == false ) {
                /* Invalid address */
                return false;
            }
            break;

        case XorOnly:
            /* XOR only */
            this->xorOnly = true;
            break;

        case ServerName:
            /* Server name */
            this->hasServerName = true;
            if (parseAtrString( body, attrLen, this->serverName) == false) {
                /* Invalid string */
                return false;
            }
            break;

        case SecondaryAddress:
            /* Secondary address */
            this->hasSecondaryAddress = true;
            if ( parseAtrAddress(  body,  attrLen,  this->secondaryAddress ) == false ) {
                /* Invalid address */
                return false;
            }
            break;

        default:
            if ( atrType <= 0x7FFF ) {
                /* Invalid attribute */
                return false;
            }
        }

        /* Skip current attribute */
        body += attrLen;
        size -= attrLen;
    }

    return true;
}

/*-----------------------------------------------------------------------------
 * Function: encodeMessage
 * Decription:
 *  This function encodes the STUN message (from the class members) that should
 *  be sent to the server.
 * Arguments:
 *  buf - The buffer (character array) into which the message is encoded
 *  bufLen - The length of the buffer passed
 * Default Arguments:
 *  None
 * Arguments modified:
 *  buf - Contains the encoded message to be send to the server
 * Returns:
 *  The length of the encoded message in bytes.
 *-----------------------------------------------------------------------------
 */
UINT StunMessage::encodeMessage(char* buf, unsigned int bufLen) {
    /* The buffer should have room for the STUN header atleast */
    assert(bufLen >= sizeof(StunMsgHdr));
    char* ptr = buf;

    /* Encode the STUN header */
    ptr = encode16(ptr, this->msgHdr.msgType);
    char* lengthp = ptr;
    ptr = encode16(ptr, 0);
    ptr = encode(ptr, reinterpret_cast<const char*>(this->msgHdr.id.octet), sizeof(this->msgHdr.id));

    if (this->hasMappedAddress) {
        /* Encode mapped address */
        ptr = encodeAtrAddress4 (ptr, MappedAddress, this->mappedAddress);
    }
    if (this->hasResponseAddress) {
        /* Encode response address */
        ptr = encodeAtrAddress4(ptr, ResponseAddress, this->responseAddress);
    }
    if (this->hasChangeRequest) {
        /* Encode change request */
        ptr = encodeAtrChangeRequest(ptr, this->changeRequest);
    }
    if (this->hasSourceAddress) {
        /* Encode source address */
        ptr = encodeAtrAddress4(ptr, SourceAddress, this->sourceAddress);
    }
    if (this->hasChangedAddress) {
        /* Encode changed address */
        ptr = encodeAtrAddress4(ptr, ChangedAddress, this->changedAddress);
    }
    if (this->hasErrorCode) {
        /* Encode error code */
        ptr = encodeAtrError(ptr, this->errorCode);
    }
    if (this->hasUnknownAttributes) {
        /* Encode unknown attribute */
        ptr = encodeAtrUnknown(ptr, this->unknownAttributes);
    }
    if (this->hasReflectedFrom) {
        /* Encode reflected from */
        ptr = encodeAtrAddress4(ptr, ReflectedFrom, this->reflectedFrom);
    }
    if (this->hasXorMappedAddress)
    {
        /* Encode XOR mapped address */
        ptr = encodeAtrAddress4 (ptr, XorMappedAddress, this->xorMappedAddress);
    }
    if (this->xorOnly) {
        /* Encode XOR only */
        ptr = encodeXorOnly( ptr );
    }
    if (this->hasServerName) {
        /* Encode server name */
        ptr = encodeAtrString(ptr, ServerName, this->serverName);
    }
    if (this->hasSecondaryAddress) {
        /* Encode secondary address */
        ptr = encodeAtrAddress4 (ptr, SecondaryAddress, this->secondaryAddress);
    }

    /* Set STUN message length attribute in the header */
    encode16(lengthp, USHORT(ptr - buf - sizeof(StunMsgHdr)));

    /* Return the size of the encoded message in bytes */
    return int(ptr - buf);
}

/*-----------------------------------------------------------------------------
 * Function: parseAtrAddress
 * Decription:
 *  This function parses the IP address, port from the given char array
 * Arguments:
 *  body - The character array that has the IP address and port at its head
 *  hdrLen - Length of this attribute
 *  result - The STUN address to be returned
 * Default Arguments:
 *  None
 * Arguments modified:
 *  result - The parsed address on sucess
 * Returns:
 *  Boolean status of the success of parsing
 *-----------------------------------------------------------------------------
 */
bool StunMessage::parseAtrAddress (char* body, UINT hdrLen, StunAtrAddress4& result) {
    if ( hdrLen != 8 ) {
        /* Valid address attribute should be 8 bytes long */
        return false;
    }
    result.pad = *body++; /* Pad */
    result.family = *body++; /* Address family */
    if (result.family == IPv4Family) { /* We support only IPv4 */
        /* Parse the port number */
        USHORT nport;
        memcpy(&nport, body, 2); body+=2;
        result.ipv4.port = ntohs(nport);

        /* Parse the IP address */
        UINT naddr;
        memcpy(&naddr, body, 4); body+=4;
        result.ipv4.addr = ntohl(naddr);
        return true;
    }
    return false;
}

/*-----------------------------------------------------------------------------
 * Function: parseChangeRequest
 * Decription:
 *  This function parses the change request field from the given char array
 * Arguments:
 *  body - The character array that has the change request attribute  at its
 *         head
 *  hdrLen - Length of this attribute
 *  result - The change request attribute to be returned
 * Default Arguments:
 *  None
 * Arguments modified:
 *  result - The parsed change request attribute on sucess
 * Returns:
 *  Boolean status of the success of parsing
 *-----------------------------------------------------------------------------
 */
bool StunMessage::parseAtrChangeRequest (char* body, UINT hdrLen, StunAtrChangeRequest& result) {
    if ( hdrLen != 4 ) {
        /* Valid change request attribute should be 4 bytes long */
        return false;
    }
    else {
        /* Parse the change request attribute */
        memcpy(&result.value, body, 4);
        result.value = ntohl(result.value);
        return true;
    }
}

/*-----------------------------------------------------------------------------
 * Function: parseAtrError
 * Decription:
 *  This function parses the error attribute from the given char array
 * Arguments:
 *  body - The character array that has the error attribute  at its head
 *  hdrLen - Length of this attribute
 *  result - The error attribute to be returned
 * Default Arguments:
 *  None
 * Arguments modified:
 *  result - The parsed error attribute on sucess
 * Returns:
 *  Boolean status of the success of parsing
 *-----------------------------------------------------------------------------
 */
bool StunMessage::parseAtrError (char* body, UINT hdrLen, StunAtrError& result) {
    if ( hdrLen >= sizeof(result) ) {
        /* Invalid size for the error attribute */
        return false;
    }
    else {
        /* Parse pad */
        memcpy(&result.pad, body, 2); body+=2;
        result.pad = ntohs(result.pad);
        /* Parse error class */
        result.errorClass = *body++;
        result.number = *body++;

        /* Parse error reason */
        result.sizeReason = hdrLen - 4;
        memcpy(&result.reason, body, result.sizeReason);
        result.reason[result.sizeReason] = 0;
        return true;
    }

}

/*-----------------------------------------------------------------------------
 * Function: parseAtrUnknown
 * Decription:
 *  This function parses the unknown attribute from the given char array
 * Arguments:
 *  body - The character array that has the unknown attribute at its head
 *  hdrLen - Length of this attribute
 *  result - The unknown attribute to be returned
 * Default Arguments:
 *  None
 * Arguments modified:
 *  result - The parsed unknown attribute on sucess
 * Returns:
 *  Boolean status of the success of parsing
 *-----------------------------------------------------------------------------
 */
bool StunMessage::parseAtrUnknown (char* body, UINT hdrLen, StunAtrUnknown& result) {
    if ( hdrLen >= sizeof(result) ) {
        /* Invalid size for unknown attribute */
        return false;
    }
    else {
        if (hdrLen % 4 != 0) return false; /* Should be in multiples of 4 */
        result.numAttributes = hdrLen / 4; /* Find the number of attributes */
        for (int i=0; i<result.numAttributes; i++) {
            /* Parse each unknown attribute */
            memcpy(&result.attrType[i], body, 2); body+=2;
            result.attrType[i] = ntohs(result.attrType[i]);
        }
        return true;
    }
}

/*-----------------------------------------------------------------------------
 * Function: parseAtrString
 * Decription:
 *  This function parses the string attribute from the given char array
 * Arguments:
 *  body - The character array that has the string attribute at its head
 *  hdrLen - Length of this attribute
 *  result - The string attribute to be returned
 * Default Arguments:
 *  None
 * Arguments modified:
 *  result - The parsed string attribute on success
 * Returns:
 *  Boolean status of the success of parsing
 *-----------------------------------------------------------------------------
 */
bool StunMessage::parseAtrString (char* body, UINT hdrLen, StunAtrString& result) {
    if ( hdrLen >= STUN_MAX_STRING ) {
        /* Exceeds maximum string length */
        return false;
    }
    else {
        if (hdrLen % 4 != 0) {
            /* Should be in multiples of 4 */
            return false;
        }

        /* Parse the string */
        result.sizeValue = hdrLen;
        memcpy(&result.value, body, hdrLen);
        result.value[hdrLen] = 0;
        return true;
    }
}

/*-----------------------------------------------------------------------------
 * Function: encode16
 * Decription:
 *  This function encodes a 16 bit integer from host byte order to network
 *  byte order character array
 * Arguments:
 *  buf - The char array in network byte order
 *  data - The 16 bit unsigned integer to be encoded
 * Default Arguments:
 *  None
 * Arguments modified:
 *  buf - The encoded char array
 * Returns:
 *  The next location of the char array to encode
 *-----------------------------------------------------------------------------
 */
char* StunMessage::encode16(char* buf, USHORT data) {
    SHORT ndata = htons(data);
    memcpy(buf, reinterpret_cast<void*>(&ndata), sizeof(SHORT));
    return buf + sizeof(SHORT);
}

/*-----------------------------------------------------------------------------
 * Function: encode32
 * Decription:
 *  This function encodes a 32 bit integer from host byte order to network
 *  byte order character array
 * Arguments:
 *  buf - The char array in network byte order
 *  data - The 32 bit unsigned integer to be encoded
 * Default Arguments:
 *  None
 * Arguments modified:
 *  buf - The encoded char array
 * Returns:
 *  The next location of the char array to encode
 *-----------------------------------------------------------------------------
 */
char* StunMessage::encode32(char* buf, UINT data) {
    UINT ndata = htonl(data);
    memcpy(buf, reinterpret_cast<void*>(&ndata), sizeof(UINT));
    return buf + sizeof(UINT);
}

/*-----------------------------------------------------------------------------
 * Function: encode
 * Decription:
 *  This function simply copies the data from specified char array to the
 *  destination char array of given length
 * Arguments:
 *  buf - The char array in network byte order
 *  data - The char array to be encoded (also in network byte order)
 *  length - The lenght of the data to be encoded
 * Default Arguments:
 *  None
 * Arguments modified:
 *  buf - The encoded char array
 * Returns:
 *  The next location of the char array to encode
 *-----------------------------------------------------------------------------
 */
char* StunMessage::encode(char* buf, const char* data, UINT length) {
    memcpy(buf, data, length);
    return buf + length;
}

/*-----------------------------------------------------------------------------
 * Function: encodeAtrAddress4
 * Decription:
 *  This function encodes a STUN address into network byte order character
 *  array
 * Arguments:
 *  ptr - The char array in network byte order
 *  type - The type of the address
 *  atr - The address to be encoded
 * Default Arguments:
 *  None
 * Arguments modified:
 *  ptr - The encoded char array
 * Returns:
 *  The next location of the char array to encode
 *-----------------------------------------------------------------------------
 */
char* StunMessage::encodeAtrAddress4(char* ptr, USHORT type, const StunAtrAddress4& atr) {
    /* Encode type of address */
    ptr = encode16(ptr, type);
    /* Encode the length of the attribute (8) */
    ptr = encode16(ptr, 8);
    /* Encode the address */
    *ptr++ = atr.pad;
    *ptr++ = IPv4Family;
    ptr = encode16(ptr, atr.ipv4.port);
    ptr = encode32(ptr, atr.ipv4.addr);

    return ptr;
}

/*-----------------------------------------------------------------------------
 * Function: encodeAtrAddress4
 * Decription:
 *  This function encodes a change request into network byte order character
 *  array
 * Arguments:
 *  ptr - The char array in network byte order
 *  atr - The change request to be encoded
 * Default Arguments:
 *  None
 * Arguments modified:
 *  ptr - The encoded char array
 * Returns:
 *  The next location of the char array to encode
 *-----------------------------------------------------------------------------
 */
char* StunMessage::encodeAtrChangeRequest(char* ptr, const StunAtrChangeRequest& atr) {
    /* Encode type */
    ptr = encode16(ptr, ChangeRequest);
    /* Encode size of the attribute */
    ptr = encode16(ptr, 4);
    /* Encode change request */
    ptr = encode32(ptr, atr.value);
    return ptr;
}

/*-----------------------------------------------------------------------------
 * Function: encodeAtrError
 * Decription:
 *  This function encodes a error attribute into network byte order character
 *  array
 * Arguments:
 *  ptr - The char array in network byte order
 *  atr - The error attribute to be encoded
 * Default Arguments:
 *  None
 * Arguments modified:
 *  ptr - The encoded char array
 * Returns:
 *  The next location of the char array to encode
 *-----------------------------------------------------------------------------
 */
char* StunMessage::encodeAtrError(char* ptr, const StunAtrError& atr) {
    /* Encode type */
    ptr = encode16(ptr, ErrorCode);
    /* Encode size of the attribute */
    ptr = encode16(ptr, 6 + atr.sizeReason);
    /* Encode pad */
    ptr = encode16(ptr, atr.pad);
    /* Encode error class */
    *ptr++ = atr.errorClass;
    /* Encode error number */
    *ptr++ = atr.number;
    /* Encode error reason */
    ptr = encode(ptr, atr.reason, atr.sizeReason);
    return ptr;
}

/*-----------------------------------------------------------------------------
 * Function: encodeAtrUnknown
 * Decription:
 *  This function encodes a unknown attribute into network byte order character
 *  array
 * Arguments:
 *  ptr - The char array in network byte order
 *  atr - The unknown attribute to be encoded
 * Default Arguments:
 *  None
 * Arguments modified:
 *  ptr - The encoded char array
 * Returns:
 *  The next location of the char array to encode
 *-----------------------------------------------------------------------------
 */
char* StunMessage::encodeAtrUnknown(char* ptr, const StunAtrUnknown& atr) {
    /* Encode type */
    ptr = encode16(ptr, UnknownAttribute);
    /* Encode size */
    ptr = encode16(ptr, 2+2*atr.numAttributes);
    /* Encode all unknown attributes */
    for (int i=0; i<atr.numAttributes; i++) {
        ptr = encode16(ptr, atr.attrType[i]);
    }
    return ptr;
}

/*-----------------------------------------------------------------------------
 * Function: encodeXorOnly
 * Decription:
 *  This function encodes XOR only attribute into network byte order character
 *  array
 * Arguments:
 *  ptr - The char array in network byte order
 * Default Arguments:
 *  None
 * Arguments modified:
 *  ptr - The encoded char array
 * Returns:
 *  The next location of the char array to encode
 *-----------------------------------------------------------------------------
 */
char* StunMessage::encodeXorOnly(char* ptr) {
    /* Encode type */
    ptr = encode16(ptr, XorOnly );
    return ptr;
}

/*-----------------------------------------------------------------------------
 * Function: encodeAtrString
 * Decription:
 *  This function encodes a string attribute into network byte order character
 *  array
 * Arguments:
 *  ptr - The char array in network byte order
 *  type - The type of the string
 *  atr - The string to be encoded
 * Default Arguments:
 *  None
 * Arguments modified:
 *  ptr - The encoded char array
 * Returns:
 *  The next location of the char array to encode
 *-----------------------------------------------------------------------------
 */
char* StunMessage::encodeAtrString(char* ptr, USHORT type, const StunAtrString& atr) {
    assert(atr.sizeValue % 4 == 0); /* Should be in multiples of 4 */

    /* Encode type */
    ptr = encode16(ptr, type);
    /* Encode size */
    ptr = encode16(ptr, atr.sizeValue);
    /* Encode string */
    ptr = encode(ptr, atr.value, atr.sizeValue);
    return ptr;
}


bool StunMessage::isStunMessage(char* ptr, UINT length)
{
    StunMsgHdr hdr ;

    if (sizeof(StunMsgHdr) > length)
    {
        return false;
    }

    /* Extract the STUN message header */
    memcpy(&hdr, ptr, sizeof(StunMsgHdr));

    /* Network to host byte order */
    hdr.msgType = ntohs(hdr.msgType);
    hdr.msgLength = ntohs(hdr.msgLength);

    // Validate msgType
    if (    hdr.msgType != BindRequestMsg &&
            hdr.msgType != BindResponseMsg &&
            hdr.msgType != BindErrorResponseMsg &&
            hdr.msgType != SharedSecretRequestMsg &&
            hdr.msgType != SharedSecretResponseMsg &&
            hdr.msgType != SharedSecretErrorResponseMsg)
    {
        return false;
    }

    // Validate msgLength
    if (hdr.msgLength + sizeof(StunMsgHdr) != length)
    {
        return false;
    }

    return true ;
}


/*-----------------------------------------------------------------------------
 * Function: randomInt
 * Decription:
 *  This function generates a true random number
 * Arguments:
 *  None
 * Default Arguments:
 *  None
 * Arguments modified:
 *  None
 * Returns:
 *  The random number generated
 *-----------------------------------------------------------------------------
 */
int OsStunQueryAgent::randomInt ()
{
    static bool init=false;
    if ( !init ) /* For the first time, seeding is needed */
    {
        init = true;

        UInt64 tick;

#if defined(WIN32)
        volatile unsigned int lowtick=0,hightick=0;
        __asm
        {
            rdtsc
            mov lowtick, eax
            mov hightick, edx
        }
        tick = hightick;
        tick <<= 32;
        tick |= lowtick;
#elif defined(__GNUC__) && ( defined(__i686__) || defined(__i386__) )
asm("rdtsc" : "=A" (tick));
#elif defined (__SUNPRO_CC) || (defined( __sparc__ ) && !defined(__FreeBSD__))
        tick = gethrtime();
#elif defined(__pingtel_on_posix__)
        tick = getpid();
#else
#     error Need some way to seed the random number generator
#endif
        unsigned int seed = (unsigned int)tick;
#ifdef WIN32
        srand(seed);
#else
        srandom(seed);
#endif
    }

#ifdef WIN32
    assert( RAND_MAX == 0x7fff );
    int r1 = rand();
    int r2 = rand();

    int ret = (r1<<16) + r2;

    return ret;
#else
    return random();
#endif
}

/*-----------------------------------------------------------------------------
 * Function: buildReqSimple
 * Decription:
 *  This function generates a simple STUN request message
 * Arguments:
 *  msg - Pointer to the STUN message to be generated
 *  changePort - Should the change port be in place?
 *  changeIP - Should the change IP address be in place
 *  id - ID to be used in the STUN header
 * Default Arguments:
 *  None
 * Arguments modified:
 *  msg - Pointer to the generated STUN message
 * Returns:
 *  None
 *-----------------------------------------------------------------------------
 */
void OsStunQueryAgent::buildReqSimple(StunMessage *msg, bool changePort, bool changeIp, unsigned int id ) {
    assert( msg ); /* Check if the pointer is valid */
    memset( msg , 0 , sizeof(*msg) ); /* Zerofy all the bits in the message */

    /* Message type is bind request */
    msg->msgHdr.msgType = BindRequestMsg;

    /* Formulate the STUN header ID */
    for ( int i=0; i<16; i=i+4 ) {
        assert(i+3<16);
        int r = randomInt();
        msg->msgHdr.id.octet[i+0]= r>>0;
        msg->msgHdr.id.octet[i+1]= r>>8;
        msg->msgHdr.id.octet[i+2]= r>>16;
        msg->msgHdr.id.octet[i+3]= r>>24;
    }

    /* Copy the specified ID to the first octet */
    msg->msgHdr.id.octet[0] = id;

    /* Place a change request depending on what is asked */
    msg->hasChangeRequest = true;
    msg->changeRequest.value =(changeIp?ChangeIpFlag:0) |
                              (changePort?ChangePortFlag:0);
}

/*-----------------------------------------------------------------------------
 * Function: sendTest
 * Decription:
 *  This function sends a test (STUN request as specified in RFC XXXX)
 *  specified
 * Arguments:
 *  oDS - UDP socket used to send the STUN request
 *  dest - Destination address to send the STUN request
 *  testNum - The test number
 * Default Arguments:
 *  None
 * Arguments modified:
 *  None
 * Returns:
 *  None
 *-----------------------------------------------------------------------------
 */
void OsStunQueryAgent::sendTest(OsDatagramSocket *oDS, StunAddress4& dest, int testNum, int stunOptions) {
    /* Check if the destination is a valid one */
    assert( dest.addr != 0 );

    bool changePort=false;
    bool changeIP=false;
    bool discard=false;

    switch (testNum) {
    case 1:
    case 10:
    case 11:
        /* Don't change anything */
        break;
    case 2:
        /* Change IP and port */
        changeIP=true;
        break;
    case 3:
        /* Change port */
        changePort=true;
        break;
    case 4:
        /* Change IP */
        changeIP=true;
        break;
    case 5:
        /* Discard */
        discard=true;
        break;
    default:
        /* Unknown test number */
        assert(0);
    }

    StunMessage req;
    memset(&req, 0, sizeof(StunMessage));

    /* Generate a STUN request */
    if (stunOptions & STUN_OPTION_CHANGE_PORT)
    {
        changePort = TRUE ;
    }

    if (stunOptions & STUN_OPTION_CHANGE_ADDRESS)
    {
        changeIP = TRUE ;
    }

    buildReqSimple( &req, changePort , changeIP , 0x00);

    /* Encode the message into char array */
    char buf[STUN_MAX_MESSAGE_SIZE];
    int len = STUN_MAX_MESSAGE_SIZE;

    len = req.encodeMessage(buf, len);

    /* Send over the socket */
    UINT ta=htonl(dest.addr);
    oDS->write (buf, len, inet_ntoa (*((in_addr*)&ta)), dest.port);
}

/*-----------------------------------------------------------------------------
 * Constructor: OsStunQueryAgent
 * Decription:
 *  Default constructor
 * Arguments:
 *  None
 * Default Arguments:
 *  None
 * Arguments modified:
 *  None
 * Returns:
 *  Not applicable
 *-----------------------------------------------------------------------------
 */
OsStunQueryAgent::OsStunQueryAgent () {
    isValidServer=false;
}

/*-----------------------------------------------------------------------------
 * Function: setServer
 * Decription:
 *  This function sets the STUN server to be used
 * Arguments:
 *  host - hostname or IP address of the server (string format)
 *  port - The port number to be used
 * Default Arguments:
 *  port - Defaults to STUN_PORT
 * Arguments modified:
 *  None
 * Returns:
 *  The boolean status of the success of the operation
 *-----------------------------------------------------------------------------
 */
bool OsStunQueryAgent::setServer (const char *host, USHORT port)
{
    assert (host!=NULL); /* Should be a valid string */
    UtlBoolean          isIp = FALSE;

    stunServer.port=port;

    // Look up name
    // TODO: DNS SRV for stun server?
    //        http://track.sipfoundry.org/browse/XPL-96
    UtlString serverAddress ;
    if (OsSocket::getHostIpByName(host, &serverAddress))
    {
        /* Check if it is a valid IPv4 address */
        isIp = OsSocket::isIp4Address(serverAddress);
        if (isIp)
        {
            stunServer.addr=htonl(inet_addr(serverAddress));
            isValidServer=true;
        }
        else
        {
            /* Not a valid address */
            isValidServer=false;
        }
    }
    return isValidServer;
}

/*-----------------------------------------------------------------------------
 * Function: getNatType
 * Decription:
 *  This function finds the type of NAT behind which the local host is running
 * Arguments:
 *  oDS1 - UDP socket with port number n
 *  oDS2 - UDP socket with port number n+1
 * Default Arguments:
 *  None
 * Arguments modified:
 *  None
 * Returns:
 *  The enumerated constant of the type of the NAT
 *-----------------------------------------------------------------------------
 */
NatType OsStunQueryAgent::getNatType (OsDatagramSocket *oDS1, OsDatagramSocket *oDS2) {
    bool isNat=true; /* Is there a NAT or not? */
    bool respTestI=false; /* Response for Test I */

    StunAddress4 testIchangedAddr; /* Test I - Changed address */
    StunAddress4 testImappedAddr; /* Test I - Mapped address */

    bool respTestI2=false; /* Response for Test I2 (Test I send second time) */
    bool mappedIpSame = true;
    StunAddress4 testI2mappedAddr; /* Test I2 - mapped address */
    StunAddress4 testI2dest=stunServer; /* Test I2 - destination address; defaults to the STUN server */
    bool respTestII=false; /* Response for Test II */
    bool respTestIII=false; /* Response for Test III */

    bool respTestHairpin=false; /* Response for hairpinnig */

    memset(&testImappedAddr,0,sizeof(testImappedAddr));

    /* If there is no valid STUN server set at this point, exit gracefully */
    if (!isValidServer) return StunTypeFailure;

    int count=0;
    while ( count < 7 )
    {
        /* Poll the socket for data if any */
        UtlBoolean waitStatus1, waitStatus2;
        waitStatus1=oDS1->isReadyToRead (75);
        waitStatus2=oDS2->isReadyToRead (75);


        if ( (waitStatus1 == FALSE) && (waitStatus2==FALSE) ) {
            /* We don't have data to read on any of those sockets */
            count++;

            if ( !respTestI ) { /* So far we didn't receive any response to Test I */
                /* Send Test I */
#ifdef TEST
                cout << "Sending Test I" << endl;
#endif
                sendTest( oDS1, stunServer, 1);
            }

            if ( (!respTestI2) && respTestI ) { /* We have received a response to Test I but not Test I2 */
                /* Check that the address to send to is valid */
                if (  ( testI2dest.addr != 0 ) ) {
                    /* Send Test I2 */
#ifdef TEST
                    cout << "Sending Test I2" << endl;
#endif
                    sendTest( oDS1, testI2dest,10 );
                }
            }

            if ( !respTestII ) { /* We didn't get any response to Test II */
                /* Send Test II */
#ifdef TEST
                cout << "Sending Test II" << endl;
#endif
                sendTest( oDS2, stunServer,2);
            }

            if ( !respTestIII ) { /* We didn't get any response to Test III */
                /* Send Test III */
#ifdef TEST
                cout << "Sending Test III" << endl;
#endif
                sendTest( oDS2, stunServer,3);
            }

            if ( respTestI && (!respTestHairpin) ) { /* Got a response for Test I; but no response for hair pin test */
                if (  ( testImappedAddr.addr != 0 ) ) {
#ifdef TEST
                    cout << "Sending hairpin Test" << endl;
#endif
                    /* Send a hairpin test */
                    sendTest( oDS1, testImappedAddr,11);
                }
            }
        }
        else {
            /* Data is available on atleast one of the sockets */
            for ( int i=0; i<2; i++)
            {
                OsDatagramSocket *oDS;
                oDS=(i==0)?oDS1:oDS2;

                char msg[STUN_MAX_MESSAGE_SIZE];
                int msgLen = sizeof(msg);
                StunMessage resp;

                if (oDS->isReadyToRead ()) { /* See if this socket has data to read */
                    msgLen=oDS->read (msg, msgLen); /* If so, read from it */
                    memset(&resp, 0, sizeof(StunMessage));
                    resp.parseMessage( msg,msgLen); /* Parse the STUN message that is recieved */
                }

                switch( resp.msgHdr.id.octet[0] ) {
                case 1:
                    /* This is response for Test I */
#ifdef TEST
                    cout << "Recieved response for Test I" << endl;
#endif
                    if ( !respTestI ) {
                        /* This is the first response; so, set the needed values */

                        testIchangedAddr.addr = resp.changedAddress.ipv4.addr;
                        testIchangedAddr.port = resp.changedAddress.ipv4.port;
                        testImappedAddr.addr = resp.mappedAddress.ipv4.addr;
                        testImappedAddr.port = resp.mappedAddress.ipv4.port;

                        testI2dest.addr = resp.changedAddress.ipv4.addr;
                        count = 0;
                    }
                    respTestI=true;
                    break;
                case 2:
                    /* This is response for Test II */
#ifdef TEST
                    cout << "Recieved response for Test II" << endl;
#endif
                    respTestII=true;
                    break;
                case 3:
                    /* This is response for Test III */
#ifdef TEST
                    cout << "Recieved response for Test III" << endl;
#endif
                    respTestIII=true;
                    break;
                case 10:
                    /* This is response for Test I2 */
#ifdef TEST
                    cout << "Recieved response for Test I2" << endl;
#endif
                    if ( !respTestI2 )
                    {
                        /* First time you are getting response; set the needed values */
                        testI2mappedAddr.addr = resp.mappedAddress.ipv4.addr;
                        testI2mappedAddr.port = resp.mappedAddress.ipv4.port;

                        mappedIpSame = false;
                        if ( (testI2mappedAddr.addr  == testImappedAddr.addr ) &&
                                (testI2mappedAddr.port == testImappedAddr.port )) {
                            mappedIpSame = true;
                        }
                    }
                    respTestI2=true;
                    break;
                case 11:
                    /* This is response for hairpin test */
#ifdef TEST
                    cout << "Recieved response for hairpin Test" << endl;
#endif
                    respTestHairpin = true;
                    break;
                }
            }
        }
    }
    /* At this point, we need to check if there is a NAT at all */
    /* For work around, it is assumed that there is a NAT */
#if 0
    UINT tA=htonl(testImappedAddr.addr);
    OsDatagramSocket *oT=new OsDatagramSocket (0, NULL, 0, inet_ntoa(*((in_addr*)&tA)));

    if (oT->isConnected()) { /* Binding worked */
        isNat=false;
    }
    else { /* Binding failed */
        isNat=true;
    }
    delete oT;
#else
    isNat=true;
#endif

#ifdef TEST
    cout << "Test summary" << endl;
    cout << "------------" << endl;
    cout << "Test I = " << respTestI << endl;
    cout << "Test II = " << respTestII << endl;
    cout << "Test III = " << respTestIII << endl;
    cout << "Test I(2) = " << respTestI2 << endl;
    cout << "Is NAT  = " << isNat <<endl;
    cout << "Mapped IP same = " << mappedIpSame << endl;
#endif

    /* Implement logic flow chart from RFC XXXX */
    if ( respTestI ) {
        if ( isNat ) {
            if (respTestII) {
                return StunTypeConeNat;
            }
            else {
                if ( mappedIpSame ) {
                    if ( respTestIII ) {
                        return StunTypeRestrictedNat;
                    }
                    else {
                        return StunTypePortRestrictedNat;
                    }
                }
                else {
                    return StunTypeSymNat;
                }
            }
        }
        else {
            if (respTestII) {
                return StunTypeOpen;
            }
            else {
                return StunTypeSymFirewall;
            }
        }
    }
    else {
        return StunTypeBlocked;
    }

    return StunTypeUnknown;
}

/*-----------------------------------------------------------------------------
 * Function: getMappedAddress
 * Decription:
 *  This function finds the NAT mapped address for the given local IP address
 *  and UDP port
 * Arguments:
 *  oDS - UDP socket on the required port
 *  addr - The mapped IP address (string format)
 *  port - The mapped UDP port number
 * Default Arguments:
 *  None
 * Arguments modified:
 *  addr - The mapped IP address (string format)
 *  port - The mapped UDP port number
 * Returns:
 *  The boolean status of the success of the operation
 *-----------------------------------------------------------------------------
 */
bool OsStunQueryAgent::getMappedAddress (OsStunDatagramSocket *oDS, UtlString &addr, int &port, int stunOptions, const OsTime& timeout)
{
    UINT ma;
    StunMessage resp;
    char msg[STUN_MAX_MESSAGE_SIZE];
    int msgLen = sizeof(msg);
    /* Send Test I */
#ifdef TEST
    cout << "Sending Test I" << endl;
#endif
    sendTest(oDS, stunServer, 1, stunOptions);
    if (oDS->isReadyToRead(timeout.cvtToMsecs()))
    {
        msgLen = oDS->readStunPacket(msg, msgLen, timeout) ;
        if (msgLen > 0)
        {
            memset(&resp, 0, sizeof(StunMessage));
            if (resp.parseMessage(msg, msgLen))
            {
                ma=htonl (resp.mappedAddress.ipv4.addr);
                addr = inet_ntoa (*((in_addr*)&ma));
                port = resp.mappedAddress.ipv4.port;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    return true;
}


void OsStunQueryAgent::sendStunRequest(OsDatagramSocket *oDS)
{
    sendTest(oDS, stunServer, 1);
}
