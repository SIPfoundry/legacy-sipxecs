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
 * SBinetURL Implementation
 *
 * 
 *
 ***********************************************************************/
#ifndef _SB_USE_STD_NAMESPACE
#define _SB_USE_STD_NAMESPACE
#endif

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <stdio.h>

#include <WWWLib.h>
#include <WWWHTTP.h>
#include <WWWInit.h>
#ifdef EINVAL
// Conflicts with OS definition
#undef EINVAL
#endif

#ifdef WIN32
#undef HTTP_VERSION
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wininet.h>
#include <urlmon.h>
#endif /* WIN32 */

#include "VXIvalue.h"
#include "VXIinet.h"
#include "VXItrd.h"

#define VXI_MIME_VXML L"text/xml"

#include "SBinetURL.h"
#include "SBinetChannel.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

#define CRLF "\r\n"  // Used for multipart messages


static void
AppendArrayIndexToName(VXIchar *fieldName,
                       VXIunsigned index)
{
  char tempBuf[8];
  VXIchar arrayIndex[8];

  sprintf(tempBuf, ".%d", index);
  mbstowcs(arrayIndex, tempBuf, sizeof(tempBuf));
  wcscat(fieldName, arrayIndex);
}

/*
 * Warning: Some bugus max size restrictions in QueryArg processing. search for 2048
 */

SBinetURL::SBinetURL(){
}

SBinetURL::~SBinetURL(){
}

SBinetURL::URLScheme
SBinetURL::GetScheme() const
{
  return(m_scheme);
}

const char* 
SBinetURL::GetAbsoluteNarrow()
{
  WideToNarrowString(m_strAbsoluteUrl.c_str(), m_strAbsoluteUrlNarrow);
  return(m_strAbsoluteUrlNarrow.c_str());
}

const char* 
SBinetURL::GetPathNarrow()
{
  WideToNarrowString(m_strPath.c_str(), m_strPathNarrow);
  return(m_strPathNarrow.c_str());
}

VXIinetResult 
SBinetURL::Parse(const VXIchar* pszName,
		 VXIinetOpenMode  eMode,
		 VXIint32         nFlags,
		 const VXIMap*    pProperties)
{
  VXIinetResult eResult = VXIinet_RESULT_SUCCESS;
  VXIString *urlBaseStr = 
    (VXIString*)VXIMapGetProperty( pProperties, INET_URL_BASE );
  const VXIchar* pszUrlBase = L"";
  if ( urlBaseStr != NULL ) 
    pszUrlBase = VXIStringCStr( urlBaseStr );
  m_strBaseUrl = pszUrlBase;
  m_strUrl = pszName;
#ifdef WIN32
    eResult = WinInetResolveUrl (pszUrlBase, 
				 pszName, 
				 &m_strAbsoluteUrl,
				 &m_scheme);
#else
    eResult = WWWResolveUrl (pszUrlBase, 
			     pszName, 
			     &m_strAbsoluteUrl,
			     &m_scheme);
#endif    
    return(eResult);
}



#ifdef WIN32

VXIinetResult 
SBinetURL::WinInetResolveUrl(const VXIchar* pszBaseUrl, 
			     const VXIchar* pszUrl, 
			     SBinetString*  strAbsoluteUrl,
			     URLScheme*     pScheme)
{
  VXIinetResult eResult( VXIinet_RESULT_SUCCESS );

  if( !pszUrl || !pszUrl[0] || !strAbsoluteUrl || !pScheme ) {
    //Error(200, L"%s%s", L"Operation", L"ResolveUrl");
    return VXIinet_RESULT_INVALID_ARGUMENT;
  }

  *pScheme = HTTP_SCHEME;

  // Combine the base and (possibly relative) URL
  wchar_t *tmpUrl = NULL;
  const wchar_t *absoluteUrl = pszUrl;

  if( pszBaseUrl && pszBaseUrl[0] ) 
  {
    VXIulong len = ::wcslen (pszBaseUrl) + ::wcslen (pszUrl) + 256;
    tmpUrl = new VXIchar [len];
    if( !tmpUrl ) {
      //Error(103, NULL);
	  return VXIinet_RESULT_OUT_OF_MEMORY;
    }

    if( InternetCombineUrl(pszBaseUrl, pszUrl, tmpUrl, &len, 
                           ICU_NO_ENCODE) == TRUE ) {
	  absoluteUrl = tmpUrl;
    }
  }
    
  // Now parse the absolute URL to decide if it is file or network access
  wchar_t scheme[MAX_PATH], host[MAX_PATH], urlPath[MAX_PATH];
  URL_COMPONENTS components;

  memset (&components, 0, sizeof (URL_COMPONENTS));
  components.dwStructSize = sizeof (URL_COMPONENTS);
  components.lpszScheme = scheme;
  components.dwSchemeLength = MAX_PATH;
  components.lpszHostName = host;
  components.dwHostNameLength = MAX_PATH;
  components.lpszUrlPath = urlPath;
  components.dwUrlPathLength = MAX_PATH;   
    
  if( InternetCrackUrl( absoluteUrl, ::wcslen (absoluteUrl), 
			            ICU_DECODE | ICU_ESCAPE, &components) == TRUE ) 
  {
    if( ::wcscmp(components.lpszScheme, L"file") == 0 )
    {
	  // File access, return the local file path
	  *pScheme = FILE_SCHEME;
	  m_strPath = urlPath;
	  *strAbsoluteUrl = absoluteUrl;
    }
    else if ((components.dwSchemeLength == 1) && (absoluteUrl[1] == L':'))
    {
	  // File access without 'file:' prefix, return the URL as-is
	  *pScheme = FILE_SCHEME;
	  m_strPath = absoluteUrl;
	  *strAbsoluteUrl = absoluteUrl;
    } 
    else {
	  // Network access, return the absolute URL
	  *pScheme = HTTP_SCHEME;
	  *strAbsoluteUrl = absoluteUrl;
    }
  } 
  else {
    // Couldn't be parsed, must be a relative path to a file based
    // on the current working directory
    wchar_t *ignored;
    urlPath[0] = L'\0';
    *pScheme = FILE_SCHEME;
      
    if(( GetFullPathName(absoluteUrl, MAX_PATH, urlPath, &ignored) > 0 ) &&
	   ( urlPath[0] != L'\0' )) 
    {
	  m_strPath = urlPath;
	  *strAbsoluteUrl = absoluteUrl;
    } 
    else {
      //Error(225, L"%s%s", L"File", absoluteUrl);
	  eResult = VXIinet_RESULT_NON_FATAL_ERROR;
    }
  }

  if( tmpUrl ) delete [] tmpUrl;
  tmpUrl = NULL;

  return eResult;
}

