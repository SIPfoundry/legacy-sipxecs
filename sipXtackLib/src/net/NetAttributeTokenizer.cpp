//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <utl/UtlList.h>
#include <net/NetAttributeTokenizer.h>
#include <net/NameValuePair.h>


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
NetAttributeTokenizer::NetAttributeTokenizer(const char* parseString)
{
   if(parseString)
   {
       attributeParseString.append(parseString);
   }
   parseIndex = 0;

}

// Copy constructor
NetAttributeTokenizer::NetAttributeTokenizer(const NetAttributeTokenizer& rNetAttributeTokenizer)
{
}

// Destructor
NetAttributeTokenizer::~NetAttributeTokenizer()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
NetAttributeTokenizer&
NetAttributeTokenizer::operator=(const NetAttributeTokenizer& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

UtlBoolean NetAttributeTokenizer::getNextAttribute(UtlString& attributeName,
                                                  UtlString& attributeValue)
{
    UtlBoolean foundNextAttribute = FALSE;
    const char* attributeSeparators = ", \t\n\r";
    const char* quoteCharacters = "\'\"";
    const char nameValueSeparator = '=';
    const char escapeChar = '\\';
    int stringLen = attributeParseString.length();
    int attributeNameStart = -1;
    int attributeNameEnd = -1;
    int attributeValueStart = -1;
    int attributeValueEnd = -1;
    UtlBoolean valuePresent = FALSE;
    attributeName.remove(0);
    attributeValue.remove(0);

    if(parseIndex < stringLen)
    {
        const char* parseString = attributeParseString.data();

        // Skip attribute separator stuff before the attribute name
        while(strchr(attributeSeparators, parseString[parseIndex]) &&
            parseIndex < stringLen)
        {
            parseIndex++;
        }

        // If a begining of a attribute name exists
        if(parseIndex < stringLen)
        {
            attributeNameStart = parseIndex;
        }

        // Find the end of the attribute name
        while(!strchr(attributeSeparators, parseString[parseIndex]) &&
             parseString[parseIndex] != nameValueSeparator &&
            parseIndex < stringLen)
        {
            parseIndex++;
            attributeNameEnd = parseIndex;
        }

        // Set the attribute name
        if(attributeNameStart >= 0 && attributeNameEnd >= 0)
        {
            attributeName.append(&parseString[attributeNameStart],
                attributeNameEnd - attributeNameStart);
            foundNextAttribute = TRUE;
        }

        // Skip name value separator stuff before the attribute value
        while((strchr(attributeSeparators, parseString[parseIndex]) ||
               parseString[parseIndex] == nameValueSeparator) &&
              parseIndex < stringLen)
        {
            if(parseString[parseIndex] == nameValueSeparator)
            {
                valuePresent = TRUE;
            }
            parseIndex++;
        }

        // If there was an equal sign, there is a value
        if(valuePresent)
        {
            // See of the value is quoted
            if(strchr(quoteCharacters, parseString[parseIndex]))
            {
                parseIndex++;
                attributeValueStart = parseIndex;

                // Need to check for escape character
                // even number contiguous means the quote is not escaped


                // Find the end quote
                int escapeCount = 0;
                while((!strchr(quoteCharacters, parseString[parseIndex]) ||
                    escapeCount % 2) && // an odd number of escape chars means the
                                        // quoteCharacter is escaped
                    parseIndex < stringLen)
                {
                    if(parseString[parseIndex] == escapeChar)
                    {
                        escapeCount++;
                    }
                    else
                    {
                        escapeCount = 0;
                    }
                    parseIndex++;
                    attributeValueEnd = parseIndex;
                }

                // Index past the last quote for the next token
                if(parseIndex < stringLen)
                {
                    parseIndex++;
                }
            }

            // Not quoted find the first white space as separator
            else
            {
                attributeValueStart = parseIndex;

                while(!strchr(attributeSeparators, parseString[parseIndex]) &&
                    parseIndex < stringLen)
                {
                    parseIndex++;
                    attributeValueEnd = parseIndex;
                }
            }

            // Set the attribute value
            if(attributeValueStart >= 0 &&
                attributeValueEnd >= 0)
            {
                attributeValue.append(&parseString[attributeValueStart],
                    attributeValueEnd - attributeValueStart);
            }
        }

    }
    return(foundNextAttribute);
}

UtlBoolean NetAttributeTokenizer::getAttributes(UtlList& attributeList)
{
    UtlBoolean attributesFound = FALSE;
    UtlString name;
    UtlString value;

    while(getNextAttribute(name, value))
    {
        attributeList.insert(new NameValuePair(name.data(), value.data()));
        attributesFound = TRUE;
    }

    name.remove(0);
    value.remove(0);
    return(attributesFound);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
