/****************License************************************************
 *
 * Copyright 2001.  SpeechWorks International, Inc.
 *
 * Use of this software is subject to notices and obligations set forth
 * in the SpeechWorks Public License - Software Version 1.1 which is
 * included with this software.
 * 
 * SpeechWorks is a registered trademark, and SpeechWorks Here, 
 * DialogModules and the SpeechWorks logo are trademarks of SpeechWorks 
 * International, Inc. in the United States and other countries. 
 * 
 ***********************************************************************
 *
 * SBinetChannel Implementation
 *
 * 
 *
 ***********************************************************************/
#ifndef _SB_USE_STD_NAMESPACE
#define _SB_USE_STD_NAMESPACE
#endif

#include <WWWLib.h>
#include <WWWHTTP.h>
#include <WWWInit.h>
#ifdef EINVAL
// Conflicts with OS definition
#undef EINVAL
#endif

#define SBINET_EXPORTS
#include "VXItypes.h"
#include "VXIvalue.h"
#include "VXIinet.h"
#include "VXItrd.h"
#include "VXIlog.h"

#include "SBinetLog.h"

#include "SBinetStream.h"
#include "SBinetChannel.h"
#include "SBinetURL.h"
#include "SBinetCookie.h"


/*****************************************************************************
 *****************************************************************************
 * SBinetChannel Implementation
 *****************************************************************************
 *****************************************************************************
 */

SBinetChannel::SBinetChannel( VXIlogInterface* pVXILog,
			      VXIunsigned diagLogBase ) :
  SBinetLogger(MODULE_SBINET_CHANNEL, pVXILog, diagLogBase)
{
  Diag (MODULE_SBINET_CHANNEL_TAGID, L"SBinetChannel::SBinetChannel",
	L"0x%p", pVXILog);

  m_cookies = NULL;
  m_numCookies = 0;
  m_jarChanged = true;
  m_cookiesEnabled = false; // Until SetCookieJar() is called with a valid jar

  m_pVXILog = pVXILog;
}

SBinetChannel::~SBinetChannel( )
{
  Diag (MODULE_SBINET_CHANNEL_TAGID, L"SBinetChannel::~SBinetChannel", NULL);
  deleteAllCookies();
  // NOP //    // NOP //    // NOP //
}


SBinetStream*
SBinetChannel::GetStream(VXIinetStream* str)
{
  return((SBinetStream*)str);        // really should validate, but for now
}

/*
 * Prefetch: For now punt, eventually spawn thread to call Open() into /dev/null
 */
VXIinetResult 
SBinetChannel::Prefetch(  const VXIchar*  pszModuleName,
			  const VXIchar*  pszName,
			  VXIinetOpenMode eMode,
			  VXIint32        nFlags,
			  const VXIMap*   pProperties )
{
  /*
   * Add prefetch request to prefetch queue. These requests are
   * processed when the fetch engine is idle, and the fetched 
   * documents are stored in the Inet cache until retrieved by
   * a subsequent Open call. Note that in order for prefetching 
   * to work, caching must be enabled, the fetched document must 
   * be cachable (server must not return a 'no-cache' directive) 
   * and the caching mode must not be SAFE.
   */
#if 0 // NOT YET IMPLEMENTED
  Diag( MODULE_SBINET_CHANNEL_TAGID, L"SBinetChannel::Prefetch",
	    L"(%s)", pszName );

  if(eMode != INET_MODE_READ)
    return VXIinet_RESULT_UNSUPPORTED;

  // Only HTTP requests can be prefetched

  // Parse URL, combine and narrow
  SBinetURL* url = new SBinetURL();
  if(url == NULL) {
    Error(103, NULL);
    return VXIinet_RESULT_OUT_OF_MEMORY;
  }
  else if(url->Parse(pszName, eMode, nFlags, pProperties) != 0) {
    delete url;
    url = NULL;
    return VXIinet_RESULT_INVALID_ARGUMENT;
  }

  int scheme = url->GetScheme();
  if(scheme == SBinetURL::FILE_SCHEME) {
    delete url;
    url = NULL;
    return VXIinet_RESULT_UNSUPPORTED;
  }

  SBinetPrefetchRequest *request = new SBinetPrefetchRequest;
  if( request == NULL) {
    Error(103, NULL);
    return VXIinet_RESULT_OUT_OF_MEMORY;
  }

  request->pChannel = this;
  request->pInetURL = url;
  request->nFlags = nFlags;

  // Clone the properties and extract the prefetch priority
  if( pProperties != NULL ) {
    request->pProperties = VXIMapClone(pProperties);

    VXIInteger *priority = 
      (VXIInteger *)VXIMapGetProperty(pProperties, INET_PREFETCH_PRIORITY);
    if( priority != NULL)
      request->ePriority = (VXIinetPrefetchPriority)VXIIntegerValue(priority);
  }

  AddToPrefetchQueue(request);
#endif
  return VXIinet_RESULT_SUCCESS;
}


