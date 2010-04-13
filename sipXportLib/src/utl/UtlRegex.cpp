//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
// Derived from regex.hpp, the original copyright and license notice from regex.cpp:
// ----------------
// regex.hpp 1.0 Copyright (c) 2003 Peter Petersen (pp@on-time.de)
// Simple C++ wrapper for PCRE
//
// This source file is freeware. You may use it for any purpose without
// restriction except that the copyright notice as the top of this file as
// well as this paragraph may not be removed or altered.
// ----------------
//

#include "assert.h"
#include <ctype.h>
#include <string.h>
#include "utl/UtlRegex.h"

#ifndef    SIPX_MAX_REGEX_RECURSION
#   define SIPX_MAX_REGEX_RECURSION 800
#endif

// STATIC VARIABLE INITIALIZATIONS
const UtlContainableType RegEx::TYPE = "UtlRegex";

const unsigned long int RegEx::MAX_RECURSION = SIPX_MAX_REGEX_RECURSION;

// Some versions do not separately define a limit for stack recursion;
// for those, we must just limit the number of matches.
#ifndef   PCRE_EXTRA_MATCH_LIMIT_RECURSION
#  define PCRE_EXTRA_MATCH_LIMIT_RECURSION PCRE_EXTRA_MATCH_LIMIT
#  define match_limit_recursion match_limit
#endif

/////////////////////////////////
RegEx::RegEx(const char * regex, int options, unsigned long int maxDepth)
{
   const char*  pcre_error;
   int          erroffset;

   // compile and study the expression
   re = pcre_compile(regex, options, &pcre_error, &erroffset, NULL);
   if (re == NULL)
   {
      UtlString errorMsg("Regular Expression compile error: ");
      errorMsg.append(pcre_error);
      errorMsg.append(" at offset ");
      char offsetStr[10];
      sprintf(offsetStr, "%9d", erroffset);
      errorMsg.append(offsetStr);
      errorMsg.append(" in expression '");
      errorMsg.append(regex);
      errorMsg.append("'");

      throw errorMsg.data();
      assert(FALSE); // regex failed to compile
   }
   pe = pcre_study(re, 0, &pcre_error);
   if ( pcre_error == NULL )
   {
      // save the compilation block sizes for the copy constructor.
      pcre_fullinfo(re, pe, PCRE_INFO_SIZE, &re_size);
      pcre_fullinfo(re, pe, PCRE_INFO_STUDYSIZE, &study_size);
      allocated_study = false;
   }
   else
   {
      re_size = 0;
      study_size = 0;
   }

   if (!pe)
   {
      // pcre_study didn't return any study data,
      // but we need the pcre_extra block anyway for the recursion limit,
      // so get one
      pe = (pcre_extra*)pcre_malloc(sizeof(pcre_extra));
      memset(pe, 0, sizeof(pcre_extra));
   }
   // set the maximum recursion depth option in the pcre_extra (pe) block
   pe->flags |= PCRE_EXTRA_MATCH_LIMIT_RECURSION;
   pe->match_limit_recursion = maxDepth;

   // allocate space for match results based on how many substrings
   // there are in the expression (+1 for the entire match)
   pcre_fullinfo(re, pe, PCRE_INFO_CAPTURECOUNT, &substrcount);
   substrcount++;
   ovector = new int[3*substrcount];
   matchlist = NULL;
};

/////////////////////////////////
RegEx::RegEx(const RegEx& regex)
{
   const char * error = __FILE__ ": unknown error in RegEx(RegEx)";

   // allocate memory for the compiled regular expression information
   re = (pcre*)pcre_malloc(regex.re_size);
   if (re)
   {
      // copy the compiled regular expression information
      memcpy(re, regex.re, regex.re_size);
      re_size = regex.re_size;

      pe = NULL;
      study_size = 0;
      allocated_study = false;
      if (regex.pe) // should always be true, because constructor allocates it
      {
         // allocate memory for the extra study information and recursion limit
         pe = (pcre_extra*)pcre_malloc(sizeof(pcre_extra));
         if (pe)
         {
            // copy the extra information
            memcpy(pe, regex.pe, sizeof(pcre_extra)) ;

            // copy any study information
            if (regex.study_size > 0)
            {
               void* copied_study_data = pcre_malloc(regex.study_size);

               if (copied_study_data)
               {
                  pe->study_data = copied_study_data;
                  memcpy(pe->study_data, regex.pe->study_data, regex.study_size) ;
                  study_size = regex.study_size;
                  allocated_study = true;
               }
            }
         }
      }
      else
      {
         // no extra or study data to copy
         // this should not happen because we always want the recursion limit
      }
      substrcount = regex.substrcount;
      ovector = new int[3*substrcount];
      matchlist = NULL;
   }
   else
   {
      throw error;
   }
};

