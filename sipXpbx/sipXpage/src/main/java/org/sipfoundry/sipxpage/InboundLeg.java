package sipxpage;

import javax.sdp.SessionDescription;
import javax.sip.address.SipURI;

/**
 * Represents an incoming Leg of a call.
 * 
 * @author Woof!
 *
 */
public class InboundLeg extends Leg
{
   SipURI fromAddress ;                   // The SIP destination who invited us
   
   InboundLeg (LegSipListener legSipListener, LegListener otherListener)
   {
      super(legSipListener, otherListener) ;
      myId = "from nowhere" ;
   }

   public void createLeg(SipURI fromAddress, String displayName, SessionDescription sdp) throws Exception
   {
      this.fromAddress = fromAddress ;
      myId = "from "+ fromAddress.toString() ;
   }
   
   
   public SipURI getAddress()
   {
      return fromAddress ;
   }
   
}
