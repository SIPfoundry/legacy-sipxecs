// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "os/OsSysLog.h"
#include "utl/UtlRegex.h"
#include "utl/UtlSortedListIterator.h"
#include "net/NameValuePair.h"
#include "net/NetBase64Codec.h"
#include "net/NetMd5Codec.h"

// APPLICATION INCLUDES
#include "RouteState.h"

// CONSTANTS
const char AUTHORIZED_DIALOG_PARAM[] = "auth";

/* ################################################################
 * State Token Syntax
 *
 * When carried as a route url parameter, the RouteState is encoded
 * as follows:
 *    <stateToken>   ::= <nvpairs> "!" <signature>
 *    <nvpairs>      ::= <namevalue> [ "." <namevalue> ]*
 *    <namevalue>    ::= <name> "~" <encodedvalue>
 *    <name>         ::= <instanceName> "*" <parameterName>
 *    <encodedvalue> ::= token_esc( base64( <value> ) )
 *    <signature>    ::= md5(<secret><callid><nvpairs>)
 *
 *    base64(x)
 *       is the NetBase64Codec encoding, which uses the
 *       following alphabet to encode each 6 bit sequence:
 *          ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=
 *
 *    token_esc(x)
 *       makes the following translations:
 *          "+" -> "'" because + becomes space in Url escaping
 *                     so not using it may help avoid tickling
 *                     Url parsing bugs.
 *          "=" -> "`" because = is not allowed in a SIP token
 *
 *    md5(x)
 *       is the NetMd5Codec encoding
 * ################################################################ */

// CONSTANTS
#define NAME  "(?i:[*a-z0-9_-]{2,})"
#define VALUE "(?i:[a-z0-9'/`]*)"

#define NAME_PART_SEP "*"
#define NV_SEP "~"
#define NV_LIST_SEP "."
#define SIG_SEP "!"

const RegEx StateAndSignature("^" "(" NAME NV_SEP VALUE "(?:" NV_LIST_SEP NAME NV_SEP VALUE ")*" ")"
                              SIG_SEP "(" "[0-9a-fA-F]{32}" ")" "$");

const RegEx NameValue("(" NAME ")" NV_SEP "(" VALUE ")" "[" NV_LIST_SEP SIG_SEP "]" );

const char* RouteState::UrlParameterName = "sipXecs-rs";

// STATIC VARIABLES
UtlString   RouteState::mSignatureSecret;

class RouteParameterName : public UtlString
{
private:
   static const RegEx ParameterNamePattern;

public:
   RouteParameterName(const char* pluginInstance,
                      const char* parameterName
                      )
      {
         RegEx valid(ParameterNamePattern);
         
         if (   valid.Search(pluginInstance)
             && valid.Search(parameterName)
             )
         {
            // encode 
            append(pluginInstance);
            append(NAME_PART_SEP); 
            append(parameterName);
         }
         else
         {
            OsSysLog::add(FAC_SIP,PRI_CRIT,
                          "RouteState RouteParameterName invalid name\n"
                          "instance '%s' parameter '%s'",
                          pluginInstance?pluginInstance:"(null)", 
                          parameterName?parameterName:"(null)"
                          );
         }
      };

   bool isValid() const
      {
         return ! isNull();
      }
};

const RegEx RouteParameterName::ParameterNamePattern("^[a-z0-9_-]*$", PCRE_CASELESS);

