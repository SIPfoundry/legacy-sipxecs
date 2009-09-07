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
#include "assert.h"
#include "string.h"

// APPLICATION INCLUDES
#include "utl/XmlContent.h"
#include "utl/UtlRegex.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

// Do the work for the XmlEscape functions.
bool XmlEscapeInternal(/// Destination to append escaped characters to.
                       UtlString& destination,
                       /// Source string.
                       const char* source,
                       /// Source string length.
                       size_t length);


/**
 * XmlContent provides conversion functions for escaping and unescaping UtlStrings
 * as appropriate for use in XML attribute and element content.
 *
 * At present, this makes no accomodation for character set differences; input is assumed
 * to be 8 bit characters.  The following characters are encoded using
 * the mandatory character entities:
 *   - < => &lt;
 *   - & => &amp;
 *   - > => &gt;
 *   - ' => &apos;
 *   - " => &quot;
 *
 * Other character values outside the range of valid 8-bit characters in XML:
 * - [pound]x09 | [pound]x0A | [pound]x0D | [[pound]x20-[pound]FF]
 * are encoded using the numeric entity encoding (&[pound]x??;).
 *
 * While this is not strictly XML conformant (in that it does not explicitly deal with
 * larger-size character encodings), it is symmetric (esaping and unescaping any 8 bit string
 * using these routines will always produce the original string), and will interoperate correctly
 * for any 8 bit encoding.
 */

#define QUOTE        "&quot;" /* " \x22 */
#define AMPERSAND    "&amp;"  /* & \x26 */
#define APOSTROPHE   "&apos;" /* ' \x27 */
#define LESS_THAN    "&lt;"   /* < \x3c */
#define GREATER_THAN "&gt;"   /* > \x3e */

#define XML_CHARS "[\\x09\\x0a\\x0d\\x20\\x21\\x23-\\x25\\x28-\\x3b\\x3d\\x3f-\\xff]"
#define ESC_CHARS "[\\x00-\\x08\\x0b\\x0c\\x0e-\\x1f\\x22\\x26\\x27\\x3c\\x3e]"

const RegEx CopyChars( "(" XML_CHARS "*)(" ESC_CHARS ")?" );

/// Append escaped source string onto destination string
/**
 * The contents of the source string are appended to the destination string, with all
 * characters escaped as described above.
 * @returns true for success, false if an error was returned from any UtlString operation.
 */
bool XmlEscape(UtlString& destination, const UtlString& source)
{
   return XmlEscapeInternal(destination, source.data(), source.length());
}

/// Append escaped source string onto destination string
/**
 * The contents of the source string are appended to the destination string, with all
 * characters escaped as described above.
 * @returns true for success, false if an error was returned from any UtlString operation.
 */
bool XmlEscape(UtlString& destination, const char* source)
{
   return XmlEscapeInternal(destination, source, strlen(source));
}

// Do the work for the XmlEscape functions.
bool XmlEscapeInternal(UtlString& destination,
                       const char* source,
                       size_t srcLen)
{
   bool resultOk = false;

   if (srcLen > 0)
   {
      // make sure that the destination is at least large enough to add the source
      size_t minDstLen = destination.length() + srcLen;
      if (destination.capacity(minDstLen) >= minDstLen)
      {
         RegEx copyChars(CopyChars);

         UtlString escapeChar;
         bool matched;
         // each iteration picks up leading n valid chars (n may be zero) and one that needs to be escaped.
         for (matched = copyChars.Search(source, srcLen);
              matched;
              matched = copyChars.SearchAgain()
              )
         {
            // copy any leading characters that don't need to be escaped
            copyChars.MatchString(&destination,1);

            if (copyChars.MatchString(&escapeChar,2)) // was there an escaped character?
            {
               switch(*escapeChar.data())
               {
               case '\x22' /* " */:
                  destination.append(QUOTE);
                  break;
               case '\x26' /* & */:
                  destination.append(AMPERSAND);
                  break;
               case '\x27' /* ' */:
                  destination.append(APOSTROPHE);
                  break;
               case '\x3c' /* < */:
                  destination.append(LESS_THAN);
                  break;
               case '\x3e' /* > */:
                  destination.append(GREATER_THAN);
                  break;
               default:
               {
                  // outside the valid range; escape as numeric entity
                  char hexval[7];
                  sprintf(hexval, "&#x%02x;", *escapeChar.data());
                  destination.append(hexval);
               }
               break;
               }
               escapeChar.remove(0); // clear for next iteration
            }
         }
         resultOk = true;
      }
      else
      {
         // UtlString capacity failed
         assert(false);
      }
   }
   else
   {
      resultOk = true; // empty source - easy
   }
   return resultOk;
}

