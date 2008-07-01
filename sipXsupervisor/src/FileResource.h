// 
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////
#ifndef _FILERESOURCE_H_
#define _FILERESOURCE_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "SipxResource.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Represents a File as a SipxResource
/**
 * Created either:
 * - As a side effect of creating a Process when parsing a 'sipXecs-process' element
 *   (in the 'findProcessResource' method).
 * - By a 'File' element listed as in the 'resources' element in some other File definition
 *   (using the 'parse' method)
 */
class FileResource : public SipxResource
{
  public:

// ================================================================
/** @name           Creation Methods
 *
 */
///@{
   /// Factory method that parses a 'file' or 'osconfigdb' resource description element.
   static
      SipxResource* parse(TiXmlElement* resourceElement, ///< the child element of 'resources'.
                          Process* currentProcess        ///< Process whose resources are being read.
                          );
   /**<
    * This is called by SipxResource::parse with any 'file' or 'osconfigdb' child of
    * the 'resources' element in a process definition.  
    *
    * @returns NULL if the element was in any way invalid.
    */

   /// Return an existing FileResource or NULL if no FileResource is found.
   static
      FileResource* find(const char* fileName /**< full path to the file */);
   
   /// get a description of the FileResource (for use in logging)
   virtual void getDescription(UtlString&  description /**< returned description */);

///@}   
// ================================================================
/** @name           Status Operations
 *
 */
///@{

   /// Whether or not the FileResource is ready for use by a Process.
   virtual boolean isReadyToStart();

///@}
// ================================================================
/** @name           Configuration Control Methods
 *
 */
///@{

   /// Whether or not the FileResource may be written by configuration update methods.
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
   FileResource(const char* uniqueId);

   /// Do any special handling when a resource is required by the process.
   virtual void requiredBy(Process* currentProcess);
   /**< this base class calls currentProcess->requireResourceToStart */
   
   /// destructor
   virtual ~FileResource();

  private:

   static OsBSem     mFileResourceTableLock;
   static UtlHashMap mFileResourceTable;

   // @cond INCLUDENOCOPY
   /// There is no copy constructor.
   FileResource(const FileResource& nocopyconstructor);

   /// There is no assignment operator.
   FileResource& operator=(const FileResource& noassignmentoperator);
   // @endcond     
};

#endif // _FILERESOURCE_H_
