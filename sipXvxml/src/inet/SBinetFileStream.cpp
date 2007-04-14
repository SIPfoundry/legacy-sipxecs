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
 * SBinetFileStream Implementation
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

#include "VXIvalue.h"
#include "VXIinet.h"
#include "VXItrd.h"

#include "SBinetLog.h"
#include "SBinetURL.h"
#include "SBinetStream.h"
#include "SBinetValidator.h"

#include <stdio.h>

#ifdef WIN32
typedef struct _stat VXIstat;
#else
typedef struct stat  VXIstat;
#endif

static
int  VXIStat(const char *fname, VXIstat *statInfo)
{
  int ret=0;
#ifdef WIN32
  ret = ::_stat(fname, statInfo);
#else
  ret = ::stat(fname, statInfo);
#endif
  return ret;
}


VXIinetResult
SBinetFileStream::Open(VXIint32                  flags,
		       const VXIMap             *properties,
		       VXIMap                   *streamInfo)
{
  VXIinetResult returnValue = VXIinet_RESULT_SUCCESS;

  /* 
     Get the flag to determine whether to open local files. Also check
     the validator to see if it contains INET_INFO_VALIDATOR and if it
     does contain the modified time. */
  SBinetValidator validator(GetLog(), GetDiagBase());
  const VXIValue *validatorVal = NULL;
  if(properties)
  {
    const VXIValue *val = VXIMapGetProperty(properties, INET_OPEN_LOCAL_FILE);
    if(val != NULL)
    {
      if (VXIValueGetType(val) == VALUE_INTEGER)
      {
	if ( VXIIntegerValue((const VXIInteger *) val) == FALSE )
	  returnValue = VXIinet_RESULT_LOCAL_FILE;
      } 
      else
      {
	Diag(MODULE_SBINET_STREAM_TAGID, L"SBinetFileStream::Open",
	     L"Passed invalid type for INET_OPEN_LOCAL_FILE property");
      }
    }

    // SBinetValidator::Create() logs errors for us
    validatorVal = VXIMapGetProperty(properties, INET_OPEN_IF_MODIFIED);
    if((validatorVal != NULL) &&
       (validator.Create(validatorVal) != VXIinet_RESULT_SUCCESS))
      validatorVal = NULL;
  }

  Diag(MODULE_SBINET_STREAM_TAGID, L"SBinetFileStream::Open",
       m_url->GetAbsolute( ));

  // Stat the file to make sure it exists and to get the length and
  // last modified time
  VXIstat statInfo;
  if ( VXIStat(m_url->GetPathNarrow(), &statInfo) != 0 ) {
    Error(222, L"%s%s", L"File", m_url->GetPath());
    return(VXIinet_RESULT_NOT_FOUND);
  }

  m_content_length = statInfo.st_size;

  // If there is a conditional open using a validator, see if the
  // validator is still valid (no need to re-read and re-process)
  if(validatorVal) {
    validator.Log(MODULE_SBINET_STREAM_TAGID, 
		  L"SBinetFileStream::Open (check validator)");

    if (! validator.Modified(statInfo.st_mtime, statInfo.st_size))
      returnValue = VXIinet_RESULT_NOT_MODIFIED;
  }

  if(( returnValue != VXIinet_RESULT_NOT_MODIFIED ) &&
     ( returnValue != VXIinet_RESULT_LOCAL_FILE )) {
    // Use fopen for now. Note: Open support reads for now
    m_pFile = ::fopen( m_url->GetPathNarrow(), "rb" );
    if(!m_pFile){
      Error(223, L"%s%s", L"File", m_url->GetPath());
      return(VXIinet_RESULT_NOT_FOUND);
    }
    m_ReadSoFar = 0;
  }

  if( streamInfo != NULL )
  {
    // Use extension mapping algorithm to determine MIME content type
    SBinetString guessContent;
    m_url->ContentTypeFromUrl(&guessContent);
    VXIMapSetProperty(streamInfo, INET_INFO_MIME_TYPE,
		    (VXIValue*)VXIStringCreate( guessContent.c_str()));

    // Set the absolute path, any file:// prefix and host name is
    // already stripped prior to this method being invoked
    VXIMapSetProperty(streamInfo, INET_INFO_ABSOLUTE_NAME, 
		      (VXIValue*)VXIStringCreate( m_url->GetPath() ));

    // Set the validator property
    VXIValue *newValidator = NULL;
    if (returnValue == VXIinet_RESULT_NOT_MODIFIED) {
      newValidator = VXIValueClone(validatorVal);
    } else {
      SBinetValidator validator(GetLog(), GetDiagBase());
      if (validator.Create(m_url->GetPath(), statInfo.st_mtime, (time_t) -1,
			   statInfo.st_size, NULL, NULL) == VXIinet_RESULT_SUCCESS) {
	newValidator = (VXIValue *) validator.Serialize();
	validator.Log(MODULE_SBINET_STREAM_TAGID, 
		      L"SBinetFileStream::Open (new validator)");
      }
    }
      
    if (newValidator)
      VXIMapSetProperty(streamInfo, INET_INFO_VALIDATOR, newValidator);
    else {
      Error(103, NULL);
      returnValue = VXIinet_RESULT_OUT_OF_MEMORY;
    }

    // Set the size
    VXIMapSetProperty(streamInfo, INET_INFO_SIZE_BYTES, 
		      (VXIValue*)VXIIntegerCreate( m_content_length));
  }

  return returnValue;
}


//
// Read is the same for all File requests (GET and POST)
//
VXIinetResult
SBinetFileStream::Read(VXIbyte                 *buffer,
		       VXIulong                 buflen,
		       VXIulong                *nread)
{
  if(!buffer || !nread) {
    Error(200, L"%s%s", L"Operation", L"Read");
    return(VXIinet_RESULT_INVALID_ARGUMENT);
  }
  if(!m_pFile) return(VXIinet_RESULT_FAILURE);
  int toRead = buflen;
  /*
   * Note: don't try to remember hos many byte are left in file and
   *   set toRead based on this, or you will never get EOF
   */
  int n = ::fread(buffer,sizeof(VXIbyte),toRead,m_pFile);
  if(n <= 0){
    // Check for EOF, otherwise ERROR
    //    printf("read %d eof = %d\n",n,feof(m_pFile));
    if( feof(m_pFile)){
      *nread = 0;
      return(VXIinet_RESULT_END_OF_STREAM);
    }
    else{
      Close();
      Error(224, L"%s%s", L"File", m_url->GetPath());
      return(VXIinet_RESULT_FAILURE);
    }
  }
  *nread = n;
  if( feof(m_pFile)){
    return(VXIinet_RESULT_END_OF_STREAM);
  }
  return(VXIinet_RESULT_SUCCESS);
}


VXIinetResult
SBinetFileStream::Close(){
  /* Clean up the request */
  if(m_pFile){
    ::fclose(m_pFile);
    m_pFile = NULL;
    return(VXIinet_RESULT_SUCCESS);
  }
  else {
    Error(200, L"%s%s", L"Operation", L"Close");
    return(VXIinet_RESULT_INVALID_ARGUMENT);
  }
}

SBinetFileStream::SBinetFileStream(SBinetURL* url, VXIlogInterface *log,
				   VXIunsigned diagLogBase):
  SBinetLogger(MODULE_SBINET_STREAM, log, diagLogBase)
{
  m_pFile = NULL;
  m_content_length = 0;
  m_ReadSoFar = 0;
  m_url = url;
}

SBinetFileStream::~SBinetFileStream()
{
  if(m_pFile) Close();
  if(m_url) delete m_url;
}
