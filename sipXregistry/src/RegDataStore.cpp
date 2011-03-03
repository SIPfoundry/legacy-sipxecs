#include "registry/RegDataStore.h"
#include "registry/RegisterPlugin.h"
#include "registry/RedirectPlugin.h"

std::string RegDataStore::_bindingsNameSpace = "imdb.registrar";
std::string RegDataStore::_entitiesNameSpace = "imdb.entity";

RegDataStore::RegDataStore()
{
    _pRegDB = RegDBPtr(new RegDBCollection(RegDataStore::_bindingsNameSpace));
    _pEntityDB = EntityDBPtr(new EntityDBCollection(RegDataStore::_entitiesNameSpace));
}

void RegDataStore::setBindingsNameSpace(const std::string& ns)
{
    RegDataStore::_bindingsNameSpace = ns;
}

void RegDataStore::setEntitiesNameSpace(const std::string& ns)
{
    RegDataStore::_entitiesNameSpace = ns;
}


bool RegDataStore::getAllEntityUri(ContactList& contactList, const RedirectPlugin& plugin) const
{
    MongoDB::Cursor pCursor = _pEntityDB->collection().items();

    while (pCursor->more())
    {
        EntityRecord entity;
        entity = pCursor->next();
        UtlString uri = entity.identity().c_str();
        Url contactUri(uri);

        // Add the contact to the contact list.
        contactList.add( contactUri, plugin );

    }
    return true;
}