//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

#include <stdlib.h>

// APPLICATION INCLUDES

#include "ResourceReference.h"
#include "ContactSet.h"
#include "SubscriptionSet.h"
#include <os/OsSysLog.h>
#include <os/OsLock.h>
#include <utl/XmlContent.h>
#include <utl/UtlSListIterator.h>
#include <net/SipDialogEvent.h>
#include <net/NameValueTokenizer.h>
#include <net/NameValuePair.h>
#include <net/HttpMessage.h>
#include <net/SipMessage.h>
#include <net/SipDialogEvent.h>
#include <xmlparser/tinyxml.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

const UtlContainableType ResourceReference::TYPE = "ResourceReference";


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
ResourceReference::ResourceReference(ResourceList* resourceList,
                                     const char* uri,
                                     const char* nameXml,
                                     const char* display_name) :
   mResourceList(resourceList),
   mNameXml(nameXml),
   mDisplayName(display_name)
{
}

// Destructor
ResourceReference::~ResourceReference()
{
}

/* ============================ MANIPULATORS ============================== */

// Add to the HttpBody the current state of the resource.
void ResourceReference::generateBody(UtlString& rlmi,
                                     HttpBody& body,
                                     UtlBoolean consolidated) const
{
   // Pass the name strings to ResourceCached::generateBody.
   mResourceCached->generateBody(rlmi, body, consolidated,
                                 mNameXml, mDisplayName);
}

/* ============================ ACCESSORS ================================= */

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ INQUIRY =================================== */

// Dump the object's internal state.
void ResourceReference::dumpState() const
{
   // indented 6

   OsSysLog::add(FAC_RLS, PRI_INFO,
                 "\t      ResourceReference %p mResourceCached = %p ('%s'), mDisplayName = '%s', mNameXml = '%s'",
                 this, mResourceCached, mResourceCached->data(),
                 mDisplayName.data(), mNameXml.data());
}

/**
 * Get the ContainableType for a UtlContainable-derived class.
 */
UtlContainableType ResourceReference::getContainableType() const
{
   return ResourceReference::TYPE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */
