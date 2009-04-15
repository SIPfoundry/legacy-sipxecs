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
 * SBinetValidator implementation
 *
 * 
 *
 ***********************************************************************/

#ifndef _SB_USE_STD_NAMESPACE
#define _SB_USE_STD_NAMESPACE
#endif

#include <WWWLib.h>
#ifdef EINVAL
// Conflicts with OS definition
#undef EINVAL
#endif

#include <wchar.h>
#include <stdio.h>

#include "SBinetInternal.h"      // For portability
#include "SBinetValidator.h"     // For this class
#include "SBinetLog.h"           // For logging definitions

#ifdef WIN32
/* Suppress/disable warnings */
#pragma warning(4 : 4706) /* Assignment within conditional expression
			     (for Libwww HTList_[...]( ) macros) */
#endif

/*****************************************************************************
 *****************************************************************************
 * SBinetValidator Implementation
 *****************************************************************************
 *****************************************************************************
 */

SBinetValidator::SBinetValidator(VXIlogInterface *log, VXIunsigned diagTagBase)
  : SBinetLogger(MODULE_SBINET, log, diagTagBase), m_expiresTime((time_t) -1),
    m_lastModifiedTime((time_t) -1), m_sizeBytes(0), m_eTag(), m_url()
{
}


VXIinetResult SBinetValidator::Create(SBinetString url, 
				      time_t lastModifiedTime, 
				      time_t expiresTime, 
				      unsigned long sizeBytes,
				      const char *eTag,
                      HTAssocList *headers)
{
  m_url = url;
  m_lastModifiedTime = lastModifiedTime;
  m_expiresTime = expiresTime;
  m_sizeBytes = sizeBytes;
  if (eTag)
    m_eTag = eTag;
  else
    m_eTag = "";

  // Sanity check
  if((expiresTime >= 0) && (lastModifiedTime >= expiresTime))
    m_expiresTime = 0;

  // Inspect the headers for other relevant directives ('no-cache' etc.)
  HTAssoc * pres;
  char valueBuf[1024];

  // Use a flag to check if any expiration or caching
  // directive was given in the headers (HTTP only)
  VXIbool directiveFound = false;
  if((expiresTime >= 0) || (headers == NULL)) directiveFound = true;

  while ((pres = (HTAssoc *) HTAssocList_nextObject(headers)) != NULL) {
    char *name = HTAssoc_name(pres);
    char *value = HTAssoc_value(pres);
      
    strncpy(valueBuf, value, 1023);
    // We're not taking into account the optional field identifiers ('1#..')
    char *valueTag = strtok(valueBuf, " \t=\r\n");
    char *valueData = strtok(NULL, " \t=\r\n");

    // Cache-control directives can be 'cache-control' and 'pragma'.
    // For 'cache-control', we don't support any custom extensions.
    if (strcasecomp(name, "cache-control") == 0) 
    {
      if((strcasecomp(valueTag, "no-cache") == 0) ||
         (strcasecomp(valueTag, "no-store") == 0) ||
         (strcasecomp(valueTag, "must-revalidate") == 0)) {
        m_expiresTime = 0;
        directiveFound = true;
        break; // This overrides all other directives
      }
      // Don't use 'else' here, as the headers can contain more than one
      // directive, to be treated according to their respective priorities
      if(strcasecomp(valueTag, "max-age") == 0) {
        // Max-age overrides the Expires header.
        if(valueData != NULL) {
          m_expiresTime = time(0) + atoi(valueData);
          directiveFound = true;
        }
      }
      if(strcasecomp(valueTag, "s-maxage") == 0) {
        // For shared caches only, overrides both Max-Age and Expires
        if(valueData != NULL) {
          m_expiresTime = time(0) + atoi(valueData);
          directiveFound = true;
        }
      }
    }
    else if(strcasecomp(name, "pragma") == 0) 
    {
      strncpy(valueBuf, value, 1023);
      // We're not taking into account the optional field identifiers ('1#')
      char *valueTag = strtok(valueBuf, " \t=\r\n");

      if(strcasecomp(valueTag, "no-cache") == 0) {
        m_expiresTime = 0;
        directiveFound = true;
        break; // This overrides all other directives
      }
    }
  }

  // No directive at all, consider immediate expiration and print a warning
  if(!directiveFound) Error(302, L"%s%s", L"URL", url.c_str());

  return VXIinet_RESULT_SUCCESS;
}