#else /* not WIN32 */

VXIinetResult 
SBinetURL::WinInetResolveUrl(const VXIchar* pszBaseUrl, 
			     const VXIchar* pszUrl, 
			     SBinetString*  strAbsoluteUrl,
			     URLScheme*     pScheme)
{
  return(VXIinet_RESULT_UNSUPPORTED);
}

#endif /* WIN32 */


VXIinetResult 
SBinetURL::WWWResolveUrl(const VXIchar* pszBaseUrl, 
			 const VXIchar* pszUrl, 
			 SBinetString*  strAbsoluteUrl,
			 URLScheme*     pScheme)
{
  // TBD: Must apply SPR 7530 fix here too, for now only in WinInetResolveUrl,
  // support for relative URL starting with / as specified by the RFC that
  // defines file:// access
  VXIinetResult eResult( VXIinet_RESULT_SUCCESS );

  if (( ! pszUrl ) || ( ! pszUrl[0] ) || ( ! strAbsoluteUrl ) ||
      ( ! pScheme ))
  {
    //Error(200, L"%s%s", L"Operation", L"ResolveUrl");
    return VXIinet_RESULT_INVALID_ARGUMENT;
  }

  *pScheme = HTTP_SCHEME;
  SBinetNString strNAbsoluteUrl;

  // First look for slashes at the start or a Win32 drive letter,
  // clear absolute file paths
  if (( pszUrl[0] == L'/' ) || ( pszUrl[0] == L'\\' ) || 
      ( pszUrl[1] == L':' )) {
    *pScheme = FILE_SCHEME;
    if ( WideToNarrowString (pszUrl, strNAbsoluteUrl) !=
	 VXIinet_RESULT_SUCCESS )
      eResult = VXIinet_RESULT_NON_FATAL_ERROR;
  } else {
    // Libwww can only handle narrow strings
    SBinetNString strBaseUrl, strUrl;
    if (( WideToNarrowString (pszUrl, strUrl) != 
	  VXIinet_RESULT_SUCCESS ) ||
	(( pszBaseUrl ) && ( pszBaseUrl[0] ) &&
	 ( WideToNarrowString (pszBaseUrl, strBaseUrl) !=
	   VXIinet_RESULT_SUCCESS )))
      return VXIinet_RESULT_NON_FATAL_ERROR;
    
    // Use libwww to retrieve the scheme type, used to determine whether
    // we're working with file scheme or not
    char *scheme = HTParse (strUrl.c_str(), strBaseUrl.c_str(), PARSE_ACCESS);
    if (( ! scheme ) || ( ! scheme[0] ) || ( strcmp (scheme, "file") == 0 )) {
      // File access, return the local file path
      char *tmpPath = HTParse (strUrl.c_str(), strBaseUrl.c_str(), 
                               PARSE_PATH | PARSE_PUNCTUATION);
      if (( tmpPath ) && ( tmpPath[0] ) && ( strcmp (tmpPath, "/") != 0 )) {
	*pScheme = FILE_SCHEME;
	strNAbsoluteUrl = tmpPath;
      } else {
	// Cannot parse the file:// URL
	eResult = VXIinet_RESULT_NON_FATAL_ERROR;
      }
      if ( tmpPath ) HT_FREE (tmpPath);
    } else {
      // Network access, returned the absolute URL
      char *tmpUrl = HTParse (strUrl.c_str(), strBaseUrl.c_str(), PARSE_ALL);
      if (( tmpUrl ) && ( tmpUrl[0] )) {
	*pScheme = HTTP_SCHEME;
	strNAbsoluteUrl = tmpUrl;
      } else {
	// Cannot parse the network URL
	eResult = VXIinet_RESULT_NON_FATAL_ERROR;
      }
      if ( tmpUrl ) HT_FREE (tmpUrl);
    }
    
    if ( scheme ) HT_FREE (scheme);
  }

  // Make sure it is a full path for files
  if (( eResult == VXIinet_RESULT_SUCCESS ) && 
      ( *pScheme == FILE_SCHEME ) && ( strNAbsoluteUrl[0] != '/' ) && 
      ( strNAbsoluteUrl[0] != '\\' ) && ( strNAbsoluteUrl[1] != ':' )) {
    char cwd[MAX_PATH];
    cwd[0] = '\0';
#ifdef WIN32
    _getcwd (cwd, MAX_PATH - 1);
#else
    getcwd (cwd, MAX_PATH - 1);
#endif
    if ( cwd[0] ) {
      SBinetNString strTmp;
      strTmp = cwd;
      strTmp += L'/';
      strTmp += strNAbsoluteUrl;
      strNAbsoluteUrl = strTmp;
    }
  }

  // Simplify it
  if ( eResult == VXIinet_RESULT_SUCCESS ) {
    char *tmpUrl = new char [strNAbsoluteUrl.length( ) + 1];
    if ( tmpUrl ) {
      strcpy (tmpUrl, strNAbsoluteUrl.c_str( ));
      char *simplified = HTSimplify (&tmpUrl);
      if (( simplified ) && ( simplified[0] ))
	strNAbsoluteUrl = simplified;
      if (( simplified ) && ( simplified != tmpUrl ))
	HT_FREE (simplified);
      delete [] tmpUrl;
      tmpUrl = NULL;
    }
  }

  // Return it
  if ( eResult == VXIinet_RESULT_SUCCESS )
    NarrowToWideString (strNAbsoluteUrl.c_str( ), *strAbsoluteUrl);

  m_strPath = *strAbsoluteUrl;
  return eResult;
}


