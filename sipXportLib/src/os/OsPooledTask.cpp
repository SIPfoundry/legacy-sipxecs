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


#include "os/OsPooledTask.h"


const int OsPooledTask::DEF_MAX_MSGS = OsMsgQ::DEF_MAX_MSGS;


OsPooledTask::OsPooledTask(const char* name, std::size_t poolSize, const int maxRequestQMsgs, bool synchronizeHandler) :
  _messageQueue(name, maxRequestQMsgs, OsMsgQ::DEF_MAX_MSG_LEN, OsMsgQ::Q_PRIORITY),
  _poolSize(poolSize),
  _isTerminated(true),
  _synchronizeHandler(synchronizeHandler),
  mName(name)
{
}

OsPooledTask::~OsPooledTask()
{
  requestShutdown();
}


UtlBoolean OsPooledTask::start(void)
{
  if (_threadPool.empty() && _isTerminated)
  {
    _isTerminated = false;
    for (std::size_t i = 0; i < _poolSize; i++)
      _threadPool.push_back(new boost::thread(boost::bind(&OsPooledTask::run, this, (void*)0)));
    return TRUE;
  }
  return FALSE;
}

void OsPooledTask::requestShutdown(void)
{
  if (!_isTerminated && !_threadPool.empty())
  {
    OsMsg msg(OsMsg::OS_SHUTDOWN, 0);

    for (ThreadPool::iterator iter = _threadPool.begin(); iter != _threadPool.end(); iter++)
      postMessage(msg); // wake up the task run loop
    
    for (ThreadPool::iterator iter = _threadPool.begin(); iter != _threadPool.end(); iter++)
    {
      boost::thread* pThread = *iter;
      pThread->join();
      delete pThread;
    }
    _threadPool.clear();
  }
}


int OsPooledTask::run(void* pArg)
{
  while (!_isTerminated)
  {
    OsMsg* pMsg = 0;
    
    if (OS_SUCCESS == receiveMessage((OsMsg*&) pMsg) && pMsg)
    {
      if (!_isTerminated)
      {
        if (_synchronizeHandler)
        {
          mutex_lock lock(_mutex);
          handleMessage(*pMsg);
        }
        else
        {
          handleMessage(*pMsg);
        }
      }
    }
    else
    {
      break;
    }

    //
    // Check if the request is a shutdown message
    //
    if (pMsg->getMsgType() == OsMsg::OS_SHUTDOWN)
    {
      if (!pMsg->getSentFromISR())
       pMsg->releaseMsg();
      break;
    }
    else if (!pMsg->getSentFromISR())
    {
       pMsg->releaseMsg();  // free the message
    }
  }
  _isTerminated = true;
  return 0;
}







