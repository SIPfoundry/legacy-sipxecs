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
 * SBinetStream Header
 *
 * 
 *
 ***********************************************************************/
#ifndef __SBINETSTREAM_H_                   /* Allows multiple inclusions */
#define __SBINETSTREAM_H_

#include <time.h>
#include <WWWLib.h>
#include <WWWHTTP.h>
#include <WWWInit.h>

#include "VXItypes.h"
#include "VXIvalue.h"
#include "VXIinet.h"
#include "SBinetLog.h"

#include "SBinetURL.h"
#include "SBinetCacheLock.h"

#include "os/OsStatus.h"
#include "net/HttpMessage.h"
#include "net/Url.h"

/*
 * Top level stream class, virtual abstract.
 */
class SBinetStream{
 public:
  virtual ~SBinetStream(){}
  virtual VXIinetResult Prefetch() = 0;
  virtual VXIinetResult Open(VXIint flags, const VXIMap* properties, VXIMap* streamInfo) = 0;
  virtual VXIinetResult Read( /* [OUT] */ VXIbyte*         pBuffer,
			      /* [IN]  */ VXIulong         nBuflen,
			      /* [OUT] */ VXIulong*        pnRead ) = 0;
  virtual VXIinetResult Close() = 0;
};

struct SBinetFileValidator;

class SBinetFileStream : public SBinetStream, public SBinetLogger {
 public:
  SBinetFileStream(SBinetURL* url, VXIlogInterface *log, 
		   VXIunsigned diagLogBase);
  ~SBinetFileStream();
  VXIinetResult Prefetch()
    {return(VXIinet_RESULT_SUCCESS);}
  VXIinetResult Open(VXIint flags, 
		     const VXIMap* properties, 
		     VXIMap* streamInfo);
  VXIinetResult Read( /* [OUT] */ VXIbyte*         pBuffer,
		      /* [IN]  */ VXIulong         nBuflen,
		      /* [OUT] */ VXIulong*        pnRead );
  VXIinetResult Close();

 private:
  void LogValidator(const VXIchar *func,
		    const SBinetFileValidator *validator) const;

 private:
  FILE* m_pFile;
  VXIint m_content_length;
  int m_ReadSoFar;
  SBinetURL* m_url;
};


class SBinetChannel;
struct SBinetHttpValidator;

class SBinetHttpStream : public SBinetStream, public SBinetLogger {
 public:
  enum SubmitMethod { GET_METHOD = 1, POST_METHOD = 2 };

 public:
  SBinetHttpStream(SBinetURL* url, SBinetChannel* ch, 
                   VXIlogInterface *log, VXIunsigned diagLogBase);
  ~SBinetHttpStream();

  VXIinetResult Prefetch()
    {return(VXIinet_RESULT_SUCCESS);}
  VXIinetResult Open(VXIint flags, 
		     const VXIMap* properties, 
		     VXIMap* streamInfo);
  VXIinetResult Read( /* [OUT] */ VXIbyte*         pBuffer,
		      /* [IN]  */ VXIulong         nBuflen,
		      /* [OUT] */ VXIulong*        pnRead );
  VXIinetResult Close();

  SBinetChannel* getChannel() { return(m_ch); }

  HTChunk* Post(SBinetURL*               url,
		VXIint32                  flags,
		const VXIMap             *properties,
		VXIMap                   *streamInfo);

  static void Init(const VXIMap * configArgs);

 private:
  HTChunk* PostMulti(SBinetURL*       url,
		     VXIint32                  flags,
		     const VXIMap             *properties,
		     VXIMap                   *streamInfo);

   /*
    * Our own POST for multipart mime.
    */
   UtlBoolean OsPostData(HTParentAnchor* source,
                        HTAnchor* destination, 
                        SBinetURL* url);

   /*
    * Our own GET.
    */
   UtlBoolean OsGetData(HTAnchor* anchor, SBinetURL* url);

  int getTime(time_t *timestamp, VXIunsigned *timestampMsec);

  int checkTimeout();
  
  HTChunk* Get(SBinetURL*               url,
	       VXIint32                  flags,
	       const VXIMap             *properties,
	       VXIMap                   *streamInfo);

  void SetProperties(VXIMap* properties);
  void GetHeaderInfo(VXIMap* streamInfo);
  
  VXIinetResult SetCachingMode( );

  static int terminate_handler (HTRequest * request, 
				HTResponse * response,
				void * param, int status);

  VXIinetResult MapError(int ht_error);

  void LogValidator(const VXIchar *func,
		    const SBinetHttpValidator *validator) const;

 private:

  HTRequest* m_request;
  HTChunk* m_chunk;
  SBinetString m_argstring;     // string representation of query ares for POST
  VXIMap* m_queryArgs;
  int m_Done;
  int m_ReadSoFar;
  int m_HttpStatus;    

  SBinetURL* m_url;
  VXIint m_iMaxAge;
  VXIint m_iMaxStale;
  long m_nTimeoutOpen;
  long m_nTimeoutIO;
  SubmitMethod m_method;
  SBinetNString m_submitMimeType;         // Real char* not VXIchar*
  // Header info
  VXIint m_content_length;
  SBinetString m_content_type;
  time_t m_ReferenceTime;
  VXIunsigned m_ReferenceTimeMsec;
  HTParentAnchor* m_newAnchor;
  SBinetCacheLock* m_lock;
  int m_reader;
  SBinetChannel* m_ch;  // for cookie mapping

  static int sm_instanceCount;
  static int sm_cleanupPeriod;
};

#endif
