//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsSysLog.h"
#include "utl/UtlSListIterator.h"

#include "xmlparser/tinyxml.h"
#include "xmlparser/XmlErrorMsg.h"
#include "xmlparser/ExtractContent.h"

#include "SipxProcess.h"

#include "SipxProcessResource.h"
#include "FileResource.h"
#include "DirectoryResource.h"
#include "ImdbResource.h"
#include "SqldbResource.h"
#include "SipxResource.h"

// DEFINES
// CONSTANTS
UtlContainableType SipxResource::TYPE = "SipxResource";

const char* SipxResource::RequiredAttributeName     = "required";
const char* SipxResource::ConfigAccessAttributeName = "configAccess";

// TYPEDEFS
// FORWARD DECLARATIONS


/// Factory method that parses a resource description element.
bool SipxResource::parse(const TiXmlDocument& processDefinitionDoc,
                         TiXmlElement* resourceElement, ///< some child element of 'resources'.
                         SipxProcess* currentProcess        ///< SipxProcess whose resources are being read.
                         )
{
   /*
    * This is called by SipxProcess::createFromDefinition with each child of the 'resources' element
    * in a process definition.  Based on the element name, this method calls the 'parse'
    * method in the appropriate subclass.
    *
    * @returns whether or not the element was valid.
    */
   bool resourceElementIsValid = false;
   const char* resourceTypeName = resourceElement->Value();

   if (0==strcmp(resourceTypeName,SipxProcessResource::SipxProcessResourceTypeName))
   {
      resourceElementIsValid =
         SipxProcessResource::parse(processDefinitionDoc, resourceElement, currentProcess);
   }
   else if (0==strcmp(resourceTypeName,ImdbResource::ImdbResourceTypeName))
   {
      resourceElementIsValid =
         ImdbResource::parse(processDefinitionDoc, resourceElement, currentProcess);
   }
   else if (0==strcmp(resourceTypeName,SqldbResource::SqldbResourceTypeName))
   {
      resourceElementIsValid =
         SqldbResource::parse(processDefinitionDoc, resourceElement, currentProcess);
   }
   else if (   (0==strcmp(resourceTypeName,FileResource::FileResourceTypeName))
            || (0==strcmp(resourceTypeName,FileResource::OsconfigResourceTypeName))
            )
   {
      resourceElementIsValid =
         FileResource::parse(processDefinitionDoc, resourceElement, currentProcess);
   }
   else if (0==strcmp(resourceTypeName,DirectoryResource::DirectoryResourceTypeName))
   {
      resourceElementIsValid =
         DirectoryResource::parse(processDefinitionDoc, resourceElement, currentProcess);
   }
   else
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxResource::parse unknown resource type '%s'",
                    resourceTypeName);
      resourceElementIsValid = false;
   }

   return resourceElementIsValid;
}

/// get a description of the SipxResource (for use in logging)
void SipxResource::appendDescription(UtlString&  description /**< returned description */) const
{
   OsSysLog::add(FAC_SUPERVISOR, PRI_CRIT, "SipxResource::appendDescription called.");
   description.append("[ERROR - SipxResource::appendDescription called]");
}


// ================================================================
/** @name           Status Operations
 *
 */
///@{

/// Whether or not the SipxResource is ready for use by a SipxProcess.
bool SipxResource::isReadyToStart(UtlString& missingResource)
{
   missingResource="ERROR: resource is missing isReadyToStart method";
   OsSysLog::add(FAC_SUPERVISOR, PRI_CRIT, "SipxResource::isReadyToStart called.");
   return false;
}


/// Whether or not it is safe to stop a SipxProcess using the SipxResource.
bool SipxResource::isSafeToStop()
{
   return true;   // assume this is ok
}


/// Some change has been made to this resource; notify any SipxProcesses that use it.
void SipxResource::modified()
{
   UtlSListIterator processResources(mUsedBy);
   SipxProcessResource* processResource;

   while ((processResource = dynamic_cast<SipxProcessResource*>(processResources())))
   {
      SipxProcess* process;
      if ((process=processResource->getProcess()))
      {
         process->configurationChange(*this);
      }
      else
      {
         /* there is no process for this process resource
          * - can happen when a definition is invalid, and has already been logged.
          */
      }
   }
}

/// Whether or not the SipxResource may be written by configuration update methods.
bool SipxResource::isWriteable()
{
   return ( mImplicitAccess || ( mAccess & WriteAccess ) );
}

