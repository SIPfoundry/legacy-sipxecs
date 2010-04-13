//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _PLUGINHOOKS_H_
#define _PLUGINHOOKS_H_

// SYSTEM INCLUDES
#include "utl/UtlString.h"
#include "utl/UtlSortedList.h"
#include "utl/UtlSortedListIterator.h"

// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class Plugin;
class PluginIterator;
class OsConfigDb;

/**
 * A PluginHooks object is used to add dynamically loaded libraries (a "plugin") to a program
 * at run time.  The module to be loaded must implement a class derived from
 * the Plugin abstract class.
 *
 * @htmlinclude PluginOverview.html
 *
 * An object of this class manages all the configured plugin hooks for a program.
 * A class of plugin hooks is identified by a factory routine name used to obtain
 * a hook instance from the dynamic library, and an OsConfigDb prefix string:
 * @code
 * PluginHooks ActionEventHooks("getFooAction", ACTION_EVENT"); // to be called for each Action
 * @endcode
 *
 * The libraries are loaded and configured by the readConfig method:
 * @code
 * ActionEventHooks.readConfig(configDb);
 * @endcode
 *
 * To invoke the plugins, see PluginIterator.
 */
class PluginHooks
{
  public:
   /// Construct a manager for a set of hooks.
   PluginHooks(const char* hookFactoryName, ///< the prefix name for the OsConfigDb values
               const char* hookPrefix       ///< the prefix name for the OsConfigDb values

               );

   ~PluginHooks();

   /// Read what hooks are configured, and instantiate and configure each hook.
   void readConfig(const OsConfigDb& configDb);
   /**<
    * This method actually reads the program configuration from the configDb passed in.
    * Each entry that has the prefix followed by "_HOOK_LIBRARY" configures a plugin library.
    * @code
    * [prefix]_HOOK_LIBRARY.[instance] : [path to libexamplereghook.so]
    * @endcode
    * for the example code above, it would look for entries like:
    * @code
    * ACTION_EVENT_HOOK_LIBRARY.RecordAction : /usr/local/lib/sipxpbx/librecordaction.so
    * ACTION_EVENT_HOOK_LIBRARY.CopyAction   : /usr/local/lib/sipxpbx/libcopyaction.so
    * @endcode
    * The readConfig method:
    * - dynamically loads each library
    * - for each value of [instance]
    *   - calls the factory method provided by the hook to instantiate a hook object,
    *     passing its instance name to it (so that it can be used in logging).
    *   - passes the new hook object a configuration subhash of its configuration
    *     entries (if any).
    *
    * Configuration entries for each hook instance are made with entries like:
    * @code
    * [prefix].[instance].FOO : foovalue
    * [prefix].[instance].BAR : barvalue
    * @endcode
    * Each instance has its own set of configuration entries:
    * @code
    * ACTION_EVENT.RecordAction.FOO : foovalue1
    * ACTION_EVENT.RecordAction.BAR : barvalue1
    * ACTION_EVENT.CopyAction.FOO : foovalue2
    * ACTION_EVENT.CopyAction.BAR : barvalue2
    * @endcode
    *
    * The readConfig method in the hook (which must inherit from Plugin) is passed
    * its own subhash of the configuration data, stripping everything through the '.'
    * following the instance name, so in the example above, the CopyAction hook would
    * be passed the equivalent of this configuration:
    * @code
    * FOO : foovalue2
    * BAR : barvalue2
    * @endcode
    *
    * readConfig can be called more than once.
    * - New plugin instances are instantiated as described above.
    * - Existing plugin instances that are no longer in the configuration
    *   are deleted (their destructor is invoked).
    * - Existing plugin instances that are still in the configuration
    *   are reconfigured (their Plugin::readConfig method is called).
    *
    */

   /**
    * Return the total number of plugins within.
    */
   size_t entries() const;

  protected:
   friend class PluginIterator;

   UtlString      mFactory;         ///< the factory routine name for this hook class
   UtlString      mPrefix;          ///< the prefix name for the OsConfigDb values
   UtlSortedList  mConfiguredHooks; ///< the list of configured hooks.
};

/**
 * PluginIterator is used to obtain a sequence of Plugin objects to be invoked.
 *
 * @htmlinclude PluginOverview.html
 *
 * The calling program gets the plugin objects by creating a PluginIterator
 * over a Plugin object and then calling the PluginIterator::next method to
 * get each instance of the Plugin (and optionally, its instance name).
 *
 * The PluginIterator always returns the configured Plugin objects in lexical
 * order by the instance name; this allows the configuration to control the
 * order in which they are invoked.
 *
 * Plugin libraries can implement any calling interface that's needed (a
 * program that uses the Plugin mechanism should create a base class that extends
 * Plugin to specify the interface).
 *
 * A typical usage would look like:
 * @code
 * PluginIterator actions(ActionEventHooks);
 * YourPluginClass* action;
 * while(action = static_cast<YourPluginClass*>(actions.next()))
 * {
 *   action->invokeMethod();
 * }
 * @endcode
 */
class PluginIterator
{
  public:
   /// Create an iterator that returns each instance managed by a PluginHooks object.
   PluginIterator( const PluginHooks& pluginHooks );

   ~PluginIterator();

   /// Advance to and return the next plugin.
   Plugin* next(UtlString* name = NULL /**< the instance name string for the returned
                                            *   Plugin (for logging purposes)
                                            *   may be NULL if the caller does not need the name.
                                            */
                    );
   /**<
    * No meaning should be attached to Plugin names, so that order of plugin
    * iteration can be controlled by the lexical order of plugin names.
    */

  private:
   UtlSortedListIterator mConfiguredHooksIterator;

};

#endif // _PLUGINHOOKS_H_