const VXIchar*
SBinetURL::ValueToString(const VXIValue* value, VXIchar* outBuf,
			 VXIunsigned outBufLen)
{
  const VXIchar *rVal = outBuf;
  outBuf[0] = 0;

  // Convert numeric types using a narrow character buffer in order to
  // avoid the need for swprintf( ) which doesn't exist in the GNU
  // GCC C library for GCC 2.x and earlier.
  char tempBuf[32];
  tempBuf[0] = 0;
  switch( VXIValueGetType(value)) {
  case VALUE_INTEGER: 
    {
      VXIint32 valInt = VXIIntegerValue( (VXIInteger *)value );
      sprintf (tempBuf, "%i", valInt);
    } break;
  case VALUE_FLOAT: 
    {
      VXIflt32 valFloat = VXIFloatValue( (VXIFloat *)value );
      sprintf (tempBuf, "%f", valFloat);
    } break;
  case VALUE_STRING: 
    {
      rVal = VXIStringCStr( (VXIString *)value );
    } break;
  case VALUE_PTR: 
    {
      void *valPtr = VXIPtrValue( (VXIPtr *)value );
      sprintf (tempBuf, "%p", valPtr);
    } break;
  case VALUE_MAP: 
  case VALUE_VECTOR: 
  case VALUE_CONTENT:
  default:
    {
      // These types are supposed to be handled before entering this func
    } break;
  } // end switch()

  // Convert back to wide characters if necessary
  if (tempBuf[0]) {
    int i = 0;
    do {
      outBuf[i] = tempBuf[i];
      i++;
    } while (tempBuf[i]);
    outBuf[i] = L'\0';
  }

  return(rVal);
}



int
SBinetURL::NeedMultipart(const VXIMap* queryArgs)
{
  // Create a list to hold the form arguments, then parse the
  // data and add the key-value pairs to the association list
  const VXIchar  *key = NULL;
  const VXIValue *value = NULL;
  if(!queryArgs) return(0);
  VXIMapIterator *mapIterator = VXIMapGetFirstProperty( queryArgs, &key, &value );
  do{
      if (( key != NULL ) && ( value != NULL ))	{
	if(VXIValueGetType(value) == VALUE_CONTENT) {
          VXIMapIteratorDestroy(&mapIterator);
	  return(1);
   	}
      }
  } while ( VXIMapGetNextProperty( mapIterator, &key, &value ) == VXIvalue_RESULT_SUCCESS );
  VXIMapIteratorDestroy(&mapIterator);
  return(0);
}


