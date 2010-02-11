//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsFS.h"
#include "os/OsSysLog.h"
#include "xmlparser/tinyxml.h"
#include "xmlparser/XmlErrorMsg.h"
#include "xmlparser/ExtractContent.h"

#include "SipxProcess.h"
#include "DirectoryResourceManager.h"
#include "DirectoryResource.h"

// DEFINES
// CONSTANTS
const UtlContainableType DirectoryResource::TYPE = "DirectoryResource";
const char* DirectoryResource::DirectoryResourceTypeName = "directory";

const RegEx RegularExpressionSpecial("[*.?+^$[(){}\\]]");
const char  PatternWildcardCharacter = '*';

const char  RegExStartString = '^';
const char  RegExEndString = '$';
/*
 * Note that the following expressions assume that they are only testing
 * the basename - they do not exclude '/'
 */
const char* InitialWildcardRegexp = "(?:[^.].*)?";
const char* InsideWildcardRegexp = ".*";

// FORWARD DECLARATIONS

// Factory method that parses a 'directory' resource description element.
bool DirectoryResource::parse(const TiXmlDocument& directoryDefinitionDoc, ///< process definition document
                              TiXmlElement* resourceElement,               // 'directory' element
                              SipxProcess* currentProcess                  // whose resources are being read.
                              )
{
   /*
    * This is called by SipxResource::parse with any 'directory' child of
    * the 'resources' element in a process definition.
    *
    * @returns NULL if the element was in any way invalid.
    */
   UtlString errorMsg;
   bool resourceIsValid = true;
   bool validResourceParm;

   TiXmlElement*      dbElement;
   UtlString          path;
   UtlSList           resources; // holding list for resources until we're done
   
   DirectoryResource* dirWithoutPatternResource = NULL;
   DirectoryResource* directoryResource = NULL;
   DirectoryResourceManager* directoryManager = DirectoryResourceManager::getInstance();

   dbElement = resourceElement->FirstChildElement();
   if (dbElement)
   {
      if (0 == strcmp("path",dbElement->Value()))
      {
         validResourceParm = textContent(path, dbElement);
         if (validResourceParm && !path.isNull())
         {
            // Initialize the resource for the directory path name alone (null pattern)
            // used for 'required' processing.
            if (!(directoryResource = directoryManager->find(path, ""))) // does this combination exist?
            {
               directoryResource = new DirectoryResource(path, "", "");
            }
      
            // get the attribute values for the directory
            for ( const TiXmlAttribute* attribute = resourceElement->FirstAttribute();
                  resourceIsValid && attribute;
                  attribute = attribute->Next()
                 )
            {
               if (!(resourceIsValid =
                     directoryResource->SipxResource::parseAttribute(directoryDefinitionDoc,
                                                                     attribute, currentProcess)
                     ))
               {
                  OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "DirectoryResource::parse "
                                "invalid attribute '%s': directory path '%s' ignored",
                                attribute->Name(), path.data());
               }         
            } // end of attribute loop

            if (resourceIsValid)
            {
               dirWithoutPatternResource = directoryResource;
               resources.append(directoryResource); // save until parsing is complete
               directoryResource = NULL;
            }

            // advance to the next element, if any.
            dbElement = dbElement->NextSiblingElement();
         }
         else
         {
            resourceIsValid = false;
            XmlErrorMsg(directoryDefinitionDoc, errorMsg);
            OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "DirectoryResource::parse "
                          "'path' element is empty or invalid %s",
                          errorMsg.data()
                          );
         }
      }
      else
      {
         // first child is not 'path'
         resourceIsValid = false;
         XmlErrorMsg(directoryDefinitionDoc, errorMsg);
         OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "DirectoryResource::parse "
                       "expected path element, found %s",
                       dbElement->Value()
                       );
      }
   }
   else
   {
      resourceIsValid = false;
      XmlErrorMsg(directoryDefinitionDoc, errorMsg);
      OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "DirectoryResource::parse "
                    "no elements are present: path element is required %s",
                    errorMsg.data()
                    );
   }

   /*
    * Loop over all filePattern elements, creating a DirectoryResource for each,
    * and saving them on the 'resources' list.
    */
   while (resourceIsValid && dbElement)
   {
      if (0 == strcmp("filePattern", dbElement->Value()))
      {
         TiXmlElement* filePatternDocElement = dbElement;
         UtlString pattern;

         validResourceParm = textContent(pattern, dbElement);
         if (validResourceParm && !pattern.isNull())
         {
            if (!(directoryResource = directoryManager->find(path, pattern))) // does this combination exist?
            {
               // No - this is a new path/pattern combination
               
               // Convert the file pattern into a regular expression
               UtlString patternExpression;
               if ( pattern2RegEx(pattern, patternExpression) )
               {
                  directoryResource = new DirectoryResource(path, pattern, patternExpression);

                  for ( const TiXmlAttribute* attribute = filePatternDocElement->FirstAttribute();
                        resourceIsValid && attribute;
                        attribute = attribute->Next()
                       )
                  {
                     if (directoryResource->
                         SipxResource::isAttribute(attribute, SipxResource::RequiredAttributeName))
                     {
                        resourceIsValid = false ;
                        OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "DirectoryResource::parse "
                                      "invalid attribute '%s' on filePattern '%s'",
                                      attribute->Name(), path.data()) ;
                     }
                     else if (!(resourceIsValid = directoryResource->
                                SipxResource::parseAttribute(directoryDefinitionDoc,
                                                             attribute, currentProcess)
                                ))
                     {
                        OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "DirectoryResource::parse "
                                      "invalid attribute '%s': filePattern '%s' ignored",
                                      attribute->Name(), pattern.data());
                     }
                  }

                  if (resourceIsValid)
                  {
                     if (directoryResource->mImplicitAccess)
                     {
                        directoryResource->mImplicitAccess
                           = dirWithoutPatternResource->mImplicitAccess;
                        directoryResource->mAccess = dirWithoutPatternResource->mAccess;
                     }
                  }
                  else
                  {
                     delete directoryResource;
                  }
               }
               else
               {
                  resourceIsValid = false;
                  OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "DirectoryResource::parse "
                                "path '%s' "
                                "filePattern '%s' did not translate to a regular expression",
                                path.data(), pattern.data()
                                );
               }
            }
            else
            {
               // existing path/pattern combination
               // parse the attributes so that the required and access attributes are set correctly
               for ( const TiXmlAttribute* attribute = filePatternDocElement->FirstAttribute();
                     resourceIsValid && attribute;
                     attribute = attribute->Next()
                    )
               {
                  if (directoryResource->
                      SipxResource::isAttribute(attribute, SipxResource::RequiredAttributeName))
                  {
                     resourceIsValid = false ;
                     OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "DirectoryResource::parse "
                                   "invalid attribute '%s' on filePattern '%s'",
                                   attribute->Name(), path.data()) ;
                  }
                  else if (!(resourceIsValid = directoryResource->
                             SipxResource::parseAttribute(directoryDefinitionDoc,
                                                          attribute, currentProcess)
                        ))
                  {
                     OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "DirectoryResource::parse "
                                   "invalid attribute '%s': filePattern '%s' ignored",
                                   attribute->Name(), pattern.data());
                  }
               }

               if (resourceIsValid)
               {
                  if (directoryResource->mImplicitAccess)
                  {
                     directoryResource->mImplicitAccess = dirWithoutPatternResource->mImplicitAccess;
                     directoryResource->mAccess = dirWithoutPatternResource->mAccess;
                  }

                  OsSysLog::add(FAC_SUPERVISOR, PRI_INFO, "DirectoryResource::parse "
                                "shared directory '%s' pattern '%s'",
                                path.data(), pattern.data()
                                );
               }
            }
            
            // advance to the next element, if any.
            dbElement = dbElement->NextSiblingElement();
         }
         else
         {
            resourceIsValid = false;
            XmlErrorMsg(directoryDefinitionDoc, errorMsg);
            OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "DirectoryResource::parse "
                          "'filePattern' element is empty"
                          );
         }
      }
      else
      {
         resourceIsValid = false;
         XmlErrorMsg(directoryDefinitionDoc, errorMsg);
         OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "DirectoryResource::parse "
                       "expected 'filePattern' element; found '%s'",
                       dbElement->Value()
                       );
      }

      if (resourceIsValid) 
      {
         resources.append(directoryResource);
      }

      directoryResource = NULL;
   } // while loop over filePattern elements

   if (resourceIsValid && resources.entries() < 2) // no filePattern elements were found
   {
      // the directory resource was created, but no filePattern elements
      resourceIsValid = false;
      OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "DirectoryResource::parse "
                    "no 'filePattern' elements found for path '%s'",
                    path.data()
                    );
   }
   
   if (resourceIsValid) // have all elements been parsed successfully?
   {
      // everything was valid - so transfer the resulting resources to the DirectoryResourceManager
      while ((directoryResource=dynamic_cast<DirectoryResource*>(resources.get())))
      {
         directoryResource->usedBy(currentProcess);

         if (! directoryResource->mFilePattern.isNull())
         {
            // pattern resources are not allowed to be required
            currentProcess->resourceIsOptional(directoryResource);
         }

         if (directoryResource->mFirstDefinition)
         {
            directoryResource->mFirstDefinition = false;
            directoryManager->save(directoryResource);

            UtlString description;
            directoryResource->appendDescription(description);
            OsSysLog::add(FAC_SUPERVISOR, PRI_INFO, "DirectoryResource::parse add %s",
                          description.data()
                          );
         }
      }
   }
   else
   {
      /*
       * Something was invalid - discard any of these resources that are not shared
       *
       * Note: this could still have modified the access of some shared resources,
       *       but that's too difficult to fix.
       */
      while ((directoryResource=dynamic_cast<DirectoryResource*>(resources.get())))
      {
         if (directoryResource->mFirstDefinition)
         {
            delete directoryResource;
         }
      }
   }
   
   return resourceIsValid;
}

