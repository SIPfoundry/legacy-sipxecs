// 
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////
#ifndef _PROCESSRESOURCE_H_
#define _PROCESSRESOURCE_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "SipxResource.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Represents a Process as a SipxResource
/**
 * Created either:
 * - As a side effect of creating a Process when parsing a 'sipXecs-process' element
 *   (in the 'findProcessResource' method).
 * - By a 'process' element listed as in the 'resources' element in some other process definition
 *   (using the 'parse' method)
 */
class ProcessResource : public SipxResource
{
  public:

// ================================================================
/** @name           Creation Methods
 *
 */
///@{
   /// Factory method that parses a 'process' resource description element.
   static
      SipxResource* parse(TiXmlElement* resourceElement, ///< some child element of 'resources'.
                          Process* currentProcess        ///< Process whose resources are being read.
                          );
   /**<
    * This is called by SipxResource::parse with any 'process' child of the 'resources' element
    * in a process definition.  
    *
    * @returns NULL if the element was in any way invalid.
    */

   /// Return an existing ProcessResource, or if none exists, create one.
   static
      ProcessResource* find(const char* processName);
   /**<
    * @note the fact that a ProcessResource exists does not ensure that the
    *         corresponding Process element exists.
    */
   
   /// get a description of the ProcessResource (for use in logging)
   virtual void getDescription(UtlString&  description /**< returned description */);

///@}   
// ================================================================
/** @name           Status Operations
 *
 */
///@{

   /// Whether or not the ProcessResource is ready for use by a Process.
   virtual boolean isReadyToStart();

   /// Whether or not it is safe to stop a Process using the ProcessResource.
   virtual boolean isSafeToStop();

///@}
// ================================================================
/** @name           Configuration Control Methods
 *
 */
///@{

   /// Whether or not the ProcessResource may be written by configuration update methods.
   virtual boolean isWriteable();

///@}
// ================================================================
/** @name           Container Support Operations
 *
 */
///@{

   /// Determine whether or not the values in a containable are comparable.
   virtual UtlContainableType getContainableType() const;
   /**<
    * This returns a unique type for UtlString
    */

   static const UtlContainableType TYPE;    ///< Class type used for runtime checking 

///@}


  protected:
   
   /// constructor
   ProcessResource(const char* uniqueId);

   /// Do any special handling when a resource is required by the process.
   virtual void requiredBy(Process* currentProcess);
   /**< this base class calls currentProcess->requireResourceToStart */
   
   /// destructor
   virtual ~ProcessResource();

  private:

   Process*   mProcess;

   static OsBSem     mProcessResourceTableLock;
   static UtlHashMap mProcessResourceTable;

   // @cond INCLUDENOCOPY
   /// There is no copy constructor.
   ProcessResource(const ProcessResource& nocopyconstructor);

   /// There is no assignment operator.
   ProcessResource& operator=(const ProcessResource& noassignmentoperator);
   // @endcond     
};

#endif // _PROCESSRESOURCE_H_
