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
#include "sqa/json/reader.h"
#include "sqa/json/writer.h"
#include "sqa/json/elements.h"


struct StateQueueRecord
{
  StateQueueRecord() : retry(0), expires(0), watcherData(false){}
  std::string id;
  std::string data;
  std::vector<std::string> exclude;
  int retry;
  int expires;
  bool watcherData;

  bool toJson(std::string& jsonString);
  bool fromJson(const std::string& jsonString);
};

inline bool StateQueueRecord::toJson(std::string& jsonString)
{
  json::Object object;
  object["id"] = json::String(id);
  object["data"] = json::String(data);
  object["retry"] = json::Number(retry);
  object["expires"] = json::Number(expires);
  object["watcherData"] = json::Boolean(watcherData);

  if (!exclude.empty())
  {
    json::Array excludeVec;
    for (std::size_t i = 0; i < exclude.size(); i++)
      excludeVec[i] = json::String(exclude[i]);
    object["exclude"] = excludeVec;
  }

  try
  {
    std::ostringstream strm;
    json::Writer::Write(object, strm);
    jsonString = strm.str();
  }
  catch(std::exception& error)
  {
    return false;
  }
  return true;
}

inline bool StateQueueRecord::fromJson(const std::string& jsonString)
{
  json::Object object;
  try
  {
    std::stringstream strm;
    strm << jsonString;
    json::Reader::Read(object, strm);
    id = ((json::String&)object["id"]).Value();
    data = ((json::String&)object["data"]).Value();
    retry = ((json::Number&)object["retry"]).Value();
    expires = ((json::Number&)object["expires"]).Value();
    watcherData = ((json::Boolean)object["watcherData"]).Value();
    exclude.clear();
    if (object.Find("exclude") != object.End())
    {
      json::Array& excludeVec = object["exclude()"];
      for (std::size_t i = 0; i < excludeVec.Size(); i++)
        exclude.push_back(((json::String&)excludeVec[i]).Value());
    }

  }
  catch(std::exception& error)
  {
    return false;
  }

  return true;
}

#endif	/* STATEQUEUERECORD_H */

