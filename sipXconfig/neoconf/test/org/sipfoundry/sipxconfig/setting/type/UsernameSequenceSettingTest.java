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
 * This junit TestCase tests the UsernameSequenceSetting type.
 */
public class UsernameSequenceSettingTest extends TestCase {
    //Will call this ALIAS_SEQUENCE since used for aliases
    private static final String ALIAS_SEQUENCE = 
          "((([-_.!~*'\\(\\)&amp;=+$,;?/;a-zA-Z0-9]|(&#37;[0-9a-fA-F]{2});)+)\\s*)*";

    public void testPattern() {

        StringSetting setting = new UsernameSequenceSetting();

        // default pattern test
        String pat = setting.getPattern();

        assertTrue(pat.equals(ALIAS_SEQUENCE));

        Pattern pattern = Pattern.compile(setting.getPattern());

        // single character: expect success
        assertTrue(pattern.matcher("1").matches());
        assertTrue(pattern.matcher("a").matches());
        assertTrue(pattern.matcher("Z").matches());

        // single character: expect failures
        assertFalse(pattern.matcher("#").matches());
        assertFalse(pattern.matcher(">").matches());
        assertFalse(pattern.matcher("<").matches());
        assertFalse(pattern.matcher("@").matches());

        // multiple characters: expect success
        assertTrue(pattern.matcher("123").matches());
        assertTrue(pattern.matcher("1*").matches());
        assertTrue(pattern.matcher("joe 153 *81").matches());

        // multiple characters: expect failures
        assertFalse(pattern.matcher("##").matches());
        assertFalse(pattern.matcher("#*c").matches());
        assertFalse(pattern.matcher("@@").matches());

    }
}
