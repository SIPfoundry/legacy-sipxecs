//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "assert.h"

// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "utl/UtlHashMap.h"
#include "utl/Plugin.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class TestPlugin;


extern "C" TestPlugin* getTestPlugin(const UtlString& name);

/**
 * TestPlugin defines the interface for plugins to the PluginsTest.
 *
 */
class TestPlugin : public Plugin
{
public:

   virtual ~TestPlugin();

   /// Read (or re-read) whatever configuration the hook requires.
   void readConfig( OsConfigDb& configDb );

   /// Return the integer value for a given configuration key
   virtual bool getConfiguredValueFor(const UtlString& key, UtlString& value ) const;

   /// Set type to the unique library name
   virtual void pluginName(UtlString& name) const;

   static const char* LibraryName;

   static const char* FactoryName;

private:
   friend TestPlugin* getTestPlugin(const UtlString& name);

   /// The constructor is called from getHook factory method.
   TestPlugin(const UtlString& hookName);

   UtlHashMap  mConfiguration;
   bool        mConfigured;
};
