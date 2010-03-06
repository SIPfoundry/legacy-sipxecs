//
// Copyright (C) 2010 Avaya Inc., certain elements licensed under a Contributor Agreement.
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
#include <string.h>
#include <ctype.h>

// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "os/OsDefs.h"
#include "utl/UtlRegex.h"
#include <os/OsSysLog.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define UTLSTRING_MIN_INCREMENT 100 ///< smallest additional memory to be allocated
#define SWS "\\s*"
#define TEST_FINDTOKEN 1

// STATIC VARIABLE INITIALIZATIONS
const UtlContainableType UtlString::TYPE = "UtlString";
const char* UtlString::ssNull = "";
const ssize_t UtlString::UTLSTRING_NOT_FOUND = -1;
const ssize_t UtlString::UTLSTRING_TO_END = -1;

const size_t  UtlString::MAX_NUMBER_STRING_SIZE = 256; // maximum size text output for appendNumber


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Default Constructor
UtlString::UtlString()
{
    mpData = mBuiltIn;
    mBuiltIn[0] = '\000';
    mSize = 0;
    mCapacity = DEFAULT_UTLSTRING_CAPACITY;
}


// Copy constructor
UtlString::UtlString(const char* szSource)
{
    mpData = mBuiltIn;
    mBuiltIn[0] = '\000';
    mSize = 0;
    mCapacity = DEFAULT_UTLSTRING_CAPACITY;
    append(szSource);
}

// Copy constructor
UtlString::UtlString(const char* szSource, size_t length)
{
    mpData = mBuiltIn;
    mBuiltIn[0] = '\000';
    mSize = 0;
    mCapacity = DEFAULT_UTLSTRING_CAPACITY;
    append(szSource, length);
}

// Copy constructor
UtlString::UtlString(const UtlString& source)
{
    mpData = mBuiltIn;
    mBuiltIn[0] = '\000';
    mSize = 0;
    mCapacity = DEFAULT_UTLSTRING_CAPACITY;
    capacity(source.mCapacity);
    append(source);
}


// Copy constructor
UtlString::UtlString(const UtlString& source, size_t length)
{
    mpData = mBuiltIn;
    mBuiltIn[0] = '\000';
    mSize = 0;
    mCapacity = DEFAULT_UTLSTRING_CAPACITY;
    capacity(source.mCapacity);

    // Check that we do not copy beyond the end of source
    if(length > source.mSize)
    {
        length = source.mSize;
    }
    append(source.mpData, length);
}


