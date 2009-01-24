/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */

package org.sipfoundry.sipxconfig.admin.commserver;

import junit.framework.JUnit4TestAdapter;

import org.junit.Assert;
import org.junit.Test;

public class ServiceStatusMessageTest {

    public static junit.framework.Test suite() {
        return new JUnit4TestAdapter(ServiceStatusMessageTest.class);
    }

    @Test
    public void testParseMessages() {
        testParse("stdout.msg-1: test", "stdout.msg", "test");
        testParse("stderr.msg-3: test2", "stderr.msg", "test2");
        testParse("version.mismatch", "test3: foo bar baz");
        testParse("resource.missing", "test4: foo bar baz");
    }

    private void testParse(String fullMessage, String prefix, String message) {
        ServiceStatusMessage statusMessage = new ServiceStatusMessage(fullMessage);
        Assert.assertEquals(prefix, statusMessage.getPrefix());
        Assert.assertEquals(message, statusMessage.getMessage());
    }

    private void testParse(String prefix, String message) {
        testParse(prefix + ": " + message, prefix, message);
    }

    @Test
    public void testParseInvalidMessages() {
        testParseInvalid("test1");
        testParseInvalid("stdout.msg-1 test2");
        testParseInvalid("stdout-msg-2; test3");
    }

    public void testParseInvalid(String message) {
        try {
            new ServiceStatusMessage(message);
            Assert.fail("Did not fail on invalid input");
        } catch (IllegalArgumentException iae) {
            Assert.assertNotNull(iae.getMessage());
        }
    }
}
