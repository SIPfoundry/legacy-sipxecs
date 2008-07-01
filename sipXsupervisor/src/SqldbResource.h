// 
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////
#ifndef _SQLDBRESOURCE_H_
#define _SQLDBRESOURCE_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "SipxResource.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Represents a Sqldb as a SipxResource
/**
 * Created by an 'Sqldb' element listed as in the 'resources' element of a sipXecs-process definition
 */
class SqldbResource : public SipxResource
{
  public:

// ================================================================
/** @name           Creation Methods
 *
 */
///@{
   /// Factory method that parses an 'sqldb' resource description element.
   static
      SipxResource* parse(TiXmlElement* resourceElement, ///< the child element of 'resources'.
                          Process* currentProcess        ///< Process whose resources are being read.
                          );
   /**<
    * This is called by SipxResource::parse with any 'sqldb' child of
    * the 'resources' element in a process definition.  
    *
    * @returns NULL if the element was in any way invalid.
    */

   /// Return an existing SqldbResource or NULL if no SqldbResource is found.
   static
      SqldbResource* find(const char* sqldbName /**< sqldb table name */);
   
   /// get a description of the SqldbResource (for use in logging)
   virtual void getDescription(UtlString&  description /**< returned description */);

///@}   
// ================================================================
/** @name           Status Operations
 *
 */
///@{

   /// Whether or not the SqldbResource is ready for use by a Process.
   virtual boolean isReadyToStart();

///@}
// ================================================================
/** @name           Configuration Control Methods
 *
 */
///@{

   /// Whether or not the SqldbResource may be written by configuration update methods.
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
   SqldbResource(const char* uniqueId);

   /// Do any special handling when a resource is required by the process.
   virtual void requiredBy(Process* currentProcess);
   /**< this base class calls currentProcess->requireResourceToStart */
   
   /// destructor
   virtual ~SqldbResource();

  private:

   static OsBSem     mSqldbResourceTableLock;
   static UtlHashMap mSqldbResourceTable;

   // @cond INCLUDENOCOPY
   /// There is no copy constructor.
   SqldbResource(const SqldbResource& nocopyconstructor);

   /// There is no assignment operator.
   SqldbResource& operator=(const SqldbResource& noassignmentoperator);
   // @endcond     
};

#endif // _SQLDBRESOURCE_H_
