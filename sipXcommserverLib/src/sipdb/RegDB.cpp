#include "sipdb/RegDB.h"

#ifndef GRUU_PREFIX
#define GRUU_PREFIX "~~gr~"
#endif

#ifndef SIP_GRUU_URI_PARAM
#define SIP_GRUU_URI_PARAM "gr"
#endif

RegDB::RegDB(MongoDB& db, const std::string& ns) :
    MongoDB::DBInterface(db, ns)
{
}

RegDB::~RegDB()
{
}

void RegDB::updateBinding(const RegBinding::Ptr& pBinding)
{
    mutex_lock lock(_mutex);

    MongoDB::BSONObj query = BSON(
        "identity" << pBinding->getIdentity() <<
        "contact" << pBinding->getContact());

    MongoDB::BSONObj update = BSON("$set" << BSON(
        "identity" << pBinding->getIdentity() <<
        "uri" << pBinding->getUri() <<
        "callId" << pBinding->getCallId() <<
        "contact" << pBinding->getContact() <<
        "qvalue" << pBinding->getQvalue() <<
        "instanceId" << pBinding->getInstanceId() <<
        "gruu" << pBinding->getGruu() <<
        "path" << pBinding->getPath() <<
        "cseq" << pBinding->getCseq() <<
        "expires" << pBinding->getExpires() <<
        "instrument" << pBinding->getInstrument()));

    std::string error;
    if (!_db.updateOrInsert(_ns, query, update, error))
    {
        //
        // Log error string here
        //
        std::cerr << error;
    }
}


void RegDB::expireOldBindings(
    const std::string& identity,
    const std::string& callId,
    unsigned int cseq,
    unsigned int timeNow)
{

    unsigned int expirationTime = timeNow-1;
    MongoDB::BSONObj query = BSON(
            "identity" << identity <<
            "callId"<< callId <<
            "cseq" << BSON_LESS_THAN(cseq) <<
            "expires" << BSON_GREATER_THAN_EQUAL(expirationTime));

    MongoDB::BSONObj update = BSON("$set" << BSON(
        "expires" << expirationTime <<
        "cseq" << cseq));

    std::string error;
    if (!_db.update(_ns, query, update, error))
    {
        //
        // Log error string here
        //
        std::cerr << error;
    }
}

void RegDB::expireAllBindings(
    const std::string& identity,
    const std::string& callId,
    unsigned int cseq,
    unsigned int timeNow)
{

    unsigned int expirationTime = timeNow-1;
    MongoDB::BSONObj query = BSON(
            "identity" << identity <<
            "expires" << BSON_GREATER_THAN_EQUAL(expirationTime));

    MongoDB::BSONObj update = BSON("$set" << BSON(
        "expires" << expirationTime <<
        "callId" << callId <<
        "cseq" << cseq));

    std::string error;
    if (!_db.update(_ns, query, update, error))
    {
        //
        // Log error string here
        //
        std::cerr << error;
    }
}


bool RegDB::isOutOfSequence(
    const std::string& identity,
    const std::string& callId,
    unsigned int cseq) const
{
    MongoDB::BSONObj query = BSON(
            "identity" << identity <<
            "callId" << callId <<
            "cseq" << BSON_GREATER_THAN_EQUAL(cseq));

    std::string error;
    MongoDB::Cursor pCursor = _db.find(_ns, query, error);
    if (pCursor.get() && pCursor->more())
    {
        return true;
    }
    else
    {
        //
        // Log error here
        //
        std::cerr << error;
        return true; /// return false here to indicate the error as out of sequence
    }

    return false;
}

bool RegDB::getUnexpiredContactsUser (
    const std::string& identity,
    int timeNow,
    Bindings& bindings) const
{
    static std::string gruuPrefix = GRUU_PREFIX;

    bool isGruu = identity.substr(0, gruuPrefix.size()) == gruuPrefix;
    MongoDB::BSONObj query;

    if (isGruu)
    {
        std::string searchString(identity);
        searchString += SIP_GRUU_URI_PARAM;
        query = BSON(
            "gruu" << searchString <<
            "expires" << BSON_GREATER_THAN(timeNow));
    }
    else
    {
        query = BSON(
            "identity" << identity <<
            "expires" << BSON_GREATER_THAN(timeNow));
    }

    std::string error;
    MongoDB::Cursor pCursor = _db.find(_ns, query, error);
    if (pCursor.get() && pCursor->more())
    {
        while (pCursor->more())
            bindings.push_back(RegBinding(pCursor->next()));
        return true;
    }

    return false;
}

bool RegDB::getUnexpiredContactsInstrument(
        const std::string& identity,
        const std::string& instrument,
        int timeNow,
        Bindings& bindings) const
{
    //
    //query="np_identity=",identity," and instrument=",instrument," and expires>",timeNow;

    MongoDB::BSONObj query = BSON(
        "identity" << identity <<
        "instrument" << instrument <<
        "expires" << BSON_GREATER_THAN(timeNow));

    std::string error;
    MongoDB::Cursor pCursor = _db.find(_ns, query, error);
    if (pCursor.get() && pCursor->more())
    {
        while (pCursor->more())
            bindings.push_back(RegBinding(pCursor->next()));
        return true;
    }

    return false;
}
