/*
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
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

#ifndef STATEQUEUERECORD_H
#define	STATEQUEUERECORD_H

#include <vector>
#include <string>

struct StateQueueRecord
{
  StateQueueRecord() : retry(0), expires(0), watcherData(false){}
  std::string id;
  std::string data;
  std::vector<std::string> exclude;
  int retry;
  int expires;
  bool watcherData;
};


#endif	/* STATEQUEUERECORD_H */

