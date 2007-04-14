// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <utl/UtlInt.h>
#include <utl/UtlLongLongInt.h>
#include <utl/UtlBool.h>
#include <utl/UtlDateTime.h>
#include <utl/UtlSListIterator.h>
#include <utl/UtlHashMapIterator.h>
#include <os/OsSysLog.h>
#include <xmlparser/tinyxml.h>
#include "net/XmlRpcResponse.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
XmlRpcResponse::XmlRpcResponse() :
   mpResponseBody(NULL),
   mResponseValue(NULL),   
   mFaultCode(ILL_FORMED_CONTENTS_FAULT_CODE),
   mFaultString(ILL_FORMED_CONTENTS_FAULT_STRING)
{
}

// Destructor
XmlRpcResponse::~XmlRpcResponse()
{
   // Clean up the memory in mResponseValue
   if (mResponseValue)
   {
      cleanUp(mResponseValue);
      mResponseValue = NULL;
   }
   
   if (mpResponseBody)
   {
      delete mpResponseBody;
      mpResponseBody = NULL;
   }
}

bool XmlRpcResponse::parseXmlRpcResponse(UtlString& responseContent)
{
   bool result = false;
   
   // Parse the XML-RPC response
   TiXmlDocument doc("XmlRpcResponse.xml");
   
   doc.Parse(responseContent);      
   if (!doc.Error())
   {
      TiXmlNode* rootNode = doc.FirstChild ("methodResponse");      
      if (rootNode != NULL)
      {
         // Positive response example
         // 
         // <methodResponse> 
         //   <params>
         //     <param>
         //       <value><string>South Dakota</string></value>
         //     </param>
         //   </params>
         // </methodResponse>
         
         TiXmlNode* paramsNode = rootNode->FirstChild("params");        
         if (paramsNode != NULL)
         {
            TiXmlNode* paramNode = paramsNode->FirstChild("param");
            if (paramNode)
            {
               TiXmlNode* subNode = paramNode->FirstChild("value");              
               if (subNode)
               {
                  result = parseValue(subNode);
               }
            }
         }
         else
         {
            // Fault response example
            //
            // <methodResponse>
            //   <fault>
            //     <value>
            //       <struct>
            //         <member>
            //           <name>faultCode</name>
            //           <value><int>4</int></value>
            //         </member>
            //         <member>
            //           <name>faultString</name>
            //           <value><string>Too many parameters.</string></value>
            //         </member>
            //       </struct>
            //     </value>
            //   </fault>
            // </methodResponse>
            
            TiXmlNode* faultNode = rootNode->FirstChild("fault");
            
            if (faultNode != NULL)
            {
               TiXmlNode* subNode = faultNode->FirstChild("value");
               
               if (subNode != NULL)
               {
                  subNode = subNode->FirstChild("struct");
                  
                  if (subNode != NULL)
                  {
                     for (TiXmlNode* memberNode = subNode->FirstChild("member");
                          memberNode; 
                          memberNode = memberNode->NextSibling("member"))
                     {
                        UtlString nameValue;
                        if (memberNode->FirstChild("name") && (memberNode->FirstChild("name"))->FirstChild())
                        {
                           nameValue = (memberNode->FirstChild("name"))->FirstChild()->Value();                         
                        
                           if (nameValue.compareTo("faultCode") == 0)
                           {
                              if (memberNode->FirstChild("value"))
                              {
                                 TiXmlNode* valueNode = (memberNode->FirstChild("value"))->FirstChild("int");
                                 if (valueNode && valueNode->FirstChild())
                                 {
                                    mFaultCode = atoi(valueNode->FirstChild()->Value());
                                 }
                                 
                                 valueNode = (memberNode->FirstChild("value"))->FirstChild("i4");
                                 if (valueNode && valueNode->FirstChild())
                                 {
                                    mFaultCode = atoi(valueNode->FirstChild()->Value());
                                 }
                              }
                           }
                              
                           if (nameValue.compareTo("faultString") == 0)
                           {
                              TiXmlNode* valueNode = memberNode->FirstChild("value");
                              if (valueNode)
                              {
                                 TiXmlNode* stringNode = valueNode->FirstChild("string");
                                 if (stringNode && stringNode->FirstChild())
                                 {
                                    mFaultString = stringNode->FirstChild()->Value();
                                 }
                                 else
                                 {
                                    if (valueNode->FirstChild())
                                    {
                                       mFaultString = valueNode->FirstChild()->Value();
                                    }
                                    else
                                    {
                                       mFaultString = NULL;
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
      }
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_ERR,
                    "XmlRpcResponse::parseXmlRpcResponse ill formatted xml contents in %s. Parsing error = %s",
                     responseContent.data(), doc.ErrorDesc());
      result = false;
   }
   
   
   return result;  
}

/* ============================ MANIPULATORS ============================== */


/* ============================ ACCESSORS ================================= */

bool XmlRpcResponse::setResponse(UtlContainable* value)
{
   bool result = false;
   assert(mpResponseBody == NULL);    // response body should only be created once
   

   // Start to construct the XML-RPC body
   mpResponseBody = new XmlRpcBody();
   assert(mpResponseBody != NULL);    // if not true, allocation failed

   mpResponseBody->append(BEGIN_RESPONSE);   
   mpResponseBody->append(BEGIN_PARAMS);   
   mpResponseBody->append(BEGIN_PARAM);  
   
   result = mpResponseBody->addValue(value);        
   
   mpResponseBody->append(END_PARAM);
   mpResponseBody->append(END_PARAMS);   
   mpResponseBody->append(END_RESPONSE);   
        
   UtlString bodyString;
   int bodyLength;
   mpResponseBody->getBytes(&bodyString, &bodyLength);
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "mpResponseBody::setResponse XML-RPC response message = \n%s", bodyString.data());
   return result;
}


bool XmlRpcResponse::setFault(int faultCode, const char* faultString)
{
   bool result = true;
   mFaultCode = faultCode;
   mFaultString = faultString;

   // Start to construct the XML-RPC body for fault response
   assert(mpResponseBody == NULL);    // response body should only be created once

   mpResponseBody = new XmlRpcBody();
   assert(mpResponseBody != NULL);    // if not true, allocation failed
   
   // Fault response example
   //
   // <methodResponse>
   //   <fault>
   //     <value>
   //       <struct>
   //         <member>
   //           <name>faultCode</name>
   //           <value><int>4</int></value>
   //         </member>
   //         <member>
   //           <name>faultString</name>
   //           <value><string>Too many parameters.</string></value>
   //         </member>
   //       </struct>
   //     </value>
   //   </fault>
   // </methodResponse>

   mpResponseBody->append(BEGIN_RESPONSE);   
   mpResponseBody->append(BEGIN_FAULT);   
   mpResponseBody->append(BEGIN_STRUCT);
      
   mpResponseBody->append(BEGIN_MEMBER);   
   mpResponseBody->append(FAULT_CODE);
   
   char temp[10];
   sprintf(temp, "%d", mFaultCode);
   UtlString paramValue = BEGIN_INT + UtlString(temp) + END_INT;
   mpResponseBody->append(paramValue);
   
   mpResponseBody->append(END_MEMBER);   
   
   mpResponseBody->append(BEGIN_MEMBER);   
   mpResponseBody->append(FAULT_STRING);
   
   paramValue = BEGIN_STRING + mFaultString + END_STRING;
   mpResponseBody->append(paramValue);
   
   mpResponseBody->append(END_MEMBER);   
      
   mpResponseBody->append(END_STRUCT);   
   mpResponseBody->append(END_FAULT);   
   mpResponseBody->append(END_RESPONSE);
      
   UtlString bodyString;
   int bodyLength;
   mpResponseBody->getBytes(&bodyString, &bodyLength);
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "mpResponseBody::setFault XML-RPC response message = \n%s", bodyString.data());

   return result;
}


bool XmlRpcResponse::getResponse(UtlContainable*& value)
{
   bool result = false;
   
   value = mResponseValue;
   
   if (value)
   {
      result = true;
   }
   else
   {
      result = false;
   }
   
   return result;
}


void XmlRpcResponse::getFault(int* faultCode, UtlString& faultString)
{
   *faultCode = mFaultCode;
   faultString = mFaultString;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
bool XmlRpcResponse::parseValue(TiXmlNode* subNode)
{
   bool result = false;

   // Clean up the memory in mResponseValue
   if (mResponseValue)
   {
      cleanUp(mResponseValue);
      mResponseValue = NULL;
   }
   
   UtlString paramValue;
                  
   // four-byte signed integer
   TiXmlNode* valueNode = subNode->FirstChild("i4");                  
   if (valueNode)
   {
      if (valueNode->FirstChild())
      {
         paramValue = valueNode->FirstChild()->Value();
         mResponseValue = new UtlInt(atoi(paramValue));
         result = true;
      }
      else
      {
         result = false;
      }
   }
   else
   {
      valueNode = subNode->FirstChild("int");                  
      if (valueNode)
      {
         if (valueNode->FirstChild())
         {
            paramValue = valueNode->FirstChild()->Value();
            mResponseValue = new UtlInt(atoi(paramValue));
            result = true;
         }
         else
         {
            result = false;
         }
      }
      else
      {
         valueNode = subNode->FirstChild("i8");                  
         if (valueNode)
         {
            if (valueNode->FirstChild())
            {
               paramValue = valueNode->FirstChild()->Value();
               mResponseValue = new UtlLongLongInt(strtoll(paramValue, 0, 0));
               result = true;
            }
            else
            {
               result = false;
            }
         }
         else
         {
            // boolean
            valueNode = subNode->FirstChild("boolean");                 
            if (valueNode)
            {
               if (valueNode->FirstChild())
               {
                  paramValue = valueNode->FirstChild()->Value();
                  mResponseValue = new UtlBool((atoi(paramValue)==1));
                  result = true;
               }
               else
               {
                  result = false;
               }
            }
            else
            {        
               // string
               valueNode = subNode->FirstChild("string");                  
               if (valueNode)
               {
                  if (valueNode->FirstChild())
                  {
                     paramValue = valueNode->FirstChild()->Value();
                     mResponseValue = new UtlString(paramValue);
                  }
                  else
                  {
                     mResponseValue = NULL;
                  }
                  result = true;
               }
               else
               {                  
                  // dateTime.iso8601
                  valueNode = subNode->FirstChild("dateTime.iso8601");                 
                  if (valueNode)
                  {
                     if (valueNode->FirstChild())
                     {
                        paramValue = valueNode->FirstChild()->Value();
                        //mResponseValue = new UtlDateTime(paramValue);
                        result = true;
                     }
                     else
                     {
                        result = false;
                     }
                  }
                  else
                  {                  
                     // struct
                     valueNode = subNode->FirstChild("struct");                  
                     if (valueNode)
                     {
                        UtlHashMap* map = new UtlHashMap();
                        if (parseStruct(valueNode, map))
                        {
                           mResponseValue = map;
                           result = true;
                        }
                     }
                     else
                     {
                        // array
                        valueNode = subNode->FirstChild("array");                  
                        if (valueNode)
                        {
                           UtlSList* list = new UtlSList();
                           if (parseArray(valueNode, list))
                           {
                              mResponseValue = list;
                              result = true;
                           }
                        }
                        else
                        {
                           // default as string
                           valueNode = subNode->FirstChild();                  
                           if (valueNode)
                           {
                              paramValue = valueNode->Value();
                              mResponseValue = new UtlString(paramValue);
                           }
                           else
                           {
                              mResponseValue = NULL;
                           }
                           
                           result = true;
                        }
                     }
                  }
               }
            }
         }
      }
   }
         
   return result;
}
   
bool XmlRpcResponse::parseStruct(TiXmlNode* subNode, UtlHashMap* members)
{
   bool result = false;

   // struct
   UtlString name;
   UtlString paramValue;
   TiXmlNode* memberValue;
   for (TiXmlNode* memberNode = subNode->FirstChild("member");
        memberNode; 
        memberNode = memberNode->NextSibling("member"))
   {
      TiXmlNode* memberName = memberNode->FirstChild("name");
      if (memberName)
      {
         if (memberName->FirstChild())
         {
            name = memberName->FirstChild()->Value();
         }
         else
         {
            result = false;
            break;
         }
         
         memberValue = memberNode->FirstChild("value");        
         if (memberValue)
         {
            // four-byte signed integer                         
            TiXmlNode* valueElement = memberValue->FirstChild("i4");
            if (valueElement)
            {
               if (valueElement->FirstChild())
               {
                  paramValue = valueElement->FirstChild()->Value();
                  members->insertKeyAndValue(new UtlString(name), new UtlInt(atoi(paramValue)));
                  result = true;
               }
               else
               {
                  result = false;
                  break;
               }
            }
            else
            {
               valueElement = memberValue->FirstChild("int");
               if (valueElement)
               {
                  if (valueElement->FirstChild())
                  {
                     paramValue = valueElement->FirstChild()->Value();
                     members->insertKeyAndValue(new UtlString(name), new UtlInt(atoi(paramValue)));
                     result = true;
                  }
                  else
                  {
                     result = false;
                     break;
                  }
               }
               else
               {
                  valueElement = memberValue->FirstChild("i8");
                  if (valueElement)
                  {
                     if (valueElement->FirstChild())
                     {
                        paramValue = valueElement->FirstChild()->Value();
                        members->insertKeyAndValue(new UtlString(name), new UtlLongLongInt(strtoll(paramValue, 0, 0)));
                        result = true;
                     }
                     else
                     {
                        result = false;
                        break;
                     }
                  }
                  else
                  {
                     valueElement = memberValue->FirstChild("boolean");
                     if (valueElement)
                     {
                        if (valueElement->FirstChild())
                        {
                           paramValue = valueElement->FirstChild()->Value();
                           members->insertKeyAndValue(new UtlString(name), new UtlBool((atoi(paramValue)==1)));
                           result = true;
                        }
                        else
                        {
                           result = false;
                           break;
                        }
                     }
                     else
                     {              
                        valueElement = memberValue->FirstChild("string");
                        if (valueElement)
                        {
                           if (valueElement->FirstChild())
                           {
                              paramValue = valueElement->FirstChild()->Value();
                              members->insertKeyAndValue(new UtlString(name), new UtlString(paramValue));
                           }
                           else
                           {
                              members->insertKeyAndValue(new UtlString(name), new UtlString());
                           }
                           
                           result = true;
                        }
                        else
                        {
                           valueElement = memberValue->FirstChild("dateTime.iso8601");
                           if (valueElement)
                           {
                              if (valueElement->FirstChild())
                              {
                                 paramValue = valueElement->FirstChild()->Value();
                                 members->insertKeyAndValue(new UtlString(name), new UtlString(paramValue));
                                 result = true;
                              }
                              else
                              {
                                 result = false;
                                 break;
                              }
                           }
                           else
                           {
                              valueElement = memberValue->FirstChild("struct");
                              if (valueElement)
                              {
                                 UtlHashMap* members = new UtlHashMap();
                                 if (parseStruct(valueElement, members))
                                 {
                                    members->insertKeyAndValue(new UtlString(name), members);
                                    result = true;
                                 }
                              }
                              else
                              {
                                 valueElement = memberValue->FirstChild("array");
                                 if (valueElement)
                                 {
                                    UtlSList* subArray = new UtlSList();
                                    if (parseArray(valueElement, subArray))
                                    {
                                       members->insertKeyAndValue(new UtlString(name), subArray);
                                       result = true;
                                    }
                                 }
                                 else
                                 {
                                    // default for string
                                    if (memberValue->FirstChild())
                                    {
                                       paramValue = memberValue->FirstChild()->Value();
                                       members->insertKeyAndValue(new UtlString(name), new UtlString(paramValue));
                                    }
                                    else
                                    {
                                       members->insertKeyAndValue(new UtlString(name), new UtlString());
                                    }
                                    
                                    result = true;
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
   }
   
   return result;   
}

bool XmlRpcResponse::parseArray(TiXmlNode* subNode, UtlSList* array)
{
   bool result = false;
   
   // array
   UtlString paramValue;
   TiXmlNode* dataNode = subNode->FirstChild("data");
   if (dataNode)
   {
      result = true; // an empty list is ok

      // see if there are values...
      for (TiXmlNode* valueNode = dataNode->FirstChild("value");
           result && valueNode; 
           valueNode = valueNode->NextSibling("value")
           )
      {
         // four-byte signed integer                         
         TiXmlNode* arrayElement;;
         if ((arrayElement = valueNode->FirstChild("i4")))
         {
            if (arrayElement->FirstChild())
            {
               paramValue = arrayElement->FirstChild()->Value();
               array->insert(new UtlInt(atoi(paramValue)));
            }
            else
            {
               result = false;
            }
         }
         else if ((arrayElement = valueNode->FirstChild("int")))
         {
            if (arrayElement->FirstChild())
            {
               paramValue = arrayElement->FirstChild()->Value();
               array->insert(new UtlInt(atoi(paramValue)));
            }
            else
            {
               result = false;
            }
         }
         else if ((arrayElement = valueNode->FirstChild("i8")))
         {
            if (arrayElement->FirstChild())
            {
               paramValue = arrayElement->FirstChild()->Value();
               array->insert(new UtlLongLongInt(strtoll(paramValue, 0, 0)));
            }
            else
            {
               result = false;
            }
         }
         else if ((arrayElement = valueNode->FirstChild("boolean")))
         {
            if (arrayElement->FirstChild())
            {
               paramValue = arrayElement->FirstChild()->Value();
               array->insert(new UtlBool((atoi(paramValue)==1)));
            }
            else
            {
               result = false;
            }
         }
         else if ((arrayElement = valueNode->FirstChild("string")))
         {
            if (arrayElement->FirstChild())
            {
               paramValue = arrayElement->FirstChild()->Value();
               array->insert(new UtlString(paramValue));
            }
            else
            {
               array->insert(new UtlString());
            }
         }
         else if ((arrayElement = valueNode->FirstChild("dateTime.iso8601")))
         {
            if (arrayElement->FirstChild())
            {
               paramValue = arrayElement->FirstChild()->Value();
               array->insert(new UtlString(paramValue));
            }
            else
            {
               result = false;
            }
         }
         else if ((arrayElement = valueNode->FirstChild("struct")))
         {
            UtlHashMap* members = new UtlHashMap();
            if (parseStruct(arrayElement, members))
            {
               array->insert(members);
            }
         }
         else if ((arrayElement = valueNode->FirstChild("array")))
         {
            UtlSList* subArray = new UtlSList();
            if (parseArray(arrayElement, subArray))
            {
               array->insert(subArray);
            }
         }
         else
         {
            // default for string
            if (valueNode->FirstChild())
            {
               paramValue = valueNode->FirstChild()->Value();
               array->insert(new UtlString(paramValue));
            }
            else
            {
               array->insert(new UtlString());
            }
         }
      } // end of for loop over values
   }

   return result;
}

void XmlRpcResponse::cleanUp(UtlContainable* value)
{
   if (value)
   {
      if (value->isInstanceOf(UtlHashMap::TYPE))
      {
         UtlHashMap* map = dynamic_cast<UtlHashMap*>(value);
         UtlHashMapIterator iterator(*map);
         UtlString* key;
         while ((key = dynamic_cast<UtlString*>(iterator())))
         {
            UtlContainable *pName;
            UtlContainable *member;
            pName = map->removeKeyAndValue(key, member);
            delete pName;
            cleanUp(member);
         }
      }
      else if (value->isInstanceOf(UtlSList::TYPE))
      {
         UtlSList* array = dynamic_cast<UtlSList*>(value);
         UtlContainable *element;
         while ((element = array->get()/* pop */))
         {
            cleanUp(element);
         }
      }

      delete value;
   }
}

/// Get the content of the response
XmlRpcBody* XmlRpcResponse::getBody()
{
   assert(mpResponseBody); // if false, no response was set
   
   return mpResponseBody;
}

/* ============================ FUNCTIONS ================================= */

