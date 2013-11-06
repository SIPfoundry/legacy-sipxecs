/**
 *
 * Copyright (c) eZuce, Inc. All rights reserved.
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


#ifndef REPROGLUE_H_INCLUDED
#define	REPROGLUE_H_INCLUDED


#include <map>
#include <string>

#include <boost/function.hpp>
#include <boost/bind.hpp>

#undef HAVE_CONFIG_H

#include <resip/stack/Transport.hxx>

#include <repro/ReproRunner.hxx>
#include <repro/AclStore.hxx>
#include <repro/ProxyConfig.hxx>
#include <repro/Processor.hxx>
#include <repro/RequestContext.hxx>
#include <repro/Proxy.hxx>
#include <repro/monkeys/StaticRoute.hxx>

#include <rutil/Log.hxx>


using namespace repro;
using namespace resip;

class ReproGlue : public repro::ReproRunner
{
public:
  //
  // Type definitions
  //
  typedef std::map<std::string, std::string> ConfigMap; /// Storage for configuration entries
  
  
  //
  // Internal classes
  //
  class ReproConfig : public repro::ProxyConfig
  {
    /// This class extends Proxyconfig to allow loading of defaults value
    /// within code as opposed to the default behavior of only allowing
    /// setting of configuration value from a config file
    ///
  public:
    void setDefaultConfigValue(const std::string& name, const std::string& value);
      /// Sets a configuration value in ProxyConfig.  This is only used internally.
      /// by configureRepro method.  The name value pair set by this method can
      /// be overridden if _configMap contains the override value.
      ///
      ///
      /// The public API call would be ReproGlue::setProxyConfigValue() method;
      ///
    
    void setCustomConfigurations();
      /// add all configuration items in _customConfigMap to the 
      /// proxy configuration
      
  protected:
    ConfigMap _configMap; /// Holds the list of proxy configuration items that 
                        /// overrides the default value set by setConfigValue
    
    ConfigMap _customConfigMap; /// Holds the list of custom configuration items.
    
    friend class ReproGlue;
  };
  
  class ReproLogger : public ExternalLogger
  {
    //
    // This class defines an external logger callback used by repro.
    // It allows application to hijack repro logs.  If not set repro
    // will send logs to std::cout
    //  
  public:
    
    //
    // Type definitions
    //
    typedef boost::function<void(int /*level*/, const char*/*message*/)> LogCallBack; /// Callback for the external logger
    
    enum LogLevel
    {
       None = resip::Log::None,
       Crit = resip::Log::Crit,
       Err =  resip::Log::Err,
       Warning = resip::Log::Warning,
       Info = resip::Log::Info,
       Debug = resip::Log::Debug,
       Stack = resip::Log::Stack,
       StdErr = resip::Log::StdErr
    };
    
    ReproLogger();
    
    virtual ~ReproLogger();
    
    /** return true to also do default logging, false to suppress default logging. */
    virtual bool operator()(Log::Level level,
                            const Subsystem& subsystem, 
                            const Data& appName,
                            const char* file,
                            int line,
                            const Data& message,
                            const Data& messageWithHeaders);
    
    void setLogLevel(ReproLogger::LogLevel level);
    /// Set the logging priority
    ///

    void setLogCallBack(LogCallBack callBack);
    /// Set the log callback function.  If not set, logs will be dumped to std::cout
    ///
    
    bool initialize();
    /// Initialize the logging subsystem
    ///
    
  protected:
    LogCallBack _callback; /// Callback function for the logger.  If not set, logs will be dumped to std::cout
    bool _initialized; /// Flag set if already initialized
    LogLevel _logLevel; /// Current log level
  };
  
  class RequestProcessor : public repro::Processor
  {
    //
    // This is the base class for custom request processor we will be inserting
    // on top of the default repro monkeys.  This chain will hold the default repro
    // chain as the first argument.  Instead of the default chain, we will be inserting
    // an instance of this class instead.  Applications are allowed to attach an
    // alternative callback so they can override the default processor chain.
    // If a callback is not defined, then the default processor chain is called.
    // We have added a new ChainReaction member named CallDefaultChain.  This is
    // an addition to processor_action_t values.  The role of the CallDefaultChain
    // is to instruct the RequestProcessor to still call the default chain after the
    // default callback has been called.
    //
  public:
    //
    // Enums
    //
    enum ChainReaction
    {
       Continue = Processor::Continue, // move onto the next Processor
       WaitingForEvent = Processor::WaitingForEvent, // stop Processor chain and wait for async response
       SkipThisChain = Processor::SkipThisChain, // skip monkey chains
       SkipAllChains = Processor::SkipAllChains, // skip all (monkey, lemur and baboon)
       CallDefaultChain // Let the default chain handle the request context
    };
    
    //
    // Type Definitions
    //
    typedef boost::function<ChainReaction(ReproGlue&, RequestContext&)> RequestCallBack;
    
    RequestProcessor(repro::Processor* pDefaultChain,
                     ReproGlue& reproGlue, 
                     const std::string& name);
    virtual ~RequestProcessor();
    
    virtual processor_action_t process(RequestContext& context);
    
    void setHandler(RequestCallBack callback);
  
  protected:
    ReproGlue& _reproGlue;
    RequestCallBack _callback;
    repro::Processor* _pDefaultChain;
  };
   
  ReproGlue(const std::string& applicationName, const std::string& databaseDir = SIPX_DBDIR);
  /// Constructs a new ReproGlue with the given applicationName.
  /// applicationName will be used as a prefix to the dataStore
  /// created for this instance. pLogger is a pointer to the external logger
  /// callback derived from ExternalLogger class.  If set, repro will invoke
  /// the callback and allow the application to bypass resiprocate logging
  /// subsystem.  The databaseDir paramater can be set to allow the application
  /// to store persistent information in a different directory other then the default.
  
  void setProxyConfigValue(const std::string& name, const std::string& value, bool custom = false);
  /// This method allows applications to override the proxy configuration defaults.
  /// Valid parameters and their default value are directly taken from repro.config.
  /// If custom is set to true, this will be treated as a special configuration and
  /// will be inserted event if it is not a standard repro configuration.  Transport<Num>
  /// values also belongs to this category.
  ///
  
  
  bool run();
  /// Start the repro subsystem using the configuration specified
  /// by path
  ///
  
  virtual bool createDatastore();
  /// Override the default datastore creation to allow ReproRunner
  /// to add a different prefix for the databases created.
  ///
 
  ReproConfig& getReproConfig();
  /// Returns a reference the the current ReproGlue configuration
  
  ///////////////////////////
  // Configuration helpers //
  ///////////////////////////
  
  
  void addTransport(
    const std::string& transport, 
    const std::string& hostPort, 
    bool recordRoute = true);
  /// Add a new non-secure listener.  Valid transports are <'UDP' | 'TCP' | 'WS'>. 
  /// Default is UDP if left empty.
  /// The hostport holds the IP:port tuple.  For IPV6 addresses, last colon separates
  /// IP address and port (square bracket notation is not used)
  /// if recordroute route is set to true, a record route header will be inserted
  /// for all requests using this transport.  
  
  void addSecureTransport(
    const std::string& transport, 
    const std::string& hostPort,
    const std::string& tlsDomain,
    const std::string& tlsCertificate,
    const std::string& tlsPrivateKey,
    const std::string& tlsClientVerification,
    bool recordRoute = true);
  /// Add a new non-secure listener.  Valid transports are <'TLS' | 'DTLS' | 'WSS'>. 
  /// The hostport holds the IP:port tuple.  For IPV6 addresses, last colon separates
  /// IP address and port (square bracket notation is not used)
  /// if recordroute route is set to true, a record route header will be inserted
  /// for all requests using this transport. TLS certificate and private key files  
  /// must be specified.  The tlsClientVerification must be set to <'None'|'Optional'|'Mandatory'>
  
  bool addTrustedNode(const std::string& address, const short& port, const short& transportType);
  /// Add the address:port:transportType for a trusted node.  A trusted node will be allowed
  /// by the proxy without requiring any authentication.
  ///
  /// Address can be in any of these formats
  ///    localhost         localhost  (becomes 127.0.0.1/8, ::1/128 and fe80::1/64)
  ///    bare hostname     server1
  ///    FQDN              server1.example.com
  ///    IPv4 address      192.168.1.100
  ///    IPv4 + mask       192.168.1.0/24
  ///    IPv6 address      ::341:0:23:4bb:0011:2435:abcd
  ///    IPv6 + mask       ::341:0:23:4bb:0011:2435:abcd/80
  ///    IPv6 reference    [::341:0:23:4bb:0011:2435:abcd]
  ///    IPv6 ref + mask   [::341:0:23:4bb:0011:2435:abcd]/64
  ///
  /// If port is 0, it means any port.
  ///
  /// Valid transport types are:
  ///
  ///   resip::TLS
  ///   resip::TCP
  ///   resip::UDP
  ///   resip::SCTP
  ///   resip::DCCP
  ///   resip::DTLS
  ///   resip::WS
  ///   resip::WSS
  ///
  
  bool addUser( const std::string& user, 
                const std::string& domain, 
                const std::string& realm, 
                const std::string& password, 
                const std::string& fullName,
                const std::string& emailAddress );
  /// Add or update new user to the user store. 
  ///
  
  void removeUser(const std::string& user, const std::string& realm);
  /// Delete the user from the user store
  ///
  
  bool addDomain(const std::string& domain, const int port);
  /// Add a new domain.  Take note that you can only add users to a domain
  /// if the domain is already existing in the configuration db
  ///
      
  void removeDomain(const std::string& domain);
  /// Remove the domain
  ///
  
  /////////////
  // Logging //
  /////////////
  
  void setLogLevel(ReproLogger::LogLevel level);
  /// Set the logging priority
  ///
  
  void setLogCallBack(ReproLogger::LogCallBack callBack);
  /// Set the log callback function.  If not set, logs will be dumped to std::cout
  ///
  
  
  ////////////////////////////
  // RequestContext helpers //
  ////////////////////////////
  
  static bool getRequestSourceAddress(RequestContext& context, std::string& address, unsigned short& port);
  /// Returns the source address and port of a request
  
  static bool getRequestViaAddress(RequestContext& context, std::string& transport, std::string& address, unsigned short& port);
  /// Returns the via address of a request
  
  //////////////////////////////
  // Request Processor Chains //
  //////////////////////////////
  
  void setStaticRouteHandler(RequestProcessor::RequestCallBack handler);
  /// Set the custom callback function for StaticRoute (monkey) 
  ///
  
  void setTargetHandler(RequestProcessor::RequestCallBack handler);
  /// Set the custom callback function for Target (baboons) 
  ///
  
  void setResponseHandler(RequestProcessor::RequestCallBack handler);
  /// Set the custom callback function for Responses (lemurs) 
  ///

  virtual void makeRequestProcessorChain(repro::ProcessorChain& chain);
  /// Override the default request processor chain of repro.
  /// This allows us to insert/change the behavior of the repro
  /// default processor
  
  virtual void makeTargetProcessorChain(ProcessorChain& chain);
  /// Override the default target processor chain of repro.
  /// This allows us to insert/change the behavior of the repro
  /// default processor
  
  virtual void makeResponseProcessorChain(ProcessorChain& chain);
  /// Override the default response processor chain of repro.
  /// This allows us to insert/change the behavior of the repro
  /// default processor
  
