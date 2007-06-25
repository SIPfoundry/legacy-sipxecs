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
 * SBinetHttpStream Implementation
 *
 * 
 *
 ***********************************************************************/
#ifndef _SB_USE_STD_NAMESPACE
#define _SB_USE_STD_NAMESPACE
#endif

#ifdef WIN32

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#undef HTTP_VERSION
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <sys/timeb.h>                  // for _ftime( )/ftime( )
#include <assert.h>

#include <WWWLib.h>
#include <WWWHTTP.h>
#include <WWWInit.h>
#ifdef EINVAL
// Conflicts with OS definition
#undef EINVAL
#endif

#include "VXIvalue.h"
#include "VXIinet.h"
#include "VXItrd.h"

#include "SBinetLog.h"
#include "SBinetURL.h"
#include "SBinetStream.h"
#include "SBinetChannel.h"
#include "SBinetValidator.h"
#include "SBinetCacheLock.h"

#include "HTCache.h"					 /* Implemented here */
#include "HTAncMan.h"
#include "HTAnchor.h"
#include "HTReq.h"
#include "HTReqMan.h"

#include "os/OsSysLog.h"
#include "SBclientUtils.h"

#ifdef WIN32
/* Suppress/disable warnings */
#pragma warning(4 : 4706) /* Assignment within conditional expression
			     (for Libwww HTList_[...]( ) macros) */
#endif

#define INET_INFO_HTTP_STATUS L"inet.httpStatus"

#define INET_PERIODIC_HTANCHOR_CLEANUP L"inet.htAnchorPeriod"


extern "C" {
#include "SBHTStream.h"
}

#ifndef WIN32
#define Sleep(x) usleep(1000*(x))
#endif
 
#define MAX_CHUNK_SIZE (10 * 1024 * 1024)

/**
May 2007 Mark Gertsvolf
Disclaimer: I do not pretend to understand this code well. All I know is it was originally designed to work with W3C wwwlib library and 
hence this API is used extensively. The original code may have been designed to use HTCache to prevent repetitive HTTP gets.
Comments in the code suggest that the original designers struggled a bit and decided to first disable the cache, then the code
may have been patched to use sipXtacklib HTTP transfers.
However the wwwlib calls have been left all over the place and they do serve some purpose I recon, at least the HTChunk carries the result of the HTTP
get to be retrieved by Read call later on.

There is a memory leak or an ever growing HTAnchor cache. It is about ~180 octets per message retrieval.
The size of the leak depends on the SIP domain names of the calling user
For a single message deposit the following URLs are searched, then inserted into the cache if not found.  
1. 'https://localhost:8091/cgi-bin/voicemail/mediaserver.cgi?action=deposit&mailbox=19801&from=sipp%3Csip%3A19811%40markg-sipx2.example.com%3E%3Btag%253D1' 
2. `https://localhost:8091/cgi-bin/voicemail/mediaserver.cgi`
3. `http://localhost:8090/vm_vxml/root.vxml'
4. `http://localhost:8090/vm_vxml/savemessage.vxml'
5. `file:/tmp/fileUk6Ekj'

Note that there are 2 URLs that are unique per call: one is the 
temporary file (presumably the message being recorded). The second URL 
is the URL to call CGI script.This URL contains from header of an 
incoming call including the to tag, which means that it will be unique 
for each message deposit call.

My preference would be to rewrite this whole module, but at this point I am just here to fix the leak.
The code seems fragile and I am afraid to make large scale changes. Instead, I am adding a band aid, i.e. an
instance counting logic and a call to clear the cache whenever the number of SBinetHttpStream objects drops
to zero and in periods that are controlled via config parameter.
Modules other then SBinetHttpStream do not seem to be using HTAnchor cache
The configuration is there to allow external control over this fix in case it does damage.

In order to configure the patch add the following line into VXI config file:
inet.htAnchorPeriod      VXIInteger      <n>

where n is an integer equal or greater then zero. Zero disables the patch, otherwise n controls how many times the  number if active 
instances of SBinetHttpStream class has to drop t zero before the HTAnchor cache is cleaned

**/

#define PERIODIC_HTANCHOR_CLEANUP 1000

int SBinetHttpStream::sm_instanceCount = 0;
int SBinetHttpStream::sm_cleanupPeriod = PERIODIC_HTANCHOR_CLEANUP;

void SBinetHttpStream::Init(const VXIMap* configArgs) {

  OsSysLog::add(FAC_HTTP, PRI_DEBUG,"SBinetHttpStream::Init configArgs = '%p'\n", configArgs);

  int newCleanupPeriod = -1;
  VXIplatformResult rc = VXIplatform_RESULT_SUCCESS;
  VXIint32 tempInt = 0;
  rc = SBclientGetParamInt(configArgs, INET_PERIODIC_HTANCHOR_CLEANUP, &tempInt, FALSE);
  if (rc == VXIplatform_RESULT_SUCCESS) {
    OsSysLog::add(FAC_HTTP, PRI_DEBUG,"SBinetHttpStream::Init Found config oparam %s:\n", INET_PERIODIC_HTANCHOR_CLEANUP);
    newCleanupPeriod = (VXIunsigned) tempInt;
  }

  sm_cleanupPeriod = (newCleanupPeriod != -1) ? newCleanupPeriod : PERIODIC_HTANCHOR_CLEANUP;
  OsSysLog::add(FAC_HTTP, PRI_DEBUG,"SBinetHttpStream::Init sm_cleanupPeriod = '%d'\n", sm_cleanupPeriod);
}


HTChunk*
SBinetHttpStream::Get(SBinetURL*       url,
		      VXIint32                  flags,
		      const VXIMap             *properties,
		      VXIMap                   *streamInfo)

