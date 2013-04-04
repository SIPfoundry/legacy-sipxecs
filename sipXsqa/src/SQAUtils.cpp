#include "sqa/SQAUtils.h"

//const int SQAUtil::SQAClientUnknown = 0;
//const int SQAUtil::SQAClientPublisher = SQAClientRolePublisher;
//const int SQAUtil::SQAClientDealer =  SQAClientRolePublisher | SQAClientRoleDealer;
//const int SQAUtil::SQAClientWatcher =  SQAClientRoleWatcher;
//const int SQAUtil::SQAClientWorker =  SQAClientRoleWatcher | SQAClientRoleWorker;
//const int SQAUtil::SQAClientWorkerMulti =  SQAClientRoleWatcher | SQAClientRoleWorker | SQAClientRoleMulti;

const char * connectionEventStr[] =
{
    "unknown",
    "established",
    "sigin",
    "keepalive",
    "logout",
    "terminate",
};

const char* SQAUtil::getConnectionEventStr(ConnectionEvent connectionEvent)
{
    if (connectionEvent >= ConnectionEventNum)
    {
        connectionEvent = ConnectionEventUnknown;
    }

    return connectionEventStr[connectionEvent];
}


void SQAUtil::generateRecordId(std::string &recordId, ConnectionEvent connectionEvent)
{
    recordId = PublisherWatcherPrefix;
    recordId += ".connection.";
    recordId += getConnectionEventStr(connectionEvent);
}

bool SQAUtil::generateZmqEventId(std::string &zmqEventId, int serviceType, std::string &eventId)
{
    bool ret = false;

    if (isWatcherOnly(serviceType))
    {
        zmqEventId = PublisherWatcherPrefix;
        zmqEventId += "." + eventId;
        ret = true;
    }
    else if (isWorker(serviceType))
    {
        zmqEventId = DealerWorkerPrefix;
        zmqEventId += "." + eventId;
        ret = true;
    }

    return ret;
}

bool SQAUtil::validateId(const std::string &id, int serviceType, const std::string &eventId)
{
    std::vector<std::string> parts;
    boost::algorithm::split(parts, id, boost::is_any_of("."), boost::token_compress_on);

    if (3 > parts.size())
        return false;

    if (parts[0] == DealerWorkerPrefix)
    {
      if (!isDealer(serviceType) && !isWorker(serviceType))
      {
        return false;
      }
    }
    else if (parts[0] == PublisherWatcherPrefix)
    {
      if (!isPublisherOnly(serviceType) && !isWatcherOnly(serviceType))
      {
        return false;
      }
    }
    else
    {
      return false;
    }

    if (eventId != parts[1])
    {
        return false;
    }

    std::string hextokens = parts[parts.size() - 1];
    parts.clear();

    boost::algorithm::split(parts, hextokens, boost::is_any_of("-"), boost::token_compress_on);
    if (2 != parts.size())
        return false;


    return (validateIdHexComponent(parts[0]) && validateIdHexComponent(parts[1]));
}

bool SQAUtil::validateId(const std::string &id, int serviceType)
{
    std::vector<std::string> parts;
    boost::algorithm::split(parts, id, boost::is_any_of("."), boost::token_compress_on);

    if (2 > parts.size())
        return false;

    if (parts[0] == DealerWorkerPrefix)
    {
      if (!isDealer(serviceType) && !isWorker(serviceType))
      {
        return false;
      }
    }
    else if (parts[0] == PublisherWatcherPrefix)
    {
      if (!isPublisherOnly(serviceType) && !isWatcherOnly(serviceType))
      {
        return false;
      }
    }
    else
    {
      return false;
    }

    return true;
}

bool SQAUtil::validateIdHexComponent(const std::string &hex)
{
    if (hex.size() != 4)
        return false;

    if (hex.find_first_not_of("0123456789ABCDEF") != std::string::npos)
    {
        return false;
    }

    return true;
}
