package org.sipfoundry.commons.log4j;

import junit.framework.TestCase;

public class SipFoundryLayoutTest extends TestCase
{

   public void testEscapeCrlf()
   {
      SipFoundryLayout l = new SipFoundryLayout() ;
      String msgA = "Woof!" ;
      String msgB ;
      
      msgB = l.escapeCrlf(msgA) ;
      assertEquals(msgA, msgB) ;
      
      msgB = l.escapeCrlf(msgA+"\n") ;
      assertEquals(msgA, msgB) ;

      msgB = l.escapeCrlf(msgA+"\r") ;
      assertEquals(msgA, msgB) ;

      msgB = l.escapeCrlf(msgA+"\r\n") ;
      assertEquals(msgA, msgB) ;

      msgB = l.escapeCrlf("\n"+msgA) ;
      assertEquals("\\n"+msgA, msgB) ;

      msgB = l.escapeCrlf("\r"+msgA) ;
      assertEquals("\\r"+msgA, msgB) ;

      msgB = l.escapeCrlf("\r\n"+msgA) ;
      assertEquals("\\r\\n"+msgA, msgB) ;

      msgB = l.escapeCrlf("") ;
      assertEquals("", msgB) ;

      msgB = l.escapeCrlf("\n") ;
      assertEquals("\\n", msgB) ;

      msgB = l.escapeCrlf("\r\n\n\n") ;
      assertEquals("\\r", msgB) ;
      
      msgB = l.escapeCrlf(null) ;
      assertNull(msgB) ;
   }

}
