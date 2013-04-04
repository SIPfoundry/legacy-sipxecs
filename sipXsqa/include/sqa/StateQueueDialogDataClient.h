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

#ifndef STATEQUEUEDIALOGDATACLIENT_H
#define	STATEQUEUEDIALOGDATACLIENT_H

#include "StateQueueClient.h"
#include "StateQueueDialogData.h"

class StateQueueDialogDataClient : private StateQueueClient
{
public:
  StateQueueDialogDataClient(
        const std::string& applicationId,
        const std::string& serviceAddress,
        const std::string& servicePort,
        std::size_t poolSize);


  bool createNewDialog(const std::string& dialogId, const StateQueueDialogDataRecord& data);
  bool updateDialog(const std::string& dialogId, const StateQueueDialogDataRecord& data);
  bool getDialog(const std::string& dialogId, StateQueueDialogDataRecord& data);
  bool destroyDialog(const std::string& dialogId);

protected:
  int _workspace;
};


inline StateQueueDialogDataClient::StateQueueDialogDataClient(
  const std::string& applicationId,
  const std::string& serviceAddress,
  const std::string& servicePort,
  std::size_t poolSize) : StateQueueClient(SQAUtil::SQAClientPublisher,
        applicationId,
        serviceAddress,
        servicePort,
        "dialog-state",
        poolSize),
  _workspace(3)
{
}

inline bool StateQueueDialogDataClient::createNewDialog(const std::string& dialogId, const StateQueueDialogDataRecord& data)
{
  StateQueueDialogData dialogData(data);
  return set(_workspace, dialogId, dialogData.data(), data.expires);
}

inline bool StateQueueDialogDataClient::updateDialog(const std::string& dialogId, const StateQueueDialogDataRecord& data)
{
  StateQueueDialogData dialogData(data);
  return set(_workspace, dialogId, dialogData.data(), data.expires);
}

inline bool StateQueueDialogDataClient::getDialog(const std::string& dialogId, StateQueueDialogDataRecord& data)
{
  std::string buff;
  if (!get(_workspace, dialogId, buff))
    return false;
  StateQueueDialogData dialogData(buff);
  dialogData.getDialogRecord(data);
  return true;
}

inline bool StateQueueDialogDataClient::destroyDialog(const std::string& dialogId)
{
  return remove(_workspace, dialogId);
}

#endif	/* STATEQUEUEDIALOGDATACLIENT_H */

