/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.setting.type;

import java.util.regex.Pattern;

import junit.framework.TestCase;

public class HostnameSettingTest extends TestCase {
    public void testPattern() {
        StringSetting setting = new HostnameSetting();
        Pattern pattern = Pattern.compile(setting.getPattern());
        assertFalse(pattern.matcher("abcd").matches());
        assertFalse(pattern.matcher("abcd.c").matches());

        assertTrue(pattern.matcher("localhost").matches());
        assertTrue(pattern.matcher("abcd.co").matches());
        assertTrue(pattern.matcher("abcd.com").matches());
        assertTrue(pattern.matcher("host.example.com").matches());

        assertTrue(pattern.matcher("10.1.1.1").matches());
        assertTrue(pattern.matcher("255.255.128.0").matches());
        assertFalse(pattern.matcher("255x255.128.0").matches());
        assertFalse(pattern.matcher("255.1255.128.0").matches());
    }
}
