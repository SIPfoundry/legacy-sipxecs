#include "sipdb/RegDB.h"


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
        "expirationTime" << pBinding->getExpirationTime() <<
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
            "expirationTime" << BSON_GREATER_THAN_EQUAL(expirationTime));

    MongoDB::BSONObj update = BSON("$set" << BSON(
        "expirationTime" << expirationTime <<
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
            "expirationTime" << BSON_GREATER_THAN_EQUAL(expirationTime));

    MongoDB::BSONObj update = BSON("$set" << BSON(
        "expirationTime" << expirationTime <<
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
            "callId" << callId);

    std::string error;
    MongoDB::Cursor pCursor = _db.find(_ns, query, error);
    if (pCursor.get() && pCursor->more())
    {
      while(pCursor->more())
      {
        RegBinding binding = pCursor->next();
        if (binding.getCallId() == callId && binding.getCseq() >= cseq)
          return true;
      }
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
            "expirationTime" << BSON_GREATER_THAN(timeNow));
    }
    else
    {
        query = BSON(
            "identity" << identity <<
            "expirationTime" << BSON_GREATER_THAN(timeNow));
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

bool RegDB::getUnexpiredContactsUserInstrument(
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
        "expirationTime" << BSON_GREATER_THAN(timeNow));

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
        const std::string& instrument,
        int timeNow,
        Bindings& bindings) const
{
     MongoDB::BSONObj query = BSON(
        "instrument" << instrument <<
        "expirationTime" << BSON_GREATER_THAN(timeNow));

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

bool RegDB::getAllOldBindings(int timeNow, Bindings& bindings)
{
    MongoDB::BSONObj query = BSON(
        "expirationTime" << BSON_LESS_THAN(timeNow));

    std::string error;
    MongoDB::Cursor pCursor = _db.find(_ns, query, error);
    if (pCursor.get() && pCursor->more())
    {
        while (pCursor->more())
        {
            RegBinding binding(pCursor->next());
            if (binding.getCallId() != "#")
                bindings.push_back(binding);
        }
        return true;
    }
    return false;
}


bool RegDB::clean(int currentExpireTime)
{
    MongoDB::BSONObj query = BSON(
        "expirationTime" << BSON_LESS_THAN(currentExpireTime));
    std::string error;
    return _db.remove(_ns, query, error);
}