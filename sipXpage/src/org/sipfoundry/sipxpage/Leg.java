/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package sipxpage;

import gov.nist.javax.sip.address.SipUri;
import gov.nist.javax.sip.message.SIPRequest;

import java.net.InetSocketAddress;

import javax.sip.Transaction;
import org.apache.log4j.Logger;

/**
 * Represents a leg of a call.
 * <p>
 * This is the object that gets the LegEvents whenever the LegSipListener
 * notices a SIP message dealing with this Leg's Dialog.  This object
 * is mapped one-to-one with a SIP Dialog.
 * <p>
 * Keeps track of the local RTP port used to advertise SDP, and the
 * remote RTP Addresses where RTP is sent.
 *
 * @author Woof!
 *
 */
public abstract class Leg implements LegListener
{
   static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxpage");
   protected LegSipListener legSipListener;
   protected LegListener otherListener;
   InetSocketAddress remoteRtpAddress;
   InetSocketAddress previousRtpAddress;
   Transaction inviteTransaction;   // The transaction from the INVITE
   boolean isServer ;               // True if the above transaction is a ServerTransaction
   String myId ;                    // An id string for logging
   String callId ;                  // The call-id used in the Dialog
   String displayName ;             // The display name used in the Dialog
   SipUri requestUri ;              // The request URI that created the Dialog
   int rtpPort ;                    // The inbound RTP port (for OUR sdp)
   String tag ;                     // This leg's tag (random number)
   boolean m_gotBye;                // True if the far end sent this leg a bye

   public Leg (LegSipListener legSipListener, LegListener otherListener)
   {
      this.legSipListener = legSipListener ;
      this.otherListener = otherListener ;
      callId = "(not set)" ;
      displayName = "" ;
      tag = Long.toString(java.lang.Math.round(1000000*java.lang.Math.random())) ; // "Cryptograpically Random" says RFC-3261
   }


   /**
    * Tear down this leg, ending any call it is associated with.
    *
    * @throws Exception
    */
   public void destroyLeg() throws Exception
   {
      if (legSipListener != null)
      {
         legSipListener.endCall(this) ;
      }
   }

   /**
    * Accept the offered call.
    *
    * @param rtpPort
    * @throws Exception
    */
   public void acceptCall(int rtpPort) throws Throwable
   {
      setRtpPort(rtpPort) ;
      if (legSipListener != null)
      {
         legSipListener.acceptCall(this) ;
      }
   }

   /**
    * Called by LegSipListener when events of interest to
    * this leg are detected.
    * <p>
    * Forwards on events to otherListener if there is one.
    */
   public boolean onEvent(LegEvent event)
   {
      LOG.debug(String.format("Leg::onEvent %s got event %s%n",
            myId, event.getDescription()));

      if (event.getDescription().equals("sdp"))
      {
         gotSdp(event.getSdpAddress()) ;
      } else if (event.getDescription().startsWith("dialog bye")) {
          m_gotBye = true;
      }

      if (otherListener != null)
      {
         event.setLeg(this) ; // Add this leg to the event so the otherListener can id it from us
         otherListener.onEvent(event) ;
      }
      return true;
   }


   /**
    * Keep track of previous and current remote RTP addresses.
    * This is so as the far end changes its SDP, the leg
    * can track where to send RTP packets, and where to stop
    * sending packets.
    * @param sdpAddress
    */
   void gotSdp(InetSocketAddress sdpAddress)
   {
      LOG.debug(String.format("Leg::gotSdp %s got remote RTP address %s%n", myId,
            sdpAddress.toString()));
      if (remoteRtpAddress == null)
      {
         previousRtpAddress = remoteRtpAddress ;
         remoteRtpAddress = sdpAddress ;
      }
      else
      {
         if (remoteRtpAddress.equals(sdpAddress))
         {
            LOG.debug(String.format("Leg::gotSdp %s already knows remote RTP address %s%n",
                  myId, sdpAddress.toString()));
         }
         else
         {
            LOG.debug(String.format("Leg::gotSdp %s switching remote RTP address %s to %s%n",
                  myId, remoteRtpAddress, sdpAddress.toString()));
            previousRtpAddress = remoteRtpAddress ;
            remoteRtpAddress = sdpAddress ;
         }
      }
   }

   public InetSocketAddress getRemoteRtpAddress()
   {
      return remoteRtpAddress ;
   }

   public InetSocketAddress getPreviousRtpAddress()
   {
      return previousRtpAddress ;
   }

   public void setInviteTransaction(Transaction inviteTransaction, boolean isServer)
   {
      this.inviteTransaction = inviteTransaction ;
      this.isServer = isServer ;
      if (inviteTransaction == null)
      {
         requestUri = null ;
      }
      else
      {
         SIPRequest dog = (SIPRequest)(this.inviteTransaction.getRequest()) ;
         requestUri = (SipUri)dog.getRequestLine().getUri() ;
      }
   }

   public Transaction getInviteTransaction()
   {
      return inviteTransaction ;
   }

   public String getTag()
   {
      return tag ;
   }

   public void setDisplayName(String name)
   {
      displayName = name ;
   }

   public String getDisplayName()
   {
      return displayName ;
   }

   public String getCallId()
   {
      return callId ;
   }

   public SipUri getRequestUri()
   {
      return requestUri ;
   }

   public void setRtpPort(int rtpPort)
   {
      this.rtpPort = rtpPort ;
   }

   public int getRtpPort()
   {
      return rtpPort ;
   }

   public boolean isServer()
   {
      return isServer ;
   }

   public boolean isByed() {
       return m_gotBye;
   }
}