void SBinetURL::AppendKeyValuePairToURL( const VXIchar *key,
                                         const VXIchar *value,
                                         VXIbool *isFirstArg )
{    
  if(key && value){
    if( *isFirstArg ) {
      m_strAbsoluteUrl += L"?";
      *isFirstArg = false;
    }
    else
      m_strAbsoluteUrl += L"&";

    VXIchar *escapedKey = EscapeStringW(key);
    VXIchar *escapedValue = EscapeStringW(value);

	m_strAbsoluteUrl += escapedKey;
	m_strAbsoluteUrl += L"=";
	m_strAbsoluteUrl += escapedValue;

    if (escapedKey)
    {
	free(escapedKey);
	escapedKey = NULL;
    }
    if (escapedValue)
    {
    	free(escapedValue);
	escapedValue = NULL;
    }
  }
}


void SBinetURL::AppendQueryArgsVector( const VXIVector *vector,
                                       VXIchar *fieldName, 
                                       VXIbool *isFirstArg )
{
  const VXIValue *value = NULL;

  VXIunsigned vectorLen = VXIVectorLength(vector);
  VXIunsigned i;

  for(i = 0 ; i < vectorLen ; i++) {
    value = VXIVectorGetElement(vector, i);
    if ( value != NULL ){
      VXIunsigned prevLen = ::wcslen(fieldName);
      AppendArrayIndexToName(fieldName, i);

	  if(VXIValueGetType(value) == VALUE_MAP) {  // nested object
        // Append the object name to the field name prefix
        AppendQueryArgsMap((const VXIMap *)value, fieldName, isFirstArg); // Parse map
      }
	  else if(VXIValueGetType(value) == VALUE_VECTOR) {  // nested vector
        // Append the vector name to the field name prefix
        AppendQueryArgsVector((const VXIVector *)value, fieldName, isFirstArg); // Recursive
      }
      else {
        VXIchar tempBuf[2048];

        const VXIchar* valStr = ValueToString(value, tempBuf, 2048);
        AppendKeyValuePairToURL(fieldName, valStr, isFirstArg);
      }

      fieldName[prevLen] = L'\0'; // Remove the array index
    }
  }
}


void SBinetURL::AppendQueryArgsMap( const VXIMap *map,
                                    VXIchar *fieldName, 
                                    VXIbool *isFirstArg )
{
  const VXIchar  *key = NULL;
  const VXIValue *value = NULL;

  VXIMapIterator *mapIterator = VXIMapGetFirstProperty( map, &key, &value );
  do{
    if (( key != NULL ) && ( value != NULL )){
	  if(VXIValueGetType(value) == VALUE_MAP) {  // nested object
        // Append the object name to the field name prefix
        if( fieldName[0] != L'\0' ) 
          wcscat(fieldName, L".");
        wcscat(fieldName, key);
        AppendQueryArgsMap((const VXIMap *)value, fieldName, isFirstArg); // Recursive
      }
	  else if(VXIValueGetType(value) == VALUE_VECTOR) {  // nested vector
        // Append the object name to the field name prefix
        if( fieldName[0] != L'\0' ) 
          wcscat(fieldName, L".");
        wcscat(fieldName, key);
        AppendQueryArgsVector((const VXIVector *)value, fieldName, isFirstArg); // Parse vector
      }
      else {
        VXIchar tempBuf[2048];

        // Append the real field name to the field name prefix
        VXIint fieldNameLen = ::wcslen(fieldName);
        if( fieldName[0] != L'\0' ) 
          wcscat(fieldName, L".");
        wcscat(fieldName, key);

        const VXIchar* valStr = ValueToString(value, tempBuf, 2048);
        AppendKeyValuePairToURL(fieldName, valStr, isFirstArg);

        // Remove the just appended file name, leave only the prefix
        fieldName[fieldNameLen] = L'\0';
      }
    }
  } while ( VXIMapGetNextProperty( mapIterator, &key, &value ) == VXIvalue_RESULT_SUCCESS );
  
  // Cut the last object name from the field name prefix
  VXIchar *fieldNamePtr = wcsrchr(fieldName, L'.');
  if( fieldNamePtr != NULL )
    *fieldNamePtr = L'\0';
  else
    fieldName[0] = L'\0';

  VXIMapIteratorDestroy(&mapIterator);
}


void
SBinetURL::AppendQueryArgs(const VXIMap* queryArgs)
{
  // Create a list to hold the form arguments, then parse the
  // data and add the key-value pairs to the association list
  VXIchar tempBuf[2048];
  const VXIchar  *key = NULL;
  const VXIValue *value = NULL;
  VXIbool isFirstArg = true;

  if(!queryArgs) return;

  // Use STL as we are already. I hope find() is portable.
  if((int)m_strAbsoluteUrl.find(L'?') == 0 ) isFirstArg = false;

  VXIMapIterator *mapIterator = VXIMapGetFirstProperty( queryArgs, &key, &value );
  do{
    if (( key != NULL ) && ( value != NULL )) {
	  if(VXIValueGetType(value) == VALUE_MAP) {  // nested object
        VXIchar fieldName[1024];
        wcscpy(fieldName, key);
        AppendQueryArgsMap((const VXIMap *)value, fieldName, &isFirstArg); // Parse map
      }
      else if(VXIValueGetType(value) == VALUE_VECTOR) { // nested vector
        VXIchar fieldName[1024];
        wcscpy(fieldName, key);
        AppendQueryArgsVector((const VXIVector *)value, fieldName, &isFirstArg); // Parse vector
      }
      else {
        const VXIchar* valStr = ValueToString(value, tempBuf, 2048);
        AppendKeyValuePairToURL(key, valStr, &isFirstArg);
      }
    }
  } while ( VXIMapGetNextProperty( mapIterator, &key, &value ) == VXIvalue_RESULT_SUCCESS );

  VXIMapIteratorDestroy(&mapIterator);
}


