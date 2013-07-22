#include "GatewayDestPlugin.h"
#include <cassert>
#include <utl/UtlString.h>
#include <sys/time.h>
#include <sstream>
#include <net/NetBase64Codec.h>
#include <os/OsDateTime.h>
#include <net/SipXlocationInfo.h>

const unsigned int GatewayDestPlugin::MongoRecordExpirationTimeout = 3600; // secs

/// Factory used by PluginHooks to dynamically link the plugin instance
extern "C" SipBidirectionalProcessorPlugin* getTransactionPlugin(const UtlString& pluginName)
{
   return new GatewayDestPlugin(pluginName);
}

GatewayDestPlugin::GatewayDestPlugin(const UtlString& instanceName, int priority) :
  SipBidirectionalProcessorPlugin(instanceName, priority)
{
  OS_LOG_DEBUG(FAC_SIP, "GatewayDestPlugin::" << __FUNCTION__);

  {
    MongoDB::ConnectionInfo info(MongoDB::ConnectionInfo::globalInfo());
    _gatewayDestDB = new GatewayDestDB(info, GatewayDestDB::NS);
  }

  {
    MongoDB::ConnectionInfo info(MongoDB::ConnectionInfo::globalInfo());
    _entityDB = new EntityDB(info, EntityDB::NS);
  }
}

GatewayDestPlugin::~GatewayDestPlugin()
{
  OS_LOG_DEBUG(FAC_SIP, "GatewayDestPlugin::" << __FUNCTION__);

  if (_gatewayDestDB)
  {
    delete _gatewayDestDB;
    _gatewayDestDB = NULL;
  }

  if (_entityDB)
  {
    delete _entityDB;
    _entityDB = NULL;
  }
}

void GatewayDestPlugin::readConfig(OsConfigDb& configDb)
{
  OS_LOG_DEBUG(FAC_SIP, "GatewayDestPlugin::" << __FUNCTION__);
}

void GatewayDestPlugin::initialize()
{
  OS_LOG_DEBUG(FAC_SIP, "GatewayDestPlugin::" << __FUNCTION__);
  assert(_pUserAgent);
}

void GatewayDestPlugin::extractRecordData(const SipMessage& message, UtlString* callid, UtlString* toTag, UtlString* fromTag) const
{
  if (callid)
  {
    message.getCallIdField(callid);
  }

  if (toTag)
  {
    Url toUrl;
    message.getToUrl(toUrl);
    toUrl.getFieldParameter("tag", *toTag);
  }

  if (fromTag)
  {
    Url fromUrl;
    message.getFromUrl(fromUrl);
    fromUrl.getFieldParameter("tag", *fromTag);
  }
}

bool GatewayDestPlugin::getUserLocation(const UtlString& identity, UtlString& location) const
{

  EntityRecord entity;
  if (_entityDB->findByIdentity(identity.str(), entity))
  {
    location = entity.location().c_str();

    return true;
  }

  return false;
}

void GatewayDestPlugin::addDBRecord(const SipMessage& message, const UtlString& lineId)
{
  UtlString callid;
  UtlString toTag;
  UtlString fromTag;
  extractRecordData(message, &callid, &toTag, &fromTag);

  // Caller identity is extracted from "From" header
  Url fromUrl;
  UtlString fromIdentity;
  message.getFromUrl(fromUrl);
  fromUrl.getIdentity(fromIdentity);

  // create new record
  GatewayDestRecord record(callid.data(), toTag.data(), fromTag.data());
  record.setLineId(lineId.data());
  record.setIdentity(fromIdentity.data());
  record.setExpirationTime(MongoRecordExpirationTimeout + (unsigned int) OsDateTime::getSecsSinceEpoch());

  // add new record in mongo
  _gatewayDestDB->updateRecord(record, true);

  OS_LOG_DEBUG(FAC_SIP, "GatewayDestPlugin::" << __FUNCTION__ << " Added new record for lineID:" << lineId
      << ", callid:" << callid << ", lineId:" << lineId);
}


void GatewayDestPlugin::addLocationInfo(SipMessage& message, const GatewayDestRecord& record)
{
  UtlString identity = record.getIdentity();
  SipXSignedHeader locationInfo(identity, SIP_SIPX_LOCATION_INFO);

  // set line-id param as found in mongo
  locationInfo.setParam(SIPX_SIPXECS_LINEID_URI_PARAM, record.getLineId());

  UtlString location;
  if (getUserLocation(identity, location) && !location.isNull())
  {
    // set location param if any
    locationInfo.setParam(SIPX_SIPXECS_LOCATION_URI_PARAM, location.data());
  }
  else
  {
    OS_LOG_DEBUG(FAC_SIP, "GatewayDestPlugin::" << __FUNCTION__ << " Identity:" << identity <<
        " has no location specified");
  }

  UtlString headerValue;
  if (locationInfo.encode(headerValue))
  {
    message.setHeaderValue(SIP_SIPX_LOCATION_INFO, headerValue.data(), 0);
    OS_LOG_DEBUG(FAC_SIP, "GatewayDestPlugin::" << __FUNCTION__ << "Added new X-SipX-Location-Info header:"
        << headerValue.data());
  }
  else
  {
    OS_LOG_DEBUG(FAC_SIP, "GatewayDestPlugin::" << __FUNCTION__ << "failed to encode location info");
  }
}


