/*
 *         Title:  Pico XML, a tiny XML pull-parser
 *          File:  pico.c
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

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pico.h"

/* Tables and constants */

/* A table of the number of bytes in a UTF-8 sequence starting with
   the character used as the array index.  Note: a zero entry
   indicates an illegal initial byte. Generated with a python script
   from the utf-8 std. */
static int utf8_len[256] = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 0, 0
};

/* Types of characters; 0 is not valid, 1 is letters, 2 are digits
   (including '.') and 3 whitespace. Also generated with a throw-away
   python script. */
static int char_type[256] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 0, 0, 3, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0,
  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

/* Casefold ascii only; no chars above 0x80 are mapped to anything
   other than the identity. */
static int casefold[256] = {
  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
  16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
  32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
  48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
  64, 97, 98, 99, 100,101,102,103,104,105,106,107,108,109,110,111,
  112,113,114,115,116,117,118,119,120,121,122,91, 92, 93, 94, 95,
  96, 97, 98, 99, 100,101,102,103,104,105,106,107,108,109,110,111,
  112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
  128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
  144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
  160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
  176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
  192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
  208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
  224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
  240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255
};

/* Types of events: start element, end element, text, attr name, attr
   val and start/end document. Other events can be ignored! */
enum {
  EVENT_START = 0, /* Start tag */
  EVENT_END,       /* End tag */
  EVENT_TEXT,      /* Text */
  EVENT_ATTR_NAME, /* Attribute name */
  EVENT_ATTR_VAL,  /* Attribute value */
  EVENT_END_DOC,   /* End of document */
  EVENT_MARK,      /* Internal only; notes position in buffer */
  EVENT_NONE       /* Internal only; should never see this event */
};

/* Internal states that the parser can be in at any given time. */
enum {
  ST_START = 0,         /* starting base state, default state */
  ST_TEXT,              /* text state */
  ST_START_TAG,         /* start tag state */
  ST_START_TAGNAME,     /* start tagname state */
  ST_START_TAGNAME_END, /* start tagname ending state */
  ST_END_TAG,           /* end tag state */
  ST_END_TAGNAME,       /* end tag tagname state */
  ST_END_TAGNAME_END,   /* end tag tagname ending */
  ST_EMPTY_TAG,         /* empty tag state */
  ST_SPACE,             /* linear whitespace state */
  ST_ATTR_NAME,         /* attribute name state */
  ST_ATTR_NAME_END,     /* attribute name ending state */
  ST_ATTR_VAL,          /* attribute value starting state */
  ST_ATTR_VAL2,         /* attribute value state */
  ST_ERROR              /* error state */
};

/* character classes that we will match against; This could be expanded if
   need be, however, we are aiming for simple. */
enum {
  CLAZZ_NONE = 0,       /* matches nothing, a base state */
  CLAZZ_LEFT_ANGLE,     /* matches start tag '<' */
  CLAZZ_SLASH,          /* matches forward slash */
  CLAZZ_RIGHT_ANGLE,    /* matches end tag '>' */
  CLAZZ_EQUALS,         /* matches equals sign */
  CLAZZ_QUOTE,          /* matches double-quotes */
  CLAZZ_LETTERS,        /* matches a-zA-Z letters and digits 0-9 */
  CLAZZ_SPACE,          /* matches whitespace */
  CLAZZ_ANY             /* matches any ASCII character; will match all
                           above classes */
};

typedef struct {
  char* entity;
  int sz;
  char ch;
} entity_t;

entity_t entity_refs [] =  {
  {"lt;", 3, '<'},
  {"gt;", 3, '>'},
  {"amp;", 4, '&'},
  {"apos;", 5, '\''},
  {"quot;", 5, '"'},
  {0, 0}
};

