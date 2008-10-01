// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <utl/UtlRegex.h>
#include <net/SipMessage.h>
#include "os/OsDateTime.h"
#include "os/OsFS.h"
#include "sipdb/ResultSet.h"
#include "registry/RedirectPlugin.h"

// DEFINES
#define UNINITIALIZED_WARNING_CODE  (-1)

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

const char* RedirectPlugin::Prefix  = "SIP_REDIRECT";
const char* RedirectPlugin::Factory = "getRedirectPlugin";

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

// Null default destructor.
RedirectPlugin::~RedirectPlugin()
{
}

// Null default cancel() implementation.
void RedirectPlugin::cancel(RequestSeqNo request)
{
}

// Null default readConfig() implementation
void 
RedirectPlugin::readConfig(OsConfigDb& configDb)
{
}

void
RedirectPlugin::addContact(SipMessage& response,
                          const UtlString& requestString,
                          const Url& contact,
                          const char* label)
{
   // Get the number of contacts already present.
   int numContactsInHeader =
      response.getCountHeaderFields(SIP_CONTACT_FIELD);

   // Add the contact field to the response at the end.
   // Need to keep this UtlString allocated till the end of this function.
   // The semantics of the Contact: header have the additional restriction
   // that if the URI contains a '?', it must be enclosed in <...> (sec. 20).
   // But beware that the BNF in sec. 25.1 does not require this.
   // Scott has changed Url::toString to always add <...> if there are header
   // parameters in the URI.
   UtlString contactUtlString = contact.toString();
   const char* contactString = contactUtlString.data();
   response.setContactField(contactString, numContactsInHeader);

   OsSysLog::add(FAC_SIP, PRI_INFO,
                 "RedirectPlugin::addContact Redirector '%s' maps '%s' to '%s'",
                 label, requestString.data(), contactString);
}

void
RedirectPlugin::removeAllContacts(SipMessage& response)
{
   // Get the number of contacts already present.
   int numContactsInHeader =
      response.getCountHeaderFields(SIP_CONTACT_FIELD);

   OsSysLog::add(FAC_SIP, PRI_INFO,
                 "RedirectPlugin::removeAllContacts Removing %d contacts",
                 numContactsInHeader);

   for (int i = numContactsInHeader - 1; i >= 0; i--)
   {
      response.removeHeader(SIP_CONTACT_FIELD, i);
   }
}

SipRedirectorPrivateStorage::~SipRedirectorPrivateStorage()
{
}

ErrorDescriptor::ErrorDescriptor() :
   mStatusCode             ( SIP_FORBIDDEN_CODE ),
   mReasonPhrase           ( SIP_FORBIDDEN_TEXT ),
   mWarningCode            ( UNINITIALIZED_WARNING_CODE ),
   mAppendRequestToResponse( false )
{   
}

ErrorDescriptor::~ErrorDescriptor()
{
   mOptionalFieldsValues.destroyAll();
}
   
bool ErrorDescriptor::setStatusLineData( const int statusCode, const UtlString& reasonPhrase )
{
   bool result = false;
   if( statusCode >= SIP_4XX_CLASS_CODE && statusCode < SIP_7XX_CLASS_CODE )
   {
      mStatusCode   = statusCode;
      mReasonPhrase = reasonPhrase;
      result = true;
   }
   return result;
}

bool ErrorDescriptor::setWarningData( const int warningCode, const UtlString& warningText )
{
   bool result = false;
   if( warningCode >= SIP_WARN_INCOMPAT_PROTO_CODE && warningCode <= SIP_WARN_MISC_CODE )
   {
      mWarningCode = warningCode;
      mWarningText = warningText;
      result = true;
   }
   return result;
}

void ErrorDescriptor::setRetryAfterFieldValue( const UtlString& fieldValue )
{
   setOptionalFieldValue( SIP_RETRY_AFTER_FIELD, fieldValue );
}

void ErrorDescriptor::setRequireFieldValue( const UtlString& fieldValue )
{
   setOptionalFieldValue( SIP_REQUIRE_FIELD, fieldValue );
}

