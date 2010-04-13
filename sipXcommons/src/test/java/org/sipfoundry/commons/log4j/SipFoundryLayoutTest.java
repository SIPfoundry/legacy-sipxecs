/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.log4j;

import junit.framework.TestCase;

public class SipFoundryLayoutTest extends TestCase
{

   public void testEscapeCrlf()
   {
      SipFoundryLayout l = new SipFoundryLayout() ;
      String msgA = "Woof!" ;
      String msgB ;

      msgB = l.escapeCrlfQuoteAndBackSlash(msgA) ;
      assertEquals(msgA, msgB) ;

/*
 * Trailing \r\n is now removed
      msgB = l.escapeCrlfQuoteAndBackSlash(msgA+"\n") ;
      assertEquals("Woof!\\n", msgB) ;

      msgB = l.escapeCrlfQuoteAndBackSlash(msgA+"\r") ;
      assertEquals("Woof!\\r", msgB) ;

      msgB = l.escapeCrlfQuoteAndBackSlash(msgA+"\r\n") ;
      assertEquals(msgA, msgB) ;
*/
      msgB = l.escapeCrlfQuoteAndBackSlash("\n"+msgA) ;
      assertEquals("\\n"+msgA, msgB) ;

      msgB = l.escapeCrlfQuoteAndBackSlash("\r"+msgA) ;
      assertEquals("\\r"+msgA, msgB) ;

      msgB = l.escapeCrlfQuoteAndBackSlash("\r\n"+msgA) ;
      assertEquals("\\r\\n"+msgA, msgB) ;

      msgB = l.escapeCrlfQuoteAndBackSlash("") ;
      assertEquals("", msgB) ;

      msgB = l.escapeCrlfQuoteAndBackSlash("\n") ;
      assertEquals("\\n", msgB) ;

      msgB = l.escapeCrlfQuoteAndBackSlash("\r\n\n\n") ;
      assertEquals("\\r\\n\\n\\n", msgB) ;

      msgB = l.escapeCrlfQuoteAndBackSlash("\\");
      assertEquals("\\\\",msgB);

      msgB = l.escapeCrlfQuoteAndBackSlash(null) ;
      assertNull(msgB) ;
   }

   public void testTimeFormat()
   {
      SipFoundryLayout l = new SipFoundryLayout() ;
      long ticks = 865915200L*1000;
      assertEquals("1997-06-10T04:00:00.000000Z", l.formatTimestamp(ticks));
      assertEquals("1997-06-10T04:00:00.001000Z", l.formatTimestamp(ticks+1));
      assertEquals("1997-06-10T04:00:00.999000Z", l.formatTimestamp(ticks+999));
   }
}