/// Translate a file pattern into a regular expression
bool DirectoryResource::pattern2RegEx(const UtlString& pattern, UtlString& patternExpression)
{
   patternExpression.remove(0);

   RegEx regularExpressionSpecial(RegularExpressionSpecial);
   
   ssize_t patternPosition = 0;
   ssize_t nextSpecialPosition;

   while ( regularExpressionSpecial.SearchAt(pattern, patternPosition) )
   {
      nextSpecialPosition = regularExpressionSpecial.MatchStart(0);
      char nextSpecial = pattern(nextSpecialPosition);
      
      if ( nextSpecialPosition == patternPosition )
      {
         if ( nextSpecial == '*' )
         {
            patternExpression.append( (patternPosition == 0)
                                     ? InitialWildcardRegexp
                                     : InsideWildcardRegexp );
            patternPosition++;
         }
         else
         {
            // all other characters that are significant in a regexp are escaped
            patternExpression.append("\\");
            patternExpression.append(nextSpecial);
            patternPosition++;
         }
      }
      else
      {
         ssize_t fixedPartLength = nextSpecialPosition - patternPosition;
         patternExpression.append(pattern, patternPosition, fixedPartLength);
         patternPosition += fixedPartLength;
         /*
          * We have not incremented past the special, so we will match it again on
          * the next iteration; this avoids having to duplicate the checks for
          * whether it is '*' or some other regular expression character here
          */
      }
   }

   // append any fixed part that follows the last wildcard
   if ((size_t)patternPosition < pattern.length())
   {
      patternExpression.append( pattern, patternPosition, UTLSTRING_TO_END );
   }

   OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                 "DirectoryResource::patternRegEx( '%s' ) -> '%s'",
                 pattern.data(), patternExpression.data()
                 );
   
   return ! patternExpression.isNull();
}

