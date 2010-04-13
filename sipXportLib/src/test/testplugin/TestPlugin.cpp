//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "os/OsConfigDb.h"

#include "TestPlugin.h"

// DEFINES
//#define TEST_DEBUG // if defined, adds logging

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


/**
 * A test hook for use with PluginTest
 *
 * This is compiled twice by the Makefile to produce two different shared libraries.
 * The only differences between those compilations are the command line definitions:
 * - For TestPluginA:
 * @code
 * -DLIBRARY_NAME=\"TestPluginA\" -DTestPlugin=TestPluginA
 * @endcode
 * - For TestPluginB:
 * @code
 * -DLIBRARY_NAME=\"TestPluginB\" -DTestPlugin=TestPluginB
 * @endcode
 * This makes the classes have unique names, so they are invoked correctly when constructed.
 */
const char* TestPlugin::LibraryName = LIBRARY_NAME;

const char* TestPlugin::FactoryName = "getTestPlugin";

/// Read (or re-read) whatever configuration the hook requires.
void TestPlugin::readConfig( OsConfigDb& configDb )
{
   UtlString key;
   UtlString value;

   if (mConfigured)
   {
      mConfiguration.destroyAll();
   }

   UtlString place;
   for (; configDb.getNext(place, key, value) == OS_SUCCESS; place = key)
   {
      mConfiguration.insertKeyAndValue(new UtlString(key), new UtlString(value));
   }

   mConfigured = true;
}



/// Return the integer value for a given configuration key
bool TestPlugin::getConfiguredValueFor(const UtlString& key, UtlString& value) const
{
   assert(mConfigured);

   UtlString* found = static_cast<UtlString*>(mConfiguration.findValue(&key));

   if (found)
   {
      value = *found;
   }

   return found != NULL;
}


/// Set to the unique library name
void TestPlugin::pluginName(UtlString& name) const
{
   assert(mConfigured);

   name.remove(0);
   name.append(LibraryName);
   name.append("::");
   name.append(mInstanceName);
}


/// The constructor is called from getTestPlugin below
TestPlugin::TestPlugin(const UtlString& name) :
   Plugin(name),
   mConfigured(false)
{
}

TestPlugin::~TestPlugin()
{
   mConfiguration.destroyAll();
}

extern "C" TestPlugin* getTestPlugin(const UtlString& name)
{
   return new TestPlugin(name);
}
