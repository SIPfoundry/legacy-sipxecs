//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <sipxunit/TestRunner.h>

// Is used by unit tests that calls exit in the function registered with atexit
// in order to know sipxunit exit status
int gSipXunitExitStatus = 0;

/**
 * include this in your project and it will find all Suite's that have been
 * registered with the CppUnit TextFactoryRegistry and run them
 */

int main( int argc, char* argv[] )
{
    TestRunner runner(argv);
    CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
    runner.addTest(registry.makeTest());
    bool wasSucessful = runner.run();

    gSipXunitExitStatus = !wasSucessful;

    return !wasSucessful;
}
