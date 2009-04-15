// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef MESSAGEIDGENERATOR_H
#define MESSAGEIDGENERATOR_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsMutex.h"
#include "os/OsStatus.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * MessageIDGenerator
 *
 * @author John P. Coffey
 * @version 1.0
 */
class MessageIDGenerator
{
public:
    /**
     * Singleton Accessor
     *
     * @return
     */
    static MessageIDGenerator* getInstance(const UtlString&);

    /**
     * Constructor
     */
    MessageIDGenerator(const UtlString&);

    /**
     * Virtual Destructor
     */
    virtual ~MessageIDGenerator();

    /** Fetches and increments the sequence number on the file system */
    OsStatus getNextMessageID ( UtlString& rMessageName ) const;

protected:
    /** 
     * Method to recursively search the mailbox folder hierarchy for the highest
     * MessageID, this returns that ID + 1 and writes to the file sytem
     */
    OsStatus recoverMessageID ( UtlString& rMessageName ) const;
    /**
     * Ctor - protected for singleton
     */
    MessageIDGenerator();
private:
    // Singtleton instance
    static MessageIDGenerator* spInstance;

    // Exclusive binary lock
    static OsMutex sLockMutex;

    // Fully qualified filename 
    UtlString m_dataFileName;
};

#endif //MESSAGEIDGENERATOR_H
