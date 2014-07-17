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

#ifndef YARDPLUGIN_H_INCLUDED
#define	YARDPLUGIN_H_INCLUDED

#include "sipxyard/YardProcessor.h"

#define YARD_INSTANCE_LOADER_SYMBOL "create_yard_instance"

class YardPlugin
{
public:
  typedef YardProcessor* YardProcessorInstance;
  typedef YardProcessorInstance (*YardInstanceLoader)(void);
  
  YardPlugin();
  
  ~YardPlugin();
  
  bool loadPlugin(const std::string& path, const std::string& instanceLoader = YARD_INSTANCE_LOADER_SYMBOL);
  
  void unloadPlugin();

  bool isPluginLoaded() const;
  
  const std::string& getPath() const;
  
  YardProcessorInstance createInstance();
private:
  std::string _path;
  std::string _loaderFunc;
  void* _handle;
};

//
// Inlines
//

inline const std::string& YardPlugin::getPath() const
{
  return _path;
}

#endif	/* YARDPLUGIN_H */

