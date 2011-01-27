
#ifndef ENTITYRECORD_H
#define	ENTITYRECORD_H

#include <string>
#include <vector>
#include <set>
#include <boost/shared_ptr.hpp>
#include "sipdb/MongoDB.h"


class EntityRecord
{
public:

    //
    // The Caller Alias Structure
    //
    struct CallerAlias
    {
        std::string targetDomain;
        std::string alias;
    };

    //
    // Alias Structure
    //
    struct Alias
    {
        std::string id;
        std::string contact;
    };

    EntityRecord();

    EntityRecord(const EntityRecord& entity);

    ~EntityRecord();

    EntityRecord& operator=(const EntityRecord& entity);

    void swap(EntityRecord& entity);

    EntityRecord& operator =(const MongoDB::BSONObj& bsonObj);

    //
    // The unique record object-id
    //
    std::string& oid();
    static const char* oid_fld();

    //
    // The sip user-id
    //
    std::string& userId();
    static const char* userId_fld();

    //
    // Identity of the a user designated by a uri with an optional port
    //
    std::string& identity();
    static const char* identity_fld();

    //
    // Realm used for authenticating users
    //
    std::string& realm();
    static const char* realm_fld();

    //
    // The SIP password
    //
    std::string& password();
    static const char* password_fld();

    //
    // The PIN for the user interface access
    //
    std::string& pin();
    static const char* pin_fld();

    //
    // The type of authentication used for this user
    //
    std::string& authType();
    static const char* authType_fld();

    //
    // User Location
    //
    std::string& location();
    static const char* location_fld();

    //
    // Permission array to which the user has access to
    //
    std::set<std::string>& permissions();
    static const char* permission_fld();

    //
    // Caller alias to be sent to certain target domains
    //
    std::vector<CallerAlias>& callerAliases();
    static const char* callerAliases_fld();
    static const char* callerAliasesDomain_fld();
    static const char* callerAliasesAlias_fld();

    //
    // Aliases that points back toa real user
    //
    std::vector<Alias>& aliases();
    static const char* aliases_fld();
    static const char* aliasesId_fld();
    static const char* aliasesContact_fld();
private:
    std::string _oid;
    std::string _userId;
    std::string _identity;
    std::string _realm;
    std::string _password;
    std::string _pin;
    std::string _authType;
    std::string _location;
    std::set<std::string> _permissions;
    std::vector<CallerAlias> _callerAliases;
    std::vector<Alias> _aliases;
};

//
// Inlines
//

inline std::string& EntityRecord::oid()
{
    return _oid;
}

inline std::string& EntityRecord::userId()
{
    return _userId;
}

inline std::string& EntityRecord::identity()
{
    return _identity;
}

inline std::string& EntityRecord::realm()
{
    return _realm;
}

inline std::string& EntityRecord::password()
{
    return _password;
}

inline std::string& EntityRecord::pin()
{
    return _pin;
}

inline std::string& EntityRecord::authType()
{
    return _authType;
}

inline std::string& EntityRecord::location()
{
    return _location;
}

inline std::set<std::string>& EntityRecord::permissions()
{
    return _permissions;
}

inline std::vector<EntityRecord::CallerAlias>& EntityRecord::callerAliases()
{
    return _callerAliases;
}

inline std::vector<EntityRecord::Alias>& EntityRecord::aliases()
{
    return _aliases;
}

#endif	/* ENTITYRECORD_H */

