// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef DISPLAYTASK_H
#define DISPLAYTASK_H

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "IMDBWorkerTask.h"
#include "sipdb/ResultSet.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsEvent;
class TiXmlNode;

class DisplayTask : public IMDBWorkerTask
{
public:
    /**
     * Ctor
     * 
     * @param rCommand
     * @param rArgument
     * @param rMsgQ
     */
    DisplayTask ( 
        const UtlString& rArgument, 
        OsMsgQ& rMsgQ,
        OsEvent& rCommandEvent);

    /**
     * Dtor
     */
    virtual ~DisplayTask();
    
    /**
     * The worker method, this is where all the command types execute, they
     * run as separate threads
     * 
     * @param runArg
     * 
     * @return 
     */
    virtual int run( void* runArg );

    /**
     * 
     * @param rTableName
     * 
     * @return 
     */
    int showTableRows ( const UtlString& rTableName ) const;

    /**
     * Populates the IMDB from an XML file
     * 
     * @param rImportFilename
     * 
     * @return 
     */
    OsStatus loadDB( const UtlString& rImportFilename ) const;

    /**
     * 
     * @param rNode
     * @param rKey
     * @param rValue
     * 
     * @return 
     */
    OsStatus getAttributeValue ( 
        const TiXmlNode& rNode, 
        const UtlString& rKey, 
        UtlString& rValue ) const;

    /**
     * 
     * @param nvPairs
     * @param tableName
     */
    void insertRow ( 
        const UtlHashMap& nvPairs, 
        const UtlString& tableName ) const;

private:
    void showProcessInfo(ResultSet& resultSet) const;
    void showCredentials(ResultSet& resultSet) const;
    void showHuntGroups(ResultSet& resultSet) const;
    void showAuthExceptions(ResultSet& resultSet) const;
    void showRegistrations(ResultSet& resultSet) const;
    void showAliases(ResultSet& resultSet) const;
    void showExtensions(ResultSet& resultSet) const;
    void showPermissions(ResultSet& resultSet) const;
    void showDialByName(ResultSet& resultSet) const;
    void showSubscriptions(ResultSet& resultSet) const;
};

#endif  // DISPLAYTASK_H