{
  /* Setup to read to chunk for now. Non-optimal in that chunk is size
      limited Also, we must wait for entire download to happen. Can we
      get a callback when header is complete??

      Note: This performs Web transaction in this thread until
      something would block, then returns, and rest of download is
      done in event thread. Since we are waiting here anyway, we could
      actually use blocking IO, but we hope to fix all in future.

      Tried to make a Stream class that would return WOULD_BLOCK when
      full (instead of failing as Chunks do). However, there seems to
      be no mechanism in libwww for dealing with this.  The result was
      deadlock as the system ceased to get events, behavior is worse
      if the doc was in cache. For now we are just going to use the
      libww chunk class and live with the size limitation. Also, as a
      consequence, Open will not return until the entire doc is
      downloaded into memory...bogus, but that seems to be the way
      libwww works.

  */
  HTRequest_setMethod(m_request,METHOD_GET);
  url->AppendQueryArgs(m_queryArgs);
  const char* absolute_url = url->GetAbsoluteNarrow();
  OsSysLog::add(FAC_HTTP, PRI_DEBUG,
                "SBinetHttpStream::Get(4) absolute_url = '%s'",
                absolute_url);
  HTAnchor* anchor = HTAnchor_findAddress(absolute_url);
  // Need to do this for CacheValid
  HTRequest_setAnchor(m_request,anchor);
  /*
   * get lock for anchor 
   */
  m_lock = SBinetCacheLockTable::FindCacheLock(anchor);
  if(m_lock == NULL){   // Should never fail
    Error (217, NULL);
    return(NULL);
  }
  /*
   * lock anchor -- Must unlock libwww while waiting to avoid deadlock
   */
  SBinetInterface::UnlockLibwww( );
  m_lock->GetReadLock(m_request);
  SBinetInterface::LockLibwww( );

  /*
   * This test used to be part of ill-fated multiple-reader scheme, now only informative
   *  Note: Change printfs to Log when satisfied.
  if(CacheValid(m_request)){
    printf("Request in cache");
  }
  else{
   printf("Request NOT in cache\n");
  }
   */
  /*
   * The actual call
   */
  m_chunk = NULL;
  HTStream * target = SBHTStream(m_request, &m_chunk, MAX_CHUNK_SIZE);
  HTRequest_setOutputStream(m_request, target);

  OsSysLog::add(FAC_HTTP, PRI_DEBUG, "SBinetHttpStream::Get(4) url = '%ls'",
                url->GetAbsolute());
  if(OsGetData(anchor, url) != TRUE){
//  if(HTLoadAnchor((HTAnchor*)anchor, m_request) != YES){
    HTChunk_delete(m_chunk);
    m_chunk =  NULL;
  }

  return(m_chunk);
}

/*
 * Our own GET.
 */
UtlBoolean
SBinetHttpStream::OsGetData(HTAnchor* anchor, 
                             SBinetURL* url)
{
   UtlBoolean retval = TRUE;

   UtlString absolute_url = url->GetAbsoluteNarrow();
   HTParentAnchor* parentanchor = (HTParentAnchor *) HTAnchor_findAddress(absolute_url);

   int contentLength = HTAnchor_length(parentanchor);

   SBinetString contentType;
   /* UNUSED VARIABLE VXIinetResult rc = */ url->ContentTypeFromUrl(&contentType);

   char* strContentType = new char[contentType.length() + 1];

   if (strContentType)
   {
      for (unsigned int ix = 0; ix < contentType.length(); ix++)
         strContentType[ix] = contentType[ix];
      strContentType[contentType.length()] = 0;
   }

   UtlString tobeEscaped("@");
   HttpMessage::escapeChars(absolute_url, tobeEscaped);
   OsSysLog::add(FAC_HTTP, PRI_DEBUG,
                 "SBinetHttpStream::OsGetData absolute_url.data() = '%s'",
                 absolute_url.data());
   // The URL is in request-URI format.
   Url newUrl(absolute_url.data(), TRUE);
   UtlString s = newUrl.toString();
   OsSysLog::add(FAC_HTTP, PRI_DEBUG,
                 "SBinetHttpStream::OsGetData newUrl.toString() = '%s'",
                 s.data());

   // get the url path up to the ? separator 
   // (indicated via the TRUE parameter)
   UtlString uriString;
   newUrl.getPath( uriString, TRUE );

   // get the hostname
   UtlString server;
   newUrl.getHostAddress( server );

   int port = newUrl.getHostPort();
   char portString [32];
   memset(portString, 0, 32*sizeof(char));
   sprintf ( portString, ":%d", port );
   server.append(portString);

   OsSysLog::add(FAC_HTTP, PRI_DEBUG,
                 "SBinetHttpStream::OsGetData uriString = '%s'",
                 uriString.data());
   HttpMessage *pRequest = new HttpMessage();
   pRequest->setFirstHeaderLine( HTTP_GET_METHOD, uriString, HTTP_PROTOCOL_VERSION );
   pRequest->addHeaderField( "Accept", "*/*"); 
//   pRequest->addHeaderField("Connection", "Keep-Alive");
//   pRequest->addHeaderField("Host", server.data()); 
   pRequest->setUserAgentField("Pingtel Mediaserver 1.0.0"); 
   pRequest->setContentType(strContentType);

    // get doc for query args
/*   VXIulong doclen = 0;
   const char* doc = url->QueryArgsToMultipart(m_queryArgs,&doclen);

   HttpBody* body = new HttpBody( 
       doc,
       doclen, 
       strContentType );
   pRequest->setBody(body);
   if ((doclen = body->getLength()) > 0)
      pRequest->setContentLength(doclen);
*/
   HttpMessage *pResponse = 
       new HttpMessage();

   int ret = pResponse->get( newUrl, *pRequest, m_nTimeoutOpen );
   if (DiagIsEnabled(MODULE_SBINET_TAGID))
       Diag (MODULE_SBINET_TAGID, (const VXIchar*)NULL, L"OsGetData get returns %d\n", ret);

   UtlString contentTypeString;
   pResponse->getContentType(&contentTypeString);
   m_HttpStatus = pResponse->getResponseStatusCode();;
   contentLength = pResponse->getContentLength();

   VXIunsigned tagID = (unsigned int)MODULE_SBINET_STREAM_TAGID;
   if (DiagIsEnabled(tagID))
   {
   	Diag (tagID, (const VXIchar*)NULL, L"*********************************************\n");
        int len = absolute_url.length();
        VXIchar *out = NULL;
        out = new VXIchar[len + 1];
        if (out)
        {
            const char* str = absolute_url.data();
            for (int i = 0; i < len; i++)
              out[i] = str[i];
            out[len] = 0;
            Diag (tagID, (const VXIchar*)NULL, L"request url %s\n", out);
	    delete[] out;
	    out = NULL;
        }
   	Diag (tagID, (const VXIchar*)NULL, L"response code (%d) response contentLength(%d)\n", m_HttpStatus, contentLength);
   }

   int bodyLen = 0;
   UtlString bodybytes;
   const HttpBody *responseBody = pResponse->getBody();
   if (responseBody) 
   {
   	responseBody->getBytes(&bodybytes,&bodyLen);
	   bodybytes = bodybytes.strip(UtlString::leading, '\r');
	   bodybytes = bodybytes.strip(UtlString::leading, '\n');
	   bodybytes = bodybytes.strip(UtlString::leading, '\r');
	   bodybytes = bodybytes.strip(UtlString::leading, '\n');
      if (DiagIsEnabled(tagID))
      {
   	Diag (tagID, (const VXIchar*)NULL, L"response body bytes(%d)\n", bodyLen);
#ifdef SBINET_DEBUG /* [ */
   	int i;
   	VXIchar *out = NULL;
        out = new VXIchar[bodyLen + 1];
	if (out)
	{
            const char* str = bodybytes.data();
            for (i = 0; i < bodyLen; i++)
              out[i] = str[i];
            out[bodyLen] = 0;
   	    Diag (tagID, (const VXIchar*)NULL, L"%s\n", out);
	    delete[] out;
	    out = NULL;
	}
#endif /* SBINET_DEBUG ] */   
       Diag (tagID, (const VXIchar*)NULL, L"*****************************************\n");
     }
   }

   if (bodyLen > 0) {
   	//char* data = new char[bodyLen + 1];
   	char* data = (char*)HT_MALLOC(bodyLen + 1);
   	memcpy(data, bodybytes.data(), bodyLen);
   	data[bodyLen] = 0;

      if (m_chunk)
         HTChunk_delete(m_chunk);
   	m_chunk = HTChunk_fromCString (data, bodyLen);
   }
   else retval = FALSE;

   if (strContentType)
   {
      delete[] strContentType;
      strContentType = NULL;
   }
   delete pResponse;
   pResponse = NULL;
   delete pRequest;
   pRequest = NULL;

   HTRequest_addGnHd(m_request, HT_G_DATE);
   HTRequest_setMethod(m_request, METHOD_GET);
   HTRequest_setAnchor(m_request, anchor);

   m_Done = TRUE;

   return retval;
}

