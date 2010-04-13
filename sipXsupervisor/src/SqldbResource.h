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
#include "utl/UtlHashMap.h"
#include "SipxResource.h"
#include "odbc/OdbcWrapper.h"

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
/** @name           Creation
 *
 */
///@{
   /// Public name of the resource element parsed by this parser.
   static const char* SqldbResourceTypeName;

   /// Factory method that parses an 'sqldb' resource description element.
   static
      bool parse(const TiXmlDocument& processDefinitionDoc, ///< process definition document
                 TiXmlElement* resourceElement, ///< the child element of 'resources'.
                 SipxProcess* currentProcess        ///< SipxProcess whose resources are being read.
                 );
   /**<
    * This is called by SipxResource::parse with any 'sqldb' child of
    * the 'resources' element in a process definition.
    *
    * @returns false if the element was in any way invalid.
    */

   /// Return an existing SqldbResource or NULL if no SqldbResource is found.
   static
      SqldbResource* find(const char* sqldbName /**< sqldb table name */);

   /// get a description of the SqldbResource (for use in logging)
   virtual void appendDescription(UtlString&  description /**< returned description */) const;

///@}
// ================================================================
/** @name           Status Operations
 *
 */
///@{

   /// Whether or not the SqldbResource is ready for use by a SipxProcess.
   virtual bool isReadyToStart(UtlString& missingResource);

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

   /// destructor
   virtual ~SqldbResource();

   UtlString   mUser;             ///< User that is used for database connectivity.
   UtlString   mPassword;         ///< User password that is used for database connectivity.
   UtlString   mDbDriver;         ///< Database Driver that is used for connectivity.
   UtlString   mServer;           ///< Database server location.
   UtlString   mDbName;           ///< Database name.

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
