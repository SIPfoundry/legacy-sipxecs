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
      
      msgB = l.escapeCrlfQuoteAndBackSlash(msgA+"\n") ;
      assertEquals(msgA, msgB) ;

      msgB = l.escapeCrlfQuoteAndBackSlash(msgA+"\r") ;
      assertEquals(msgA, msgB) ;

      msgB = l.escapeCrlfQuoteAndBackSlash(msgA+"\r\n") ;
      assertEquals(msgA, msgB) ;

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
      assertEquals("\\r", msgB) ;
      
      msgB = l.escapeCrlfQuoteAndBackSlash("\\");
      assertEquals("\\\\",msgB);
      
      msgB = l.escapeCrlfQuoteAndBackSlash(null) ;
      assertNull(msgB) ;
   }

}
