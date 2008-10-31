/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxivr;

import org.sipfoundry.sipxivr.User;
import org.sipfoundry.sipxivr.ValidUsersXML;

import junit.framework.TestCase;

public class ValidUsersXMLTest extends TestCase {

    /*
     * public void testUpdate() { fail("Not yet implemented"); }
     * 
     * public void testImdb() { fail("Not yet implemented"); }
     * 
     * public void testLookupDTMF() { fail("Not yet implemented"); }
     * 
     * public void testIsValidUser() { fail("Not yet implemented"); }
     * 
     */
    public void testCompress() {
        assertEquals("WOOF", ValidUsersXML.compress("Woof!")); 
        assertEquals("WOOFWASHERE", ValidUsersXML.compress("Woof! was here."));
        assertEquals("SPYDERTROBINSONRD", ValidUsersXML.compress("Spyder T. Robinson 3rd"));
        assertEquals("FRAUBRUCHER", ValidUsersXML.compress("Fräu BrÜcher"));
    }

    public void testMapDTMF() {
        assertEquals("9663", ValidUsersXML.mapDTMF("WOOF"));
        assertEquals("22233344455566677778889999", ValidUsersXML
                .mapDTMF("ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
        assertEquals("", ValidUsersXML.mapDTMF(null));
        assertEquals("22333", ValidUsersXML.mapDTMF("A B-D.E?F"));
    }

    public void testGetDisplayName() {
        assertNull(ValidUsersXML.getDisplayName(null));
        assertNull(ValidUsersXML.getDisplayName("woof"));
        assertEquals("", ValidUsersXML.getDisplayName("sip:woof@somewhere"));
        assertEquals("woof", ValidUsersXML.getDisplayName("woof <sip:woof@somewhere>"));
        assertEquals("Quebit T. Dogg", ValidUsersXML
                .getDisplayName("\"Quebit T. Dogg\" <sip:woof@somewhere>"));
    }

    public void testBuildDialPatterns() {
        User u = new User();

        u.setDisplayName(null);
        ValidUsersXML.buildDialPatterns(u);
        assertEquals(0, u.getDialPatterns().size());

        u.setDisplayName("");
        ValidUsersXML.buildDialPatterns(u);
        assertEquals(0, u.getDialPatterns().size());

        u.setDisplayName("!@#$%^&*");
        ValidUsersXML.buildDialPatterns(u);
        assertEquals(0, u.getDialPatterns().size());

        u.setDisplayName("woof");
        ValidUsersXML.buildDialPatterns(u);
        assertEquals(1, u.getDialPatterns().size());
        assertEquals("9663", u.getDialPatterns().firstElement());

        u.setDisplayName("Quebit T. Dogg");
        ValidUsersXML.buildDialPatterns(u);
        assertEquals(3, u.getDialPatterns().size());
        assertEquals("78324883644", u.getDialPatterns().get(0));
        assertEquals("83644783248", u.getDialPatterns().get(1));
        assertEquals("36447832488", u.getDialPatterns().get(2));

    }

}
