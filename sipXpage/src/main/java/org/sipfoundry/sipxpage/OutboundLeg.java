package sipxpage;

import javax.sdp.SessionDescription;
import javax.sip.address.SipURI;


/**
 * Represents and outgoing Leg of a call.
 * 
 * @author Woof!
 *
 */
public class OutboundLeg extends Leg implements LegListener
{
   SipURI toAddress;

   public OutboundLeg (LegSipListener legSipListener, LegListener otherListener)
   {
      super(legSipListener, otherListener) ;
      myId = "to nowhere" ;
   }
   
   public void createLeg(SipURI toAddress, String displayName, SessionDescription sdp, String alertInfoKey) throws Throwable
   {
      this.toAddress = toAddress ;
      myId = "to "+ toAddress.toString() ;
      callId = legSipListener.placeCall(this, toAddress, displayName, sdp, alertInfoKey) ;
   }

   public SipURI getAddress()
   {
      return toAddress ;
   }
}
