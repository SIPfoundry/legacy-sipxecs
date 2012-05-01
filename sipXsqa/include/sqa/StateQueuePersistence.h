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

#ifndef STATEQUEUEPERSISTENCE_H
#define	STATEQUEUEPERSISTENCE_H

#include "TimedQueue.h"
#include "TimedMap.h"
#include "StateQueueRecord.h"


class StateQueuePersistence
{
public:
  typedef std::map<int, TimedQueue*> WorkSpaces ;
  typedef std::map<int, TimedMap*> MapWorkSpaces ;
  typedef std::vector<StateQueueRecord> RecordVector;
  StateQueuePersistence(unsigned workspaceCount = 16);
  ~StateQueuePersistence();
  void set(unsigned workspace, const StateQueueRecord& record, int expires);
  bool get(unsigned workspace, const std::string& id, StateQueueRecord& record);
  bool erase(unsigned workspace, const std::string& id);
  //
  // Map operations
  //
  void mapSet(unsigned workspace, const std::string& mapId, const StateQueueRecord& record, int expires);
  bool mapGet(unsigned workspace, const std::string& mapId, const std::string& id, StateQueueRecord& record);
  bool mapGetInc(unsigned workspace, const std::string& mapId, const std::string& id, StateQueueRecord& record);
  bool mapGet(unsigned workspace, const std::string& mapId, RecordVector& records);
  void mapClear(unsigned workspace, const std::string& mapId);
  void mapRemove(unsigned workspace, const std::string& mapId, const std::string& dataId);
  void stop();
protected:
  void onDataExpired(const std::string&, const boost::any&, TimedQueue* pQueue);
  WorkSpaces _workspaces;
  MapWorkSpaces _mapWorkspaces;
  bool _terminated;
private:
  boost::asio::io_service _ioService;
  boost::thread* _pIothread;
  boost::asio::deadline_timer* _pHousekeeper;
  void onHousekeeping(const boost::system::error_code&);
};

#endif	/* STATEQUEUEPERSISTENCE_H */

