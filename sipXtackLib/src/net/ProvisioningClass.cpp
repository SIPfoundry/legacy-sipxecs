//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <xmlparser/tinyxml.h>
#include "net/ProvisioningAttrList.h"
#include "net/ProvisioningClass.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const UtlContainableType ProvisioningClass::TYPE = "ProvisioningClass";

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningClass::ProvisioningClass
//
//  SYNOPSIS:
//
//  DESCRIPTION: Default constructor
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ProvisioningClass::ProvisioningClass(const char* pClassName)
: mClassName(pClassName)
{
   mConfigurationLoaded = false;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningClass::~ProvisioningClass
//
//  SYNOPSIS:
//
//  DESCRIPTION: Destructor
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ProvisioningClass::~ProvisioningClass()
{
}

/* ============================ MANIPULATORS ============================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningClass::setXmlConfigDoc
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ProvisioningClass::setXmlConfigDoc(TiXmlDocument* pConfigDoc)
{
   mpXmlConfigDoc = pConfigDoc;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningClass::loadConfiguration
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

bool ProvisioningClass::loadConfiguration(void)
{
   return false;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningClass::Create
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ProvisioningAttrList* ProvisioningClass::Create(ProvisioningAttrList& rRequestAttributes)
{
   return NULL;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningClass::Delete
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ProvisioningAttrList* ProvisioningClass::Delete(ProvisioningAttrList& rRequestAttributes)
{
   return NULL;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningClass::Set
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ProvisioningAttrList* ProvisioningClass::Set(ProvisioningAttrList& rRequestAttributes)
{
   return NULL;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningClass::Get
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ProvisioningAttrList* ProvisioningClass::Get(ProvisioningAttrList& rRequestAttributes)
{
   return NULL;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningClass::Action
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ProvisioningAttrList* ProvisioningClass::Action(ProvisioningAttrList& rRequestAttributes)
{
   return NULL;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningClass::deletePSInstance
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

bool ProvisioningClass::deletePSInstance(const char* pClassName, const char* pIndexAttr, const char* pIndexValue)
{
   bool        results = false;
   UtlString   fullClassName(pClassName);
   TiXmlHandle docHandle(mpXmlConfigDoc);
   TiXmlNode*  pClassNode;
   TiXmlNode*  pInstanceNode;
   TiXmlNode*  pIndexNode;
   TiXmlNode*  pIndexValueNode;

   // Append the "-class" to the end of the class name given.
   fullClassName += "-class";

   pClassNode = docHandle.FirstChild("sipxacd").FirstChild(fullClassName.data()).Node();
   if (pClassNode == NULL) {
      // The class does not exist.
      return false;
   }

   for (pInstanceNode = pClassNode->FirstChild(); pInstanceNode; pInstanceNode = pInstanceNode->NextSibling()) {
      pIndexNode = pInstanceNode->FirstChild(pIndexAttr);
      if (pIndexNode == NULL) {
         // Index attribute not found. Keep looking.
         continue;
      }
      pIndexValueNode = pIndexNode->FirstChild();
      if (pIndexValueNode == NULL) {
         // Index attribute value not found. Keep looking.
         continue;
      }
      if (strcmp(pIndexValueNode->Value(), pIndexValue) == 0) {
         // Index attribute value match. Delete it.
         pClassNode->RemoveChild(pInstanceNode);
         results = true;
         break;
      }
   }

   return results;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningClass::createPSInstance
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

TiXmlNode* ProvisioningClass::createPSInstance(const char* pClassName, const char* pIndexAttr, const char* pIndexValue)
{
   UtlString   fullClassName(pClassName);
   TiXmlHandle docHandle(mpXmlConfigDoc);
   TiXmlNode*  pRootNode;
   TiXmlNode*  pClassNode;
   TiXmlNode*  pInstanceNode;
   TiXmlNode*  pIndexNode;
   TiXmlNode*  pIndexValueNode;

   // Append the "-class" to the end of the class name given.
   fullClassName += "-class";

   // First make sure that the instance does not already exist.
   pInstanceNode = findPSInstance(pClassName, pIndexAttr, pIndexValue);
   if (pInstanceNode != NULL) {
      // Already exists
      return pInstanceNode;
   }

   // Find the class
   pClassNode = docHandle.FirstChild("sipxacd").FirstChild(fullClassName.data()).Node();
   if (pClassNode == NULL) {
      // The class does not exist, create it.
      pRootNode = mpXmlConfigDoc->FirstChild("sipxacd");
      TiXmlElement classElement(fullClassName.data());
      pClassNode = pRootNode->InsertEndChild(classElement);
      if (pClassNode == NULL) {
         // Insertion failed.
         return NULL;
      }
   }

   // Create the instance.
   TiXmlElement instanceElement(pClassName);
   pInstanceNode = pClassNode->InsertEndChild(instanceElement);
   if (pInstanceNode == NULL) {
      // Insertion failed.
      return NULL;
   }

   // Create the index.
   TiXmlElement indexElement(pIndexAttr);
   pIndexNode = pInstanceNode->InsertEndChild(indexElement);
   if (pIndexNode == NULL) {
      // Insertion failed.
      return NULL;
   }

   // Create the index value.
   TiXmlText indexValueText(pIndexValue);
   pIndexValueNode = pIndexNode->InsertEndChild(indexValueText);
   if (pIndexValueNode == NULL) {
      // Insertion failed.
      return NULL;
   }

   return pInstanceNode;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningClass::setPSAttribute
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

bool ProvisioningClass::setPSAttribute(TiXmlNode* pClassInstance, const char* pAttribute, UtlString& rValue)
{
   return setPSAttribute(pClassInstance, pAttribute, rValue.data());
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningClass::setPSAttribute
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

bool ProvisioningClass::setPSAttribute(TiXmlNode* pClassInstance, const char* pAttribute, const char* pValue)
{
   TiXmlNode*  pAttributeNode;
   TiXmlNode*  pAttributeValueNode;

   // Find the attribute
   pAttributeNode = pClassInstance->FirstChild(pAttribute);
   if (pAttributeNode == NULL) {
      // The attribute does not exist, create it.
      TiXmlElement attributeElement(pAttribute);
      pAttributeNode = pClassInstance->InsertEndChild(attributeElement);
      if (pAttributeNode == NULL) {
         // Insertion failed.
         return false;
      }
   }

   // See if the attribute value exists
   pAttributeValueNode = pAttributeNode->FirstChild();
   if (pAttributeValueNode == NULL) {
      // The attribute value does not exist, create it.
      TiXmlText attributeValueText(pValue);
      pAttributeValueNode = pAttributeNode->InsertEndChild(attributeValueText);
      if (pAttributeValueNode == NULL) {
         // Insertion failed.
         return false;
      }
   }
   else {
      pAttributeValueNode->SetValue(pValue);
   }

   return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningClass::setPSAttribute
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

bool ProvisioningClass::setPSAttribute(TiXmlNode* pClassInstance, const char* pAttribute, int value)
{
   char valueString[16];

   snprintf(valueString, 16, "%d", value);
   return setPSAttribute(pClassInstance, pAttribute, valueString);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningClass::setPSAttribute
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

bool ProvisioningClass::setPSAttribute(TiXmlNode* pClassInstance, const char* pAttribute, bool value)
{
   if (value == true) {
      return setPSAttribute(pClassInstance, pAttribute, "TRUE");
   }
   else {
      return setPSAttribute(pClassInstance, pAttribute, "FALSE");
   }
}


/* ============================ ACCESSORS ================================= */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningClass::getClassName
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

const UtlString* ProvisioningClass::getClassName(void)
{
   return &mClassName;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningClass::getPSAttribute
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

bool ProvisioningClass::getPSAttribute(TiXmlNode* pClassInstance, const char* pAttribute, UtlString& rValue)
{
   TiXmlNode*  pAttributeNode;
   TiXmlNode*  pAttributeValueNode;

   // Find the attribute
   pAttributeNode = pClassInstance->FirstChild(pAttribute);
   if (pAttributeNode == NULL) {
      // The attribute does not exist.
      return false;
   }

   // See if the attribute value exists
   pAttributeValueNode = pAttributeNode->FirstChild();
   if (pAttributeValueNode == NULL) {
      // The attribute value does not exist, interpret as NULL
      rValue = NULL;
      return true;
   }
   else {
      rValue = pAttributeValueNode->Value();
   }

   return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningClass::getPSAttribute
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

bool ProvisioningClass::getPSAttribute(TiXmlNode* pClassInstance, const char* pAttribute, int& rValue)
{
   UtlString stringValue;

   if (!getPSAttribute(pClassInstance, pAttribute, stringValue)) {
      // Failed to retrieve attribute value.
      return false;
   }

   rValue = atoi(stringValue.data());

   return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningClass::getPSAttribute
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

bool ProvisioningClass::getPSAttribute(TiXmlNode* pClassInstance, const char* pAttribute, bool& rValue)
{
   UtlString stringValue;

   if (!getPSAttribute(pClassInstance, pAttribute, stringValue)) {
      // Failed to retrieve attribute value.
      return false;
   }

   if (stringValue == "TRUE") {
      rValue = true;
   }
   else {
      rValue = false;
   }

   return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningClass::findPSInstance
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

TiXmlNode* ProvisioningClass::findPSInstance(const char* pClassName, const char* pIndexAttr, const char* pIndexValue)
{
   UtlString   fullClassName(pClassName);
   TiXmlHandle docHandle(mpXmlConfigDoc);
   TiXmlNode*  pClassNode;
   TiXmlNode*  pInstanceNode;
   TiXmlNode*  pIndexNode;
   TiXmlNode*  pIndexValueNode;

   // Append the "-class" to the end of the class name given.
   fullClassName += "-class";

   pClassNode = docHandle.FirstChild("sipxacd").FirstChild(fullClassName.data()).Node();
   if (pClassNode == NULL) {
      // The class does not exist.
      return NULL;
   }

   // See if this is a FIND FIRST operation.
   if (pIndexAttr == NULL) {
      return pClassNode->FirstChild();
   }

   for (pInstanceNode = pClassNode->FirstChild(); pInstanceNode; pInstanceNode = pInstanceNode->NextSibling()) {
      pIndexNode = pInstanceNode->FirstChild(pIndexAttr);
      if (pIndexNode == NULL) {
         // Index attribute not found. Keep looking.
         continue;
      }
      pIndexValueNode = pIndexNode->FirstChild();
      if (pIndexValueNode == NULL) {
         // Index attribute value not found. Keep looking.
         continue;
      }
      if (strcmp(pIndexValueNode->Value(), pIndexValue) == 0) {
         // Index attribute value match.
         break;
      }
   }

   return pInstanceNode;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningClass::deletePSAttribute
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

bool ProvisioningClass::deletePSAttribute(TiXmlNode* pClassInstance, const char* pAttribute)
{
    TiXmlNode*  pAttributeNode;

    // Find the attribute
    pAttributeNode = pClassInstance->FirstChild(pAttribute);
    if (pAttributeNode == NULL) {
       // The attribute does not exist.
       return false;
    }
    else {
        pClassInstance->RemoveChild(pAttributeNode);
        return true ;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningClass::hash
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned ProvisioningClass::hash() const
{
   return mClassName.hash();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningClass::getContainableType
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

UtlContainableType ProvisioningClass::getContainableType() const
{
   return ProvisioningClass::TYPE;
}

/* ============================ INQUIRY =================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningClass::compareTo
//
//  SYNOPSIS:
//
//  DESCRIPTION: Compare the this object to another like-object.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

int ProvisioningClass::compareTo(UtlContainable const* pInVal) const
{
   int result ;

   if (pInVal->isInstanceOf(ProvisioningClass::TYPE)) {
      result = mClassName.compareTo(((ProvisioningClass*)pInVal)->getClassName());
   }
   else {
      result = -1;
   }

   return result;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
