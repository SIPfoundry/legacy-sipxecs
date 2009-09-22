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
      XmlRpcBody::deallocateValue(mResponseValue);
   }

   if (mpResponseBody)
   {
      delete mpResponseBody;
      mpResponseBody = NULL;
   }
}

bool XmlRpcResponse::parseXmlRpcResponse(UtlString& responseContent)
{
   bool isFault = true;

   // assume the worst
   mFaultCode   = IllFormedContents;
   mFaultString = ILL_FORMED_CONTENTS_FAULT_STRING;

   // Parse the XML-RPC response
   TiXmlDocument doc("XmlRpcResponse.xml"); // document name is required but not used

   doc.Parse(responseContent);
   if (!doc.Error())
   {
      TiXmlNode* rootNode = doc.FirstChild ("methodResponse");
      if (rootNode != NULL)
      {
         // Positive response (example)
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
            bool parseOk = false;
            UtlString parseErrorMsg;

            TiXmlNode* paramNode = paramsNode->FirstChild("param");
            if (paramNode)
            {
               TiXmlNode* subNode = paramNode->FirstChild("value");
               if (subNode)
               {
                  // Clean up the memory in mResponseValue
                  if (mResponseValue)
                  {
                     XmlRpcBody::deallocateValue(mResponseValue);
                  }

                  mResponseValue = XmlRpcBody::parseValue(subNode,0,parseErrorMsg);
                  parseOk = (mResponseValue != NULL);
               }
               else
               {
                  OsSysLog::add(FAC_XMLRPC, PRI_ERR,
                                "XmlRpcResponse::parseXmlRpcResponse"
                                " Invalid response - "
                                "no value element in param");
               }
            }
            else
            {
               OsSysLog::add(FAC_XMLRPC, PRI_ERR,
                             "XmlRpcResponse::parseXmlRpcResponse"
                             " Invalid response - "
                             "no param element in params");
            }

            if (parseOk)
            {
               isFault = false;
            }
            else
            {
               OsSysLog::add(FAC_XMLRPC, PRI_ERR,
                             "XmlRpcResponse::parseXmlRpcResponse"
                             " value parsing error: %s",
                             parseErrorMsg.data());
            }

         } // end of params (success) parsing
         else
         {
            // Fault response
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
                        if (memberNode->FirstChild("name")
                            && (memberNode->FirstChild("name"))->FirstChild())
                        {
                           nameValue = (memberNode->FirstChild("name"))->FirstChild()->Value();

                           if (nameValue.compareTo("faultCode") == 0)
                           {
                              if (memberNode->FirstChild("value"))
                              {
                                 TiXmlNode* valueNode =
                                    (memberNode->FirstChild("value"))->FirstChild("int");
                                 if (valueNode && valueNode->FirstChild())
                                 {
                                    mFaultCode = atoi(valueNode->FirstChild()->Value());
                                 }
                                 else
                                 {
                                    valueNode =
                                       (memberNode->FirstChild("value"))->FirstChild("i4");
                                    if (valueNode && valueNode->FirstChild())
                                    {
                                       mFaultCode = atoi(valueNode->FirstChild()->Value());
                                    }
                                    else
                                    {
                                       OsSysLog::add(FAC_XMLRPC, PRI_ERR,
                                                     "XmlRpcResponse::parseXmlRpcResponse"
                                                     " Invalid response - "
                                                     "no int or i4 element in faultCode value");
                                    }
                                 }
                              }
                              else
                              {
                                 OsSysLog::add(FAC_XMLRPC, PRI_ERR,
                                               "XmlRpcResponse::parseXmlRpcResponse"
                                               " Invalid response - "
                                               "no value element in faultCode member");
                              }
                           }
                           else if (nameValue.compareTo("faultString") == 0)
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
                              else
                              {
                                 OsSysLog::add(FAC_XMLRPC, PRI_ERR,
                                               "XmlRpcResponse::parseXmlRpcResponse"
                                               " Invalid response - "
                                               "no value element in faultString member");
                              }
                           }
                           else
                           {
                              // unrecognized element - ignore it
                           }
                        }
                        else
                        {
                           OsSysLog::add(FAC_XMLRPC, PRI_ERR,
                                         "XmlRpcResponse::parseXmlRpcResponse"
                                         " Invalid response - no name element in fault member");
                        }
                     } // loop over members
                  }
                  else
                  {
                     OsSysLog::add(FAC_XMLRPC, PRI_ERR,
                                   "XmlRpcResponse::parseXmlRpcResponse"
                                   " Invalid response - no struct element in fault value");
                  }
               }
               else
               {
                  OsSysLog::add(FAC_XMLRPC, PRI_ERR,
                                "XmlRpcResponse::parseXmlRpcResponse"
                                " Invalid response - no value element in fault");
               }
            }
            else
            {
               OsSysLog::add(FAC_XMLRPC, PRI_ERR,
                             "XmlRpcResponse::parseXmlRpcResponse"
                             " Invalid response - no params or fault element");
            }
         } // end of fault parsing
      }
      else
      {
         OsSysLog::add(FAC_XMLRPC, PRI_ERR,
                       "XmlRpcResponse::parseXmlRpcResponse"
                       " Invalid response - no methodResponse element");
      }
   }
   else
   {
      OsSysLog::add(FAC_XMLRPC, PRI_ERR,
                    "XmlRpcResponse::parseXmlRpcResponse"
                    " ill formatted xml contents in %s. Parsing error = %s",
                     responseContent.data(), doc.ErrorDesc());
   }

   return !isFault;
}


