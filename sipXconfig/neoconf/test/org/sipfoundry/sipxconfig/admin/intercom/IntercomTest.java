/*
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.intercom;

import java.util.Set;

import org.sipfoundry.sipxconfig.setting.Group;

import junit.framework.TestCase;

public class IntercomTest extends TestCase {
    public static final String TEST_PREFIX = "prefix";
    public static final int TEST_TIMEOUT = 123;
    public static final String TEST_CODE = "code";

    public void testPrefix() {
        Intercom intercom = new Intercom();
        intercom.setPrefix(TEST_PREFIX);
        assertEquals(TEST_PREFIX, intercom.getPrefix());
    }

    public void testTimeout() {
        Intercom intercom = new Intercom();
        intercom.setTimeout(TEST_TIMEOUT);
        assertEquals(TEST_TIMEOUT, intercom.getTimeout());
    }

    public void testCode() {
        Intercom intercom = new Intercom();
        intercom.setCode(TEST_CODE);
        assertEquals(TEST_CODE, intercom.getCode());
    }

    public void testGroups() {
        Intercom intercom = new Intercom();
        Group group = new Group();
        intercom.addGroup(group);
        Set groups = intercom.getGroups();
        assert(groups.contains(group));
        assertEquals(1, groups.size());
    }
}
