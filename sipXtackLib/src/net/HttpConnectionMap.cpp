//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include <string.h>
#include <ctype.h>
#ifdef _VXWORKS
#include <resparse/vxw/hd_string.h>
#endif

#ifdef __pingtel_on_posix__  //needed by linux
#include <wctype.h>
#endif

#include <stdlib.h>
#include <stdio.h>

// APPLICATION INCLUDES
#include <net/HttpMessage.h>
#include <net/HttpConnectionMap.h>
#include <net/NameValuePair.h>
// Needed for SIP_SHORT_CONTENT_LENGTH_FIELD.
#include <net/SipMessage.h>
#include <utl/UtlHashMapIterator.h>

#include <os/OsConnectionSocket.h>
#ifdef HAVE_SSL
#include <os/OsSSLConnectionSocket.h>
#endif /* HAVE_SSL */
#include <os/OsSysLog.h>
#include <os/OsTask.h>
#include <os/OsLock.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define HTTP_READ_TIMEOUT_MSECS  30000

HttpConnectionMap* HttpConnectionMap::pInstance = NULL;
OsBSem HttpConnectionMap::mLock(OsBSem::Q_FIFO, OsBSem::FULL);

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

HttpConnectionMap* HttpConnectionMap::getHttpConnectionMap()
{
    OsLock lock(mLock);

    if (pInstance == NULL)
    {
        pInstance = new HttpConnectionMap();
    }
    return pInstance;
}

void HttpConnectionMap::releaseHttpConnectionMap()
{
    OsLock lock(mLock);

    if (pInstance)
    {
        delete pInstance;
        pInstance = NULL;
    }
}

void HttpConnectionMap::clearHttpConnectionMap()
{
    destroyAll();
}
/* ============================ MANIPULATORS ============================== */

HttpConnectionMapEntry* HttpConnectionMap::getPersistentConnection(const Url& url,
                                                                   OsConnectionSocket*& socket)
{
    UtlString keyString;
    socket = NULL;

    getPersistentUriKey(url, keyString);

    HttpConnectionMapEntry* pEntry;

    { // table lock scope
       OsLock lock(mLock);

       pEntry = dynamic_cast<HttpConnectionMapEntry*>(findValue(&keyString));
       if (!pEntry)
       {
          // Now create a new one
          pEntry = new HttpConnectionMapEntry("ConnectionMapEntry-%d");
          if (pEntry)
          {
             if (insertKeyAndValue(new UtlString(keyString.data()), pEntry) != NULL)
             {
                OsSysLog::add(FAC_HTTP, PRI_DEBUG,
                              "HttpConnectionMap::getPersistentConnection "
                              "- Adding %s for %s",
                              pEntry->data(), keyString.data());
             }
             else
             {
                OsSysLog::add(FAC_HTTP, PRI_ERR,
                              "HttpConnectionMap::getPersistentConnection "
                              "- adding %s (entry %s) failed)",
                              keyString.data(), pEntry->data());
                delete pEntry;
                pEntry = NULL;
             }
          }
       }
    } // end of  table lock

    if (pEntry)
    {
       pEntry->mLock.acquire();
       socket = pEntry->mpSocket;
       pEntry->mbInUse = true;
       OsSysLog::add(FAC_HTTP, PRI_DEBUG,
                     "HttpConnectionMap::getPersistentConnection - Found %s for %s, socket %p",
                     pEntry->data(), keyString.data(), socket);

    }

    return pEntry;
}

void HttpConnectionMap::getPersistentUriKey(const Url& url, UtlString& key)
{
    UtlString urlType;
    UtlString httpHost;
    UtlString httpPort;

    url.getUrlType(urlType);
    url.getHostAddress(httpHost);

    int tempPort = url.getHostPort();

    UtlString httpType = (url.getScheme() == Url::HttpsUrlScheme) ? "https" : "http";
    if (tempPort == PORT_NONE)
    {
        if (httpType == "https")
        {
            httpPort = "443";
        }
        else
        {
            httpPort = "80";
        }
    }
    else
    {
        char t[10];
        sprintf(t, "%d", tempPort);
        httpPort = t;
    }
    key = httpType + ":" + httpHost + ":" + httpPort;
    key.toLower();
}

/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
/* //////////////////////////// PRIVATE /////////////////////////////////// */

HttpConnectionMap::HttpConnectionMap()
{
}

HttpConnectionMap::~HttpConnectionMap()
{
    clearHttpConnectionMap();
}

int HttpConnectionMapEntry::count = 0;

HttpConnectionMapEntry::HttpConnectionMapEntry(const UtlString& name) :
                                 mLock(OsBSem::Q_FIFO, OsBSem::FULL),
                                 mbInUse(true)
{
    char nameBuffer[256];
    sprintf(nameBuffer, name.data(), count++);
    this->append(nameBuffer);
    mpSocket = NULL;
}

HttpConnectionMapEntry::~HttpConnectionMapEntry()
{
    //OsSysLog::add(FAC_HTTP, PRI_DEBUG,
    //              "HttpConnectionMapEntry::destructor %s", this->data());
    if (mpSocket)
    {
        delete mpSocket;
        mpSocket = NULL;
    }
}