char *SBinetURL::EscapeString( const VXIchar *value )
{
  char tempVal[2048];

  int len = ::wcslen(value);
  if(len > 2047) len = 2047;  // should realloc but...
  if (::wcstombs(tempVal, value, len) != (unsigned int) len)
  {
     fprintf(stderr, "SBinetURL::EscapeString translation of wide string to ASCII failed\n");
  }
  tempVal[len] = '\0';
  
  return HTEscape(tempVal, URL_XALPHAS);
} 

VXIchar *SBinetURL::EscapeStringW( const VXIchar *value )
{
  char *escaped = EscapeString(value);
  if(escaped == NULL) return NULL;

  int len = ::strlen(escaped) + 1;
  VXIchar *retStr = (VXIchar *)malloc(len * sizeof(VXIchar));
  if(retStr == NULL) return NULL;

  ::mbstowcs(retStr, escaped, len);
  HT_FREE(escaped);
  return retStr;
} 


void SBinetURL::AddObjectToHTAssocList( HTAssocList *arglist,
                                        const VXIchar *key,
                                        const VXIchar *value )
{    
  if((key == NULL) || (value == NULL)) return;

  char *escapedKey = EscapeString(key);
  char *escapedValue = EscapeString(value);

  HTAssocList_addObject( arglist, escapedKey, escapedValue );

  if (escapedKey)
  {
  	free(escapedKey);
	escapedKey = NULL;
  }
  if (escapedValue)
  {
	free(escapedValue);
	escapedValue = NULL;
  }
}


void SBinetURL::QueryArgsVectorToHtList( HTAssocList *arglist, 
                                         const VXIVector *vector, 
                                         VXIchar *fieldName )
{
  const VXIValue *value = NULL;

  VXIunsigned vectorLen = VXIVectorLength(vector);
  VXIunsigned i;

  for(i = 0 ; i < vectorLen ; i++) {
    value = VXIVectorGetElement(vector, i);
    if ( value != NULL ){
      VXIunsigned prevLen = ::wcslen(fieldName);
      AppendArrayIndexToName(fieldName, i);

	  if(VXIValueGetType(value) == VALUE_MAP) {  // nested object
        // Append the object name to the field name prefix
        QueryArgsMapToHtList(arglist, (const VXIMap *)value, fieldName); // Parse map
      }
	  else if(VXIValueGetType(value) == VALUE_VECTOR) {  // nested vector
        // Append the vector name to the field name prefix
        QueryArgsVectorToHtList(arglist, (const VXIVector *)value, fieldName); // Recursive
      }
      else {
        VXIchar tempBuf[2048];

        const VXIchar* valStr = ValueToString(value, tempBuf, 2048);
        AddObjectToHTAssocList(arglist, fieldName, valStr);
      }

      fieldName[prevLen] = L'\0'; // Remove the array index
    }
  }
}


void SBinetURL::QueryArgsMapToHtList( HTAssocList *arglist, 
                                      const VXIMap *map, 
                                      VXIchar *fieldName )
{
  const VXIchar  *key = NULL;
  const VXIValue *value = NULL;

  VXIMapIterator *mapIterator = VXIMapGetFirstProperty( map, &key, &value );
  do{
    if (( key != NULL ) && ( value != NULL )){
	  if(VXIValueGetType(value) == VALUE_MAP) {  // nested object
        // Append the object name to the field name prefix
        if( fieldName[0] != L'\0' ) 
          wcscat(fieldName, L".");
        wcscat(fieldName, key);
        QueryArgsMapToHtList(arglist, (const VXIMap *)value, fieldName); // Recursive
      }
	  else if(VXIValueGetType(value) == VALUE_VECTOR) {  // nested vector
        // Append the object name to the field name prefix
        if( fieldName[0] != L'\0' ) 
          wcscat(fieldName, L".");
        wcscat(fieldName, key);
        QueryArgsVectorToHtList(arglist, (const VXIVector *)value, fieldName); // Parse vector
      }
      else {
        VXIchar tempBuf[2048];

        // Append the real field name to the field name prefix
        VXIint fieldNameLen = ::wcslen(fieldName);
        if( fieldName[0] != L'\0' ) 
          wcscat(fieldName, L".");
        wcscat(fieldName, key);

        const VXIchar* valStr = ValueToString(value, tempBuf, 2048);
        AddObjectToHTAssocList(arglist, fieldName, valStr);

        // Remove the just appended file name, leave only the prefix
        fieldName[fieldNameLen] = L'\0';
      }
    }
  } while ( VXIMapGetNextProperty( mapIterator, &key, &value ) == VXIvalue_RESULT_SUCCESS );
  
  // Cut the last object name from the field name prefix
  VXIchar *fieldNamePtr = wcsrchr(fieldName, L'.');
  if( fieldNamePtr != NULL )
    *fieldNamePtr = L'\0';
  else
    fieldName[0] = L'\0';

  VXIMapIteratorDestroy(&mapIterator);
}


