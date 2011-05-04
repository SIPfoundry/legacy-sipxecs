//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsLogger.h"
#include "os/OsConfigDb.h"
#include "os/OsSharedLibMgr.h"
#include "utl/UtlString.h"
#include "utl/UtlSList.h"

#include "utl/Plugin.h"
#include "utl/PluginHooks.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define HOOK_LIB_PREFIX "_HOOK_LIBRARY."

// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
// GLOBAL VARIABLES

// ConfiguredHook is the container used to hold each Plugin.
/*
 * ConfiguredHook inherits from UtlString so that it will be a
 * UtlContainable and will be identifiable by its configured prefix name.
 */
class ConfiguredHook : public UtlString
{
public:

   /// Wrapper around the constructor to do error checking and return NULL for errors
   static ConfiguredHook* newHook(const UtlString& hookName,
                                  const UtlString& hookFactoryName,
                                  const UtlString& libName
                                  )
      {
         ConfiguredHook* theNewHook = NULL;

         if (! libName.isNull())
         {
            theNewHook = new ConfiguredHook(hookName, hookFactoryName, libName);
            // check if the ConfiguredHook constructor actually managed to create a plug-in
            if( !theNewHook->plugin() )
            {
               // an error happened while trying to create the plug-in -
               // de-allocate the ConfiguredHook and return NULL to convey
               // the failure to the caller.
               delete theNewHook;
               theNewHook = NULL;
            }
         }
         else
         {
            Os::Logger::instance().log(FAC_KERNEL, PRI_CRIT,
                          "PluginHooks: no library configured for hook '%s': ignored",
                          hookName.data()
                          );
         }

         return theNewHook;
      }

   ~ConfiguredHook()
      {
         // don't try to unload any libraries
         // it's not worth the complexity of reference counting it
         // the same library could be configured more than once with
         // different hook names and parameters.
      }

   /// Get the name of this hook.
   void name(UtlString* hookName) const
      {
         if (hookName)
         {
            hookName->remove(0);
            hookName->append(*this);
         }
      }

   /// Compare the library name.
   bool isLibrary(const UtlString& libName) const
      {
         return mLibName.compareTo(libName) == 0;
      }

   /// Get the actual hook object.
   Plugin* plugin() const
      {
         return mHook;
      }

   /// Construct the subhash for the hook and configure it.
   void readConfig(const UtlString& prefix, const OsConfigDb& configDb)
      {
         if (mHook)
         {
            OsConfigDb myConfig;
            UtlString myConfigName;

            // build up "<prefix>.<instance>." key for configuration subhash
            myConfigName.append(prefix);
            myConfigName.append('.');
            myConfigName.append(*this);
            myConfigName.append('.');

            if (OS_SUCCESS == configDb.getSubHash(myConfigName, myConfig))
            {
               Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG,
                             "ConfiguredHook:: configuring instance '%s' using prefix '%s'",
                             data(), myConfigName.data()
                             );
               mHook->readConfig(myConfig);
            }
            else
            {
               Os::Logger::instance().log(FAC_KERNEL, PRI_CRIT,
                             "PluginHooks no configuration found for instance '%s'",
                             data()
                             );
            }
         }
      };

private:

   // load the library for a hook and use its factory to get a new instance.
   ConfiguredHook(const UtlString& hookName,
                  const UtlString& hookFactoryName,
                  const UtlString& libName
                  )
      : UtlString(hookName),
        mHook(NULL),
        mLibName(libName)
      {
         OsSharedLibMgrBase* sharedLibMgr = OsSharedLibMgr::getOsSharedLibMgr();

         if (sharedLibMgr)
         {
            Plugin::Factory factory;

            if (OS_SUCCESS == sharedLibMgr->getSharedLibSymbol(libName.data(),
                                                               hookFactoryName,
                                                               (void*&)factory
                                                               )
                )
            {
               // Use the factory to get an instance of the hook
               // and tell the new instance its own name.
               mHook = factory(hookName);

               Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG,
                             "PluginHooks ConfiguredHook:: created instance '%s' from '%s'",
                             hookName.data(), libName.data()
                             );
            }
            else
            {
               Os::Logger::instance().log(FAC_KERNEL, PRI_ERR,
                             "PluginHooks ConfiguredHook:: "
                             "factory '%s' not found in library '%s' for instance '%s'",
                             hookFactoryName.data(), libName.data(), hookName.data()
                             );
            }
         }
         else
         {
            Os::Logger::instance().log(FAC_KERNEL, PRI_CRIT,
                          "PluginHooks ConfiguredHook:: failed to getOsSharedLibMgr"
                          );
         }
      }

   Plugin*    mHook;    ///< the actual hook instance
   UtlString  mLibName; ///< the library name (for checking reconfiguration)
};


