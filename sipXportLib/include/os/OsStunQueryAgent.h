//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

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

#ifndef _OsStunQueryAgent_h_
#define _OsStunQueryAgent_h_

/*-----------------------------------------------------------------------------
 * File: OsStunQueryAgent.h
 * Module: STUN
 * Description:
 *  This file contains the declaration of the classes that are used for
 *  querying STUN server to find out the type of the NAT. The classes that
 *  are declared in this file are as follows:
 *   1. StunMessage
 *   2. OsStunQueryAgent
 *-----------------------------------------------------------------------------
 */

/*-----------------------------------------------------------------------------
 * Module history
 *
 * Date			Description
 * ----			-----------
 * 26 Feb 05		Initial version of the file from Vovida
 *-----------------------------------------------------------------------------
 */

/*-----------------------------------------------------------------------------
 * File inclusions:
 *  This section includes the other files that are needed by this file.
 *-----------------------------------------------------------------------------
 */
#include "os/qsTypes.h"
#include "os/OsTime.h"
#include "utl/UtlString.h"


/*-----------------------------------------------------------------------------
 * Declarations and definitions:
 *  This section contains the local declarations and definitions of constants,
 *  macros, typdefs, etc.
 *-----------------------------------------------------------------------------
 */

class OsStunDatagramSocket ;
class OsDatagramSocket ;

/* Some STUN constants */
#define STUN_MAX_STRING 256
#define STUN_MAX_UNKNOWN_ATTRIBUTES 8
#define STUN_MAX_MESSAGE_SIZE 2048
#define STUN_PORT 3478

/* IP addres family */
const UCHAR  IPv4Family = 0x01;
const UCHAR  IPv6Family = 0x02;

/* Define STUN flags */
const UINT ChangeIpFlag   = 0x04;
const UINT ChangePortFlag = 0x02;

/* Define STUN attributes */
const USHORT MappedAddress    = 0x0001;
const USHORT ResponseAddress  = 0x0002;
const USHORT ChangeRequest    = 0x0003;
const USHORT SourceAddress    = 0x0004;
const USHORT ChangedAddress   = 0x0005;
const USHORT Username         = 0x0006;
const USHORT Password         = 0x0007;
const USHORT MessageIntegrity = 0x0008;
const USHORT ErrorCode        = 0x0009;
const USHORT UnknownAttribute = 0x000A;
const USHORT ReflectedFrom    = 0x000B;
const USHORT XorMappedAddress = 0x0020;
const USHORT XorOnly          = 0x0021;
const USHORT ServerName       = 0x0022;
const USHORT SecondaryAddress = 0x0050;  /* Non standard extention */

/* Define types for a STUN message */
const USHORT BindRequestMsg               = 0x0001;
const USHORT BindResponseMsg              = 0x0101;
const USHORT BindErrorResponseMsg         = 0x0111;
const USHORT SharedSecretRequestMsg       = 0x0002;
const USHORT SharedSecretResponseMsg      = 0x0102;
const USHORT SharedSecretErrorResponseMsg = 0x0112;

/* Stun message header
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |      Stun Message Type        |         Message Length        |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *                            Transaction ID
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *                                                                   |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
typedef struct
{
    USHORT msgType;
    USHORT msgLength;
    UINT128 id;
} StunMsgHdr;

/* Stun attribute header
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |         Type                  |            Length             |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
typedef struct
{
    USHORT type;
    USHORT length;
} StunAtrHdr;

/* Stun IPv4 address */
typedef struct
{
    USHORT port;
    UINT addr;
} StunAddress4;

/* Mapped address attribute
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |x x x x x x x x|    Family     |           Port                |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                             Address                           |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
typedef struct
{
    UCHAR pad;
    UCHAR family;
    StunAddress4 ipv4;
} StunAtrAddress4;

/* Change request attribute
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 A B 0|
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
typedef struct
{
    UINT value;
} StunAtrChangeRequest;

/* Error code attribute
 *     0                   1                   2                   3
 *     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                   0                     |Class|     Number    |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |      Reason Phrase (variable)                                ..
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
typedef struct
{
    USHORT pad;  /* all 0 */
    UCHAR errorClass;
    UCHAR number;
    char reason[STUN_MAX_STRING];
    USHORT sizeReason;
} StunAtrError;

/* Unknown attributes
 *     0                   1                   2                   3
 *     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |      Attribute 1 Type           |     Attribute 2 Type        |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |      Attribute 3 Type           |     Attribute 4 Type    ...
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
typedef struct
{
    USHORT attrType[STUN_MAX_UNKNOWN_ATTRIBUTES];
    USHORT numAttributes;
} StunAtrUnknown;

/* String attribute */
typedef struct
{
    char value[STUN_MAX_STRING];
    USHORT sizeValue;
} StunAtrString;

/* Define enum with different types of NAT */
typedef enum
{
    StunTypeUnknown=0,
    StunTypeOpen,
    StunTypeConeNat,
    StunTypeRestrictedNat,
    StunTypePortRestrictedNat,
    StunTypeSymNat,
    StunTypeSymFirewall,
    StunTypeBlocked,
    StunTypeFailure
} NatType;

