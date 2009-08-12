/**
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.discovery;

import org.sipfoundry.commons.icmp.Pinger;

import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;

/**
 * Basic test of the Pinger class.
 * <p>
 * NOTE THAT THIS TEST MUST BE RUN WITH ROOT PRIVILEGES.
 *
 * @author Mardy Marshall
 */
public class PingerTest extends TestCase {
    /**
     * Create the test case
     *
     * @param testName name of the test case
     */
    public PingerTest(String testName) {
        super(testName);
    }

    /**
     * @return the suite of tests being tested
     */
    public static Test suite() {
        return new TestSuite(PingerTest.class);
    }

    /**
     * Rigorous Test :-)
     */
    public void testApp() {
        Pinger pinger = new Pinger(1234, "127.0.0.1", 2000);

        assertTrue(pinger.ping());
    }

}
