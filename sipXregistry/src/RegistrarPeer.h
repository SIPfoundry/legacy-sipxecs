// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _REGISTRARPEER_H_
#define _REGISTRARPEER_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "net/Url.h"
#include "utl/UtlString.h"
#include "os/OsBSem.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipRegistrar;


/**
 * A peer registrar is another registry server in the same SIP domain
 * with which this registrar synchronizes all registration data.  This class
 * tracks the relationship with a particular peer.  Instances of this class
 * are constructed by SipRegistrar based on the configuration, and may not
 * be destroyed other than by SipRegistrar.
 *
 * The RegistrarPeer object for a given peer may be obtained using SipRegistrar::getPeer.
 *
 * A UtlSListIterator over a list of all RegistrarPeer objects may be obtained
 * using SipRegistrar::getPeerList.
 *
 * There is not a RegistrarPeer object for the local registrar
 * (we do not treat ourselves as a peer).
 *
 * @nosubgrouping
 */
class RegistrarPeer : public UtlString
{
public:

// ================================================================
/** @name                  Addressing
 */
///@{

   /// The unique (fully qualified domain) name of this peer.
   const char* name()
      {
         return UtlString::data();
      }

   /// The full URL to be used to make an XML RPC request of this peer.
   void rpcURL(Url& url);

///@}

// ================================================================
/** @name                  Reachability
 */
///@{

   typedef enum
      {
         Uninitialized,    ///< initial condition, until a successful reset
         Reachable,        ///< a successful reset to the peer has been done
         UnReachable,      ///< the most recent request to this peer failed
         Incompatible      ///< serious error indicating incompatible version
      } SynchronizationState;

   /// Whether or not the most recent attempt to reach this peer succeeded.
   SynchronizationState synchronizationState();
   /**
    * No attempt should be made to push updates to or accept push updates from a peer
    * that is not Reachable; the RegisterTest thread is responsible for attempting
    * to re-establish contact.
    */

   inline bool isReachable() { return synchronizationState() == Reachable; }

   /// Indicate that a request to this peer failed.
   void markUnReachable();
   /**<
    * This triggers the RegisterTest thread to begin polling
    * this peer; until that succeeds or a request is received from this
    * peer, the isReachable method will return false.
    */

   /// Indicate that a request to this peer succeeded or a request was received from it.
   void markReachable();
   /**<
    * If the peer was previously unreachable, this stops the RegisterTest polling.
    * Until the next time markUnReachable is called, isReachable returns true.
    */

   /// Indicate that a permanent error has occurred with this peer.
   void markIncompatible();
   /**<
    * This is used only if an interaction has determined that the peer
    * is broken or otherwise incompatible (for example, response
    * parameters were not the expected type).
    *
    * No further RPC calls will be made to a peer marked Incompatible.
    */

   /// Set the peer state to a non-initial state (any state but Uninitialized)
   void setState(SynchronizationState state);

   /// Return the name of the peer's state, for debugging
   const char* getStateName();

///@}

// ================================================================
/** @name                  Synchronization
 */
///@{

   /// The update number of the oldest update successfully sent to this peer.
   Int64 sentTo();

   /// The update number of the last update received from this peer.
   Int64 receivedFrom();

   void setSentTo(Int64 updateNumber);

   void setReceivedFrom(Int64 updateNumber);


///@}

protected:
   /// only SipRegistrar may construct and destroy RegistrarPeer objects
   friend class SipRegistrar;

   // State names are used for debugging
   static const char* STATE_NAMES[];

   OsBSem               mLock;       ///< must be held to access to other member variables.
   SynchronizationState mSyncState;
   Int64                mSentTo;
   Int64                mReceivedFrom;
   Url                  mUrl;        ///< XML RPC URL
   SipRegistrar*        mRegistrar;

   /// All RegistrarPeer objects are initially considered to be not reachable.
   RegistrarPeer( SipRegistrar*    registrar
                 ,const UtlString& name
                 ,int              rpcPort
                 ,const char*      rpcPath = "/RPC2"
                 );
   ~RegistrarPeer();

private:
   /// There is no copy constructor.
   RegistrarPeer(const RegistrarPeer&);

   /// There is no assignment operator.
   RegistrarPeer& operator=(const RegistrarPeer&);

};

#endif // _REGISTRARPEER_H_
