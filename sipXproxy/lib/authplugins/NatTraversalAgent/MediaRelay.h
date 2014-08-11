//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _MEDIARELAY_H_
#define _MEDIARELAY_H_

// SYSTEM INCLUDES
#include <vector>
#include "utl/UtlString.h"
#include "net/Url.h"
#include "os/OsMutex.h"
#include "os/OsServerTask.h"
#include "os/OsMsg.h"
#include "os/OsTimer.h"
#include "os/OsNotification.h"

// APPLICATION INCLUDES
#include <sipxproxy/AuthPlugin.h>
#include "NatTraversalAgentDataTypes.h"

// DEFINES
#define INVALID_MEDIA_RELAY_HANDLE (-1)

/**
 * The class is used to abstract the Symmitron which is the external process
 * that actually performs the media relaying.  Applicatiosn needing to use the
 * services of the Symmitron need to isntantiate and initialize() a MediaRelay
 * object which will take care of all the communication with the Symmitron.  The
 * API it exposes lets the application perform all the necessary operations
 * to utilize the services of the Symmitron.
 */
// : public OsNotification
class MediaRelay
{
   public:
     MediaRelay();

     ~MediaRelay();

     /**
      * Method to call to initialize the MediaRelay object.
      *  - publicAddress: [input] public IP address of Symmitron.
      *                   If the symmitron is behind a NAT, that value should
      *                   be the public IP address of that NAT otherwise it is
      *                   the IP address of the machine it is running on.
      *  - nativeAddress: [input] IP address of the machine the Symmitron is running on
      *  - bXmlRpcSecured: [input] true will send XMLRPC over HTTPS, otherwise it will use HTTP.
      *  - isPartOfsipXLocalPrivateNetwork: [input] True of the symmitron is part
      *                                     of the same local private network as the
      *                                     application that will utilize the media relay.
      *  - xmlRpcPort: [input] Port number to utilize to communicate with Symmitron.
      *  - maxMediaRelaySessions: [input] maximum of media relay sessions to allocate
      *
      * Returns: true for success and false for failure.
      */
      bool initialize( const  UtlString& publicAddress,
                       const  UtlString& nativeAddress,
                       bool   isPartOfsipXLocalPrivateNetwork,
                       int    port);

      /**
       * Getter for isPartOfsipXLocalPrivateNetwork setting passed to initialize() method
       */
      bool isPartOfsipXLocalPrivateNetwork( void ) const{ return mbIsPartOfsipXLocalPrivateNetwork; }

      /**
       * Getter for publicAddress setting passed to initialize() method
       */
      const UtlString& getPublicAddress( void ) const{ return mPublicAddress; }

      /**
       * Getter for nativeAddress setting passed to initialize() method
       */
      const UtlString& getNativeAddress( void ) const{ return mNativeAddress; }



   private:
      // clean up
      void cleanUpEverything( void );

      UtlString                      mPublicAddress;            // public IP address of the media relay
      UtlString                      mNativeAddress;            // native address of the media relay
      bool                           mbIsPartOfsipXLocalPrivateNetwork;  // indicates whether the media relay is inside the same local private network as sipXecs
      size_t                         mMaxMediaRelaySessions;    // Configured maximum number of media relay sessions allowed
      OsMutex                        mMutex;
      int                            mPort;
};

#endif
