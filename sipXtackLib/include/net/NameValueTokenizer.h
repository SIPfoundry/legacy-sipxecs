//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _NameValueTokenizer_h_
#define _NameValueTokenizer_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "utl/UtlString.h"

// DEFINES
#define NEWLINE '\n'
#define CARRIAGE_RETURN '\r'
#define CARRIAGE_RETURN_NEWLINE "\r\n"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//: Parses name value pairs from multiple lines of text
//
class NameValueTokenizer
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   NameValueTokenizer(const char* multiLineText, ssize_t textLength = -1);
     //:Default constructor

   virtual
   ~NameValueTokenizer();
     //:Destructor

/* ============================ MANIPULATORS ============================== */
   static ssize_t findNextLineTerminator(const char* text, ssize_t length,
                                     ssize_t* nextLineIndex);
   //: Finds the index to the next line terminator
   //! param: text - the char array in which to search for the
   // terminator
   //! param: length - the length of the text array
   //! param: nextLineIndex - the index to the begining of the next
   // line.  This may be -1 if the end of the string is encountered
   //! returns: index into the text char array to the line terminator
   // Note: the line terminator may be 1 or 2 characters

   static void frontTrim(UtlString* string, const char* whiteSpace);
   static void backTrim(UtlString* string, const char* whiteSpace);
   static void frontBackTrim(UtlString* string, const char* whiteSpace);

   static UtlBoolean getSubField(const char* textField,
                                 ssize_t subfieldIndex,
                                 const char* subfieldSeparator,
                                 UtlString* subfieldText,
                                 ssize_t* lastCharIndex = NULL);

   static UtlBoolean getSubField(const char* textField,
                                 ssize_t textFieldLength,
                                 ssize_t subfieldIndex,
                                 const char* subfieldSeparators,
                                 const char*& subfieldPtr,
                                 ssize_t& subFieldLength,
                                 ssize_t* lastCharIndex);

   /**
    * Split the string (up to the point found by findNextLineTerminator)
    * into two fields by the first instance of 'separator' found.
    * Trim leading and trailing whitespace off the two fields and return
    * them in name and value.
    */
   UtlBoolean getNextPair(char separator, UtlString* name, UtlString* value);

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */
    UtlBoolean isAtEnd();
    ssize_t getProcessedIndex();

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    const char* textPtr;
    ssize_t textLen;
    ssize_t bytesConsumed;

    NameValueTokenizer(const NameValueTokenizer& rNameValueTokenizer);
    //:disable Copy constructor

    NameValueTokenizer& operator=(const NameValueTokenizer& rhs);
    //: disable Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  // _NameValueTokenizer_h_
