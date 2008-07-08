// 
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////
#ifndef _SIPXRESOURCE_H_
#define _SIPXRESOURCE_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "utl/UtlSList.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class TiXmlElement;
class TiXmlAttribute;
class Process;

/// Abstract base class for things upon which Process objects are dependant.
/**
 * SipxResources are declared by an element belonging to the resource class in the
 * sipXecs-process schema (a sub-element of the 'resources' element in a 'process' document).
 * They control three aspects of sipXsupervisor functionality:
 *
 * - Only something that is declared to be a resource can be written using the configuration
 *   replication methods (@see FileRpcMethod and ImdbRpcMethod).
 *
 * - A Process definition can declare that a resource is 'required', which means that:
 *   - The Process can not start before that SipxResource is "ready"
 *     The definition of "ready" differs depending on the subclass of SipxResource.
 *   - The Process can not stop before that SipxResource is no longer "in use".
 */
class SipxResource : public UtlString
{
  public:

   /// destructor
   virtual ~SipxResource();

   /// Factory method that parses a resource description element.
   static
      bool parse(TiXmlElement* resourceElement, ///< some child element of 'resources'.
                 Process* currentProcess        ///< Process whose resources are being read.
                 );
   /**<
    * This is called by Process::createFromDefinition with each child of the 'resources' element
    * in a process definition.  Based on the element name, this method calls the 'parse'
    * method in the appropriate subclass.
    *
    * @returns whether or not the element was valid.
    */
   
   /// get a description of the SipxResource (for use in logging)
   virtual void getDescription(UtlString&  description /**< returned description */);
   
// ================================================================
/** @name           Status Operations
 *
 */
///@{

   /// Whether or not the SipxResource is ready for use by a Process.
   virtual bool isReadyToStart();

   /// Whether or not it is safe to stop a Process using the SipxResource.
   virtual bool isSafeToStop();

   /// Some change has been made to this resource; notify any Processes that use it.
   virtual void modified();

///@}
// ================================================================
/** @name           Configuration Control Methods
 *
 */
///@{

   /// Whether or not the SipxResource may be written by configuration update methods.
   bool isWriteable();

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
   SipxResource(const char* uniqueId);

   /// Parses attributes common to all SipxResource classes.
   bool parseAttribute(const TiXmlAttribute* attribute);
   /**<
    * This method should be called for each attribute child of a 'resource' type node.
    * @returns true iff the attribute was recognized and handled by the SipxResource class.
    * If this returns false, then the subclass must attempt to parse the attribute
    * as one specific to its subclass.
    */

   /// Do any special handling when a resource is required by the process.
   virtual void requiredBy(Process* currentProcess);
   /**< this base class calls currentProcess->requireResourceToStart */
         
   bool mWritable; ///< from the 'configAccess' attribute.

   UtlSList mUsedBy;  ///< Process objects that use this SipxResource.
   
  private:

   
   
   // @cond INCLUDENOCOPY
   /// There is no copy constructor.
   SipxResource(const SipxResource& nocopyconstructor);

   /// There is no assignment operator.
   SipxResource& operator=(const SipxResource& noassignmentoperator);
   // @endcond     
};

#endif // _RESOURCE_H_
