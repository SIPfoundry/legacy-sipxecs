//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <utl/UtlDListIterator.h>
#include <net/SipRequestContext.h>
#include <net/NameValueTokenizer.h>
#include <net/NameValuePair.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const char* SipRequestContext::sAUTH_USER = "AUTH_USER";
const char* SipRequestContext::sAUTH_REALM = "AUTH_REALM";
const char* SipRequestContext::sREQUEST_METHOD = "REQUEST_METHOD";
const char* SipRequestContext::sSERVER_DOMAIN = "SERVER_DOMAIN";

/* //////////////////////////// PUBLIC //////////////////////////////////// */
/* ============================ CREATORS ================================== */
// Constructor
SipRequestContext::SipRequestContext(const char* requestMethod)
{
    if(requestMethod)
    {
        addVariable(sREQUEST_METHOD, requestMethod);
    }
}

// Copy constructor
SipRequestContext::SipRequestContext(const SipRequestContext& rSipRequestContext)
{
   // delete the old values in the UtlDList
   if(!mVariableList.isEmpty())
   {
      mVariableList.destroyAll();
   }

   //copy mVariableList memebers individually
	UtlDListIterator iterator((UtlDList&)rSipRequestContext.mVariableList);
	NameValuePair* nameValuePair = NULL;
   UtlString value;
   UtlString name;
   int index = 0;
   do
   {
      nameValuePair = (NameValuePair*)iterator();
      if(nameValuePair)
      {
         name.append(*nameValuePair);
         value.append(nameValuePair->getValue());
         NameValuePair* newNvPair = new NameValuePair(name, value);
         mVariableList.insertAt(index, newNvPair);
         index ++;
         name.remove(0);
         value.remove(0);
      }
   }
   while (nameValuePair != NULL);
}

// Destructor
SipRequestContext::~SipRequestContext()
{
	mVariableList.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipRequestContext&
SipRequestContext::operator=(const SipRequestContext& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;
   else
   {
      // delete the old values in the UtlDList
      if(!mVariableList.isEmpty())
      {
         mVariableList.destroyAll();
      }
      //copy mVariableList members individually
	   UtlDListIterator iterator((UtlDList&)rhs.mVariableList);
	   NameValuePair* nameValuePair = NULL;
      UtlString value;
      UtlString name;

      int index = 0;
      do
      {
         nameValuePair = (NameValuePair*)iterator();
         if(nameValuePair)
         {
            name.append(*nameValuePair);
            value.append(nameValuePair->getValue());
            NameValuePair* newNvPair = new NameValuePair(name, value);
            mVariableList.insertAt(index, newNvPair);
            index ++;
            value.remove(0);
            name.remove(0);
         }
      }
      while (nameValuePair != NULL);
   }
   return *this;
}

/* ============================ ACCESSORS ================================= */


UtlBoolean SipRequestContext::getVariable(const char* name,
                                             UtlString& value,
                                             int occurance) const
{
    UtlDListIterator iterator((UtlDList&)mVariableList);
    NameValuePair* nameValuePair = NULL;
    int fieldIndex = 0;
    UtlString upperCaseName;
    UtlBoolean foundName = FALSE;

    value.remove(0);

    if(name)
    {
        upperCaseName.append(name);
        upperCaseName.toUpper();
    }
    NameValuePair matchName(upperCaseName);

    // For each name value:
    while(fieldIndex <= occurance)
    {
        // Go to the next header field
        nameValuePair = (NameValuePair*) iterator.findNext(&matchName);

        if(!nameValuePair || fieldIndex == occurance)
        {
            break;
        }
        fieldIndex++;
    }

    if(fieldIndex == occurance && nameValuePair)
    {
        value.append(nameValuePair->getValue());
        foundName = TRUE;
    }

    upperCaseName.remove(0);
    return(foundName);
}

int SipRequestContext::toString(UtlString& dumpString)
{
   int count = 0;
   UtlString name;
   UtlString value;
   dumpString.remove(0);

   while(getVariable(count, name, value))
   {
      dumpString.append(name);
      dumpString.append(": ");
      dumpString.append(value);
      dumpString.append("\n");
   }

   return(count);
}

void SipRequestContext::addVariable(const char* name,
                                    const char* value)
{
   NameValuePair* newNvPair = new NameValuePair(name, value);
   mVariableList.append(newNvPair);
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

UtlBoolean SipRequestContext::getVariable(int index, UtlString& name, UtlString& value) const
{
    NameValuePair* nameValuePair = NULL;

    if((int)(mVariableList.entries()) > index && index >= 0)
    {
        nameValuePair = (NameValuePair*)mVariableList.at(index);
        if(nameValuePair)
        {
            name = *nameValuePair;
            value.remove(0);
            value.append(nameValuePair->getValue());
        }
        else
        {
            name.remove(0);
            value.remove(0);
        }
    }

    return(nameValuePair != NULL);
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */
