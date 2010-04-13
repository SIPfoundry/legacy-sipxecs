//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsSysLog.h>
#include <os/OsDateTime.h>
#include <utl/UtlInt.h>
#include <utl/UtlLongLongInt.h>
#include <utl/UtlBool.h>
#include <utl/UtlDateTime.h>
#include <utl/UtlSList.h>
#include <utl/UtlSListIterator.h>
#include <utl/UtlHashMapIterator.h>
#include <utl/XmlContent.h>
#include <net/XmlRpcBody.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const char* XmlVersion = XML_VERSION_1_0;
const int MAX_VALUE_NESTING_DEPTH = 12; // maximum nesting of arrays, structs, values

const size_t XmlRpcBody::MAX_LOG = 1000;

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
XmlRpcBody::XmlRpcBody()
   : HttpBody(XmlVersion,strlen(XmlVersion),CONTENT_TYPE_TEXT_XML)
{
}

XmlRpcBody::XmlRpcBody(const XmlRpcBody& rhs)
  : HttpBody(rhs)
{
}

XmlRpcBody* XmlRpcBody::copy() const
{
  return new XmlRpcBody(*this);
}

// Destructor
XmlRpcBody::~XmlRpcBody()
{
}

/* ============================ MANIPULATORS ============================== */


void XmlRpcBody::append(const char* string)
{
   mBody.append(string);
}


/* ============================ ACCESSORS ================================= */

ssize_t XmlRpcBody::getLength() const
{
   return (mBody.length());
}

void XmlRpcBody::getBytes(const char** bytes, ssize_t* length) const
{
   // This version of getBytes exists so that a caller who is
   // calling this method through an HttpBody will get the right
   // thing - we fill in the mBody string and then return that.
   UtlString tempBody;
   getBytes( &tempBody, length );
   ((XmlRpcBody*)this)->mBody = tempBody.data();
   *bytes = mBody.data();
}

void XmlRpcBody::getBytes(UtlString* bytes, ssize_t* length) const
{
   *bytes = mBody;
   *length = bytes->length();
}

bool XmlRpcBody::addValue(const UtlContainable* value)
{
   bool result = false;

   UtlString paramValue;

   // UtlInt
   if (value->isInstanceOf(UtlInt::TYPE))
   {
      UtlInt* pValue = (UtlInt *)value;
      // allow room for the widest possible value, INT_MIN = -2147483648
      char temp[20];
      sprintf(temp, "%" PRIdPTR, pValue->getValue());
      paramValue.append(BEGIN_INT);
      paramValue.append(temp);
      paramValue.append(END_INT);
      result = true;
   }
   // UtlLongLongInt
   else if (value->isInstanceOf(UtlLongLongInt::TYPE))
   {
      UtlLongLongInt* pValue = (UtlLongLongInt *)value;
      // always encode these in hex - more readable for values this big
      char temp[19];
      sprintf(temp, "%0#16" FORMAT_INTLL "x", static_cast<UInt64>(pValue->getValue()));
      paramValue.append(BEGIN_I8);
      paramValue.append(temp);
      paramValue.append(END_I8);
      result = true;
   }
   else if (value->isInstanceOf(UtlBool::TYPE))
   {
      UtlBool* pValue = (UtlBool *)value;
      paramValue.append(BEGIN_BOOLEAN);
      paramValue.append(pValue->getValue() ? "1" : "0");
      paramValue.append(END_BOOLEAN);
      result = true;
   }
   else if (value->isInstanceOf(UtlString::TYPE))
   {
      UtlString* pValue = (UtlString *)value;

      // Fix XSL-116: XML-RPC must escape special chars in string values
      result = XmlEscape(paramValue, *pValue);

      paramValue.insert(0, BEGIN_STRING);
      paramValue.append(END_STRING);
   }
   else if (value->isInstanceOf(UtlDateTime::TYPE))
   {
      UtlDateTime* pTime = (UtlDateTime *)value;
      OsDateTime time;
      pTime->getTime(time);
      UtlString isoTime;
      time.getIsoTimeStringZ(isoTime);
      paramValue = BEGIN_TIME + isoTime + END_TIME;
      result = true;
   }
   else if (value->isInstanceOf(UtlHashMap::TYPE))
   {
      result = addStruct((UtlHashMap *)value);
   }
   else if (value->isInstanceOf(UtlSList::TYPE))
   {
      //Try and be smart about the size of the body so we don't need to constantly re-allocate.
      UtlSList* pArray = (UtlSList *)value;
      mBody.capacity(pArray->entries() * 850);
      result = addArray(pArray);
   }
   else
   {
      OsSysLog::add(FAC_XMLRPC, PRI_CRIT, "XmlRpcBody::addValue unsupported type");
      assert(false);
   }

   mBody.append(paramValue);
   return result;
}


