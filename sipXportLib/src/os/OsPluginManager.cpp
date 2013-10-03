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

#include "os/OsPluginManager.h"
#include "os/OsLogger.h"



void plugin_log_debug(int facility, const char* data)
{
  OS_LOG_DEBUG(facility, data);
}

void plugin_log_info(int facility, const char* data)
{
  OS_LOG_INFO(facility, data);
}

void plugin_log_notice(int facility, const char* data)
{
  OS_LOG_NOTICE(facility, data);
}

void plugin_log_warning(int facility, const char* data)
{
  OS_LOG_WARNING(facility, data);
}

void plugin_log_error(int facility, const char* data)
{
  OS_LOG_ERROR(facility, data);
}

void plugin_log_critical(int facility, const char* data)
{
  OS_LOG_CRITICAL(facility, data);
}

OsPluginContainer::FuncLog OsPluginContainer::logCritical = plugin_log_critical;
OsPluginContainer::FuncLog OsPluginContainer::logDebug = plugin_log_debug;
OsPluginContainer::FuncLog OsPluginContainer::logError = plugin_log_error;
OsPluginContainer::FuncLog OsPluginContainer::logInfo = plugin_log_info;
OsPluginContainer::FuncLog OsPluginContainer::logNotice = plugin_log_notice;
OsPluginContainer::FuncLog OsPluginContainer::logWarning = plugin_log_warning;



OsPluginManager::OsPluginManager()
{
}

OsPluginManager::~OsPluginManager()
{
  _plugins.cleanUp();
  
  for (Handles::iterator iter = _handles.begin(); iter != _handles.end(); iter++)
    unloadPlugin(*iter);
}

OsApplicationPlugin* OsPluginManager::loadPlugin(const std::string& path)
{
  void* handle = 0;
  
  handle = ::dlopen(path.c_str(), RTLD_NOW | RTLD_GLOBAL);
  
  
  if (handle)
  {
    FuncInitializePlugin initializePlugin = (FuncInitializePlugin)findPluginSymbol((intptr_t)handle, "initializePlugin");
    if (initializePlugin != 0)
    {
      OS_LOG_INFO(FAC_PROCESS, "Calling initialize for " << path);
      
      OsApplicationPlugin* pPlugin = initializePlugin();
      if (!pPlugin)
      {
        unloadPlugin((intptr_t)handle);
        OS_LOG_ERROR(FAC_PROCESS, path << " plugin initialization returned false.");
        return 0;
      }
      else
      {
        _plugins.registerApplicationPlugin(pPlugin, (intptr_t)handle);
        _handles.insert((intptr_t)handle);
        pPlugin->setHandle((intptr_t)handle);
        pPlugin->setLibraryFile(path);
        return pPlugin;
      }
    }
    else
    {
      unloadPlugin((intptr_t)handle);
      OS_LOG_ERROR(FAC_PROCESS, path << " doesn't have an initialize method exported.");
      return 0;
    }
  }
  else
  {
    OS_LOG_ERROR(FAC_PROCESS, "Unable to load " << path << " Error: " << dlerror());
  }
  
  return 0;
}

void OsPluginManager::unloadPlugin(intptr_t handle)
{
  if (handle)
  {
    ::dlclose((void*)handle);
    handle = 0;
  }
}

void* OsPluginManager::findPluginSymbol(intptr_t handle, const std::string& name)
{
  void* result = 0;
  if (handle)
  {
    result = ::dlsym((void*)handle, name.c_str());
  }
  return result;
}

OsPluginContainer& OsPluginManager::plugins()
{
  return _plugins;
}














