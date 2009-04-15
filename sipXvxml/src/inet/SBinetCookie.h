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
 * SBinetCookie header
 *
 * 
 *
 ***********************************************************************/
#ifndef __SBCOOKIE_H_                   /* Allows multiple inclusions */
#define __SBCOOKIE_H_

#include "VXItypes.h"
#include <string.h>
#include <time.h>    // For time( )

#include "SBinetString.hpp"

class SBinetCookie
{
public: // CTOR/DTOR
    SBinetCookie( const char*   pszDomain,
		  const char*   pszPath,
		  const char*   pszName,
		  const char*   pszValue,
		  const time_t  nExpires,
		  const VXIbool fSecure ) :
      m_pszName(pszName), m_pszValue(pszValue), m_pszDomain(pszDomain),
      m_pszPath(pszPath), m_nExpires(nExpires), m_fSecure(fSecure),
      m_pNext(NULL)
    {
      // All values should be text so we can use assignments above.
      // We keep in small char values as this is what is needed most.
      // Values are checked for NULL before we are called.
      updateTimeStamp();
    }

    virtual ~SBinetCookie( ) {}

public:
    void update( const char*   pszValue,
                 const time_t  nExpires,
                 const VXIbool fSecure ) 
    {
      // All values should be text so we can use strcpy
      // We keep in small char values as this is what is needed most
      // Values are checked for NULL before we are called
      m_pszValue = pszValue;
      m_nExpires = nExpires;
      m_fSecure = fSecure;
      updateTimeStamp();
    }

    bool matchExact( const char* pszDomain, 
		     const char* pszPath, 
		     const char *pszName );

    bool matchRequest( const char* pszDomain, 
		       const char* pszPath );

    bool expired()
    {
      // Zero expiration time means it expires immediately
      return( m_nExpires < time(0) );
    }

    void updateTimeStamp() { m_tTimeStamp = time(0); }

    inline const char* getName()     { return m_pszName.c_str( ); }
    inline const char* getValue()    { return m_pszValue.c_str( ); }
    inline const char* getDomain()   { return m_pszDomain.c_str( ); }
    inline const char* getPath()     { return m_pszPath.c_str( ); }
    inline time_t getExpires() { return m_nExpires; }
    inline VXIbool getSecure() { return m_fSecure; }
    inline time_t getTimeStamp() { return m_tTimeStamp; }

    class SBinetCookie* getNext() { return m_pNext; }
    void setNext(class SBinetCookie* cookie) { m_pNext = cookie; }

private:
    SBinetNString    m_pszName;
    SBinetNString    m_pszValue;
    SBinetNString    m_pszDomain;
    SBinetNString    m_pszPath;
    time_t           m_nExpires;
    VXIbool          m_fSecure;

    time_t           m_tTimeStamp;

    class SBinetCookie* m_pNext; // Linked list
};

#endif // include guard
