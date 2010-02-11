//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef _DIRECTORYRESOURCE_H_
#define _DIRECTORYRESOURCE_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlRegex.h"
#include "FileResource.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Represents a Directory (and the files it can contain) as a SipxResource
/**
 * Created as a side effect of creating a SipxProcess when parsing a 'sipXecs-process' element
 *   (in the 'findProcessResource' method).  @see the SipxProcess class documentation.
 *
 * @note One DirectoryResource object is created for each unique combination of a 'directory'
 * element and a 'filePattern' element in the process definition.
 *
 * To locate a particular DirectoryResource, call DirectoryResourceManager::findFilename
 */
class DirectoryResource : public FileResource
{
  public:

// ================================================================
/** @name           Creation
 *
 */
///@{
   /// Public name of the resource element parsed by this parser.
   static const char* DirectoryResourceTypeName;       ///< name of 'file' resource element

   /// Factory method that parses a 'directory' resource description element.
   static
      bool parse(const TiXmlDocument& processDefinitionDoc, ///< process definition document
                 TiXmlElement* resourceElement, ///< the child element of 'resources'.
                 SipxProcess* currentProcess        ///< SipxProcess whose resources are being read.
                 );
   /**<
    * This is called by SipxResource::parse with any 'directory' child of
    * the 'resources' element in a process definition.
    *
    * This can actualy create multiple DirectoryResource objects, since there
    * is one for each unique combination of directory path and file pattern.
    *
    * @returns false if the element was in any way invalid.
    */
///@}
// ================================================================
/** @name           Status Operations
 *
 */
///@{

   /// get a description of the DirectoryResource (for use in logging)
   virtual void appendDescription(UtlString&  description /**< returned description */) const;

   /// Whether or not the DirectoryResource is ready for use by a SipxProcess.
   virtual bool isReadyToStart(UtlString& missingResource);

///@}

// ================================================================
/** @name DirectoryResourceManager Support Operations
 *
 */
///@{

   /// Does the given filename match the pattern for this directory resource?
   bool matches(const UtlString& filename) const;

   /// Does this DirectoryResource have this exact file pattern string?
   bool isFilePattern(const char* pattern) const;
   
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

   /// Translate a file pattern into a regular expression 
   static bool pattern2RegEx(const UtlString& pattern,      ///< shell-style file glob pattern
                             UtlString& patternExpression   ///< regular expression equivalent of pattern
                             );
   /**<
    * See ../meta/sipXecs-process.xsd.in for rules on supported patterns.
    * @returns true if patternExpression is not empty
    */
   
   /// constructor
   DirectoryResource(const char* path, const char* pattern, const char* patternExp);

   /// destructor
   virtual ~DirectoryResource();

  private:

   // the parent string 
   UtlString mFilePattern;  ///< the original shell-style pattern
   RegEx     mPatternRegex; ///< the regular expression version of the pattern
   
   // @cond INCLUDENOCOPY
   /// There is no copy constructor.
   DirectoryResource(const DirectoryResource& nocopyconstructor);

   /// conceal the underlying single-argument constructor (not implemented)
   DirectoryResource(const char* uniqueId);

   /// There is no assignment operator.
   DirectoryResource& operator=(const DirectoryResource& noassignmentoperator);
   // @endcond
};

#endif // _DIRECTORYRESOURCE_H_
