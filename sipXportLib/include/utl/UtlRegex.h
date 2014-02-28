//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _UTL_REGEX_H
#define _UTL_REGEX_H

#include <string.h>
#include <pcre.h>
#include "utl/UtlString.h"
#include "utl/UtlContainableAtomic.h"

/**
 * RegEx impelments Perl-compatible regular expressions
 *
 * A simple and small C++ wrapper for PCRE.
 * PCRE (or libprce) is the Perl Compatible Regular Expression library.
 * http://www.pcre.org/
 *
 * Adapted for the sipXportLib from the regex.hpp wrapper:
 *
 * regex.hpp 1.0 Copyright (c) 2003 Peter Petersen (pp@on-time.de)
 * Simple C++ wrapper for PCRE
 *
 *   This source file is freeware. You may use it for any purpose without
 *   restriction except that the copyright notice as the top of this file as
 *   well as this paragraph may not be removed or altered.
 *
 * Original wrapper by Peter Petersen, adapted to sipX by Scott Lawrence
 *
 * The regular expression is compiled in the constructor, and then may be applied
 * to target strings using one of the Search interfaces.  The results are obtained
 * using the Results interfaces.
 *
 * This class is a wrapper around the PCRE package (see the project INSTALL for a
 * pointer to where PCRE can be found).  All the Options variables are identical
 * to those in pcre.h
 *
 * @note
 * Compiling the regular expressions is usually expensive compared to executing
 * the actual search, so if an expression is frequently reused, it is best to
 * compile it only once and then construct the expression to use in the search
 * using the copy constructor.
 *
 *
 * @nosubgrouping
 */
class RegEx : public UtlContainableAtomic
{
 public:

// ================================================================
/** @name    Constructors, Destructor, and Expression Information
 */
///@{

   /// Compile a regular expression to create the matching object.
  RegEx( const char *      regex,       //< the regular expression
         int               options = 0, //< any sum of PCRE options bits
         unsigned long int maxDepth = MAX_RECURSION // see MAX_RECURSION
         );
  /**<
   * If compiling the regular expression fails, an error message string is
   * thrown as an exception.
   * For options documentation, see 'man pcre'
   */

  /// Default maximum for the recursion depth in searches.
  static const unsigned long int MAX_RECURSION;
  /**<
   * The PCRE internal match() function implements some searches by recursion.
   * This value is the default maximumm allowed depth for that recursion.  It can
   * be changed to some other value by passing the maxDepth option argument to the
   * RegEx constructor.  It is set at compile time from the SIPX_MAX_REGEX_RECURSION
   * macro, if that value is defined.
   *
   * If the maximum is exceeded, the match fails.
   *
   * If this or the maxDepth constructor argument are set to zero, then no limit
   * is enforced (use with caution).
   *
   * See the discussions of stack size in the pcre documentation.
   *
   * @note Caution
   * Test your limits carefully - in versions of PCRE prior to 6.5, there is no
   * way to limit recursive matches, so this is implemented as a limit on the
   * total number of calls to 'match' (PCRE_EXTRA_MATCH_LIMIT); this can dramatically
   * shorten the length of the strings that a pattern that has nested parenthesis
   * will match.
   */

  /// Construct from a constant regex to save compilation time.
  RegEx( const RegEx& );
  /**<
   * If you are using the same constant regular expression frequently, you can
   * use this constructor to save the time to compile and study it.  First, declare
   * a private constant copy of your expression - this will be compiled by PCRE
   * just once when it is instantiated:
   * \code
   * static const RegEx FooNumbers("foo([0-9]+)");
   * \endcode
   * Then in your method, construct a copy of it to use when matching strings:
   * \code
   *    RegEx fooNumbers(FooNumbers);
   *    fooNumbers.Search(someString);
   * \endcode
   * Constructing this copy does not require a PCRE call to compile the expression.
   */

  ~RegEx();

  /// Count the number of possible substrings returned by this expression
  int SubStrings(void) const;
  /**<
   *   SubStrings() @returns the number of substrings defined by
   *   the regular expression.
   *
   *   The match of the entire expression is also considered a substring, so the return
   *   value will always be >= 1.
   *
   *   This method is especially useful when the regular expression is loaded
   *   from some external source.  For a hard-coded expression, the return is
   *   a constant, so you really don't need this method.
   */

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
// ================================================================
/** @name Searching
 *
 * The searching methods apply a compiled regular expression to a subject
 * string.  All searching methods return a boolean result indicating whether
 * or not some match was found in the subject.  To get information about
 * the match, use the Results methods.
 */
///@{

  /// Search a string for matches to this regular expression
  bool Search( const char * subject,  ///< the string to be searched for a match
               int len = -1,          ///< the length of the subject string
               int options = 0        ///< sum of any PCRE options flags
               );
  /**<
   *    Apply the regular expression to the subject string.
   *    Optional parameter len can be used to pass the subject's length to
   *    Search(). If not specified (or less than 0), strlen() is used
   *    internally to determine the length. Parameter options can contain
   *    any combination of options; for options documentation, see 'man pcre'
   *    @returns true if a match is found.
   */

