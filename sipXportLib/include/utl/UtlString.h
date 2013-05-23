//
// Copyright (C) 2007, 2010 Avaya, Inc., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _UtlString_h_
#define _UtlString_h_

// SYSTEM INCLUDES
#include <string>
#include "os/OsDefs.h"

// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlContainable.h"

// DEFINES
#define DEFAULT_UTLSTRING_CAPACITY 100 ///< initial capacity unless overridden by the constructor

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * UtlString is a resizable string which is also containable in
 * any UtlContainer.  It may include null characters.
 *
 * A UtlString will grow as needed to hold any value stored in it; in
 * the worst case every operation that enlarges a UtlString's contents
 * may reallocate the stored data.  Thus, building a large string in
 * small increments may require order(N^2) time.  For efficiency,
 * pre-allocate the required amount of space using an argument to the
 * constructor or using the capacity() method, or monitor the
 * UtlString and explicitly reallocate its space as needed by, e.g.,
 * doubling its capacity (which requires order(N) time to build a
 * large string).
 *
 * @nosubgrouping
 */
class UtlString : public UtlContainable
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    static const char* ssNull;

    static const ssize_t UTLSTRING_NOT_FOUND; ///< Returned from a search that failed.

    static const ssize_t UTLSTRING_TO_END; /**< When specifying the length of a substring,
                                           *   indicates that the substring should extend
                                           *   to the end of the string. */

    static const size_t  MAX_NUMBER_STRING_SIZE; ///< maximum size text output for appendNumber

    /**
     * Flags to the various strip methods
     */
    typedef enum StripType
       {
          leading=1,  /**< only strip from the beginning of the string */
          trailing,   /**< only strip from the end of the string */
          both        /**< strip from both ends of the string */
       } StripType;

    /**
     * Case sensitivity control constants
     */
    typedef enum CompareCase
       {
          matchCase,    /**< Case sensitive comparison */
          ignoreCase    /**< Case insensitive comparison */
       } CompareCase;

// ================================================================
/** @name                  Constructors
 */
///@{

    /// Default Constructor
    UtlString();

    /// Constructor accepting a null terminated source string.
    // szSource may be NULL.
    UtlString(const char* szSource /**< initial string value */ );
    /**<
     * The source string is copied to this object.
     */

    /// Constructor accepting a source string with an explicit length.
    UtlString(const char* szSource, size_t length);
    /**<
     * Any zero byte in the source string is ignored; all length chars of the source
     * string are copied to this object.
     * szSource can be NULL if length == 0.
     */

    /// Copy the source UtlString.
    UtlString(const UtlString& source);

    /// Copy an initial substring.
    UtlString(const UtlString& source, size_t length);
    /**<
     * Constructor accepting a reference to another string and a length.
     * Up to length characters are copied from the source string into
     * this object.  If length is greater than the length of source
     * this becomes a copy of source with the same data and length.
     */

///@}
// ================================================================
/** @name                Assignment Operators
 *
 */
///@{

    /// Replace the contents of this string with a null terminated string.
    UtlString& operator=(const char* szStr);

    /// Replace the contents of this string with contents of a UtlString
    UtlString& operator=(const UtlString& str);

///@}
// ================================================================
/** @name              Accessors
 *
 */
///@{

    /// The current length of the string value
    size_t length() const;

    /// Return a read-only pointer to the stored string value..
    const char* data() const;
    /**<
     * This pointer should not be stored, since operations on the UtlString
     * may free it.
     *
     * The returned pointer is never null, even if the UtlString is
     * null or newly-allocated.
     *
     * The stored value will have a null byte at the end, but may also have
     * null bytes at other positions within the value.
     */

    /// Cast to a const char* - identical to the data() method.
    operator const char*() const;
    /**<
     * Cast this object to a const char* pointer.  This is exactly the
     * same as calling data().
     */

    /// Return the current capacity allocated for this string.
    size_t capacity() const;

///@}
// ================================================================
/** @name              Substring Operators
 *
 * Extract a portion of the string value.
 */
