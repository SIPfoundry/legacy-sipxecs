//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ProvisioningClass_h_
#define _ProvisioningClass_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <utl/UtlContainable.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class TiXmlNode;
class TiXmlDocument;
class ProvisioningServer;
class ProvisioningAttrList;


class ProvisioningClass : public UtlContainable {
public:
/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

   /**
    * Default constructor
    */
   ProvisioningClass(const char* pClassName);

   /**
    * Destructor
    */
   virtual ~ProvisioningClass();

/* ============================ MANIPULATORS ============================== */
   virtual ProvisioningAttrList* Create(ProvisioningAttrList& rRequestAttributes);
   virtual ProvisioningAttrList* Delete(ProvisioningAttrList& rRequestAttributes);
   virtual ProvisioningAttrList* Set(ProvisioningAttrList& rRequestAttributes);
   virtual ProvisioningAttrList* Get(ProvisioningAttrList& rRequestAttributes);
   virtual ProvisioningAttrList* Action(ProvisioningAttrList& rRequestAttributes);

   void    setXmlConfigDoc(TiXmlDocument* pConfigDoc);

   virtual bool loadConfiguration(void);

/* ============================ ACCESSORS ================================= */

   TiXmlNode* findPSInstance(const char* pClassName, const char* pIndexAttr = NULL, const char* pIndexValue = NULL);
   TiXmlNode* createPSInstance(const char* pClassName, const char* pIndexAttr, const char* pIndexValue);
   bool       deletePSInstance(const char* pClassName, const char* pIndexAttr, const char* pIndexValue);

   bool       setPSAttribute(TiXmlNode* pClassInstance, const char* pAttribute, UtlString& rValue);
   bool       setPSAttribute(TiXmlNode* pClassInstance, const char* pAttribute, const char* pValue);
   bool       setPSAttribute(TiXmlNode* pClassInstance, const char* pAttribute, int value);
   bool       setPSAttribute(TiXmlNode* pClassInstance, const char* pAttribute, bool value);

//   bool       getPSAttribute(TiXmlNode* pClassInstance, const char* pAttribute, const char*& prValue);
   bool       getPSAttribute(TiXmlNode* pClassInstance, const char* pAttribute, UtlString& rValue);
   bool       getPSAttribute(TiXmlNode* pClassInstance, const char* pAttribute, int& rValue);
   bool       getPSAttribute(TiXmlNode* pClassInstance, const char* pAttribute, bool& rValue);

   bool       deletePSAttribute(TiXmlNode* pClassInstance, const char* pAttribute);

   const UtlString* getClassName(void);

   /**
    * Calculate a unique hash code for this object.  If the equals
    * operator returns true for another object, then both of those
    * objects must return the same hashcode.
    */
   virtual unsigned hash() const;

   /**
    * Get the ContainableType for a UtlContainable derived class.
    */
   virtual UtlContainableType getContainableType() const;

/* ============================ INQUIRY =================================== */

   /**
    * Compare the this object to another like-objects.  Results for
    * designating a non-like object are undefined.
    *
    * @returns 0 if equal, < 0 if less then and >0 if greater.
    */
   virtual int compareTo(UtlContainable const *) const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   static const UtlContainableType TYPE;           /** < Class type used for runtime checking */

   TiXmlDocument*      mpXmlConfigDoc;       /* The TinyXml persistant store for
                                              * this Provisioning Class. */
   ProvisioningServer* mpProvisioningServer; /* The reference to the Provisioning
                                              * Server that this is registered with. */
   bool                mConfigurationLoaded; /** < Flag indicating if configuration has been loaded. */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   UtlString           mClassName;           /* The name of the provisioning class
                                              * that this is managing. */
};

#endif  // _ProvisioningClass_h_
