#include "sipdb/DbHelper.h"


#include "sipdb/EntityRecord.h"
#include "sipdb/EntityDB.h"

#include "os/OsDateTime.h"

#include <string>

#include <mongo/util/net/hostandport.h>
#include <mongo/client/connpool.h>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

const char* pTimeNowMacro                                = "$now";


void DbHelper::printSetElements(std::ostream& strm,
                                const std::string& setName,
                                const std::set<std::string>& set,
                                bool multipleLines)
{
   int i = 0;

   if (multipleLines)
   {
      strm << boost::format("%-20s\n") % setName;

      for (std::set<std::string>::const_iterator iter = set.begin();
           iter != set.end(); iter++)
      {
         strm << boost::format(" %-3d %-20s\n") % i % *iter;
         i++;
      }
   }
   else
   {
      strm << boost::format("\"%s\":[") % setName;

      for (std::set<std::string>::const_iterator iter = set.begin();
           iter != set.end(); iter++)
      {
         strm << boost::format("%d %s ") % i % *iter;
         i++;
      }

      strm << boost::format("]   ") ;
   }
}

boost::posix_time::ptime DbHelper::convertSecondsToLocalTime(unsigned int seconds)
{
   typedef boost::date_time::c_local_adjustor<boost::posix_time::ptime> localAdj;

   boost::posix_time::ptime utcTime = boost::posix_time::ptime(boost::gregorian::date(1970,boost::gregorian::Jan,1),
                                       boost::posix_time::seconds(seconds));

   boost::posix_time::ptime localTime = localAdj::utc_to_local(utcTime);

   return localTime;
}

void DbHelper::printRegBindingEntry(std::ostream& strm,
                                    const mongo::BSONObj& bson,
                                    int currentNr,
                                    bool multipleLines)
{
   RegBinding binding(bson);

   printCell(strm, "Nr", currentNr, currentNr, multipleLines);

   printCell(strm, binding.callId_fld(), binding.getCallId(), currentNr, multipleLines);                    // "callId"
   printCell(strm, binding.contact_fld(), binding.getContact(), currentNr, multipleLines);                  // "contact"
   printCell(strm, binding.cseq_fld(), binding.getCseq(), currentNr, multipleLines);                        // "cseq"
   printCell(strm, binding.expirationTime_fld(), binding.getExpirationTime(), currentNr, multipleLines);                                        // "expirationTime"
   printCell(strm, binding.expirationTime_fld(),
             DbHelper::convertSecondsToLocalTime(binding.getExpirationTime()), currentNr, multipleLines);   // "expirationTime"
   printCell(strm, binding.expired_fld(), binding.getExpired(), currentNr, multipleLines);                  // "expired"
   printCell(strm, binding.gruu_fld(), binding.getGruu(), currentNr, multipleLines);                        // "gruu"
   printCell(strm, binding.identity_fld(), binding.getIdentity(), currentNr, multipleLines);                // "identity"
   printCell(strm, binding.instanceId_fld(), binding.getInstanceId(), currentNr, multipleLines);            // "instanceId"
   printCell(strm, binding.instrument_fld(), binding.getInstrument(), currentNr, multipleLines);            // "instrument"
   printCell(strm, binding.localAddress_fld(), binding.getLocalAddress(), currentNr, multipleLines);        // "localAddress"
   printCell(strm, binding.path_fld(), binding.getPath(), currentNr, multipleLines);                        // "path"
   printCell(strm, binding.qvalue_fld(), binding.getQvalue(), currentNr, multipleLines);                    // "qvalue"
   printCell(strm, binding.timestamp_fld(), binding.getTimestamp(), currentNr, multipleLines);              // "timeStamp"
   printCell(strm, binding.timestamp_fld(),
             DbHelper::convertSecondsToLocalTime(binding.getTimestamp()), currentNr, multipleLines);        // "timeStamp"
   printCell(strm, binding.uri_fld(), binding.getUri(), currentNr, multipleLines);                          // "uri"

   strm << "\n";
}

