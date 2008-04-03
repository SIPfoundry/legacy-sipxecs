/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import org.apache.log4j.PropertyConfigurator;
import org.sipfoundry.sipxbridge.Gateway;
import org.sipfoundry.sipxbridge.ItspAccountInfo;

import junit.framework.TestCase;

public class SimpleCallSetupTest extends TestCase {

    private MockSipxProxy sipxProxy;

    static {

        PropertyConfigurator.configure("log4j.properties");
    }

    public void testSendInviteFromSipxProxy() {
        try {
          
            this.sipxProxy.sendInviteToIbridge();

            Thread.sleep(15000);

            if (!sipxProxy.isInviteOkSeen()) {
                fail("SipxProxy did not see OK");
            }
        } catch (Exception ex) {
            ex.printStackTrace();
            fail("Unexpected exception occured");

        }

    }

    @Override
    public void tearDown() throws Exception {
        sipxProxy.stop();

        Gateway.stop();
    }

    @Override
    public void setUp() throws Exception {
        super.setUp();
        Gateway.init();
        this.sipxProxy = new MockSipxProxy();
        this.sipxProxy.init(100);

    }

}