#if 0 // NOT YET IMPLEMENTED
// PrefetchRequest
//
class SBinetPrefetchRequest {
 public:
  VXIinetPrefetchPriority ePriority;
  SBinetURL* pInetURL;
  VXIint32 nFlags;
  VXIMap *pProperties;

  SBinetChannel *pChannel;

  SBinetPrefetchRequest() {
    ePriority = INET_PREFETCH_PRIORITY_LOW;
    pInetURL = NULL;
    nFlags = 0;
    pProperties = NULL;
  }

  ~SBinetPrefetchRequest() {
    if( pInetURL != NULL )
      delete pInetURL;
    pInetURL = NULL;
    if( pProperties != NULL )
      VXIMapDestroy(&pProperties);
  }
};

VXIinetResult 
SBinetChannel::ProcessPrefetchRequest( SBinetPrefetchRequest *pRequest )
{
  Diag( MODULE_SBINET_CHANNEL_TAGID, L"SBinetChannel::ProcessPrefetchRequest",
	    L"(%s)", pRequest->pInetURL->GetAbsoluteWide());

  /*
   * Create Stream
   */
  SBinetStream* pStream = 
    (SBinetStream *)(new SBinetHttpStream(pRequest->pInetURL, this));

  if(pStream == NULL) {
    delete pRequest;
    pRequest= NULL;
    return VXIinet_RESULT_OUT_OF_MEMORY;
  }

  /*
   * Call Stream Open.
   *  At the moment (5/9/01, this will block until entire page is loaded
   */
  VXIinetResult result = 
    pStream->Open(pRequest->nFlags, (VXIMap*)pRequest->pProperties, NULL);

  if( result == VXIinet_RESULT_SUCCESS )
    result = Close((VXIinetStream **)&pStream);

/* DWW TODO should we delete here?   (we do two methods down in open!)
   like this...  (I won't check this in now because we are close to shipping and this code 
                  is not called) 
   else
      delete pStream;
*/

  return result;
}
#endif

VXIinetResult
SBinetChannel::CloseAll(){
  // should interate over open stream and close gracefully
  return(VXIinet_RESULT_SUCCESS);
}

/*
 * Open: Serious work here.
 *   Parse URL and combine with base
 *   Collect properties and query args
 *   Call appropriate Stream constructor base on URL scheme (http or file)
 *   Call Open() on stream
 */
VXIinetResult 
SBinetChannel::Open( const VXIchar*   pszModuleName,
		     const  VXIchar*  pszName,
		     VXIinetOpenMode  eMode,
		     VXIint32         nFlags,
		     const VXIMap*    pProperties,
		     VXIMap*          pmapStreamInfo,
		     VXIinetStream**  ppStream     )
{
  if(eMode != INET_MODE_READ)
    return(VXIinet_RESULT_UNSUPPORTED);
  if(!ppStream) {
    Error(200, L"%s%s", L"Operation", L"Open");
    return(VXIinet_RESULT_INVALID_ARGUMENT);
  }
  *ppStream = (VXIinetStream*)NULL; // in case of failure

  /*
   * Parse URL, combine and narrow
   */
  Diag (MODULE_SBINET_CHANNEL_TAGID, L"SBinetChannel::Open",
	L"(%s)", pszName);

  SBinetURL* url = new SBinetURL();  // NULL constructor
  if (! url) {
    Error(103, NULL);
    return(VXIinet_RESULT_OUT_OF_MEMORY);
  } else if(url->Parse(pszName,eMode,nFlags,pProperties)) {
    delete url;
    url = NULL;
    Error(200, L"%s%s", L"Operation", L"Open");
    return(VXIinet_RESULT_INVALID_ARGUMENT);
  }

  /*
   * Create Stream
   */
  SBinetStream* st = NULL;
  int scheme = url->GetScheme();
  /*
   * Note: Stream now owns url and must delete on cleanup
   */
  if(scheme == SBinetURL::HTTP_SCHEME)
    st = (SBinetStream*)(new SBinetHttpStream(url, this, 
                                              GetLog(), GetDiagBase()));
  else if(scheme == SBinetURL::FILE_SCHEME)
    st = (SBinetStream*)(new SBinetFileStream(url, GetLog(), GetDiagBase()));
  if(!st) {
    delete url;
    url = NULL;
    Error(103, NULL);
    return(VXIinet_RESULT_OUT_OF_MEMORY);
  }
  /*
   * Call Stream Open.
   *  At the moment (5/9/01, this will block until entire page is loaded
   */
  VXIinetResult err = st->Open(nFlags,(VXIMap*)pProperties,pmapStreamInfo);
  if(err == VXIinet_RESULT_SUCCESS)
    *ppStream = (VXIinetStream*)st;
  else
  {
    delete st;
    st = NULL;
  }
  return(err);
}