///@{

    /// Allows references of the form stringVar(start,length) - also see append
    UtlString operator() (size_t start, ///< Starting character position
                                        //   (zero based)
                          size_t len    /**< Number of characters to copy or
                                         *   UTLSTRING_TO_END for
                                         *   "the rest of the string" */
                          ) const;
    /**<
     * An empty string is returned if the start and/or len is invalid.
     *
     * @note
     * This method constructs a temporary UtlString object.  It will usually be more efficient
     * to use the append method taking a UtlString, postion, and length.
     *
     */

    /// Get the character at position N.
    char operator()(size_t N) const;
    /**
     * @returns the nth character (zero based)
     * @code
     * UtlString aString("abc");
     * char secondChar = aString(1);
     * @endcode
     * Sets secondChar to 'b'
     */

///@}
// ================================================================
/** @name              Comparison Operations
 *
 */
///@{


    /// Return true if this is an empty string.
    UtlBoolean isNull() const;

    /// Compare this string to a null terminated string (case sensitive).
    virtual int compareTo(const char *) const;
    /**<
     * @returns 0 if equal, < 0 if less then and >0 if greater.
     */

    /// Compare this string to a null terminated string, with case sensitivity control
    int compareTo(const char *, CompareCase type) const;

    /// Compare this object to another string, with case sensitivity control
    int compareTo(UtlString const *, CompareCase type) const;
    /**<
     * @returns 0 if equal, < 0 if less then and >0 if greater.
     */

    /// Check for a null terminated value in this string.
    UtlBoolean contains(const char *) const;
    /**<
     * This is equivalent to ( index(str) != UTL_NOT_FOUND )
     *
     * @return true if this string contains the specified string.
     */

    friend UtlBoolean operator!=(const char *, const UtlString& );

    friend UtlBoolean operator!=(const char , const UtlString& );

    friend UtlBoolean operator==(const char , const UtlString& );

    friend UtlBoolean operator==(const char *, const UtlString& );

    UtlBoolean operator==(const char *) const;

    UtlBoolean operator!=(const char *) const;

    UtlBoolean operator==(const UtlString&) const;

    UtlBoolean operator!=(const UtlString&) const;

    

///@}
// ================================================================
/** @name              Search Operations
 *
 * Also see the regular expression search operations in the RegEx class.
 */
///@{

    /// Find the first instance of a single character.
    ssize_t index(char c) const;
    /**<
     * @return the offset of the first c or UTL_NOT_FOUND
     */

    /// Find the first instance of a single character starting at a specified offset.
    ssize_t index(char c, size_t offset) const;
    /**<
     * @return the offset from the beginning of the string of the
     * first c at or after offset, or UTL_NOT_FOUND
     */

    /// Find the first instance of a single character.
    ssize_t first(char c) const;
    /**<
     * @return the offset of the first c or UTL_NOT_FOUND
     *
     * (this is the same as index(c) )
     */

    /// Find the first instance of a null terminated string.
    ssize_t index(const char* ) const;
    /**<
     * @return the offset of the first matching string or UTL_NOT_FOUND
     */

    /// Find the first instance of a specified length string.
    ssize_t index(const char* , size_t ) const;
    /**<
     * The search target value may contain null characters.
     *
     * @return the offset of the first matching string or UTL_NOT_FOUND
     */

    /// Find the first instance of a specified length string, with case control
    ssize_t index(const char* , size_t , CompareCase ) const;
    /**<
     * The search target value may contain null characters.
     *
     * @return the offset of the first matching string or UTL_NOT_FOUND
     */

    /// Find the first match for the specified string (which may contain null characters)
    ssize_t index(const UtlString& ) const;
    /**<
     * The search target value may contain null characters.
     *
     * @return the offset of the first matching string or UTL_NOT_FOUND
     */

    /// Find a match for the specified string starting at offset
    ssize_t index(const UtlString& match, ///< string value to search for
                 size_t offset           ///< offset in target to start search
                 ) const;
    /**<
     * The search target value (match) may contain null characters.
     *
     * @return the offset from the beginning of the string of the match value after offset or UTL_NOT_FOUND
     */

    /// Find a match for the specified string starting at offset, with case sensitivity control
    ssize_t index(const UtlString& , size_t , CompareCase ) const;
    /**<
     * The search target value (match) may contain null characters.
     *
     * @return the offset from the beginning of the string of the match value after offset or UTL_NOT_FOUND
     */

    /// Find the first instance of a null terminated string
    ssize_t first(const char* ) const;
    /**<
     * @return the offset of the string or UTL_NOT_FOUND
     */

    /// Find the last instance of a character.
    ssize_t last(char s) const;
    /**<
     * @return the offset of the string or UTL_NOT_FOUND
     */

    UtlBoolean findToken(const char* token,
                         const char* delimiter,
                         const char* suffix = NULL,
                         bool regex = false) const;
    /**<
     * Search for 'token' in the string.
     * The string is a sequence of substrings, separated by 'delimiter's,
     * possibly surrounded by whitespace.  See if 'token' equals any
     * of these substrings.
     * If 'suffix' is not NULL, a substring may be followed by 'suffix'
     * and a string (not containing 'delimiter') before the next
     * 'delimiter'.
     * If 'regex' is false (the default), 'token' is interpreted as a string
     * which must be literally present in the UtlString.
     * If 'regex' is true, 'token' is interpreted as a regular expression
     * (the argument to Regex::Regex()), and the UtlString must contain
     * a delimited substring which is matched by 'token'.
     * Matching is done case-insensitively.
     *
     * @return True if 'token' matches any of the substrings.
     */