HTAssocList*
SBinetURL::QueryArgsToHtList(const VXIMap* queryArgs){
  // Create a list to hold the form arguments, then parse the
  // data and add the key-value pairs to the association list
  const VXIchar  *key = NULL;
  const VXIValue *value = NULL;
  VXIchar tempBuf[2048];

  if(!queryArgs) return(NULL);
  HTAssocList* arglist = NULL;

  VXIMapIterator *mapIterator = VXIMapGetFirstProperty( queryArgs, &key, &value );
  do{
    if (( key != NULL ) && ( value != NULL )){
      if( arglist == NULL ) 
        arglist = HTAssocList_new();

	  if(VXIValueGetType(value) == VALUE_MAP) {  // nested object
        VXIchar fieldName[1024];
        wcscpy(fieldName, key);
        QueryArgsMapToHtList(arglist, (const VXIMap *)value, fieldName); // Parse map
      }
      else if(VXIValueGetType(value) == VALUE_VECTOR) { // nested vector
        VXIchar fieldName[1024];
        wcscpy(fieldName, key);
        QueryArgsVectorToHtList(arglist, (const VXIVector *)value, fieldName); // Parse vector
      }
      else {
        const VXIchar* valStr = ValueToString(value, tempBuf, 2048);
        AddObjectToHTAssocList( arglist, key, valStr);
	  }
    }
  } while ( VXIMapGetNextProperty( mapIterator, &key, &value ) == VXIvalue_RESULT_SUCCESS );
  
  VXIMapIteratorDestroy(&mapIterator);
  return(arglist);
}

/*
 * Utilities
 */

void SBinetURL::AppendKeyToMultipart( const VXIchar *key )
{    
  char tempCBuf[2048];
  char tempKey[2048];
  int len;
	  
  len = ::wcslen(key);
  if(len > 2047) len = 2047;  // should realloc but...
  ::wcstombs( tempKey, key, len );
  tempKey[len] = 0;

  // Print the boundary start
  sprintf(tempCBuf,"--%s%s", SB_BOUNDARY, CRLF);
  appendStr(tempCBuf);
  sprintf(tempCBuf, "Content-Disposition: form-data; name=\"%s\"%s", 
          tempKey, CRLF);
  appendStr(tempCBuf);
}


void SBinetURL::AppendStringValueToMultipart( const VXIchar *value )
{    
  char tempCBuf[2048];
  char tempVal[2048];
  int len;

  if(value){
	len = ::wcslen(value);
	if(len > 2047) len = 2047; // should realloc but...
	::wcstombs( tempVal, value, len );
	tempVal[len] = 0;
	      
	// I am guessing what headers should be
	sprintf(tempCBuf,"Content-Type: text/plain%s", CRLF);
	appendStr(tempCBuf);
	sprintf(tempCBuf,"Content-Length: %zu%s", strlen(tempVal), CRLF);
	appendStr(tempCBuf);
	sprintf(tempCBuf,"%s%s%s", CRLF, tempVal, CRLF); // One blank line before
	appendStr(tempCBuf);
  }
}


void SBinetURL::QueryArgsVectorToMultipart( const VXIVector *vector,
                                            VXIchar *fieldName )
{
  const VXIValue *value = NULL;

  VXIunsigned vectorLen = VXIVectorLength(vector);
  VXIunsigned i;

  for(i = 0 ; i < vectorLen ; i++) {
    value = VXIVectorGetElement(vector, i);
    if ( value != NULL ){
      VXIunsigned prevLen = ::wcslen(fieldName);
      AppendArrayIndexToName(fieldName, i);

	  if(VXIValueGetType(value) == VALUE_MAP) {  // nested object
        // Append the object name to the field name prefix
        QueryArgsMapToMultipart((const VXIMap *)value, fieldName); // Parse map
      }
	  else if(VXIValueGetType(value) == VALUE_VECTOR) {  // nested vector
        // Append the vector name to the field name prefix
        QueryArgsVectorToMultipart((const VXIVector *)value, fieldName); // Recursive
      }
      else {
        VXIchar tempBuf[2048];

        const VXIchar* valStr = ValueToString(value, tempBuf, 2048);
        AppendKeyToMultipart(fieldName);
        AppendStringValueToMultipart(valStr);
      }

      fieldName[prevLen] = L'\0'; // Remove the array index
    }
  }
}


