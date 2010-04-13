/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.setting.type;

import java.util.regex.Pattern;

import junit.framework.TestCase;

/**
 * This junit TestCase tests the PhonePadPinSetting type.
 */
public class PhonePadPinSettingTest extends TestCase {
    private static final String PHONE_PAD = "[\\d#*]+";

    public void testPattern() {

        StringSetting setting = new PhonePadPinSetting();

        // default pattern test
        String pat = setting.getPattern();

        assertTrue(pat.equals(PHONE_PAD));

        Pattern pattern = Pattern.compile(setting.getPattern());

        // single character: expect success
        assertTrue(pattern.matcher("1").matches());
        assertTrue(pattern.matcher("#").matches());
        assertTrue(pattern.matcher("*").matches());

        // single character: expect failures
        assertFalse(pattern.matcher("a").matches());
        assertFalse(pattern.matcher("%").matches());
        assertFalse(pattern.matcher("+").matches());

        // multiple characters: expect success
        assertTrue(pattern.matcher("1234567890*#").matches());
        assertTrue(pattern.matcher("123#").matches());
        assertTrue(pattern.matcher("1#*").matches());
        assertTrue(pattern.matcher("##").matches());

        // multiple characters: expect failures
        assertFalse(pattern.matcher("12#*b").matches());
        assertFalse(pattern.matcher("#*c").matches());
        assertFalse(pattern.matcher("1d").matches());
        assertFalse(pattern.matcher("3@").matches());

    }
}
