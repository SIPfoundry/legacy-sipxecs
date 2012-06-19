/**
 *
 * Copyright (c) eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License (LGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 *
 */


#ifndef SipBidirectionalProcessorPlugin_H
#define	SipBidirectionalProcessorPlugin_H

#include <boost/noncopyable.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/any.hpp>
#include <string>
#include <map>
#include "utl/UtlString.h"
#include "utl/Plugin.h"
#include "net/SipMessage.h"
#include "net/SipUserAgent.h"

class SipBidirectionalProcessorPlugin : public Plugin, boost::noncopyable
{
public:
  typedef boost::function<void(SipMessage&, const char*, int)> Handler;
  typedef std::map<std::string, boost::any> PropertyMap;

  class InputProcessor : public SipInputProcessor
  {
  public:
    InputProcessor(int priority);
    virtual ~InputProcessor();
    void handleOutputMessage(SipMessage& message, const char* address, int port);
    void setEnabled(bool enabled);
  private:
    bool _enabled;
    Handler _handler;
    friend class SipBidirectionalProcessorPlugin;
  };

  class OutputProcessor : public SipOutputProcessor
  {
  public:
    OutputProcessor(int priority);
    virtual  ~OutputProcessor();
    void handleOutputMessage(SipMessage& message, const char* address, int port);
    void setEnabled(bool enabled);
  private:
    bool _enabled;
    Handler _handler;
    friend class SipBidirectionalProcessorPlugin;
  };

  static const char* Prefix;  ///< the configuration file prefix = "SIPX_PROXY"
  static const char* Factory; ///< the factory routine name = "getSipBidirectionalProcessorPlugin"

  /// destructor
  virtual ~SipBidirectionalProcessorPlugin();

  /// Initialize everything that needs to be done prior to plugin operations
  virtual void initialize() = 0;

  /// Read (or re-read) whatever configuration the plugin requires.
  virtual void readConfig( OsConfigDb& configDb /**< a subhash of the individual configuration
                                      * parameters for this instance of this plugin. */
               ) = 0;

  /// Used to announce the SIP UserAgent instance that is logically associated with this Plugin.
  /// Plugins that need to interact with their associated SIP UserAgent can override this method
  /// and save the passed pointer for later use.
  virtual void announceAssociatedSipUserAgent(SipUserAgent* userAgent);

  ///
  /// All incoming Sip Messages will be sent to this virtual function.
  /// Plugins that need to manipulate incoming Sip messages must do so here.
  virtual void handleIncoming(SipMessage& message, const char* address, int port) = 0;

  ///
  /// All outgoing Sip Messages will be sent to this virtual function.
  /// Plugins that need to manipulate outgoing Sip messages must do so here.
  virtual void handleOutgoing(SipMessage& message, const char* address, int port) = 0;

  //
  // Custom property setters
  //
  void setProperty(const std::string& name, const boost::any& value);
  template <typename TypeCast>
  bool getProperty(const std::string& name, TypeCast& value)
  {
    try
    {
      PropertyMap::iterator iter = _properties.find(name);
      if (iter == _properties.end())
        return false;
      value = boost::any_cast<TypeCast>(*iter);
      return true;
    }
    catch(const boost::bad_any_cast &)
    {
      return false;
    }
  }
  /**<
  * @note
  * The parent service may call the readConfig method at any time to
  * indicate that the configuration may have changed.  The plugin
  * should reinitialize itself based on the configuration that exists when
  * this is called.  The fact that it is a subhash means that whatever prefix
  * is used to identify the plugin (see PluginHooks) has been removed (see the
  * examples in PluginHooks::readConfig).
  */

protected:

  /// constructor
  SipBidirectionalProcessorPlugin(
    const UtlString& instanceName, ///< the configured name for this plugin instance
    int priority
  );

  SipUserAgent* _pUserAgent;
  InputProcessor _input;
  OutputProcessor _output;
  PropertyMap _properties;
};

//
// Inlines
//
inline SipBidirectionalProcessorPlugin::SipBidirectionalProcessorPlugin(const UtlString& instanceName, int priority) :
  Plugin(instanceName),
  _pUserAgent(0),
  _input(priority),
  _output(priority)
{
  _input._handler = boost::bind(&SipBidirectionalProcessorPlugin::handleIncoming, this, _1, _2, _3);
  _output._handler = boost::bind(&SipBidirectionalProcessorPlugin::handleOutgoing, this, _1, _2, _3);
}

inline SipBidirectionalProcessorPlugin::~SipBidirectionalProcessorPlugin()
{
}

inline void SipBidirectionalProcessorPlugin::announceAssociatedSipUserAgent(SipUserAgent* userAgent)
{
  _pUserAgent = userAgent;
  _pUserAgent->addSipInputProcessor(&_input);
  _pUserAgent->addSipOutputProcessor(&_output);
}

inline void SipBidirectionalProcessorPlugin::setProperty(const std::string& name, const boost::any& value)
{
  _properties[name] = value;
}


//
// Input Processor Inlines
//
inline SipBidirectionalProcessorPlugin::InputProcessor::InputProcessor(int priority) :
  SipInputProcessor(priority),
  _enabled(true)
{
}

inline SipBidirectionalProcessorPlugin::InputProcessor::~InputProcessor()
{
}

inline void SipBidirectionalProcessorPlugin::InputProcessor::setEnabled(bool enabled)
{
  _enabled = enabled;
}

inline void SipBidirectionalProcessorPlugin::InputProcessor::handleOutputMessage(SipMessage& message, const char* address, int port)
{
  if (_enabled && _handler)_handler(message, address, port);
}


//
// Output Processor Inlines
//
inline SipBidirectionalProcessorPlugin::OutputProcessor::OutputProcessor(int priority) :
  SipOutputProcessor(priority),
  _enabled(true)
{
}

inline SipBidirectionalProcessorPlugin::OutputProcessor::~OutputProcessor()
{
}

inline void SipBidirectionalProcessorPlugin::OutputProcessor::setEnabled(bool enabled)
{
  _enabled = enabled;
}

inline void SipBidirectionalProcessorPlugin::OutputProcessor::handleOutputMessage(SipMessage& message, const char* address, int port)
{
  if (_enabled && _handler)_handler(message, address, port);
}


#endif	/* SipBidirectionalProcessorPlugin_H */

