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

#ifndef REDISCLIENTASYNC_H
#define	REDISCLIENTASYNC_H

extern "C"
{
  #include <hiredis/hiredis.h>
  #include <hiredis/async.h>
  #include <sys/types.h>
  #include <libev/ev.h>
}

#include <cassert>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include "RedisClient.h"
#include "Semaphore.h"


#ifndef REDIS_EVENT_CHANNEL
#define REDIS_EVENT_CHANNEL "REDIS"
#endif

class RedisClientAsync : protected RedisClient
{
public: // Members
  typedef boost::function<void(int)> StatusCallback;
  typedef boost::function<void(const std::vector<std::string>&)> ResponseCallback;
  redisAsyncContext* _contextAsync;
  boost::thread* _pEventLoopThread;
  bool _calledConnect;
  StatusCallback _connectCb;
  StatusCallback _disconnectCb;
  ResponseCallback _responseCb;
  struct ev_loop* _eventLoop;
  Semaphore _eventSync;
  bool _terminated;
public: // Methods

  RedisClientAsync() :
    _contextAsync(0),
    _pEventLoopThread(0),
    _calledConnect(false),
    _eventLoop(0),
    _terminated(false)
  {
    _context = 0;
    _type = TCP;
    _db = 0;
    _eventLoop = ev_loop_new(0);
  }

  ~RedisClientAsync()
  {
    stop();

    if (_eventLoop)
      ev_loop_destroy(_eventLoop);

    if (_contextAsync)
    {
      redisAsyncFree(_contextAsync);
      _contextAsync = 0;
    }
  }

  void connect(
    StatusCallback connectCb,
    StatusCallback disconnectCb,
    ResponseCallback responseCb)
  {
    try
    {
      std::ostringstream sqaconfig;
      sqaconfig << SIPX_CONFDIR << "/" << "redis-client.ini";
      ServiceOptions configOptions(sqaconfig.str());
      if (configOptions.parseOptions())
      {
        bool enabled = false;
        if (configOptions.getOption("enabled", enabled, enabled) && enabled)
        {
          configOptions.getOption("tcp-address", _tcpHost);
          configOptions.getOption("tcp-port", _tcpPort);
        }
      }
    }
    catch(...)
    {
    }

    _type = TCP;
    connect(_tcpHost, _tcpPort, connectCb, disconnectCb, responseCb);
  }


  void connect(
    const std::string& tcpHost,
    int tcpPort,
    StatusCallback connectCb,
    StatusCallback disconnectCb,
    ResponseCallback responseCb)
  {
    _contextAsync = redisAsyncConnect(_tcpHost.c_str(), _tcpPort);
    if (!_contextAsync || _contextAsync->err)
    {
      OS_LOG_ERROR(FAC_NET, "RedisClientAsync::connect is unable to create a context!");
      return;
    }
    _contextAsync->data = (void*)this;
    redisLibevAttach(_eventLoop ,_contextAsync);
    redisAsyncSetConnectCallback(_contextAsync, &RedisClientAsync::redisClientAsyncConnectCB);
    redisAsyncSetDisconnectCallback(_contextAsync, &RedisClientAsync::redisClientAsyncDisconnectCB);
    _calledConnect = true;
    _connectCb = connectCb;
    _disconnectCb = disconnectCb;
    _responseCb = responseCb;
    _eventSync.signal();
  }

  void disconnect()
  {
    if (_contextAsync)
      redisAsyncDisconnect(_contextAsync);
     _eventSync.signal();
  }

  void run()
  {
    assert(_calledConnect && !_pEventLoopThread);
    _pEventLoopThread = new boost::thread(boost::bind(&RedisClientAsync::internal_run, this));
  }

