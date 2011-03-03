
#ifndef ENTITYDB_H
#define	ENTITYDB_H

#include "utl/UtlString.h"
#include "net/Url.h"
#include "sipdb/EntityRecord.h"
#include <set>

class EntityDB : public MongoDB::DBInterface
{
public:
    typedef std::vector<EntityRecord> Entities;
    typedef std::map<std::string, EntityRecord> EntitiesByIdentity;
    typedef std::vector<EntityRecord::Alias> Aliases;
    typedef std::set<std::string> Permissions;
    
    EntityDB(
        MongoDB& db,
        const std::string& ns = EntityDB::_defaultNamespace);

    ~EntityDB();

    static std::string& defaultNamespace();

    bool findByIdentity(const std::string& identity, EntityRecord& entity) const;
    bool findByIdentity(const Url& uri, EntityRecord& entity) const;
    bool findByUserId(const std::string& userId, EntityRecord& entity) const;
      /// Retrieve the SIP credential check values for a given identity and realm
    bool getCredential (
       const Url& uri,
       const UtlString& realm,
       UtlString& userid,
       UtlString& passtoken,
       UtlString& authType) const;

    /// Retrieve the SIP credential check values for a given userid and realm
    bool getCredential (
       const UtlString& userid,
       const UtlString& realm,
       Url& uri,
       UtlString& passtoken,
       UtlString& authType) const;

    // Query interface to return a set of mapped full URI
    // contacts associated with the alias
    void getAliasContacts (
        const Url& aliasIdentity,
        Aliases& aliases,
        bool& isUserIdentity) const;

    static MongoDB::Collection<EntityDB>& defaultCollection();
    static std::string _defaultNamespace;


};

//
// Inline
//

inline bool EntityDB::findByIdentity(const Url& uri, EntityRecord& entity) const
{
    UtlString identity;
    uri.getIdentity(identity);
    return findByIdentity(identity.str(), entity);
}
#endif	/* ENTITYDB_H */

