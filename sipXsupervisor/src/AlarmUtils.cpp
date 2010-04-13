//
//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "AlarmUtils.h"
#include "os/OsSysLog.h"
#include "utl/UtlSListIterator.h"
#include "utl/XmlContent.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

//=================  START OF FUNCTIONS FOR STRING LOCALIZATION =================

/// build message by substituting parameters for placeholders
void assembleMsg(const UtlString& formatStr,   //< input string with placeholders {0} etc
                 const UtlSList&  params,      //< list of UtlString parameters
                       UtlString& outMsg)      //< output formatted string
{
   UtlSListIterator paramListIterator(params);
   UtlString* pParam;
   int paramNum=0;
   const char* separator=" ";
   char placeHolder[255];

   outMsg = formatStr;
   while ( (pParam = dynamic_cast<UtlString*> (paramListIterator())) )
   {
      UtlString tempStr;
      XmlUnEscape(tempStr, *pParam);
      sprintf(placeHolder, "{%d}", paramNum);
      ssize_t pos = outMsg.index(placeHolder);
      if (pos != UTL_NOT_FOUND)
      {
         // replace placeholder with the n'th parameter.
         outMsg = outMsg.replace(pos, strlen(placeHolder), tempStr);
      }
      else
      {
         OsSysLog::add(FAC_ALARM, PRI_DEBUG, "placeholder not found for parameter %d; appending", paramNum);
         outMsg += separator;
         outMsg += tempStr;
      }
      paramNum++;
   }
}

/// build message by substituting a string parameter for a single placeholder
void assembleMsg(const UtlString& formatStr, //< input string containing one placeholder {0}
                 const UtlString& param,     //< string parameter
                       UtlString& outMsg)    //< formatted string
{
   const char* placeHolder = "{0}";
   const char* separator = " ";

   outMsg = formatStr;
   ssize_t pos = outMsg.index(placeHolder);
   if (pos != UTL_NOT_FOUND)
   {
      // replace placeholder with the parameter.
      outMsg = outMsg.replace(pos, strlen(placeHolder), param);
   }
   else
   {
      OsSysLog::add(FAC_ALARM, PRI_DEBUG, "placeholder not found for parameter; appending");
      outMsg += separator;
      outMsg += param;
   }

}


//=====================  START OF FUNCTIONS FOR XML PARSING =====================

int getIntAttribute(TiXmlElement* element, const char* attrStr, int defValue)
{
   int value=defValue;
   if (element)
   {
      const char* lTemp = element->Attribute(attrStr);
      if (lTemp)
      {
         value = atoi(lTemp);
      }
   }
   return value;
}

bool getBoolAttribute(TiXmlElement* element, const char* attrStr, bool defValue)
{
   bool value=defValue;
   if (element)
   {
      const char* lTemp = element->Attribute(attrStr);
      if (lTemp)
      {
         value = !strcmp(lTemp, "true");
      }
   }
   return value;
}

