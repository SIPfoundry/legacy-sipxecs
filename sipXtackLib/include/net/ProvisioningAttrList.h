//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ProvisioningAttrList_h_
#define _ProvisioningAttrList_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlHashMap;
class UtlSList;
class UtlContainable;

/**
 *
 */
class ProvisioningAttrList {
public:
/* //////////////////////////// PUBLIC //////////////////////////////////// */

   enum eAttributeType {
      INT,
      BOOL,
      STRING
   };

/* ============================ CREATORS ================================== */

   /**
    * Default constructor
    */
   ProvisioningAttrList(void);

   /**
    * Reference constructor
    */
   ProvisioningAttrList(UtlHashMap* pData);

   /**
    * Destructor
    */
   virtual ~ProvisioningAttrList();

/* ============================ MANIPULATORS ============================== */

   bool setAttribute(const char* pKey, UtlSList* pValue);
   bool setAttribute(const char* pKey, const UtlString& rValue);
   bool setAttribute(const char* pKey, const char* pValue);
   bool setAttribute(const char* pKey, int value);
   bool setAttribute(const char* pKey, bool value);

/* ============================ ACCESSORS ================================= */

   UtlContainable* getAttribute(const char* pKey);
   UtlContainable* operator[](const char* pKey) { return getAttribute(pKey);}
   bool getAttribute(const char* pKey, UtlString& rValue);
   bool getAttribute(const char* pKey, const char*& prValue);
   bool getAttribute(const char* pKey, int& rValue);
   bool getAttribute(const char* pKey, bool& rValue);
   UtlHashMap* getData(void) { return mpData;}
   void dumpAttributes(void);

/* ============================ INQUIRY =================================== */

   bool attributeMissing(const char* pKey);
   bool attributePresent(const char* pKey);
   void validateAttribute(const char* pKey, eAttributeType type, bool ignoreMissing = false);
   void validateAttributeType(const char* pKey, eAttributeType type);

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   void deleteAttrElements(UtlContainable* pAttrElements);
   static void dumpAttributes(const UtlContainable* pAttribute);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   UtlHashMap*         mpData;               /* Pointer to the containing data. */
   bool                mIsReference;         /* Is mpData a reference ? */
};

#endif  // _ProvisioningAttrList_h_