/// Decode the route state for a received message.
RouteState::RouteState(const SipMessage& message,      ///< normalized incoming request
                       const UtlSList&   removedRoutes,///< routes removed by normalizeProxyRoutes
                       const UtlString   routeHostPort  ///< hostport value to use when adding Record-Route
                       ) :
   mRouteHostPort(routeHostPort),
   mMayBeMutable(false),
   mModified(false),
   mFoundRouteState(false),
   mAddCopyRequested( false )
{
   mRecordRouteIndices.clear();
   
   message.getCallIdField(&mCallId);

   Url fromUrl;
   message.getFromUrl(fromUrl);
   fromUrl.getFieldParameter("tag", mFromTag);

   mFoundRouteState = false;
   UtlSListIterator routes(removedRoutes);
   UtlString* route;
   while (!mFoundRouteState && (route = dynamic_cast<UtlString*>(routes())))
   {
      if (route->contains(UrlParameterName)) // quick test before doing a full url parse
      {
         Url routeUrl(route->data()); // parse the route url so we can read the parameter value
         UtlString routeStateValue;
         if (routeUrl.getUrlParameter(UrlParameterName, routeStateValue))
         {
            /*
             * Got some value for the parameter we expect;
             * parse it and check the signature - if this returns true, we've got it.
             */
             mFoundRouteState = decode(routeStateValue); 
         }
      }
   }

   if (!mFoundRouteState) // not found in a Route header?
   {
      // No state found in the removed Route headers, so look in the Record-Routes
      UtlString recordRoute;

      for (int rrNum = 0;
           message.getRecordRouteUri(rrNum, &recordRoute);
           rrNum++
           )
      {
         Url recordRouteUrl(recordRoute);

         if (recordRoute.contains(UrlParameterName)) // quick test before doing a full url parse
         {
            UtlString routeStateValue;
            if (recordRouteUrl.getUrlParameter(UrlParameterName, routeStateValue))
            {
               /*
                * Got some value for the parameter we expect;
                * parse it and check the signature - if this returns true, we've got it.
                */
               bool foundRouteState;
               foundRouteState = decode(routeStateValue);
               if (foundRouteState)
               {
                  mRecordRouteIndices.push_back( rrNum ); // save this for when we need to update it
               }
               mFoundRouteState |= foundRouteState;
            }
         }
         else
         {
            UtlString recordRouteHostAndPort;
            recordRouteUrl.getHostWithPort( recordRouteHostAndPort );
            //TODO: improvement = compare against list of aliases instead of mRouteHostPort
            if( recordRouteHostAndPort.compareTo( mRouteHostPort ) == 0 )
            {
               mRecordRouteIndices.push_back( rrNum ); // save this for when we need to update it
            }
         }
      }
      
      if (!message.isResponse()) // is this a request?
      {
         // this is a request
         if (mFoundRouteState) // could only have been found in a Record-Route
         {
            // this is a spiraled dialog-forming request that already has state in it.
            mMayBeMutable = true;
         }
         else // request that has no route state in either header
         {
            UtlString method;
            message.getRequestMethod(&method);

            if (method.compareTo(SIP_ACK_METHOD, UtlString::ignoreCase) != 0) // ACK is never mutable
            {
               // check for a To tag to see if it is in-dialog
               Url toUrl;
               UtlString toTag;
               message.getToUrl(toUrl);

               if (! toUrl.getFieldParameter("tag", toTag))
               {
                  /*
                   * This has no To tag, so it appears to be a request that is not in-dialog
                   * (or callee is not 3261 compliant)
                   * It may be the dialog-forming request for a new dialog, so we can record state.
                   */
                  mMayBeMutable = true;
               }
            }
         }
      }
   }
   else
   {
      // We got route state from a Route header, so this is not mutable
   }
}

// Used to discover the request direction.
bool RouteState::directionIsCallerToCalled(const char* instanceName)
{
   /*
    * Some AuthPlugin classes need to make different changes
    * to a request depending on whether it was sent from the
    * original caller or the called party.  For example, anything
    * that makes changes to the To or From headers needs to
    * know which is which because they will be swapped for an
    * in-dialog request sent by the called party.  This method
    * provides a uniform way to discover this.
    *
    * This caches the From tag from the original caller in
    * a shared parameter in the RouteState (a shared parameter
    * passes null string for the instanceName) so that once
    * it is computed by any plugin it is accessible to all.
    *
    * Returns true iff the From header is the original caller.
    */
   assert(instanceName && *instanceName != '\000'); // catch if caller passes null instance name

   bool isCallerToCalled;

   // mFromTag has the from tag from this message.
   
   UtlString originalFromTag;
   if (originalCallerFromTagValue(instanceName, originalFromTag))
   {
      // there was recorded state on this message, so compare the current from tag to that one
      isCallerToCalled = (originalFromTag.compareTo(mFromTag, UtlString::ignoreCase) == 0);
   }
   else
   {
      isCallerToCalled = false; // no way to know, really, but we have to say something...
   }

   return isCallerToCalled;
}

