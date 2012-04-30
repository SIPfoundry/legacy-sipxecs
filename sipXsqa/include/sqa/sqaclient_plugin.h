

#ifndef SQACLIENT_PLUGIN_H
#define	SQACLIENT_PLUGIN_H

#ifndef PLUGIN_LOADER
#define EXCLUDE_SQA_INLINES 1
#endif
#include "sqaclient.h"
#include <dlfcn.h>

class SQAClientPlugin
{
public:
  SQAClientPlugin(const char* pluginPath) :
    _handle(0)     
  {
    _handle = ::dlopen(pluginPath, RTLD_LAZY | RTLD_GLOBAL);
  }

  ~SQAClientPlugin()
  {
    if (_handle)
    {
      ::dlclose(_handle);
      _handle = 0;
    }
  }

  typedef SQAWatcher* (*PluginCreateWatcher)(
    const char* , // Unique application ID that will identify this watcher to SQA
    const char* , // The IP address of the SQA
    const char* , // The port where SQA is listening for connections
    const char* , // Event ID of the event being watched. Example: "sqa.not"
    int  // Number of active connections to SQA
  );
  
  SQAWatcher* createWatcher(
    const char* applicationId, // Unique application ID that will identify this watcher to SQA
    const char* serviceAddress, // The IP address of the SQA
    const char* servicePort, // The port where SQA is listening for connections
    const char* eventId, // Event ID of the event being watched. Example: "sqa.not"
    int poolSize // Number of active connections to SQA
  )
  {
    PluginCreateWatcher plugin_createWatcher = (PluginCreateWatcher)findPluginSymbol("plugin_createWatcher");
    if (!plugin_createWatcher)
      return 0;
    return plugin_createWatcher(applicationId, serviceAddress, servicePort, eventId, poolSize);
  }

  typedef void* (*PluginDestroyWatcher)(SQAWatcher*);
  void destroyWatcher(SQAWatcher* obj)
  {
    PluginDestroyWatcher plugin_destroyWatcher = (PluginDestroyWatcher)findPluginSymbol("plugin_destroyWatcher");
    if (!plugin_destroyWatcher)
      return;
    plugin_destroyWatcher(obj);
  }

  typedef SQAPublisher* (*PluginCreatePublisher)(
    const char* , // Unique application ID that will identify this watcher to SQA
    const char* , // The IP address of the SQA
    const char* , // The port where SQA is listening for connections
    int  // Number of active connections to SQA
  );

  SQAPublisher* createPublisher(
    const char* applicationId, // Unique application ID that will identify this watcher to SQA
    const char* serviceAddress, // The IP address of the SQA
    const char* servicePort, // The port where SQA is listening for connections
    int poolSize // Number of active connections to SQA
  )
  {
    PluginCreatePublisher plugin_createPublisher = (PluginCreatePublisher)findPluginSymbol("plugin_createPublisher");
    if (!plugin_createPublisher)
      return 0;
    return plugin_createPublisher(applicationId, serviceAddress, servicePort, poolSize);
  }

  typedef void* (*PluginDestroyPublisher)(SQAPublisher*);
  void destroyPublisher(SQAPublisher* obj)
  {
    PluginDestroyPublisher plugin_destroyPublisher = (PluginDestroyPublisher)findPluginSymbol("plugin_destroyPublisher");
    if (!plugin_destroyPublisher)
      return;
    plugin_destroyPublisher(obj);
  }

  typedef bool* (*PluginDoPublish)(SQAPublisher*, const char*, const char*);
  bool publish(SQAPublisher* obj, const char* name, const char* data)
  {
    PluginDoPublish plugin_doPublish = (PluginDoPublish)findPluginSymbol("plugin_doPublish");
    if (!plugin_doPublish)
      return false;
    return plugin_doPublish(obj, name, data);
  }

  typedef SQAWorker* (*PluginCreateWorker)(
    const char* , // Unique application ID that will identify this watcher to SQA
    const char* , // The IP address of the SQA
    const char* , // The port where SQA is listening for connections
    const char* , // Event ID of the event being watched. Example: "sqa.not"
    int  // Number of active connections to SQA
  );

  SQAWorker* createWorker(
    const char* applicationId, // Unique application ID that will identify this watcher to SQA
    const char* serviceAddress, // The IP address of the SQA
    const char* servicePort, // The port where SQA is listening for connections
    const char* eventId, // Event ID of the event being watched. Example: "sqa.not"
    int poolSize // Number of active connections to SQA
  )
  {
    PluginCreateWorker plugin_createWorker = (PluginCreateWorker)findPluginSymbol("plugin_createWorker");
    if (!plugin_createWorker)
      return 0;
    return plugin_createWorker(applicationId, serviceAddress, servicePort, eventId, poolSize);
  }

  typedef void* (*PluginDestroyWorker)(SQAWorker*);
  void destroyWorker(SQAWorker* obj)
  {
    PluginDestroyWorker plugin_destroyWorker = (PluginDestroyWorker)findPluginSymbol("plugin_destroyWorker");
    if (!plugin_destroyWorker)
      return;
    plugin_destroyWorker(obj);
  }

  typedef SQADealer* (*PluginCreateDealer)(
    const char* , // Unique application ID that will identify this watcher to SQA
    const char* , // The IP address of the SQA
    const char* , // The port where SQA is listening for connections
    const char* , // Event ID of the event being watched. Example: "sqa.not"
    int  // Number of active connections to SQA
  );

  SQADealer* createDealer(
    const char* applicationId, // Unique application ID that will identify this watcher to SQA
    const char* serviceAddress, // The IP address of the SQA
    const char* servicePort, // The port where SQA is listening for connections
    const char* eventId, // Event ID of the event being watched. Example: "sqa.not"
    int poolSize // Number of active connections to SQA
  )
  {
    PluginCreateDealer plugin_createDealer = (PluginCreateDealer)findPluginSymbol("plugin_createDealer");
    if (!plugin_createDealer)
    {
      return 0;
    }
    return plugin_createDealer(applicationId, serviceAddress, servicePort, eventId, poolSize);
  }
  
  typedef void* (*PluginDestroyDealer)(SQADealer*);
  void destroyDealer(SQADealer* obj)
  {
    PluginDestroyDealer plugin_destroyDealer = (PluginDestroyDealer)findPluginSymbol("plugin_destroyDealer");
    if (!plugin_destroyDealer)
      return;
    plugin_destroyDealer(obj);
  }

  typedef bool* (*PluginDoDeal)(SQADealer*, const char*, int);
  bool deal(SQADealer* obj, const char* data, int expires)
  {
    PluginDoDeal plugin_doDeal = (PluginDoDeal)findPluginSymbol("plugin_doDeal");
    if (!plugin_doDeal)
      return false;
    return plugin_doDeal(obj, data, expires);
  }

private:
  void* findPluginSymbol(const char* name)
  {
    void* result = 0;
    if (_handle)
    {
      result = ::dlsym(_handle, name);
    }
    return result;
  }
  void* _handle;
};



#endif	/* SQACLIENT_PLUGIN_H */

