//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES

#if defined(_VXWORKS)
#   include <taskLib.h>
#   include <netinet/in.h>
#endif

#include <utl/UtlHashBagIterator.h>
#include <net/SipLine.h>
#include <net/Url.h>
#include <net/SipLineCredentials.h>
#include <os/OsConfigDb.h>
#include <os/OsRWMutex.h>
#include <os/OsReadLock.h>
#include <os/OsWriteLock.h>
#include <os/OsDateTime.h>
#include <os/OsSysLog.h>
#include <net/NetMd5Codec.h>
#include <net/SipMessage.h>


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// Copy constructor
SipLine::SipLine(const SipLine& rSipLine)
{
#ifdef DEPRECATED_SIPLINE_FEATURE
   mIsVisible = rSipLine.mIsVisible ;
   mIsAutoEnabled = rSipLine.mIsAutoEnabled ;
   mIsUsingCallHandling = rSipLine.mIsUsingCallHandling;
#endif
   mIdentity = rSipLine.mIdentity ;
   mUserEnteredUrl = rSipLine.mUserEnteredUrl;
   mUser = rSipLine.mUser ;
   mCurrentState = rSipLine.mCurrentState ;
   mCanonicalUrl = rSipLine.mCanonicalUrl;
   mLineId = rSipLine.mLineId;
   mPreferredContactUri = rSipLine.mPreferredContactUri ;
   meContactType = rSipLine.meContactType;

   copyCredentials(rSipLine);
   copyAliases(mAliases, rSipLine.mAliases) ;
}

SipLine& SipLine::operator=(const SipLine& rSipLine)
{
   if (this == &rSipLine)            // handle the assignment to self case
      return *this;
   else
   {
#ifdef DEPRECATED_SIPLINE_FEATURE
      mIsVisible = rSipLine.mIsVisible ;
      mIsAutoEnabled = rSipLine.mIsAutoEnabled ;
      mIsUsingCallHandling = rSipLine.mIsUsingCallHandling;
#endif
      mIdentity = rSipLine.mIdentity ;
      mUserEnteredUrl = rSipLine.mUserEnteredUrl;
      mCanonicalUrl = rSipLine.mCanonicalUrl;
      mUser = rSipLine.mUser ;
      mCurrentState = rSipLine.mCurrentState ;
      mLineId = rSipLine.mLineId;
      mPreferredContactUri = rSipLine.mPreferredContactUri ;
      meContactType = rSipLine.meContactType;
      copyCredentials(rSipLine);
      copyAliases(mAliases, rSipLine.mAliases) ;
   }
   return *this;
}
//deep copy of credentials
void SipLine::copyCredentials(const SipLine &rSipLine)
{
   UtlString Realm;
   UtlString UserID;
   UtlString Type;
   UtlString Password;

   if(!mCredentials.isEmpty())
   {
      mCredentials.destroyAll();
   }
   UtlHashBagIterator observerIterator(const_cast<UtlHashBag&> (rSipLine.mCredentials));
   SipLineCredentials* credential = NULL;
   do
   {
     credential = (SipLineCredentials*) observerIterator();
     if ( credential)
     {
         credential->getRealm(&Realm);
         credential->getUserId(&UserID);
         credential->getType(&Type);
         credential->getPasswordToken(&Password);
         addCredentials(Realm , UserID , Password , Type);
     }
   }
   while(credential != NULL) ;
}

//deep copy of aliases
void SipLine::copyAliases(UtlSList& dest, const UtlSList& source) const
{
    // Clear dest list
    if (!dest.isEmpty())
    {
        dest.destroyAll() ;
    }

    // Copy maintaining order
    int length = source.entries() ;
    for (int i=0; i<length; i++)
    {
        UtlString* pEntry = (UtlString*) source.at(i) ;
        dest.append(new UtlString(*pEntry)) ;
    }
}

SipLine::SipLine(Url userEnteredUrl,
                 Url identityUri,
                 UtlString user,
                 UtlBoolean visible,
                 int state,
                 UtlBoolean isAutoEnabled,
                 UtlBoolean useCallHandling)
{
   if (user.isNull())
   {
      mUser = userEnteredUrl.toString();
   }
   else
   {
      mUser = user;
   }
#ifdef DEPRECATED_SIPLINE_FEATURE
   mIsVisible = visible;
   mIsAutoEnabled = isAutoEnabled;
   mIsUsingCallHandling = useCallHandling;
#endif

   mCurrentState = state;
   mUserEnteredUrl = userEnteredUrl;
   meContactType = LINE_CONTACT_NAT_MAPPED ;
   if (identityUri.toString().isNull())
   {
      //then get uri from user entered url ...uri is complete in it
      UtlString uri;
      mUserEnteredUrl.getUri(uri);
      mIdentity = Url(uri);
   }
   else
   {
      mIdentity = identityUri;
   }

   //construct a complete url from identity and userEntered Url.
   mCanonicalUrl = mUserEnteredUrl;
   UtlString address;
   mUserEnteredUrl.getHostAddress(address);
   if (address.isNull())
   {
      UtlString identityHost;
      mIdentity.getHostAddress(identityHost);
      int identityPort = mIdentity.getHostPort();
      mCanonicalUrl.setHostAddress(identityHost);
      mCanonicalUrl.setHostPort(identityPort);
   }
   // create new Line Id for this line
   generateLineID(mLineId);
   setPreferredContactUri(mIdentity) ;
}

