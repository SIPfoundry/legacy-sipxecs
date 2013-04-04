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


#define PLUGIN_LOADER 1

#include "sqa/sqaclient_plugin.h"

extern "C" {

SQAWatcher* plugin_createWatcher(
  const char* applicationId, // Unique application ID that will identify this watcher to SQA
  const char* serviceAddress, // The IP address of the SQA
  const char* servicePort, // The port where SQA is listening for connections
  const char* eventId, // Event ID of the event being watched. Example: "sqa.not"
  int poolSize // Number of active connections to SQA
)
{
  return new SQAWatcher(applicationId, serviceAddress, servicePort, eventId, poolSize, SQA_CONN_READ_TIMEOUT, SQA_CONN_WRITE_TIMEOUT);
}

void plugin_destroyWatcher(SQAWatcher* obj)
{
  delete obj;
}

SQAPublisher* plugin_createPublisher(
  const char* applicationId, // Unique application ID that will identify this watcher to SQA
  const char* serviceAddress, // The IP address of the SQA
  const char* servicePort, // The port where SQA is listening for connections
  int poolSize // Number of active connections to SQA
)
{
  return new SQAPublisher(applicationId, serviceAddress, servicePort, poolSize, SQA_CONN_READ_TIMEOUT, SQA_CONN_WRITE_TIMEOUT);
}

void plugin_destroyPublisher(SQAPublisher* obj)
{
  delete obj;
}

bool plugin_doPublish(SQAPublisher* obj, const char* id, const char* data)
{
  assert(obj);
  return obj->publish(id, data, false);
}

SQAWorker* plugin_createWorker(
  const char* applicationId, // Unique application ID that will identify this watcher to SQA
  const char* serviceAddress, // The IP address of the SQA
  const char* servicePort, // The port where SQA is listening for connections
  const char* eventId, // Event ID of the event being watched. Example: "sqa.not"
  int poolSize // Number of active connections to SQA
)
{
  return new SQAWorker(applicationId, serviceAddress, servicePort, eventId, poolSize, SQA_CONN_READ_TIMEOUT, SQA_CONN_WRITE_TIMEOUT);
}

void plugin_destroyWorker(SQAWorker* obj)
{
  delete obj;
}

SQADealer* plugin_createDealer(
  const char* applicationId, // Unique application ID that will identify this watcher to SQA
  const char* serviceAddress, // The IP address of the SQA
  const char* servicePort, // The port where SQA is listening for connections
  const char* eventId, // Event ID of the event being watched. Example: "sqa.not"
  int poolSize // Number of active connections to SQA
)
{
  return new SQADealer(applicationId, serviceAddress, servicePort, eventId, poolSize, SQA_CONN_READ_TIMEOUT, SQA_CONN_WRITE_TIMEOUT);
}

void plugin_destroyDealer(SQADealer* obj)
{
  delete obj;
}

bool plugin_doDeal(SQADealer* obj, const char* data, int expires)
{
  assert(obj);
  return obj->deal(data, expires);
}

}; /// extern "C"