void GatewayDestPlugin::handleIncoming(SipMessage& message, const char* address, int port)
{
  OS_LOG_DEBUG(FAC_SIP, "GatewayDestPlugin::" << __FUNCTION__ << " Entering");

  UtlString method;

  if (message.isResponse())
  {
    //response

    int seq = 0;
    if (message.getCSeqField(&seq, &method)) // get the method out of cseq field
    {
      UtlString lineId;
      int responseStatus = message.getResponseStatusCode();
      bool successfulCallResp = (0 == method.compareTo(SIP_INVITE_METHOD, UtlString::ignoreCase)) &&
          ((responseStatus >= SIP_2XX_CLASS_CODE) && (responseStatus <  SIP_3XX_CLASS_CODE));

      if (successfulCallResp && topViaHasLineId(message, lineId))
      {
        // response to an INVITE, successful call, contains sipxecs-lineid in rr -> add db record for it
        addDBRecord(message, lineId);
      }
    }
  }
  else
  {
    UtlString targetCallId;
    UtlString targetToTag;
    UtlString targetFromTag;

    bool needsLocationInfo = message.getHeaderValue(0, "Replaces")  && // does contain Replaces header
        (NULL == message.getHeaderValue(0, SIP_SIPX_LOCATION_INFO));  // does not contain location info

    if (needsLocationInfo)
    {
      if (message.getReplacesData(targetCallId, targetToTag, targetFromTag))
      {
        //see if there is an associated record in mongo
        GatewayDestRecord record(targetCallId.data(), targetToTag.data(), targetFromTag.data());
        if (_gatewayDestDB->getUnexpiredRecord(record))
        {
          addLocationInfo(message, record);
        }
        else
        {
          OS_LOG_DEBUG(FAC_SIP, "GatewayDestPlugin::" << __FUNCTION__ << " No gateway destination record in mongo for callid:"
              << targetCallId.data() << ", to-tag:" << targetToTag.data() << ", from-tag:" << targetFromTag.data());
        }
      }
      else
      {
        OS_LOG_ERROR(FAC_SIP, "GatewayDestPlugin::" << __FUNCTION__ << " getReplacesData failed");
      }
    }
  }
}

bool GatewayDestPlugin::topViaHasLineId(const SipMessage& message, UtlString& lineId)
{
  bool ret = false;

  UtlString via;
  if (message.getFieldSubfield(SIP_VIA_FIELD, 0, &via))
  {
    // add the tag to top via

    // sipxecs-lineid will not be added to rr in case it is already there
    ret = SipMessage::getViaTag(via.data(), SIPX_SIPXECS_LINEID_URI_PARAM, lineId);
  }

  return ret;
}


void GatewayDestPlugin::addLineIdToTopVia(SipMessage& message, const UtlString& lineId)
{
  UtlString via;
  if (message.getFieldSubfield(SIP_VIA_FIELD, 0, &via))
  {
    // add the tag to top via

    // sipxecs-lineid will not be added to rr in case it is already there
    UtlString existingLineId;
    if (!SipMessage::getViaTag(via.data(), SIPX_SIPXECS_LINEID_URI_PARAM, existingLineId))
    {

      if (message.setViaTag(lineId.data(), SIPX_SIPXECS_LINEID_URI_PARAM, 0))
      {
        OS_LOG_DEBUG(FAC_SIP, "GatewayDestPlugin::" << __FUNCTION__ << " adding to top via" << SIPX_SIPXECS_LINEID_URI_PARAM);
      }
      else
      {
        OS_LOG_NOTICE(FAC_SIP, "GatewayDestPlugin::" << __FUNCTION__ << " setViaTag failed adding " << SIPX_SIPXECS_LINEID_URI_PARAM << " to Via");
      }
    }
    else
    {
      OS_LOG_ERROR(FAC_SIP, "GatewayDestPlugin::" << __FUNCTION__ << " via already contains tag " << SIPX_SIPXECS_LINEID_URI_PARAM);
    }
  }
  else
  {
    OS_LOG_ERROR(FAC_SIP, "GatewayDestPlugin::" << __FUNCTION__ << " getFieldSubfield failed to get top via");
  }
}


void GatewayDestPlugin::handleOutgoing(SipMessage& message, const char* address, int port)
{
  OS_LOG_DEBUG(FAC_SIP, "GatewayDestPlugin::" << __FUNCTION__ );

  if (message.isResponse())
  {
    // message is NOT a request
    return;
  }

  UtlString method;
  message.getRequestMethod(&method);
  if (0 != method.compareTo(SIP_INVITE_METHOD, UtlString::ignoreCase))
  {
    //method is not an INVITE
    return;
  }


  Url toUrl;
  message.getToUrl(toUrl);
  UtlString toTag;
  toUrl.getFieldParameter("tag", toTag);

  if (!toTag.isNull())
  {
    // NOT a new call INVITE
    return;
  }

  UtlString inviteUriStr;
  message.getRequestUri(&inviteUriStr);
  Url inviteUri(inviteUriStr, Url::AddrSpec);
  UtlString headerLineId;
  if (!inviteUri.getUrlParameter(SIPX_SIPXECS_LINEID_URI_PARAM, headerLineId, 0))
  {
    // Request uri does not contain sipxecs-lineid tags in it.
    return;
  }

  if (message.getHeaderValue(0, SIP_SIPX_SPIRAL_HEADER))
  {
    // INVITE is still spiraling, wait until it  is leaving proxy
    return;
  }

  if (message.getHeaderValue(0, SIP_SIPX_LOCATION_INFO))  // does not contain location info
  {
    // INVITE already contains a location info header, si it already has the right gateway as destination
    return;
  }

  // this is an INVITE which to have lineid in via, so when call is completed with 200 OK
  // a record will be added in mongo with its gateway dest
  addLineIdToTopVia(message, headerLineId);
}

