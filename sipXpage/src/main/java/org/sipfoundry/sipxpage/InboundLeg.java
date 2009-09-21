package sipxpage;

/**
 * Represents an incoming Leg of a call.
 *
 * @author Woof!
 *
 */
public class InboundLeg extends Leg
{
   String fromAddress; // Identity of Page initiator in "user@hostport" format
   String callId;      // Call Id of the initiator call. 

   InboundLeg (LegSipListener legSipListener, LegListener otherListener)
   {
      super(legSipListener, otherListener) ;
      myId = "from nowhere" ;
   }

   public void setAddress(String fromAddress)
   {
      this.fromAddress = fromAddress;
      myId = "from " +	fromAddress;
   }

   public String getAddress()
   {
      return fromAddress ;
   }

   public void setCallId(String callId)
   {
      this.callId = callId;
   }

   public String getCallId()
   {
      return callId ;
   }

}