static const RegEx Entity("&(?:(quot)|(amp)|(apos)|(lt)|(gt)|#(?:([0-9]{1,3})|x([0-9a-fA-F]{1,2})));([^&]*)");
/* substring indicies          1      2     3      4    5        6             7                    8       */
/// Append unescaped source string onto destination string
/**
 * The contents of the source string are appended to the destination string, with all
 * characters unescaped as described above.
 * @returns true for success, false if an error was returned from any UtlString operation.
 */
bool XmlUnEscape(UtlString& destination, const UtlString& source)
{
   bool resultOk = false;
   size_t srcLen = source.length();

   if (srcLen > 0)
   {
      // make sure that the destination is large enough to add the source (which cannot grow)
      size_t minDstLen = destination.length() + srcLen;
      if (destination.capacity(minDstLen) >= minDstLen)
      {
         RegEx entity(Entity);

         UtlString number;
         bool matched;
         bool matchedOnce;
         bool firstMatch = true;
         // each iteration picks up leading n valid chars (n may be zero) and one that needs to be escaped.
         for (matchedOnce = matched = entity.Search(source.data(), srcLen);
              matched;
              matched = entity.SearchAgain()
              )
         {
            if (firstMatch)
            {
               // copy any leading characters that don't need to be escaped
               entity.BeforeMatchString(&destination);
               firstMatch = false;
            }

            if      (entity.MatchString(NULL,1))
            {
               destination.append('"');
            }
            else if (entity.MatchString(NULL,2))
            {
               destination.append('&');
            }
            else if (entity.MatchString(NULL,3))
            {
               destination.append("'");
            }
            else if (entity.MatchString(NULL,4))
            {
               destination.append('<');
            }
            else if (entity.MatchString(NULL,5))
            {
               destination.append('>');
            }
            else if (entity.MatchString(&number,6))
            {
               char* unconverted;
               int decimalNum = strtol(number.data(), &unconverted, /* base */ 10);

               if ('\000'==*unconverted && decimalNum >= 0 && decimalNum < 256)
               {
                  destination.append(decimalNum);
               }
               else
               {
                  // invalid decimal numeric entity - transcribe it untranslated
                  destination.append(number);
               }
               number.remove(0); // clear for next iteration
            }
            else if (entity.MatchString(&number,7))
            {
               char* unconverted;
               int decimalNum = strtol(number.data(), &unconverted, /* base */ 16);

               if ('\000'==*unconverted && decimalNum >= 0 && decimalNum < 256)
               {
                  destination.append(decimalNum);
               }
               else
               {
                  // invalid decimal numeric entity - transcribe it untranslated
                  destination.append(number);
               }
               number.remove(0); // clear for next iteration
            }
            else
            {
               assert(false); // the Entity expression should have matched one of the above
            }

            // copy any leading characters that don't need to be escaped
            entity.MatchString(&destination,8);

         }
         if (!matchedOnce)
         {
            // there were no entities, so just copy the content.
            destination.append(source);
         }

         resultOk = true;
      }
      else
      {
         // UtlString capacity failed
         assert(false);
      }
   }
   else
   {
      resultOk = true; // empty source - easy
   }

   return resultOk;
}