// Destructor
UtlString::~UtlString()
{
    if(mpData && mpData != mBuiltIn)
    {
        delete[] mpData;
    }
    mpData = NULL;
    mCapacity = 0;
    mSize = 0;
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator, use append(const char*).
UtlString& UtlString::operator=(const char* szStr)
{
    remove(0);
    if(szStr && *szStr)
    {
        append(szStr);
    }

    return *this;
}


// Assignment operator, use append(const char* , size_t ).
UtlString& UtlString::operator=(const UtlString& str)
{
    if (this != &str)
    {
       remove(0);
       if(str.mCapacity > mCapacity)
       {
          capacity(str.mCapacity);
       }
       append(str.mpData, str.mSize);
    }

    return *this;
}


// Plus equals operator, use append(const char*).
UtlString& UtlString::operator+=(const char* szStr)
{
    return append(szStr);
}


// Plus equals operator, use append(const UtlString& ).
UtlString& UtlString::operator+=(const UtlString& str)
{
    return append(str);
}

// Plus equals operator, use append(const char).
UtlString& UtlString::operator+=(const char c)
{
    return append(c);
}

// Get the character at position N.
char UtlString::operator()(size_t N) const
{
    char foundChar = '\000';

    if (mpData && N >= 0 && N < mSize)
    {
        foundChar = mpData[N];
    }

    return foundChar;
}


// prepend:prepend a copy of the null-terminated character string pointed
// to by szStr to self.Returns a reference to self.
// use append().
UtlString& UtlString::prepend(const char* szStr)
{
    UtlString newUtlString;

    //FIX: should avoid allocation if capacity is big enough
    newUtlString.append(szStr);
    newUtlString.append(mpData);
    remove(0);
    append(newUtlString);

    return *this ;
}


// Append the designated string to the end of this string.
// use append(const char* szStr, size_t N)
UtlString& UtlString::append(const char* szStr)
{
    if (szStr)
    {
        append(szStr, strlen(szStr));
    }

    return *this;
}


// Append the designated character to the end of this string.
// To append a string, use append(const char* szStr, size_t N)
UtlString& UtlString::append(const char c)
{
    append(&c, 1);

    return *this;
}


// Append up to N bytes of the designated string to the end of this string.
UtlString& UtlString::append(const char* szStr, size_t N)
{
    // The N bytes to append may contain zero bytes.
    if(szStr && N>0 && N!=(size_t)UTLSTRING_NOT_FOUND)
    {
        // Calculate the total space needed, including room for a final
        // zero byte.
        size_t maxCap = mSize + N + 1;
        // If necessary, reallocate the data area to hold maxCap bytes.
        if (maxCap <= capacity(maxCap))
        {
            // Copy the N bytes after the existing mSize bytes in the string.
            memcpy(&mpData[mSize], szStr, N);
            // Update the size of the string.
            mSize += N;
            // Append a final zero byte.
            mpData[mSize] = '\000';
        }
        else
        {
            //allocate failure, do nothing;
        }
    }

    return *this;
}

// Append the designated string to the end of this string.
// use append(const char* ).
UtlString& UtlString::append(const UtlString& str)
{
   append(str.mpData, str.mSize);

   return *this ;
}

// Append the designated string to the end of this string.
// use append(const char* ).
UtlString& UtlString::append(const UtlString& str, size_t position, size_t length)
{
   if (position < str.mSize)
   {
      if (   (length == (size_t)UTLSTRING_TO_END)
          || (position+length > str.mSize)
          )
      {
         length = str.mSize - position;
      }

      append(str.mpData+position, length);
   }

   return *this;
}

UtlString& UtlString::appendFormattedNumber(int formatResult, char* conversionString, const char* format)
{
   if ((0 < formatResult) && ((int)MAX_NUMBER_STRING_SIZE > formatResult))
   {
      append(conversionString);
   }
   else
   {
      append("(UtlString::appendNumber format error using '");
      append(format);
      append("')");
   }

   return(*this);
}

UtlString& UtlString::appendNumber(int value, const char* format)
{
   char conversionString[MAX_NUMBER_STRING_SIZE];
   conversionString[0]='\000';

   int formatResult = snprintf(conversionString,MAX_NUMBER_STRING_SIZE,format,value);
   return appendFormattedNumber(formatResult, conversionString, format);
}

UtlString& UtlString::appendNumber(size_t value, const char* format)
{
   char conversionString[MAX_NUMBER_STRING_SIZE];
   conversionString[0]='\000';

   int formatResult = snprintf(conversionString,MAX_NUMBER_STRING_SIZE,format,value);
   return appendFormattedNumber(formatResult, conversionString, format);
}

#if __WORDSIZE == 64
UtlString& UtlString::appendNumber(ssize_t value, const char* format)
{
   char conversionString[MAX_NUMBER_STRING_SIZE];
   conversionString[0]='\000';

   int formatResult = snprintf(conversionString,MAX_NUMBER_STRING_SIZE,format,value);
   return appendFormattedNumber(formatResult, conversionString, format);
}
#else
UtlString& UtlString::appendNumber(long value, const char* format)
{
   char conversionString[MAX_NUMBER_STRING_SIZE];
   conversionString[0]='\000';

   int formatResult = snprintf(conversionString,MAX_NUMBER_STRING_SIZE,format,value);
   return appendFormattedNumber(formatResult, conversionString, format);
}
#endif

UtlString& UtlString::appendNumber(Int64 value, const char* format)
{
   char conversionString[MAX_NUMBER_STRING_SIZE];
   conversionString[0]='\000';

   int formatResult = snprintf(conversionString,MAX_NUMBER_STRING_SIZE,format,value);
   return appendFormattedNumber(formatResult, conversionString, format);
}

UtlString& UtlString::insert(size_t position, const char* source)
{
   if(source && position < mSize)
   {
      size_t len = strlen(source);
      insert(position, source, len);
   }
   else if (source && position == mSize)
   {
      append(source);
   }

   return(*this);
}

UtlString& UtlString::insert(size_t position, const char* source, size_t sourceLength)
{
   if(position <= mSize)
   {
      if(mCapacity < mSize + sourceLength + 1)
      {
         capacity(mSize + sourceLength + 1);
      }

      memmove(&mpData[position + sourceLength],
              &mpData[position],
              mSize - position);

      memcpy(&mpData[position],
             source,
             sourceLength);

      mSize+= sourceLength;
      mpData[mSize] = '\000';
   }

   // Else do nothing

   return(*this);
}

UtlString& UtlString::insert(size_t position, const char source)
{
   return(insert(position, &source, 1));
}

// Insert the designated string starting at character position start.
UtlString& UtlString::insert(size_t start, const UtlString& str)
{
    if(start >= 0 && start <= mSize && str.mpData && str.mSize > 0)
    {
        insert(start, str.mpData, str.mSize);
    }
    else
    {
        //do nothing.
    }

    return *this;
}


// Remove all characters after the specified position.
UtlString& UtlString::remove(size_t pos)
{
    if(mpData && pos >= 0 && pos < mSize)
    {
        mSize = pos;
        mpData[mSize] = '\000';
    }
    else
    {
        //do nothing
    }

    return *this;
}


// Remove N characters from this string starting at designated position.
UtlString& UtlString::remove(size_t pos, size_t N)
{
    if(mpData && N > 0 && N <= mSize - pos && pos >= 0 && pos < mSize)
    {
        // Add one extra byte for the '\000'
        size_t bytesToShift = mSize - (pos + N) + 1;

        // memcpy() cannot be used here due to possible overlap of source
        // and destination areas!
        memmove(&mpData[pos], &mpData[pos + N], bytesToShift);
        mSize -= N;
        mpData[mSize] = '\000';
    }
    else
    {
        //do nothing
    }

    return *this;
}


// Replace a single character at the designated position
void UtlString::replaceAt(size_t pos, char newChar)
{
   if (mpData && mSize > pos)
   {
      mpData[pos] = newChar;
   }
}


// Replace N characters starting at the designated position with the
// designated replacement string.
// use replace(size_t pos, size_t N, const char* replaceStr, size_t L).
UtlString& UtlString::replace(size_t pos, size_t N, const char* replaceStr)
{
    return replace(pos, N, replaceStr, strlen(replaceStr));
}

// Replace N characters starting at the designated position with the
// designated replacement string.
UtlString& UtlString::replace(size_t pos, size_t N, const UtlString& replaceStr)
{
    return replace(pos, N, replaceStr.mpData, replaceStr.mSize);
}

// Replace N characters starting at the designated position with a subset
// of the designated replacement string.
// use strncpy() to realize.
UtlString& UtlString::replace(size_t pos, size_t N, const char* replaceStr, size_t L)
{
    // FIX: need to avoid the allocation and extra copy if possible
    if (replaceStr != NULL && pos >= 0 && N >= 0 && strlen(replaceStr) >= L)
    {
        UtlString newUtlString;
        newUtlString.append(mpData, pos);
        newUtlString.append(replaceStr, L);
        if(int(mSize - N - pos) > 0)                //can not change to size_t
        {
            newUtlString.append(&mpData[pos + N], mSize - N - pos);
        }
        remove(0);
        append(newUtlString);
    }
    else
    {
        //do nothing;
    }

    return *this;
}


// Replace all instances of character src with character tgt
UtlString& UtlString::replace(const char src, const char tgt)
{
    if (mpData && (src!=0) && (tgt!=0))
    {
        for (size_t i=0; i<mSize; i++)
        {
            if (mpData[i] == src)
            {
                mpData[i] = tgt ;
            }
        }
    }
    return *this ;
}


// Removes whitespace (space, tab, Cr, Lf) from the end of the string.
// use strip(StripType type)
UtlString UtlString::strip()
{
    return strip(trailing);
}


// Removes whitespace (space, tab, Cr, Lf) from the beginning of the string,
// from the end of the string, or from both the beginning and end of the string.
// use strip(StripType , char ).
UtlString UtlString::strip(StripType type)
{
    if(mpData && mSize > 0)
    {
        if(type == both)
        {
            strip(leading);     //Here I use the method of recursion.
            strip(trailing);
        }
        else if(type == leading)
        {
            unsigned int removeLeading = 0;
            const char* leadingPtr = mpData;
            while(removeLeading < mSize &&
                  (*leadingPtr == '\t' ||
                   *leadingPtr == ' ' ||
                   *leadingPtr == '\n' ||
                   *leadingPtr == '\r'))
            {
                removeLeading++;
                leadingPtr++;
            }
            if(removeLeading > 0)
            {
                remove(0, removeLeading);
            }
        }
        else
        {
            unsigned int removeTrailing = 0;
            const char* trailingPtr = mpData + mSize - 1;
            while(removeTrailing < mSize &&
                  (*trailingPtr == '\t' ||
                   *trailingPtr == ' ' ||
                   *trailingPtr == '\n' ||
                   *trailingPtr == '\r'))
            {
                removeTrailing++;
                trailingPtr--;
            }
            if(removeTrailing > 0)
            {
                remove(mSize - removeTrailing);
            }
        }
    }

    return  *this;
}


// Remove the designated character from the beginning of the string,
// from the end of the string, or from both the beginning and end of
// the string.
// use remove(size_t pos, size_t N)
UtlString UtlString::strip(StripType type , char toBeStriped)
{
   if(mpData && mSize > 0)
   {
        if (type == leading || type == both)
        {
            unsigned int removeLeading = 0;
            while(mSize > removeLeading &&
                  mpData[removeLeading] == toBeStriped)
            {
                removeLeading++;
            }

            if(removeLeading > 0)
            {
                remove(0, removeLeading);
            }
        }

        if(type == trailing || type == both)
        {
            unsigned int removeTrailing = 0;
            while(mSize > removeTrailing &&
                  mpData[mSize - 1 - removeTrailing] == toBeStriped)
            {
                removeTrailing++;
            }
            if(removeTrailing > 0)
            {
                remove(mSize - removeTrailing);
            }
        }
    }

    return *this;
}


// Convert the string to all lower case characters.
// use tolower(char*) of Standard C++ Library.
void UtlString::toLower()
{
    if(mpData)
    {
        char* charPtr;

        for(size_t i = 0; i < mSize; i++)
        {
            charPtr = &mpData[i];
            *charPtr = tolower(*charPtr);
        }
    }
}


// Convert the string to all upper case characters.
// use toUpper(char*) of Standard C++ Library
void UtlString::toUpper()
{
    if(mpData)
    {
        char* charPtr;

        for(size_t i = 0; i < mSize; i++) //waring point
        {
            charPtr = &mpData[i];
            *charPtr = toupper(*charPtr);
        }
    }
}

void UtlString::setLength(size_t newLength)
{
   assert(newLength+1 <= mCapacity);

   if (newLength+1 <= mCapacity)
   {
      mSize = newLength;
      mpData[mSize] = '\000';
   }
}

// Resize the string to the specified size.
// use capacity(size_t).
void UtlString::resize(size_t N)
{
    // CHECK: is this what it is suppoesed to do???
    if(N > mSize)
    {
        if (mCapacity <= N)
        {
            capacity(N + 1);
        }

        if(mpData)
        {
            for (; mSize < N; mSize++)
            {
                mpData[mSize] = '\000';
            }
            mpData[mSize] = '\000';
        }
    }
    else
    {
        remove(N);
    }
}

// Set the string's storage capacity to the designated value.
size_t UtlString::capacity(size_t N)
{
#ifdef _VXWORKS
    size_t maxFreeBlockSize = 0;
#endif
    char* newData = 0;

    if(mCapacity < N && N > 0)
    {
        if(mCapacity + UTLSTRING_MIN_INCREMENT > N)
        {
            N = mCapacity + UTLSTRING_MIN_INCREMENT;
        }
#ifdef _VXWORKS
        if (N > CHECK_BLOCK_THRESHOLD)
        {
            maxFreeBlockSize = FindMaxFreeBlock();
            // We are not going over the largest block size then try to
            // allocate otherwise we will return NULL, and the size returned
            // will be zero!
            if (N < maxFreeBlockSize)
            {
                newData = new char[N];
            }
        }
        else
        {
            newData = new char[N];
        }
#else

        //for all other OS's lets assume it really returns NULL and not crashes
        newData = new char[N];
#endif

        if (newData)
        {
            if(mSize > 0 && mpData)
            {
                memcpy(newData, mpData, mSize);
            }
            else
            {
                newData[0] = '\000';
            }
            if(mpData && mpData != mBuiltIn)
            {
                delete[] mpData;
            }
            mpData = newData;
            mCapacity = N;
        }
    }

    return(mCapacity);
}


//Addition operator
UtlString operator+(const UtlString& str1, const UtlString& str2)
{
    UtlString NewS = str1;
    NewS.append(str2);

    return NewS;

}


//Addition operator
UtlString operator+(const UtlString& str, const char* szStr)
{
    UtlString NewS = str;
    NewS.append(szStr);

    return NewS;
}


//Addition operator
UtlString operator+(const char* szStr, const UtlString& str)
{
    UtlString newS(szStr);
    newS.append(str);

    return newS;
}

/* ============================ ACCESSORS ================================= */

// Returns the index of the first occurrence of the character searchChar in self.
// Returns "-1" if there is no such character or if there is an embedded
// null prior to finding c.
// use index().
ssize_t UtlString::first(char searchChar) const
{
    return index(searchChar) ;
}


// Find the first instance of the designated string or UTLSTRING_NOT_FOUND
// if not found.
ssize_t UtlString::first(const char * searchStr) const
{
    return index(searchStr);
}


// Return the string length.
size_t UtlString::length() const
{
    return mSize;
}


// Return a read-only pointer to the underlying data.
const char* UtlString::data() const
{
    return(mpData ? mpData : ssNull);
}


// Return the index of the designated character or UTLSTRING_NOT_FOUND
// if not found.
ssize_t UtlString::index(char searchChar) const
{
    return index(searchChar, 0);
}


// Return the index of the designated character starting at the
// designated postion or UTLSTRING_NOT_FOUND if not found.
ssize_t UtlString::index(char searchChar, size_t start) const
{
    ssize_t foundPosition = UTLSTRING_NOT_FOUND;
    if(mpData)
    {
        size_t startIndex = start;

        for(size_t pos = startIndex;
            pos < mSize && foundPosition == UTLSTRING_NOT_FOUND;
            pos++)
        {
            if(mpData[pos] == searchChar)
            {
                 foundPosition = pos;
            }
        }
    }

    return foundPosition;
}

// Optimization to avoid strlen call as well as the only
// safe way to work with binary or opaque data
ssize_t UtlString::index(const UtlString& searchString) const
{
    return(index(searchString, 0));
}

// Optimization to avoid strlen call as well as the only
// safe way to work with binary or opaque data
ssize_t UtlString::index(const UtlString& searchString, size_t start) const
{
    ssize_t foundPosition = UTLSTRING_NOT_FOUND;
    // mpData may be null, so use data() which returns an
    // static empty string if mpData is null
    const char* dataPtr = data();
    size_t searchStrSize = searchString.length();
    size_t startIndex = start;

    if(searchStrSize <= mSize)
    {
        for (size_t pos = startIndex;
             pos <= (mSize - searchStrSize)
             && foundPosition == UTLSTRING_NOT_FOUND;
             pos++)
        {
            if (memcmp(dataPtr + pos, searchString.data(), searchStrSize) == 0)
            {
                foundPosition = pos;
            }
        }
    }

    return foundPosition;
}

// Return the index of the designated substring starting at the
// designated position or UTLSTRING_NOT_FOUND  if not found.
// Optimization to avoid strlen call as well as the only
// safe way to work with binary or opaque data
ssize_t UtlString::index(const UtlString& searchString, size_t start, CompareCase type) const
{
    ssize_t foundPosition = UTLSTRING_NOT_FOUND;
    size_t searchStrSize = searchString.length();
    size_t startIndex = start;

    if (type == matchCase)
    {

        foundPosition = index(searchString, start);
    }
    else
    {
        // mpData may be null, so use data() which returns an
        // static empty string if mpData is null
        const char* dataPtr = data();

        if(searchStrSize <= mSize)
        {
            for (size_t pos = startIndex;
                 pos <= (mSize - searchStrSize)
                 && foundPosition == UTLSTRING_NOT_FOUND;
                 pos++)
            {
                if(strncasecmp(dataPtr + pos,
                               searchString.data(),
                               searchStrSize) == 0)
                {
                    foundPosition = pos;
                }
            }
        }
    }

    return foundPosition;
}

// Return the index of the designated substring or UTLSTRING_NOT_FOUND if not found.
// Pattern matching. I think the arithmetic should be optimized.
// such as KMP: http://www.ics.uci.edu/~eppstein/161/960227.html
ssize_t UtlString::index(const char* searchStr) const
{
    return index(searchStr, 0);
}


// Return the index of the designated substring starting at the
// designated position or UTLSTRING_NOT_FOUND if not found.
ssize_t UtlString::index(const char* searchStr, size_t start) const
{
    ssize_t foundPosition = UTLSTRING_NOT_FOUND;
    if(searchStr)
    {
        // mpData may be null, so use data() which returns an
        // static empty string if mpData is null
        const char* dataPtr = data();

        size_t searchStrSize = strlen(searchStr);
        size_t startIndex = start;

        if(searchStrSize <= mSize)
        {
            for (size_t pos = startIndex;
                 pos <= (mSize - searchStrSize)
                 && foundPosition == UTLSTRING_NOT_FOUND;
                 pos++)
            {
                if (memcmp(dataPtr + pos, searchStr, searchStrSize) == 0)
                {
                    foundPosition = pos;
                }
            }
        }
    }

    return foundPosition;
}


// Return the index of the designated substring starting at the
// designated position or UTLSTRING_NOT_FOUND  if not found.
ssize_t UtlString::index(const char* searchStr, size_t start, CompareCase type) const
{
    ssize_t foundPosition = UTLSTRING_NOT_FOUND;
    if(searchStr)
    {
        size_t searchStrSize = strlen(searchStr);
        size_t startIndex = start;

        if (type == matchCase)
        {

            foundPosition = index(searchStr, start);
        }
        else
        {
            // mpData may be null, so use data() which returns an
            // static empty string if mpData is null
            const char* dataPtr = data();

            if(searchStrSize <= mSize)
            {
               for (size_t pos = startIndex;
                    pos <= (mSize - searchStrSize)
                    && foundPosition == UTLSTRING_NOT_FOUND;
                    pos++)
               {
                   if(strncasecmp(dataPtr + pos, searchStr, searchStrSize) == 0)
                   {
                       foundPosition = pos;
                   }
               }
            }

        }
    }

    return foundPosition;
}


// Returns the index of the last occurrence in the string of the character s.
ssize_t UtlString::last(char searchChar) const
{
    ssize_t foundPosition = UTLSTRING_NOT_FOUND;
    if(mpData)
    {
        size_t startIndex = 0;

        for(size_t pos = startIndex; pos < mSize; pos++)
        {
            if(mpData[pos] == searchChar)
            {
                foundPosition = pos;
            }
        }
    }

    return foundPosition;
}


// Return the storage capacity allocated for this string.
size_t UtlString::capacity() const
{
    return mCapacity;
}


// Returns an UtlString of self with length len, starting at index.
// Just return a copy.
UtlString UtlString::operator() (size_t start, size_t len) const
{
   UtlString newUtlString;

   // Note that since unsigned arithmetic can overflow, we have to test that
   // start + len >= start.
   if (mpData && start <= mSize && len == (size_t)UTLSTRING_TO_END )
   {
      // Get all of the string after start.
      newUtlString.append(&mpData[start], mSize - start);
   } else if (mpData && start <= start + len && start + len <= mSize)
   {
      // Get len characters.
      newUtlString.append(&mpData[start], len);
   }

   return newUtlString;
}


#if 0
UtlString UtlString::operator()(const RWCRegexp& re) const
{

}
#endif /* 0 */


// Cast this object to a const char* pointer.
UtlString::operator const char*() const
{
    return (data());
}


// Returns a hash value.
unsigned UtlString::hash() const
{
    // Need to use data() in case mpData is null
    const char* pHashData = data();
    size_t hashSize = mSize;
    unsigned hashValue = 0;

    while (hashSize > 0)
    {
        hashValue = (hashValue << 5) - hashValue + *pHashData;
        pHashData++;
        hashSize--;
    }

    return hashValue;
}


// Get the ContainableType for a UtlContainable derived class.
UtlContainableType UtlString::getContainableType() const
{
    return UtlString::TYPE;
}


UtlBoolean UtlString::findToken(const char* token,
                                const char* delimiter,
                                const char* suffix) const
{
    RegEx*   ptmpRegEx = NULL;
    UtlBoolean  matched = FALSE;

    // build regular expression
    UtlString regExpStr;
    // find beginning of line or delimiter
    regExpStr.append("(^|");
    regExpStr.append(delimiter);
    regExpStr.append(")");

    // allow whitespace around token
    regExpStr.append(SWS);
    // "\Q" causes all characters to be treated as non-special until "\E".
    regExpStr.append("\\Q");
    regExpStr.append(token);
    regExpStr.append("\\E");
    regExpStr.append(SWS);

    // find another delimiter, end of line or (optional) suffix
    regExpStr.append("(");
    regExpStr.append(delimiter);
    if (suffix)
    {
       regExpStr.append("|");
       regExpStr.append(suffix);
    }
    regExpStr.append("|$)");
    // "(^|" delimiter ")" SWS "\Q" token "\E" SWS "(" delimiter "|" suffix "|$)");
    // e.g., with delimiter = "," and suffix= ";" 
    //      '( ^|,) SWS \Q token \E SWS (,|;|$)'
    // e.g., with delimiter = "," and without suffix
    //      '( ^|,) SWS \Q token \E SWS (,|$)'

#ifdef TEST_FINDTOKEN
    OsSysLog::add( FAC_LOG, PRI_DEBUG
                  ,"UtlString::findRegEx: "
                   "built regexp '%s' to find '%s' with delimiter '%s' "
                   "suffix '%s'"
                  ,regExpStr.data(),token, delimiter,
                   suffix);
#endif

    try
    {
        ptmpRegEx = new RegEx(regExpStr.data());
    }
    catch(const char* compileError)
    {
        OsSysLog::add( FAC_LOG, PRI_ERR
                      ,"UtlString::findRegEx: "
                       "Invalid regexp '%s' for '%s': "
                       "compile error '%s'"
                      ,regExpStr.data()
                      ,data()
                      ,compileError
                      );
    }

    if (ptmpRegEx)
    {
        matched = ptmpRegEx->Search(data());
    }

#ifdef TEST_FINDTOKEN
    OsSysLog::add( FAC_LOG, PRI_DEBUG
                  ,"UtlString::findRegEx: "
                   "'%s' with delimiter '%s' %sfound in '%s': "
                  ,token
                  ,delimiter
                  ,(matched ? "":"not ")
                  ,data()
                  );
#endif
    return matched;
}


/* ============================ INQUIRY =================================== */

// Not equals operator.
// use compareTo().
UtlBoolean operator!=(const char* compareStr, const UtlString& compareUtlStr)
{
    return compareUtlStr.compareTo(compareStr) != 0;
}


// Not equals operator.
// compare the first byte of str and the char.
UtlBoolean operator!=(const char compareChar, const UtlString& compareUtlStr)
{
    UtlBoolean operatorFlag = TRUE;

    if (compareUtlStr.mpData &&
        compareUtlStr.mSize == 1 &&
        compareUtlStr.mpData[0] == compareChar)
    {
        operatorFlag = FALSE;
    }

    return operatorFlag;
}


// Equals operator.
// compare the first byte of str and the char.
UtlBoolean operator==(const char compareChar, const UtlString& compareUtlStr)
{
    UtlBoolean operatorFlag = FALSE;

    if (compareUtlStr.mpData &&
        compareUtlStr.mSize == 1 &&
        compareUtlStr.mpData[0] == compareChar)
    {
        operatorFlag = TRUE;
    }

    return operatorFlag;
}


UtlBoolean operator==(const char* compareStr, const UtlString& compareUtlStr)
{
    return compareUtlStr.compareTo(compareStr) == 0;
}

UtlBoolean UtlString::operator==(const char* compareStr) const
{
    return compareTo(compareStr) == 0;
}

UtlBoolean UtlString::operator==(const UtlString& compareStr) const
{
    return compareTo(compareStr) == 0;
}

UtlBoolean UtlString::operator!=(const char* compareStr) const
{
    return compareTo(compareStr) != 0;
}

UtlBoolean UtlString::operator!=(const UtlString& compareStr) const
{
    return compareTo(compareStr) != 0;
}



// Compare this object to another like-objects.
int UtlString::compareTo(UtlContainable const * compareContainable) const
{
    int compareFlag = -1;

    if (compareContainable
                && compareContainable->isInstanceOf(UtlString::TYPE)
           )
    {
        compareFlag = compareTo(((UtlString*) compareContainable)->data());
    }

    return compareFlag;
}


// Test this object to another like-object for equality.
UtlBoolean UtlString::isEqual(UtlContainable const * compareContainable) const
{
    return (compareTo(compareContainable) == 0);
}


// Compare this object to the specified string (case sensitive).
int UtlString::compareTo(const char * compareStr) const
{
    int compareFlag;

    if (compareStr)
    {
        compareFlag = compareTo(compareStr, matchCase);
    }
    else
    {
       // compareStr == NULL means it represents the null string.
       compareFlag = mSize > 0 ? 1 : 0;
    }

    return(compareFlag);
}


// Compare this object to the specified string with option of forcing
// either a case insensitive compare or a case sensitive compare.
int UtlString::compareTo(UtlString const * compareStr, CompareCase type) const
{
    int compareFlag = -1;

    if (compareStr->isInstanceOf(UtlString::TYPE))
    {
        if (mSize == 0 || compareStr->mSize == 0 )
        {
            if (compareStr->mSize != mSize)
            {
                compareFlag = mSize > compareStr->mSize ? 1 : -1;
            }
            else
            {
                compareFlag = 0;
            }
        }
        else
        {
            if (type == matchCase)
            {
                // BUG: Need to use memcmp mpData may have null char before end
                compareFlag = strcmp(mpData ? mpData : "", compareStr->mpData);
            }
            else
            {
                compareFlag = strcasecmp(mpData ? mpData : "", compareStr->mpData);
            }
        }
    }

    return compareFlag;
}


// Compare this object to the specified string with option of forcing
// either a case insensitive compare of a case sensitive compare.
int UtlString::compareTo(const char* compareStr, CompareCase type) const
{
    int compareFlag;

    if (type == matchCase)
    {
        // BUG: should use memcmp as compareStr may have null char before end
        compareFlag = strcmp(mpData ? mpData : "",
                             compareStr ? compareStr : "");
    }
    else
    {
       compareFlag = strcasecmp(mpData ? mpData : "",
                                compareStr ? compareStr : "");
    }

    return compareFlag;
}


// Returns TRUE if szStr occurs in self.
// use index(const char * szStr).
UtlBoolean UtlString::contains(const char* searchStr) const
{
    UtlBoolean containFlag = TRUE;

    ssize_t containPosition = index(searchStr);
    if (containPosition == UTLSTRING_NOT_FOUND)
    {
        containFlag = FALSE;
    }

    return containFlag;
}


// Return true if this is a empty (or null) string.
UtlBoolean UtlString::isNull() const
{
    return(mSize == 0);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ INLINE METHODS ============================ */