void ErrorDescriptor::setUnsupportedFieldValue( const UtlString& fieldValue )
{
   setOptionalFieldValue( SIP_UNSUPPORTED_FIELD, fieldValue );
}

void ErrorDescriptor::setAllowFieldValue( const UtlString& fieldValue )
{
   setOptionalFieldValue( SIP_ALLOW_FIELD, fieldValue );
}

void ErrorDescriptor::setAcceptFieldValue( const UtlString& fieldValue )
{
   setOptionalFieldValue( SIP_ACCEPT_FIELD, fieldValue );
}

void ErrorDescriptor::setAcceptEncodingFieldValue( const UtlString& fieldValue )
{
   setOptionalFieldValue( SIP_ACCEPT_ENCODING_FIELD, fieldValue );
}

void ErrorDescriptor::setAcceptLanguageFieldValue( const UtlString& fieldValue )
{
   setOptionalFieldValue( SIP_ACCEPT_LANGUAGE_FIELD, fieldValue );
}

void ErrorDescriptor::setOptionalFieldValue( const UtlString& fieldName, const UtlString& fieldValue )
{
   mOptionalFieldsValues.destroy( &fieldName );
   mOptionalFieldsValues.insertKeyAndValue( new UtlString( fieldName ), new UtlString( fieldValue ) );
}

void ErrorDescriptor::appendRequestToResponse( void )
{
   mAppendRequestToResponse = true;
}

void ErrorDescriptor::dontAppendRequestToResponse( void )
{
   mAppendRequestToResponse = false;
}

void ErrorDescriptor::getStatusLineData( int& statusCode, UtlString& reasonPhrase ) const
{
   statusCode   = mStatusCode;
   reasonPhrase = mReasonPhrase;
}

bool ErrorDescriptor::getWarningData( int& warningCode, UtlString& warningText ) const
{
   bool result = isWarningDataSet();  
   if( result )
   {
      warningCode = mWarningCode;
      warningText = mWarningText;
   }
   return result;
}

bool ErrorDescriptor::getRetryAfterFieldValue( UtlString& fieldValue ) const
{
   return getOptinalFieldValue( SIP_RETRY_AFTER_FIELD, fieldValue );
}

bool ErrorDescriptor::getRequireFieldValue( UtlString& fieldValue ) const
{
   return getOptinalFieldValue( SIP_REQUIRE_FIELD, fieldValue );
}

bool ErrorDescriptor::getUnsupportedFieldValue( UtlString& fieldValue ) const
{
   return getOptinalFieldValue( SIP_UNSUPPORTED_FIELD, fieldValue );
}

bool ErrorDescriptor::getAllowFieldValue( UtlString& fieldValue ) const
{
   return getOptinalFieldValue( SIP_ALLOW_FIELD, fieldValue );
}

bool ErrorDescriptor::getAcceptFieldValue( UtlString& fieldValue ) const
{
   return getOptinalFieldValue( SIP_ACCEPT_FIELD, fieldValue );
}

bool ErrorDescriptor::getAcceptEncodingFieldValue( UtlString& fieldValue ) const
{
   return getOptinalFieldValue( SIP_ACCEPT_ENCODING_FIELD, fieldValue );
}

bool ErrorDescriptor::getAcceptLanguageFieldValue( UtlString& fieldValue ) const
{
   return getOptinalFieldValue( SIP_ACCEPT_LANGUAGE_FIELD, fieldValue );
}

bool ErrorDescriptor::getOptinalFieldValue( const UtlString& fieldName, UtlString& fieldValue ) const
{
   bool result = false;
   UtlString* pReturnedValue;
   
   pReturnedValue = dynamic_cast<UtlString*>(mOptionalFieldsValues.findValue( &fieldName ) );
   if( pReturnedValue )
   {
      fieldValue = *pReturnedValue;
      result = true;
   }
   return result;
}

bool ErrorDescriptor::isWarningDataSet( void ) const
{
   return mWarningCode != UNINITIALIZED_WARNING_CODE;
}

bool ErrorDescriptor::shouldRequestBeAppendedToResponse( void ) const
{
   return mAppendRequestToResponse;
}
