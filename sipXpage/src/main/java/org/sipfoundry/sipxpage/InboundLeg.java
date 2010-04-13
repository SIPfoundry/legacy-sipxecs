/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
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
