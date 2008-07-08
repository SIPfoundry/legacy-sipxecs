// 
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "SipxResource.h"

// DEFINES
// CONSTANTS
UtlContainableType SipxResource::TYPE = "SipxResource";

// TYPEDEFS
// FORWARD DECLARATIONS


/// Factory method that parses a resource description element.
bool SipxResource::parse(TiXmlElement* resourceElement, ///< some child element of 'resources'.
                         Process* currentProcess        ///< Process whose resources are being read.
                         )
{
/**<
 * This is called by Process::createFromDefinition with each child of the 'resources' element
 * in a process definition.  Based on the element name, this method calls the 'parse'
 * method in the appropriate subclass.
 *
 * @returns whether or not the element was valid.
 */
   return true;                 // @TODO 
}

/// get a description of the SipxResource (for use in logging)
void SipxResource::getDescription(UtlString&  description /**< returned description */)
{
   // @TODO 
}

   
// ================================================================
/** @name           Status Operations
 *
 */
///@{

/// Whether or not the SipxResource is ready for use by a Process.
bool SipxResource::isReadyToStart()
{
   return false;                // @TODO 
}


/// Whether or not it is safe to stop a Process using the SipxResource.
bool SipxResource::isSafeToStop()
{
   return true;                 // @TODO 
}


/// Some change has been made to this resource; notify any Processes that use it.
void SipxResource::modified()
{
   // @TODO 
}

/// Whether or not the SipxResource may be written by configuration update methods.
bool SipxResource::isWriteable()
{
   return mWritable;
}

/// Determine whether or not the values in a containable are comparable.
UtlContainableType SipxResource::getContainableType() const
{
   return TYPE;
}

/// constructor
SipxResource::SipxResource(const char* uniqueId) :
   UtlString(uniqueId)
{
   // @TODO 
}


/// Parses attributes common to all SipxResource classes.
bool SipxResource::parseAttribute(const TiXmlAttribute* attribute)
{
   /**<
    * This method should be called for each attribute child of a 'resource' type node.
    * @returns true iff the attribute was recognized and handled by the SipxResource class.
    * If this returns false, then the subclass must attempt to parse the attribute
    * as one specific to its subclass.
    */
   return false;                // @TODO 
}

/// Do any special handling when a resource is required by the process.
void SipxResource::requiredBy(Process* currentProcess)
{
   /**< this base class calls currentProcess->requireResourceToStart */
   // @TODO 
}

/// destructor
SipxResource::~SipxResource()
{
};
