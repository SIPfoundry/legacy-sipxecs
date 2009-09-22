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

// APPLICATION INCLUDES
#include <net/NameValueTokenizer.h>
#include <os/OsSysLog.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
NameValueTokenizer::NameValueTokenizer(const char* multiLineText, ssize_t length)
{
    if(multiLineText && length < 0)
    {
        length = strlen(multiLineText);
    }

   textPtr = multiLineText;
   bytesConsumed = 0;
   textLen = length;
}

// Copy constructor
NameValueTokenizer::NameValueTokenizer(const NameValueTokenizer& rNameValueTokenizer)
{
}

// Destructor
NameValueTokenizer::~NameValueTokenizer()
{
}

/* ============================ MANIPULATORS ============================== */

ssize_t NameValueTokenizer::findNextLineTerminator(const char* text, ssize_t length, ssize_t* nextLineIndex)
{
    ssize_t byteIndex = 0;
    ssize_t terminatorIndex = -1;
    *nextLineIndex = -1;
    //char textChar;

    while(byteIndex < length)
    {
        //textChar = *text;
        if(text[byteIndex] == NEWLINE || text[byteIndex] == CARRIAGE_RETURN)
        {
            terminatorIndex = byteIndex;
            // Check for NL after CR
            if(byteIndex < length - 1 && text[byteIndex + 1] == NEWLINE &&
                text[byteIndex] == CARRIAGE_RETURN)
            {
                *nextLineIndex = terminatorIndex + 2;
            }
            else
            {
                *nextLineIndex = terminatorIndex + 1;
            }
            break;
        }
        //text++;
        byteIndex++;
    }

    return(terminatorIndex);
}

void NameValueTokenizer::frontTrim(UtlString* string, const char* whiteSpace)
{
    ssize_t len = 0;
    ssize_t index = 0;
    const char* stringData;
    if(string != NULL)
    {
        len = string->length();
        stringData = string->data();
        while(index < len && strchr(whiteSpace, stringData[index]))
        {
            index++;
        }
        if(index > 0)
        {
            string->remove(0, index);
        }
    }
}

void NameValueTokenizer::backTrim(UtlString* string, const char* whiteSpace)
{
    ssize_t len = 0;
    ssize_t index = 0;
    const char* stringData;
    if(string != NULL)
    {
        len = string->length();
        stringData = string->data();
        while(index < len && strchr(whiteSpace, stringData[len - index - 1]))
        {
            index++;
        }
        if(index > 0)
        {
            string->remove(len - index, index);
        }
    }
}

void NameValueTokenizer::frontBackTrim(UtlString* string, const char* whiteSpace)
{
    frontTrim(string, whiteSpace);
    backTrim(string, whiteSpace);
}

UtlBoolean NameValueTokenizer::getSubField(const char* textField,
                                           ssize_t textFieldLength,
                                           ssize_t subFieldIndex,
                                           const char* subFieldSeparators,
                                           const char*& subFieldPtr,
                                           ssize_t& subFieldLength,
                                           ssize_t* lastCharIndex)
{
    UtlBoolean found = FALSE;
    if(textField)
    {
#if 0
       printf("NameValueTokenizer::getSubField textField = '%s', textFieldLength = %zu, subFieldIndex = %zu, subFieldSeparators = '%s'\n",
              textField, textFieldLength, subFieldIndex,
              subFieldSeparators);
#endif
    ssize_t subFieldI = -1;
    ssize_t subFieldBegin = 0;
    ssize_t separatorIndex = -1;
    size_t numSeparators = strlen(subFieldSeparators);

    for(ssize_t charIndex = 0; subFieldI < subFieldIndex; charIndex++)
    {
        if((textFieldLength >= 0 &&
        charIndex >= textFieldLength) ||
        textField[charIndex] == '\0')
        {
        subFieldI++;
        subFieldBegin = separatorIndex + 1;
        separatorIndex = charIndex;
        break;
        }

        // If we found a separator character
             // 1 separator check it directly, it is much faster
        else if((numSeparators == 1 &&
             subFieldSeparators[0] == textField[charIndex]) ||
            // 2 separator characters, check both:
            (numSeparators == 2 &&
             (subFieldSeparators[0] == textField[charIndex] ||
              subFieldSeparators[1] == textField[charIndex])) ||
            // 3 or more separator characters, do it the slow way:
            (numSeparators > 2 &&
             strchr(subFieldSeparators, textField[charIndex])))
        {
        subFieldBegin = separatorIndex + 1;
        separatorIndex = charIndex;

        // Ignore empty subfields (i.e. they do not count
        if(subFieldBegin != separatorIndex)
        {
            subFieldI++;
        }
        }
    }

    if(subFieldI == subFieldIndex)
    {
        found = TRUE;
        //subfieldText->append(&(textField[subfieldBegin]),
        //    separatorIndex - subfieldBegin);
        //subfieldText->replace(0, separatorIndex - subfieldBegin,
        //    &(textField[subfieldBegin]), separatorIndex - subfieldBegin);
        //subfieldText->remove(separatorIndex - subfieldBegin);
        subFieldPtr = &(textField[subFieldBegin]);
        subFieldLength = separatorIndex - subFieldBegin;
#if 0
        printf("NameValueTokenizer::getSubField subField = '%.*s'\n",
               subFieldLength, subFieldPtr);
#endif

        if(lastCharIndex) *lastCharIndex = separatorIndex;
    }
    }

    if(!found)
    {
    subFieldPtr = NULL;
    subFieldLength = 0;

    if(lastCharIndex) *lastCharIndex = 0;
    }

    return(found);
}