  /// Search a string starting at some offset for matches to this regular expression
  bool SearchAt(const char* subject,  ///< the string to be searched for a match
                int offset,           ///< offset to begin search in subject string
                int len = -1,         ///< the length of the subject string
                int options = 0       ///< sum of any PCRE options flags
                );
  /**<
   *    Apply the regular expression to the subject string, starting at the given offset.
   *    If the length is not specified, then strlen(subject) is used.
   *    Parameter options can contain
   *    any combination of options; for options documentation, see 'man pcre'
   *    @returns true if a match is found.
   *    @note
   *    The start of this search is not considered the start of the subject for
   *    the purposes of anchoring.  So if the expresssion is "^xx", then subject
   *    "fooxx" will not match, even if offset is passed as '3'.
   *
   */


  /// Repeat the last search operation, starting immediately after the previous match
  bool SearchAgain( int options = 0        ///< sum of any PCRE options flags
                   );
  /**<
   *    SearchAgain() applies the regular expression to the same
   *    subject last passed to Search or SearchAt, but restarts the search after the last match.
   *    Subsequent calls to SearchAgain() will find all matches in the subject.
   *    @returns true if a further match is found.
   *    Example:
   *    \code
   *       RegEx Pattern("A[0-9]");
   *       const char* value = "xyzA1abcA2def";
   *       for (matched = Pattern.Search(value); matched; matched = Pattern.SearchAgain())
   *       {
   *          printf("%s\n", Pattern.Match());
   *       }
   *    \endcode
   *    Would print "A1\n" and then "A2\n".
   *
   * @note Prefer MatchString over the less efficient Match
   */

///@}
// ================================================================
/** @name     Results
 *
 * The results methods provide information about the matches based on the
 * results of the most recent Searching method call.  It is an error to
 * call any of these methods unless the most recent Searching call returned
 * 'true'.
 *
 * The substring index must be less than the result of RegEx::SubStrings on
 * the regular expression, but may also be zero or -1 as follows:
 * - (-1) returns the last searched subject.
 * - (0) returns the match of the complete regular expression.
 * - (1) returns $1, etc.
 *
 */
///@{

  /// Get the maximum substring value from the most recent search.
  int Matches();
  /**<
   * May only be called after a successful search using one of the searching interfaces,
   * and applies to the results of that call.
   * - any negative return indicates a caller error - the preceeding search call did not match
   * - a return value of 1 indicates that the entire pattern matched, but no substrings
   *   within it matched.
   * - a return value of N > 1 indicates that the full string and N-1 substrings are available
   *
   * @note
   * If the expression has internal optional matches, they may not be matched; for example the
   * expression "(foo|(bar))(bing)" matches subject "foobingo", and Matches would return 4
   * because substring 3 "bing" was matched, but substring 2 would be the null string for
   * that match.
   *
   */


  /// Append a match from the last search operation to a UtlString.
  bool MatchString(UtlString* matched, /**< string to append the match to -
                                        * may be NULL, in which case no string is returned,
                                        * but the return code still indicates whether or not
                                        * this substring was matched.
                                        */
                   int i = 0           /**< which substring to append from the last search
                                        * - Match(-1) returns the last searched subject.
                                        * - Match(0) returns the match of the complete regular expression.
                                        * - Match(i>0) returns $i
                                        */
                   );
  /**<
   * May only be called after a successful search
   * and applies to the results of that call.
   * @returns true if there was an ith match, false if not
   *
   * Example:@code
   * RegEx matchBs("((B)B+)");
   * UtlString getB;
   * UtlString getBs;
   * if (matchB.Search("xxaBBBBcyy"))
   * {
   *   matchB.MatchString(&getBs,0);
   *   matchB.MatchString(&getB,2);
   * }
   * @endcode
   * would set the UtlStrings
   *  - getBs to "BBBB"
   *  - getB  to "B"
   */


  /// Get the position and length of a match in the subject
  bool Match(const int i, ///< input - must be < SubStrings() */
             int& offset, ///< output - offset in last subject of the n'th match
             int& length  ///< output - length in last subject of the n'th match
             );
  /**<
   * May only be called after a successful
   * call to one of the searching methods, and applies to the results of
   * that call.
   *
   * Parameter i must be less than SubStrings().
   * - Match(-1) returns the last searched subject.
   * - Match(0) returns the match of the complete regular expression.
   * - Match(1) returns $1, etc.
   *
   * @returns true if the last search had an n'th match, false if not
   *
   * Example:@code
   * RegEx matchABCs("A+(B+)(C+)");
   * UtlString subject("xAABBBBC");
   * int offset = 1;
   * if (matchABCs.SearchAt(subject, offset))
   * {
   *   int all    = matchABCs.Match(0, allStart, allLength);
   *   int firstB = matchABCs.Match(1, firstB, numBs);
   *   int firstC = matchABCs.Match(2, firstC, numCs);
   * }
   * @endcode
   * would set the values
   *  - allStart = 1, allLength = 2
   *  - firstB = 3, numBs = 4
   *  - firstC = 7, numCs = 1
   *
   * @note
   * The returned start position is relative to the beginning of the subject string,
   * not from any offset value.
   *
   */


