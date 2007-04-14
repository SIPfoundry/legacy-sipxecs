//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdlib.h>

// APPLICATION INCLUDES
#include "os/OsFS.h"
#include "os/OsLock.h"
#include "os/OsSysLog.h"
#include "mailboxmgr/MessageIDGenerator.h"
#include "sipxcgi/CgiValues.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern CgiValues *gValues;

// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
// STATIC INITIALIZERS
MessageIDGenerator* MessageIDGenerator::spInstance = NULL;
OsMutex             MessageIDGenerator::sLockMutex (OsMutex::Q_FIFO);

/* ============================ CREATORS ================================== */

MessageIDGenerator::MessageIDGenerator(const UtlString& mailstoreRoot)
{
   // This is the filename for the ID generator text file.
   m_dataFileName = mailstoreRoot + OsPathBase::separator + "messageid.txt";
   OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                 "MessageIDGenerator::MessageIDGenerator: setting m_dataFileName = '%s'",
                 m_dataFileName.data());
}

MessageIDGenerator::~MessageIDGenerator()
{}

MessageIDGenerator*
MessageIDGenerator::getInstance(const UtlString& mailstoreRoot)
{
    // Critical Section here
    OsLock lock( sLockMutex );

    // See if this is the first time through for this process
    // Note that this being null => pgDatabase is also null
    if ( spInstance == NULL )
    {
       // Create the singleton instance for clients to use.
      spInstance = new MessageIDGenerator(mailstoreRoot);
    }
    return spInstance;
}

OsStatus
MessageIDGenerator::getNextMessageID ( UtlString& rMessageName ) const
{
    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                  "MessageIDGenerator::getNextMessageID called");
    OsStatus result = OS_SUCCESS;

    if ( OsFileSystem::exists( m_dataFileName ) )
    {
        OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                      "MessageIDGenerator::getNextMessageID: m_dataFileName = '%s' exists",
                      m_dataFileName.data());
        // the file exists however it still may be corrupt
        OsFile messageIDFile ( m_dataFileName );
        result = messageIDFile.open( OsFile::READ_WRITE | OsFile::FSLOCK_WRITE );
        if (result == OS_SUCCESS)
        {
            // use the file system locking
            result = messageIDFile.readLine ( rMessageName );

            // Single line ending in EOF with numeric content however
            // also handle the case where there is an extra return thereby
            // not returning the EOF character
            if ( ((result == OS_FILE_EOF) || (result == OS_SUCCESS)) &&
                 (rMessageName.length() > 0) )
            {
                OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                              "MessageIDGenerator::getNextMessageID successfully read rMessageName = '%s'",
                              rMessageName.data());
                int nextSequenceNum = atoi ( rMessageName.data() ) + 1;
                char buffer[32];
                // Write it, zero extended to eight digits.
                sprintf ( buffer, "%08d", nextSequenceNum );
                unsigned long bytesWritten;

                // reset the pointer to 0 to overwrite the sequence
                result = messageIDFile.setPosition(0);
                OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                              "MessageIDGenerator::getNextMessageID setPosition returns %d",
                              result);

                if (result == OS_SUCCESS)
                {
                    result = messageIDFile.write( buffer, strlen(buffer), bytesWritten );
                    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                                  "MessageIDGenerator::getNextMessageID returns %d",
                                  result);
                }
                // Should work
                messageIDFile.close();
            }
        }
    } else
    {
        OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                      "MessageIDGenerator::getNextMessageID: m_dataFileName = '%s' does not exist, calling recoverMessageID",
                      m_dataFileName.data());
        result = recoverMessageID ( rMessageName );
    }
    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                  "MessageIDGenerator::getNextMessageID returns %d, rMessageName = '%s'",
                  result, rMessageName.data());
    return result;
}


OsStatus
MessageIDGenerator::recoverMessageID ( UtlString& rMessageName ) const
{
    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                  "MessageIDGenerator::recoverMessageID called");
    OsStatus result = OS_SUCCESS;

    // Create a filename in the data directory
    OsFile messageIDFile ( m_dataFileName );

    result = messageIDFile.open( OsFile::CREATE );
    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                  "MessageIDGenerator::recoverMessageID: attempt to open file '%s' returns %d",
                  m_dataFileName.data(), result);

    if (result == OS_SUCCESS)
    {
        // Recurse directories (@JC TODO)
        // Choose 1 as the sequence number to return.
        rMessageName = "00000001";
        // Save 2 as the next sequence number.
        UtlString nextSequenceNum = "00000002";
        unsigned long bytesWritten;
        result = messageIDFile.write( nextSequenceNum , nextSequenceNum.length(), bytesWritten );
        OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                      "MessageIDGenerator::recoverMessageID: write to ID file returns %d",
                      result);
        messageIDFile.close();
    }
    else
    {
       OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_CRIT,
                     "MessageIDGenerator::recoverMessageID: Attempt to recover the message ID file '%s' failed",
                     m_dataFileName.data());
    }

    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                  "MessageIDGenerator::recoverMessageID returns %d, rMessageName = '%s'",
                  result, rMessageName.data());
    return result;
}
