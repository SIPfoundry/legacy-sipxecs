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
#include <string>
#include <vector>
#include <os/OsPlugin.h>


class OsPluginLoader
{
public:
  OsPluginLoader();

  ~OsPluginLoader();

  bool loadPlugin(const std::string& path);

  void unloadPlugin();

  bool isPluginLoaded() const;

  void* findPluginSymbol(const std::string& name);

  const std::string& getPath() const;

  OsPluginContainer& plugins();

protected:
  OsPluginContainer _plugins;

private:
  typedef bool (*FuncInitializePlugin)(OsPluginContainer*);
  std::string _path;
  void* _handle;
};

class OsPluginManager
{
public:
  OsPluginManager();

  ~OsPluginManager();

  bool loadApplicationPlugin(const std::string& fileName);

private:
  OsPluginLoader _applicationLoader;
};

#endif	/// OSPLUGINMANAGER_H_INCLUDED

