/****************License************************************************
 *
 * Copyright 2000-2001.  SpeechWorks International, Inc.  
 *
 * Use of this software is subject to notices and obligations set forth
 * in the SpeechWorks Public License - Software Version 1.1 which is
 * included with this software.
 *
 * SpeechWorks is a registered trademark, and SpeechWorks Here, 
 * DialogModules and the SpeechWorks logo are trademarks of SpeechWorks 
 * International, Inc. in the United States and other countries. 
 * 
 ************************************************************************
 *
 * 
 *
 ************************************************************************
 */

#ifndef _VXIINET_H
#define _VXIINET_H

#include "VXIvalue.h"                  /* For VXIMap, VXIString, etc. */

#include "VXIheaderPrefix.h"
#ifdef VXIINET_EXPORTS
#define VXIINET_API SYMBOL_EXPORT_DECL
#else
#define VXIINET_API SYMBOL_IMPORT_DECL
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
struct VXIinetStream;
#else
typedef struct VXIinetStream { void * dummy; } VXIinetStream;
#endif


/**
 * @name VXIinet
 * @memo Internet Interface
 * @version 1.0
 * @doc
 * Abstract interface for accessing Internet functionality including
 * HTTP requests, local file access, URL caching, and cookie access. <p>
 *
 * The interface is a synchronous interface based on the ANSI/ISO C
 * standard file I/O interface, the only exception is that pre-fetches are
 * asynchronous. The client of the interface may use this in an
 * asynchronous manner by using non-blocking I/O operations, creating
 * threads, or by invoking this from a separate server process. <p>
 * 
 * There is one Internet interface per thread/line. 
 */

/*@{*/


/**
 * @name VXIinet argument properties
 * @memo Keys identifying properties in VXIMap for Prefetch( ), Open( ),
 * and Unlock( ). 
 * @doc VXIinet functions take a VXIMap argument which contains a set of
 * key/value pairs. The listed arguments must be supported by an 
 * implementation. Additional arguments can be added to this argument in
 * other implementations. Time durations are specified in milliseconds, see
 * below for valid values for the enumerated type properties.
 */

/*@{*/
    /** Value for the HTTP 1.1 Cache-Control max-age directive for
	requests. This specifies the client is willing to accept a
	cached object no older then this value (given in seconds). A
	value of 0 may be used to force re-validating the cached copy
	with the origin server for every request. In most cases, this
	property should not be present, thus allowing the origin
	server to control expiration. Value is a VXIInteger */
#define INET_CACHE_CONTROL_MAX_AGE       L"inet.maxage"

    /** Value for the HTTP 1.1 Cache-Control max-stale directive for
	requests. This specifies the client is willing to accept a
	cached object that is expired by up to this value (given in
	seconds) past the expiration time specified by the origin
	server. In most cases, this property should be set to 0 or not
	present, thus respecting the expiration time specified by the
	origin server. Value is a VXIInteger */
#define INET_CACHE_CONTROL_MAX_STALE     L"inet.maxstale"

    /** Type of caching to apply: safe or fast. See the INET_CACHING
	defines.  NOTE: Supported for backward compatibility only, use
	INET_CACHE_CONTROL_MAX_AGE instead ("safe" mode is identical
	to setting that parameter to 0, while "fast" mode is identical
	to leaving that parameter unspecified).  Value is a VXIString */
#define INET_CACHING                     L"inet.caching"

    /** Conditional open, used for cases where a cache outside of
      VXIinet is being maintained and the desired object is already
      present in that cache, but the user needs to verify whether
      that cached object is valid for reuse or not. (For example,
      maintaining a compiled grammar cache where the grammar source
      is obtained from a URL.) To do so, when writing data to the 
      cache the user must also store the value of the 
      INET_INFO_VALIDATOR return property. Then for the next request
      for that data, the user must set this property to that validator
      object and do an Open( ). If VXIinet_RESULT_NOT_MODIFIED is
      returned, the user may re-use the cached data, but must also
      update the associated validator with the updated validator
      returned in INET_INFO_VALIDATOR. Otherwise if
      VXIinet_RESULT_SUCCESS is returned, the cached entry is invalid
      and the user obtains a stream from which they must re-create 
      (re-compile) the data.  (A simple case to understand is file://
      access: the validator will usually just be the modification time,
      if that changes the cached data is invalid. The need for an opaque
      validator and this conditional open mechanism is driven by HTTP 1.1,
      where the validator is far more complex, a combination of expiration
      times, a modification date, entity tags, and other data.)  Value is 
      a VXIContent as returned for INET_INFO_VALIDATOR */