  void stop()
  {
    _terminated = true;
    _eventSync.signal(); // signal the internal loop that we are about to terminate

    if (_eventLoop)
    {
      ev_unloop(_eventLoop, EVUNLOOP_ALL);
    }

    if (_pEventLoopThread)
    {
      _pEventLoopThread->join();
      delete _pEventLoopThread;
      _pEventLoopThread = 0;
    }

    if (_eventLoop)
    {
      ev_loop_destroy(_eventLoop);
      _eventLoop = 0;
    }

    _calledConnect = false;
  }

  int asyncCommand(const std::vector<std::string>& args)
  {
    char** argv = (char**)std::malloc((args.size() + 1) * sizeof(char*));
    std::size_t i=0;
    for(std::vector<std::string>::const_iterator iter = args.begin();
        iter != args.end();
        iter++, ++i)
    {
      std::string arg = *iter;
      argv[i] = (char*)std::malloc((arg.length()+1) * sizeof(char));
      std::strcpy(argv[i], arg.c_str());
    }
    argv[args.size()] = NULL; // argv must be NULL terminated

    int ret = redisAsyncCommandArgv(
            _contextAsync, 
            &RedisClientAsync::redisClientAsyncReplyCb,
            0,
            args.size(),
            (const char**)argv, (const size_t*)0);

    for (i = 0; i < args.size(); i++)
      free((argv)[i]);
    free(argv);

    _eventSync.signal();

    return ret;
  }

protected:
  

  void onConnect(int status)
  {
    if (_connectCb)
      _connectCb(status);
  }

  void onDisconnect(int status)
  {
    if (_disconnectCb)
      _disconnectCb(status);
  }

  void onReply(const std::vector<std::string>& reply)
  {
    if (_responseCb)
      _responseCb(reply);
  }

  void internal_run()
  {
    OS_LOG_NOTICE(FAC_NET, "RedisClientAsync::internal_run STARTED.");
    while (!_terminated)
    {
      _eventSync.wait();
      OS_LOG_NOTICE(FAC_NET, "RedisClientAsync::internal_run AWAKENED");
      if (!_terminated && _eventLoop)
        ev_loop(_eventLoop, 0);
      else
        break;
    }
    OS_LOG_NOTICE(FAC_NET, "RedisClientAsync::internal_run TERMINATED.");
  }

  struct redisLibevEvents
  {
      redisAsyncContext *context;
      struct ev_loop *loop;
      int reading, writing;
      ev_io rev, wev;
  };

  static void redisLibevReadEvent(EV_P_ ev_io *watcher, int revents)
  {
  #if EV_MULTIPLICITY
      ((void)loop);
  #endif
      ((void)revents);

      redisLibevEvents *e = (redisLibevEvents*)watcher->data;
      redisAsyncHandleRead(e->context);
  }

  static void redisLibevWriteEvent(EV_P_ ev_io *watcher, int revents)
  {
  #if EV_MULTIPLICITY
      ((void)loop);
  #endif
      ((void)revents);

      redisLibevEvents *e = (redisLibevEvents*)watcher->data;
      redisAsyncHandleWrite(e->context);
  }

  static void redisLibevAddRead(void *privdata)
  {
      redisLibevEvents *e = (redisLibevEvents*)privdata;
      struct ev_loop *loop = e->loop;
      ((void)loop);
      if (!e->reading) {
          e->reading = 1;
          ev_io_start(EV_A_ &e->rev);
      }
  }

  static void redisLibevDelRead(void *privdata)
  {
      redisLibevEvents *e = (redisLibevEvents*)privdata;
      struct ev_loop *loop = e->loop;
      ((void)loop);
      if (e->reading) {
          e->reading = 0;
          ev_io_stop(EV_A_ &e->rev);
      }
  }

  static void redisLibevAddWrite(void *privdata)
  {
      redisLibevEvents *e = (redisLibevEvents*)privdata;
      struct ev_loop *loop = e->loop;
      ((void)loop);
      if (!e->writing) {
          e->writing = 1;
          ev_io_start(EV_A_ &e->wev);
      }
  }