///@}
// ================================================================
/** @name                  Append Operations
 *
 * Append operations add to the end of the current value of the string.
 */
///@{

    /// Append a null terminated string to the end of the lvalue.
    UtlString& operator+=(const char *);

    /// Append a null terminated string to the end of this string.
    // szStr may be NULL.
    UtlString& append(const char* szStr);

    /// Append a UtlString to this string.
    UtlString& operator+=(const UtlString& s);

    /// Append a UtlString to this string.
    UtlString& append(const UtlString& str ///< source string
                      );

    /// Append a substring of a UtlString to this string.
    UtlString& append(const UtlString& str, ///< source string
                      size_t position,      ///< offset into source to start copy
                      size_t length ///< length of substring to copy
                      );
    /**<
     * Note the difference between these two copies:
     * @code
     * UtlString source("0123456789");
     *
     * UtlString destA;
     * destA = source(1,3);
     *
     * UtlString destB;
     * destB.append(source, 6, 2);
     * @endcode
     * The assignment to destA actually constructs a temporary UtlString on the stack
     * as the output of the source(1,3), invokes the copy constructor to copy
     * its contents into destA, and then destructs it.
     *
     * The assignment to destB copies the string contents directly from source to destB.
     */

    /// Append upto N bytes of the designated string to the end of this string.
    //  szStr can be NULL if N == 0.
    UtlString& append(const char* szStr, size_t N);

    /// Append a single character to the end of this string.
    UtlString& operator+=(const char c);

    /// Append a single character to the end of this string.
    UtlString& append(const char c);

    /// Append a string representation of the value to the string
    UtlString& appendNumber(Int64 value, const char* format = "%"FORMAT_INTLL"d");
    UtlString& appendNumber(int value, const char* format = "%d");
    UtlString& appendNumber(size_t value, const char* format = "%zu");
#if __WORDSIZE == 64
    UtlString& appendNumber(ssize_t value, const char* format = "%zd");
#else
    UtlString& appendNumber(long value, const char* format = "%ld");
#endif
    /**< MAX_NUMBER_STRING_SIZE is the maximum size text representation of the value allowed
     *   If this size is exceeded an error string is appended in place of the number
     */

///@}
// ================================================================
/** @name              Insertion Operators
 *
 * These methods insert values into the string; the capacity is increased
 * as needed to hold the additional value.
 *
 * Nothing is overwritten - Any existing value at the insertion point is
 * shifted to make room.
 */