  /// Get the position of a match in the subject
  int MatchStart(const int i ///< input - must be < SubStrings() */
                 );
  /**<
   * May only be called after a successful call to one of the searching
   * methods, and applies to the results of that call.
   *
   * Parameter i must be less than SubStrings().
   * - Match(-1) returns the last searched subject.
   * - Match(0) returns the match of the complete regular expression.
   * - Match(1) returns $1, etc.
   *
   * This is useful when searching at an offset in a string to check whether or not
   * the match was at the offset or somewhere later in the string.
   *
   * Example:@code
   * RegEx matchABCs("A+(B+)(C+)");
   * UtlString subject("xAABBBBC");
   * int offset = 1;
   * bool result = (   (matchABCs.SearchAt(subject, offset))
   *                && (matchABCs.MatchStart(0) == offset));
   * @endcode
   * Note that this is not the same as haveing written the regular expression so
   * that it is anchored: "^A+(B+)(C+)" because the anchor always refers to the
   * actual start of the string (in the example, before the 'x'), even when used
   * with an offset.  So the 'result' variable in the example would be true.
   */

  /// Append string preceeding the most recently matched value to a UtlString.

  bool BeforeMatchString(UtlString* before /**< string to append to -
                                            * may be NULL, in which case no string is returned,
                                            * but the return code still indicates whether or not
                                            * there was some string preceeding the last match.
                                            */
                         );
  /**<
   * May only be called after a successful search and applies to
   * the results of that call. This is equivalent to the Perl $` variable.
   *
   * @returns true if there was a string before the match, false if not
   * Example:@code
   * RegEx matchB("B");
   * UtlString getBefore;
   * if (matchB.Search("xxaBcyy"))
   * {
   *   matchB.BeforeMatchString(&getBefore);
   * }
   * @endcode
   *
   * would set the UtlString getBefore to "xxa".
   */

  /// Append string following the most recently matched value to a UtlString.
  bool AfterMatchString(UtlString* before /**< string to append to -
                                            * may be NULL, in which case no string is returned,
                                            * but the return code still indicates whether or not
                                            * there was some string following the last match.
                                            */
                         );
  /**<
   * May only be called after a successful search and applies to
   * the results of that call. This is equivalent to the Perl $' variable.
   *
   * @returns true if there was a string following the match, false if not
   * Example:@code
   * RegEx matchB("B");
   * UtlString getAfter;
   * if (matchB.Search("xxaBcyy"))
   * {
   *   matchB.AfterMatchString(&getAfter);
   * }
   * @endcode
   *
   * would set the UtlString getAfter to "cyy".
   */

  /// Get the offset of the first character past the matched value
  int AfterMatch(int i ///< the substring specifier
                 );
  /**<
   * May only be called after a successful search and applies to
   * the results of that call.
   *
   * Example:@code
   * RegEx matchBseq("A+(B+)C+");
   * if (matchBseq.Search("xxAABBBCCCyy"))
   * {
   *   int afterB = matchBseq.AfterMatch(1);
   *   int afterC = matchBseq.AfterMatch(0);
   * }
   * @endcode
   *
   * would set
   * - afterB = 7
   * - afterC = 10
   */

  /// Get a string matched by a previous search
  const char * Match(int i = 0 /**< must be < SubStrings() */ );
  /**<
   * @note
   * This does more memory allocation and data copying than any of the other results methods;
   * use one of the others when possible.
   *
   *
   * May only be called after a successful search, and applies to the results of
   * that call. Parameter i must be less than
   * SubStrings().
   * - Match(-1) returns the last searched subject.
   * - Match(0) returns the match of the complete regular expression.
   * - Match(1) returns $1, etc.
   * @returns a pointer to the ith matched substring.
   */

  /// Turn a string into a regexp string that matches exactly the given string.
  //  Much like the Perl quotemeta() function.
  static void Quotemeta(const char* literal,
                        ///< input, "literal" string
                        UtlString& regex
                        ///< output, "regexp" string
     );

///@}

 private:
  /*
   * Use the copy constructor above instead of the = operator.
   */
  RegEx& operator=(const char *);

  void ClearMatchList(void);

  pcre * re;
  size_t re_size;
  pcre_extra * pe;
  bool allocated_study;
  size_t  study_size;
  int substrcount;         // maximum substrings in pattern
  const char * subjectStr; // original subject
  int subjectLen;          // original length
  int lastStart;           // offset of start for most recent Search or SearchAgain
  int lastMatches;         // pcre_exec return for most recent Search or SearchAgain
  int * ovector;           // results from (and workspace for) pcre_exec
  const char * * matchlist;// string cache for Match
};

#endif // _UTL_REGEX_H
