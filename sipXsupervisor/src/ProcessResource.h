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
#include "utl/UtlHashMap.h"

#include "SipxResource.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class Process;

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
/** @name           Creation 
 *
 */
///@{
   /// Public name of the resource element parsed by this parser.
   static const char* ProcessResourceTypeName;
   
   /// Factory method that parses a 'process' resource description element.
   static
      bool parse(const TiXmlDocument& processDefinitionDoc, ///< process definition document
                 TiXmlElement* resourceElement, ///< some child element of 'resources'.
                 Process* currentProcess        ///< Process whose resources are being read.
                 );
   /**<
    * This is called by SipxResource::parse with any 'process' child of the 'resources' element
    * in a process definition.  
    *
    * @returns false if the element was in any way invalid.
    */

   /// get a description of the ProcessResource (for use in logging)
   virtual void appendDescription(UtlString&  description /**< returned description */) const;

///@}   
// ================================================================
/** @name           Configuration Control Methods
 *
 */
///@{

   /// A ProcessResource may not written by configuration update methods.
   bool isWriteable();

   /// If possible, get the corresponding Process object.
   Process* getProcess();
   
///@}
// ================================================================
/** @name           Status Operations
 *
 */
///@{

   /// Whether or not the ProcessResource is ready for use by a Process.
   virtual bool isReadyToStart();

   /// Whether or not it is safe to stop a Process using the ProcessResource.
   virtual bool isSafeToStop();

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
   friend class Process;
   
   /// constructor
   ProcessResource(const char* uniqueId, Process* currentProcess);

   /// destructor
   virtual ~ProcessResource();

  private:

   // @cond INCLUDENOCOPY
   /// There is no copy constructor.
   ProcessResource(const ProcessResource& nocopyconstructor);

   /// There is no assignment operator.
   ProcessResource& operator=(const ProcessResource& noassignmentoperator);
   // @endcond     
};

#endif // _PROCESSRESOURCE_H_
