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
 * SBinetURL header
 *
 * 
 *
 ***********************************************************************/
#ifndef __SBINETURL_H_                   /* Allows multiple inclusions */
#define __SBINETURL_H_


#include "VXItypes.h"
#include "VXIvalue.h"
#include "VXIinet.h"

#include "SBinetString.hpp"

#define DEFAULT_GENERIC_MIME_TYPE    L"application/octet-stream"
#define DEFAULT_GENERIC_MIME_TYPE_N   "application/octet-stream"

// TODO : This 'SBBoundary' needs to be changed
#define SB_BOUNDARY "osb_inet_multipart_boundary"
#define SB_MULTIPART "multipart/form-data; boundary=osb_inet_multipart_boundary"

class SBinetURL{
public:
  enum URLScheme {
    NULL_SCHEME = 0,
    HTTP_SCHEME = 1,
    FILE_SCHEME = 2 };

public:
  SBinetURL();
  ~SBinetURL();
  URLScheme GetScheme() const;
  const VXIchar*  GetAbsolute() const { return m_strAbsoluteUrl.c_str(); }
  const char* GetAbsoluteNarrow();   // return absolute url Narrow (for fopen)
  const VXIchar*  GetPath() const { return m_strPath.c_str(); }
  const char*  GetPathNarrow();
  VXIinetResult Parse(const VXIchar* pszName,
		      VXIinetOpenMode  eMode,
		      VXIint32         nFlags,
		      const VXIMap*    pProperties);

  void AppendQueryArgs(const VXIMap* m_queryArgs);

  HTAssocList* QueryArgsToHtList(const VXIMap* queryArgs);

  VXIinetResult ContentTypeFromUrl(SBinetString* strContentType) const;


  int NeedMultipart(const VXIMap* queryArgs);

  const char* QueryArgsToMultipart(const VXIMap* queryArgs,VXIulong* plen);

 private:
  void initDoc(int size) { m_doc = ""; }
  void appendStr(const char* src) { m_doc += src; }
  void appendData(const char* src,int size) {
    m_doc.append(src, size);
  }

  const VXIchar* ValueToString(const VXIValue* value, VXIchar* outBuf,
			       VXIunsigned outBufLen);

  void AppendKeyValuePairToURL( const VXIchar *key,
                                const VXIchar *value,
                                VXIbool *isFirstArg );
  void AppendQueryArgsMap( const VXIMap *map,
                           VXIchar *fieldName, 
                           VXIbool *isFirstArg );
  void AppendQueryArgsVector( const VXIVector *vector,
                              VXIchar *fieldName, 
                              VXIbool *isFirstArg );
  void AddObjectToHTAssocList( HTAssocList *arglist,
                               const VXIchar *key,
                               const VXIchar *value );
  void QueryArgsMapToHtList( HTAssocList *arglist, 
                             const VXIMap *map, 
                             VXIchar *fieldName );
  void QueryArgsVectorToHtList( HTAssocList *arglist, 
                                const VXIVector *vector, 
                                VXIchar *fieldName );
  void AppendKeyToMultipart( const VXIchar *key );
  void AppendStringValueToMultipart( const VXIchar *value );
  void QueryArgsMapToMultipart( const VXIMap *map, 
                                VXIchar *fieldName );
  void QueryArgsVectorToMultipart( const VXIVector *vector, 
                                   VXIchar *fieldName );

  VXIinetResult WinInetResolveUrl(const VXIchar* pszBaseUrl, 
				  const VXIchar* pszUrl, 
				  SBinetString*  strAbsoluteUrl,
				  URLScheme*     pScheme);

  VXIinetResult WWWResolveUrl(const VXIchar* pszBaseUrl, 
			      const VXIchar* pszUrl, 
			      SBinetString*  strAbsoluteUrl,
			      URLScheme*     pScheme);

  VXIinetResult NarrowToWideString(const char*    pszNarrow, 
				   SBinetString&     strWide);

  VXIinetResult WideToNarrowString(const VXIchar* pszWide, 
				   SBinetNString& strNarrow);

  char *EscapeString( const VXIchar *value );
  VXIchar *EscapeStringW( const VXIchar *value );


 private:
  SBinetString m_strUrl;
  SBinetString m_strPath; // path for file://
  SBinetNString m_strPathNarrow; // path for file://
  SBinetString m_strBaseUrl;
  SBinetString m_strAbsoluteUrl;
  SBinetNString m_strAbsoluteUrlNarrow;
  URLScheme m_scheme;
  SBinetNString m_doc;
};
#endif