#define INET_OPEN_IF_MODIFIED   L"inet.openIfModified" 

    /** Whether to open local files normally or to return a 
      VXIinet_RESULT_LOCAL_FILE error while still returning the
      stream information if requested.  Value is a VXIInteger where
      it is set to TRUE (1) or FALSE (0). */
#define INET_OPEN_LOCAL_FILE    L"inet.openLocalFile"

    /** Prefetch priority. For implementations supporting priority
      this controls the order of opens and reads for multiple current
      reads.  Argument is a VXIInteger */
#define INET_PREFETCH_PRIORITY  L"inet.prefetchPriority"

    /** Submit method: GET or POST.  Argument is a VXIString.
        Default is INET_SUBMIT_METHOD_DEFAULT. */
#define INET_SUBMIT_METHOD      L"inet.submitMethod"    

    /** Submit MIME type. MIME type of data sent in the submit. Default 
        is INET_SUBMIT_MIME_TYPE_DEFAULT.  Argument is a VXIString */
#define INET_SUBMIT_MIME_TYPE   L"inet.submitMimeType"   

    /** Open timeout. Amount of time in (ms) to wait for an open to
      succeed before abadoning the open.  Default is
      INET_TIMEOUT_OPEN_DEFAULT Argument is a VXIInteger */
#define INET_TIMEOUT_OPEN       L"inet.timeoutOpen"            

    /** Read timeout.  Time in (ms) to attempt to read on the socket before
      abadoning the read. Default is INET_TIMEOUT_IO_DEFAULT. Argument is a
      VXIInteger */
#define INET_TIMEOUT_IO         L"inet.timeoutIO"              

    /** Total download timeout.  Time in (ms) to attempt to open and read
      all the contents on the socket before issuing a read.  This is 
      optional and should be used to implement the VoiceXML timeout.
      Argument is a VXIinteger */
#define INET_TIMEOUT_DOWNLOAD         L"inet.timeoutDownload"              
    
    /** URL base for resolving relative URLs.  Note this is not a
      directory name, it is the full URL to the document that refers to
      this URL being fetched. No default.  Argument is a VXIString */
#define INET_URL_BASE           L"inet.urlBase"               

    /** URL Query Arguments.  Argument is a VXIMap containing zero or more
      key/value pairs.  When specified a submit is done as controlled
      by the INET_SUBMIT_METHOD and INET_SUBMIT_MIME_TYPE properties, for
      example doing a POST where the key/value pairs are appended to the
      URL in order to perform a query.  Only valid for INET_MODE_READ.
      By default is undefined (no submit performed). */
#define INET_URL_QUERY_ARGS     L"inet.urlQueryArgs"
/*@}*/


/**
 * @name INET_CACHING 
 * @memo VXIinet caching property values.
 * @doc Set of defined caching property values that are used to control 
 * when to retrieve information from the cache versus when to do a fetch.
 */
/*@{*/
    /** Safe caching, follows VoiceXML 1 safe fetchhint property.
      Safe caching is the equivalent of end-to-end caching expiration
      checking.  The INET component will open a socket and check the
      document on the web server, even if the cache has not expired
      with safe caching */
#define INET_CACHING_SAFE       L"safe"

    /** Fast caching, follows VoiceXML 1 fast fetchhint property.
      Fast caching will check the cache expiration of a document
      before opening a socket to the web server. If the document has
      not expired in the cache it will simply be returned.
      */
#define INET_CACHING_FAST       L"fast"
/*@}*/

/** 
 * INET_PREFETCH_PRIORITY property values.
 */
typedef enum VXIinetPrefetchPriority {
  /** Caller is waiting for the document */
  INET_PREFETCH_PRIORITY_CRITICAL    =   40,
  /** Caller is going to get to this real soon */
  INET_PREFETCH_PRIORITY_HIGH        =   30,
  /** Caller is likely to need this in a little bit */
  INET_PREFETCH_PRIORITY_MEDIUM      =   20,
  /** Just initializing the system, no callers yet */
  INET_PREFETCH_PRIORITY_LOW         =   10
} VXIinetPrefetchPriority;

