//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _XMLRPCBODY_H_
#define _XMLRPCBODY_H_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <xmlparser/tinyxml.h>
#include <net/HttpBody.h>
#include <utl/UtlHashMap.h>

// DEFINES
#define CONTENT_TYPE_TEXT_XML "text/xml"

#define BEGIN_METHOD_CALL "<methodCall>\n"
#define END_METHOD_CALL "</methodCall>\n"

#define BEGIN_METHOD_NAME "<methodName>"
#define END_METHOD_NAME "</methodName>\n"

#define BEGIN_PARAMS "<params>\n"
#define END_PARAMS "</params>\n"

#define BEGIN_PARAM "<param>\n"
#define END_PARAM "</param>\n"

#define BEGIN_I4 "<value><i4>"
#define END_I4 "</i4></value>\n"

#define BEGIN_INT "<value><int>"
#define END_INT "</int></value>\n"

#define BEGIN_I8 "<value><i8>"
#define END_I8 "</i8></value>\n"

#define BEGIN_BOOLEAN "<value><boolean>"
#define END_BOOLEAN "</boolean></value>\n"

#define BEGIN_STRING "<value><string>"
#define END_STRING "</string></value>\n"

#define BEGIN_TIME "<value><dateTime.iso8601>"
#define END_TIME "</dataTime.iso8601></value>\n"

#define BEGIN_ARRAY "<value><array><data>\n"
#define END_ARRAY "</data></array></value>\n"

#define BEGIN_STRUCT "<value><struct>\n"
#define END_STRUCT "</struct></value>\n"

#define BEGIN_MEMBER "<member>\n"
#define END_MEMBER "</member>\n"

#define BEGIN_NAME "<name>"
#define END_NAME "</name>"

#define BEGIN_RESPONSE "<methodResponse><!-- "
#define END_NAME_COMMENT " -->\n"
#define END_RESPONSE "</methodResponse>\n"

#define BEGIN_FAULT "<fault>\n"
#define END_FAULT "</fault>\n"

#define FAULT_CODE "<name>faultCode</name>"
#define FAULT_STRING "<name>faultString</name>"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * This class contains all the contents presented in a XML-RPC body. This class
 * has the methods to construct a XML-RPC body. It is only used by XmlRpcRequest
 * and XmlRpcResponse classes.
 *
 */
class XmlRpcBody : public HttpBody
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   /// Construct an empty body of a XML-RPC
   XmlRpcBody();

   /// Destructor
   virtual ~XmlRpcBody();

   virtual XmlRpcBody* copy() const;

   /// Append the string to the body
   void append(const char* string);

   /// Get the string length of this object
   virtual ssize_t getLength() const;

   /// Get the serialized char representation of this body.
   virtual void getBytes(const char** bytes, ///< body content output, null terminated
                         ssize_t* length ///< # bytes written (not including the null terminator
                         ) const;

   /// Get the serialized string representation of this body.
   virtual void getBytes(UtlString* bytes, ///< body content output, null terminated
                         ssize_t* length ///< # bytes written (not including the null terminator
                         ) const;

   /// Add a value to the XML-RPC content
   bool addValue(const UtlContainable* value);

   /// Add an array to the XML-RPC content
   bool addArray(UtlSList* array); ///< array of elements

   /// Add a struct to the XML-RPC content
   bool addStruct(UtlHashMap* members); ///< struct of members

   static const size_t MAX_LOG; ///< maximum size of an XML-RPC body written to the log

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

   friend class XmlRpcDispatch;
   friend class XmlRpcResponse;
   friend class XmlRpcTest;

   /// Parse a value in the XML-RPC request
   static
      UtlContainable* parseValue(TiXmlNode* valueNode, ///< pointer to the <value> node
                                 int nestDepth,        ///< current level of recursion
                                 UtlString& errorTxt   ///< explanation of parse error if any
                                 );
   ///< @returns NULL and sets errorTxt if there was a parse error

   /// Parse an array in the XML-RPC request
   static
      UtlSList* parseArray(TiXmlNode* arrayNode, ///< pointer to the <array> node
                           int nestDepth,        ///< current level of recursion
                           UtlString& errorTxt   ///< explanation of parse error if any
                           );
   ///< @returns NULL and sets errorTxt if there was a parse error

   /// Parse a struct in the XML-RPC request
   static
      UtlHashMap* parseStruct(TiXmlNode* structNode, ///< pointer to the <struct> node
                              int nestDepth,         ///< current level of recursion
                              UtlString& errorTxt    ///< explanation of parse error if any
                              );
   ///< @returns NULL and sets errorTxt if there was a parse error

   /// Delete the value and any memory it contains
   static
      void deallocateValue(UtlContainable*& value);

   /// Clean up the memory in a struct, but not the UtlHashMap object itself
   static
      void deallocateContainedValues(UtlHashMap* members);

   /// Clean up the memory in an array, but not the UtlSList object itself
   static
      void deallocateContainedValues(UtlSList* array);


/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   /// Disabled copy constructor
   XmlRpcBody(const XmlRpcBody& rXmlRpcBody);

   /// Disabled assignment operator
   XmlRpcBody& operator=(const XmlRpcBody& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _XMLRPCBODY_H_