protected:
  virtual void configureRepro();
  /// This method is called from the ReproGlue::run method.  This produces
  /// the current configuration parameters for the repro proxy.  The original
  /// method of configuring repro is via a configuration file.  We however
  /// intend ReproGlue as an API.  It makes sense to allow developers to be
  /// able to hard code certain proxy behavior specific to the application type
  /// or purpose.
  ///
  
  ReproConfig* _pReproConfig;  /// Holds a pointer to the proxy configuration.
                               /// Pointer will be deleted by ReproRunner.
  
  std::string _applicationName; /// Holds the application name used to prefix data stores
   
  static ReproLogger _logger;  /// The external logger.  This is static because there can be only one logger instance system-wide.
  
  RequestProcessor* _pStaticRouter; /// Pointer to the static router request processor
  
  RequestProcessor* _pTargetProcessor; /// Pointer to the target processor
  
  RequestProcessor* _pResponseProcessor; /// Pointer to the response processor
   
  int _currentTransportCount; /// Holds the number of listeners added
  
  bool _enableWebAdmin; /// Enables the web-admin gui.  Defaults to false
};


//
// Inlines
//

inline ReproGlue::ReproConfig& ReproGlue::getReproConfig()
{
  assert(_pReproConfig);
  return *_pReproConfig;
}

inline void ReproGlue::setLogLevel(ReproLogger::LogLevel level)
{
  _logger.setLogLevel(level);
}
  
inline void ReproGlue::setLogCallBack(ReproLogger::LogCallBack callBack)
{
  _logger.setLogCallBack(callBack);
}

inline void ReproGlue::setStaticRouteHandler(RequestProcessor::RequestCallBack handler)
{
  assert(_pStaticRouter);
  _pStaticRouter->setHandler(handler);
}

inline void ReproGlue::setTargetHandler(RequestProcessor::RequestCallBack handler)
{
  assert(_pTargetProcessor);
  _pTargetProcessor->setHandler(handler);
}
  
inline void ReproGlue::setResponseHandler(RequestProcessor::RequestCallBack handler)
{
  assert(_pResponseProcessor);
  _pResponseProcessor->setHandler(handler);
}
#endif	// REPROGLUE_H_INCLUDED

