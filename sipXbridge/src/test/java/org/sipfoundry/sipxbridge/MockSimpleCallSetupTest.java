/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.util.UUID;

import javax.sip.ClientTransaction;
import javax.sip.Dialog;
import javax.sip.DialogTerminatedEvent;
import javax.sip.IOExceptionEvent;
import javax.sip.RequestEvent;
import javax.sip.ResponseEvent;
import javax.sip.SipListener;
import javax.sip.SipProvider;
import javax.sip.TimeoutEvent;
import javax.sip.TransactionTerminatedEvent;
import javax.sip.header.CSeqHeader;
import javax.sip.message.Request;
import javax.sip.message.Response;

import org.apache.log4j.PropertyConfigurator;
import org.sipfoundry.sipxbridge.*;

import junit.framework.TestCase;

/**
 * Test the functionality of the b2bua.
 * 
 * @author M. Ranganathan
 * 
 */

public class MockSimpleCallSetupTest extends TestCase {

    private MockSipxProxy sipxProxy;
    private MockItsp itsp;
    static {

        PropertyConfigurator.configure("log4j.properties");
        System.getProperties().setProperty("startupProperties",
                "selftest.properties");
    }

    public void testSendInviteFromSipxProxy() {
        this.sipxProxy.sendInviteToIbridge();
        try {
            Thread.sleep(10000);

            this.itsp.sendBye();
            if (!itsp.isInviteSeen()) {
                fail("Invite not seen as ITSP");
            }
            if (!itsp.isAckSeen()) {
                fail("Ack not seen at ITSP");
            }
            if (!sipxProxy.isInviteOkSeen()) {
                fail("SipxProxy did not see OK");
            }
            assertTrue("SipProxy: Need 100 count", this.sipxProxy
                    .getReadCount() == 100);
            assertTrue("SipProxy: Need 100 count",
                    this.itsp.getReadCount() == 100);
        } catch (Exception ex) {
            ex.printStackTrace();
            fail("Unexpected exception occured");

        }

    }

    @Override
    public void tearDown() throws Exception {
        sipxProxy.stop();
        itsp.stop();
        Gateway.stop();
        System.clearProperty("startupProperties");
    }

    @Override
    public void setUp() throws Exception {
        super.setUp();
        this.sipxProxy = new MockSipxProxy();
        this.sipxProxy.init(100);
        this.itsp = new MockItsp();
        this.itsp.init(100);
        Gateway.start();

    }

}
