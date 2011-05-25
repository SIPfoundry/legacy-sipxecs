/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package sipxpage;

import java.net.InetSocketAddress;

/**
 * An event sent to LegListeners.
 *
 * @author Woof!
 *
 */
public class LegEvent
{
   String description ;
   InetSocketAddress sdpAddress ;
   Leg leg ;

   public LegEvent(Leg leg, String what)
   {
      this.leg = leg ;
      this.description = what ;
   }

   public String getDescription()
   {
      return description ;
   }

   public void setSdpAddress(InetSocketAddress addressPort)
   {
      this.sdpAddress = addressPort ;
   }

   public InetSocketAddress getSdpAddress()
   {
      return sdpAddress ;
   }

   public void setLeg(Leg leg)
   {
      this.leg = leg ;
   }

   public Leg getLeg()
   {
      return leg ;
   }
}
