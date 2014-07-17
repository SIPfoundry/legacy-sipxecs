
/*
 * Copyright (c) eZuce, Inc. All rights reserved.
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


#include <dlfcn.h>
#include "sipxyard/YardPlugin.h"
#include "os/OsLogger.h"


YardPlugin::YardPlugin() :
  _handle(0)
{
}

YardPlugin::~YardPlugin()
{
  unloadPlugin();
}

YardPlugin::YardProcessorInstance YardPlugin::createInstance()
{
  if (!_handle)
    return 0;
  
  YardInstanceLoader loader = (YardInstanceLoader)(::dlsym(_handle, _loaderFunc.c_str()));
  if (!loader)
    return 0;
  
  YardPlugin::YardProcessorInstance pInstance = loader();
  return pInstance;
}

bool YardPlugin::loadPlugin(const std::string& path, const std::string& instanceLoader)
{
  if (_handle)
  {
    OS_LOG_ERROR(FAC_PROCESS, path << " is already loaded!");
    return false;
  }

  _handle = ::dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
  _path = path;
  _loaderFunc = instanceLoader;
  if (_handle)
  {
    void* pLoaderFunc =  ::dlsym(_handle, _loaderFunc.c_str());
    if (pLoaderFunc != 0)
    {
      OS_LOG_INFO(FAC_PROCESS,  "YARD Plugin LOADED - " << path);
    }
    else
    {
      unloadPlugin();
      OS_LOG_ERROR(FAC_PROCESS, path << " doesn't have an initialize method exported.");
      return false;
    }
  }
  return true;
}

void YardPlugin::unloadPlugin()
{
  if (_handle)
  {
    ::dlclose(_handle);
    _handle = 0;
  }
}

bool YardPlugin::isPluginLoaded() const
{
  return _handle != 0;
}

  