bool RouteState::originalCallerFromTagValue( const char* instanceName ///< used for logging - must not be null
                                            ,UtlString& fromTagValue
                                            )
{
   assert(instanceName && *instanceName != '\000'); // catch if caller passes null instance name

   bool originalFromTagFound;
   fromTagValue.remove(0);
   
   if ( ! (originalFromTagFound = getParameter("" /* no plugin instance name */, "from", fromTagValue)) )
   {
      // There was no original from tag recorded in the state
      if (mMayBeMutable)
      {
         // it may be mutable, so this is the first request, so this from is the caller
         originalFromTagFound = true;
         fromTagValue.append(mFromTag);

         // since someone cared about this, record it in the state
         setParameter("","from",mFromTag);
      }
      else
      {
         /*
          * No from tag recorded, and not mutable
          * This can happen only if:
          * - this is either a response or an in-dialog request
          * and
          * - there is no recorded original from tag.
          * This is an error, since it means that someone is asking
          * for direction but did not ask on the original request
          * (if they had, we'd have saved the original from).
          */
         OsSysLog::add(FAC_SIP,PRI_CRIT,
                       "RouteState::originalCallerFromTagValue plugin '%s': "
                       "no original from in state for nonmutable request; probable plugin error",
                       instanceName?instanceName:"(null)"
                       );
      }
   }

   return originalFromTagFound;
}


// encodes 'auth' parameter to indicate that dialog has been authorized 
void RouteState::markDialogAsAuthorized( void )
{
   if (isMutable())
   {
      UtlString unused;
      setParameter( "", AUTHORIZED_DIALOG_PARAM, unused );              
   } 
}

// tests for the presence of the 'auth' parameter which indicates that dialog has been authorized
bool RouteState::isDialogAuthorized( void )
{
   UtlString unused;
   return getParameter( "", AUTHORIZED_DIALOG_PARAM, unused );
}

/// Encode and sign the state as a single SIP token
void RouteState::encode(UtlString& stateToken)
{
   UtlSortedListIterator nvpairList(mValues);
   NameValuePair* nvpair;
   for(bool firstParam = true;
       (nvpair = dynamic_cast<NameValuePair*>(nvpairList()));
       firstParam = false
       )
   {
      UtlString value(nvpair->getValue());
      UtlString encodedValue;

      // base64()
      NetBase64Codec::encode(value, encodedValue);

      // token_esc()
      encodedValue.replace('+', '\'');
      encodedValue.replace('=', '`');

      if (!firstParam)
      {
         stateToken.append(NV_LIST_SEP);
      }
      // add the nvpair
      stateToken.append(nvpair->data());
      stateToken.append(NV_SEP);
      stateToken.append(encodedValue);
   }

   NetMd5Codec signature;
   signature.hash(mSignatureSecret);
   signature.hash(mCallId);
   signature.hash(stateToken);

   stateToken.append(SIG_SEP);
   signature.appendHashValue(stateToken);
}