/**
 * @name INET_SUBMIT_METHOD 
 * @memo INET_SUBMIT_METHOD supported property values.  PUT and DELETE
 are not supported.
 * @doc Full HTTP 1.1 support could be added to this interface. The
 current interface only supports the two methods commonly used for
 sending data back to a web server GET and POST.
 */
/*@{*/
  /** HTTP GET */
#define INET_SUBMIT_METHOD_GET   L"GET"
  /** HTTP POST */
#define INET_SUBMIT_METHOD_POST  L"POST"
/*@}*/

/**
 * @name VXIinet property defaults
 * @memo Default values for properties in the VXIMap argument for
 Prefetch(), Open() and Unlock().  
 * @doc If the properties are not set in the call to these function
 the given default value will be assumed.
 */
/*@{*/
   /** Cache Control max-age Default - NULL not present by default */
#define INET_CACHE_CONTROL_MAX_AGE_DEFAULT    NULL

   /** Cache Control max-stale Default - 0, do not use expired entries */
#define INET_CACHE_CONTROL_MAX_STALE_DEFAULT  0

   /** Caching Default - INET_CACHING_FAST */
#define INET_CACHING_DEFAULT                  INET_CACHING_FAST

   /** Open If Modified Default - NULL not present by default */
#define INET_OPEN_IF_MODIFIED_DEFAULT         NULL  

   /** Open Local File Default - TRUE */
#define INET_OPEN_LOCAL_FILE_DEFAULT          TRUE

   /** Prefetch priority default - INET_PREFETCH_PRIORITY_LOW */
#define INET_PREFETCH_PRIORITY_DEFAULT        INET_PREFETCH_PRIORITY_LOW

   /** Submit method default - INET_SUBMIT_METHOD_GET */
#define INET_SUBMIT_METHOD_DEFAULT            INET_SUBMIT_METHOD_GET

   /** Submit MIME type default - "application/x-www-form-urlencoded" */
#define INET_SUBMIT_MIME_TYPE_DEFAULT      L"application/x-www-form-urlencoded"

   /** Open timeout default - 5000 (5 seconds) */
#define INET_TIMEOUT_OPEN_DEFAULT             5000

   /** Read timeout default - 5000 (5 seconds) */
#define INET_TIMEOUT_IO_DEFAULT               5000

   /** URL base default - "" (no base) */
#define INET_URL_BASE_DEFAULT                 L""

   /** URL query arguments default - NULL not present by default */
#define INET_URL_QUERY_ARGS_DEFAULT           NULL
/*@}*/

/**
 * Mode values for Open( )
 */
typedef enum VXIinetOpenMode { 
  /** Open for reading, for http:// access this corresponds to a GET
   *  or POST operation (GET in most cases, POST if
   *  INET_URL_QUERY_ARGS is specified and INET_SUBMIT_METHOD is set
   *  to POST)
   */
  INET_MODE_READ    = 0x0,

  /** Open for writing, for http:// access this corresponds to a PUT
   *  operation.  Note that this is OPTIONAL functionality that most
   *  implementations do not support (because most Web servers do not
   *  support PUT operations for security reasons).
   */
  INET_MODE_WRITE   = 0x1
} VXIinetOpenMode;

/**
 * @name Open flags
 * @memo Flags for Open( ), may be combined through bitwise or.
 * @doc The Open() call takes a bitwise or of open flags which control
 * the behavior of the returned stream.
 */
/*@{*/
  /** Null flag.  This causes the cache to use default behavior,
      specifically I/O using blocking operations. */
#define INET_FLAG_NULL             0x0
  /** Non-blocking reads/writes. Do all I/O using non-blocking operations. */
#define INET_FLAG_NONBLOCKING_IO   0x8    
/*@}*/

/**
 * @name Open return properties
 * @memo Keys identifying properties in VXIMap used to return stream
 * information for Open( ).
 * @doc The VXIinet implementation determines information about the
 file or URI when it opens the URI. These are returned as key/value
 pairs through a VXIMap. Some values are not guaranteed to be returned
 after an open, see below.
 */
