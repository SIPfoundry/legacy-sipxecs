//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef CREDENTIALDB_H
#define CREDENTIALDB_H

// SYSTEM INCLUDES


// APPLICATION INCLUDES
#include "os/OsMutex.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class Url;
class dbDatabase;
class dbFieldDescriptor;
class UtlHashMap;
class TiXmlNode;
class ResultSet;

class CredentialDB
{
  public:
    /// Singleton Accessor
    static CredentialDB* getInstance( const UtlString& name = "credential" );
    /**<
     * @returns the instance pointer to the database singleton
     * Loads the database from the persistent store on the first access.
     */

    /// releaseInstance - cleans up the singleton (for use at exit)
    static void releaseInstance();
    /**<
     * Ensures that the current database is flushed to the persistent store.
     */

    /// Flush the current database to the persistent store.
    OsStatus store();

    /// Load a database row
    UtlBoolean insertRow (const Url& uri,
                          const UtlString& realm,
                          const UtlString& userid,
                          const UtlString& passToken,
                          const UtlString& pinToken,
                          const UtlString& authType = "DIGEST"
                          );

    UtlBoolean insertRow ( const UtlHashMap& nvPairs );

    void removeRows (const Url& uri,
                     const UtlString& realm
                     );

    /// Remove all rows matching the specified identity
    void removeRows (const Url& uri);

    /// Delete all rows in the current database.
    void removeAllRows ();

    /// Utility method for retrieving all rows
    void getAllRows ( ResultSet& rResultset ) const;
    /**<
     * This should <strong>not</strong> be used for any normal operational purpose.
     */

    /// Count rows in table
    int getRowCount () const;

    /// Retrieve the SIP credential check values for a given identity, realm, and userid
    UtlBoolean getCredentialByUserid (
       const Url& uri,
       const UtlString& realm,
       const UtlString& userid,
       UtlString& passtoken,
       UtlString& authType
                                      ) const;
    /// Retrieve the SIP credential check values for a given identity and realm
    UtlBoolean getCredential (
       const Url& uri,
       const UtlString& realm,
       UtlString& userid,
       UtlString& passtoken,
       UtlString& authType
                              ) const;

    /// Retrieve the SIP credential check values for a given userid and realm
    UtlBoolean getCredential (
       const UtlString& userid,
       const UtlString& realm,
       Url& uri,
       UtlString& passtoken,
       UtlString& authType
                              ) const;

    /// Retrieve the User PIN check values for a given identity and realm
    UtlBoolean getUserPin (
       const Url& uri,
       const UtlString& realm,
       UtlString& userid,
       UtlString& pintoken,
       UtlString& authType
                           ) const;

    /// Retrieve the User PIN check values for a given userid and realm
    UtlBoolean getUserPin (
       const UtlString& userid,
       const UtlString& realm,
       Url& uri,
       UtlString& pintoken,
       UtlString& authType
                           ) const;

    /// Utility method for retrieving all information for a given identity
    void getAllCredentials (
       const Url& uri,
       ResultSet& cursor
                            ) const;

    /// Determine whether or not a given identity is in the credentials database
    UtlBoolean isUriDefined (
       const Url& uri,
       UtlString& realm,
       UtlString& authType
                             ) const;

    // The 'type' attribute of the top-level 'items' element.
    static const UtlString sType;

    // The XML namespace of the top-level 'items' element.
    static const UtlString sXmlNamespace;

    // Determine if the table is loaded from xml file
    bool isLoaded();

  protected:

  private:

    /// Reads the persistent store into the database.
    OsStatus load();
    /**<
     * This is executed implicitly when the singleton is instantiated.
     */

    // Singleton Constructor is private
    CredentialDB( const UtlString& name = "credentials" );

    static CredentialDB* spInstance;

    /// Singleton and Serialization mutex
    static OsMutex sLockMutex;

    // ResultSet column Keys
    static UtlString gUriKey;
    static UtlString gRealmKey;
    static UtlString gUseridKey;
    static UtlString gPasstokenKey;
    static UtlString gPintokenKey;
    static UtlString gAuthtypeKey;

    /// Fast DB instance
    dbDatabase* m_pFastDB;

    /// the persistent filename for loading/saving
    UtlString mDatabaseName;

    // boolean indicating table is loaded
    bool mTableLoaded;

    virtual ~CredentialDB();
};

#endif //CREDENTIALSDB_H