///@{

    /// Insert a UtlString
    UtlString& insert(size_t position,     ///< position to insert the src string.
                      const UtlString& src ///< value to be inserted
                      );
    /**<
     * If an invalid position is specified, nothing is performed.
     */

    /// Insert a single character
    UtlString& insert(size_t position,   ///< position to insert newChar.
                      const char newChar ///< character to insert
                      );
    /**<
     * If an invalid position is specified, nothing is performed.
     */

    /// Insert a null terminated string
        UtlString& insert(size_t position,   ///< postion to insert src C string
                          const char* src    ///< null terminated string to insert
                          );
     /**<
      * If an invalid position is specified, nothing is performed.
      */

    /// Prepend a null terminated string at the beginning of this string.
    UtlString& prepend(const char* szStr);
    /**< Equivalent to insert(0, szStr); */

    /// Insert arbitrary length value
    UtlString& insert(size_t position,   ///< position to insert src value
                      const char* src,   ///< value to insert (may contain null bytes)
                      size_t sourceLenth ///< number of bytes to insert
                      );
    /**<
     * Insert the value at character position.
     * nn
     * If an invalid position is specified, nothing is performed.
     */

///@}
// ================================================================
/** @name              Replacement Operations
 *
 */
///@{

    /// Replace a single character at the designated position.
    void replaceAt(size_t pos,  ///< offset into data to replace - must be < the length
                   char newChar ///< character to be put into that offset
       );


    /// Replace all instances of character src with character tgt.
    UtlString& replace(const char src, const char tgt);

    /// Replace N characters from a char*.
    UtlString& replace(size_t pos,    ///< position in string to begin replacing
                       size_t N,      ///< number of characters to replace
                       const char* replaceStr ///< replacement value
                       );
    /**<
     * Replace N characters starting at the designated position with the
     * designated replacement string.  Invalid position or length results
     * in no changes.
     */

    /// Replace N characters from a char*, with specified length string.
    UtlString& replace(size_t pos,             ///< starting postion of the replacement
                       size_t N,               ///< number of characters to replace
                       const char* replaceStr, ///< target replacement string
                       size_t L                ///< maximum number of characters of the replacement string to use.
                       );
    /**<
     * Replace N characters starting at the designated position with a subset
     * of the designated replacement string.  Invalid position or length results
     * in no changes.
     */

    /// Replace N characters from another UtlString.
    UtlString& replace(size_t pos,    ///< position in string to begin replacing
                       size_t N,      ///< number of characters to replace
                       const UtlString& replaceStr ///< replacement value
                       );
    /**<
     * Replace N characters starting at the designated position with the
     * designated replacement string.  Invalid position or length results
     * in no changes.
     */

///@}
// ================================================================
/** @name              Concatenation Operations
 *
 */
///@{

    /// Concatenate two UtlStrings as: s1 + s2
    friend UtlString operator+(const UtlString& s1, const UtlString& s2);

    /// Concatenate a UtlString with a constant string as: s + c
    friend UtlString operator+(const UtlString& s,  const char* c);

    /// Concatenate a constant string with a UtlString as: c + s
    friend UtlString operator+(const char* c, const UtlString& s);

///@}
// ================================================================
/** @name              Remove Operations
 *
 * Also see Stripping and Trimming, below
 */
///@{

    /// Remove all characters after the specified position.
    UtlString& remove(size_t pos);
    /**<
     * Nothing is performed if the position is invalid.
     */

    /// Remove N characters from this string starting at designated position.
    UtlString& remove(size_t pos, size_t N);
    /**<
     * Invalid position or length results in no changes.
     */

///@}
// ================================================================
/** @name        Stripping and Trimming Operations
 *
 */
///@{

    /// Removes whitespace (space, tab, Cr, Lf) from the end of the string.
    UtlString strip();

    /// Removes whitespace (space, tab, Cr, Lf) from either or both ends of the string.
    UtlString strip(StripType whichEnd);
    /**<
     * Removes whitespace (space, tab, Cr, Lf) from the beginning of the string,
     * from the end of the string, or from both the beginning and end of the
     * string.
     */

    /// Removes all instances of the specified character from either or both ends of the string.
    UtlString strip(StripType whichEnd, char charToStrip);
    /**<
     * Remove the designated character from the beginning of the string,
     * from the end of the string, or from both the beginning and end of
     * the string.
     */

///@}
// ================================================================
/** @name              Case Conversion Operations
 *
 */
///@{

    /**
     * Convert the string to all lower case characters (e.g. AbC1 -> abc1)
     */
    void toLower();

    /**
     * Convert the string to all upper case characters (e.g. AbC1 -> ABC1)
     */
    void toUpper();

