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

#ifndef OSPLUGIN_H_INCLUDED
#define	OSPLUGIN_H_INCLUDED


#include <dlfcn.h>
#include <string>
#include <vector>
#include <sstream>


//
// This is the only header file that plugins would need to include
// to be able to comply to the plug-in framework. There is no need
// for the plug-ins to link to sipXportLib
//


//
// Base class for the plug-in factory
//
template <typename T>
class OsPlugin
{
public:
  OsPlugin()
  {
    _pluginName = "OsPlugin";
    _priority = 0;
  }
  virtual ~OsPlugin(){}
  virtual T* createInstance() = 0;
  const std::string& getPluginName() const{ return _pluginName; };
  const std::string& getPluginSubType() const{ return _pluginSubType; };
  int getPriority(){ return _priority; };
protected:
  std::string _pluginName;
  std::string _pluginSubType;
  int _priority;
};

//
// The application plug-in
//
class OsApplicationPluginBase
{
public:
  OsApplicationPluginBase(){}
  virtual ~OsApplicationPluginBase(){}
  virtual bool run(int argc, char** argv) = 0;
};
typedef OsPlugin<OsApplicationPluginBase> OsApplicationPlugin;
typedef std::vector<OsApplicationPlugin*> OsApplicationPluginVector;

class OsPluginContainer
{
public:
  typedef void(*FuncLog)(int, const char*);

  OsPluginContainer(){}
  ~OsPluginContainer()
  {
  }
  void registerApplicationPlugin(OsApplicationPlugin* pPlugin)
  {
    _applicationPlugins.push_back(pPlugin);
  }

  void cleanUp()
  {
    for (OsApplicationPluginVector::iterator iter = _applicationPlugins.begin();
      iter != _applicationPlugins.end(); iter++)
    {
      delete *iter;
    }
  }

  OsApplicationPluginVector& applications(){return _applicationPlugins; }

public:
  static FuncLog logDebug;
  static FuncLog logInfo;
  static FuncLog logWarning;
  static FuncLog logError;
  static FuncLog logNotice;
  static FuncLog logCritical;

private:
  OsApplicationPluginVector _applicationPlugins;
};

#define PLUGIN_LOG_DEBUG(facility, data) \
{ \
  std::ostringstream strm; \
  strm << data; \
  OsPluginContainer::logDebug(strm.str().c_str()); \
}

#define PLUGIN_LOG_INFO(facility, data) \
{ \
  std::ostringstream strm; \
  strm << data; \
  OsPluginContainer::logInfo(strm.str().c_str()); \
}

#define PLUGIN_LOG_NOTICE(facility, data) \
{ \
  std::ostringstream strm; \
  strm << data; \
  OsPluginContainer::logNotice(strm.str().c_str()); \
}

#define PLUGIN_LOG_ERROR(facility, data) \
{ \
  std::ostringstream strm; \
  strm << data; \
  OsPluginContainer::logError(facility, strm.str().c_str()); \
}

#define PLUGIN_LOG_WARNING(facility, data) \
{ \
  std::ostringstream strm; \
  strm << data; \
  OsPluginContainer::logWarning(facility, strm.str().c_str()); \
}

#define PLUGIN_LOG_CRITICAL(facility, data) \
{ \
  std::ostringstream strm; \
  strm << data; \
  OsPluginContainer::logCritical(facility, strm.str().c_str()); \
}

#endif /// OSPLUGIN_H_INCLUDED