VXIinetResult 
SBinetChannel::Close(VXIinetStream**  ppStream)
{
  if(!ppStream) {
    Error(200, L"%s%s", L"Operation", L"Close");
    return(VXIinet_RESULT_INVALID_ARGUMENT);
  }
  SBinetStream* st = GetStream(*ppStream);   // Really just validates and casts
  VXIinetResult err;
  if(st){
    err = st->Close();
    delete st;
    st = NULL;
  }
  else{
    Error(200, L"%s%s", L"Operation", L"Close");
    err = VXIinet_RESULT_INVALID_ARGUMENT;
  }
  *ppStream = (VXIinetStream*)NULL;
  return(err);
}


//
// Static method
//
VXIinetResult SBinetChannel::SetCookieJar( const VXIVector* pJar )
{
  // const VXIchar  *key = NULL;
  // const VXIValue *value = NULL;
  const char *EMPTY_STRING = "";

  deleteAllCookies();

  if(pJar == NULL) { // NULL jar means 'disable cookie usage'
    m_jarChanged = false;
    m_cookiesEnabled = false;
    return VXIinet_RESULT_SUCCESS;
  }
  m_cookiesEnabled = true; // Enable cookie usage

  VXIunsigned numElements = VXIVectorLength(pJar);
  for (VXIunsigned i = 0; i < numElements; i++) {
    const VXIMap *cookie = (const VXIMap *) VXIVectorGetElement(pJar, i);
    if ( cookie == NULL ) {
      Error (212, NULL);
    } else if ( VXIValueGetType((const VXIValue *) cookie) != VALUE_MAP ) {
      Error (213, L"%s%d", L"VXIValueType", 
	  VXIValueGetType((const VXIValue *) cookie));
    } else {
      // Get the expiration date
      const VXIInteger *tempInt = 
        (const VXIInteger *)VXIMapGetProperty(cookie, INET_COOKIE_EXPIRES);
      time_t expires = 0;
      if(tempInt != NULL)
        expires = (time_t)VXIIntegerValue(tempInt);

      // Check if the cookie is expired, if so don't add it, zero
      // expiration time means it expires immediately
      if(expires < time(0))
        continue;

      // Get the name
      const VXIString *tempStr = (const VXIString *)
	VXIMapGetProperty(cookie, INET_COOKIE_NAME);
      VXIint len = VXIStringLength(tempStr) + 1;
      char *name = new char [len];
      wcstombs(name, VXIStringCStr(tempStr), len);

      // Get the domain
      tempStr = (const VXIString *)VXIMapGetProperty(cookie, 
						     INET_COOKIE_DOMAIN);
      char *domain = (char*)EMPTY_STRING;
      if(tempStr != NULL) {
        len = VXIStringLength(tempStr) + 1;
        domain = new char [len];
        wcstombs(domain, VXIStringCStr(tempStr), len);
      }

      // Get the path
      tempStr = (const VXIString *)VXIMapGetProperty(cookie, INET_COOKIE_PATH);
      char *path = (char*)EMPTY_STRING;
      if(tempStr != NULL) {
        len = VXIStringLength(tempStr) + 1;
        path = new char [len];
        wcstombs(path, VXIStringCStr(tempStr), len);
      }

      // Get the secure flag
      tempInt = (const VXIInteger *)VXIMapGetProperty(cookie, 
						      INET_COOKIE_SECURE);
      if(tempInt == NULL) {
        Error(200, NULL);
        continue;
      }
      VXIbool secure = (VXIbool)VXIIntegerValue(tempInt);

      // Get the value
      tempStr = (const VXIString *)VXIMapGetProperty(cookie,INET_COOKIE_VALUE);
      char *value = (char*)EMPTY_STRING;
      if(tempStr != NULL) {
        len = VXIStringLength(tempStr) + 1;
        value = new char [len];
        wcstombs(value, VXIStringCStr(tempStr), len);
      }

      // Create the cookie
      SBinetCookie* pSBinetCookie = new SBinetCookie(domain, path, name, 
						     value, expires, secure);
      if(domain != EMPTY_STRING)
      {
	delete [] domain;
	domain = NULL;
      }
      if(path != EMPTY_STRING)
      {
        delete [] path;
        path = NULL;
      }
      if(value != EMPTY_STRING)
      {
        delete [] value;
        value = NULL;
      }

      // Add the cookie to the channel's list
      if (( pSBinetCookie ) && ( addCookie(pSBinetCookie) == 0 ))
      {
	delete pSBinetCookie; // Could not add
	pSBinetCookie = NULL;
      }
    }
  }
  
  m_jarChanged = false;
  return VXIinet_RESULT_SUCCESS;
}


