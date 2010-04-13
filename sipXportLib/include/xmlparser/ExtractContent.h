// Functions for extracting the text content of an XML element.

#include "xmlparser/tinyxml.h"
#include "utl/UtlString.h"

/// Get the top-level text content of an XML element.
void textContentShallow(UtlString& string, ///< replaced with TEXT content of node.
                        const TiXmlNode *node);
///< This skips over and ignores any non-text nodes.
//   'node' may be NULL, in which case 'string' is set to empty.

/// Get the content of an XML element, which must be a leaf element with only text content.
bool textContent(UtlString& string, ///< TEXT content of node is appended to this.
                 const TiXmlNode *node);
/**<
 * @returns true iff all child nodes are of TiXmlNode::TEXT and/or TiXmlNode::COMMENT type;
 *          if any other types are encountered, returns false
 * If the children are valid, the values of all TEXT children are appended to string.
 * COMMENT content is ignored.
 * Otherwise, string is not modified.
 */

/// Get the complete text content of an XML element (including sub-elements).
void textContentDeep(UtlString& string,
                     TiXmlNode *node);
//   'node' may be NULL, in which case 'string' is set to empty.

/// Service function for textContentDeep.
void textContentDeepRecursive(UtlString& string,
                              TiXmlNode *node);