  static void redisLibevDelWrite(void *privdata)
  {
      redisLibevEvents *e = (redisLibevEvents*)privdata;
      struct ev_loop *loop = e->loop;
      ((void)loop);
      if (e->writing) {
          e->writing = 0;
          ev_io_stop(EV_A_ &e->wev);
      }
  }

  static void redisLibevCleanup(void *privdata)
  {
      redisLibevEvents *e = (redisLibevEvents*)privdata;
      redisLibevDelRead(privdata);
      redisLibevDelWrite(privdata);
      free(e);
  }

  static int redisLibevAttach(EV_P_ redisAsyncContext *ac)
  {
      redisContext *c = &(ac->c);
      redisLibevEvents *e;

      /* Nothing should be attached when something is already attached */
      if (ac->ev.data != NULL)
          return REDIS_ERR;

      /* Create container for context and r/w events */
      e = (redisLibevEvents*)malloc(sizeof(*e));
      e->context = ac;
  #if EV_MULTIPLICITY
      e->loop = loop;
  #else
      e->loop = NULL;
  #endif
      e->reading = e->writing = 0;
      e->rev.data = e;
      e->wev.data = e;

      /* Register functions to start/stop listening for events */
      ac->ev.addRead = &RedisClientAsync::redisLibevAddRead;
      ac->ev.delRead = &RedisClientAsync::redisLibevDelRead;
      ac->ev.addWrite = &RedisClientAsync::redisLibevAddWrite;
      ac->ev.delWrite = &RedisClientAsync::redisLibevDelWrite;
      ac->ev.cleanup = &RedisClientAsync::redisLibevCleanup;
      ac->ev.data = e;

      /* Initialize read/write events */
      ev_io_init(&e->rev,&RedisClientAsync::redisLibevReadEvent,c->fd,EV_READ);
      ev_io_init(&e->wev,&RedisClientAsync::redisLibevWriteEvent,c->fd,EV_WRITE);
      return REDIS_OK;
  }

  static void redisClientAsyncConnectCB(const redisAsyncContext *c, int status)
  {
    RedisClientAsync* pClient = reinterpret_cast<RedisClientAsync*>(c->data);
    if (pClient)
      pClient->onConnect(status);
    else
      OS_LOG_ERROR(FAC_NET, "redisClientAsyncConnectCB c->data==NULL");
  }

  static void redisClientAsyncDisconnectCB(const redisAsyncContext *c, int status)
  {
    RedisClientAsync* pClient = reinterpret_cast<RedisClientAsync*>(c->data);
    if (pClient)
      pClient->onDisconnect(status);
    else
      OS_LOG_ERROR(FAC_NET, "redisClientAsyncConnectCB c->data==NULL");
  }

  static void redisClientAsyncReplyCb(struct redisAsyncContext *c, void *r, void *privdata)
  {
    RedisClientAsync* pClient = reinterpret_cast<RedisClientAsync*>(c->data);
    if (pClient)
    {
      redisReply* reply = reinterpret_cast<redisReply*>(r);
      if (reply)
      {
        //
        // Propagate to the client as vector
        //
        std::vector<std::string> array;

        if (reply && reply->type == REDIS_REPLY_ARRAY && reply->elements > 0)
        {
          for (size_t i = 0; i < reply->elements; i++)
          {
            redisReply* item = reply->element[i];
            if (item && item->type == REDIS_REPLY_STRING && item->len > 0)
            {
              array.push_back(std::string(item->str, item->len));
            }
            else if (item && item->type == REDIS_REPLY_INTEGER)
            {
              try
              {
                array.push_back(boost::lexical_cast<std::string>(item->integer));
              }
              catch(...)
              {
              }
            }
          }

          // Do not free the reply.  The internal redis event loop deletes it
          //
          // freeReplyObject(reply);
          //
        }

        pClient->onReply(array);
      }
    }
  }
};


#endif	/* REDISCLIENTASYNC_H */

