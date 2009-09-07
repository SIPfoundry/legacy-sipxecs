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

#ifndef __OsTokenizer_h
#define __OsTokenizer_h

typedef struct PT_TOKEN {
        char * string;
        int offset[8];
        int offsets;
        struct PT_TOKEN * next;
} pt_token_t;

/* parse a string and return a (pointer to a) token structure */
/* sets the token count in *args */
pt_token_t * parse_tokenize(char * string, int * args);

/* return a pointer to the given token */
const char * parse_token(pt_token_t * t, int which);

/* destroy the token structure */
void parse_kill(pt_token_t * t);

#endif /* __OsTokenizer_h */