PluginHooks::PluginHooks(const char* hookFactoryName, // the prefix name for the OsConfigDb values
                         const char* hookPrefix       // the prefix name for the OsConfigDb values
                         )
   : mFactory(hookFactoryName),
     mPrefix(hookPrefix)
{
}

PluginHooks::~PluginHooks()
{
   mConfiguredHooks.destroyAll();
}

void PluginHooks::readConfig(const OsConfigDb& configDb)
{
   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG,
                 "PluginHooks::readConfig mFactory = '%s', mPrefix = '%s'",
                 mFactory.data(), mPrefix.data());

   // Move any existing hooks from the current configured list to
   // a temporary holding list.
   UtlSList existingHooks;
   UtlContainable* existingHook;

   UtlSortedListIterator nextHook(mConfiguredHooks);
   while ((existingHook = nextHook()))
   {
      existingHooks.append(mConfiguredHooks.removeReference(existingHook));
   }
   // the mConfiguredHooks list is now empty

   // Walk the current configuration,
   //   any existing hook is moved back to the mConfiguredHooks list,
   //   newly configured hooks are added,
   //   each configured hook is called to read its own configuration.
   UtlString  hookPrefix(mPrefix);
   hookPrefix.append(HOOK_LIB_PREFIX);

   OsConfigDb allHooks;

   Os::Logger::instance().log(FAC_KERNEL, PRI_DEBUG,
                 "PluginHooks::readConfig looking up hooks '%s'",
                 hookPrefix.data()
                 );
   if (OS_SUCCESS == configDb.getSubHash(hookPrefix, allHooks)) // any hooks configured for prefix?
   {
      UtlString lastHook;
      UtlString hookName;
      UtlString hookLibrary;

      // walk each hook and attempt to load and configure it
      for ( lastHook = "";
            OS_SUCCESS == allHooks.getNext(lastHook, hookName, hookLibrary);
            (lastHook = hookName, hookName.remove(0), hookLibrary.remove(0))
           )
      {
         ConfiguredHook* thisHook;

         if (NULL == (thisHook = dynamic_cast<ConfiguredHook*>(existingHooks.remove(&hookName))))
         {
            // not an existing hook, so create a new one
            thisHook = ConfiguredHook::newHook(hookName, mFactory, hookLibrary);
         }
         else
         {
            // this is a pre-existing hook; check to see that the library has not changed.
            if (! thisHook->isLibrary(hookLibrary))
            {
               // the library for thisHook has changed, so delete and recreate it with the new one.
               delete thisHook;
               thisHook = ConfiguredHook::newHook(hookName, mFactory, hookLibrary);
            }
         }

         if (thisHook)
         {
            // put the hook onto the list of active hooks
            mConfiguredHooks.insert(thisHook);

            // (re)configure the hook
            thisHook->readConfig(mPrefix, configDb);
         }
      }
   }
   else
   {
      Os::Logger::instance().log(FAC_KERNEL, PRI_INFO,
                    "PluginHooks: no '%s' hooks configured", mPrefix.data()
                    );
   }

   // discard any hooks that are no longer in the configuration
   existingHooks.destroyAll();
}

size_t PluginHooks::entries() const
{
   return mConfiguredHooks.entries();
}

PluginIterator::PluginIterator( const PluginHooks& pluginHooks ) :
   mConfiguredHooksIterator(pluginHooks.mConfiguredHooks)
{
}

Plugin* PluginIterator::next(UtlString* name)
{
   Plugin* nextPlugin = NULL;

   // make sure that name is cleared if passed in case this is the last hook
   if (name)
   {
      name->remove(0);
   }

   // step the parent iterator on the mConfiguredHooks list
   ConfiguredHook* nextHook = static_cast<ConfiguredHook*>(mConfiguredHooksIterator());
   if (nextHook)
   {
      nextHook->name(name); // return the name, if it's been asked for
      nextPlugin = nextHook->plugin();
   }

   return nextPlugin;
}

PluginIterator::~PluginIterator()
{
}