/// Check the signature and parse the name/value pairs from a state token
bool RouteState::decode(const UtlString& stateToken)
{
   bool decodedOk = false; // true iff the token was correctly signed and successfully parsed

   mValues.destroyAll();
   
   RegEx stateAndSignature(StateAndSignature);

   if (stateAndSignature.Search(stateToken))
   {
      // the syntax is right - it looks like a signed state token

      // compute the signature it should have
      NetMd5Codec calcSignature;
      calcSignature.hash(mSignatureSecret);
      calcSignature.hash(mCallId);
      
      int nvpairsOffset; // will always be zero, so ignore it
      int nvpairsLength;
      stateAndSignature.Match(1,nvpairsOffset,nvpairsLength); 
      calcSignature.hash(stateToken.data(),nvpairsLength);

      UtlString validSignature;
      calcSignature.appendHashValue(validSignature);

      UtlString actualSignature;
      stateAndSignature.MatchString(&actualSignature,2);

      if (actualSignature.compareTo(validSignature) == 0)
      {
         // the signature checks out - this is probably a good state token
         decodedOk = true;

         RegEx nameValuePair(NameValue);
         nameValuePair.Search(stateToken.data(),stateToken.length(),PCRE_ANCHORED);
         do
         {
            UtlString names;
            nameValuePair.MatchString(&names,1);

            /*
             * The regular expressions would not detect that a name had either:
             * no '*' or more than one '*'; both would be invalid names - exactly one is required.
             * Expressions that did that check for all valid combinations of null
             * name components would take a lot longer to match, so we just do this
             * very quick test here instead.
             */
            UtlString encodedValue;
            UtlString decodedValue;
            ssize_t star;
            if (   ((star = names.index('*')) != UtlString::UTLSTRING_NOT_FOUND)
                && (names.index('*', star+1)  == UtlString::UTLSTRING_NOT_FOUND)
                ) // exactly one '*' in names
            {
               decodedValue.remove(0);
               if (   nameValuePair.MatchString(&encodedValue,2)
                   )
               {
                  if (encodedValue.isNull()) // null values will still match, and are ok
                  {
                     decodedOk = true;
                  }
                  else
                  {
                     // undo token_esc()
                     encodedValue.replace('\'', '+');
                     encodedValue.replace('`' , '=');

                     // undo base64()
                     decodedOk = NetBase64Codec::decode(encodedValue, decodedValue);
                  }
               }

               // Insert the name/value pair
               if (decodedOk)
               {
                  mValues.insert(new NameValuePair(names,decodedValue));
               }
               else
               {
                  OsSysLog::add(FAC_SIP, PRI_ERR,
                                "RouteState::decode value decode failed for '%s' in '%s'",
                                encodedValue.data(), stateToken.data()
                                );
               }
            }
            else
            {
               // wrong number of '*' in the name
               decodedOk = false;
               OsSysLog::add(FAC_SIP, PRI_ERR,
                             "RouteState::decode invalid name '%s' in '%s'",
                             names.data(), stateToken.data()
                             );
            }
         } while ( decodedOk && nameValuePair.SearchAgain() );

         if (!decodedOk)
         {
            // something was bad in the name/value pairs; clear out any partial data
            OsSysLog::add(FAC_SIP, PRI_WARNING,
                          "RouteState::decode nvpairs failed '%s'", stateToken.data()
                          );
            mValues.destroyAll();
         }
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_WARNING,
                       "RouteState::decode signature failed '%s'", stateToken.data()
                       );
      }
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_ERR,
                    "RouteState::decode invalid state token '%s'", stateToken.data()
                    );
   }
   
   return decodedOk;
}



/// Extract value of a parameter saved in the route state.
bool RouteState::getParameter(const char* pluginInstance,
                              const char* parameterName, 
                              UtlString&  parameterValue  ///< output
                              ) const
{
   bool foundParameter = false;

   parameterValue.remove(0);
   RouteParameterName name(pluginInstance,parameterName);

   if (name.isValid())
   {
      NameValuePair* parameter = dynamic_cast<NameValuePair*>(mValues.find(&name));
      if (parameter)
      {
         parameterValue.append(parameter->getValue());
         foundParameter = true;
      }
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT,
                    "RouteState::getParameter called with invalid names '%s' '%s'",
                    pluginInstance?pluginInstance:"(null)", 
                    parameterName?parameterName:"(null)"
                    );
   }
   return foundParameter;
}


/* ================================================================
 *                         Encoding Operations
 *
 * These methods manipulate the state encoded into a dialog-forming request
 */

/// Stores the value of a parameter in the route state.
void RouteState::setParameter(const char*       pluginInstance,
                              const char*       parameterName,  
                              const UtlString&  parameterValue  
                              )
{
   /*
    * Establish a new value for the named parameter to be included
    * when the updated route state is generated.
    *
    * Note  A parameter may be set to the null (zero length) value.
    */
   if (mMayBeMutable)
   {
      RouteParameterName name(pluginInstance,parameterName);
      if (name.isValid())
      {
         NameValuePair* parameter = dynamic_cast<NameValuePair*>(mValues.find(&name));
         if (parameter)
         {
            parameter->setValue(parameterValue);
         }
         else
         {
            mValues.insert(new NameValuePair(name,parameterValue));
         }

         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "RouteState::setParameter plugin '%s' parameter '%s'",
                       pluginInstance?pluginInstance:"(null)", 
                       parameterName?parameterName:"(null)"
                       );

         mModified = true;
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_CRIT,
                       "RouteState::setParameter called with invalid names '%s' '%s'",
                       pluginInstance?pluginInstance:"(null)", 
                       parameterName?parameterName:"(null)"
                       );
      }
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT,
                    "RouteState::setParameter called on non-mutable message\n"
                    "plugin '%s' parameter '%s' value '%s'",
                    pluginInstance, parameterName, parameterValue.data()
                    );
   }
}