/// Set the method name (for logging and comment in return)
void XmlRpcResponse::setMethod(const UtlString& methodName)
{
   mMethod = methodName;
}

bool XmlRpcResponse::setResponse(UtlContainable* value)
{
   bool result = false;
   if (mpResponseBody != NULL)    // response body should only be created once
   {
      OsSysLog::add(FAC_XMLRPC, PRI_CRIT,
                    "XmlRpcResponse::setResponse - body already set");
      assert(false);
   }

   // Start to construct the XML-RPC body
   mpResponseBody = new XmlRpcBody();
   if (mpResponseBody != NULL)
   {
      mpResponseBody->append(BEGIN_RESPONSE);   // includes comment leader...
      mpResponseBody->append(mMethod);          // method name in the comment
      mpResponseBody->append(END_NAME_COMMENT); // and close the comment

      mpResponseBody->append(BEGIN_PARAMS BEGIN_PARAM);

      result = mpResponseBody->addValue(value);

      mpResponseBody->append(END_PARAM END_PARAMS END_RESPONSE);

      OsSysLog::add(FAC_XMLRPC, PRI_DEBUG,
                    "XmlRpcResponse::setResponse called");
   }
   else
   {
      OsSysLog::add(FAC_XMLRPC, PRI_CRIT,
                    "XmlRpcResponse::setResponse body allocation failed");
   }

   return result;
}


bool XmlRpcResponse::setFault(int faultCode, const char* faultString)
{
   bool result = true;
   mFaultCode = faultCode;
   mFaultString = faultString;

   // Start to construct the XML-RPC body for fault response
   if (mpResponseBody != NULL)    // response body should only be created once
   {
      OsSysLog::add(FAC_XMLRPC, PRI_CRIT,
                    "XmlRpcResponse::setResponse - body already set");
      assert(false);
   }

   mpResponseBody = new XmlRpcBody();
   if (mpResponseBody)
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

      mpResponseBody->append(BEGIN_RESPONSE);   // includes comment leader...
      mpResponseBody->append(mMethod);          // method name in the comment
      mpResponseBody->append(END_NAME_COMMENT); // and close the comment

      mpResponseBody->append(BEGIN_FAULT BEGIN_STRUCT BEGIN_MEMBER FAULT_CODE BEGIN_INT);

      char temp[10];
      sprintf(temp, "%d", mFaultCode);
      mpResponseBody->append(temp);

      mpResponseBody->append(END_INT END_MEMBER BEGIN_MEMBER FAULT_STRING BEGIN_STRING);
      mpResponseBody->append(mFaultString);
      mpResponseBody->append(END_STRING END_MEMBER END_STRUCT END_FAULT END_RESPONSE);

      OsSysLog::add(FAC_XMLRPC, PRI_DEBUG,
                    "mpResponseBody::setFault %d %s", mFaultCode, mFaultString.data());
   }
   else
   {
      OsSysLog::add(FAC_XMLRPC, PRI_CRIT,
                    "mpResponseBody::setFault body allocation failed");
   }

   return result;
}


bool XmlRpcResponse::getResponse(UtlContainable*& value)
{
   value = mResponseValue;
   return value != NULL;
}


void XmlRpcResponse::getFault(int* faultCode, UtlString& faultString)
{
   *faultCode = mFaultCode;
   faultString = mFaultString;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/// Get the content of the response
XmlRpcBody* XmlRpcResponse::getBody()
{
   if (!mpResponseBody)
   {
      OsSysLog::add(FAC_XMLRPC,PRI_CRIT,"XmlRpcResponse::getBody no body set");
      assert(false);
   }

   return mpResponseBody;
}

/* ============================ FUNCTIONS ================================= */