/*
 * POST simple form data. Don't know how to combine Form data 
 * and audio in multipart POST
 */
HTChunk*
SBinetHttpStream::PostMulti(SBinetURL*       url,
		      VXIint32                  flags,
		      const VXIMap             *properties,
		      VXIMap                   *streamInfo)
{
  /* Get an anchor object for the URI */
  const char* absolute_url = url->GetAbsoluteNarrow();
  OsSysLog::add(FAC_HTTP, PRI_DEBUG,
                "SBinetHttpStream::PostMulti absolute_url = '%s'",
                absolute_url);
  HTAnchor* anchor = HTAnchor_findAddress(absolute_url);
  /*
   * get lock for anchor 
   */
  m_lock = SBinetCacheLockTable::FindCacheLock(anchor);
  if(m_lock == NULL){   // Should never fail
    Error (217, NULL);
    return(NULL);
  }
  /*
   * lock anchor -- Must unlock libwww while waiting to avoid deadlock
   */
  SBinetInterface::UnlockLibwww( );
  m_lock->GetReadLock(m_request);
  SBinetInterface::LockLibwww( );
  /*
   * Set up Post (copied from HTPostFormAnchorToChunk
   */
  // get doc for query args
  VXIulong doclen = 0;
  const char* doc = url->QueryArgsToMultipart(m_queryArgs,&doclen);
  HTUserProfile * up = HTRequest_userProfile(m_request);
  char * tmpfile = HTGetTmpFileName(HTUserProfile_tmp(up));
  char * tmpurl = HTParse(tmpfile, "file:", PARSE_ALL);
	    /*
	    **  Now create a new anchor for the post data and set up
	    **  the rest of the metainformation we know about this anchor. The
	    **  tmp anchor may actually already exist from previous postings.
	    */
  HTParentAnchor* postanchor = (HTParentAnchor *) HTAnchor_findAddress(tmpurl);
  HTAnchor_clearHeader(postanchor);
    // Who frees doc???
  HTAnchor_setDocument(postanchor, (void *) doc);
  HTAnchor_setLength(postanchor, doclen);
  HTAnchor_setFormat(postanchor, HTAtom_for(SB_MULTIPART));
  HTStream * target = SBHTStream(m_request, &m_chunk, MAX_CHUNK_SIZE);
  HTRequest_setOutputStream(m_request, target);
  // I think this does the right thing

//  if(HTPostAnchor(postanchor,anchor,m_request) == YES){
  if(OsPostData(postanchor, anchor, url) == TRUE){
    HT_FREE(tmpfile);
    HT_FREE(tmpurl);
  }
  else{
    HTChunk_delete(m_chunk);
    m_chunk = NULL;
  }
  return(m_chunk);
}


/*
 * Our own POST for multipart mime.
 */