/////////////////////////////////
RegEx::~RegEx()
{
  ClearMatchList();
  if (ovector != NULL)
  {
     delete [] ovector;
  }
  if (pe)
  {
     if (allocated_study && study_size)
     {
        pcre_free(pe->study_data);
     }
     pcre_free(pe);
  }
  pcre_free(re);
}

// Get the ContainableType for a UtlContainable derived class.
UtlContainableType RegEx::getContainableType() const
{
    return RegEx::TYPE;
}

/////////////////////////////////
int RegEx::SubStrings(void) const
{
  return substrcount;
}

/////////////////////////////////
bool RegEx::Search(const char * subject, int len, int options)
{
  ClearMatchList();

  subjectStr  = subject;
  lastStart   = 0;
  subjectLen  = (len >= 0) ? len : (int)strlen(subject);
  lastMatches = pcre_exec(re, pe, subjectStr, subjectLen, 0, options, ovector, 3*substrcount);

  return lastMatches > 0;
}

/////////////////////////////////
bool RegEx::SearchAt(const char* subject,  ///< the string to be searched for a match
                     int offset,           ///< offset to begin search in subject string
                     int len,              ///< offset to begin search in subject string
                     int options           ///< sum of any PCRE options flags
                     )
{
   /*
    * Search the subject string for matches to this regular expression
    *    @returns true if a match is found.
    */
   ClearMatchList();

   subjectStr  = subject;
   lastStart   = 0;
   subjectLen  = (len >= 0) ? len : (int)strlen(subject);
   lastMatches = pcre_exec(re, pe, subject, subjectLen, offset, options, ovector, 3*substrcount);

   return lastMatches > 0;
}

/////////////////////////////////
bool RegEx::SearchAgain(int options)
{
  ClearMatchList();
  bool matched;
  lastStart = ovector[1];
  if (lastStart < subjectLen)
  {
     lastMatches = pcre_exec(re, pe, subjectStr, subjectLen, lastStart, options,
                             ovector, 3*substrcount);
     matched = lastMatches > 0;
  }
  else
  {
     // The last search matched the entire subject string
     // If the pattern allows a null string to match, then another call to pcre_exec
     // would return that match, so don't do that.
     // Instead, return no match to prevent an infinite loop.
     matched = false;
  }
  return matched;
}

/////////////////////////////////
int RegEx::Matches()
{
   /*
    * Get the maximum number of substrings matched by a previous Search or SearchAgain call.
    * May only be called after a successful
    * call to Search() or SearchAgain() and applies to the results of
    * that call.
    * - any negative return indicates a caller error - the preceeding search call did not match
    * - a return value of 1 indicates that the entire pattern matched, but no substrings
    *   within it matched.
    * - a return value of N > 1 indicates that the full string and N-1 substrings are available
    *
    * @note
    * If the expression has internal optional matches, they may not be matched; for example the
    * expression "foo(bar)?(bing)" matches subject "foobingo", and Matches would return 2
    * because substring 2 "bing" was matched, but substring 1 would be the null string for
    * that match.
    * @endnote
    */
   return lastMatches;
}


////////////////////////////////////////////////////////////////
bool RegEx::BeforeMatchString(UtlString* before)
{
   bool hadBefore = false; // assume no match

   if (lastMatches) // any matches in the last search?
   {
      int startOffset = ovector[0]; // start of all of most recent match
      if (lastStart < startOffset) // anything before the last match?
      {
         int length = startOffset - lastStart;
         if (NULL!=before)
         {
            before->append(subjectStr+lastStart, length);
         }
         hadBefore = true;
      }
   }
   return hadBefore;
}

////////////////
int RegEx::AfterMatch(int i ///< the substring specifier
                      )
{
   /*
    * Get the offset of the first character past the matched value
    *
    * May only be called after a successful call to Search() or SearchAgain() and applies to
    * the results of that call.
    */
   return (  i < lastMatches
           ? ovector[(2*i)+1]
           : -1
           );
}

bool RegEx::AfterMatchString(UtlString* after)
{
   bool hadAfter = false; // assume no match

   if (lastMatches) // any matches in the last search?
   {
      int endOffset = ovector[1]; // end of all of most recent match
      if (endOffset < subjectLen) // anything after the last match?
      {
         int length = subjectLen - endOffset;
         if (NULL!=after)
         {
            after->append(subjectStr+endOffset, length);
         }
         hadAfter = true;
      }
   }
   return hadAfter;
}