static const char*
state_str(int state)
{
  switch(state) {
  case ST_START: return "start"; break;
  case ST_TEXT: return "text"; break;
  case ST_START_TAG: return "start tag"; break;
  case ST_START_TAGNAME: return "start tag name"; break;
  case ST_START_TAGNAME_END: return "start tag name end"; break;
  case ST_END_TAG: return "end tag"; break;
  case ST_END_TAGNAME: return "end tagname"; break;
  case ST_END_TAGNAME_END: return "end tag name end"; break;
  case ST_EMPTY_TAG: return "empty tag"; break;
  case ST_SPACE: return "space"; break;
  case ST_ATTR_NAME: return "attr name"; break;
  case ST_ATTR_NAME_END: return "attr name end"; break;
  case ST_ATTR_VAL: return "attr val"; break;
  case ST_ATTR_VAL2: return "attr val2"; break;
  case ST_ERROR:
  default:
    break;
  }
  return "err";
}

/* Map an internal event to an external visible event */
static pico_event_t
event_map(int event)
{
  switch(event) {
  case EVENT_START: return PICO_EVENT_START_TAG; break;
  case EVENT_END: return PICO_EVENT_END_TAG; break;
  case EVENT_TEXT: return PICO_EVENT_TEXT; break;
  case EVENT_ATTR_NAME: return PICO_EVENT_ATTR_NAME; break;
  case EVENT_ATTR_VAL: return PICO_EVENT_ATTR_VAL; break;
  case EVENT_END_DOC: return PICO_EVENT_END_DOC; break;
  }
  return PICO_EVENT_ERROR;
}

/* State transition table element; contains (1) current state, (2)
   clazz that must match, (3) next state if we match, and (4) event
   that is emitted upon match. If you want to see how take a simple
   idea and make it overly complicated, see:
   http://www.faqs.org/rfcs/rfc3076.html and
   http://www.w3.org/TR/REC-xml/ for details on XML.
*/
typedef struct state_t {
  int state, clazz, next_state, event;
} state_t;

