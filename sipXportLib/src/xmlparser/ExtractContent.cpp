// Functions for extracting the text content of an XML element.
// Actually, these work on any node.  That probably isn't useful,
// other than that it avoids some casing from TiXmlNode to
// TiXmlElement when recursing.

#include "xmlparser/ExtractContent.h"

void textContentShallow(UtlString& string,
                        const TiXmlNode *node)
{
   // Clear the string.
   string.remove(0);

   // Exit if node == NULL.
   if ( !node )
      return;

   // Iterate through all the children.
   for (const TiXmlNode* child = node->FirstChild(); child;
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

bool textContent(UtlString& string,
                 const TiXmlNode *node)
{
   bool allTextContent = true;

   // save the original length of the string so that we can return to it in case of an error.
   size_t originalLength = string.length();

   // Iterate through all the children.
   for (const TiXmlNode* child = node->FirstChild();
        allTextContent && child;
        child = child->NextSibling())
   {
      // Examine the text nodes.
      switch (child->Type())
      {
      case TiXmlNode::TEXT:
         string.append(child->Value());
         break;

      case TiXmlNode::COMMENT:
         // this is allowed but ignored
         break;

      default:
         allTextContent = false;
      }
   }

   if (!allTextContent)
   {
      string.remove(originalLength);
   }

   return allTextContent;
}


void textContentDeep(UtlString& string,
                     TiXmlNode *node)
{
   // Clear the string.
   string.remove(0);

   // Exit if node == NULL.
   if ( !node )
      return;

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
