/*
 *         Title:  Pico XML, a tiny XML pull-parser
 *          File:  pico.h
 *        Author:  Brian O. Bush, http://kd7yhr.org
 *       License:  LGPL
 *       Version:  0.1
 * Last revision:  10-Dec-2006
 *
 * Copyright (c) 2004-2006 by Brian O. Bush
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 *
 * This library is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU Library General Public License for moref
 * details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA.
 *
 * If you take advantage of the option in the LGPL to put a
 * particular version of this library under the GPL, the author
 * would regard it as polite if you would put any direct
 * modifications under the LGPL as well, and include a copy of
 * this request near the beginning of the modified library
 * source.  A "direct modification" is one that enhances or
 * extends the library in line with its original concept, as
 * opposed to developing a distinct application or library which
 * might use it.
 */

#ifndef _PICO_H_
#define _PICO_H_

/* Event types that are returned from pico_next */
typedef enum {
  /* Note: There is no start document event type */
  PICO_EVENT_START_TAG = 0, /* Start tag */
  PICO_EVENT_END_TAG,       /* End tag */
  PICO_EVENT_TEXT,          /* Text */
  PICO_EVENT_ATTR_NAME,     /* Attribute name */
  PICO_EVENT_ATTR_VAL,      /* Attribute value */
  PICO_EVENT_END_DOC,       /* End of document */
  PICO_EVENT_ERROR          /* Error */
} pico_event_t;

/* An opaque handle to the pico parser context itself */
typedef struct pico_t pico_t;

/* Create a pico parser context with a buffer of specific size. The
   buffer is copied, thus the source can be disposed of during pico
   context usage. */
pico_t* pico_new_buf(const char* buf, int sz);

/* Create a pico parser context from a file */
pico_t* pico_new_file(const char* filename);

/* Set pico context to verbose on parsing */
void pico_verbose(pico_t* p);

/* Free a pico parser context */
void pico_free(pico_t* p);

/* Step parser to next event; Returns a type that is defined in the
   above enumeration, see TYPE_xxx. */
pico_event_t pico_next(pico_t* p);

/* Get a human readable version of the event; useful in debugging. */
const char* pico_event_str(pico_event_t event);

/* Matches text with current event text; invalid for start/end
   document types; primarily useful in matching start tags */
int pico_match(pico_t* p, const char* txt);

/* Fetches the content from last event; e.g., for start/end tags, the
   actual tag text is returned, text/attr name/attr value all return
   the specific text, and start/end document return null. Note: if
   text has whitespace, it is retained. */
char* pico_getstr(pico_t* p);

/* Similar to pico_getstr, however, returns an integer value; also
   does no allocation */
int pico_getint(pico_t* p);


#endif /* _PICO_H_ */