void DbHelper::printEntityRecordEntry(std::ostream& strm,
                                      const mongo::BSONObj& bson,
                                      int currentNr,
                                      bool multipleLines)
{
   EntityRecord entityRecord;
   EntityRecord& entityRecordRef = entityRecord;

   entityRecordRef = bson;


   printCell(strm, "Nr", currentNr, currentNr, multipleLines);

   printCell(strm, entityRecord.identity_fld(), entityRecord.identity(), currentNr, multipleLines);                  // "ident"
   printCell(strm, entityRecord.userId_fld(), entityRecord.userId(), currentNr, multipleLines);                      // "uid"
   printCell(strm, entityRecord.realm_fld(), entityRecord.realm(), currentNr, multipleLines);                        // "rlm"
   printCell(strm, entityRecord.password_fld(), entityRecord.password(), currentNr, multipleLines);                  // "pstk"
   printCell(strm, entityRecord.pin_fld(), entityRecord.pin(), currentNr, multipleLines);                            // "pntk"
   printCell(strm, entityRecord.authType_fld(), entityRecord.authType(), currentNr, multipleLines);                  // "authtp"
   printCell(strm, entityRecord.callForwardTime_fld(), entityRecord.callForwardTime(), currentNr, multipleLines);    // "cfwdtm"
   printCell(strm, entityRecord.location_fld(), entityRecord.location(), currentNr, multipleLines);                  // "loc"

   printCell(strm, entityRecord.callerId_fld(), entityRecord.callerId().id, currentNr, multipleLines);               // "clrid"
   printCell(strm, entityRecord.callerIdEnforcePrivacy_fld(),
             entityRecord.callerId().enforcePrivacy, currentNr, multipleLines);                                      // "blkcid"
   printCell(strm, entityRecord.callerIdIgnoreUserCalleId_fld(),
             entityRecord.callerId().ignoreUserCalleId, currentNr, multipleLines);                                   // "ignorecid"
   printCell(strm, entityRecord.callerIdTransformExtension_fld(),
             entityRecord.callerId().transformExtension, currentNr, multipleLines);                                  // "trnsfrmext"
   printCell(strm, entityRecord.callerIdExtensionLength_fld(),
             entityRecord.callerId().extensionLength, currentNr, multipleLines);                                     // "kpdgts"
   printCell(strm, entityRecord.callerIdExtensionPrefix_fld(),
             entityRecord.callerId().extensionPrefix, currentNr, multipleLines);                                     // "pfix"

   DbHelper::printSetElements(strm, entityRecord.permission_fld(), entityRecord.permissions(), multipleLines);       // "prm"

   printEntityRecordAliases(strm, entityRecord.aliases_fld(), entityRecord.aliases(), multipleLines);                // "als"
   printEntityRecordStaticUserLocations(strm, entityRecord.staticUserLoc_fld(),
                                        entityRecord.staticUserLoc(), multipleLines);                                // "stc"


   strm << "\n";
}

void DbHelper::printEntityRecordAliases(std::ostream& strm,
                                        const std::string& aliasesName,
                                        const std::vector<EntityRecord::Alias>& aliases,
                                        bool multipleLines)
{
   int i = 0;

   if (multipleLines)
   {
      strm << boost::format("%-20s\n") % aliasesName;

      for (std::vector<EntityRecord::Alias>::const_iterator iter = aliases.begin();
           iter != aliases.end(); iter++)
      {
         strm << boost::format(" %d \"%s\":  %s\n") % i % EntityRecord::aliasesId_fld() % iter->id;               // "id"
         strm << boost::format(" %d \"%s\": %s\n") % i % EntityRecord::aliasesContact_fld() % iter->contact;     // "cnt"
         strm << boost::format(" %d \"%s\": %s\n") % i % EntityRecord::aliasesRelation_fld() % iter->relation;   // "rln"
         i++;
      }
   }
   else
   {
      strm << boost::format("\"%s\":[") % aliasesName;

      for (std::vector<EntityRecord::Alias>::const_iterator iter = aliases.begin();
           iter != aliases.end(); iter++)
      {
         strm << boost::format("%d \"%s\":%s ") % i % EntityRecord::aliasesId_fld() % iter->id;             // "id"
         strm << boost::format("%d \"%s\":%s ") % i % EntityRecord::aliasesContact_fld() % iter->contact;   // "cnt"
         strm << boost::format("%d \"%s\":%s ") % i % EntityRecord::aliasesRelation_fld() % iter->relation; // "rln"                                    // "rln"
         i++;
      }

      strm << boost::format("]   ") ;
   }
}