/* Note: States must be grouped in match order AND grouped together! */
static state_t PICO_STATES [] = {
  /* [0-2] starting state, which also serves as the default state in case
     of error */
  { ST_START,         CLAZZ_SPACE,        ST_SPACE,             EVENT_NONE },
  { ST_START,         CLAZZ_LEFT_ANGLE,   ST_START_TAG,         EVENT_NONE },
  { ST_START,         CLAZZ_ANY,          ST_TEXT,              EVENT_MARK },

  /* [3-5] space state handles linear white space */
  { ST_SPACE,         CLAZZ_SPACE,        ST_SPACE,             EVENT_NONE },
  { ST_SPACE,         CLAZZ_LEFT_ANGLE,   ST_START_TAG,         EVENT_TEXT },
  { ST_SPACE,         CLAZZ_ANY,          ST_TEXT,              EVENT_MARK },

  /* [6-8] handle start tag */
  { ST_START_TAG,     CLAZZ_LETTERS,      ST_START_TAGNAME,     EVENT_MARK },
  { ST_START_TAG,     CLAZZ_SLASH,        ST_END_TAG,           EVENT_MARK },
  /* below added since some individuals get a little carried away with
     spacing around tag names, e.g. < tag > */
  { ST_START_TAG,     CLAZZ_SPACE,        ST_START_TAG,         EVENT_NONE },

  /* [9-12] handle start tag name */
  { ST_START_TAGNAME, CLAZZ_LETTERS,      ST_START_TAGNAME,     EVENT_NONE },
  { ST_START_TAGNAME, CLAZZ_SPACE,        ST_START_TAGNAME_END, EVENT_START },
  /* below added for tags without any space between tag and ending
     slash, e.g., <br/> */
  { ST_START_TAGNAME, CLAZZ_SLASH,        ST_EMPTY_TAG,         EVENT_END },
  { ST_START_TAGNAME, CLAZZ_RIGHT_ANGLE,  ST_START,             EVENT_START },

  /* [13-16] handle start tag name end */
  { ST_START_TAGNAME_END,  CLAZZ_LETTERS, ST_ATTR_NAME,         EVENT_MARK },
  /* below added to handle additional space in between attribute value
     pairs in start tags, e.g., <tag attr="2" attr2="test" > */
  { ST_START_TAGNAME_END,  CLAZZ_SPACE,   ST_START_TAGNAME_END, EVENT_NONE },
  { ST_START_TAGNAME_END,  CLAZZ_RIGHT_ANGLE, ST_START,         EVENT_START },
  /* below supports tags that are self-closing, e.g., <br /> */
  { ST_START_TAGNAME_END,  CLAZZ_SLASH,   ST_EMPTY_TAG,         EVENT_MARK },

  /* [17] handle empty tags, e.g., <br /> */
  { ST_EMPTY_TAG,     CLAZZ_RIGHT_ANGLE,  ST_START,             EVENT_END },

  /* [18] handle end tag, e.g., <tag /> */
  { ST_END_TAG,       CLAZZ_LETTERS,      ST_END_TAGNAME,       EVENT_NONE },

  /* [19-21] handle end tag name */
  { ST_END_TAGNAME,   CLAZZ_LETTERS,      ST_END_TAGNAME,       EVENT_NONE },
  { ST_END_TAGNAME,   CLAZZ_RIGHT_ANGLE,  ST_START,             EVENT_END },
  /* below adds support for spaces at the end of an end tag (before
     closing bracket) */
  { ST_END_TAGNAME,   CLAZZ_SPACE,        ST_END_TAGNAME_END,   EVENT_END },

  /* [22] handle ending of end tag name */
  { ST_END_TAGNAME_END, CLAZZ_SPACE,      ST_END_TAGNAME_END,   EVENT_NONE },
  { ST_END_TAGNAME_END, CLAZZ_RIGHT_ANGLE,ST_START,             EVENT_NONE },

  /* [23-25] handle text */
  { ST_TEXT,          CLAZZ_SPACE,        ST_SPACE,             EVENT_NONE },
  { ST_TEXT,          CLAZZ_LEFT_ANGLE,   ST_START_TAG,         EVENT_TEXT },
  { ST_TEXT,          CLAZZ_ANY,          ST_TEXT,              EVENT_NONE },

  /* [26-30] handle attribute names */
  { ST_ATTR_NAME,     CLAZZ_LETTERS,      ST_ATTR_NAME,         EVENT_MARK },
  /* below add support for space before the equals sign, e.g, <tag
     attr ="2"> */
  { ST_ATTR_NAME,     CLAZZ_SPACE,        ST_ATTR_NAME_END,     EVENT_ATTR_NAME },
  { ST_ATTR_NAME,     CLAZZ_EQUALS,       ST_ATTR_VAL,          EVENT_ATTR_NAME },

  /* [31-33] attribute name end */
  { ST_ATTR_NAME_END, CLAZZ_SPACE,        ST_ATTR_NAME_END,     EVENT_NONE },
  { ST_ATTR_NAME_END, CLAZZ_LETTERS,      ST_ATTR_NAME,         EVENT_MARK },
  { ST_ATTR_NAME_END, CLAZZ_EQUALS,       ST_ATTR_VAL,          EVENT_NONE },

  /* [34-35] handle attribute values, initial quote and spaces */
  { ST_ATTR_VAL,      CLAZZ_QUOTE,        ST_ATTR_VAL2,         EVENT_NONE },
  /* below handles initial spaces before quoted attribute value */
  { ST_ATTR_VAL,      CLAZZ_SPACE,        ST_ATTR_VAL,          EVENT_NONE },

  /* [36-37] handle actual attribute values */
  { ST_ATTR_VAL2,     CLAZZ_QUOTE,        ST_START_TAGNAME_END, EVENT_ATTR_VAL },
  { ST_ATTR_VAL2,     CLAZZ_LETTERS,      ST_ATTR_VAL2,         EVENT_MARK },

  /* End of table marker */
  { ST_ERROR,         CLAZZ_NONE,         ST_ERROR,             EVENT_NONE }
};

