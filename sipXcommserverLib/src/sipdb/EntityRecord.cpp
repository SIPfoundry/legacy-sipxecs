/*
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */


#include <algorithm>
#include "sipdb/EntityRecord.h"

//
// Field names
//
const char* EntityRecord::oid_fld(){ static std::string fld = "_id"; return fld.c_str(); }
const char* EntityRecord::userId_fld(){ static std::string fld = "uid"; return fld.c_str(); }
const char* EntityRecord::identity_fld(){ static std::string fld = "ident"; return fld.c_str(); }
const char* EntityRecord::realm_fld(){ static std::string fld = "rlm"; return fld.c_str(); }
const char* EntityRecord::password_fld(){ static std::string fld = "pstk"; return fld.c_str(); }
const char* EntityRecord::pin_fld(){ static std::string fld = "pntk"; return fld.c_str(); }
const char* EntityRecord::authType_fld(){ static std::string fld = "authtp"; return fld.c_str(); }
const char* EntityRecord::location_fld(){ static std::string fld = "loc"; return fld.c_str(); }
const char* EntityRecord::permission_fld(){ static std::string fld = "prm"; return fld.c_str(); }
const char* EntityRecord::callerAliases_fld(){ static std::string fld = "cals"; return fld.c_str(); }
const char* EntityRecord::callerAliasesDomain_fld(){ static std::string fld = "dm"; return fld.c_str(); }
const char* EntityRecord::callerAliasesAlias_fld(){ static std::string fld = "als"; return fld.c_str(); }
const char* EntityRecord::aliases_fld(){ static std::string fld = "als"; return fld.c_str(); }
const char* EntityRecord::aliasesId_fld(){ static std::string fld = "id"; return fld.c_str(); }
const char* EntityRecord::aliasesContact_fld(){ static std::string fld = "cnt"; return fld.c_str(); }
const char* EntityRecord::aliasesRelation_fld(){ static std::string fld = "rln"; return fld.c_str(); }
const char* EntityRecord::callForwardTime_fld(){ static std::string fld = "cfwdtm"; return fld.c_str(); }       
const char* EntityRecord::staticUserLoc_fld(){ static std::string fld = "stc"; return fld.c_str(); }
const char* EntityRecord::staticUserLocEvent_fld(){ static std::string fld = "evt"; return fld.c_str(); }
const char* EntityRecord::staticUserLocContact_fld(){ static std::string fld = "cnt"; return fld.c_str(); }
const char* EntityRecord::staticUserLocFromUri_fld(){ static std::string fld = "from"; return fld.c_str(); }
const char* EntityRecord::staticUserLocToUri_fld(){ static std::string fld = "to"; return fld.c_str(); }
const char* EntityRecord::staticUserLocCallId_fld(){ static std::string fld = "cid"; return fld.c_str(); }

EntityRecord::EntityRecord()
{
    _callForwardTime = 0;
}

EntityRecord::EntityRecord(const EntityRecord& entity)
{
    _oid = entity._oid;
    _userId = entity._userId;
    _identity = entity._identity;
    _realm = entity._realm;
    _password = entity._password;
    _pin = entity._pin;
    _authType = entity._authType;
    _permissions = entity._permissions;
    _callerAliases = entity._callerAliases;
    _aliases = entity._aliases;
    _callForwardTime = entity._callForwardTime;
    _staticUserLoc = entity._staticUserLoc;
}

EntityRecord::~EntityRecord()
{

}

EntityRecord& EntityRecord::operator=(const EntityRecord& entity)
{
    EntityRecord clonable(entity);
    swap(clonable);
    return *this;
}

void EntityRecord::swap(EntityRecord& entity)
{
    std::swap(_oid, entity._oid);
    std::swap(_userId, entity._userId);
    std::swap(_identity, entity._identity);
    std::swap(_realm, entity._realm);
    std::swap(_password, entity._password);
    std::swap(_pin, entity._pin);
    std::swap(_authType, entity._authType);
    std::swap(_permissions, entity._permissions);
    std::swap(_callerAliases, entity._callerAliases);
    std::swap(_aliases, entity._aliases);
    std::swap(_callForwardTime, entity._callForwardTime);
    std::swap(_staticUserLoc, entity._staticUserLoc);
}