void DbHelper::printEntityRecordStaticUserLocations(std::ostream& strm,
                                                      const std::string& staticUserLocationsName,
                                                      const std::vector<EntityRecord::StaticUserLoc>& staticUserLocations,
                                                      bool multipleLines)
{
   int i = 0;

   if (multipleLines)
   {
      strm << boost::format("%-20s\n") % staticUserLocationsName;

      for (std::vector<EntityRecord::StaticUserLoc>::const_iterator iter = staticUserLocations.begin();
           iter != staticUserLocations.end(); iter++)
      {
         strm << boost::format(" %d \"%s\":  %s\n") % i % EntityRecord::staticUserLocEvent_fld() % iter->event;      // "evt"
         strm << boost::format(" %d \"%s\": %s\n") % i % EntityRecord::staticUserLocContact_fld() % iter->contact;   // "cnt"
         strm << boost::format(" %d \"%s\": %s\n") % i % EntityRecord::staticUserLocFromUri_fld() % iter->fromUri;   // "from"
         strm << boost::format(" %d \"%s\": %s\n") % i % EntityRecord::staticUserLocToUri_fld() % iter->toUri;       // "to"
         strm << boost::format(" %d \"%s\": %s\n") % i % EntityRecord::staticUserLocCallId_fld() % iter->callId;     // "cid"
         i++;
      }
   }
   else
   {
      strm << boost::format("\"%s\":[") % staticUserLocationsName;

      for (std::vector<EntityRecord::StaticUserLoc>::const_iterator iter = staticUserLocations.begin();
           iter != staticUserLocations.end(); iter++)
      {
         strm << boost::format("%d \"%s\":%s ") % i % EntityRecord::staticUserLocEvent_fld() % iter->event;       // "evt"
         strm << boost::format("%d \"%s\":%s ") % i % EntityRecord::staticUserLocContact_fld() % iter->contact;   // "cnt"
         strm << boost::format("%d \"%s\":%s ") % i % EntityRecord::staticUserLocFromUri_fld() % iter->fromUri;   // "from"
         strm << boost::format("%d \"%s\":%s ") % i % EntityRecord::staticUserLocToUri_fld() % iter->toUri;       // "to"
         strm << boost::format("%d \"%s\":%s ") % i % EntityRecord::staticUserLocCallId_fld() % iter->callId;     // "cid"
         i++;
      }

      strm << boost::format("]   ") ;
   }
}

bool DbHelper::getLogicalOperator(const std::string& logicalOperator,
                               mongo::Labeler::Label& label)
{
  if (logicalOperator == ">")
  {
     label = mongo::GT;
     return true;
  }
  else if (logicalOperator == "<")
  {
     label = mongo::LT;
     return true;
  }
  else if (logicalOperator == "<=")
  {
     label = mongo::LTE;
     return true;
  }
  else if (logicalOperator == ">=")
  {
     label = mongo::GTE;
     return true;
  }
  else if (logicalOperator == "!=")
  {
     label = mongo::NE;
     return true;
  }
  else if (logicalOperator == "=")
  {
     return false;
  }

  BOOST_THROW_EXCEPTION(DbHelperException() <<
        DbHelperTagInfo((boost::format("Unknown logical operator %s\n") % logicalOperator).str()));

  return false;
}

void DbHelper::extractOperands(const std::string& string,
                                 std::string& leftOperand,
                                 std::string& logicalOperator,
                                 std::string& rightOperand)
{
    do
    {
       std::size_t pos = 0;
       std::size_t len = 0;
       const char* pSep = "><=!";

       // ex string="cseq<44"
       // find the first char of known logical operators
       len = string.find_first_of(pSep);
       if (len != std::string::npos)
       {
          //extract first token (left operand)
          leftOperand = string.substr(pos, len);
          boost::algorithm::trim(leftOperand);
       }
       else
          break;

       // update position and length in string for the second token (logical operator)
       pos = len;
       len = 1;

       // check if logical operator is 2 chars size based type (">=", "<=", "!=")
       if (string[pos + 1] == '=')
          len ++;

       // extract the second token (logical operator)
       logicalOperator = string.substr(pos, len);
       boost::algorithm::trim(logicalOperator);

       // update position and length in string for the third token
       pos += len;
       len = string.length() - pos;

       // extract third token (right operand)
       rightOperand = string.substr(pos, len);
       boost::algorithm::trim(rightOperand);
    }
    while(false);
}

void DbHelper::createQuery(mongo::BSONObj& query,
                             MongoDB::ScopedDbConnectionPtr& pConn,
                             std::vector<std::string>& whereOptVector,
                             const std::string& databaseName)
{
   std::size_t size = whereOptVector.size();

   if (size > 0)
   {
      mongo::BSONObjBuilder queryObjBuilder;

      int i = 0;

      while (i < (int)whereOptVector.size())
      {
         // temporary BSONObjBuilder object
         mongo::BSONObjBuilder queryObjBuilderTmp;

         appendRequiredType(pConn, queryObjBuilderTmp, databaseName, whereOptVector[i]);
         queryObjBuilder.appendElements(queryObjBuilderTmp.obj());
         i ++;
      }

      query = queryObjBuilder.obj();
   }
   else
   {
      query =  mongo::BSONObj();
   }
}

void DbHelper::deleteDbEntries(const MongoDB::ConnectionInfo* pConnectionInfo,
                                 const std::string& databaseName,
                                 std::vector<std::string>& whereOptVector)
{
   MongoDB::ScopedDbConnectionPtr pConn(mongoMod::ScopedDbConnection::getScopedDbConnection(pConnectionInfo->getConnectionString().toString()));

   try
   {
      mongo::BSONObj query;
      createQuery(query, pConn, whereOptVector, databaseName);

      pConn->get()->remove(databaseName,  query);
   }
   catch(...)
   {
      // catch any exception in order to clean mongo::ScopedDbConnection and rethrow it
      pConn->done();
      throw;
   }

   pConn->done();
}