void SBinetURL::QueryArgsMapToMultipart( const VXIMap *map, 
                                         VXIchar *fieldName )
{
  const VXIchar  *key = NULL;
  const VXIValue *value = NULL;

  VXIMapIterator *mapIterator = VXIMapGetFirstProperty( map, &key, &value );
  do{
    if (( key != NULL ) && ( value != NULL )){
	  if(VXIValueGetType(value) == VALUE_MAP) {  // nested object
        // Append the object name to the field name prefix
        if( fieldName[0] != L'\0' ) 
          wcscat(fieldName, L".");
        wcscat(fieldName, key);
        QueryArgsMapToMultipart((const VXIMap *)value, fieldName); // Recursive
      }
	  else if(VXIValueGetType(value) == VALUE_VECTOR) {  // nested vector
        // Append the object name to the field name prefix
        if( fieldName[0] != L'\0' ) 
          wcscat(fieldName, L".");
        wcscat(fieldName, key);
        QueryArgsVectorToMultipart((const VXIVector *)value, fieldName); // Parse vector
      }
      else {
        VXIchar tempBuf[2048];

        // Append the real field name to the field name prefix
        VXIint fieldNameLen = ::wcslen(fieldName);
        if( fieldName[0] != L'\0' ) 
          wcscat(fieldName, L".");
        wcscat(fieldName, key);

        const VXIchar* valStr = ValueToString(value, tempBuf, 2048);
        AppendKeyToMultipart(fieldName);
        AppendStringValueToMultipart(valStr);

        // Remove the just appended file name, leave only the prefix
        fieldName[fieldNameLen] = L'\0';
      }
    }
  } while ( VXIMapGetNextProperty( mapIterator, &key, &value ) == VXIvalue_RESULT_SUCCESS );
  
  // Cut the last object name from the field name prefix
  VXIchar *fieldNamePtr = wcsrchr(fieldName, L'.');
  if( fieldNamePtr != NULL )
    *fieldNamePtr = L'\0';
  else
    fieldName[0] = L'\0';

  VXIMapIteratorDestroy(&mapIterator);
}


const char*
SBinetURL::QueryArgsToMultipart(const VXIMap* queryArgs,VXIulong* plen){
  // Create a list to hold the form arguments, then parse the
  // data and add the key-value pairs to the association list
  const VXIchar  *key = NULL;
  const VXIValue *value = NULL;
  char tempKey[2048];
  const VXIchar* valStr;
  int len;

  if(!queryArgs) return(NULL);
  VXIMapIterator *mapIterator = VXIMapGetFirstProperty( queryArgs, &key, &value );

  // First go through and compute lengths
  int totalLen = 0;
  do{
      if (( key != NULL ) && ( value != NULL ))	{
	  len = ::wcslen(key);
	  if(len > 2047) len = 2047;           // should realloc but...
	  ::wcstombs( tempKey, key, len );
	  tempKey[len] = 0;

      if(VXIValueGetType(value) == VALUE_CONTENT){  // audio
	    const VXIchar* type;
	    const VXIbyte* data;
	    VXIulong size;
	    VXIContentValue((const VXIContent  *)value,&type,&data,&size);
	    totalLen += size;
	  }
	  else{ // normal stuff
        // TODO: We should compute the size for maps and vectors !!!
	    totalLen += 2048;       // bogus max value length.
	  }
      }
  } while ( VXIMapGetNextProperty( mapIterator, &key, &value ) == VXIvalue_RESULT_SUCCESS );
  VXIMapIteratorDestroy(&mapIterator);

  initDoc(totalLen + 2048); // extra for headers
  mapIterator = VXIMapGetFirstProperty( queryArgs, &key, &value );

  char tempCBuf[2048];
  // 
  do{
    if (( key != NULL ) && ( value != NULL )) {

	  if(VXIValueGetType(value) == VALUE_CONTENT){  // audio
	    const VXIchar* type;
	    const VXIbyte* data;
	    VXIulong size;

        AppendKeyToMultipart(key);

        VXIContentValue((const VXIContent  *)value, &type, &data, &size);
	    // I am guessing what headers should be
	    sprintf(tempCBuf,"Content-Type: %S%s", type, CRLF);
	    appendStr(tempCBuf);
	    sprintf(tempCBuf,"Content-Length: %ld%s", size, CRLF);
	    appendStr(tempCBuf);
	    sprintf(tempCBuf,"%s", CRLF); // One blank line
	    appendStr(tempCBuf);
	    appendData((char*)data, size);
	    sprintf(tempCBuf,"%s", CRLF); // One blank line
	    appendStr(tempCBuf);
	  }
      else if(VXIValueGetType(value) == VALUE_MAP){ // nested object
        VXIchar fieldName[1024];
        wcscpy(fieldName, key);
        QueryArgsMapToMultipart((const VXIMap *)value, fieldName); // Parse map
      }
      else if(VXIValueGetType(value) == VALUE_VECTOR){ // nested vector
        VXIchar fieldName[1024];
        wcscpy(fieldName, key);
        QueryArgsVectorToMultipart((const VXIVector *)value, fieldName); // Parse vector
      }
	  else{ // normal stuff
	    VXIchar tempBuf[2048];
	    valStr = ValueToString(value,tempBuf, 2048);   // either tempBuf or VXIString contents

        AppendKeyToMultipart(key);
        AppendStringValueToMultipart(valStr);
	  }
    }
  } while ( VXIMapGetNextProperty( mapIterator, &key, &value ) == VXIvalue_RESULT_SUCCESS );

  // Print the boundary terminator
  sprintf(tempCBuf,"--%s--%s", SB_BOUNDARY, CRLF);
  appendStr(tempCBuf);

  VXIMapIteratorDestroy(&mapIterator);
  *plen = m_doc.length();
  return(m_doc.c_str());
}


