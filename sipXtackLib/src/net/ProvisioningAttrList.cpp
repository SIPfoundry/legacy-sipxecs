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
#include <utl/UtlInt.h>
#include <utl/UtlBool.h>
#include <utl/UtlString.h>
#include <utl/UtlHashMap.h>
#include <utl/UtlSList.h>
#include <utl/UtlContainable.h>
#include <utl/UtlSListIterator.h>
#include <utl/UtlHashMapIterator.h>
#include "net/ProvisioningAttrList.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningAttrList::ProvisioningAttrList
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

ProvisioningAttrList::ProvisioningAttrList(void)
{
   mpData = new UtlHashMap;
   mIsReference = false;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningAttrList::ProvisioningAttrList
//
//  SYNOPSIS:
//
//  DESCRIPTION: Reference constructor
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ProvisioningAttrList::ProvisioningAttrList(UtlHashMap* pData)
{
   mpData = pData;
   mIsReference = true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningAttrList::~ProvisioningAttrList
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

ProvisioningAttrList::~ProvisioningAttrList()
{
   if (!mIsReference) {
      deleteAttrElements(dynamic_cast<UtlContainable*>(mpData));
      delete mpData;
   }
}

/* ============================ MANIPULATORS ============================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningAttrList::setAttribute
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

bool ProvisioningAttrList::setAttribute(const char* pKey, UtlSList* pValue)
{
   UtlContainable* results;

   results = mpData->insertKeyAndValue(new UtlString(pKey), pValue);

   if (results == NULL) {
      return false;
   }
   else {
      return true;
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningAttrList::setAttribute
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

bool ProvisioningAttrList::setAttribute(const char* pKey, const UtlString& rValue)
{
   UtlContainable* results;

   results = mpData->insertKeyAndValue(new UtlString(pKey), new UtlString(rValue));

   if (results == NULL) {
      return false;
   }
   else {
      return true;
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningAttrList::setAttribute
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

bool ProvisioningAttrList::setAttribute(const char* pKey, const char* pValue)
{
   UtlContainable* results;

   results = mpData->insertKeyAndValue(new UtlString(pKey), new UtlString(pValue));

   if (results == NULL) {
      return false;
   }
   else {
      return true;
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningAttrList::setAttribute
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

bool ProvisioningAttrList::setAttribute(const char* pKey, int value)
{
   UtlContainable* results;

   results = mpData->insertKeyAndValue(new UtlString(pKey), new UtlInt(value));

   if (results == NULL) {
      return false;
   }
   else {
      return true;
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningAttrList::setAttribute
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

bool ProvisioningAttrList::setAttribute(const char* pKey, bool value)
{
   UtlContainable* results;

   results = mpData->insertKeyAndValue(new UtlString(pKey), new UtlBool(value));

   if (results == NULL) {
      return false;
   }
   else {
      return true;
   }
}


/* ============================ ACCESSORS ================================= */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningAttrList::getAttribute
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

UtlContainable* ProvisioningAttrList::getAttribute(const char* pKey)
{
   UtlString utlKey(pKey);

   return dynamic_cast<const UtlHashMap*>(mpData)->findValue(&utlKey);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningAttrList::getAttribute
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

bool ProvisioningAttrList::getAttribute(const char* pKey, UtlString& rValue)
{
   UtlString utlKey(pKey);
   UtlContainable* results;

   results = dynamic_cast<const UtlHashMap*>(mpData)->findValue(&utlKey);
   if (results == NULL) {
      return false;
   }

   if (UtlString(results->getContainableType()) != "UtlString") {
      return false;
   }

   rValue = dynamic_cast<UtlString*>(results)->data();

   return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningAttrList::getAttribute
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

bool ProvisioningAttrList::getAttribute(const char* pKey, const char*& prValue)
{
   UtlString utlKey(pKey);
   UtlContainable* results;

   results = dynamic_cast<const UtlHashMap*>(mpData)->findValue(&utlKey);
   if (results == NULL) {
      return false;
   }

   if (UtlString(results->getContainableType()) != "UtlString") {
      return false;
   }

   prValue = dynamic_cast<UtlString*>(results)->data();

   return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningAttrList::getAttribute
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

bool ProvisioningAttrList::getAttribute(const char* pKey, int& rValue)
{
   UtlString utlKey(pKey);
   UtlContainable* results;

   results = dynamic_cast<const UtlHashMap*>(mpData)->findValue(&utlKey);
   if (results == NULL) {
      return false;
   }

   if (UtlString(results->getContainableType()) != "UtlInt") {
      return false;
   }

   rValue = dynamic_cast<UtlInt*>(results)->getValue();

   return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningAttrList::getAttribute
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

bool ProvisioningAttrList::getAttribute(const char* pKey, bool& rValue)
{
   UtlString utlKey(pKey);
   UtlContainable* results;

   results = dynamic_cast<const UtlHashMap*>(mpData)->findValue(&utlKey);
   if (results == NULL) {
      return false;
   }

   if (UtlString(results->getContainableType()) != "UtlBool") {
      return false;
   }

   rValue = dynamic_cast<UtlBool*>(results)->getValue();

   return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningAttrList::dumpAttributes
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

void ProvisioningAttrList::dumpAttributes(void)
{
   dumpAttributes(dynamic_cast<UtlContainable*>(mpData));
}


/* ============================ INQUIRY =================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningAttrList::attributeMissing
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

bool ProvisioningAttrList::attributeMissing(const char* pKey)
{
   UtlString utlKey(pKey);
   return !dynamic_cast<const UtlHashMap*>(mpData)->contains(&utlKey);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningAttrList::attributePresent
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

bool ProvisioningAttrList::attributePresent(const char* pKey)
{
   UtlString utlKey(pKey);
   return dynamic_cast<const UtlHashMap*>(mpData)->contains(&utlKey);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningAttrList::validateAttribute
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

void ProvisioningAttrList::validateAttribute(const char* pKey, eAttributeType type, bool ignoreMissing)
{
   UtlContainable* attribute;
   UtlString utlKey(pKey);

   attribute = dynamic_cast<const UtlHashMap*>(mpData)->findValue(&utlKey);
   if (attribute == NULL) {
      if (ignoreMissing) {
         // If it is missing, just return
         return;
      }
      else {
         UtlString errorText("Missing attribute: '");
         errorText += pKey;
         errorText += "'";
         throw errorText;
      }
   }

   switch (type) {
      case ProvisioningAttrList::INT:
         if (UtlString(attribute->getContainableType()) != "UtlInt") {
            UtlString errorText("Attribute '");
            errorText += pKey;
            errorText += "' must be of type: INT";
            throw errorText;
         }
         break;
      case ProvisioningAttrList::BOOL:
         if (UtlString(attribute->getContainableType()) != "UtlBool") {
            UtlString errorText("Attribute: '");
            errorText += pKey;
            errorText += "' must be of type: BOOL";
            throw errorText;
         }
         break;
      case ProvisioningAttrList::STRING:
         if (UtlString(attribute->getContainableType()) != "UtlString") {
            UtlString errorText("Attribute: '");
            errorText += pKey;
            errorText += "' must be of type: STRING";
            throw errorText;
         }
         break;
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningAttrList::validateAttributeType
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

void ProvisioningAttrList::validateAttributeType(const char* pKey, eAttributeType type)
{
   this->validateAttribute(pKey, type, true);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningAttrList::deleteAttrElements
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

void ProvisioningAttrList::deleteAttrElements(UtlContainable* pAttrElements)
{
   UtlString*         pMemberName;
   UtlContainable*    pMemberValue;

   if (UtlString(pAttrElements->getContainableType()) == "UtlHashMap") {
      UtlHashMapIterator structureIterator(*dynamic_cast<UtlHashMap*>(pAttrElements));
      while ((pMemberName = dynamic_cast<UtlString*>(structureIterator())) != NULL) {
         pMemberValue = dynamic_cast<UtlHashMap*>(pAttrElements)->findValue(pMemberName);
         if (UtlString(pMemberValue->getContainableType()) == "UtlHashMap"
             || UtlString(pMemberValue->getContainableType()) == "UtlSList") {
            deleteAttrElements(pMemberValue);
         }

         delete pMemberName;
         delete pMemberValue;
      }
   }
   else if (UtlString(pAttrElements->getContainableType()) == "UtlSList") {
      UtlSListIterator arrayIterator(*dynamic_cast<UtlSList*>(pAttrElements));
      while ((pMemberValue = arrayIterator()) != NULL) {
         if (UtlString(pMemberValue->getContainableType()) == "UtlHashMap"
             || UtlString(pMemberValue->getContainableType()) == "UtlSList") {
            deleteAttrElements(pMemberValue);
         }

         delete pMemberValue;
      }
   }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningAttrList::dumpAttributes
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

void ProvisioningAttrList::dumpAttributes(const UtlContainable* pAttribute)
{
   static UtlString*  pMemberName;
   UtlContainable*    pMemberValue;

   if (UtlString(pAttribute->getContainableType()) == "UtlHashMap") {
      UtlHashMapIterator structureIterator(*dynamic_cast<const UtlHashMap*>(pAttribute));
      while ((pMemberName = dynamic_cast<UtlString*>(structureIterator())) != NULL) {
         pMemberValue = dynamic_cast<const UtlHashMap*>(pAttribute)->findValue(pMemberName);
         if (UtlString(pMemberValue->getContainableType()) == "UtlHashMap"
             || UtlString(pMemberValue->getContainableType()) == "UtlSList") {
            dumpAttributes(pMemberValue);
         }

         if (UtlString(pMemberValue->getContainableType()) == "UtlBool") {
            osPrintf("{%s} = (BOOL) %s\n",
                   pMemberName->data(),
                   (dynamic_cast<UtlBool*>(pMemberValue)->getValue() ? "TRUE" : "FALSE"));
         }
         else if (UtlString(pMemberValue->getContainableType()) == "UtlInt") {
            osPrintf("{%s} = (INT) %" PRIdPTR "\n",
                   pMemberName->data(),
                   dynamic_cast<UtlInt*>(pMemberValue)->getValue());
         }
         else if (UtlString(pMemberValue->getContainableType()) == "UtlString") {
            osPrintf("{%s} = (STRING) \"%s\"\n",
                   pMemberName->data(),
                   dynamic_cast<UtlString*>(pMemberValue)->data());
         }
      }
   }
   else if (UtlString(pAttribute->getContainableType()) == "UtlSList") {
      UtlSListIterator arrayIterator(*dynamic_cast<const UtlSList*>(pAttribute));
      int arrayIndex = 0;
      while ((pMemberValue = arrayIterator()) != NULL) {
         if (UtlString(pMemberValue->getContainableType()) == "UtlHashMap"
             || UtlString(pMemberValue->getContainableType()) == "UtlSList") {
            dumpAttributes(pMemberValue);
         }

         if (UtlString(pMemberValue->getContainableType()) == "UtlBool") {
            osPrintf("{%s}[%d] = (BOOL) %s\n",
                   pMemberName->data(),
                   arrayIndex++,
                   (dynamic_cast<UtlBool*>(pMemberValue)->getValue() ? "TRUE" : "FALSE"));
         }
         else if (UtlString(pMemberValue->getContainableType()) == "UtlInt") {
            osPrintf("{%s}[%d] = (INT) %" PRIdPTR "\n",
                   pMemberName->data(),
                   arrayIndex++,
                   dynamic_cast<UtlInt*>(pMemberValue)->getValue());
         }
         else if (UtlString(pMemberValue->getContainableType()) == "UtlString") {
            osPrintf("{%s}[%d] = (STRING) \"%s\"\n",
                   pMemberName->data(),
                   arrayIndex++,
                   dynamic_cast<UtlString*>(pMemberValue)->data());
         }
      }
   }
}

/* ============================ FUNCTIONS ================================= */
