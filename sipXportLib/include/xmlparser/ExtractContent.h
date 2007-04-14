// Functions for extracting the text content of an XML element.

#include "xmlparser/tinyxml.h"
#include "utl/UtlString.h"

// Get the top-level text content of an XML element.
void textContentShallow(UtlString& string,
                        TiXmlNode *node);

// Get the complete text content of an XML element (including sub-elements).
void textContentDeep(UtlString& string,
                     TiXmlNode *node);

// Service function for textContentDeep.
void textContentDeepRecursive(UtlString& string,
                              TiXmlNode *node);