VXIinetResult 
SBinetURL::WideToNarrowString(const VXIchar* pszWide, 
			      SBinetNString& strNarrow)
{
    VXIinetResult eResult( VXIinet_RESULT_SUCCESS ); strNarrow = "";

    if (pszWide == NULL) return VXIinet_RESULT_INVALID_ARGUMENT;
 
    size_t nCharCount( ::wcslen( pszWide ) );
    char *tmp = 0;
    if (nCharCount > 0)
    {
       tmp = new char[sizeof(char) *(nCharCount + 1)];

       if (!tmp) return VXIinet_RESULT_OUT_OF_MEMORY;

       for (size_t i=0; i < nCharCount; i++) 
       {
          if ((signed)pszWide[i] < (signed)CHAR_MIN || (signed)pszWide[i] > (signed)CHAR_MAX) 
          {
             eResult = VXIinet_RESULT_INVALID_ARGUMENT; 
          }
          tmp[i] = (char)pszWide[i];
       }
 
       tmp[nCharCount] = 0;
       strNarrow = SBinetNString((const char*)tmp);
       delete[] tmp;
       tmp = NULL;
    }

    return eResult;
}


VXIinetResult 
SBinetURL::NarrowToWideString(const char*    pszNarrow, 
			      SBinetString&     strWide)
{
    VXIinetResult eResult( VXIinet_RESULT_SUCCESS ); strWide = L"";

    if (pszNarrow == NULL) return VXIinet_RESULT_INVALID_ARGUMENT;
 
    size_t nCharCount( ::strlen( pszNarrow ) );
    for (size_t i=0; i < nCharCount; i++) 
    {
        strWide += (wchar_t)pszNarrow[i];
    }
 
    return eResult;
}

//
// Infer a MIME content type from a URL (by the extension)
//
VXIinetResult 
SBinetURL::ContentTypeFromUrl(SBinetString* strContentType) const
{
  if(!strContentType) 
    return VXIinet_RESULT_INVALID_ARGUMENT;

  //
  // Determine the extension. This has to work for local files and also HTTP
  //
  const VXIchar *url = m_strAbsoluteUrl.c_str();
  const VXIchar *urlEnd = url + ::wcslen(url);

  if((url == NULL) ||  (url == urlEnd))
    return VXIinet_RESULT_FAILURE;

  // Don't look farther than the first '?' (query args) or '#'
  const VXIchar *urlQueryArgs = wcschr(url, L'?');
  const VXIchar *urlHash = wcschr(url, L'#');

  if(urlQueryArgs) {
    if(urlHash && (urlHash < urlQueryArgs))
      urlEnd = urlHash;
    else
      urlEnd = urlQueryArgs;
  }
  else if(urlHash)
    urlEnd = urlHash;

  // Now look for the last '.'
  VXIchar ext[1024] = L"";
  const VXIchar *urlDot = urlEnd - 1;

  while(urlDot >= url) {
    if(*urlDot == L'.')
      break;
    urlDot--;
  }

  if(urlDot >= url) {
    ::wcsncpy(ext, urlDot, urlEnd - urlDot);
    ext[urlEnd - urlDot] = L'\0';
  }
  if(ext[0] != L'\0') {
    const VXIString* typeFromMap = SBinetInterface::mapExtension(ext);
    if(typeFromMap){
      *strContentType = VXIStringCStr(typeFromMap);
      return(VXIinet_RESULT_SUCCESS);
    }
  }

  /*
   * Could not find in map, use Win32 if available
   */

#ifdef WIN32
  // Try Windows, this supports a number of hardcoded MIME content
  // types as well as extension mapping rules. To define a new
  // extension mapping rule, create a new Win32 registry key called
  // HKEY_CLASSES_ROOT\.<ext> where <ext> is the extension name. Then
  // create a value under that called "Content Type" where the data is
  // the content type string, such as "audio/aiff". Browse that
  // location in the registry for numerous examples (not all have a
  // content type mapping though).
  //
  // TBD sniff the actual data buffer too as this permits, pass the
  // proposed MIME type too (as returned by the web server, if any).
  VXIchar *mimeBuf;
  if (( FindMimeFromData (NULL, url, NULL, 0, NULL, 0,
			  &mimeBuf, 0) == NOERROR ) && 
      mimeBuf && mimeBuf[0] ) {
    *strContentType = mimeBuf;
    return VXIinet_RESULT_SUCCESS;
  }
#endif

  /*
   * Couldn't figure out MIME type, set to default.
   */
  *strContentType = DEFAULT_GENERIC_MIME_TYPE;
  return VXIinet_RESULT_FAILURE; // Should we return success??
}