bool XmlRpcBody::addArray(UtlSList* array)
{
   bool result = false;
   mBody.append(BEGIN_ARRAY);

   UtlSListIterator iterator(*array);
   UtlContainable* pObject;
   while (   (pObject = iterator())
          && (result = addValue(pObject))
          )
   {
   }
   mBody.append(END_ARRAY);
   return result;
}

bool XmlRpcBody::addStruct(UtlHashMap* members)
{
   bool result = true;
   mBody.append(BEGIN_STRUCT);

   UtlHashMapIterator iterator(*members);
   UtlString* pName;
   UtlContainable* pObject;
   UtlString structName;
   while (result && (pName = (UtlString *)iterator()))
   {
      mBody.append(BEGIN_MEMBER);

      structName = BEGIN_NAME + *pName + END_NAME;
      mBody.append(structName);

      pObject = members->findValue(pName);
      result = addValue(pObject);
      mBody.append(END_MEMBER);
   }

   mBody.append(END_STRUCT);
   return result;
}

UtlContainable* XmlRpcBody::parseValue(TiXmlNode* valueNode, ///< pointer to the <value> node
                                       int nestDepth,        ///< current level of recursion
                                       UtlString& errorTxt   ///< explanation of parse error if any
                                       )
{
   UtlContainable* value = NULL;
   if (++nestDepth <= MAX_VALUE_NESTING_DEPTH)
   {
      UtlString paramValue;

      // four-byte signed integer
      TiXmlNode* typeNode = valueNode->FirstChild("i4");
      if (typeNode)
      {
         if (typeNode->FirstChild())
         {
            paramValue = typeNode->FirstChild()->Value();
            value = new UtlInt(atoi(paramValue));
         }
         else
         {
            OsSysLog::add(FAC_XMLRPC, PRI_ERR, "XmlRpcBody::parseValue"
                          " 'i4' element is empty");
            errorTxt.append("'i4' element is empty");
         }
      }
      else
      {
         typeNode = valueNode->FirstChild("int");
         if (typeNode)
         {
            if (typeNode->FirstChild())
            {
               paramValue = typeNode->FirstChild()->Value();
               value = new UtlInt(atol(paramValue));
            }
            else
            {
               OsSysLog::add(FAC_XMLRPC, PRI_ERR, "XmlRpcBody::parseValue"
                             " 'int' element is empty");
               errorTxt.append("'int' element is empty");
            }
         }
         else
         {
            typeNode = valueNode->FirstChild("i8");
            if (typeNode)
            {
               if (typeNode->FirstChild())
               {
                  paramValue = typeNode->FirstChild()->Value();

                  // We could use "atoll" here but it is obsolete,
                  // "strtoll" is the recommended function
                  // See http://www.delorie.com/gnu/docs/glibc/libc_423.html .
                  value = new UtlLongLongInt(strtoll(paramValue, 0, 0));
               }
               else
               {
                  OsSysLog::add(FAC_XMLRPC, PRI_ERR, "XmlRpcBody::parseValue"
                                " 'i8' element is empty");
                  errorTxt.append("'i8' element is empty");
               }
            }
            else
            {
               typeNode = valueNode->FirstChild("boolean");
               if (typeNode)
               {
                  if (typeNode->FirstChild())
                  {
                     paramValue = typeNode->FirstChild()->Value();
                     value = new UtlBool((atoi(paramValue)==1));
                  }
                  else
                  {
                     OsSysLog::add(FAC_XMLRPC, PRI_ERR, "XmlRpcBody::parseValue"
                                   " 'boolean' element is empty");
                     errorTxt.append("'boolean' element is empty");
                  }
               }
               else
               {
                  // string
                  // Note: In the string case, we allow a null string
                  typeNode = valueNode->FirstChild("string");
                  if (typeNode)
                  {
                     if (typeNode->FirstChild())
                     {
                        paramValue = typeNode->FirstChild()->Value();
                        value = new UtlString(paramValue);
                     }
                     else
                     {
                        value = new UtlString();
                     }
                  }
                  else
                  {
                     // dateTime.iso8601
                     typeNode = valueNode->FirstChild("dateTime.iso8601");
                     if (typeNode)
                     {
                        if (typeNode->FirstChild())
                        {
                           paramValue = typeNode->FirstChild()->Value(); // need to change to UtlDateTime
                           value = new UtlString(paramValue);
                        }
                        else
                        {
                           OsSysLog::add(FAC_XMLRPC, PRI_ERR, "XmlRpcBody::parseValue"
                                         " 'dateTime.iso8601' element is empty");
                           errorTxt.append("'dateTime.iso8601' element is empty");
                        }
                     }
                     else
                     {
                        // struct
                        typeNode = valueNode->FirstChild("struct");
                        if (typeNode)
                        {
                           if (!(value=parseStruct(typeNode, nestDepth, errorTxt)))
                           {
                              OsSysLog::add(FAC_XMLRPC, PRI_ERR, "XmlRpcBody::parseValue"
                                            " error parsing 'struct' content");
                              errorTxt.append(" in 'struct' element");
                           }
                        }
                        else
                        {
                           // array
                           typeNode = valueNode->FirstChild("array");
                           if (typeNode)
                           {
                              if (!(value=parseArray(typeNode, nestDepth, errorTxt)))
                              {
                                 OsSysLog::add(FAC_XMLRPC, PRI_ERR, "XmlRpcBody::parseValue"
                                               " error parsing 'array' content");
                              }
                           }
                           else
                           {
                              // Default case for string
                              if (valueNode->FirstChild())
                              {
                                 paramValue = valueNode->FirstChild()->Value();
                                 value = new UtlString(paramValue);
                              }
                              else
                              {
                                 value = new UtlString();
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }
   else
   {
      char errMsg[200];
      sprintf(errMsg, "parameter nesting depth exceeds maximum allowed (%d)",
              MAX_VALUE_NESTING_DEPTH);
      errorTxt.append(errMsg);
      OsSysLog::add(FAC_XMLRPC, PRI_ERR, "XmlRpcBody::parseValue %s",
                    errMsg
                    );
   }

   return value;
}


UtlHashMap* XmlRpcBody::parseStruct(TiXmlNode* structNode, ///< pointer to the <struct> node
                                    int nestDepth,         ///< current level of recursion
                                    UtlString& errorTxt    ///< explanation of parse error if any
                                    )
{
   UtlHashMap* returnedStruct = NULL;

   if (++nestDepth <= MAX_VALUE_NESTING_DEPTH)
   {
      // struct
      returnedStruct = new UtlHashMap();
      UtlString name;
      UtlString paramValue;
      TiXmlNode* memberValue;
      bool structIsOk = true;
      TiXmlNode* memberNode;
      for ((structIsOk = true, memberNode = structNode->FirstChild("member"));
           structIsOk && memberNode;
           memberNode = memberNode->NextSibling("member"))
      {
         TiXmlNode* memberName = memberNode->FirstChild("name");
         if (memberName)
         {
            if (memberName->FirstChild())
            {
               name = memberName->FirstChild()->Value();

               memberValue = memberNode->FirstChild("value");
               if (memberValue)
               {
                  UtlContainable* value;
                  if ((value=parseValue(memberValue, nestDepth, errorTxt)))
                  {
                     returnedStruct->insertKeyAndValue(new UtlString(name), value);
                  }
                  else
                  {
                     OsSysLog::add(FAC_XMLRPC, PRI_ERR, "XmlRpcBody::parseStruct"
                                   " error parsing member/value"
                                   );
                     errorTxt.append(" in member '");
                     errorTxt.append(name);
                     errorTxt.append("'");
                     structIsOk = false;
                  }
               }
               else
               {
                  OsSysLog::add(FAC_XMLRPC, PRI_ERR, "XmlRpcBody::parseStruct"
                                " 'member' element does not have a 'value' child"
                                );
                  errorTxt.append(" 'member' element name '");
                  errorTxt.append(name);
                  errorTxt.append("' does not have a value");
                  structIsOk=false;
               }
            }
            else
            {
               OsSysLog::add(FAC_XMLRPC, PRI_ERR, "XmlRpcBody::parseStruct"
                             " 'name' element is empty"
                             );
               errorTxt.append( "'name' element is empty");
               structIsOk = false;
            }
         }
         else
         {
            OsSysLog::add(FAC_XMLRPC, PRI_ERR, "XmlRpcBody::parseStruct"
                          " 'member' element does not have a 'name' child"
                          );
            errorTxt.append("'member' element does not have a 'name' child");
            structIsOk=false;
         }
      }

      if (!structIsOk)
      {
         deallocateContainedValues(returnedStruct);
         delete returnedStruct;
         returnedStruct=NULL;
      }
   }
   else
   {
      char errMsg[200];
      sprintf(errMsg, "parameter nesting depth exceeds maximum allowed (%d)",
              MAX_VALUE_NESTING_DEPTH);
      errorTxt.append(errMsg);
      OsSysLog::add(FAC_XMLRPC, PRI_ERR, "XmlRpcBody::parseStruct %s",
                    errMsg
                    );
   }

   return returnedStruct;
}

UtlSList* XmlRpcBody::parseArray(TiXmlNode* arrayNode, ///< pointer to the <array> node
                                 int nestDepth,        ///< current level of recursion
                                 UtlString& errorTxt   ///< explanation of parse error if any
                                 )
{
   UtlSList* pList = NULL;
   if (++nestDepth <= MAX_VALUE_NESTING_DEPTH)
   {
      pList = new UtlSList();
      bool arrayIsOk = true;

      // array
      UtlString paramValue;
      TiXmlNode* dataNode = arrayNode->FirstChild("data");
      if (dataNode)
      {
         TiXmlNode* valueNode;
         int index = 0;
         for (valueNode = dataNode->FirstChild("value");
              valueNode && arrayIsOk;
              valueNode = valueNode->NextSibling("value"))
         {
            UtlContainable* value;
            if ((value=parseValue(valueNode, nestDepth, errorTxt)))
            {
               pList->append(value);
               index++;
            }
            else
            {
               char errMsg[200];
               sprintf(errMsg, " in value %d of array", index);
               OsSysLog::add(FAC_XMLRPC, PRI_ERR, "XmlRpcBody::parseArray %s",
                             errMsg
                             );
               errorTxt.append(errMsg);
               arrayIsOk = false;
            }
         }
      }
      else
      {
         OsSysLog::add(FAC_XMLRPC, PRI_ERR, "XmlRpcBody::parseArray"
                       " 'array' element does not have 'data' child");
         errorTxt.append("'array' element does not have 'data' child");
         arrayIsOk=false;
      }

      if (!arrayIsOk)
      {
         deallocateContainedValues(pList);
         delete pList;
         pList = NULL;
      }
   }
   else
   {
      char errMsg[200];
      sprintf(errMsg, "parameter nesting depth exceeds maximum allowed (%d)",
              MAX_VALUE_NESTING_DEPTH);
      errorTxt.append(errMsg);
      OsSysLog::add(FAC_XMLRPC, PRI_ERR, "XmlRpcBody::parseArray %s",
                    errMsg
                    );
   }

   return pList;
}

/*
 * Note: The deallocateValue method deletes the value (UtlContainable)
 *       object passed to it, and sets the pointer to the object back to NULL, after
 *       recursively cleaning up any contained list or structure components.
 *
 *       The deallocateContainedValues methods below that take a UtlHashMap (structure)
 *       or UtlSList (array) and implement the recursive deletion by calling this one
 *       for each contained value.  This allows arbitrary array and structure nesting.
 *
 *       Note however that the deallocateContainedValues methods do NOT free the value
 *       object that is passed to them; this is deliberate, since in some other places
 *       that value is a local variable not created using 'new'.
 */

void XmlRpcBody::deallocateValue(UtlContainable*& value)
{
   if (value)
   {
      if (value->isInstanceOf(UtlHashMap::TYPE))
      {
         UtlHashMap* map = dynamic_cast<UtlHashMap*>(value);
         deallocateContainedValues(map);
         delete map;
      }
      else if (value->isInstanceOf(UtlSList::TYPE))
      {
         UtlSList* array = dynamic_cast<UtlSList*>(value);
         deallocateContainedValues(array);
         delete array;
      }
      else
      {
         delete value;
      }
      value = NULL;
   }
}

void XmlRpcBody::deallocateContainedValues(UtlHashMap* map)
{
   UtlHashMapIterator iterator(*map);

   UtlContainable* key;
   while ((key = iterator()))
   {
      UtlContainable* value = iterator.value();
      delete map->removeReference(key);
      if (value)
      {
         deallocateValue(value);
      }
   }
}

void XmlRpcBody::deallocateContainedValues(UtlSList* array)
{
   UtlContainable *value;
   while ((value = array->get()))
   {
      deallocateValue(value);
   }
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */
