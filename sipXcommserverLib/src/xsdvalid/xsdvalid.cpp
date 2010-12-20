/*
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * Derived from: xerces-c sample application DOMCount, which is
 * Copyright 1999-2002,2004 The Apache Software Foundation.
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/AbstractDOMParser.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>
#ifdef XERCES_2_8
  #include <xercesc/dom/DOMBuilder.hpp>
#else
  #include <xercesc/parsers/DOMLSParserImpl.hpp>
  #include <xercesc/dom/DOMLSParser.hpp>
  #include <xercesc/dom/DOMConfiguration.hpp>
#endif
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMError.hpp>
#include <xercesc/dom/DOMLocator.hpp>

#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/util/XMLString.hpp>

#include "utl/UtlString.h"

#if defined(XERCES_NEW_IOSTREAMS)
#include <iostream>
#else
#include <iostream.h>
#endif

#include <string.h>
#include <stdlib.h>

XERCES_CPP_NAMESPACE_USE

// ---------------------------------------------------------------------------
//  Simple error handler deriviative to install on parser
// ---------------------------------------------------------------------------
class XSDValidErrorHandler : public DOMErrorHandler
{
public:
   // -----------------------------------------------------------------------
   //  Constructors and Destructor
   // -----------------------------------------------------------------------
   XSDValidErrorHandler();
   ~XSDValidErrorHandler();


   // -----------------------------------------------------------------------
   //  Getter methods
   // -----------------------------------------------------------------------
   bool getSawErrors() const;


   // -----------------------------------------------------------------------
   //  Implementation of the DOM ErrorHandler interface
   // -----------------------------------------------------------------------
   bool handleError(const DOMError& domError);
   void resetErrors();


private :
   // -----------------------------------------------------------------------
   //  Unimplemented constructors and operators
   // -----------------------------------------------------------------------
   XSDValidErrorHandler(const XSDValidErrorHandler&);
   void operator=(const XSDValidErrorHandler&);


   // -----------------------------------------------------------------------
   //  Private data members
   //
   //  fSawErrors
   //      This is set if we get any errors, and is queryable via a getter
   //      method. Its used by the main code to suppress output if there are
   //      errors.
   // -----------------------------------------------------------------------
   bool    fSawErrors;
};


// ---------------------------------------------------------------------------
//  This is a simple class that lets us do easy (though not terribly efficient)
//  trancoding of XMLCh data to local code page for display.
// ---------------------------------------------------------------------------
class StrX
{
public :
   // -----------------------------------------------------------------------
   //  Constructors and Destructor
   // -----------------------------------------------------------------------
   StrX(const XMLCh* const toTranscode)
      {
         // Call the private transcoding method
         fLocalForm = XMLString::transcode(toTranscode);
      }

   ~StrX()
      {
         XMLString::release(&fLocalForm);
      }


   // -----------------------------------------------------------------------
   //  Getter methods
   // -----------------------------------------------------------------------
   const char* localForm() const
      {
         return fLocalForm;
      }

private :
   // -----------------------------------------------------------------------
   //  Private data members
   //
   //  fLocalForm
   //      This is the local code page form of the string.
   // -----------------------------------------------------------------------
   char*   fLocalForm;
};

inline XERCES_STD_QUALIFIER ostream& operator<<(XERCES_STD_QUALIFIER ostream& target, const StrX& toDump)
{
   target << toDump.localForm();
   return target;
}

inline bool XSDValidErrorHandler::getSawErrors() const
{
   return fSawErrors;
}

#if defined(XERCES_NEW_IOSTREAMS)
#include <fstream>
#else
#include <fstream.h>
#endif


// ---------------------------------------------------------------------------
//  This is a simple program which invokes the DOMParser to build a DOM
//  tree for the specified input file. It then walks the tree and counts
//  the number of elements. The element count is then printed.
// ---------------------------------------------------------------------------
static void usage()
{
    XERCES_STD_QUALIFIER cout <<
       "\nUsage:\n"
       "    xsdvalid [<options>] <XML file> | -l <list file>\n"
       "\n"
       "Validates an XML instance document against its schema as determined\n"
       "by examining the attributes in the document.\n"
       "\n"
       "Options:\n"
       "    --version   prints the version number\n"
       "    -s|--schema <namespace> <schema-location>\n"
       "                Specifies a namespace and the location of a local\n"
       "                copy of the schema definition file for that namespace.\n"
       "                (may be repeated for multiple namespaces)\n"
       "    -S|--schema-list <file>\n"
       "                Specifies a file containing namespace/schema-location\n"
       "                pairs.  (One pair per line, separated by whitespace.\n"
       "                Comments start with '#'.)\n"
       "    -l          The input file is a list file, containing a list of names of XML files.\n"
       "                Default is argument is the name of an XML file.\n"
       "    -?          Show this help.\n"
       "\n"
       "    Returns zero if the file (or all files in the list file) is valid, non-zero if not.\n"
       "\n"
         << XERCES_STD_QUALIFIER endl;
}

// ---------------------------------------------------------------------------
//
//   main
//
// ---------------------------------------------------------------------------
int main(int argC, char* argV[])
{

    // Check command line and extract arguments.
    if (argC < 2)
    {
        usage();
        return 1;
    }

    const char*                xmlFile = 0;
    bool                       doList = false;
    bool                       errorOccurred = false;
    char                       localeStr[64];

    UtlString schemaLocationPairs;

    memset(localeStr, 0, sizeof localeStr);

    int argInd;
    for (argInd = 1; argInd < argC; argInd++)
    {
        // Break out on first parm not starting with a dash
        if (argV[argInd][0] != '-')
            break;

        // Watch for special case help request
        if (!strcmp(argV[argInd], "-?"))
        {
           usage();
           return 2;
        }
        if (!strcmp(argV[argInd], "--version"))
        {
           XERCES_STD_QUALIFIER cerr
              << "xsdvalid "
              << "\n  xerces-c library version " << gXercesVersionStr
              << "\n" << XERCES_STD_QUALIFIER endl;

           return 0;
        }
        else if (!strcmp(argV[argInd], "-l")
                 ||  !strcmp(argV[argInd], "-L"))
        {
           doList = true;
        }
        else if (   !strncmp(argV[argInd], "-s", 2)
                 || !strncmp(argV[argInd], "--schema", 9)
                 )
        {
           if (argInd+2 < argC)
           {
              if (!schemaLocationPairs.isNull())
              {
                 schemaLocationPairs.append(" ");
              }
              schemaLocationPairs.append(argV[++argInd]);
              schemaLocationPairs.append(" ");
              schemaLocationPairs.append(argV[++argInd]);
           }
           else
           {
              XERCES_STD_QUALIFIER cerr
                 << "Option '" << argV[argInd]
                 << "' must be followed by <schema-namespace> <schema-location>\n"
                 << XERCES_STD_QUALIFIER endl;
              return 2;
           }
        }
        else if (   !strncmp(argV[argInd], "-S", 2)
                 || !strncmp(argV[argInd], "--schema-list", 14)
                 )
        {
           if (argInd+1 < argC)
           {
              // the input is a list file
              XERCES_STD_QUALIFIER ifstream fin;
              fin.open(argV[++argInd]);

              if (fin.fail()) {
                 XERCES_STD_QUALIFIER cerr
                    <<"Cannot open schema location file '"
                    << argV[argInd] << "'"
                    << XERCES_STD_QUALIFIER endl;
                 return 2;
              }

              int lineNo = 0;
              while (1)
              {
                 char line[1000];
                 //initialize the array to zeros
                 memset(line, 0, sizeof (line));
                 #define WHITESPACE " \t\n\v\f\r"

                 if (! fin.eof() ) {
                    fin.getline (line, sizeof(line));
                    lineNo++;

                    char* first = strtok(line, WHITESPACE);
                    // Ignore empty lines and comment lines.
                    if (first == NULL || *first == '#')
                       continue;

                    char* second = strtok(NULL, WHITESPACE);

                    char* third = strtok(NULL, WHITESPACE);

                    if (!(second != NULL && third == NULL)) {
                       XERCES_STD_QUALIFIER cerr
                          << "Line " << lineNo
                          << " of schema location file '" << argV[argInd]
                          << "' does not contain two fields.\n"
                          << XERCES_STD_QUALIFIER endl;
                       return 2;
                    }

                    if (!schemaLocationPairs.isNull())
                    {
                       schemaLocationPairs.append(" ");
                    }
                    schemaLocationPairs.append(first);
                    schemaLocationPairs.append(" ");
                    schemaLocationPairs.append(second);
                 }
                 else
                    break;
              }

              fin.close();
           }
           else
           {
              XERCES_STD_QUALIFIER cerr
                 << "Option '" << argV[argInd]
                 << "' must be followed by <file-name>\n"
                 << XERCES_STD_QUALIFIER endl;
              return 2;
           }
        }
        else
        {
           XERCES_STD_QUALIFIER cerr << "Unknown option '" << argV[argInd]
                                     << "', ignoring it\n" << XERCES_STD_QUALIFIER endl;
        }
    }

    //
    //  There should be only one and only one parameter left, and that
    //  should be the file name.
    //
    if (argInd != argC - 1)
    {
        usage();
        return 1;
    }

    // Initialize the XML4C system
    try
    {
        if (strlen(localeStr))
        {
            XMLPlatformUtils::Initialize(localeStr);
        }
        else
        {
            XMLPlatformUtils::Initialize();
        }
    }
    catch (const XMLException& toCatch)
    {
       XERCES_STD_QUALIFIER cerr << "Error during initialization! :\n"
                                 << StrX(toCatch.getMessage()) << XERCES_STD_QUALIFIER endl;
       return 1;
    }
    catch (...)
    {
       XERCES_STD_QUALIFIER cerr << "Unexpected error during initialization!\n";
       return 1;
    }

    // Instantiate the DOM parser.
    static const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };
    DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(gLS);
    XSDValidErrorHandler errorHandler;
#ifdef XERCES_2_8
    DOMBuilder        *parser = ((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS, 0);
    parser->setFeature(XMLUni::fgDOMNamespaces, true);
    parser->setFeature(XMLUni::fgXercesSchema, true);
    parser->setFeature(XMLUni::fgXercesSchemaFullChecking, true); 
    parser->setFeature(XMLUni::fgDOMValidation, true);
    parser->setFeature(XMLUni::fgDOMDatatypeNormalization, true); 

    parser->setErrorHandler(&errorHandler);
 
    if (!schemaLocationPairs.isNull())
    {
       XMLCh* propertyValue = XMLString::transcode(schemaLocationPairs.data());
       parser->setProperty(XMLUni::fgXercesSchemaExternalSchemaLocation, propertyValue);
    }
#else
    DOMLSParserImpl    *parser = (DOMLSParserImpl*)((DOMImplementationLS*)impl)->createLSParser(DOMImplementationLS::MODE_SYNCHRONOUS, 0);
    parser->setValidationScheme(AbstractDOMParser::Val_Always);
    parser->setDoNamespaces(true);
    parser->setParameter(XMLUni::fgDOMNamespaces, true);
    parser->setParameter(XMLUni::fgXercesSchema, true);
    parser->setParameter(XMLUni::fgXercesSchemaFullChecking, true);
    parser->setParameter(XMLUni::fgXercesValidationErrorAsFatal, true);
    parser->setParameter(XMLUni::fgDOMDatatypeNormalization, true);
    parser->setParameter(XMLUni::fgDOMErrorHandler, &errorHandler);

    if (!schemaLocationPairs.isNull())
    {
       XMLCh* propertyValue = XMLString::transcode(schemaLocationPairs.data());
       parser->setParameter(XMLUni::fgXercesSchemaExternalSchemaLocation, propertyValue);
    }
#endif

    //
    //  Get the starting time and kick off the parse of the indicated
    //  file. Catch any exceptions that might propagate out of it.
    //
    unsigned long duration;

    bool more = true;
    XERCES_STD_QUALIFIER ifstream fin;

    // the input is a list file
    if (doList)
        fin.open(argV[argInd]);

    if (fin.fail()) {
        XERCES_STD_QUALIFIER cerr <<"Cannot open the list file: " << argV[argInd] << XERCES_STD_QUALIFIER endl;
        return 2;
    }

    while (more)
    {
        char fURI[1000];
        //initialize the array to zeros
        memset(fURI,0,sizeof(fURI));

        if (doList) {
            if (! fin.eof() ) {
                fin.getline (fURI, sizeof(fURI));
                if (!*fURI)
                    continue;
                else {
                    xmlFile = fURI;
                    XERCES_STD_QUALIFIER cerr << "==Parsing== " << xmlFile << XERCES_STD_QUALIFIER endl;
                }
            }
            else
                break;
        }
        else {
            xmlFile = argV[argInd];
            more = false;
        }

        //reset error count first
        errorHandler.resetErrors();

        XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *doc = 0;

        try
        {
            // reset document pool
            parser->resetDocumentPool();

            const unsigned long startMillis = XMLPlatformUtils::getCurrentMillis();
            doc = parser->parseURI(xmlFile);
            const unsigned long endMillis = XMLPlatformUtils::getCurrentMillis();
            duration = endMillis - startMillis;
        }

        catch (const XMLException& toCatch)
        {
            XERCES_STD_QUALIFIER cerr << "\nError during parsing: '" << xmlFile << "'\n"
                 << "Exception message is:  \n"
                 << StrX(toCatch.getMessage()) << "\n" << XERCES_STD_QUALIFIER endl;
            errorOccurred = true;
            continue;
        }
        catch (const DOMException& toCatch)
        {
            const unsigned int maxChars = 2047;
            XMLCh errText[maxChars + 1];

            XERCES_STD_QUALIFIER cerr << "\nDOM Error during parsing: '" << xmlFile << "'\n"
                 << "DOMException code is:  " << toCatch.code << XERCES_STD_QUALIFIER endl;

            if (DOMImplementation::loadDOMExceptionMsg(toCatch.code, errText, maxChars))
                 XERCES_STD_QUALIFIER cerr << "Message is: " << StrX(errText) << XERCES_STD_QUALIFIER endl;

            errorOccurred = true;
            continue;
        }
        catch (...)
        {
            XERCES_STD_QUALIFIER cerr << "\nUnexpected exception during parsing: '" << xmlFile << "'\n";
            errorOccurred = true;
            continue;
        }
    }

    //
    //  Delete the parser itself.  Must be done prior to calling Terminate, below.
    //
    parser->release();

    // And call the termination method
    XMLPlatformUtils::Terminate();

    if (doList)
        fin.close();

    return errorOccurred || errorHandler.getSawErrors() ? 1 : 0;
}





XSDValidErrorHandler::XSDValidErrorHandler() :

   fSawErrors(false)
{
}

XSDValidErrorHandler::~XSDValidErrorHandler()
{
}

// ---------------------------------------------------------------------------
//  XSDValidHandlers: Overrides of the DOM ErrorHandler interface
// ---------------------------------------------------------------------------
bool XSDValidErrorHandler::handleError(const DOMError& domError)
{
    fSawErrors = true;
    if (domError.getSeverity() == DOMError::DOM_SEVERITY_WARNING)
    {
       XERCES_STD_QUALIFIER cerr << "\nWarning at file ";
    }
    else if (domError.getSeverity() == DOMError::DOM_SEVERITY_ERROR)
    {
        XERCES_STD_QUALIFIER cerr << "\nError at file ";
    }
    else
    {
        XERCES_STD_QUALIFIER cerr << "\nFatal Error at file ";
    }

    XERCES_STD_QUALIFIER cerr << StrX(domError.getLocation()->getURI())
         << ", line " << domError.getLocation()->getLineNumber()
         << ", char " << domError.getLocation()->getColumnNumber()
         << "\n  Message: " << StrX(domError.getMessage()) << XERCES_STD_QUALIFIER endl;

    return true;
}

void XSDValidErrorHandler::resetErrors()
{
    fSawErrors = false;
}
