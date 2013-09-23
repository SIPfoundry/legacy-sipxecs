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


#ifndef UNIFIEDPROXYPLUGIN_H_INCLUDED
#define	UNIFIEDPROXYPLUGIN_H_INCLUDED


#include <os/OsPlugin.h>
#include <os/OsPluginManager.h>
#include <os/OsLogger.h>
#include <boost/function.hpp>
#include <sipxproxy/AuthPlugin.h>
#include <sipxproxy/RouteState.h>
#include <net/Url.h>
#include <net/SipMessage.h>
#include <net/SipBidirectionalProcessorPlugin.h>
#include <list>

//
//  Design Motivation:  Unified Proxy Plugin Mechanism
//
//  This new plug-in will unify authenticators and transaction plug-ins into one 
//  interface.  The need arises when a plug-in requires to have a handle on 
//  both protocol layer and application layer states at the same time.
//
//  Authentication:  This plug-in would allow custom code to participate 
//  in authentication.  It will be called after all the internal proxy 
//  authenticators has been evaluated.  In most cases this authenticator will 
//  not participate on actually authenticating the call but rather to have a 
//  handle on the transaction and be able to access application layer state 
//  accordingly based on internal proxy configuration such as 
//  MongoDB if the need arises.
//
//  I/O Processor:  The new plug-in would have a handle on all incoming and 
//  outgoing messages.  This would allow the plug-in to correct any potential 
//  interoperability issue in the SIP Message prior to processing as well as 
//  prior to propagating to remote endpoints.  Protocol correction is the 
//  primary role of this functionality.  Example of I/O processor is the 
//  proxy plug-in that generates events for sipXhomer.
//
//  Forwarder:  As of the moment, there is no plug-in that can enforce a 
//  forwarding rule in the proxy.  Forwarding rules are enforced using and XML 
//  file with a limited set of configurable behavior based on a predefined set 
//  of parameters.  This is limiting our capability to add more complex rules 
//  based on custom parameters that are not supported by the forwarding rule 
//  schema.  The new plug-in type will have an additional function that can 
//  override the forwarding rules after they have been evaluated by the proxy.  
//  This would mean the custom forwarding rule can bypass the target returned 
//  by the internal forwarding rule.  As an example, this functionality is 
//  needed to redirect a particular request towards a WS destination to always 
//  traverse through the WebSocket proxy.  In all other cases, it would 
//  silently ignore anything that is not relevant to the plug-in's operation.
//
//  Directory Based/Zero Configuration:  Plug-in of this (new) type  will not
//  require any configuration entry in the proxy ini for it to be loaded.  
//  Instead, there will be a predefined folder where plug-ins of this type will 
//  reside.  All files in this folder ending with .so will be evaluated.  
//  If the .so contains the required function signature, it will be registered 
//  and loaded as a plug-in by the proxy process.  Plugin of this new type will 
//  have a priority number.  The priority number will serve as its priority 
//  index just in case there are multiple plug-ins defined.  The priority will be 
//  sorted from lowest to highest.  The plug-in with the lowest value will be 
//  called first.  If there are more than one plug-in trying to register the 
//  same priority value, it will be treated as an error condition and must cause 
//  the proxy to exit.
//
//  Functionality Opt-out mechanism:  There will be instances when a plug-in 
//  only needs two of the three exposed callbacks.  For example, a certain 
//  plug-in may only be interested in forwarding and I/O processing but does not 
//  need to participate in authentication.  So that extra processing is not 
//  incurred, a plug-in that does not require certain callback to be processed 
//  shall not be called within the iteration for such callback.  In the previous 
//  case, such plug-in will not be registered as an authenticator but only 
//  registered as an I/O processor and a forwarder.
//

#define SIPX_DEFAULT_UNIFIED_PLUGIN_DIR SIPX_LIBDIR "/unifiedplugins"

class SipRouter;

class UnifiedProxyPlugin : public OsApplicationPlugin
{
  public:
    typedef AuthPlugin::AuthResult AuthResult;
    
    static const AuthResult CONTINUE = AuthPlugin::CONTINUE;
    /// plugin neither authorizes nor forbids
    /// (but may have modified) this message
    
    static const AuthResult DENY = AuthPlugin::DENY;
    /// this request is not authorized - do not proxy
    
    static const AuthResult ALLOW = AuthPlugin::ALLOW;
    /// this request is authorized - proxy the message (possibly modified)
    
    typedef boost::function< bool() > FuncInitialize;
    
    typedef boost::function< AuthResult(
      const UtlString& /* id */,      /**  The authenticated identity of the
                                        *  request originator, if any
                                        *  (the null string if not).
                                        *  This is in the form of a SIP uri
                                        *  identity value as used in the
                                        *  credentials database (user@domain)
                                        *  without the scheme or any
                                        *  parameters.
                                        */
        const Url&  /* requestUri */,  /// parsed target Uri
        RouteState& /* routeState */,  /// the state for this request.  
        const UtlString& /* method */, /// the request method
        AuthResult  /* priorResult */, /// results from earlier plugins.
        SipMessage& /* request */,     /// see below regarding modifying this
        bool /* bSpiralingRequest */,  /// true if request is still spiraling through proxy
                                 /// false if request is ready to be sent to target
        UtlString&  /* reason */       /// rejection reason
    )> FuncAuthorizeAndModify;
    
