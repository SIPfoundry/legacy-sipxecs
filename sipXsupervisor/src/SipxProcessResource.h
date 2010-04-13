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
class SipxProcess;

/// Represents a SipxProcess as a SipxResource
/**
 * Created either:
 * - As a side effect of creating a SipxProcess when parsing a 'sipXecs-process' element
 *   (in the 'findProcessResource' method).
 * - By a 'process' element listed as in the 'resources' element in some other process definition
 *   (using the 'parse' method)
 */
class SipxProcessResource : public SipxResource
{
  public:

// ================================================================
/** @name           Creation
 *
 */
///@{
   /// Public name of the resource element parsed by this parser.
   static const char* SipxProcessResourceTypeName;

   /// Factory method that parses a 'process' resource description element.
   static
      bool parse(const TiXmlDocument& processDefinitionDoc, ///< process definition document
                 TiXmlElement* resourceElement, ///< some child element of 'resources'.
                 SipxProcess* currentProcess        ///< SipxProcess whose resources are being read.
                 );
   /**<
    * This is called by SipxResource::parse with any 'process' child of the 'resources' element
    * in a process definition.
    *
    * @returns false if the element was in any way invalid.
    */

   /// get a description of the SipxProcessResource (for use in logging)
   virtual void appendDescription(UtlString&  description /**< returned description */) const;

///@}
// ================================================================
/** @name           Configuration Control Methods
 *
 */
///@{

   /// A SipxProcessResource may not written by configuration update methods.
   bool isWriteable();

   /// If possible, get the corresponding SipxProcess object.
   SipxProcess* getProcess();

///@}
// ================================================================
/** @name           Status Operations
 *
 */
///@{

   /// Whether or not the SipxProcessResource is ready for use by a SipxProcess.
   virtual bool isReadyToStart(UtlString& missingResource);

   /// Whether or not it is safe to stop a SipxProcess using the SipxProcessResource.
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
   friend class SipxProcess;
   friend class SipxSupervisorProcess;

   /// constructor
   SipxProcessResource(const char* uniqueId);

   /// destructor
   virtual ~SipxProcessResource();

  private:

   // @cond INCLUDENOCOPY
   /// There is no copy constructor.
   SipxProcessResource(const SipxProcessResource& nocopyconstructor);

   /// There is no assignment operator.
   SipxProcessResource& operator=(const SipxProcessResource& noassignmentoperator);
   // @endcond
};

#endif // _PROCESSRESOURCE_H_
