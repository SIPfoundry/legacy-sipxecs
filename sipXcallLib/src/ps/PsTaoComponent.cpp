//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "ps/PsTaoComponent.h"
#include <os/OsLock.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PsTaoComponent::PsTaoComponent() :
mMutex(OsMutex::Q_FIFO)
{
}

PsTaoComponent::PsTaoComponent(const UtlString& rComponentName, int componentType) :
mMutex(OsMutex::Q_FIFO),
mName(rComponentName),
mType(componentType)
{
}

// Copy constructor
PsTaoComponent::PsTaoComponent(const PsTaoComponent& rPsTaoComponent) :
mMutex(OsMutex::Q_FIFO)
{
}

// Destructor
PsTaoComponent::~PsTaoComponent()
{
        mName.remove(0);
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PsTaoComponent&
PsTaoComponent::operator=(const PsTaoComponent& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/* ============================ ACCESSORS ================================= */
void PsTaoComponent::getName(UtlString& rName)
{
        rName = mName;
}

int PsTaoComponent::getType(void)
{
        return mType;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
OsMutex* PsTaoComponent::getMutex(void)
{
        return &mMutex;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
