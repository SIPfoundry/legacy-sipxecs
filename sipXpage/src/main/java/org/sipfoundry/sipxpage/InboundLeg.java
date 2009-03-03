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
   
}
