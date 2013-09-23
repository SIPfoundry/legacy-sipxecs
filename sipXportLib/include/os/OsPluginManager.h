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

#ifndef OSPLUGINMANAGER_H_INCLUDED
#define	OSPLUGINMANAGER_H_INCLUDED

#include <dlfcn.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <set>
#include <os/OsPlugin.h>


class OsPluginManager
{
public:
  OsPluginManager();

  ~OsPluginManager();

  OsApplicationPlugin* loadPlugin(const std::string& path);

  void unloadPlugin(intptr_t handle);


  void* findPluginSymbol(intptr_t handle, const std::string& name);

  OsPluginContainer& plugins();

protected:
  OsPluginContainer _plugins;
  typedef std::set<intptr_t> Handles;
  Handles _handles;

private:
  typedef OsApplicationPlugin* (*FuncInitializePlugin)();
};

#define EXPORT_APP_PLUGIN(plugin) \
extern "C" OsApplicationPlugin* initializePlugin() \
{ \
  return dynamic_cast<OsApplicationPlugin*>(plugin); \
} 

#endif	/// OSPLUGINMANAGER_H_INCLUDED

