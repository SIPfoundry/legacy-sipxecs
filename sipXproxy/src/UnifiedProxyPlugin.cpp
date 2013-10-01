/**
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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
 * 
 */


#include <sipxproxy/UnifiedProxyPlugin.h>
#include <sipxproxy/SipRouter.h>
#include <boost/filesystem.hpp>

#define SIPX_DEFAULT_UNIFIED_PLUGIN_DIR SIPX_LIBDIR "/unifiedplugins"

//
// std::list::sort comparison callbacks
//
// Binary predicate that, taking two values of the same type of those contained in the list, returns true if the first argument goes before the second argument in the strict weak ordering it defines, and false otherwise.
// This shall be a function pointer or a function object.
//
bool compare_authenticator_priority(const UnifiedProxyPlugin* first, const UnifiedProxyPlugin* second)
{
  return first->getAuthPriority() < second->getAuthPriority();
}

bool compare_io_priority(const UnifiedProxyPlugin* first, const UnifiedProxyPlugin* second)
{
  return first->getIoPriority() < second->getIoPriority();
}

bool compare_forward_priority(const UnifiedProxyPlugin* first, const UnifiedProxyPlugin* second)
{
  return first->getForwardPriority() < second->getForwardPriority();
}

//
// This function evaluates a string using reverse find if it ends
// with the string specified in "key"
//
bool string_ends_with(const std::string& str, const char* key)
{
  size_t i = str.rfind(key);
  return (i != std::string::npos) && (i == (str.length() - ::strlen(key)));
}

//
// Boost compatibility function to support old and new version of the 
// file system API
// 
std::string boost_file_name(const boost::filesystem::path& path)
{
  std::ostringstream fileName;
  fileName << path.filename();
  return fileName.str();
}

//
// Boost compatibility function to support old and new version of the 
// file system API
// 
std::string boost_path(const boost::filesystem::path& path)
{
  std::ostringstream pathStr;
  pathStr << path;
  return pathStr.str();
}


//
// This function iterates through a directory and find all files ending with the 
// .so extension. This function is not recursive.  It will not process files
// within a subdirectory.  Each file found with the .so extension is returned 
// as an element in the string vector return value.
//
std::vector<std::string> find_shared_libs(const std::string& plugindirectory)
{
  boost::filesystem::path directory = plugindirectory;
  std::vector<std::string> plugins;
  
  std::string data;
  if (boost::filesystem::exists(directory))
  {
    boost::filesystem::directory_iterator end_itr; // default construction yields past-the-end
    for (boost::filesystem::directory_iterator itr(directory); itr != end_itr; ++itr)
    {
      if (boost::filesystem::is_directory(itr->status()))
      {
        continue;
      }
      else
      {
        boost::filesystem::path currentFile = itr->path();
        std::string fileName = boost_file_name(currentFile);
        if (boost::filesystem::is_regular(currentFile))
        {
          if (string_ends_with(fileName, ".so"))
          {
            plugins.push_back(boost_path(currentFile));
          }
        }
      }
    }
  }
  return  plugins;
}


////////////////////////////////////////////////////////////////////////////////


UnifiedProxyPlugin::UnifiedProxyPlugin(
  const std::string& pluginName,
  int authPriority,
  int ioPriority,
  int forwardPriority) :
  _enabled(true),
  _authPriority(authPriority),
  _ioPriority(ioPriority),
  _forwardPriority(forwardPriority),
  _pSipRouter(0),
  _pSipUserAgent(0),
  _pInputProcessor(0),
  _pOutputProcessor(0)
{
  _pluginName = pluginName;
}

 void UnifiedProxyPlugin::loadIoProcessors()
 {
   if (_pSipUserAgent && !_pInputProcessor && !_pOutputProcessor)
   {
     _pInputProcessor = new InputProcessor(_ioPriority);
     _pOutputProcessor = new OutputProcessor(_ioPriority);
     _pInputProcessor->_handler = _inbound;
     _pOutputProcessor->_handler = _outbound;
     _pSipUserAgent->addSipInputProcessor(_pInputProcessor);
     _pSipUserAgent->addSipOutputProcessor(_pOutputProcessor);
   }
 }

////////////////////////////////////////////////////////////////////////////////


