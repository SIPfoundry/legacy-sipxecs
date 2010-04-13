Pico XML
========
Copyright (c) 2004-2006 by Brian O. Bush
Small, fast, blocking, memory efficient, non-reversable, embeddable pull-parser for XML in C.

What is it?
===========
Yet another XML library? Yes, but let me first provide a little background. I was writting a app back in late-2004 that required configuration information from a file. I didn't feel like writing a parser for yet another configuration file format and I didn't want to employ s-expressions since the file might be changed by individuals that didn't understand nor care to learn about s-expressions. So, XML seemed like a logical choice. However, the standard libraries aren't always present, and if they are, they are large and complex to employ. Overall, this barrier is what drove me to write Pico XML.
As opposed to typical SAX and DOM parsers, Pico XML is a pull-parser that is driven by user-code. Tim Bray on the xml-dev mailing list stated "pull parsing is the way to go in the future. The first 3 XML parsers (Lark, NXP, and expat) all were event-driven because... er well that was 1996, can't exactly remember, seemed like a good idea at the time." (Wednesday, 18 Sep 2002). Microsoft has an xml pull-parser called XmlTextReader, that has been cloned in libxml2.

Features
--------
	• Plain ANSI C, no external library dependencies
	• Tiny and embeddable (500 lines of C code)
	• Blocking parser
	• Handles start/end tags, text, attribute names and values
	• Translates entity references (on pico_getstr() calls)
	• Handles UTF-8 (ASCII is a proper subset of UTF-8, thus supported)

Caveats
-------
	• Like SAX1 with regards to features; SAX1 omits Comments, Lexical Information (CDATA sections, entity
          references, etc.), DTD declarations, Validation, Namespaces. However, Pico XML handles the most common
          entity references.
	• Non-validating, however, you can do your own validation, e.g., expect specific tags and fail if not
          found at the application level.
	• Does not handle Processing Instruction, CDATA, DTDs, comments, namespaces.

Building
========
Simply unpack the tarball and run the make file in the untarred pico directory:
$ make
gcc -c -o pico.o pico.c -g -Wall -O2 -I.
ar cr libpico.a pico.o
ranlib libpico.a
gcc  -c -o pico_test.o pico_test.c -g -Wall -O2 -I.
gcc -o pico_test pico_test.o pico.o -g -Wall -O2 -I. -lm
gcc -c -o pico_dump.o pico_dump.c -g -Wall -O2 -I.
gcc -o pico_dump pico_dump.o pico.o -g -Wall -O2 -I. -lm
You should now have two items: the "libpico.a" library and two test applications: (1) pico_test and (2) pico_dump.
The pico_dump application might prove useful in visually debugging xml files

Example Usage
=============
Basially, Pico XML is a forward-only tree walking interface to the loaded document. The basic usage model is: (1) build a pico context with content buffer or file, (2) loop over all nodes in document, and (3) free pico context. The following example dumps all nodes to stderr:
...
pico_t* pico = pico_new_file(filename);
while(1) {
  int event = pico_next(pico);
  if(event == PICO_EVENT_END_DOC) {
    break;
  } else {
    char* txt = pico_getstr(pico);
    fprintf(stderr, "[%s] '%s'\n", pico_event_str(event), txt);
    free(txt);
  }
}

A sample XML file containing the following content:
<cfg version="1.0"><ident>Test</ident></cfg>
Would produce the following output:
[start tag] 'cfg'
[attr name] 'version'
[attr val] '1.0'
[start tag] 'ident'
[text] 'Test'
[end tag] '/ident'
[end tag] '/cfg'

Download
========
Release 0.1 - Initial public release, 10-Dec-2006.
	• http://kd7yhr.org/downloads/pico_xml-0.1.tar.gz (source)

License
=======
GNU Lesser General Public License (LGPL)

Contact
=======
I would be interested in an applications using Pico XML, bug reports or patches.
Thanks,
Brian O. Bush,
brianobush@gmail.com
10-Dec-2006
