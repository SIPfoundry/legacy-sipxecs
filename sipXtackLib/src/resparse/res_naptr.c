#include <sys/types.h>
#include <regex.h>
#include <string.h>

#include "resparse/rr.h"

#ifdef RES_PARSE_NAPTR

/*
 *  res_naptr.c
 *
 *  Support routines for NAPTR regexp fields (yuck!)
 *
 */

        /* res_naptr_split_regexp:
         *     Split a regexp field into the match and replace parts.
         */
int             res_naptr_split_regexp(const char *field,
                                       char       *delim_p,
                                       char       *match,
                                       const char **replace_p,
                                       int        *i_flag_p)
{
   /* (Did I ever mention what a bad idea it was to represent the search and
    * replace strings in the NAPTR records as one string field, rather than
    * two?)
    */
   int split_ok = 1;
   const char *p = field;
   char delim = *p++;
   *delim_p = delim;
   if (delim != '\0')
   {
      /* Copy the match regexp, un-escaping instances of delim. */
      char *q = match;
      while (1)
      {
         char c = *p++;
         switch (c)
         {
         case '\\':
            /* Read escaped character. */
            c = *p++;
            if (c == '\0')
            {
               /* NUL is an error. */
               split_ok = 0;
               goto match_scan_done;
            }
            else if (c == delim)
            {
               /* delim is copied without escaping. */
               *q++ = delim;
            }
            else
            {
               /* All other characters are copied, preserving escaping. */
               *q++ = '\\';
               *q++ = c;
            }
            break;
         case '\0':
            /* NUL is an error. */
            split_ok = 0;
            goto match_scan_done;
         default:
            if (c == delim)
            {
               /* delim means match regexp is finished. */
               goto match_scan_done;
            }
            else
            {
               /* Other characters are copied. */
               *q++ = c;
            }
            break;
         }
      }
     match_scan_done:;
      /* At this point, we've just processed the medial delim,
       * or an error has been found. */
      if (split_ok)
      {
         /* Add final NUL to match string. */
         *q++ = '\0';
         /* Return location of replacement string. */
         *replace_p = p;

         /* Scan the replacement string to verify it is syntactically OK
          * and find the final delim, so we can test for flags.
          */
         while (1)
         {
            char c = *p++;
            switch (c)
            {
            case '\\':
               /* The rules for '\' in the replacement part are vague.
                * Due to the grammar:
                *     repl         = 1 * ( OCTET /  backref )
                *     backref      = "\" 1POS_DIGIT
                * I think that '\[digit]' is special, but other occurrences
                * of '\' are not special.
                */
               c = *p;
               if (c == '\0')
               {
                  /* NUL is an error. */
                  split_ok = 0;
                  goto replacement_scan_done;
               }
               else if (c >= '1' && c <= '9')
               {
                  /* Step over the digit as part of a backreference. */
                  p++;
               }
               else
               {
                  /* All other following characters mean '/' is not special. */
               }
               break;
            case '\0':
               /* NUL is an error. */
               split_ok = 0;
               goto replacement_scan_done;
            default:
               if (c == delim)
               {
                  /* delim means replacement string is finished. */
                  goto replacement_scan_done;
               }
               else
               {
                  /* Other characters are copied. */
               }
               break;
            }
         }
        replacement_scan_done:;
         /* At this point, we've just processed the final delim,
          * or an error has been found. */
         if (split_ok)
         {
            /* Scan the remainder of the string as flags. */
            /* Initialize the output. */
            *i_flag_p = 0;
            while (1)
            {
               char c = *p++;
               switch (c)
               {
               case 'i':
               case 'I':
                  *i_flag_p = 1;
                  break;
               case '\0':
                  goto done;
               default:
                  /* All other chars are errors. */
                  split_ok = 0;
                  goto done;
               }
            }
           done:;
            /* At this point, we've processed the whole string,
             * or an error has been found. */
         }
      }
   }
   return split_ok;
}


        /* res_naptr_replace:
         *     Construct a replacement string from a match.
         */
char             *res_naptr_replace(const char *replace,
                                    char       delim,
                                    regmatch_t *match,
                                    const char *original,
                                    int        keep_context)
{
   /* Set up string to write to. */
   int allocated_size = 10;
   char *string = malloc(allocated_size);
   int used_size = 0;

   /* If requested, copy the portion of the original string before the match. */
   if (keep_context)
   {
      int len = match[0].rm_so;
      if (used_size + len > allocated_size)
      {
         allocated_size *= 2;
         allocated_size += len;
         string = realloc(string, allocated_size);
      }
      strncpy(&string[used_size], &original[0], len);
      used_size += len;
   }

   /* Scan the replacement string, appending to the output string. */
   const char *p = replace;
   while (1)
   {
      char c = *p++;
      switch (c)
      {
      case '\\':
         /* The rules for '\' in the replacement part are vague.
          * Due to the grammar:
          *     repl         = 1 * ( OCTET /  backref )
          *     backref      = "\" 1POS_DIGIT
          * I think that '\[digit]' is special, but other occurrences
          * of '\' are not special.
          */
         c = *p;
         if (c == '\0')
         {
            /* NUL is an error. */
            goto done;
         }
         else if (c >= '1' && c <= '9')
         {
            /* Consume the digit. */
            p++;
            /* Copy backreference. */
            int n = c - '0';
            // If the substring matched
            if (match[n].rm_so != -1)
            {
               int len = match[n].rm_eo - match[n].rm_so;
               if (used_size + len > allocated_size)
               {
                  allocated_size *= 2;
                  allocated_size += len;
                  string = realloc(string, allocated_size);
               }
               strncpy(&string[used_size], &original[match[n].rm_so], len);
               used_size += len;
            }
         }
         else
         {
            /* All other following characters mean '/' is not special. */
            if (used_size + 1 > allocated_size)
            {
               allocated_size *= 2;
               string = realloc(string, allocated_size);
            }
            string[used_size++] = '\\';
            /* Continue to re-process 'c'. */
         }
         break;
      case '\0':
         /* NUL is an error. */
         goto done;
      default:
         if (c == delim)
         {
            /* delim means replacement string is finished. */
            goto done;
         }
         else
         {
            /* Other characters are copied. */
            /* All other characters are not special. */
            if (used_size + 1 > allocated_size)
            {
               allocated_size *= 2;
               string = realloc(string, allocated_size);
            }
            string[used_size++] = c;
         }
         break;
      }
   }
  done:;

   /* If requested, copy the portion of the original string after the match. */
   if (keep_context)
   {
      size_t len = strlen(original) - match[0].rm_eo;
      if (used_size + len > allocated_size)
      {
         allocated_size *= 2;
         allocated_size += len;
         string = realloc(string, allocated_size);
      }
      strncpy(&string[used_size], &original[match[0].rm_eo], len);
      used_size += len;
   }

   /* We are done constructing the replacement string.
    * Or an error occurred, which should not happen, as the
    * replacement string was syntax-checked by res_naptr_split_regexp.
    */
   /* Add the final NUL. */
   if (used_size + 1 > allocated_size)
   {
      allocated_size *= 2;
      string = realloc(string, allocated_size);
   }
   string[used_size++] = '\0';

   return string;
}

#endif /* RES_PARSE_NAPTR */
