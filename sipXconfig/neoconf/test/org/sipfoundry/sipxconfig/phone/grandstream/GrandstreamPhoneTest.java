/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.grandstream;

import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IArgumentMatcher;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.sip.SipService;

public class GrandstreamPhoneTest extends TestCase {

    GrandstreamPhone phone;

    PhoneTestDriver tester;

    protected void setUp() {
        GrandstreamModel model = new GrandstreamModel();
        model.setMaxLineCount(1);
        model.setModelId("gsPhoneBt100");
        model.setModelDir("grandstream");
        phone = new GrandstreamPhone();
        phone.setModel(model);
        tester = PhoneTestDriver.supplyTestData(phone);
    }

    public void testGenerateBinaryProfiles() throws Exception {
        ByteArrayOutputStream actualBinary = new ByteArrayOutputStream();
        GrandstreamBinaryProfileWriter wtr = new GrandstreamBinaryProfileWriter(phone);
        ByteArrayOutputStream inmemory = new ByteArrayOutputStream();
        inmemory.write("P1=A&".getBytes());
        wtr.writeBody(inmemory, actualBinary);
        byte[] actual = actualBinary.toByteArray();

        InputStream stream = new BufferedInputStream(getClass().getResourceAsStream("simple.bin"));
        byte[] expected = new byte[actual.length];
        int len = stream.read(expected);
        assertEquals(len, actual.length);
        // no more characters to read
        assertEquals(-1, stream.read());

        // debug aid
        // HexDump.dump(actual, 0, System.out, 0);

        for (int i = 0; i < actual.length; i++) {
            assertEquals("Different byte: " + i, expected[i], actual[i]);
        }
    }

    static byte[] resetMatcher() {
        EasyMock.reportMatcher(new ResetArgumentMatcher());
        return null;
    }

    public void testReset() throws Exception {
        tester.sipControl = EasyMock.createStrictControl();
        tester.sip = tester.sipControl.createMock(SipService.class);

        tester.sip.sendNotify(EasyMock.eq("sip:juser@sipfoundry.org"), EasyMock.eq("sys-control"), EasyMock
                .eq("application/octet-stream"), resetMatcher());
        tester.sipControl.replay();

        phone.setSipService(tester.sip);
        phone.restart();
    }

    static class ResetArgumentMatcher implements IArgumentMatcher {
        public boolean matches(Object argument) {
            byte[] payload = (byte[]) argument;
            return payload.length > 0;
        }

        public void appendTo(StringBuffer buffer) {
            buffer.append("byte array payload must not be empty");
        }
    }
}