UtlBoolean NameValueTokenizer::getSubField(const char* textField,
                                           ssize_t subFieldIndex,
                                           const char* subFieldSeparators,
                                           UtlString* subFieldText,
                                           ssize_t* lastCharIndex)
{
    ssize_t subFieldLength = 0;
    const char* subFieldPtr = NULL;

    UtlBoolean found = getSubField(textField,
                                   -1, // stop at null (i.e. '\0')
                                   subFieldIndex,
                                   subFieldSeparators,
                                   subFieldPtr,
                                   subFieldLength,
                                   lastCharIndex);

    if(subFieldPtr && subFieldLength > 0)
    {
    subFieldText->replace(0, subFieldLength, subFieldPtr, subFieldLength);
    subFieldText->remove(subFieldLength);
    }

    else
    {
    subFieldText->remove(0);
    }

    return(found);
}

UtlBoolean NameValueTokenizer::getNextPair(char separator, UtlString* name,
                                           UtlString* value)
{
   UtlBoolean nameFound = 0;
   name->remove(0);
   value->remove(0);

   ssize_t nextLineOffset;

   // Find the end of the line and the begining of the next
   ssize_t lineLength = findNextLineTerminator(&textPtr[bytesConsumed],
                                           textLen - bytesConsumed,
                                           &nextLineOffset);

   // Did not find an end of line, assume the rest of the text is the line
   if(lineLength < 0)
   {
      lineLength = textLen - bytesConsumed;
   }

   // Add a header field
   if(lineLength > 0)
   {
      // Find the name value delimiter
      ssize_t nameEnd = 0;
      while(nameEnd < lineLength &&
            textPtr[bytesConsumed + nameEnd] != separator)
      {
         nameEnd++;
      }

      if(nameEnd > 0)
      {
         name->append(&textPtr[bytesConsumed], nameEnd);
         name->strip(UtlString::both); // remove any leading and trailing whitespace
         nameFound = !name->isNull();
      }

      // Skip over the separator.
      nameEnd++;

      if(nameEnd < lineLength)
      {
         value->append(&textPtr[bytesConsumed + nameEnd],
                       lineLength - nameEnd);
         value->strip(UtlString::both); // remove any leading and trailing whitespace
      }
   }

   if(nextLineOffset > 0)
   {
      bytesConsumed += nextLineOffset;
   }
   else
   {
      bytesConsumed += lineLength;
   }
   return(nameFound);
}


// Assignment operator
NameValueTokenizer&
NameValueTokenizer::operator=(const NameValueTokenizer& rhs)
{
   if (this == &rhs)        // handle the assignment to self case
      return *this;

   return *this;
}

/* ============================ ACCESSORS ================================= */

ssize_t NameValueTokenizer::getProcessedIndex()
{
   return(bytesConsumed);
}

/* ============================ INQUIRY =================================== */
UtlBoolean NameValueTokenizer::isAtEnd()
{
   return(bytesConsumed >= textLen);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */
