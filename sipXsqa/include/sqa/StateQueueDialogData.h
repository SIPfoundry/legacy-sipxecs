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

#ifndef STATEQUEUEDIALOGDATA_H
#define	STATEQUEUEDIALOGDATA_H


#include "StateQueueMessage.h"


struct StateQueueDialogDataRecord
{
  StateQueueDialogDataRecord() : localCSeq(0), remoteCSeq(0), expires(0) {};
  std::string callId;
  std::string localBranch;
  std::string remoteBranch;
  std::string localAor;
  std::string remoteAor;
  std::string localContact;
  std::string remoteContact;
  std::string localRouteSet;
  std::string remoteRouteSet;
  int localCSeq;
  int remoteCSeq;
  int expires;
};

class StateQueueDialogData : private StateQueueMessage
{
public:
  StateQueueDialogData()
  {
    setType(StateQueueMessage::Data);
  }

  StateQueueDialogData(const std::string& rawString) :
    StateQueueMessage(rawString)
  {
  }

  StateQueueDialogData(const StateQueueDialogDataRecord& record)
  {
    setType(StateQueueMessage::Data);
    setExpires(record.expires);
    setCallId(record.callId);
    setLocalAor(record.localAor);
    setLocalBranch(record.localBranch);
    setLocalCSeq(record.localCSeq);
    setLocalContact(record.localContact);
    setLocalRouteSet(record.localRouteSet);

    setRemoteAor(record.remoteAor);
    setRemoteBranch(record.remoteBranch);
    setRemoteCSeq(record.remoteCSeq);
    setRemoteContact(record.remoteContact);
    setRemoteRouteSet(record.remoteRouteSet);
  }

  StateQueueDialogData(const StateQueueDialogData& data) :
    StateQueueMessage(data)
  {
  }

  void getDialogRecord(StateQueueDialogDataRecord& record)
  {
    getExpires(record.expires);
    getCallId(record.callId);
    getLocalAor(record.localAor);
    getLocalBranch(record.localBranch);
    getLocalCSeq(record.localCSeq);
    getLocalContact(record.localContact);
    getLocalRouteSet(record.localRouteSet);

    getRemoteAor(record.remoteAor);
    getRemoteBranch(record.remoteBranch);
    getRemoteCSeq(record.remoteCSeq);
    getRemoteContact(record.remoteContact);
    getRemoteRouteSet(record.remoteRouteSet);
  }
  
  void swap(StateQueueDialogData& data)
  {
    std::swap(_type, data._type);
    std::swap(_object, data._object);
  }
  
  StateQueueDialogData& operator=(StateQueueDialogData& data)
  {
    StateQueueDialogData clonable(data);
    swap(clonable);
    return *this;
  }

  StateQueueDialogData& operator=(const StateQueueDialogDataRecord& record)
  {
    StateQueueDialogData clonable(record);
    swap(clonable);
    return *this;
  }

  void setExpires(int expires)
  {
    set("expires", expires);
  }

  bool getExpires(int& expires)
  {
    return get("expires", expires);
  }

  void setCallId(const std::string& callId)
  {
    if (callId.empty())
      return;
    set("call-id", callId);
  }

  bool getCallId(std::string& callId)
  {
    return get("call-id", callId);
  }

  void setLocalCSeq(int cseq)
  {
    set("local-cseq", cseq);
  }

  bool getLocalCSeq(int& cseq)
  {
    return get("local-cseq", cseq);
  }

  void setRemoteCSeq(int cseq)
  {
    set("remote-cseq", cseq);
  }

  bool getRemoteCSeq(int& cseq)
  {
    return get("remote-cseq", cseq);
  }

  void setLocalBranch(const std::string& branch)
  {
    if (branch.empty())
      return;
    set("local-branch", branch);
  }

  bool getLocalBranch(std::string& branch)
  {
    return get("local-branch", branch);
  }

  void setRemoteBranch(const std::string& branch)
  {
    if (branch.empty())
      return;
    set("remote-branch", branch);
  }

  bool getRemoteBranch(std::string& branch)
  {
    return get("remote-branch", branch);
  }

  void setLocalAor(const std::string& aor)
  {
    if (aor.empty())
      return;
    set("local-aor", aor);
  }

  bool getLocalAor(std::string& aor)
  {
    return get("local-aor", aor);
  }

  void setRemoteAor(const std::string& aor)
  {
    if (aor.empty())
      return;
    set("remote-aor", aor);
  }

  bool getRemoteAor(std::string& aor)
  {
    return get("remote-aor", aor);
  }

  void setLocalContact(const std::string& contact)
  {
    if (contact.empty())
      return;
    set("local-contact", contact);
  }

  bool getLocalContact(std::string& contact)
  {
    return get("local-contact", contact);
  }

  void setRemoteContact(const std::string& contact)
  {
    if (contact.empty())
      return;
    set("remote-contact", contact);
  }

  bool getRemoteContact(std::string& contact)
  {
    return get("remote-contact", contact);
  }

  void setLocalRouteSet(const std::string& routeset)
  {
    if (routeset.empty())
      return;
    set("local-route-set", routeset);
  }

  bool getLocalRouteSet(std::string& routeset)
  {
    return get("local-route-set", routeset);
  }

  void setRemoteRouteSet(const std::string& routeset)
  {
    if (routeset.empty())
      return;
    set("remote-route-set", routeset);
  }

  bool getRemoteRouteSet(std::string& routeset)
  {
    return get("remote-route-set", routeset);
  }

  std::string data()
  {
    return StateQueueMessage::data();
  }


};

#endif	/* STATEQUEUEDIALOGDATA_H */

