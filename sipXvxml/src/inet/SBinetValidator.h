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
 * SBinetValidator header
 *
 * 
 *
 ***********************************************************************/
#ifndef __SBVALIDATOR_H_                   /* Allows multiple inclusions */
#define __SBVALIDATOR_H_

#include <string.h>
#include <time.h>    // For time( )

#include "VXIinet.h"          // For VXIinetResult
#include "SBinetLogger.hpp"   // Base class
#include "SBinetString.hpp"   // For SBinetString

#define VALIDATOR_MIME_TYPE L"application/x-vxi-SBinet-validator"

class SBinetValidator : protected SBinetLogger
{
public:
  SBinetValidator(VXIlogInterface *log, VXIunsigned diagTagBase);
  virtual ~SBinetValidator() {}

  // Creation
  VXIinetResult Create(SBinetString url, time_t lastModifiedTime, 
		       time_t expiresTime, unsigned long sizeBytes,
		       const char *eTag, HTAssocList *headers);
  VXIinetResult Create(const VXIValue *value);

  // Serialization
  VXIContent *Serialize() const;

  // Determine if it is expired or modified
  VXIbool Expired() const;
  VXIbool Modified(time_t lastModifiedTime, unsigned long sizeBytes) const;

  // Log the validator to the diagnostic log
  VXIlogResult Log(VXIunsigned tagID, const VXIchar *func) const;

private:
  // VXIContent destructor
  static void ContentDestroy(VXIbyte **content, void *userData);

private:
  time_t  m_expiresTime;
  time_t  m_lastModifiedTime;
  unsigned long m_sizeBytes;
  SBinetNString m_eTag;
  SBinetString m_url;
};

#endif // include guard
