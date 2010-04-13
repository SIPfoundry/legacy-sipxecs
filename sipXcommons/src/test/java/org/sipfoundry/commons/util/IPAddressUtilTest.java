/**
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.util;

import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;


/**
* Basic test of the IPAddressUtil class.
* <p>
*
* @author Mardy Marshall
*/
public class IPAddressUtilTest extends TestCase {
   /**
    * Create the test case
    *
    * @param testName name of the test case
    */
   public IPAddressUtilTest(String testName) {
       super(testName);
   }

   /**
    * @return the suite of tests being tested
    */
   public static Test suite() {
       return new TestSuite(IPAddressUtilTest.class);
   }

   /**
    * Rigorous Test :-)
    */
   public void testApp() {
	   // Negative tests.
       if (IPAddressUtil.isLiteralIPAddress("192.168.o.8") == true) {
           fail("Failed to disqualify: 192.168.o.8");
       }

       if (IPAddressUtil.isLiteralIPAddress("192.168.0/24") == true) {
           fail("Failed to disqualify: 192.168.0/24");
       }

       if (IPAddressUtil.isLiteralIPAddress("192.168.0.24.nortel.com") == true) {
           fail("Failed to disqualify: 192.168.0.24.nortel.com");
       }

       if (IPAddressUtil.isLiteralIPAddress("www.nortel.com") == true) {
           fail("Failed to disqualify: www.nortel.com");
       }

       // Positive tests.
       if (IPAddressUtil.isLiteralIPAddress("192.168.0.8") == false) {
           fail("Failed to qualify: 192.168.0.8");
       }

       if (IPAddressUtil.isLiteralIPAddress("10.1.8.56") == false) {
           fail("Failed to qualify: 10.1.8.56");
       }

       if (IPAddressUtil.isLiteralIPAddress("176.25.10.201") == false) {
           fail("Failed to qualify: 176.25.10.201");
       }
   }

}
