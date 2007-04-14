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
 * SBinetCookie implementation
 *
 * 
 *
 ***********************************************************************/

#ifndef _SB_USE_STD_NAMESPACE
#define _SB_USE_STD_NAMESPACE
#endif

#include "SBinetCookie.h"

/*****************************************************************************
 *****************************************************************************
 * SBinetCookie Implementation
 *****************************************************************************
 *****************************************************************************
 */

bool SBinetCookie::matchExact( const char* pszDomain, 
			       const char* pszPath, 
			       const char *pszName )
{
  if(strcmp(m_pszName.c_str( ), pszName)) return(false);
  if(strcmp(m_pszDomain.c_str( ), pszDomain)) return(false);
  if(strcmp(m_pszPath.c_str( ), pszPath)) return(false);

  return(true);
}


bool SBinetCookie::matchRequest( const char* pszDomain, 
				 const char* pszPath)
{
  if (( ! pszDomain ) || ( ! pszPath ))
    return false;

  // Domain name must belong to the domain for the cookie, TBD this is
  // overly restrictive.
  if(strcmp(m_pszDomain.c_str( ), pszDomain) != 0)
    return false;

  // Path must match the cookie's path attribute, or represent a child
  // of the path attribute. The filename portion of the request path
  // has already been removed, may or may not have a trailing slash.
  const char *ptr = pszPath, *cPtr = m_pszPath.c_str( );

  if (((*cPtr == '/') || (*cPtr == '\\')) && (*(cPtr+1) == '\0'))
    cPtr++;   // Strip trailing slash on cookie path

  while ((*ptr) && (*cPtr) && (*ptr == *cPtr)) {
    ptr++;
    cPtr++;
    
    if (((*cPtr == '/') || (*cPtr == '\\')) && (*(cPtr+1) == '\0'))
      cPtr++;    // Strip trailing slash on cookie path
  }

  // Cookie path must have terminated and we must be up to a new
  // component of the path in the path portion
  if ((*cPtr) || ((*ptr) && (*ptr != '/') && (*ptr != '\\')))
    return false;

  // TBD, if port was specified by the remote web server for the
  // cookie, the port must be included in the list of ports of the
  // cookie's port attribute. Libwww isn't returning that information
  // to us so we don't even have a port stored in our cookies.

  return true;
}