UtlBoolean
SBinetHttpStream::OsPostData(HTParentAnchor* source, 
                             HTAnchor* destination, 
                             SBinetURL* url)
{
   UtlBoolean retval = TRUE;

   const char* absolute_url = url->GetAbsoluteNarrow();
   const char *contentType = HTAtom_name(HTAnchor_format(source));

   OsSysLog::add(FAC_HTTP, PRI_DEBUG,
                 "SBinetHttpStream::OsPostData absolute_url.data() = '%s'",
                 absolute_url);
   Url newUrl(absolute_url);
   UtlString s = newUrl.toString();
   OsSysLog::add(FAC_HTTP, PRI_DEBUG,
                 "SBinetHttpStream::OsPostData newUrl.toString() = '%s'",
                 s.data());

   // get doc for query args
   VXIulong doclen = 0;
   const char* doc = url->QueryArgsToMultipart(m_queryArgs,&doclen);

   HttpBody* body = new HttpBody( 
       doc,
       doclen, 
       contentType );

   HttpMessage request;
   UtlString    bytes;
   int         reqlen = 0;

   request.setBody(body);
   request.setRequestFirstHeaderLine(HTTP_POST_METHOD,
                                    newUrl.toString(),
                                    HTTP_PROTOCOL_VERSION);
   request.setContentType(contentType);
   request.setContentLength(body->getLength());
   request.getBytes(&bytes,&reqlen);

   HttpMessage response;
   int         len = 0;

   int ret = response.get(newUrl, request, m_nTimeoutOpen);
   if (DiagIsEnabled(MODULE_SBINET_TAGID))
       Diag (MODULE_SBINET_TAGID, (const VXIchar*)NULL, L"OsGetData get returns %d\n", ret);

   response.getBytes(&bytes,&len);
   response.parseMessage(bytes.data(), len);

   int contentLength = response.getContentLength();

   int i;
   VXIchar *out = NULL;
   VXIunsigned tagID = (unsigned int)MODULE_SBINET_STREAM_TAGID;
   if (DiagIsEnabled(tagID))
   {
   	Diag (tagID, (const VXIchar*)NULL, L"*********************************************\n");
   	Diag (tagID, (const VXIchar*)NULL, L"request bytes(%d) response bytes(%d) response contentLength(%d)\n", reqlen, len, contentLength);
   	out = new VXIchar[len + 1];
	if (out)
	{
   	   const char* str = bytes.data();
   	   for (i = 0; i < len; i++)
	      out[i] = str[i];
   	   out[len] = 0;
   	   Diag (tagID, (const VXIchar*)NULL, L"%s\n", out);
	}
   }

   UtlString contentTypeString;
   response.getContentType(&contentTypeString);

   int bodyLen = 0;
   const HttpBody *responseBody = response.getBody();
   if (responseBody) {
   	responseBody->getBytes(&bytes,&bodyLen);
	bytes = bytes.strip(UtlString::leading, '\r');
	bytes = bytes.strip(UtlString::leading, '\n');
	bytes = bytes.strip(UtlString::leading, '\r');
	bytes = bytes.strip(UtlString::leading, '\n');
   	if (DiagIsEnabled(tagID))
   	{
   	   Diag (tagID, (const VXIchar*)NULL, L"response body bytes(%d)\n", bodyLen);
	   if (out)
	   {
              const char* str = bytes.data();
              for (i = 0; i < bodyLen; i++)
                 out[i] = str[i];
              out[bodyLen] = 0;
   	      Diag (tagID, (const VXIchar*)NULL, L"%s\n", out);
            delete[] out;
            out = NULL;
	   }
   	   Diag (tagID, (const VXIchar*)NULL, L"*****************************************\n");
	}
   }

   HTRequest_addGnHd(m_request, HT_G_DATE);
   HTRequest_setEntityAnchor(m_request, source);
   HTRequest_setMethod(m_request, METHOD_POST);
   HTRequest_setAnchor(m_request, destination);

   if (bodyLen > 0) {
        // Use malloc() rather than 'new' here because 'data' will be freed by
        // HTChunk_delete, which uses free().
        char* data = (char*) malloc(bodyLen + 1);
   	memcpy(data, bytes.data(), bodyLen);
   	data[bodyLen] = 0;

      if (m_chunk)
         HTChunk_delete(m_chunk);
   	m_chunk = HTChunk_fromCString (data, bodyLen);
   }
   else retval = FALSE;

   m_HttpStatus = response.getResponseStatusCode();;
   m_Done = TRUE;

   return retval;
}

/*
 * POST simple form data. Don't know how to combine Form data and audio in multipart POST
 */
HTChunk*
SBinetHttpStream::Post(SBinetURL*       url,
		      VXIint32                  flags,
		      const VXIMap             *properties,
		      VXIMap                   *streamInfo)
{
  /* Get an anchor object for the URI */
  const char* absolute_url = url->GetAbsoluteNarrow();
  HTAnchor* anchor = HTAnchor_findAddress(absolute_url);
  //  HTRequest_setOutputFormat( m_request, HTAtom_for( m_submitMimeType ));

  // If we are a multipart MIME (ie sending audio) branch to special POST
  if(url->NeedMultipart(m_queryArgs)){
    return(PostMulti(url,flags,properties,streamInfo));
  }

  HTAssocList* arglist = url->QueryArgsToHtList(m_queryArgs);
  /*
   * get lock for anchor 
   */
  m_lock = SBinetCacheLockTable::FindCacheLock(anchor);
  if(m_lock == NULL){   // Should never fail
    Error (217, NULL);
    return(NULL);
  }
  /*
   * lock anchor -- Must unlock libwww while waiting to avoid deadlock
   */
  SBinetInterface::UnlockLibwww( );
  m_lock->GetReadLock(m_request);
  SBinetInterface::LockLibwww( );

  /* Post the data and get the result in a chunk */
  m_chunk = NULL;
  HTStream * target = SBHTStream(m_request, &m_chunk, MAX_CHUNK_SIZE);
  HTRequest_setOutputStream(m_request, target);
  if(arglist == NULL){ 
    // NULL arglist, converting to GET
    // PostFormAnchor will bomb, use LoadAnchor instead
    if(HTLoadAnchor(anchor, m_request) == NO){
      HTChunk_delete(m_chunk);
      m_chunk =  NULL;
    }
  }
  else{
    if(HTPostFormAnchor(arglist, anchor, m_request) == NULL){
      HTChunk_delete(m_chunk);
      m_chunk =  NULL;
    }
    HTAssocList_delete(arglist);
  }
  return(m_chunk);
}