///@}
// ================================================================
/** @name              Capacity Management
 *
 */
///@{

    /// Set the minimum capacity a string can hold without reallocation.
    size_t capacity(size_t N);
    /**<
     * Set the string's storage capacity to at least N.
     * This does not modify the value of the string, but rather
     * adjusts the dynamic memory allocated for this string.
     *
     * This never reduces the capacity - any size less then the
     * current capacity is a no-op.
     *
     * @return The new capacity of the string.  In case of success, it
     * may be greater than the requested capacity.  In case of failure,
     * it may be less than the requested capacity.
     * @code
     * UtlString buffer;
     * if (buffer.capacity(requiredSize) >= requiredSize)
     * {
     *    // setting capacity succeeded, proceed with filling buffer
     * }
     * else
     * {
     *    // setting capacity failed
     * }
     * @endcode
     */

    /// Set a new size for the string.
    void resize(size_t );
    /**<
     * Resize the string to the specified size.  If the requested size is less
     * then the the current size (string length), the string will be truncated.
     * If larger, the string will be padded with nulls.
     */

    /// Set a new length for the content
    void setLength(size_t newLength);
    /**<
     * This resets the value that would be returned by UtlString::length and
     * adds a zero byte after that length.
     * The newLength value MUST be <= (UtlString::capacity - 1).
     *
     * This allows external code to directly manipulate the contents of
     * buffer and adjust the UtlString data accordingly.  For example, the
     * following sets the 'buffer' to contain a 'sizeNeeded' number of bytes
     * of all one bits:
     * @code
     *   UtlString buffer;
     *   if (buffer.capacity(sizeNeeded+1) >= sizeNeeded+1)
     *   {
     *      memset(buffer.data(), 0xff, sizeNeeded);
     *      buffer.setLength(sizeNeeded);
     *   }
     * @endcode
     */

///@}
// ================================================================
/** @name           Container Support Operations
 *
 */
///@{

    /// Calculate a hash over the string contents.
    virtual unsigned hash() const;
    /**<
     * If the equals operator returns true for another object, then both
     * objects must return the same hashcode.
     */

    /// Determine whether or not the values in a containable are comparable.
    virtual UtlContainableType getContainableType() const;
    /**<
     * This returns a unique type for UtlString
     */

    static const UtlContainableType TYPE;    ///< Class type used for runtime checking

    /// Compare to any other UtlContainable
    virtual int compareTo(UtlContainable const *other) const;
    /**<
     * Compare this object to another containable object.
     * If the UtlContainableType of the other object is not the UtlString type,
     * this will return unequal.
     *
     * @returns 0 if equal, < 0 if less than, and > 0 if greater.
     */

    virtual UtlBoolean isEqual(UtlContainable const *) const;
    /**<
     * Test this object to another like-object for equality.  This method
     * returns false if unlike-objects are specified.
     */

///@}
// ================================================================
/** @name                   Destructors
 *
 */
///@{

    /// Destructor
    virtual ~UtlString();

///@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   UtlString& appendFormattedNumber(int formatResult, char* conversionString, const char* format);


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    char*  mpData;      //: The value of UtlString.
    size_t mSize;       //: The number of bytes of data used.
    size_t mCapacity;   //: The allocated size of data.
    char   mBuiltIn[DEFAULT_UTLSTRING_CAPACITY];

public:
    //
    // STL string compatibility layer
    //
    UtlString(const std::string& str);
    UtlString operator=(const std::string& str);
    std::string str() const;
};

/* ============================ INLINE METHODS ============================ */


inline UtlString::UtlString(const std::string& str)
{
    mpData = mBuiltIn;
    mBuiltIn[0] = '\000';
    mSize = 0;
    mCapacity = DEFAULT_UTLSTRING_CAPACITY;

    operator=(str);
}

inline UtlString UtlString::operator=(const std::string& str)
{
    return UtlString::operator=(str.c_str());
}

inline std::string UtlString::str() const
{
  if (this->isNull())
    return "";
  std::string val = this->data();
  return val;
}

#endif    // _UtlString_h_
