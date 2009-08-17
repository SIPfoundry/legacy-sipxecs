//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _CpMediaInterfaceFactoryFactory_h_
#define _CpMediaInterfaceFactoryFactory_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "CpMediaInterfaceFactory.h"
#include "CpMediaInterfaceFactoryImpl.h"

class OsConfigDb;

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS

/**
 * Well known function that is called to create a CpMediaInterfaceFactory object,
 * which owns a CpMediaInterfaceFactoryImpl object, which this function also
 * creates.
 * This function must be implemented for all "plug-in" static sipx media processing
 * libraries.
 */
extern "C" CpMediaInterfaceFactory* sipXmediaFactoryFactory(OsConfigDb* pConfigDb);

/**
 * Destroy the singleton media factory
 */
extern "C" void sipxDestroyMediaFactoryFactory() ;

// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

#endif
