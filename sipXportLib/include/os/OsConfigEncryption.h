//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsConfigEncryption_h_
#define _OsConfigEncryption_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsEncryption.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsConfigDb; // for some reason, #include os/OsConfigDb was not working

/*! OsConfigDb consults a this class to handle all the details of encrypting and
 decrypting files and buffers w/o knowing the details. Systems provide an implemenation
 of this and in the case of the phone, determines which profiles should be
 encrypted, what the key is and can configure the OsEncryption instance
 */
class OsConfigEncryption
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    //! Test a buffer if it's actually encrypted
    virtual UtlBoolean isEncrypted(OsConfigDb *cfg, const char *buffer, int buffLen) = 0;

    //! Is this profile typically encrypted, decision usually based from cfg->getIndentyLabel()
    virtual UtlBoolean isNormallyEncrypted(OsConfigDb *cfg) = 0;

    //! Handle the details of encrypting, look in OsEncryption instance for results
    virtual OsStatus encrypt(OsConfigDb *cfg, OsEncryption *encryption, char *buffer, int buffLen) = 0;

    //! Handle the details of decrypting, look in OsEncryption instance for results
    virtual OsStatus decrypt(OsConfigDb *cfg, OsEncryption *encryption, char *buffer, int buffLen) = 0;

    //! Is writing profile encrypted on/off at a system level.
    virtual UtlBoolean isWriteEncryptedEnabled() = 0;

    //! If not NULL, a binary prefix header on files to tell if files are encrypted or not
    // virtual const unsigned char *getFileHeader(int& headerLen) = 0;

    virtual ~OsConfigEncryption(){};
};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsConfigEncryption_h_