bool RegEx::MatchString(UtlString* matched, int i)
{
   bool hadMatch = false; // assume no match

   if (i < lastMatches) // enough matches in the last search?
   {
      if (-1 == i) // return entire subject string
      {
         if (NULL!=matched)
         {
            matched->append(subjectStr, subjectLen);
         }
         hadMatch = true;
      }
      else
      {
         int startOffset = ovector[i*2];
         if (0 <= startOffset) // did ith string match?
         {
            int length = ovector[(i*2)+1] - startOffset;
            if (0<length)
            {
               if (NULL!=matched)
               {
                  matched->append(subjectStr+startOffset, length);
               }
            }
            else
            {
               // the matched substring was a null string - can happen
            }
            hadMatch = true;
         }
      }
   }
   return hadMatch;
}

/////////////////////////////////
bool RegEx::Match(const int i, ///< input - must be < SubStrings() */
                  int& offset, ///< output - offset in last subject of the n'th match
                  int& length  ///< output - length in last subject of the n'th match
                  )
{
   bool i_matched;

/*
 * Get a string matched by a previous Search or SearchAgain call.
 * May only be called after a successful
 * call to Search() or SearchAgain() and applies to the results of
 * that call. Parameter i must be less than
 * SubStrings().
 * - Match(-1) returns the last searched subject.
 * - Match(0) returns the match of the complete regular expression.
 * - Match(1) returns $1, etc.
 * @returns true if the last search had an n'th match, false if not
 */
   assert(i < lastMatches);
   if (i <= lastMatches)
   {
      offset = ovector[(2*i)];
      length = ovector[(2*i)+1] - ovector[(2*i)];

      i_matched = offset != -1;
   }
   else
   {
      i_matched = false;
   }

   return i_matched;
}

/// Get the position of a match in the subject
int RegEx::MatchStart(const int i ///< input - must be < SubStrings() */
               )
{
   assert(i < lastMatches);
   return (  (i <= lastMatches)
           ? ovector[(2*i)]
           : -1
           );
}

/////////////////////////////////
const char * RegEx::Match(int i)
{
   // use of this routine is discouraged for efficiency reasons - use MatchString instead
   if (i >= 0)
   {
      if (matchlist == NULL)
      {
         pcre_get_substring_list(subjectStr, ovector, substrcount, &matchlist);
      }
      return matchlist[i];
   }
   else
   {
      return subjectStr;
   }
}

// Turn a string into a regexp string that matches exactly the given string.
// Implemented directly, rather than by invoking the PCRE Quotemeta(), as
// the operation is simple enough to code by hand, and the relevant add-on
// to PCRE includes parts of the C++ STD.
void RegEx::Quotemeta(const char* literal,
                      UtlString& regex)
{
   // Clear 'regex'.
   regex.remove(0);
   int l = strlen(literal);
   // Set 'regex' large enough to hold the longest possible result.
   regex.capacity(l * 2);
   // Copy the characters of 'literal', prepending backslashes to all
   // non-alphanumeric characters.  (This is slightly different from Perl's
   // quotemeta(), as we backslash "_", but Perl does not.)
   // Some unpleasant casts are needed, because isalnum() wants 'unsigned char'
   // arguments.
   const unsigned char* lp =
      reinterpret_cast <const unsigned char*> (literal);
   unsigned char* rp =
      const_cast <unsigned char*>
      (reinterpret_cast <const unsigned char*> (regex.data()));
   for (int i = 0; i < l; i++)
   {
      // Beware that isalnum()'s arguments are 'unsigned char' values.
      unsigned char c = *lp++;
      if (!isalnum(c))
      {
         *rp++ = '\\';
      }
      *rp++ = c;
   }
   // Set the length of 'regex' appropriately.
   regex.setLength(rp -
                   reinterpret_cast <const unsigned char*> (regex.data()));
}


// PRIVATE METHODS

void RegEx::ClearMatchList(void)
{
   if (matchlist)
   {
      pcre_free_substring_list(matchlist);
      matchlist = NULL;
   }
}

// Below is a little demo/test program using class RegEx

#ifdef REGEX_DEMO

#include <stdio.h>
#include "regex.hpp"

///////////////////////////////////////
int main(int argc, char * argv[])
{
  if (argc != 2)
    {
      fprintf(stderr, "Usage: grep pattern\n\n"
              "Reads stdin, searches 'pattern', writes to stdout\n");
      return 2;
    }
  try
    {
      RegEx Pattern(argv[1]);
      int count = 0;
      char buffer[1024];

      while (fgets(buffer, sizeof(buffer), stdin))
        if (Pattern.Search(buffer))
          fputs(buffer, stdout),
            count++;
      return count == 0;
    }
  catch (const char * ErrorMsg)
    {
      fprintf(stderr, "error in regex '%s': %s\n", argv[1], ErrorMsg);
      return 2;
    }
}

#endif // REGEX_DEMO
