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



OsPluginLoader::OsPluginLoader()
{
  _handle = 0;
}

OsPluginLoader::~OsPluginLoader()
{
  
  _plugins.cleanUp();
  unloadPlugin();
}

bool OsPluginLoader::loadPlugin(const std::string& path)
{
  if (_handle)
  {
    OS_LOG_ERROR(FAC_PROCESS, path << " is already loaded!");
    return false;
  }

  _handle = ::dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
  _path = path;
  if (_handle)
  {
    FuncInitializePlugin initializePlugin = (FuncInitializePlugin)findPluginSymbol("initializePlugin");
    if (initializePlugin != 0)
    {
      OS_LOG_INFO(FAC_PROCESS, "Calling initialize for " << path);
      return initializePlugin(&_plugins);
    }
    else
    {
      unloadPlugin();
      OS_LOG_ERROR(FAC_PROCESS, path << " doesn't have an initialize method exported.");
      return false;
    }
  }
  return false;
}

void OsPluginLoader::unloadPlugin()
{
  if (_handle)
  {
    ::dlclose(_handle);
    _handle = 0;
  }
}

bool OsPluginLoader::isPluginLoaded() const
{
  return _handle != 0;
}

void* OsPluginLoader::findPluginSymbol(const std::string& name)
{
  void* result = 0;
  if (_handle)
  {
    result = ::dlsym(_handle, name.c_str());
  }
  return result;
}

const std::string& OsPluginLoader::getPath() const
{
  return _path;
}

OsPluginContainer& OsPluginLoader::plugins()
{
  return _plugins;
}

//
// The plug-in manager
//
OsPluginManager::OsPluginManager()
{
}

OsPluginManager::~OsPluginManager()
{
}

 
bool OsPluginManager::loadApplicationPlugin(const std::string& plugin)
{
  OS_LOG_INFO(FAC_PROCESS, "Loading application plug-in " << plugin)
  if (!_applicationLoader.loadPlugin(plugin))
  {
    OS_LOG_ERROR(FAC_PROCESS, "Unable to load application plug-in " << plugin);
    return false;
  }

  if (_applicationLoader.plugins().applications().size() == 0)
  {
    OS_LOG_ERROR(FAC_PROCESS, "Unable to load application plug-in. Shared file " << plugin << " registered zero application.");
    return false;
  }
  
  return true;
}