    typedef boost::function< void(
        SipMessage& /* message */, /// Incoming or outgoing SIP Message
        const char* /* address */, /// Remote IP address of the remote target
        int /* port */,             /// Port of the remote target
        bool /* buffered */ /// true if the read or write operation is buffered
    )> FuncMessageHandler;
    
    typedef boost::function< void(
      const Url& /* requestUri */, /// The normalized request-uri of the SIP Message
      const SipMessage& /* request */ /// The outbound request
    )> FuncForwarder;
    
    class InputProcessor : public SipInputProcessor
    {
    public:
      InputProcessor(int priority);
      virtual ~InputProcessor();
      void handleOutputMessage(SipMessage& message, const char* address, int port);
      void setEnabled(bool enabled);
    private:
      bool _enabled;
      FuncMessageHandler _handler;
      friend class UnifiedProxyPlugin;
    };

    class OutputProcessor : public SipOutputProcessor
    {
    public:
      OutputProcessor(int priority);
      virtual  ~OutputProcessor();
      void handleOutputMessage(SipMessage& message, const char* address, int port);
      void handleBufferedOutputMessage(SipMessage& message, const char* address, int port);
      void setEnabled(bool enabled);
    private:
      bool _enabled;
      FuncMessageHandler _handler;
      friend class UnifiedProxyPlugin;
    };
    
    int getAuthPriority() const;
    /// Returns the authenticator priority
    ///
    
    int getIoPriority() const;
    /// Returns the I/O priority
    ///
    
    int getForwardPriority() const;
    /// Returns the forwarder priority
    ///
    
protected:
    void setSipRouter(SipRouter* pSipRouter);
    /// Set the associated SIP Router for this plug-in
    ///
    
    void setSipUserAgent(SipUserAgent* pSipUserAgent);
    /// Set the associated user-agent
    ///
    
    void loadIoProcessors();
    /// Called by the manager to instruct the plug-in to load the I/O call-backs
    
  protected:
    UnifiedProxyPlugin(const std::string& pluginName,
      int authPriority,
      int ioPriority,
      int forwardPriority);
    /// Constructor can only be called within UnifiedProxyPluginLoader
    ///
    
    FuncInitialize _initialize;
    FuncAuthorizeAndModify _authorize;
    FuncMessageHandler _inbound;
    FuncMessageHandler _outbound;
    FuncForwarder _forward;
    
    bool _enabled;
    /// A plug-in is enabled by default.  If a plug-in needs to temporary exclude
    /// itself from being loaded by the UnifiedProxyPluginLoader, this variable
    /// may be explicitly set to false.
    ///
    
    int _authPriority;
    /// The authenticator priority
    ///
    
    int _ioPriority;
    /// The inbound and outbound priority
    ///
    
    int _forwardPriority;
    /// The forwarder priority
    ///
    
    SipRouter* _pSipRouter;
    /// Pointer to the associated SIP Router
    
    SipUserAgent* _pSipUserAgent;
    /// Pointer to the associated SIP User Agent  
    
    InputProcessor* _pInputProcessor;
    /// Input processor to be registered with the SIP User Agent
    
    OutputProcessor* _pOutputProcessor;
    /// Input processor to be registered with the SIP User Agent
    
    friend class UnifiedProxyPluginLoader;
    /// always provide access to UnifiedProxyPluginLoader
    ///
};

class UnifiedProxyPluginLoader : protected OsPluginManager
{
  /// This class is used by the application layer to load plug-ins from a specific
  /// directory.  All files with the ".so" extension will be validated if they 
  /// export "initializePlugin" global function and will be loaded as a plug-in.
  /// This is an improvement to the current plug-in architecture where proxy
  /// authenticators are loaded using a preconfigured set of paths per plug-in. 
  ///
  
public:
  typedef UnifiedProxyPlugin::AuthResult AuthResult;
  typedef std::list<UnifiedProxyPlugin*> UnifiedProxyPluginList;
  
  UnifiedProxyPluginLoader();
  /// Creates a new plug-in loader instance
  ///
  
  int loadUnifiedProxyPlugins(SipRouter* pSipRouter, SipUserAgent* pSipUserAgent, const std::string& directory = std::string());
  ///
  /// Load the plug-ins located inside "directory".  If directory is not set, the default
  /// plugin directory will be used $SIPX_DEFAULT_UNIFIED_PLUGIN_DIR.  Valid plug-ins are those 
  /// libraries that export the "bool initializePlugin(OsPluginContainer*)" and at least assigned
  /// a binder for one of the optional functors supported by UnifiedProxyPlugin
  /// class.  Plugins can temporarily disable themselves, and thus, preventing
  /// their instance to be loaded by explicitly setting the enabled() property
  /// to false.  This function returns the number of valid plug-ins that are 
  /// successfully loaded.
  ///
  