bool SipxResource::isReadable()
{
   return ( mImplicitAccess || ( mAccess & ReadAccess ) );
}

/// Determine whether or not the values in a containable are comparable.
UtlContainableType SipxResource::getContainableType() const
{
   return TYPE;
}

/// constructor
SipxResource::SipxResource(const char* uniqueId) :
   UtlString(uniqueId),
   mFirstDefinition(true),
   mImplicitAccess(true),
   mAccess(ReadAccess+WriteAccess)
{
}

void SipxResource::usedBy(SipxProcess* currentProcess)
{
   /*
    * When a SipxProcess creates its own SipxProcessResource,
    * it passes NULL rather than require itself...
    */
   if (currentProcess)
   {
      mUsedBy.append(currentProcess->resource());
      currentProcess->requireResource(this);
   }
}

/// Does the name in this DOM attribute match attributeName?
bool SipxResource::isAttribute(const TiXmlAttribute* attribute,
                               const char* attributeName
                               )
{
   return 0==strcmp(attributeName, attribute->Name());
}


/// Parses attributes common to all SipxResource classes.
bool SipxResource::parseAttribute(const TiXmlDocument& document,
                                  const TiXmlAttribute* attribute,
                                  SipxProcess* currentProcess
                                  )
{
   /**<
    * This method should be called for each attribute child of a 'resource' type node.
    * @returns true iff the attribute was recognized and handled by the SipxResource class.
    * If this returns false, then the subclass must attempt to parse the attribute
    * as one specific to its subclass.
    */
   bool attributeIsValid = false;

   UtlString attributeName(attribute->Name());
   UtlString attributeValue(attribute->Value());

   UtlString errorMsg;

   if (0==attributeName.compareTo(RequiredAttributeName, UtlString::matchCase))
   {
      if (0==attributeValue.compareTo("true", UtlString::ignoreCase))
      {
         attributeIsValid = true;
      }
      else if (0==attributeValue.compareTo("false", UtlString::ignoreCase))
      {
         currentProcess->resourceIsOptional(this);
         attributeIsValid = true;
      }
      else
      {
         XmlErrorMsg(document, errorMsg);
         OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxResource::parseAttribute "
                       "invalid value '%s' for '%s' attribute %s",
                       attributeValue.data(), RequiredAttributeName, errorMsg.data());
      }
   }
   else if (0==attributeName.compareTo(ConfigAccessAttributeName, UtlString::matchCase))
   {
      if (0==attributeValue.compareTo("read-write", UtlString::ignoreCase))
      {
         mImplicitAccess = false;
         mAccess = ReadAccess+WriteAccess;
         attributeIsValid = true;
      }
      else if (0==attributeValue.compareTo("write-only", UtlString::ignoreCase))
      {
         if (mFirstDefinition)
         {
            mAccess = WriteAccess;
         }
         else if (mImplicitAccess)
         {
            // implicit is read-write, so this is ok
         }
         else
         {
            mAccess |= WriteAccess;
         }
         mImplicitAccess = false;            
         attributeIsValid = true;
      }
      else if (0==attributeValue.compareTo("read-only", UtlString::ignoreCase))
      {
         if (mFirstDefinition)
         {
            mAccess = ReadAccess;
         }
         else if (mImplicitAccess)
         {
            // implicit is read-write, so this is ok
         }
         else
         {
            mAccess |= ReadAccess;
         }
         mImplicitAccess = false;            
         attributeIsValid = true;
      }
      else if (0==attributeValue.compareTo("no-access", UtlString::ignoreCase))
      {
         if (mFirstDefinition || mAccess == 0)
         {
            mAccess = 0;
         }
         else
         {
            UtlString description;
            appendDescription(description);
            
            OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxResource::parseAttribute "
                          "conflicting access for '%s': resolved in favor of no-access",
                          description.data());
         }
         mImplicitAccess = false;            
         attributeIsValid = true;
      }
      else
      {
         XmlErrorMsg(document, errorMsg);
         OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxResource::parseAttribute "
                       "invalid value '%s' for '%s' attribute %s",
                       attributeValue.data(), ConfigAccessAttributeName, errorMsg.data());
      }
   }
   else
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG, "SipxResource::parseAttribute "
                    "unrecognized attribute '%s'",
                    attributeName.data());
   }

   return attributeIsValid;
}

/// destructor
SipxResource::~SipxResource()
{
};
