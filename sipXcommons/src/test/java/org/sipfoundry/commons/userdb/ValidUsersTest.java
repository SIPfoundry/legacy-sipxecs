/*
 *
 *
 * Copyright (C) 2008-2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.commons.userdb;

import java.util.Properties;

import junit.framework.TestCase;

public class ValidUsersTest extends TestCase {

    public void testCompress() {
        assertEquals("WOOF", ValidUsers.compress("Woof!"));
        assertEquals("WOOFWASHERE", ValidUsers.compress("Woof! was here."));
        assertEquals("SPYDERTROBINSONRD", ValidUsers.compress("Spyder T. Robinson 3rd"));
        assertEquals("FRAUBRUCHER", ValidUsers.compress("Fräu BrÜcher"));
    }

    public void testMapDTMF() {
        assertEquals("9663", ValidUsers.mapDTMF("WOOF"));
        assertEquals("22233344455566677778889999", ValidUsers
                .mapDTMF("ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
        assertEquals("", ValidUsers.mapDTMF(null));
        assertEquals("22333", ValidUsers.mapDTMF("A B-D.E?F"));
    }

    public void testGetDisplayName() {
        assertNull(ValidUsers.getDisplayPart(null));
        assertNull(ValidUsers.getDisplayPart("woof"));
        assertEquals("", ValidUsers.getDisplayPart("sip:woof@somewhere"));
        assertEquals("woof", ValidUsers.getDisplayPart("woof <sip:woof@somewhere>"));
        assertEquals("Quebit T. Dogg", ValidUsers
                .getDisplayPart("\"Quebit T. Dogg\" <sip:woof@somewhere>"));
    }

    public void testGetUserPart() {
        assertNull(ValidUsers.getUserPart(null));
        assertNull(ValidUsers.getUserPart("woof"));
        assertEquals("woof", ValidUsers.getUserPart("sip:woof@somewhere"));
        assertEquals("woof", ValidUsers.getUserPart("Woof <sip:woof@somewhere>"));
        assertEquals("woof", ValidUsers.getUserPart("\"Me and I\" <woof@somewhere>"));
        assertEquals("woof", ValidUsers
                .getUserPart("\"Quebit T. Dogg\" <sip:woof@somewhere>"));

    }

    public void testBuildDialPatterns() {
        User u = new User();

        u.setDisplayName(null);
        ValidUsers.buildDialPatterns(u);
        assertEquals(0, u.getDialPatterns().size());

        u.setDisplayName("");
        ValidUsers.buildDialPatterns(u);
        assertEquals(0, u.getDialPatterns().size());

        u.setDisplayName("!@#$%^&*");
        ValidUsers.buildDialPatterns(u);
        assertEquals(0, u.getDialPatterns().size());

        u.setDisplayName("woof");
        ValidUsers.buildDialPatterns(u);
        assertEquals(1, u.getDialPatterns().size());
        assertEquals("9663", u.getDialPatterns().firstElement());

        u.setDisplayName("Quebit T. Dogg");
        ValidUsers.buildDialPatterns(u);
        assertEquals(3, u.getDialPatterns().size());
        assertEquals("78324883644", u.getDialPatterns().get(0));
        assertEquals("83644783248", u.getDialPatterns().get(1));
        assertEquals("36447832488", u.getDialPatterns().get(2));

    }

    public void testPin() {
        // Configure log4j
        Properties props = new Properties();
        props.setProperty("log4j.rootLogger", "debug, cons");
        props.setProperty("log4j.appender.cons", "org.apache.log4j.ConsoleAppender");
        props.setProperty("log4j.appender.cons.layout", "org.sipfoundry.commons.log4j.SipFoundryLayout");
        props.setProperty("log4j.appender.cons.layout.facility", "sipXivr");

        User u = new User();
        u.setIdentity("201@woof3.fuzzy");
        u.setUserName("201");
        u.setVoicemailPintoken("3c46585d782a8b9444feca1c697e4d4f");

        assertTrue(u.isVoicemailPinCorrect("9663", "woof3.fuzzy"));
    }
}
