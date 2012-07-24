/*
 * Copyright (c) eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */


#ifndef STATEQUEUEREGDATA_H
#define	STATEQUEUEREGDATA_H

#include "StateQueueMessage.h"

struct StateQueueRegDataRecord
{
  StateQueueRegDataRecord() : expires(0) {};
  std::string aor;
  std::string supported;
  std::string contact;
  std::string callId;
  int expires;
};


class StateQueueRegData : public StateQueueMessage
{
public:
  StateQueueRegData();
  StateQueueRegData(const std::string& rawString);
  StateQueueRegData(const StateQueueRegDataRecord& record);

  StateQueueRegData& operator=(const StateQueueRegDataRecord& record);

  void setAor(const std::string& value);
  bool getAor(std::string& value);
  void setSupported(const std::string& value);
  bool getSupported(std::string& value);
  void setContact(const std::string& value);
  bool getContact(std::string& value);
  void setCallId(const std::string& value);
  bool getCallId(std::string& value);
  void setExpires(int value);
  bool getExpires(int& value);

  void getRegRecord(StateQueueRegDataRecord& record);
};


//
// Inlines
//


inline StateQueueRegData& StateQueueRegData::operator=(const StateQueueRegDataRecord& record)
{
  setType(StateQueueMessage::Data);
  setAor(record.aor);
  setSupported(record.supported);
  setContact(record.contact);
  setCallId(record.callId);
  setExpires(record.expires);
  return *this;
}

inline StateQueueRegData::StateQueueRegData()
{
  setType(StateQueueMessage::Data);
}

inline StateQueueRegData::StateQueueRegData(const std::string& rawString) :
  StateQueueMessage(rawString)
{
}

inline StateQueueRegData::StateQueueRegData(const StateQueueRegDataRecord& record)
{
  setType(StateQueueMessage::Data);
  setAor(record.aor);
  setSupported(record.supported);
  setContact(record.contact);
  setCallId(record.callId);
  setExpires(record.expires);
}

inline void StateQueueRegData::getRegRecord(StateQueueRegDataRecord& record)
{
  getAor(record.aor);
  getSupported(record.supported);
  getContact(record.contact);
  getCallId(record.callId);
  getExpires(record.expires);
}


inline void StateQueueRegData::setAor(const std::string& value)
{
  set("aor", value);
}

inline bool StateQueueRegData::getAor(std::string& value)
{
  return get("aor", value);
}

inline void StateQueueRegData::setSupported(const std::string& value)
{
  set("supported", value);
}

inline bool StateQueueRegData::getSupported(std::string& value)
{
  return get("supported", value);
}

inline void StateQueueRegData::setContact(const std::string& value)
{
  set("contact", value);
}

inline bool StateQueueRegData::getContact(std::string& value)
{
  return get("contact", value);
}

inline void StateQueueRegData::setCallId(const std::string& value)
{
  set("call-id", value);
}

inline bool StateQueueRegData::getCallId(std::string& value)
{
  return get("call-id", value);
}

inline void StateQueueRegData::setExpires(int value)
{
  set("expires", value);
}

inline bool StateQueueRegData::getExpires(int& value)
{
  return get("expires", value);
}
#endif	/* STATEQUEUEREGDATA_H */