SipLine::~SipLine()
{
    mCredentials.destroyAll();
    mAliases.destroyAll() ;
}

UtlString& SipLine::getLineId()
{
   return mLineId;
}

#ifdef DEPRECATED_SIPLINE_FEATURE
UtlBoolean SipLine::isDeviceLine()
{
    return (getUser().compareTo("Device", UtlString::ignoreCase) == 0) ;
}
#endif


int SipLine::getState()
{
    return mCurrentState;
}

void SipLine::setState(int state)
{
    mCurrentState = state;
}

UtlString& SipLine::getUser()
{
    return mUser;
}

void SipLine::setUser(const UtlString user)
{
   mUser.remove(0);
   mUser.append(user);
}

void SipLine::getIdentityAndUrl(Url &identity, Url &userEnteredUrl)
{
   identity = mIdentity;
   userEnteredUrl = mUserEnteredUrl;
}

void SipLine::setIdentityAndUrl(Url identity, Url userEnteredUrl)
{
   UtlString identityHost;
   UtlString address;
   int identityPort;

   mIdentity = identity;
   mLineId.remove(0);
   generateLineID(mLineId);
   mUserEnteredUrl = userEnteredUrl;
   //construct a complete url from identity and userEntered Url.
   mCanonicalUrl = mUserEnteredUrl;

   mUserEnteredUrl.getHostAddress(address);
   if (address.isNull())
   {
      mIdentity.getHostAddress(identityHost);
      identityPort = mIdentity.getHostPort();
      mCanonicalUrl.setHostAddress(identityHost);
      mCanonicalUrl.setHostPort(identityPort);
   }
}


Url& SipLine::getUserEnteredUrl()
{
    return mUserEnteredUrl;
}
Url& SipLine::getIdentity()
{
    return mIdentity;
}

Url& SipLine::getCanonicalUrl()
{
    return mCanonicalUrl;
}

#ifdef DEPRECATED_SIPLINE_FEATURE
void SipLine::setAutoEnableStatus( UtlBoolean isAutoEnabled)
{
    mIsAutoEnabled = isAutoEnabled;
}

UtlBoolean SipLine::getAutoEnableStatus()
{
    return mIsAutoEnabled ;
}

void SipLine::setVisibility(UtlBoolean isEnable)
{
    mIsVisible = isEnable;
}

UtlBoolean SipLine::getVisibility()
{
    return mIsVisible;
}
void SipLine::setCallHandling(UtlBoolean useCallHandling)
{
    mIsUsingCallHandling = useCallHandling;
}

UtlBoolean SipLine::getCallHandling()
{
    return mIsUsingCallHandling;
}
#endif

UtlBoolean SipLine::addCredentials( const UtlString& strRealm,
                             const UtlString& strUserID,
                             const UtlString& MD5_token,
                             const UtlString& Type)
{
   UtlBoolean isAdded = FALSE;

   if(!IsDuplicateRealm(strRealm, Type))
   {
      SipLineCredentials* credential = new SipLineCredentials(strRealm , strUserID, MD5_token , Type);
      mCredentials.insert((UtlString*)credential);
      isAdded = TRUE;
   }
   return isAdded;
}

int SipLine::GetNumOfCredentials()
{
   return mCredentials.entries();

}

UtlBoolean SipLine::getCredentials(const UtlString& type /*[in]*/,
                                   const UtlString& realm /*[in]*/,
                                   UtlString* userID /*[out]*/,
                                   UtlString* MD5_token /*[out]*/)
{
   UtlBoolean credentialsFound = FALSE;

   SipLineCredentials* credential = (SipLineCredentials*) mCredentials.find(&realm);
   if (credential)
   {
      credential->getUserId(userID);
      credential->getPasswordToken(MD5_token);
      credentialsFound = TRUE;
      credential = NULL;
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipLine::getCredentials "
                    "Credentials for realm '%s' found: "
                    "userID '%s'",
                    realm.data(), userID->data());
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipLine::getCredentials "
                    "Credentials for realm '%s' NOT found",
                    realm.data());
   }

   return  credentialsFound;
}

void SipLine::removeCredential(const UtlString *realm)
{
   UtlString matchRealm(*realm);

   UtlContainable* wasRemoved = mCredentials.removeReference(&matchRealm);

   if(wasRemoved)
   {
      delete wasRemoved;
   }

}

void SipLine::removeAllCredentials()
{
    mCredentials.destroyAll();

}


void SipLine::setPreferredContactUri(const Url& preferredContactUri)
{
    mPreferredContactUri = preferredContactUri ;
    mPreferredContactUri.removeUrlParameter(SIP_LINE_IDENTIFIER);
    mPreferredContactUri.setUrlParameter(SIP_LINE_IDENTIFIER, mLineId);
}