void DbHelper::printDbEntries(std::ostream& strm,
                                 const MongoDB::ConnectionInfo* pConnectionInfo,
                                 const std::string& databaseName,
                                 std::vector<std::string>& whereOptVector,
                                 const DbType dbType,
                                 bool multipleLines)
{
   MongoDB::ScopedDbConnectionPtr pConn(mongoMod::ScopedDbConnection::getScopedDbConnection(pConnectionInfo->getConnectionString().toString()));

   try
   {
      if (dbType == DbTypeRegBinding)
      {
         _pFnPrintEntry = &DbHelper::printRegBindingEntry;
      }
      else if (dbType == DbTypeEntityRecord)
      {
         _pFnPrintEntry = &DbHelper::printEntityRecordEntry;
      }
      else
      {
        BOOST_THROW_EXCEPTION(DbHelperException() <<
              DbHelperTagInfo(std::string("Unknown database type")));
      }

      mongo::BSONObj query;
      createQuery(query, pConn, whereOptVector, databaseName);

      int dbEntryNr = 0;
      std::auto_ptr<mongo::DBClientCursor> pCursor = pConn->get()->query(databaseName,  query);
      if (pCursor.get() && pCursor->more())
      {
         while (pCursor->more())
         {
            (this->*_pFnPrintEntry)(strm, pCursor->next(), dbEntryNr, multipleLines);
            dbEntryNr++;
         }
      }
   }
   catch(...)
   {
      // catch any exception in order to clean mongo::ScopedDbConnection and rethrow it
      pConn->done();
      throw;
   }

   pConn->done();
}

void DbHelper::expandMacro(std::string& macro)
{
   std::size_t pos = 0;
   std::string expandedMacro = getTimeNowMacro();

   if (0 == macro.find("$"))
   {
      pos = macro.find(pTimeNowMacro);
      if (std::string::npos != pos)
      {
         macro.replace(pos, strlen(pTimeNowMacro), expandedMacro);
         return;
      }

      BOOST_THROW_EXCEPTION(DbHelperException() <<
            DbHelperTagInfo((boost::format("Unknown macro %s") % macro).str()));
   }
}

std::string DbHelper::getTimeNowMacro()
{
   unsigned long timeNow = OsDateTime::getSecsSinceEpoch();

   return boost::lexical_cast<std::string>(timeNow);
}

void DbHelper::appendRequiredType(MongoDB::ScopedDbConnectionPtr& pConn,
                                    mongo::BSONObjBuilder& queryObjBuilder,
                                    const std::string& databaseName,
                                    const std::string& string)
{
   mongo::BSONObj bsonObj;
   std::auto_ptr<mongo::DBClientCursor> pCursor = pConn->get()->query(databaseName, mongo::BSONObj());
   if (pCursor.get() && pCursor->more())
   {
      bsonObj = pCursor->next();
   }

   std::string key;
   std::string value;
   std::string logicalOperator;

   extractOperands(string, key, logicalOperator, value);

   expandMacro(value);


   mongo::Labeler::Label logicalOperatorLabel = mongo::BSIZE;
   bool found = false;

   found = getLogicalOperator(logicalOperator, logicalOperatorLabel);

   mongo::BSONElement obj = bsonObj[key];

   if (mongo::String == obj.type())
   {
      found == true ?
               queryObjBuilder << key << logicalOperatorLabel << value :
               queryObjBuilder << key << value;
   }
   else if (mongo::Bool == obj.type())
   {
      found == true ?
               queryObjBuilder << key << logicalOperatorLabel << boost::lexical_cast<bool>(value) :
               queryObjBuilder << key << boost::lexical_cast<bool>(value);
   }
   else if (mongo::NumberInt == obj.type())
   {
      found == true ?
               queryObjBuilder << key << logicalOperatorLabel << boost::lexical_cast<int>(value) :
               queryObjBuilder << key << boost::lexical_cast<int>(value);
   }
   else
   {
      BOOST_THROW_EXCEPTION(DbHelperException() <<
            DbHelperTagInfo((boost::format("No such type defined: %d\n") % obj.type()).str()));
   }
}

template <class T>
void DbHelper::printCell(std::ostream& strm, const std::string& cellName, T cellValue, int currentNr, bool multipleLines)
{
   if (multipleLines)
   {
      // %1% - first argument - cellName
      // %2% - second argument - cellValue
      // %|20t| - tabulation of 20 spaces
      strm << boost::format("%1% %|20t|%2%\n") % cellName % cellValue;
   }
   else
   {
      strm << boost::format("\"%1%\":%2%   ") % cellName % cellValue;
   }
}

DbHelper::DbHelper()
{

}

DbHelper::~DbHelper()
{
}