int 
SBinetHttpStream::getTime(time_t *timestamp,
			  VXIunsigned *timestampMsec)
{
#ifdef WIN32
  struct _timeb tbuf;
  _ftime(&tbuf);
  *timestamp = tbuf.time;
  *timestampMsec = (VXIunsigned) tbuf.millitm;
#else
  struct timeb tbuf;
  ftime(&tbuf);
  *timestamp = tbuf.time;
  *timestampMsec = (VXIunsigned) tbuf.millitm;
#endif

  return 0;
}

int
SBinetHttpStream::checkTimeout()
{
  time_t currentTime;
  VXIunsigned currentTimeMsec;
  getTime( &currentTime, &currentTimeMsec ); 
  long elapsedMillis = ( currentTime - m_ReferenceTime ) * 1000L + 
    currentTimeMsec - m_ReferenceTimeMsec;
  if(elapsedMillis >= m_nTimeoutOpen){
    return(1);
  }
  return(0);
}


int
SBinetHttpStream::terminate_handler (HTRequest * request, 
				     HTResponse * response,
				     void * param, int status) 
{
    /* Check for status */
  SBinetHttpStream* str =   (SBinetHttpStream*)param;
  /* UNUSED VARIABLE HTParentAnchor* anc = */ HTRequest_anchor(request);

  /* we're not handling other requests */
  if(str && !str->m_Done && (status != 301) && (status != 302)){
    str->m_Done = 1;
    str->m_HttpStatus = status;
  }
  /* stop here */
  return HT_OK;
}

VXIinetResult
SBinetHttpStream::Open(VXIint32                  flags,
		       const VXIMap             *properties,
		       VXIMap                   *streamInfo)
{
  // Set up  properties instance
  SetProperties((VXIMap*)properties);

  // Determine if this is a conditional open with a validator,
  // irrelevant if caching is disabled
  const VXIValue* validatorVal = NULL;
  if (m_iMaxAge != 0) {
    validatorVal = VXIMapGetProperty( properties, INET_OPEN_IF_MODIFIED );
    if (validatorVal != NULL) {
      SBinetValidator validator(GetLog(), GetDiagBase());
      if (validator.Create(validatorVal) == VXIinet_RESULT_SUCCESS) {
	validator.Log(MODULE_SBINET_STREAM_TAGID, L"SBinetHttpStream::Open");
	
	// No need to fetch this if it has not expired, however still
	// have to return the validator in streamInfo
	if (! validator.Expired()) {
	  if (streamInfo) {
	    if (VXIMapSetProperty(streamInfo, INET_INFO_VALIDATOR,
				  VXIValueClone(validatorVal)) !=
		VXIvalue_RESULT_SUCCESS) {
	      Error(103, NULL);
	      return VXIinet_RESULT_OUT_OF_MEMORY;
	    }
	  }
	  return VXIinet_RESULT_NOT_MODIFIED;
	}
      }
    }
  }

  // Set up request
  SBinetInterface::LockLibwww( );
  getTime( &m_ReferenceTime, &m_ReferenceTimeMsec ); 

  m_request = HTRequest_new();
  m_Done = 0;
  m_HttpStatus = -1234;
  /* Store pointer so we can get at it later in callbacks */
  HTRequest_setContext(m_request,(void*) this);
  /* We want user output: no headers or strange length stuff */
  HTRequest_setOutputFormat(m_request, WWW_SOURCE);

  /* Close connection immediately */
  HTRequest_addConnection(m_request, (char*)"close", (char*)"");
  /* Add our own filter to handle termination */
  HTRequest_addAfter(m_request, SBinetHttpStream::terminate_handler, NULL, this, 
		     HT_ALL, HT_FILTER_LAST,NO);

  SetCachingMode();

  // Start request - get or post
  if(m_method == GET_METHOD){
    m_chunk = Get(m_url,flags,properties,streamInfo);
  }
  else if(m_method == POST_METHOD){
    m_chunk = Post(m_url,flags,properties,streamInfo);
  }

  SBinetInterface::UnlockLibwww( );

  if(!m_chunk){
    Error(218, L"%s%s%s%s", L"URL", m_url->GetAbsolute(), 
        L"Method", (m_method == GET_METHOD) ? L"GET" : L"POST");
    // clean up
    Close();
    return(VXIinet_RESULT_FAILURE);
  }

  // Wait for completion -- common
  //  Really should sleep on timer and wait for wakeup? Actually depends on cost of sleep/wakeup vs
  //    a little gratuitous spinning.
  int sleepTime = 1;
  while(!m_Done){
    if(checkTimeout()) {
      Error(228, L"%s%s%s%s%s%i", L"URL", m_url->GetAbsolute(), 
        L"Method", (m_method == GET_METHOD) ? L"GET" : L"POST",
        L"Delay", m_nTimeoutOpen);
      Close();
      return( VXIinet_RESULT_FETCH_TIMEOUT );
    }
    Sleep(sleepTime);
    // Rather than Sleep and Wake on timers, lets try increasing timeouts
    //  worse case, process should take 128ms more than necessary.
    if(sleepTime < 100) sleepTime *= 2;
  }
  if((m_HttpStatus < 0) || !((m_HttpStatus/100 == 2) || (m_HttpStatus == 304))){
    Error(219, L"%s%s%s%s%s%d", L"URL", m_url->GetAbsolute(), 
          L"Method", (m_method == GET_METHOD) ? L"GET" : L"POST",
          L"Error", m_HttpStatus);
    Close();  
    return(MapError(m_HttpStatus));
  }

  /*
   * Success
   */
  // Init for reading
  m_ReadSoFar = 0;
  SBinetInterface::LockLibwww( );
  // clean up -- we seem to be able to delete and flush in either order.
  //                flush calls the after filters, so delete first??
  HTRequest_deleteAfterAll(m_request);
  // Why do we flush?? -- If we don't we crash!!!
  HTRequest_forceFlush(m_request);

  // Get header info
  GetHeaderInfo(streamInfo);
  SBinetInterface::UnlockLibwww( );

  if((validatorVal != NULL) && (m_HttpStatus == 304)) {
    Close();
    return VXIinet_RESULT_NOT_MODIFIED;
  }

  return VXIinet_RESULT_SUCCESS;
}



