// Functions for extracting the text content of an XML element.
// Actually, these work on any node.  That probably isn't useful,
// other than that it avoids some casing from TiXmlNode to
// TiXmlElement when recursing.

#include "xmlparser/ExtractContent.h"

void textContentShallow(UtlString& string,
                        TiXmlNode *node)
{
   // Clear the string.
   string.remove(0);

   // Iterate through all the children.
   for (TiXmlNode* child = node->FirstChild(); child;
        child = child->NextSibling())
   {
      // Examine the text nodes.
      if (child->Type() == TiXmlNode::TEXT)
      {
         // Append the content to the string.
         string.append(child->Value());
      }
   }
}

void textContentDeep(UtlString& string,
                     TiXmlNode *node)
{
   // Clear the string.
   string.remove(0);

   // Recurse into the XML.
   textContentDeepRecursive(string, node);
}

void textContentDeepRecursive(UtlString& string,
                              TiXmlNode *node)
{
   // Iterate through all the children.
   for (TiXmlNode* child = node->FirstChild(); child;
        child = child->NextSibling())
   {
      // Examine the text nodes.
      if (child->Type() == TiXmlNode::TEXT)
      {
         // Append the content to the string.
         string.append(child->Value());
      }
      else if (child->Type() == TiXmlNode::ELEMENT)
      {
         // Recurse on this element.
         textContentDeepRecursive(string, child);
      }
   }
}