  UnifiedProxyPlugin* loadOneUnifiedProxyPlugin(SipRouter* pSipRouter, SipUserAgent* pSipUserAgent, const std::string& path);
  /// Loads a single plug-in.  Same function as loadUnifiedProxyPlugins but 
  /// loads a single file instead of iterating through a directory.
  /// Returns a pointer to the plug-in if successful, NULL otherwise.
  ///
  
  AuthResult authorizeAndModify(
    const UtlString& id,      /**  The authenticated identity of the
                                          *  request originator, if any
                                          *  (the null string if not).
                                          *  This is in the form of a SIP uri
                                          *  identity value as used in the
                                          *  credentials database (user@domain)
                                          *  without the scheme or any
                                          *  parameters.
                                          */
    const Url&  requestUri,  /// parsed target Uri
    RouteState& routeState,  /// the state for this request.  
    const UtlString& method, /// the request method
    AuthResult  priorResult, /// results from earlier plugins.
    SipMessage& request,     /// see below regarding modifying this
    bool bSpiralingRequest,  /// true if request is still spiraling through proxy
                             /// false if request is ready to be sent to target
    UtlString&  reason       /// rejection reason
  );
  
  void forwardRequest(
      const Url& requestUri, /// The normalized request-uri of the SIP Message
      const SipMessage& request /// The outbound request
  );
  /// This function is called after the internal forwarding rules has been evaluated.
  /// This would allow registered plug-ins to add custom forwarding rule on
  /// top of the sipx internal rules.
    
  UnifiedProxyPluginList& authenticators();
  /// Returns a reference to the registered authenticators
  ///
  
  UnifiedProxyPluginList& ioProcessors();
  /// Returns a reference to the registered I/O processors
  ///
  
  UnifiedProxyPluginList& forwarders();
  /// Returns a reference to the registered forwarders
  ///
  
private:
  UnifiedProxyPluginList _authenticators;
  UnifiedProxyPluginList _ioProcessors;
  UnifiedProxyPluginList _forwarders;
};


//
// Inlines
//

inline int UnifiedProxyPlugin::getAuthPriority() const
{
  return _authPriority;
}

inline int UnifiedProxyPlugin::getIoPriority() const
{
  return _ioPriority;
}

inline int UnifiedProxyPlugin::getForwardPriority() const
{
  return _forwardPriority;
}

inline void UnifiedProxyPlugin::setSipRouter(SipRouter* pSipRouter)
{
  _pSipRouter = pSipRouter;
}

inline void UnifiedProxyPlugin::setSipUserAgent(SipUserAgent* pSipUserAgent)
{
  _pSipUserAgent = pSipUserAgent;
}

//
// Input Processor Inlines
//
inline UnifiedProxyPlugin::InputProcessor::InputProcessor(int priority) :
  SipInputProcessor(priority),
  _enabled(true)
{
}

inline UnifiedProxyPlugin::InputProcessor::~InputProcessor()
{
}

inline void UnifiedProxyPlugin::InputProcessor::setEnabled(bool enabled)
{
  _enabled = enabled;
}

inline void UnifiedProxyPlugin::InputProcessor::handleOutputMessage(SipMessage& message, const char* address, int port)
{
  if (_enabled && _handler)_handler(message, address, port, false);
}


//
// Output Processor Inlines
//
inline UnifiedProxyPlugin::OutputProcessor::OutputProcessor(int priority) :
  SipOutputProcessor(priority),
  _enabled(true)
{
}

inline UnifiedProxyPlugin::OutputProcessor::~OutputProcessor()
{
}

inline void UnifiedProxyPlugin::OutputProcessor::setEnabled(bool enabled)
{
  _enabled = enabled;
}

inline void UnifiedProxyPlugin::OutputProcessor::handleOutputMessage(SipMessage& message, const char* address, int port)
{
  if (_enabled && _handler)_handler(message, address, port, false);
}

inline void UnifiedProxyPlugin::OutputProcessor::handleBufferedOutputMessage(SipMessage& message, const char* address, int port)
{
  if (_enabled && _handler)_handler(message, address, port, true);
}


//
// Loader inlines
//

inline UnifiedProxyPluginLoader::UnifiedProxyPluginList& UnifiedProxyPluginLoader::authenticators()
{
  return _authenticators;
}
  
inline UnifiedProxyPluginLoader::UnifiedProxyPluginList& UnifiedProxyPluginLoader::ioProcessors()
{
  return _ioProcessors;
}
  
inline UnifiedProxyPluginLoader::UnifiedProxyPluginList& UnifiedProxyPluginLoader::forwarders()
{
  return _forwarders;
}
    
#endif	// UNIFIEDPROXYPLUGIN_H_INCLUDED