/// Removes any value for a parameter from the route state.
void RouteState::unsetParameter(const char* pluginInstance,
                                const char* parameterName
                                )
{
   /*
    * This removes the named parameter from those included when
    * the updated route state is recorded.  This does not
    * affect the value returned by any subsequent call to getParameter
    * on this object.
    */
   if (mMayBeMutable)
   {
      RouteParameterName name(pluginInstance,parameterName);
      if (name.isValid())
      {
         mValues.destroy(&name);
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_CRIT,
                       "RouteState::unsetParameter called with invalid names '%s' '%s'",
                       pluginInstance?pluginInstance:"(null)", 
                       parameterName?parameterName:"(null)"
                       );
      }
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT, "RouteState::unsetParameter called on non-mutable message\n"
                    "plugin '%s' parameter '%s'",
                    pluginInstance, parameterName
                    );
   }
}

void RouteState::addCopy( void )
{
   mAddCopyRequested = true;
}

/// Add or update the state in the Record-Route header.
void RouteState::update(SipMessage* request )
{
   /*
    * This has no effect other than logging an error if isMutable would return false.
    */
   if (mMayBeMutable)
   {
      if (mModified || mAddCopyRequested)
      {
         UtlString routeValue;
         UtlString newRouteStateToken;
         encode(newRouteStateToken);

         Url route(mRouteHostPort);
         route.setUrlParameter("lr",NULL);
         route.setUrlParameter(UrlParameterName, newRouteStateToken.data());
         route.toString(routeValue);

         if( mRecordRouteIndices.empty() )
         {
            // we did not have our own Record-Route in the header, so push ours on top

            request->addRecordRouteUri(routeValue.data());
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "RouteState::update adding new route state");
         }
         else
         {
            // keep the existing Record-routes and simply replace the RouteState
            // URL parameter.  This is done to preserve any modifications that
            // may have been made to the Record-route header by other components.
            std::vector<size_t>::iterator pos;
            for( pos = mRecordRouteIndices.begin(); pos != mRecordRouteIndices.end(); ++pos )
            {
               request->getRecordRouteUri( *pos, &routeValue );
               Url recordRouteUrl( routeValue );
               recordRouteUrl.setUrlParameter( UrlParameterName, newRouteStateToken.data() );
               recordRouteUrl.toString( routeValue );
               request->setRecordRouteField( routeValue.data(), *pos );
               OsSysLog::add( FAC_SIP, PRI_DEBUG, "RouteState::update rewriting route state for RR index %zu", *pos );
            }
         }
         
         if( mAddCopyRequested == true )
         {
            // A plugin as asked to add a new Record-Route header carrying a copy of the route state.
            // Skip the addition if the top Record-Route header is already pointing to us.
            if( mRecordRouteIndices.empty() || mRecordRouteIndices[0] != 0 )
            {
               request->addRecordRouteUri(routeValue.data());
               OsSysLog::add(FAC_SIP, PRI_DEBUG, "RouteState::update adding a copy of the route state");
            }
         }
      }
      else
      {
         // no changes made, so don't bother rewriting.
         OsSysLog::add(FAC_SIP, PRI_DEBUG, "RouteState::update no state changes");
      }
      
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT, "RouteState::record called on non-mutable state");
   }
}

/// Initialize the secret value used to sign hashes.
void RouteState::setSecret(const char* secret /**< a null terminated string used as input to sign the
                                               * state value.  This should be chosen such that it:
                                               * - is hard for an attacker to guess
                                               * - ideally, is the same in replicated authproxies
                                               *   (this is not important yet, since we don't use
                                               *   SRV names in the Record-Route targets).
                                               */
                           )
{
   /*
    * This must be called once at initialization time,
    * before any RouteState objects are created.
    *
    * It may be called after that, but doing so with a
    * new value will invalidate any outstanding route
    * state.
    */
   if (!mSignatureSecret.isNull() && mSignatureSecret.compareTo(secret))
   {
      OsSysLog::add(FAC_SIP,PRI_NOTICE,
                    "RouteState::setSecret called more than once;\n"
                    " value changed from '%s' to '%s'\n"
                    " previously signed state will now fail signature checks",
                    mSignatureSecret.data(), secret
                    );
   }
   mSignatureSecret.remove(0);
   mSignatureSecret.append(secret);   
}
   
/// destructor
RouteState::~RouteState()
{
   mValues.destroyAll();
}