VXIinetResult SBinetValidator::Create(const VXIValue *content)
{
  if (VXIValueGetType(content) != VALUE_CONTENT) {
    Error(214, L"%s%d", L"type", VXIValueGetType(content));
    return VXIinet_RESULT_INVALID_ARGUMENT;
  }

  // Get the content
  const VXIchar *contentType;
  const VXIbyte *contentData;
  VXIulong contentSizeBytes;
  if ( VXIContentValue((const VXIContent*)content, &contentType, &contentData, 
		       &contentSizeBytes) != VXIvalue_RESULT_SUCCESS ) {
    Error(215, NULL);
    return VXIinet_RESULT_INVALID_ARGUMENT;
  } else if ( ::wcscmp(contentType, VALIDATOR_MIME_TYPE) != 0 ) {
    Error(216, L"%s%s", L"mimeType", contentType);
    return VXIinet_RESULT_INVALID_ARGUMENT;
  }

  // Unpack the data
  VXIinetResult rVal = VXIinet_RESULT_SUCCESS;
  VXIchar *ptr = (VXIchar *) contentData;
  m_expiresTime = (time_t) ::wcstol(ptr, &ptr, 10);
  if (*ptr != L'\n') {
    Error(226, NULL);
    rVal = VXIinet_RESULT_NON_FATAL_ERROR;
  }
  else
    ptr++;

  if (rVal == VXIinet_RESULT_SUCCESS) {
    m_lastModifiedTime = (time_t) ::wcstol(ptr, &ptr, 10);
    if (*ptr != L'\n') {
      Error(226, NULL);
      rVal = VXIinet_RESULT_NON_FATAL_ERROR;
    }
    else
      ptr++;
  }

  if (rVal == VXIinet_RESULT_SUCCESS) {
    m_sizeBytes = (unsigned long) ::wcstol(ptr, &ptr, 10);
    if (*ptr != L'\n') {
      Error(226, NULL);
      rVal = VXIinet_RESULT_NON_FATAL_ERROR;
    }
    else
      ptr++;
  }

  if (rVal == VXIinet_RESULT_SUCCESS) {
    m_eTag = "";
    while ((*ptr) && (*ptr != L'\n')) {
      m_eTag += (char) *ptr;
      ptr++;
    }

    if (*ptr != L'\n') {
      Error(226, NULL);
      rVal = VXIinet_RESULT_NON_FATAL_ERROR;
    }
    else
      ptr++;
  }

  if (rVal == VXIinet_RESULT_SUCCESS) {
    m_url = "";
    while ((*ptr) && (*ptr != L'\n')) {
      m_url += *ptr;
      ptr++;
    }

    if (*ptr != L'\n') {
      Error(226, NULL);
      rVal = VXIinet_RESULT_NON_FATAL_ERROR;
    }
    else
      ptr++;
  }

  return rVal;
}


VXIContent *SBinetValidator::Serialize() const
{
  // Pack the data
  char tempBuf[32];
  SBinetString tempData;
  if (m_expiresTime == (time_t) -1) {
    tempData += L"-1\n";
  } else {
    sprintf(tempBuf, "%lu\n", (unsigned long) m_expiresTime);
    tempData += tempBuf;
  }
  if (m_lastModifiedTime == (time_t) -1) {
    tempData += L"-1\n";
  } else {
    sprintf(tempBuf, "%lu\n", (unsigned long) m_lastModifiedTime);
    tempData += tempBuf;
  }
  sprintf(tempBuf, "%lu\n", m_sizeBytes);
  tempData += tempBuf;
  tempData += m_eTag.c_str();
  tempData += L'\n';
  tempData += m_url;
  tempData += L'\n';

  // Create the content
  VXIunsigned len = (tempData.length( ) + 1) * sizeof(wchar_t);
  VXIbyte *data = new VXIbyte [len];
  VXIContent *content = NULL;
  if (data) {
    ::wcscpy((VXIchar *) data, tempData.c_str());
    content = VXIContentCreate(VALIDATOR_MIME_TYPE, data, len,
			       ContentDestroy, NULL);
  }

  // Return the content
  if (! content)
    delete [] data;
  data = NULL;

  return content;
}


VXIbool SBinetValidator::Expired() const
{
  if (( m_expiresTime == (time_t) -1 ) || ( time(0) >= m_expiresTime ))
    return TRUE;
  return FALSE;
}


VXIbool SBinetValidator::Modified(time_t lastModifiedTime, 
				  unsigned long sizeBytes) const
{
  if (( lastModifiedTime == m_lastModifiedTime ) && 
      ( sizeBytes == m_sizeBytes ))
    return FALSE;
  return TRUE;
}


VXIlogResult SBinetValidator::Log(VXIunsigned tagID, const VXIchar *func) const
{
  if ( DiagIsEnabled(tagID) ) {
    const char *expiresStr, *lastModStr;
    char expiresBuf[64], lastModBuf[64];
#ifdef WIN32
    if (m_expiresTime == (time_t) -1)
      ::strcpy(expiresBuf, "-1");
    else
      ::strcpy(expiresBuf, ctime(&m_expiresTime));

    if (m_lastModifiedTime == (time_t) -1)
      ::strcpy(lastModBuf, "-1");
    else
      ::strcpy(lastModBuf, ctime(&m_lastModifiedTime));

    expiresStr = expiresBuf;
    lastModStr = lastModBuf;
#else
    if (m_expiresTime == (time_t) -1)
      expiresStr = ::strcpy(expiresBuf, "-1");
    else
      expiresStr = ctime_r(&m_expiresTime, expiresBuf);

    if (m_lastModifiedTime == (time_t) -1)
      lastModStr = ::strcpy(lastModBuf, "-1");
    else
      lastModStr = ctime_r(&m_lastModifiedTime, lastModBuf);
#endif

    return Diag(tagID, func,L"validator: '%s', expires %S, last mod %S, "
		L"size %lu, etag '%S'", 
		m_url.c_str(), expiresStr, lastModStr, m_sizeBytes,
		m_eTag.c_str());
  }

  return VXIlog_RESULT_SUCCESS;
}


void SBinetValidator::ContentDestroy(VXIbyte **content, void *userData)
{
  if ((content) && (*content)) {
    delete [] *content;
    *content = NULL;
  }
}
