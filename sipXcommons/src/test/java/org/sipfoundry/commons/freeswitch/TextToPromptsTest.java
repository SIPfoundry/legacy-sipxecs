/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.commons.freeswitch;

import java.util.Locale;

import org.sipfoundry.commons.freeswitch.TextToPrompts;

import junit.framework.TestCase;

public class TextToPromptsTest extends TestCase {

    public void testGetTextToPrompt() {
        // Configure log4j
        // TODO: This can be re-enabled once logging support is added back to TextToPrompts class.
        // Properties props = new Properties();
        // props.setProperty("log4j.rootLogger", "debug, cons");
        // props.setProperty("log4j.appender.cons", "org.apache.log4j.ConsoleAppender");
        // props.setProperty("log4j.appender.cons.layout", "org.sipfoundry.commons.log4j.SipFoundryLayout");
        // props.setProperty("log4j.appender.cons.layout.facility", "freeswitch");

        // PropertyConfigurator.configure(props);

        // See if English works
        TextToPrompts ttp = TextToPrompts.getTextToPrompt(Locale.ENGLISH, null);
        assertEquals(TextToPrompts.class.getCanonicalName() + "_en", ttp.getClass().getCanonicalName());

        // Unknown Locales (such as Klingon) should revert to English. Sucks, but true.
        ttp = TextToPrompts.getTextToPrompt(new Locale("Klingon"), null);
        assertEquals(TextToPrompts.class.getCanonicalName() + "_en", ttp.getClass().getCanonicalName());
    }

}
