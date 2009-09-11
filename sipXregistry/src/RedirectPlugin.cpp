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
#include "registry/SipRedirectServer.h"

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

void RedirectPlugin::observe(
   const SipMessage& message,
   const UtlString& requestString,
   const Url& requestUri,
   const UtlString& method,
   const ContactList& contactList,
   RequestSeqNo requestSeqNo,
   int redirectorNo )
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "RedirectPlugin::observe() [%s] called for %s",
                             name().data(),
                             requestString.data() );
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

void RedirectPlugin::resumeRedirection(RequestSeqNo request,
                                       int redirector)
{
   SipRedirectServer::getInstance()->resumeRequest(request, redirector);
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
   else
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT, "ErrorDescriptor::setStatusLineData(): redirector supplied invalid status code: %d", statusCode );
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
   else
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT, "ErrorDescriptor::setWarningData(): redirector supplied invalid warning code: %d", warningCode );
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

ContactList::ContactList( const UtlString& requestString ) :
   mRequestString( requestString ),
   mbListWasModified( false )
{
}

bool ContactList::add( const Url& contactUrl, const RedirectPlugin& plugin )
{
   return add( contactUrl.toString(), plugin );
}

bool ContactList::add( const UtlString& contact, const RedirectPlugin& plugin )
{
   mbListWasModified = true;
   mContactList.push_back( contact );
   OsSysLog::add(FAC_SIP, PRI_NOTICE, "ContactList::add(): %s added contact for '%s':\n"
                             "   '%s' (contact index %d)",
                             plugin.name().data(),
                             mRequestString.data(),
                             contact.data(),
                             mContactList.size() - 1 );
   return true;
}

bool ContactList::set( size_t index, const Url& contactUrl, const RedirectPlugin& plugin )
{
   return set( index, contactUrl.toString(), plugin );
}

bool ContactList::set( size_t index, const UtlString& contact, const RedirectPlugin& plugin )
{
   bool success = false;
   if( index < mContactList.size() )
   {
      mbListWasModified = true;
      success = true;

      OsSysLog::add(FAC_SIP, PRI_NOTICE, "ContactList::set(): %s modified contact index %d for '%s':\n"
                                "   was:    '%s'\n"
                                "   now is: '%s'",
                                plugin.name().data(),
                                index,
                                mRequestString.data(),
                                mContactList[ index ].data(),
                                contact.data() );
      mContactList[ index ] = contact;
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_ERR, "ContactList::set(): %s failed to set contact index %d - list only has %d elements",
                                plugin.name().data(),
                                index,
                                mContactList.size() );
   }
   return success;
}

bool ContactList::get( size_t index, UtlString& contact ) const
{
   bool success = false;
   if( index < mContactList.size() )
   {
      contact = mContactList[ index ];
      success = true;
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "ContactList::get(): plugin failed to get contact index %d - list only has %d elements",
                                index,
                                mContactList.size() );
   }
   return success;
}

bool ContactList::get( size_t index, Url& contactUrl ) const
{
   UtlString contactAsString;
   bool success = get( index, contactAsString );
   if( success )
   {
      contactUrl.fromString( contactAsString );
   }
   return success;
}

bool ContactList::remove( size_t index, const RedirectPlugin& plugin )
{
   bool success = false;
   if( index < mContactList.size() )
   {
      success = true;
      mbListWasModified = true;

      OsSysLog::add(FAC_SIP, PRI_NOTICE, "ContactList::remove(): %s removed contact index %d  for '%s':\n"
                                        "   was:    '%s'",
                                plugin.name().data(),
                                index,
                                mRequestString.data(),
                                mContactList[ index ].data() );

      mContactList.erase( mContactList.begin() + index );
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_ERR, "ContactList::remove(): %s failed to remove contact index %d - list only has %d elements",
                                plugin.name().data(),
                                index,
                                mContactList.size() );
   }
   return success;
}

bool ContactList::removeAll( const RedirectPlugin& plugin )
{
   OsSysLog::add(FAC_SIP, PRI_NOTICE, "ContactList::removeAll(): %s removed %d contacts for '%s'",
                             plugin.name().data(),
                             mContactList.size(),
                             mRequestString.data() );

   mbListWasModified = true;
   mContactList.clear();
   return true;
}

void ContactList::touch( const RedirectPlugin& plugin )
{
   OsSysLog::add(FAC_SIP, PRI_NOTICE, "ContactList::touch(): list touched by %s",
                             plugin.name().data() );
   mbListWasModified = true;
}

size_t ContactList::entries( void ) const
{
   return mContactList.size();
}

void ContactList::resetWasModifiedFlag( void )
{
   mbListWasModified = false;
}

bool ContactList::wasListModified( void ) const
{
   return mbListWasModified;
}