/*-----------------------------------------------------------------------------
 * Class: StunMessage
 * Inherits: None
 * Description:
 *  This class is used to represent and manipulate the STUN messages.
 *-----------------------------------------------------------------------------
 */
class StunMessage
{
public:
    /* Parse the STUN message received from the server and
       translate it to the attributes present in the class */
    bool parseMessage (char *buf, UINT bufLen);

    /* Encode a STUN message that has to be sent to the server
       from the attributes of the class */
    UINT encodeMessage(char* buf, UINT bufLen);

    /* Determine if the buffer contains a valid stun message */
    static bool isStunMessage(char* ptr, UINT length) ;
protected:
    /* Parse and translate the address field in the message */
    static bool parseAtrAddress (char* body, UINT hdrLen, StunAtrAddress4& result);

    /* Parse and translate the change request field in the message */
    static bool parseAtrChangeRequest (char* body, UINT hdrLen, StunAtrChangeRequest& result);

    /* Parse and translate the error field in the message */
    static bool parseAtrError (char* body, UINT hdrLen, StunAtrError& result);

    /* Parse and translate the unknown field in the message */
    static bool parseAtrUnknown (char* body, UINT hdrLen, StunAtrUnknown& result);

    /* Parse and translate the string field in the message */
    static bool parseAtrString (char* body, UINT hdrLen, StunAtrString& result);

    /* Encode a 16 bit integer from host order to network order */
    static char* encode16(char* buf, USHORT data);

    /* Encode a 32 bit integer from host order to network order */
    static char* encode32(char* buf, UINT data);

    /* Simply copy the string without any special treatment */
    static char* encode(char* buf, const char* data, UINT length);

    /* Encode address field in the message */
    static char* encodeAtrAddress4(char* ptr, USHORT type, const StunAtrAddress4& atr);

    /* Encode change request field in the message */
    static char* encodeAtrChangeRequest(char* ptr, const StunAtrChangeRequest& atr);

    /* Encode error field in the message */
    static char* encodeAtrError(char* ptr, const StunAtrError& atr);

    /* Encode unknown field in the message */
    static char* encodeAtrUnknown(char* ptr, const StunAtrUnknown& atr);

    /* Encode XOR only */
    static char* encodeXorOnly(char* ptr);

    /* Encode string field in the message */
    static char* encodeAtrString(char* ptr, USHORT type, const StunAtrString& atr);



protected:
    /* STUN header */
    StunMsgHdr msgHdr;

    /* Mapped address */
    bool hasMappedAddress;
    StunAtrAddress4  mappedAddress;

    /* Response address */
    bool hasResponseAddress;
    StunAtrAddress4  responseAddress;

    /* Change request */
    bool hasChangeRequest;
    StunAtrChangeRequest changeRequest;

    /* Source address */
    bool hasSourceAddress;
    StunAtrAddress4 sourceAddress;

    /* Changes address */
    bool hasChangedAddress;
    StunAtrAddress4 changedAddress;

    /* Error code */
    bool hasErrorCode;
    StunAtrError errorCode;

    /* Unknown */
    bool hasUnknownAttributes;
    StunAtrUnknown unknownAttributes;

    /* Reflected from */
    bool hasReflectedFrom;
    StunAtrAddress4 reflectedFrom;

    /* XOR mapped address */
    bool hasXorMappedAddress;
    StunAtrAddress4  xorMappedAddress;

    /* XOR only */
    bool xorOnly;

    /* Server name */
    bool hasServerName;
    StunAtrString serverName;

    /* Secondary address */
    bool hasSecondaryAddress;
    StunAtrAddress4 secondaryAddress;

    /* OsStunQueryAgent is declared as friend so that it can freely access
       the private/protected variables/functions. */
    friend class OsStunQueryAgent;
    friend class OsStunAgentTask;
};

/*-----------------------------------------------------------------------------
 * Class: OsStunQueyAgent
 * Inherits: None
 * Description:
 *  This class is used to do STUN queries
 *-----------------------------------------------------------------------------
 */
class OsStunQueryAgent
{
public:
    /* Default constructor */
    OsStunQueryAgent ();

    /* Set the STUN server address and port */
    bool setServer (const char *host, USHORT port=STUN_PORT);

    /* Get the NAT type behind which the localhost runs */
    NatType getNatType (OsDatagramSocket *oDS1, OsDatagramSocket *oDS2);

    /* Get the mapped IP address and port for a given local host IP address
       and port */
    bool getMappedAddress(OsStunDatagramSocket *oDS, UtlString &server, int &port, int stunOptions, const OsTime& timeout);

    /* Send a stun request on the supplied socket */
    void sendStunRequest(OsDatagramSocket *oDS);
protected:
    /* Generate a true random number */
    static int randomInt ();

    /* Build a simple STUN request */
    static void buildReqSimple(StunMessage *msg, bool changePort, bool changeIp, UINT id);

    /* Send STUN test as specified in the RFC XXXX */
    static void sendTest (OsDatagramSocket *oDS, StunAddress4& dest, int testNum, int stunOptions = 0);


protected:
    /* STUN server to be queried */
    StunAddress4 stunServer;

    /* Is a valid server defined */
    bool isValidServer;
};

#endif // _OsStunQueryAgent_h_
