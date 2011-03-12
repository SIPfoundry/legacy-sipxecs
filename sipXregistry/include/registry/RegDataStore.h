
#ifndef REGDATASTORE_H
#define	REGDATASTORE_H


#include "sipdb/RegDB.h"
#include "sipdb/EntityDB.h"
#include "utl/UtlString.h"
#include "net/Url.h"

class ContactList;
class RedirectPlugin;

class RegDataStore
{
public:
    typedef MongoDB::Collection<RegDB> RegDBCollection;
    typedef MongoDB::Collection<EntityDB> EntityDBCollection;
    typedef boost::shared_ptr<RegDBCollection> RegDBPtr;
    typedef boost::shared_ptr<EntityDBCollection> EntityDBPtr;


    RegDataStore();
    static void setBindingsNameSpace(const std::string& ns);
    static void setEntitiesNameSpace(const std::string& ns);

    RegDB& regDB();
    EntityDB& entityDB();


    bool getAllEntityUri(ContactList& contactList, const RedirectPlugin& plugin) const;

     // Query interface to return a set of mapped full URI
    // contacts associated with the alias
    //void getContacts (
    //    const Url& aliasIdentity,
    //    std::vector<>) const;

    // Query interface to return aliases associated with a sipIdentity
    //void getAliases (
    //    const Url& contactIdentity,
    //    ResultSet& rResultSet ) const;
private:
    static std::string _bindingsNameSpace;
    static std::string _entitiesNameSpace;
    RegDBPtr _pRegDB;
    EntityDBPtr _pEntityDB;
};

//
// inlines
//

inline RegDB& RegDataStore::regDB()
{
    return _pRegDB->collection();
}
    
inline EntityDB& RegDataStore::entityDB()
{
    return _pEntityDB->collection();
}

#endif	/* REGDATASTORE_H */