UnifiedProxyPluginLoader::UnifiedProxyPluginLoader()
{
}
  
int UnifiedProxyPluginLoader::loadUnifiedProxyPlugins(SipRouter* pSipRouter, SipUserAgent* pSipUserAgent, const std::string& dir)
{
  //
  // Use default directory if dir is empty
  //
  std::string directory = SIPX_DEFAULT_UNIFIED_PLUGIN_DIR;
  if (!dir.empty())
    directory = dir;
  //
  // Get all files with the .so extension
  //
  std::vector<std::string> sharedLibs = find_shared_libs(directory);
  
  if (sharedLibs.empty())
    return 0;
  
  //
  // iterate through the sharedLibs vector and load the plug-ins  one by one.
  //
  for (std::vector<std::string>::const_iterator iter = sharedLibs.begin(); iter != sharedLibs.end(); iter++)
  {
    loadOneUnifiedProxyPlugin(pSipRouter, pSipUserAgent, *iter);
  }
  
  return plugins().applications().size();
}

UnifiedProxyPlugin* UnifiedProxyPluginLoader::loadOneUnifiedProxyPlugin(SipRouter* pSipRouter, SipUserAgent* pSipUserAgent, const std::string& path)
{
  UnifiedProxyPlugin* pPlugin = dynamic_cast<UnifiedProxyPlugin*>(loadPlugin(path));
  if (!pPlugin)
    return 0;
  
  if (pPlugin->_enabled)
  { 
    //
    // Set the associate SIP Router
    //
    pPlugin->setSipRouter(pSipRouter);

    //
    // Set the associated User Agent
    //
    pPlugin->setSipUserAgent(pSipUserAgent);

    //
    // Call initialize call-back if it is set by the plug-in
    //
    if (pPlugin->_initialize)
      pPlugin->_initialize();

    //
    // Load authenticators
    //
    if (pPlugin->getAuthPriority() > 0)
    {
      //
      // Check if the authenticator functor has been registered by the plug-in
      //
      if (pPlugin->_authorize)
        _authenticators.push_back(pPlugin);

    }

    //
    // Load I/O processors
    //
    if (pPlugin->getIoPriority() > 0)
    {
      //
      // Check if both inbound or outbound functors are registered
      //
      if (pPlugin->_inbound || pPlugin->_outbound)
      {
        pPlugin->loadIoProcessors();
        _ioProcessors.push_back(pPlugin);
      }
    }

    //
    // Load forwarders
    //
    if (pPlugin->getForwardPriority() > 0)
    {
      //
      // Check if the forward functor is registered
      //
      if (pPlugin->_forward)
        _forwarders.push_back(pPlugin);
    }
  }
  
  //
  // Sort the plug-ins according to priority
  // 
  _authenticators.sort(compare_authenticator_priority);
  _ioProcessors.sort(compare_io_priority);
  _forwarders.sort(compare_forward_priority);
  
  return pPlugin;
}


UnifiedProxyPluginLoader::AuthResult UnifiedProxyPluginLoader::authorizeAndModify(
  const UtlString& id, const Url&  requestUri, RouteState& routeState,  
  const UtlString& method, AuthResult priorResult, SipMessage& request, 
  bool bSpiralingRequest, UtlString&  reason 
)
{
  AuthResult result = priorResult;
  AuthResult decision = priorResult;
  
  for (UnifiedProxyPluginList::iterator iter = _authenticators.begin(); iter != _authenticators.end(); iter++)
  {
    result = (*iter)->_authorize(id,
                              requestUri,
                              routeState,
                              method,
                              priorResult,
                              request,
                              bSpiralingRequest,                                                          
                              reason);
    
    // the first plugin to return something other than CONTINUE wins
    if (AuthPlugin::CONTINUE != result && priorResult == AuthPlugin::CONTINUE )
    {
      decision = result;
      priorResult = result;
    }
  }
  
  return decision;
  
}

void UnifiedProxyPluginLoader::forwardRequest(
    const Url& requestUri, /// The normalized request-uri of the SIP Message
    const SipMessage& request /// The outbound request
)
{
  for (UnifiedProxyPluginList::iterator iter = _forwarders.begin(); iter != _forwarders.end(); iter++)
  {
    (*iter)->_forward(requestUri, request);
  } 
}



