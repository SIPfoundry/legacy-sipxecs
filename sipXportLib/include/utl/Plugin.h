//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _PLUGIN_H_
#define _PLUGIN_H_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlString.h"
class OsConfigDb;

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class Plugin;

/**
 * A Plugin is a dynamically loaded object that is invoked by some component at some
 *   well defined time (it's an abstract class - how specific can the
 *   description be?).
 *
 * This class is the abstract base from which all plugins must inherit; it
 * decouples the configuration of what plugins should be invoked and the
 * configuration parameters specific to each plugin from the program that
 * uses them.
 *
 * @htmlinclude PluginOverview.html
 *
 * All Plugin classes must implement three methods to configure the plugin
 * into the component:
 * - An extern "C" factory routine
 * - The constructor for its class, which must ultimately derive from Plugin
 * - The readConfig method used to pass configuration data to the plugin (this decouples plugin
 *   configuration from the component configuration).
 *
 * Each class derived from Plugin should also define the method(s) that
 * the program should invoke on the plugin, and all those methods must be virtual.
 *
 * @see PluginHooks for details of how a plugin is configured into a program,
 * and PluginIterator for how plugins are invoked by the calling component.
 *
 */
class Plugin
{
  public:

   typedef Plugin* (*Factory)(const UtlString& pluginName ///< the name for this instance
                              );
   /**<
    * The Factory uses external C linkage to support dynamic loading of Plugin objects.
    *
    * In addition to the class derived from this base, a plugin must implement a
    * Factory routine with extern "C" linkage so that the OsSharedLib mechanism
    * can look it up in the dynamically loaded library (looking up C++ symbols
    * is problematic because of name mangling).  The Factory routine looks like:
    * @code
    * class ExamplePlugin;
    *
    * extern "C" ExamplePlugin* getExamplePlugin(const UtlString& name)
    * {
    *   return new ExamplePlugin;
    * }
    *
    * class ExamplePlugin : public Plugin
    * {
    *    friend ExamplePlugin* getExamplePlugin(const UtlString& name);
    *   ...
    * private:
    *    ExamplePlugin(const UtlString& name);
    * }
    * @endcode
    */

   /// The plugin destructor must be virtual.
   virtual ~Plugin()
      {
      };

   /// Read (or re-read) whatever configuration the plugin requires.
   virtual void readConfig( OsConfigDb& configDb /**< a subhash of the individual configuration
                                                  * parameters for this instance of this plugin. */
                           ) = 0;
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

   /// Derived constructors should be private so that only the Factory can call them.
   Plugin(const UtlString& instanceName ///< instance name passed by the factory
          ) :
      mInstanceName(instanceName)
      {
      };

   /// The instance name from the configuration directive - for logging and other identification.
   UtlString   mInstanceName;

  private:

   /// There is no copy constructor.
   Plugin(const Plugin&);

   /// There is no assignment operator.
   Plugin& operator=(const Plugin&);

};

#endif // _PLUGIN_H_