UtlBoolean SipLine::getPreferredContactUri(Url& preferredContactUri) const
{
    UtlString host ;

    preferredContactUri = mPreferredContactUri ;
    preferredContactUri.getHostAddress(host) ;

    return (host.length() > 0) ;
}


LINE_CONTACT_TYPE SipLine::getContactType() const
{
    return meContactType ;
}


void SipLine::setContactType(LINE_CONTACT_TYPE eContactType)
{
    meContactType = eContactType ;
}


UtlBoolean SipLine::getAllCredentials( int MaxEnteries/*[in]*/ ,
      int& actualEnteries /*[out/int]*/,
      UtlString realm[]/*[out/int]*/,
      UtlString userId[]/*[out/int]*/,
      UtlString type[]/*[out/int]*/,
      UtlString passtoken[]/*[out/int]*/)
{
    UtlBoolean credentialsFound = FALSE;
    UtlString Realm;
    UtlString UserID;
    UtlString Type;
    UtlString PassToken;
    int i = 0;

    UtlHashBagIterator observerIterator(mCredentials);
    SipLineCredentials* credential = NULL;

    do
    {
        credential = (SipLineCredentials*) observerIterator();
        if ( credential)
        {
            credential->getRealm(&Realm);
            credential->getUserId(&UserID);
            credential->getType(&Type);
            credential->getPasswordToken(&PassToken) ;

            realm[i].remove(0);
            realm[i].append(Realm);

            userId[i].remove(0);
            userId[i].append(UserID);

            type[i].remove(0);
            type[i].append(Type);

            passtoken[i].remove(0) ;
            passtoken[i].append(PassToken) ;

            i++;
            credentialsFound = TRUE;
        }
    }
    while(credential != NULL && i< MaxEnteries) ;
    actualEnteries = i;
    return  credentialsFound ;
}


UtlBoolean SipLine::IsDuplicateRealm(const UtlString realm, const UtlString scheme)
{
   UtlString userID;
   UtlString passToken;
   return getCredentials(scheme, realm, &userID, &passToken);
}


void SipLine::generateLineID(UtlString& lineId)
{
   NetMd5Codec::encode(mIdentity.toString().data(), lineId);
}


void SipLine::addAlias(const Url& alias)
{
    mAliases.append(new UtlString(alias.toString())) ;
}


int SipLine::getAliases(UtlSList& list) const
{
    copyAliases(list, mAliases)    ;
    return list.entries() ;
}

UtlBoolean SipLine::matchesLineId(const char* szLineId) const
{
    UtlBoolean bMatch = false ;

    if (mLineId.compareTo(szLineId) == 0)    // Case sensitive
    {
        bMatch = true ;
    }

    return bMatch ;
}

UtlBoolean SipLine::matchesUserId(const char* szUserId) const
{
    UtlString matchUserId(szUserId) ;
    UtlString lineUserId ;
    UtlString host ;
    UtlBoolean bMatch = false ;

    // First, try identify
    mIdentity.getUserId(lineUserId);
    mIdentity.getHostAddress(host) ;    // if host is empty, assume invalid line
    if (!host.isNull() && matchUserId.compareTo(lineUserId) == 0) // Case sensitive
    {
        bMatch = true ;
    }
    else
    {
        // Next, try preferred contact
        lineUserId.remove(0) ;
        mPreferredContactUri.getUserId(lineUserId) ;
        host.remove(0);
        mPreferredContactUri.getHostAddress(host) ;    // if host is empty, assume invalid line
        if (!host.isNull() && matchUserId.compareTo(lineUserId) == 0) // Case sensitive
        {
            bMatch = true ;
        }
    }

    // Finally, try aliases
    for (size_t i=0; !bMatch && i<mAliases.entries(); i++)
    {
        UtlString* pAlias = (UtlString*) mAliases.at(i) ;
        assert(pAlias != NULL) ;
        if (pAlias != NULL)
        {
            Url aliasUrl(pAlias->data()) ;
            lineUserId.remove(0) ;
            aliasUrl.getUserId(lineUserId) ;
            host.remove(0);
            aliasUrl.getHostAddress(host) ;    // if host is empty, assume invalid line
            if (!host.isNull() && matchUserId.compareTo(lineUserId) == 0) // Case sensitive
            {
                bMatch = true ;
            }
        }
    }
    return bMatch;
}

UtlBoolean SipLine::matchesIdentity(const Url& identity) const
{
    UtlBoolean bMatch = false ;

    // First, try identify
    if (identity.isUserHostPortEqual(mIdentity, SIP_PORT))
    {
        bMatch = true ;
    }
    else
    {
        if (identity.isUserHostPortEqual(mCanonicalUrl, SIP_PORT))
        {
            bMatch = true ;
        }
    }

    // Finally, try aliases
    for (size_t i=0; !bMatch && i<mAliases.entries(); i++)
    {
        UtlString* pAlias = (UtlString*) mAliases.at(i) ;
        assert(pAlias != NULL) ;
        if (pAlias != NULL)
        {
            Url aliasUrl(pAlias->data()) ;
            if (identity.isUserHostPortEqual(aliasUrl, SIP_PORT))
            {
                bMatch = true ;
            }
        }
    }
    return bMatch;
}