//
// Read is the same for all HTTP requests (GET and POST)
//   At the moment, Read is not locks as merely copies data out of chunk
//
VXIinetResult
SBinetHttpStream::Read(VXIbyte                 *buffer,
		       VXIulong                 buflen,
		       VXIulong                *nread)
{
  if(nread != NULL) *nread = 0L;
  if(m_chunk == NULL) {
    Error(221, NULL);
    return VXIinet_RESULT_FAILURE;
  }
  int len = HTChunk_size(m_chunk);
  int toRead = buflen;
  int remaining = len - m_ReadSoFar;
  if(remaining <= 0) return(VXIinet_RESULT_END_OF_STREAM);
  if(toRead > remaining) toRead = remaining;
  char* data = HTChunk_data(m_chunk);
  if(data == NULL) {
    Error(220, NULL);
    return VXIinet_RESULT_FAILURE;
  }
  data += m_ReadSoFar;
  ::memcpy( (void*)buffer, (void*)data, toRead );
  m_ReadSoFar += toRead;
  if(nread != NULL) *nread = toRead;
  return(VXIinet_RESULT_SUCCESS);
}


VXIinetResult
SBinetHttpStream::Close()
{
  // If no request (multiple calls to Close() are allowed), 
  // check lock to make sure.
  if(m_request == NULL){
    if(m_lock){
      m_lock->FreeReadLock(m_request);   // free lock on anchor
      m_lock = NULL;
      return(VXIinet_RESULT_SUCCESS);
    }
  }
  /* Clean up the request 
   *  Note: we might want to do the delete in the destructor to give libwww extra time
   *   to settle down
   */
  SBinetInterface::LockLibwww( );
  if(m_chunk){
    HTChunk_delete(m_chunk);
    m_chunk = NULL;
  }
  if(m_request){
    HTRequest_kill( m_request );
    HTRequest_delete(m_request);
    m_request = NULL;
  }
  if(m_lock){
    m_lock->FreeReadLock(m_request);   // free lock on anchor
    m_lock = NULL;
  }
  SBinetInterface::UnlockLibwww( );

  /*
   * Might need to do Sleep here to let libwww timers settle out
   */
  return(VXIinet_RESULT_SUCCESS);
}

SBinetHttpStream::SBinetHttpStream(SBinetURL* url, SBinetChannel* ch,
                          VXIlogInterface *log, VXIunsigned diagLogBase):
  SBinetLogger(MODULE_SBINET_STREAM, log, diagLogBase), 
  m_submitMimeType(), m_content_type()
{
  m_request = NULL;
  m_chunk = NULL;
  m_argstring = L"";  // string representation of query ares for GET
  m_queryArgs = NULL;
  m_Done = 0;
  m_ReadSoFar = 0;
  m_HttpStatus = 0;    

  m_url = url;
  m_iMaxAge = -1;
  m_iMaxStale = 0;
  m_nTimeoutOpen = 0;
  m_nTimeoutIO = 0;
  m_method = GET_METHOD;
  m_newAnchor = NULL;
  m_lock = NULL;
  m_content_length = 0;
  m_reader = 2;
  m_ch = ch;

  SBinetInterface::LockLibwww( );
  sm_instanceCount++;
  SBinetInterface::UnlockLibwww( );

  OsSysLog::add(FAC_HTTP, PRI_DEBUG,
                "SBinetHttpStream::SBinetHttpStream(instance='%d') m_url = '%ls'",
                sm_instanceCount, m_url->GetAbsolute());
}

SBinetHttpStream::~SBinetHttpStream()
{
  Close();

  int instance;
  static int onceInaWhileTrigger = 0;
  SBinetInterface::LockLibwww( );
  instance = sm_instanceCount--;
  // If sm_cleanupPeriod is 0 - do not ever cleanup
  if (sm_cleanupPeriod && (sm_instanceCount == 0)) {
    onceInaWhileTrigger = (onceInaWhileTrigger+1)%sm_cleanupPeriod;
    if (!onceInaWhileTrigger) {
      HTAnchor_deleteAll(NULL);
      OsSysLog::add(FAC_HTTP, PRI_DEBUG,
		    "SBinetHttpStream::~SBinetHttpStream(instance='%d') m_url = '%ls' - perform HTAnchor cleanup",
		    instance, m_url->GetAbsolute());
    }
  }
  SBinetInterface::UnlockLibwww( );

  OsSysLog::add(FAC_HTTP, PRI_DEBUG,
                "SBinetHttpStream::~SBinetHttpStream(instance='%d') m_url = '%ls'",
                instance, m_url->GetAbsolute());

  if(m_url) delete m_url;
  m_url = NULL;
}

