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
 * SBinetChannel
 *
 * 
 *
 ***********************************************************************/
#ifndef __SBINETCHANNEL_H_                   /* Allows multiple inclusions */
#define __SBINETCHANNEL_H_

#include "VXItypes.h"
#include "VXIvalue.h"
#include "VXIinet.h"
#include "VXItrd.h"
#include "SBinetLog.h"

#include "HTAssoc.h"

class SBinetStream;
class SBinetCookie;

typedef struct _HTCookie HTCookie;
typedef struct _HTRequest HTRequest;

// Minimum from Cookie spec is 300
#define MAX_COOKIES 500 

class SBinetChannel : public SBinetLogger {
 private:
  SBinetCookie* m_cookies; // Linked list
  VXIunsigned m_numCookies;
  VXIbool m_jarChanged; // For GetCookieJar()
  VXIbool m_cookiesEnabled; // Enable or diable cookie usage

  VXIlogInterface *m_pVXILog;

 public:
  VXIint addCookie(SBinetCookie* cookie);
  VXIint cleanCookies();
  void deleteAllCookies();

  HTAssocList* collectCookies( const char* domain, 
                               const char* path );

  VXIint updateCookieIfExists( const char*   pszDomain,
                               const char*   pszPath,
                               const char*   pszName,
                               const char*   pszValue,
                               const time_t  nExpires,
                               const VXIbool fSecure );
  VXIbool cookiesEnabled() { return m_cookiesEnabled; }

 public:
  SBinetChannel(VXIlogInterface* log, VXIunsigned diagLogBase);
  ~SBinetChannel();

  VXIinetResult CloseAll();                    // Close all Streams
  SBinetStream* GetStream(VXIinetStream* st);   // Really just validates and casts

  VXIinetResult Prefetch(/* [IN]  */ const VXIchar*   pszModuleName,  
                         /* [IN]  */ const VXIchar*   pszName,
			 /* [IN]  */ VXIinetOpenMode  eMode,
			 /* [IN]  */ VXIint32         nFlags,
			 /* [IN]  */ const VXIMap*    pProperties  );

  VXIinetResult Open(/* [IN]  */ const VXIchar*   pszModuleName,
                     /* [IN]  */ const  VXIchar*  pszName,
                     /* [IN]  */ VXIinetOpenMode  eMode,
                     /* [IN]  */ VXIint32         nFlags,
                     /* [IN]  */ const VXIMap*    pProperties,
                     /* [OUT] */ VXIMap*          pmapStreamInfo,
                     /* [OUT] */ VXIinetStream**  ppStream     );

  VXIinetResult Close(/* [IN]  */ VXIinetStream**  ppStream     );
  
  VXIinetResult Read(/* [OUT] */ VXIbyte*         pBuffer,
                     /* [IN]  */ VXIulong         nBuflen,
                     /* [OUT] */ VXIulong*        pnRead,
                     /* [IN]  */ VXIinetStream*   pStream      );
  
  VXIinetResult Write(/* [OUT] */ const VXIbyte*   pBuffer,
                      /* [IN]  */ VXIulong         nBuflen,
                      /* [OUT] */ VXIulong*        pnWritten,
                      /* [IN]  */ VXIinetStream*   pStream      );

  VXIinetResult SetCookieJar( /* [IN]  */ const VXIVector*    pJar );
  
  VXIinetResult GetCookieJar( /* [OUT] */ VXIVector**      ppJar,
                              /* [OUT] */ VXIbool*         pfChanged    );
};

// Per thread object that implements VXIinetInterface (in Java sense)
//  Simply contains static routines that call corresponding
//   methods on the SBinetChannel instance
//    
class SBinetInterface : public VXIinetInterface, public SBinetLogger {
private:
  SBinetChannel* m_ch;

public: // CTOR/DTOR 
  SBinetInterface( VXIlogInterface* pVXILog, VXIunsigned diagLogBase );
  ~SBinetInterface();

  static VXIint32 GetVersion(void);
  
  static const VXIchar* GetImplementationName(void);
  
  static VXIinetResult Prefetch(/* [IN]  */ VXIinetInterface*      pThis,
                                /* [IN]  */ const VXIchar*   pszModuleName,  
                                /* [IN]  */ const VXIchar*   pszName,
                                /* [IN]  */ VXIinetOpenMode  eMode,
                                /* [IN]  */ VXIint32         nFlags,
                                /* [IN]  */ const VXIMap*    pProperties  );


  static VXIinetResult Open(/* [IN]  */ VXIinetInterface*      pThis,
                            /* [IN]  */ const VXIchar*   pszModuleName,
                            /* [IN]  */ const  VXIchar*  pszName,
                            /* [IN]  */ VXIinetOpenMode  eMode,
                            /* [IN]  */ VXIint32         nFlags,
                            /* [IN]  */ const VXIMap*    pProperties,
                            /* [OUT] */ VXIMap*          pmapStreamInfo,
                            /* [OUT] */ VXIinetStream**  ppStream     );

  static VXIinetResult Close(/* [IN]  */ VXIinetInterface*      pThis,
                             /* [IN]  */ VXIinetStream**  ppStream     );
  
  static VXIinetResult Read(/* [IN]  */ VXIinetInterface*      pThis,
                            /* [OUT] */ VXIbyte*         pBuffer,
                            /* [IN]  */ VXIulong         nBuflen,
                            /* [OUT] */ VXIulong*        pnRead,
                            /* [IN]  */ VXIinetStream*   pStream      );
  
  static VXIinetResult Write(/* [IN]  */ VXIinetInterface*      pThis,
                             /* [OUT] */ const VXIbyte*   pBuffer,
                             /* [IN]  */ VXIulong         nBuflen,
                             /* [OUT] */ VXIulong*        pnWritten,
                             /* [IN]  */ VXIinetStream*   pStream      );

  static VXIinetResult SetCookieJar( /* [IN]  */ VXIinetInterface*      pThis,
                                     /* [IN]  */ const VXIVector*       pJar );
  
  static VXIinetResult GetCookieJar( /* [IN]  */ VXIinetInterface* pThis,
                                     /* [OUT] */ VXIVector**       ppJar,
                                     /* [OUT] */ VXIbool*          pfChanged );
  static void LockLibwww( );
  static void UnlockLibwww( );

  // Callbacks from libwww for cookies
  static BOOL setCookie(HTRequest* pHtRequest, HTCookie* pHtCookie, void* pParam );
  static HTAssocList* findCookie( HTRequest* pHtRequest, void* pParam);
  
  // Static method to use global ext map
  static const VXIString* mapExtension(const VXIchar* ext );

};
#endif

