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
#include "xmlparser/tinyxml.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class TiXmlElement;
class TiXmlAttribute;
class SipxProcess;
class SipxProcessDefinitionParserTest;

/// Abstract base class for things upon which SipxProcess objects are dependant.
/**
 * SipxResources are declared by an element belonging to the resource class in the
 * sipXecs-process schema (a sub-element of the 'resources' element in a 'process' document).
 * They control three aspects of sipXsupervisor functionality:
 *
 * - Only something that is declared to be a resource can be written using the configuration
 *   replication methods (@see FileRpcMethod and ImdbRpcMethod).
 *
 * - A SipxProcess definition can declare that a resource is 'required', which means that:
 *   - The SipxProcess can not start before that SipxResource is "ready"
 *   - The SipxProcess can not stop before that SipxResource is no longer "in use".
 *
 *   The definitions of "ready" and "in use" differ depending on the subclass of SipxResource.
 */
class SipxResource : public UtlString
{
  public:

   /// destructor
   virtual ~SipxResource();

   /// Factory method that parses a resource description element.
   static
      bool parse(const TiXmlDocument& processDefinitionDoc, ///< process definition document
                 TiXmlElement* resourceElement, ///< some child element of 'resources'.
                 SipxProcess* currentProcess        ///< SipxProcess whose resources are being read.
                 );
   /**<
    * This is called by SipxProcess::createFromDefinition with each child of the 'resources' element
    * in a process definition.  Based on the element name, this method calls the 'parse'
    * method in the appropriate subclass.
    *
    * @returns whether or not the element was valid.
    */

   /// get a description of the SipxResource (for use in logging)
   virtual void appendDescription(UtlString&  description /**< returned description */) const;

   /// record that a resource is used by a particular SipxProcess
   void usedBy(SipxProcess* currentProcess);


// ================================================================
/** @name           Status Operations
 *
 */
///@{

   /// Whether or not the SipxResource is ready for use by a SipxProcess.
   virtual bool isReadyToStart(UtlString& missingResource);

   /// Whether or not it is safe to stop a SipxProcess using the SipxResource.
   virtual bool isSafeToStop();

   /// Some change has been made to this resource; notify any SipxProcesses that use it.
   virtual void modified();

///@}
// ================================================================
/** @name           Configuration Control Methods
 *
 */
///@{

   /// Whether or not the SipxResource may be written by configuration update methods.
   virtual bool isWriteable();

   /// Whether or not the SipxResource may be read by file access methods.
   virtual bool isReadable();
   
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
   bool parseAttribute(const TiXmlDocument& document,  ///< document being parsed (for error mess)
                       const TiXmlAttribute* attribute,///< attribute to be parsed.
                       SipxProcess* currentProcess         ///< SipxProcess whose resources are being read.
                       );
   /**<
    * This method should be called for each attribute child of a 'resource' type node.
    * @returns true iff the attribute was recognized and handled by the SipxResource class.
    * If this returns false, then the subclass must attempt to parse the attribute
    * as one specific to its subclass.
    */

   static const char* RequiredAttributeName;
   static const char* ConfigAccessAttributeName;

   /// is the attribute name the value in attributeName?
   bool isAttribute(const TiXmlAttribute* attribute,
                    const char* attributeName
                    );

   bool mFirstDefinition; ///< true only until saved in the appropriate ResourceManager


   bool mImplicitAccess; ///< true if no definition had an explicit 'configAccess' attribute.

   typedef enum 
   {
      ReadAccess   = 0x01,
      WriteAccess  = 0x02
   } ResourceAccess;
   
   int mAccess; ///< the value from the 'configAccess' attribute.

   UtlSList mUsedBy;  ///< SipxProcessResource objects that use this SipxResource.

  private:



   // @cond INCLUDENOCOPY
   /// There is no copy constructor.
   SipxResource(const SipxResource& nocopyconstructor);

   /// There is no assignment operator.
   SipxResource& operator=(const SipxResource& noassignmentoperator);
   // @endcond

   friend class SipxProcessDefinitionParserTest;
   friend class SipxResourceUseTest;
};

#endif // _RESOURCE_H_
