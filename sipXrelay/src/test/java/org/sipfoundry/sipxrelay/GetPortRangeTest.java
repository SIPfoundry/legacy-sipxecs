/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxrelay;

import java.util.HashMap;

import org.apache.log4j.Logger;
import org.apache.xmlrpc.XmlRpcException;
import org.sipfoundry.sipxrelay.SymmitronServer;

public class GetPortRangeTest extends AbstractSymmitronTestCase {
    private static Logger logger = Logger.getLogger(GetPortRangeTest.class);
  

    /*
     * (non-Javadoc)
     * 
     * @see junit.framework.TestCase#setUp()
     */

    protected void setUp() throws Exception {
        super.setUp();
        super.start();

    }

    public void testStartServer() {
        try {
            HashMap<String, Integer> retval = (HashMap<String, Integer>) client
                    .execute("sipXbridge.getRtpPortRange", (Object[]) null);

            assertTrue("Port Range Lower Bound Must be 30000", retval
                    .get("lowerBound") == 30000);

        } catch (XmlRpcException e) {
            e.printStackTrace();
            logger.error("Unexpected exeption ", e);
            fail("Unexpected exception");

        }

    }

    protected void tearDown() throws Exception {
        super.tearDown();
        client.execute("sipXrelay.stop", (Object[]) null);
        SymmitronServer.stopXmlRpcServer();
    }

}