void
SBinetHttpStream::SetProperties(VXIMap* properties)
{
  const VXIString* strCachingMode = 
    (const VXIString*)VXIMapGetProperty( properties, INET_CACHING );
  const VXIInteger* intMaxAge = 
    (const VXIInteger*)VXIMapGetProperty( properties, 
					  INET_CACHE_CONTROL_MAX_AGE );
  const VXIInteger* intMaxStale = 
    (const VXIInteger*)VXIMapGetProperty( properties, 
					  INET_CACHE_CONTROL_MAX_STALE );

  // Cache control, INET_CACHING overrides the individual settings
  m_iMaxAge = -1;
  m_iMaxStale = 0;
  if ( intMaxAge != NULL ) 
    m_iMaxAge = VXIIntegerValue(intMaxAge);
  if ( intMaxStale != NULL )
    m_iMaxStale = VXIIntegerValue(intMaxStale);
  if (( strCachingMode != NULL ) && 
      ( VXIStringCompareC( strCachingMode, INET_CACHING_SAFE  ) == 0 )) {
    m_iMaxAge = 0;
    m_iMaxStale = 0;
  }

  // Open timeout
  m_nTimeoutOpen = INET_TIMEOUT_OPEN_DEFAULT * 10; // TBD wrong!
  VXIInteger *timeoutOpen = 
    (VXIInteger *)VXIMapGetProperty( properties, INET_TIMEOUT_OPEN );
  if( timeoutOpen != NULL ) 
    m_nTimeoutOpen = VXIIntegerValue( timeoutOpen );

  // IO timeout
  m_nTimeoutIO = INET_TIMEOUT_IO_DEFAULT;
  VXIInteger *timeoutIO = 
    (VXIInteger *)VXIMapGetProperty( properties, INET_TIMEOUT_IO );
  if( timeoutIO != NULL ) 
    m_nTimeoutIO = VXIIntegerValue( timeoutIO );

  // Get the submit MIME type from the properties, if defined
  const VXIchar* wType = INET_SUBMIT_MIME_TYPE_DEFAULT;
  VXIString *strType = 
    (VXIString *)VXIMapGetProperty( properties, INET_SUBMIT_MIME_TYPE );
  if ( strType ) wType = VXIStringCStr( strType );

  m_submitMimeType = "";
  size_t len = ::wcslen( wType );
  for (size_t i = 0; i < len; i++)
    m_submitMimeType += (char) wType[i];

  // Get the SUBMIT method from the properties, if defined
  wType = INET_SUBMIT_METHOD_DEFAULT;
  m_method = GET_METHOD;
  strType = (VXIString *)VXIMapGetProperty( properties, INET_SUBMIT_METHOD );
  if ( strType ) {
    wType = VXIStringCStr( strType );
    if ( !::wcscmp( wType, INET_SUBMIT_METHOD_GET  ))
      m_method = GET_METHOD;
    else if ( !::wcscmp( wType, INET_SUBMIT_METHOD_POST  ))
      m_method = POST_METHOD;
  }

  // Get the query arguments, if defined
  m_queryArgs = (VXIMap *)VXIMapGetProperty( properties, INET_URL_QUERY_ARGS );
}


VXIinetResult SBinetHttpStream::SetCachingMode( )
{
    if ( m_iMaxAge == 0 )
    {
       // We always want to check Modified and Validator
       HTRequest_addRqHd(m_request, HT_C_IF_NONE_MATCH );
       HTRequest_addRqHd(m_request, HT_C_IMS);
       // We may have to use _VALIDATE rather then _END_VALIDATE  for
       //  the non-proxy case to avoid gratuitous downloads
       HTRequest_setReloadMode( m_request, HT_CACHE_END_VALIDATE );
    }
    else
    {
       HTRequest_setReloadMode( m_request, HT_CACHE_OK );
    }
    return VXIinet_RESULT_SUCCESS;
}


void 
SBinetHttpStream::GetHeaderInfo(VXIMap* streamInfo){
  HTParentAnchor* anchor      = HTRequest_anchor(m_request);
  int nStatus = m_HttpStatus;

  // Check if the HTTP return status is 200 (OK) or 304 (NOT_MODIFIED)
  if (( streamInfo != NULL ) && (( nStatus == 200 ) || ( nStatus == 304 ))){
    // Set HTTP status
    VXIMapSetProperty(streamInfo, INET_INFO_HTTP_STATUS,
			  (VXIValue*)VXIIntegerCreate( nStatus ));

    // Set the ABSOLUTE_NAME property to the URL returned by the server
    char *serverURL = HTAnchor_address((HTAnchor *)anchor);
    if(( serverURL != NULL ) && ( serverURL[0] != '\0' )){
	VXIint len = ::strlen( serverURL ) + 1;
	VXIchar *wServerURL = new VXIchar [len];
	::mbstowcs( wServerURL, serverURL, len );
	VXIMapSetProperty(streamInfo, INET_INFO_ABSOLUTE_NAME, 
			  (VXIValue*)VXIStringCreate( wServerURL ));
	delete [] wServerURL;
	wServerURL = NULL;
    }
    HT_FREE(serverURL); // HTAnchor_address does MALLOC or copy

    // Get relevant response fields
    m_content_length = HTAnchor_length(anchor);
    m_content_type = L"";

    VXIbool haveValidator = FALSE;
    SBinetValidator validator(GetLog(), GetDiagBase());
    if (validator.Create(m_url->GetAbsolute(), HTAnchor_lastModified(anchor),
			 HTAnchor_expires(anchor), m_content_length,
			 HTAnchor_etag(anchor), HTAnchor_header(anchor)) 
             == VXIinet_RESULT_SUCCESS) {
      validator.Log(MODULE_SBINET_STREAM_TAGID, 
		    L"SBinetHttpStream::GetHeaderInfo");
      haveValidator = TRUE;
    }

    const char *contentType = HTAtom_name(HTAnchor_format(anchor));
    if (( contentType ) && ( contentType[0] ) &&
	( ::strcmp(contentType, DEFAULT_GENERIC_MIME_TYPE_N) != 0 )) {
      size_t len = ::strlen(contentType);
      VXIchar *tmp = 0;
      if (len > 0)
      {
         tmp = new VXIchar[len + 1];
      }
      if (!tmp) Error (301, L"%s%s", L"url", m_url->GetAbsolute());;

      for (size_t i = 0; i < len; i++)
         tmp[i] = (VXIchar) contentType[i];
      tmp[len] = 0;
      m_content_type = SBinetString(tmp);
      if (tmp) 
      { 
         delete[] tmp;
         tmp = 0;
      }
    } else {
      VXIinetResult rc = m_url->ContentTypeFromUrl(&m_content_type);
      if(rc == VXIinet_RESULT_SUCCESS) {
        Error (300, L"%s%s%s%s", L"url", m_url->GetAbsolute(), 
	       L"mime", m_content_type.c_str());
      } else {
	    m_content_type = DEFAULT_GENERIC_MIME_TYPE;
        Error (301, L"%s%s", L"url", m_url->GetAbsolute());
      }
    }

    // Fill properties 
    VXIMapSetProperty( streamInfo, INET_INFO_SIZE_BYTES, 
		       (VXIValue*)VXIIntegerCreate(m_content_length));

    VXIMapSetProperty( streamInfo, INET_INFO_MIME_TYPE, 
		       (VXIValue*)VXIStringCreate(m_content_type.c_str()));

    if (haveValidator)
      VXIMapSetProperty(streamInfo, INET_INFO_VALIDATOR, 
			(VXIValue*)validator.Serialize());
    
  } // Ends if (streamInfo && HTTP200/304)
  
}