/*@{*/
  /** Absolute Name, always returned.  The absolute URI for a URI
    which may have been provided as a relative URI against a base.
    For local file access (file:// access or an OS dependant path) an
    OS dependant path must be returned, never a file:// URI.  This
    should be passed unmodified as the value of the INET_URL_BASE
    property for fetching URIs referenced within this document, if any.
    Returned as a VXIString */
#define INET_INFO_ABSOLUTE_NAME  L"inet.info.absoluteName" 

  /** MIME type, always returned.  The MIME type of the URI.  
    For HTTP requests, this is set to the type returned by the 
    HTTP server. For file: requests, a MIME mapping table is used 
    to determine the correct MIME type. This table is also used
    when the HTTP server returns no MIME type or a generic type. 
    If the MIME type cannot be determined at all, it is set to 
    "application/octet-stream". Returned as a VXIString */
#define INET_INFO_MIME_TYPE      L"inet.info.mimeType"

  /** Size in bytes, always returned.  Size of the file in bytes.  
    For HTTP requests, this is set to the size returned by the
    HTTP server. If the server returns no size, or if the file 
    size cannot be determined for file: requests, it is set to 
    zero. Returned as a VXIInteger */
#define INET_INFO_SIZE_BYTES     L"inet.info.sizeBytes"    

  /** Validator, always returned on a successful Open( ) or
    when INET_OPEN_IF_MODIFIED was specified and 
    VXIinet_RESULT_NOT_MODIFIED was returned.  Opaque validator 
    for future conditional open operations against the named object,
    see INET_OPEN_IF_MODIFIED for details.  Returned as a VXIContent */
#define INET_INFO_VALIDATOR      L"inet.info.validator"
/*@}*/

/**
 * @name INET_COOKIE
 * @memo Cookie jar properties
 *
 * @doc
 *
 * Cookie jars are represented by a VXIVector. Each element of the
 * vector is a VXIMap that represents a cookie. The cookie VXIMap will
 * contain zero or more properties, each of which represent properties
 * of the cookie.
 *
 * The properties of the cookie VXIMap match the cookie attribute
 * names as defined in RFC 2965. The only exceptions are as follows:
 *
 *  - INET_COOKIE_NAME, name of the cookie as defined in RFC 2965
 *  - INET_COOKIE_VALUE, value of the cookie as defined in RFC 2965
 *  - INET_COOKIE_EXPIRES, expiration time for the cookie as calculated
 *        off the MaxAge parameter defined in RFC 2965 when the cookie
 *        is accepted
 *  - RFC 2965 Discard attribute: will never be returned in the VXIMap,
 *        cookies with this flag set will never be returned by
 *        GetCookieJar( )
 */
/*@{*/
  /** Cookie name.  Value of the key is a VXIString. */
#define INET_COOKIE_NAME         L"inet.cookie.NAME"  

  /** Cookie value key.  Value of the key is a VXIString. */
#define INET_COOKIE_VALUE        L"inet.cookie.VALUE"   /* VXIString  */

  /** Cookie expires key, calculated off the Max-Age property for the
      cookie.  Value of the key is a VXIInteger giving time since the
      epoch for expiration. */
#define INET_COOKIE_EXPIRES      L"inet.cookie.EXPIRES"

  /** Cookie comment, optional.  Value of the key is a VXIString. */
#define INET_COOKIE_COMMENT      L"inet.cookie.Comment"  

  /** Cookie comment URL, optional.  Value of the key is a VXIString. */
#define INET_COOKIE_COMMENT_URL  L"inet.cookie.CommentURL"  

  /** Cookie domain key.  Value of the key is a VXIString. */
#define INET_COOKIE_DOMAIN       L"inet.cookie.Domain"  

  /** Cookie path key.  Value of the key is a VXIString. */
#define INET_COOKIE_PATH         L"inet.cookie.Path" 

  /** Cookie port key, optional.  Value of the key is a VXIInteger. */
#define INET_COOKIE_PORT         L"inet.cookie.Port" 

  /** Cookie secure key, optional.  Value of the key is a VXIInteger set
      to 0 (FALSE) or 1 (TRUE). */
#define INET_COOKIE_SECURE       L"inet.cookie.Secure"

  /** Cookie standard version.  Value of the key is a VXIInteger. */