//
// Static method
//
VXIinetResult SBinetChannel::GetCookieJar( VXIVector** ppJar, 
					   VXIbool* ppfChanged )
{
  if(ppJar == NULL) {
    Error(200, L"%s%s", L"Operation", L"GetCookieJar");
    return VXIinet_RESULT_INVALID_ARGUMENT;
  }

  *ppJar = VXIVectorCreate();
  if(*ppJar == NULL) {
    Error(103, NULL);
    return VXIinet_RESULT_OUT_OF_MEMORY;
  }

  // Parse the channel's cookie list
  SBinetCookie *elem = m_cookies;
  while(elem != NULL)
  {    
    if(elem->expired()) {  // Skip expired cookies
      elem = elem->getNext();
      continue;
    }

    VXIMap *cookie = VXIMapCreate();
    if(cookie == NULL) {
      VXIVectorDestroy(ppJar);
      Error(103, NULL);
      return VXIinet_RESULT_OUT_OF_MEMORY;
    }

    // Set the domain
    VXIint len = ::strlen(elem->getDomain()) + 1;
    VXIchar *tempStr = new VXIchar [len];
    mbstowcs(tempStr, elem->getDomain(), len);
    VXIMapSetProperty(cookie, INET_COOKIE_DOMAIN, 
                      (VXIValue *)VXIStringCreate(tempStr));
    delete [] tempStr;
    tempStr = NULL;

    // Set the path
    len = ::strlen(elem->getPath()) + 1;
    tempStr = new VXIchar [len];
    mbstowcs(tempStr, elem->getPath(), len);
    VXIMapSetProperty(cookie, INET_COOKIE_PATH, 
                      (VXIValue *)VXIStringCreate(tempStr));
    delete [] tempStr;
    tempStr = NULL;

    // Set the expiration date
    VXIMapSetProperty(cookie, INET_COOKIE_EXPIRES, 
                      (VXIValue *)VXIIntegerCreate(elem->getExpires()));

    // Set the secure flag
    VXIMapSetProperty(cookie, INET_COOKIE_SECURE,
                      (VXIValue *)VXIIntegerCreate(elem->getSecure()));

    // Set the value
    len = ::strlen(elem->getValue()) + 1;
    tempStr = new VXIchar [len];
    mbstowcs(tempStr, elem->getValue(), len);
    VXIMapSetProperty(cookie, INET_COOKIE_VALUE, 
                      (VXIValue *)VXIStringCreate(tempStr));
    delete [] tempStr;
    tempStr = NULL;

    // Set the name
    len = ::strlen(elem->getName()) + 1;
    tempStr = new VXIchar [len];
    mbstowcs(tempStr, elem->getName(), len);
    VXIMapSetProperty(cookie, INET_COOKIE_NAME,
		      (VXIValue *)VXIStringCreate(tempStr));
    delete [] tempStr;
    tempStr = NULL;

    // Set the version, TBD currently a hack, should get the version
    // from libwww
    VXIMapSetProperty(cookie, INET_COOKIE_VERSION,
		      (VXIValue *)VXIIntegerCreate(1));

    // Add the cookie to the jar
    VXIVectorAddElement(*ppJar, (VXIValue *)cookie);

    elem = elem->getNext();
  } // while

  if(ppfChanged != NULL) {
    *ppfChanged = m_jarChanged;
  }

  return VXIinet_RESULT_SUCCESS;
}

  
void
SBinetChannel::deleteAllCookies()
{
  SBinetCookie *elem = m_cookies;
  while(elem != NULL) {
    SBinetCookie *next = elem->getNext();
    delete elem;
    elem = next;
  }

  m_cookies = NULL;
  m_numCookies = 0;
  m_jarChanged = true;
}