VXIinetResult 
SBinetHttpStream::MapError(int ht_error)
{
  switch(ht_error)
    {
    case HT_NO_ACCESS: /* Unauthorized */
    case HT_FORBIDDEN: /* Access forbidden */
    case HT_NOT_ACCEPTABLE:/* Not Acceptable */
    case HT_NO_PROXY_ACCESS:    /* Proxy Authentication Failed */
    case HT_CONFLICT:    /* Conflict */
    case HT_LENGTH_REQUIRED:    /* Length required */
    case HT_PRECONDITION_FAILED:    /* Precondition failed */
    case  HT_TOO_BIG:    /* Request entity too large */
    case HT_URI_TOO_BIG:    /* Request-URI too long */
    case HT_UNSUPPORTED:    /* Unsupported */
    case HT_BAD_RANGE:    /* Request Range not satisfiable */
    case HT_EXPECTATION_FAILED:    /* Expectation Failed */
    case HT_REAUTH:    /* Reauthentication required */
    case HT_PROXY_REAUTH:    /* Proxy Reauthentication required */
    case HT_RETRY:	/* If service isn't available */
    case HT_BAD_VERSION:	/* Bad protocol version */
      return VXIinet_RESULT_FETCH_ERROR;

    case HT_INTERNAL:    /* Weird -- should never happen. */
      return VXIinet_RESULT_NON_FATAL_ERROR;

    case  HT_WOULD_BLOCK:    /* If we are in a select */
      return VXIinet_RESULT_WOULD_BLOCK;

    case HT_INTERRUPTED:    /* Note the negative value! */
    case HT_PAUSE:    /* If we want to pause a stream */
    case HT_RECOVER_PIPE:    /* Recover pipe line */
      return VXIinet_RESULT_SUCCESS;

    case HT_TIMEOUT:    /* Connection timeout */
      return VXIinet_RESULT_FETCH_TIMEOUT;

    case HT_NOT_FOUND: /* Not found */
    case HT_NO_HOST:    /* Can't locate host */
      return VXIinet_RESULT_NOT_FOUND;
    default:
      return VXIinet_RESULT_NON_FATAL_ERROR;
    }
}


/*
 * Return one if there is a non-expired cache copy and we can use it without validation
 */
///// The only call to this has been commented out...
/// static
/// int CacheValid (HTRequest * request)
/// {
///     HTParentAnchor * anchor = HTRequest_anchor(request);
///     char * default_name = HTRequest_defaultPutName (request); 
///     HTCache * cache = NULL;
///     HTReload reload = HTRequest_reloadMode(request);
///     HTMethod method = HTRequest_method(request);
///     /* UNUSED VARIABLE HTDisconnectedMode disconnect = */ HTCacheMode_disconnected();
///     /*
///     **  If the cache is disabled all together then it won't help looking, huh?
///     */
///     if (!HTCacheMode_enabled()) return(0);
///     /*
///     **  Now check the cache...
///     */
///     if (method != METHOD_GET) {
///       return(0);
///     } else if (reload == HT_CACHE_FLUSH) {
///       return(0);
///     } else {
/// 	/*
/// 	** Check the persistent cache manager. If we have a cache hit then
/// 	** continue to see if the reload mode requires us to do a validation
/// 	** check. This filter assumes that we can get the cached version
/// 	** through one of our protocol modules (for example the file module)
/// 	*/
/// 	cache = HTCache_find(anchor, default_name);
/// 	if (cache) {
/// 	    HTReload cache_mode = HTCache_isFresh(cache, request);
/// 	    if (cache_mode == HT_CACHE_ERROR) cache = NULL;
/// 	    reload = HTMAX(reload, cache_mode);
/// 	    /*
/// 	    **  Now check the mode and add the right headers for the validation
/// 	    **  If we are to validate a cache entry then we get a lock
/// 	    **  on it so that not other requests can steal it.
/// 	    */
/// 	    if (reload == HT_CACHE_RANGE_VALIDATE) {
/// 	      return(0);
/// 	    } else if (reload == HT_CACHE_END_VALIDATE) {
/// 	      return(0);
/// 	    } else if (reload == HT_CACHE_VALIDATE) {
/// 	      return(0);
/// 	    } else if (cache) {
/// 		/*
/// 		**  The entity does not require any validation at all. We
/// 		**  can just go ahead and get it from the cache. In case we
/// 		**  have a fresh subpart of the entity, then we issue a 
/// 		**  conditional GET request with the range set by the cache
/// 		**  manager. Issuing the conditional range request is 
/// 		**  equivalent to a validation as we have to go out on the
/// 		**  net. This may have an effect if running in disconnected
/// 		**  mode. We disable all BEFORE filters as they don't make
/// 		**  sense while loading the cache entry.
/// 		*/
/// 	      return(1);
/// 	    }
/// 	}
///     }
///     return(0);
/// }

