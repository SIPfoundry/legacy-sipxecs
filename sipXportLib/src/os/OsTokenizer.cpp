//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


/* This code was taken from a program I wrote some time ago. It parses a string
 * for whitespace and quotes much like a shell would. Characters enclosed in
 * quotes are counted as a single token. Extra whitespace at the beginning and
 * end of the string is removed. The tokenizing function returns a token
 * structure, which can be used to access the tokens in the original string
 * (which will have been modified). Once the tokens have been used by the
 * caller, the token structure must be destroyed to free the token pointers. */

#include <stdlib.h>
#include <string.h>

#include "os/OsTokenizer.h"

pt_token_t * parse_tokenize(char * string, int * args)
{
        pt_token_t * first, * t;
        int which = 0, offset = 0, quoted;
        first = t = (pt_token_t *) malloc(sizeof(pt_token_t));
        if(!t)
                return NULL;
        *args = 0;
        t->string = string;
        t->offsets = 0;
        t->next = NULL;
        while(string[offset])
        {
                while(string[offset] && (string[offset] == ' ' || string[offset] == '\t'))
      {
         offset++;
      }
                if(!string[offset])
      {
                        break;
      }
                if((quoted = (string[offset] == '"')))
      {
                        offset++;
      }
                /* add a token offset */
                t->offset[which] = offset;
                t->offsets = ++which;
                ++*args;
                if(which == 8)
                {
                        t->next = (pt_token_t *) malloc(sizeof(pt_token_t));
                        if(!t)
                        {
                                parse_kill(first);
                                return NULL;
                        }
                        t = t->next;
                        t->string = string;
                        t->offsets = which = 0;
                        t->next = NULL;
                }
                /* scan for end of token */
                if(!quoted)
      {
                        while(string[offset] && string[offset] != ' ' && string[offset] != '\t')
         {
                                offset++;
         }
      }
                else
      {
                        while(string[offset] && string[offset] != '"')
         {
            offset++;
         }
      }
                if(!string[offset])
      {
                        break;
      }
                string[offset++] = 0;
        }
        return first;
}

const char * parse_token(pt_token_t * t, int which)
{
        if(t && t->offsets == 1 && which == 1)
   {
                return "";
   }
        while(which > 7)
        {
                if(t)
                        t = t->next;
                which -= 8;
        }
        if(!t)
   {
                return NULL;
   }
        if(which >= t->offsets)
   {
                return NULL;
   }
        return t->string + t->offset[which];
}

void parse_kill(pt_token_t * t)
{
        pt_token_t * next;
        while(t)
        {
                next = t->next;
                free(t);
                t = next;
        }
}
