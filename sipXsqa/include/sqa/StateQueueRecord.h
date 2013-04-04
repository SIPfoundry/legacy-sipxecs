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

#ifndef STATEQUEUERECORD_H
#define	STATEQUEUERECORD_H

#include <vector>
#include <string>
#include <cstdlib>
#include <sqa/StateQueueMessage.h>

struct StateQueueRecord
{
  StateQueueRecord() : retry(0), expires(0), watcherData(false){}
  StateQueueRecord(std::string &_id, std::string &_data, int _expires)
   : id(_id), data(_data), retry(0), expires(_expires), watcherData(false){}
  std::string id;
  std::string data;
  std::vector<std::string> exclude;
  int retry;
  int expires;
  bool watcherData; /// true, this record is an event for a watcher
                    /// false, this record is work from a dealer for a worker

  bool toJson(std::string& jsonString) const;
  bool fromJson(const std::string& jsonString);
};

inline bool StateQueueRecord::toJson(std::string& jsonString) const
{
  StateQueueMessage obj(StateQueueMessage::Data);
  obj.set("id", id);
  obj.set("data", data);
  obj.set("retry", retry);
  obj.set("expires", expires);
  obj.set("watcherData", watcherData);
  jsonString = obj.data();
  return !jsonString.empty();
}

inline bool StateQueueRecord::fromJson(const std::string& jsonString)
{
  StateQueueMessage obj;
  obj.parseData(jsonString, true);
  obj.get("id", id);
  obj.get("data", data);
  obj.get("retry", retry);
  obj.get("expires", expires);
  obj.get("watcherData", watcherData);
  return true;
}

#endif	/* STATEQUEUERECORD_H */