VXIint
SBinetChannel::updateCookieIfExists( const char*   pszDomain,
                                     const char*   pszPath,
                                     const char*   pszName,
                                     const char*   pszValue,
                                     const time_t  nExpires,
                                     const VXIbool fSecure )
{
  SBinetCookie *elem = m_cookies;
  VXIbool found = false;

  // Check if the cookie doesn't already exist, if so update it
  while(elem != NULL) {
    if(elem->matchExact(pszDomain, pszPath, pszName)){
      elem->update(pszValue, nExpires, fSecure);
      m_jarChanged = true;
      found = true;
      break;
    }
    elem = elem->getNext();
  }
  
  return (VXIint)found;
}


VXIint
SBinetChannel::cleanCookies()
{
  SBinetCookie *elem = m_cookies;
  SBinetCookie *prev = NULL;
  VXIint deleted = 0;

  // Delete expired cookies first
  while(elem != NULL) {
    SBinetCookie *next = elem->getNext();
    if(elem->expired()){
      if( prev )
        prev->setNext( next );
      else
        m_cookies = next;

      delete elem;
      deleted++;
      m_numCookies--;
      m_jarChanged = true;
    }
    else {
      prev = elem;
    }
    elem = next;
  }

  if( deleted > 0 )
    return deleted;

  // No cookies were expired, so delete the least recently used
  time_t oldestDate = 0;

  // Find the oldest timestamp
  elem = m_cookies;
  while(elem != NULL) {
    if(( oldestDate == 0 ) || ( elem->getTimeStamp() < oldestDate ))
      oldestDate =  elem->getTimeStamp();

    elem = elem->getNext();
  }

  // Now delete all 'old' cookies
  elem = m_cookies;
  while(elem != NULL) {
    SBinetCookie *next = elem->getNext();
    if(elem->getTimeStamp() == oldestDate){
      if( prev )
        prev->setNext( next );
      else
        m_cookies = next;

      delete elem;
      deleted++;
      m_numCookies--;
      m_jarChanged = true;
    }
    else {
      prev = elem;
    }
    elem = next;
  }

  return deleted;
}


VXIint
SBinetChannel::addCookie(SBinetCookie* cookie)
{
  if(cookie != NULL) {
    if(m_numCookies < MAX_COOKIES) {
      cookie->setNext(m_cookies); // Add to the head of the list
      m_cookies = cookie;
      m_numCookies++;
      m_jarChanged = true;
    }
    else {
      cleanCookies(); // Delete old cookies
      if(m_numCookies >= MAX_COOKIES) {
        Error(210, NULL);
        return 0;
      }
      else
        addCookie( cookie ); // Call again, it'll work this time
    }
  }
  return 1;
}


HTAssocList*
SBinetChannel::collectCookies( const char* domain, 
                               const char* path )
{
  HTAssocList* list = NULL;
  SBinetCookie *elem = m_cookies;

  while(elem != NULL) {
    if(elem->matchRequest(domain, path)){
      if ( !list ) {
        list = HTAssocList_new(); // Deleted by the cookie module
      }
      if(list) {
	    HTAssocList_addObject(list, elem->getName(), elem->getValue());
      }
      else {
        Error(211, NULL);
      }
      elem->updateTimeStamp();
    }
    elem = elem->getNext();
  }
  return(list);
}