struct pico_t {
  char* buf;            /* reference to buffer */
  int bufsz,            /* size of buf */
    destroy_buf,        /* destroy buffer when done */
    state, event,       /* current state & event */
    ix,                 /* index into buffer */
    err,                /* count of errors */
    markix, marksz,     /* current element of interest */
    verbose;            /* verbose flag */
};


static void*
read_file(const char* filename, int* len)
{
  FILE* f;
  void* buf = 0;

  if(filename) {
    f = fopen(filename, "r");
    if(f) {
      fseek(f, 0, SEEK_END);
      *len = ftell(f);
      fseek(f, 0, SEEK_SET);
      buf = malloc(*len);
      if(buf) {
        fread(buf, 1, *len, f);
      }
      fclose(f);
    }
  }
  return buf;
}

void
pico_verbose(pico_t* p)
{
  if(p) {
    p->verbose = 1;
  }
}

static pico_t*
pico_new(const char* buf, int sz, int copy_buf)
{
  pico_t* p = 0;
  if(buf && sz > 0) {
    p = calloc(1, sizeof(pico_t));
    if(p) {
      p->bufsz = sz;
      if(copy_buf) {
        p->buf = malloc(sz);
        memcpy(p->buf, buf, sz);
      } else {
        p->buf = (char*) buf;
      }
      p->state = ST_START;
      p->event = EVENT_NONE;
    }
  }
  return p;
}

pico_t*
pico_new_buf(const char* buf, int sz)
{
  return pico_new(buf, sz, 1);
}

pico_t*
pico_new_file(const char* filename)
{
  char* buf;
  int len = 0;
  pico_t* p = 0;
  if(filename) {
    buf = read_file(filename, &len);
    if(buf && len > 0) {
      p = pico_new(buf, len, 0);
    }
  }
  return p;
}

void
pico_free(pico_t* p) {
  if(p) {
    free(p->buf);
    free(p);
  }
}

const char*
pico_event_str(pico_event_t event)
{
  switch(event) {
  case PICO_EVENT_START_TAG: return "start tag"; break;
  case PICO_EVENT_END_TAG: return "end tag"; break;
  case PICO_EVENT_TEXT: return "text"; break;
  case PICO_EVENT_ATTR_NAME: return "attr name"; break;
  case PICO_EVENT_ATTR_VAL: return "attr val"; break;
  case PICO_EVENT_END_DOC: return "end document"; break;
  default:
    break;
  }
  return "err";
}

/* This is the main driver that moves through the state table based on
   characters in the input buffer. Returns an event type: start, end,
   text, attr name, attr val */
