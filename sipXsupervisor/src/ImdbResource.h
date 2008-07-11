// 
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////
#ifndef _IMDBRESOURCE_H_
#define _IMDBRESOURCE_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlHashMap.h"
#include "SipxResource.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Represents a Imdb as a SipxResource
/**
 * Created by an 'Imdb' element listed as in the 'resources' element of a sipXecs-process definition
 */
class ImdbResource : public SipxResource
{
  public:

// ================================================================
/** @name           Creation
 *
 */
///@{
   /// Public name of the resource element parsed by this parser.
   static const char* ImdbResourceTypeName;
   
   /// Factory method that parses an 'imdb' resource description element.
   static
      bool parse(const TiXmlDocument& processDefinitionDoc, ///< process definition document
                 TiXmlElement* resourceElement, ///< the child element of 'resources'.
                 Process* currentProcess        ///< Process whose resources are being read.
                 );
   /**<
    * This is called by SipxResource::parse with any 'imdb' child of
    * the 'resources' element in a process definition.  
    *
    * @returns false if the element was in any way invalid.
    */

   /// Return an existing ImdbResource or NULL if no ImdbResource is found.
   static
      ImdbResource* find(const char* imdbName /**< imdb table name */);
   
   /// get a description of the ImdbResource (for use in logging)
   virtual void appendDescription(UtlString&  description /**< returned description */);

///@}   
// ================================================================
/** @name           Status Operations
 *
 */
///@{

   /// Whether or not the ImdbResource is ready for use by a Process.
   virtual bool isReadyToStart();

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
   ImdbResource(const char* uniqueId);

   // Do any special handling when a resource is required by the process.
   // virtual void requiredBy(Process* currentProcess);
   /* at present, there is none, so let the base class to this */
   
   /// destructor
   virtual ~ImdbResource();

  private:

   static OsBSem     mImdbResourceTableLock;
   static UtlHashMap mImdbResourceTable;

   // @cond INCLUDENOCOPY
   /// There is no copy constructor.
   ImdbResource(const ImdbResource& nocopyconstructor);

   /// There is no assignment operator.
   ImdbResource& operator=(const ImdbResource& noassignmentoperator);
   // @endcond     
};

#endif // _IMDBRESOURCE_H_