#define INET_COOKIE_VERSION      L"inet.cookie.Version"

/*@}*/

/**
 * @name VXIinetResult
 * @memo Result codes for interface methods
 * 
 * @doc
 * Result codes less then zero are severe errors (likely to be
 * platform faults), those greater then zero are warnings (likely to
 * be application issues) 
 */
typedef enum VXIinetResult {
  /** Fatal error, terminate call    */
  VXIinet_RESULT_FATAL_ERROR       =  -100, 
  /** I/O error                      */
  VXIinet_RESULT_IO_ERROR           =   -8,
  /** Out of memory                  */
  VXIinet_RESULT_OUT_OF_MEMORY      =   -7, 
  /** System error, out of service   */
  VXIinet_RESULT_SYSTEM_ERROR       =   -6, 
  /** Errors from platform services  */
  VXIinet_RESULT_PLATFORM_ERROR     =   -5, 
  /** Return buffer too small        */
  VXIinet_RESULT_BUFFER_TOO_SMALL   =   -4, 
  /** Property name is not valid    */
  VXIinet_RESULT_INVALID_PROP_NAME  =   -3, 
  /** Property value is not valid   */
  VXIinet_RESULT_INVALID_PROP_VALUE =   -2, 
  /** Invalid function argument      */
  VXIinet_RESULT_INVALID_ARGUMENT   =   -1, 
  /** Success                        */
  VXIinet_RESULT_SUCCESS            =    0,
  /** Normal failure, nothing logged */
  VXIinet_RESULT_FAILURE            =    1,
  /** Non-fatal non-specific error   */
  VXIinet_RESULT_NON_FATAL_ERROR    =    2, 
  /** Named data not found           */
  VXIinet_RESULT_NOT_FOUND          =   50, 
  /** URL fetch timeout              */
  VXIinet_RESULT_FETCH_TIMEOUT      =   51,
  /** Other URL fetch error          */
  VXIinet_RESULT_FETCH_ERROR        =   52,
  /** I/O operation would block      */
  VXIinet_RESULT_WOULD_BLOCK        =   53,
  /** End of stream                  */
  VXIinet_RESULT_END_OF_STREAM      =   54,
  /** Local file, told not to open it */
  VXIinet_RESULT_LOCAL_FILE         =   55,
  /** Conditional open attempted, cached
      data may be used               */
  VXIinet_RESULT_NOT_MODIFIED       =   57,
  /** Operation is not supported     */
  VXIinet_RESULT_UNSUPPORTED        =  100
} VXIinetResult;

/*
** ==================================================
** VXIinetInterface Interface Definition
** ==================================================
*/
/** @name VXIinetInterface
 ** @memo VXIinet interface for URL fetching and posting
 ** @doc VXIinetInterface provides the URI fetch functions required by
 all the OpenVXI browser components.  The ability to prefetch
 and read URIs along with cookie management is provided.
 */