pico_event_t
pico_next(pico_t* p)
{
  int i, j, c, jmp, fired, type, match, mark;
  if(p) {
    for(i=p->ix, fired=0, mark=0; i<p->bufsz && !fired; i+=jmp) {
      c = p->buf[i] & 0xff;
      jmp = utf8_len[c]; /* advance through buffer by utf-8 char sz */
      assert(jmp != 0);
      p->ix += jmp;
      if(p->verbose) {
        fprintf(stderr, "ix=%d; c='%c' (0x%x); state=[%s]\n", i,
                (isprint(c) ? c : '.'), c, state_str(p->state));
      }
      type = char_type[c];
      for(j=0, match = 0; PICO_STATES[j].state != ST_ERROR; j++) {
        if(PICO_STATES[j].state != p->state) { continue; }
        switch(PICO_STATES[j].clazz) {
        case CLAZZ_LETTERS:
          match = (type == 1 || type == 2);
          break;
        case CLAZZ_LEFT_ANGLE:  match = (c == '<');  break;
        case CLAZZ_SLASH:       match = (c == '/');  break;
        case CLAZZ_RIGHT_ANGLE: match = (c == '>');  break;
        case CLAZZ_EQUALS:      match = (c == '=');  break;
        case CLAZZ_QUOTE:       match = (c == '"');  break;
        case CLAZZ_SPACE:       match = (type == 3); break;
        case CLAZZ_ANY:         match = 1;           break;
        default:
          break;
        }
        if(match) { /* we matched a character class */
          if(PICO_STATES[j].event == EVENT_MARK) {
            if(!mark) { mark = i; } /* mark the position */
          } else if(PICO_STATES[j].event != EVENT_NONE) {
            if(mark) {
              /* basically we are guaranteed never to have an event of
                 type EVENT_MARK or EVENT_NONE here. */
              p->markix = mark;
              p->marksz = i-mark;
              p->event = event_map(PICO_STATES[j].event);
              fired = 1;
              if(p->verbose) {
                fprintf(stderr, "event fired: [%s]\n",
                        pico_event_str(p->event));
              }
            }
          }
          p->state = PICO_STATES[j].next_state; /* change state */
          break; /* break out of loop though state search */
        }
      }
      if(!match) { /* didn't match, default to start state */
        p->err++;
        fprintf(stderr,
                "WARNING: No match in state [%s] defaulting\n",
                state_str(p->state));
        p->state = ST_START;
      }
    }
    if(!fired) {
      p->event = PICO_EVENT_END_DOC;
    }
    return p->event;
  }
  return PICO_EVENT_ERROR;
}

#define TSIZE 64
int
pico_getint(pico_t* p)
{
  const char* src;
  char buf[TSIZE];
  int sz, val = 0;
  if(p) {
    src = (p->buf + p->markix);
    sz = p->marksz;
    if(sz >= TSIZE) { sz = TSIZE; } /* bound size */
    memcpy(buf, src, sz);
    buf[sz] = 0;
    val = atoi(buf);
  }
  return val;
}

/* fetches the content as a string from last event; e.g., for
   start/end tags, the actual tag text is returned, text/attr
   name/attr value all return the specific text, and start/end
   document return null. Note: if text context has whitespace, it is
   retained. */
char*
pico_getstr(pico_t* p) {
  char* buf = 0;
  int i, j, k, sz, found;
  const char* src;

  if(p) {
    sz = p->marksz;
    buf = malloc(sz + 1);
    if(buf) {
      src = (p->buf + p->markix);
      for(i=0, j=0; i<sz; i++) {
        /* i indexes src buffer, while j indexes output buffer */
        if(src[i] == '&' && i+1 < sz) {
          i++;
          for(k=0, found=0; entity_refs[k].entity; k++) {
            if(!strncmp(&src[i], entity_refs[k].entity, entity_refs[k].sz)) {
              buf[j] = entity_refs[k].ch;
              i+= entity_refs[k].sz - 1; /* increment index into src */
              found = 1;
              break;
            }
          }
          if(!found) { /* didn't find any defined entity */
            buf[j] = src[--i];
          }
        } else {
          buf[j] = src[i];
        }
        j++;
      }
      buf[j] = 0; /* null terminate */
    }
  }
  return buf;
}

/* like strcasecmp, but works on size specified buffers */
static int
memcasecmp(const char* x, const char* y, int n)
{
  int i, delta;
  for(i=0; i<n; i++) {
    delta = (casefold[x[i] & 0xff] - casefold[y[i] & 0xff]);
    if(delta) { return delta; }
  }
  return 0;
}

/* matches text with current event text; invalid for start/end
   document events; primarily useful in matching start tags; Note:
   case sensitive! */
int
pico_match(pico_t* p, const char* txt)
{
  int match = 0;
  int len;
  if(p && txt) {
    len = strlen(txt);
    if(p->ix + len < p->bufsz) {
      match = !(memcasecmp(p->buf + p->markix, txt, len));
    }
  }
  return match;
}