// get a description of the DirectoryResource (for use in logging)
void DirectoryResource::appendDescription(UtlString&  description /**< returned description */) const
{
   if (mFilePattern.isNull())
   {
      description.append("directory '");
      description.append(data());
      description.append("'");
   }
   else
   {
      description.append("directory '");
      description.append(data());
      description.append("' pattern '");
      description.append(mFilePattern.data());
      description.append("'");
   }
}


// Whether or not the DirectoryResource is ready for use by a SipxProcess.
bool DirectoryResource::isReadyToStart(UtlString& missingResource)
{
   OsPath directoryPath(*this);
   OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                 "DirectoryResource::isReadyToStart checking for existence of %s",
                 data());
   bool bReady;

   if (mFilePattern.isNull())
   {
      bReady = true;
   }
   else
   {
      bReady = OsFileSystem::exists(directoryPath);
   }
   
   if ( !bReady )
   {
      missingResource = "";
      appendDescription(missingResource);
   }
   return bReady;
}


/// Does the given filename match the pattern for this directory resource?
bool DirectoryResource::matches(const UtlString& filename) const
{
   RegEx pattern(mPatternRegex);

   return ! mFilePattern.isNull() && pattern.Search(filename);
}


/// Does this DirectoryResource have this exact file pattern string?
bool DirectoryResource::isFilePattern(const char* pattern) const
{
   return mFilePattern.compareTo(pattern) == 0;
}

   
// Determine whether or not the values in a containable are comparable.
UtlContainableType DirectoryResource::getContainableType() const
{
   return TYPE;
}

/// constructor
DirectoryResource::DirectoryResource(const char* path, const char* pattern, const char* patternExp) :
   FileResource(path),
   mFilePattern(pattern),
   mPatternRegex(patternExp)
{
}

/// destructor
DirectoryResource::~DirectoryResource()
{

}