typedef struct VXIinetInterface {
  /**
   * @name GetVersion
   * @memo Get the VXI interface version implemented
   *
   * @return  VXIint32 for the version number. The high high word is 
   *          the major version number, the low word is the minor version 
   *          number, using the native CPU/OS byte order. The current
   *          version is VXI_CURRENT_VERSION as defined in VXItypes.h.
   */ 
  VXIint32 (*GetVersion)(void);
  
  /**
   * @name GetImplementationName
   * @memo Get the name of the implementation
   *
   * @return  Implementation defined string that must be different from
   *          all other implementations. The recommended name is one
   *          where the interface name is prefixed by the implementator's
   *          Internet address in reverse order, such as com.xyz.rec for
   *          VXIrec from xyz.com. This is similar to how VoiceXML 1.0
   *          recommends defining application specific error types.
   */
  const VXIchar* (*GetImplementationName)(void);
  
  /**
   * @name Prefetch
   * @memo Prefetch information (non-blocking)
   *
   * @doc
   * Note that this is optional, it does not need to be called prior
   * to Open( ). <p>
   *
   * Use the INET_PREFETCH_PRIORITY property to provide a hint to
   * indicate the priority of the prefetch. Note that the
   * implementation may opt to ignore the prefetch request (for
   * example a low priority prefetch when the cache is already full
   * with frequently used information). <p>
   *
   * @param moduleName  [IN] Name of the software module that is 
   *                      outputting the error. See the top of VXIlog.h
   *                      for moduleName allocation rules.
   * @param name        [IN] Name of the data to fetch, see Open( ) for
   *                      a description of supported types
   * @param mode        [IN] Reserved for future use, pass INET_MODE_READ
   * @param flags       [IN] Reserved for future use, pass 0
   * @param properties  [IN] Properties to control the prefetch, as listed
   *                      above. May be NULL. Of particular note are:
   *                    INET_PREFETCH_PRIORITY, hint to indicate whether
   *                      and how soon to prefetch the data
   *                    INET_URL_BASE, base URL for resolving relative URLs
   *                    INET_URL_QUERY_ARGS, map containing key/value
   *                      pairs for HTTP queries, where the name of each
   *                      key is the query argument name, and the value of
   *                      each key is a VXIValue subclass that provides the
   *                      value for that argument
   *
   * @return VXIinet_RESULT_SUCCESS on success
   */
  VXIinetResult (*Prefetch)(struct VXIinetInterface *pThis,
			    const VXIchar           *moduleName,
			    const VXIchar           *name,
			    VXIinetOpenMode          mode,
			    VXIint32                 flags,
			    const VXIMap            *properties);

  /**
   * @name Open
   * @memo Open a stream for reading or writing
   *
   * @doc
   * All implementations must support opening a URL for reading using
   * file:// or http:// access, and opening a platform dependant path
   * for reading. All implementations must support all the flags for
   * each of the above. For all other combinations support is
   * implementation dependant (i.e. write for URLs and platform dependant
   * paths). <p>
   *
   * For URLs, the only accepted unsafe characters are: { } [ ] ^ ~ '
   * They will be escaped if only when processing a HTTP request. <p>
   *
   * @param moduleName   [IN] Name of the software module that is 
   *                       outputting the error. See the top of VXIlog.h
   *                       for moduleName allocation rules.
   * @param name         [IN] Name of the data to fetch, see above for
   *                       supported types
   * @param mode         [IN] Mode to open the data with, an INET_MODE
   *                       value as defined above
   * @param flags        [IN] Flags to control the open:
   *                     INET_FLAG_NONBLOCKING_IO, non-blocking reads and
   *                       writes, although the open and close is still
   *                       blocking
   * @param properties   [IN] Properties to control the open, as listed
   *                       above. May be NULL. Of particular note are:
   *                     INET_URL_BASE, base URL for resolving relative URLs
   *                     INET_URL_QUERY_ARGS, map containing key/value
   *                       pairs for HTTP queries, where the name of each
   *                       key is the query argument name, and the value of
   *                       each key is a VXIValue subclass that provides the
   *                       value for that argument
   * @param streamInfo   [OUT] (Optional, pass NULL if not required) Map
   *                       that will be populated with information about 
   *                       the stream. See the INET_INFO_[...] keys listed
   *                       above, with the implementation possibly setting
   *                       other keys
   * @param stream       [OUT] Handle to the opened stream
   *
   * @return VXIinet_RESULT_SUCCESS on success
   */
  VXIinetResult (*Open)(struct VXIinetInterface  *pThis,
			const VXIchar            *moduleName,
			const VXIchar            *name,
			VXIinetOpenMode           mode,
			VXIint32                  flags,
			const VXIMap             *properties,
			VXIMap                   *streamInfo,
			VXIinetStream           **stream);

  /**
   * @name Close
   * @memo Close a previously opened stream
   * @doc Close a stream that was previously opened. Closing a NULL or
   *      previously closed stream will result in an error.
   * 
   * @param stream       [IN] Stream to close
   *
   * @return VXIinet_RESULT_SUCCESS on success
   */
  VXIinetResult (*Close)(struct VXIinetInterface  *pThis,
			 VXIinetStream           **stream);
  
  /**
   * @name Read
   * @memo Read from a stream
   *
   * @doc
   * This may or not block, as determined by the flags used when opening
   * the stream. When in non-blocking mode, partial buffers may be
   * returned instead of blocking, or an VXIinet_RESULT_WOULD_BLOCK error
   * is returned if no data is available at all.
   *
   * @param buffer   [OUT] Buffer that will receive data from the stream
   * @param buflen   [IN] Length of buffer, in bytes
   * @param nread    [OUT] Number of bytes actual read, may be less then
   *                   buflen if the end of the stream was found, or if
   *                   using non-blocking I/O and there is currently no
   *                   more data available to read
   * @param stream   [IN] Handle to the stream to read from
   *
   * @return VXIinet_RESULT_SUCCESS on success
   */
  VXIinetResult (*Read)(struct VXIinetInterface *pThis,
			VXIbyte                 *buffer,
			VXIulong                 buflen,
			VXIulong                *nread,
			VXIinetStream           *stream);
  
  /**
   * @name Write
   * @memo Write to a stream
   *
   * @doc
   * This may or not block, as determined by the flags used when opening
   * the stream. When in non-blocking mode, partial writes may occur
   * instead of blocking, or an VXIinet_RESULT_WOULD_BLOCK error
   * is returned if no data could be written at all.
   *
   * @param buffer   [OUT] Buffer of data to write to the stream
   * @param buflen   [IN] Number of bytes to write
   * @param nread    [OUT] Number of bytes actual written, may be less then
   *                   buflen if an error is returned, or if using 
   *                   non-blocking I/O and the write operation would block
   * @param stream   [IN] Handle to the stream to write to
   *
   * @return VXIinet_RESULT_SUCCESS on success
   */
  VXIinetResult (*Write)(struct VXIinetInterface *pThis,
			 const VXIbyte           *buffer,
			 VXIulong                 buflen,
			 VXIulong                *nwritten,
			 VXIinetStream           *stream);
  
  /**
   * @name SetCookieJar
   * @memo Set the cookie jar
   *
   * @doc
   * The cookie jar is used to provide cookies and store cookies
   * during future VXIinet Prefetch( ) and Open( ) operations. Expired
   * cookies within the jar will not be used. Each time this is called
   * the prior cookie jar is discarded, the caller is responsible for
   * persistent storage of the cookie jar if desired. See
   * GetCookieJar( ) for details.
   *
   * If SetCookieJar( ) is never called, or if it is called with a
   * NULL jar, the VXIinet implementation will refuse to accept
   * cookies for fetches.
   *
   * @param jar     [IN] Cookie jar, specified as a VXIVector (see the
   *                  description of the cookie jar structure above).
   *                  Pass NULL to refuse all cookies. Pass an empty 
   *                  VXIVector to accept new cookies starting with an 
   *                  empty jar. Pass a non-empty VXIVector as returned
   *                  by GetCookieJar( ) to implement persist cookies
   *                  across multiple user sessions (telephone calls).
   *
   * @return VXIinet_RESULT_SUCCESS on success 
   */
  VXIinetResult (*SetCookieJar)(struct VXIinetInterface  *pThis,
				const VXIVector          *jar);
  
  /**
   * @name GetCookieJar
   * @memo Get the cookie jar
   *
   * @doc
   * The caller of VXIinet is responsible for persistent storage of
   * the cookie jar if desired. This is done by calling SetCookieJar( ) 
   * with the caller's cookie jar at the start of each call (use an
   * empty cookie jar if this is a new caller), then calling this
   * function to retrieve the updated cookie jar at the end of the
   * call for storage. When the cookie jar is returned, any expired
   * cookies will have been deleted.
   *
   * @param jar     [OUT] Cookie jar, returned as a newly allocated
   *                  VXIVector (see the description of the cookie jar 
   *                  structure above, it may be empty). The client 
   *                  is responsible for destroying this via 
   *                  VXIVectorDestroy( ) when appropriate.
   * @param changed [OUT] Flag to indicate whether the cookie jar
   *                  has been modified since SetCookieJar( ), allows
   *                  suppressing the write of the cookie jar to
   *                  persistant storage when that operation is 
   *                  expensive. Pass NULL if this information is not
   *                  desired.
   *
   * @return VXIinet_RESULT_SUCCESS on success 
   */
  VXIinetResult (*GetCookieJar)(struct VXIinetInterface  *pThis,
				VXIVector               **jar,
				VXIbool                  *changed);

} VXIinetInterface;

/*@}*/

#ifdef __cplusplus
}
#endif

#include "VXIheaderSuffix.h"

#endif  /* include guard */