EntityRecord& EntityRecord::operator =(const MongoDB::BSONObj& bsonObj)
{
    try
    {
        _oid = bsonObj.getStringField(EntityRecord::oid_fld());

        if (bsonObj.hasField(EntityRecord::userId_fld()))
        {
            _userId = bsonObj.getStringField(EntityRecord::userId_fld());
        }

        if (bsonObj.hasField(EntityRecord::identity_fld()))
        {
            _identity = bsonObj.getStringField(EntityRecord::identity_fld());
        }

        if (bsonObj.hasField(EntityRecord::realm_fld()))
        {
            _realm = bsonObj.getStringField(EntityRecord::realm_fld());
        }

        if (bsonObj.hasField(EntityRecord::password_fld()))
        {
            _password = bsonObj.getStringField(EntityRecord::password_fld());
        }

        if (bsonObj.hasField(EntityRecord::pin_fld()))
        {
            _pin = bsonObj.getStringField(EntityRecord::pin_fld());
        }

        if (bsonObj.hasField(EntityRecord::authType_fld()))
        {
            _authType = bsonObj.getStringField(EntityRecord::authType_fld());
        }

        if (bsonObj.hasField(EntityRecord::location_fld()))
        {
            _location = bsonObj.getStringField(EntityRecord::location_fld());
        }

        if (bsonObj.hasField(EntityRecord::callForwardTime_fld()))
        {
            _callForwardTime = bsonObj.getIntField(EntityRecord::callForwardTime_fld());
        }

        if (bsonObj.hasField(EntityRecord::permission_fld()))
        {
            MongoDB::BSONElement obj = bsonObj[EntityRecord::permission_fld()];
            if ( obj.isABSONObj() &&  obj.type() == mongo::Array)
            {
                std::vector<MongoDB::BSONElement> permissions = obj.Array();
                _permissions.clear();
                for (std::vector<MongoDB::BSONElement>::iterator iter = permissions.begin();
                    iter != permissions.end(); iter++)
                {
                    _permissions.insert(iter->String());
                }
            }
        }

        if (bsonObj.hasField(EntityRecord::callerAliases_fld()))
        {
            MongoDB::BSONElement obj = bsonObj[EntityRecord::callerAliases_fld()];
            if ( obj.isABSONObj() &&  obj.type() == mongo::Array)
            {
                std::vector<MongoDB::BSONElement> callerAliases = obj.Array();
                for (std::vector<MongoDB::BSONElement>::iterator iter = callerAliases.begin();
                    iter != callerAliases.end(); iter++)
                {
                    MongoDB::BSONObj innerObj = iter->Obj();
                    CallerAlias callerAlias;
                    if (innerObj.hasField(EntityRecord::callerAliasesAlias_fld()))
                        callerAlias.alias = innerObj.getStringField(EntityRecord::callerAliasesAlias_fld());
                    if (innerObj.hasField(EntityRecord::callerAliasesDomain_fld()))
                        callerAlias.targetDomain = innerObj.getStringField(EntityRecord::callerAliasesDomain_fld());

                    _callerAliases.push_back(callerAlias);
                }
            }
        }

        if (bsonObj.hasField(EntityRecord::aliases_fld()))
        {
            MongoDB::BSONElement obj = bsonObj[EntityRecord::aliases_fld()];
            if ( obj.isABSONObj() &&  obj.type() == mongo::Array)
            {
                std::vector<MongoDB::BSONElement> aliases = obj.Array();
                for (std::vector<MongoDB::BSONElement>::iterator iter = aliases.begin();
                    iter != aliases.end(); iter++)
                {
                    MongoDB::BSONObj innerObj = iter->Obj();
                    Alias alias;
                    if (innerObj.hasField(EntityRecord::aliasesId_fld()))
                        alias.id = innerObj.getStringField(EntityRecord::aliasesId_fld());
                    if (innerObj.hasField(EntityRecord::aliasesContact_fld()))
                        alias.contact = innerObj.getStringField(EntityRecord::aliasesContact_fld());
                    if (innerObj.hasField(EntityRecord::aliasesRelation_fld()))
                        alias.relation = innerObj.getStringField(EntityRecord::aliasesRelation_fld());
                    _aliases.push_back(alias);
                }
            }
        }

        if (bsonObj.hasField(EntityRecord::staticUserLoc_fld()))
        {
            MongoDB::BSONElement obj = bsonObj[EntityRecord::staticUserLoc_fld()];
            if ( obj.isABSONObj() &&  obj.type() == mongo::Array)
            {
                std::vector<MongoDB::BSONElement> userLocs = obj.Array();
                for (std::vector<MongoDB::BSONElement>::iterator iter = userLocs.begin();
                    iter != userLocs.end(); iter++)
                {
                    MongoDB::BSONObj innerObj = iter->Obj();
                    StaticUserLoc userLoc;
                    if (innerObj.hasField(EntityRecord::staticUserLocEvent_fld()))
                        userLoc.event = innerObj.getStringField(EntityRecord::staticUserLocEvent_fld());
                    if (innerObj.hasField(EntityRecord::staticUserLocContact_fld()))
                        userLoc.contact = innerObj.getStringField(EntityRecord::staticUserLocContact_fld());
                    if (innerObj.hasField(EntityRecord::staticUserLocFromUri_fld()))
                        userLoc.fromUri = innerObj.getStringField(EntityRecord::staticUserLocFromUri_fld());
                    if (innerObj.hasField(EntityRecord::staticUserLocToUri_fld()))
                        userLoc.toUri = innerObj.getStringField(EntityRecord::staticUserLocToUri_fld());
                    if (innerObj.hasField(EntityRecord::staticUserLocCallId_fld()))
                        userLoc.callId = innerObj.getStringField(EntityRecord::staticUserLocCallId_fld());
                    _staticUserLoc.push_back(userLoc);
                }
            }
        }
    }
    catch(std::exception& e)
    {
        SYSLOG_ERROR("MongoDB Exception: (EntityRecord::operator =(const MongoDB::BSONObj& bsonObj))" << e.what());
    }

    return *this;
}
