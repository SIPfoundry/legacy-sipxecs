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

public class IpAddrSettingTest extends TestCase {
    public void testPattern() {
        StringSetting setting = new IpAddrSetting();
        Pattern pattern = Pattern.compile(setting.getPattern());
        assertFalse(pattern.matcher("abcd").matches());
        assertTrue(pattern.matcher("10.1.1.1").matches());
        assertTrue(pattern.matcher("255.255.128.0").matches());
        assertFalse(pattern.matcher("255x255.128.0").matches());
        assertFalse(pattern.matcher("255.1255.128.0").matches());
    }
}
