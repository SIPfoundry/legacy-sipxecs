/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.speeddial;

import junit.framework.TestCase;

public class ButtonTest extends TestCase {

    public void testGetUri() {
        Button button = new Button();
        button.setNumber("abc@sipfoundry.org");
        assertEquals("sip:abc@sipfoundry.org", button.getUri("example.org"));
        button.setNumber("sip:abc@sipfoundry.org");
        assertEquals("sip:abc@sipfoundry.org", button.getUri("example.org"));
        button.setNumber("1234");
        assertEquals("sip:1234@example.org", button.getUri("example.org"));
    }
}
