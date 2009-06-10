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

    public void testEquals() {
        Button button1 = new Button();
        button1.setNumber("abc@sipfoundry.org");
        button1.setLabel("ABC");

        Button button2 = new Button();
        button2.setNumber("abc@sipfoundry.org");
        button2.setLabel("ABC");

        Button button3 = new Button();
        button3.setNumber("abc@sipfoundry.org");
        button3.setLabel("XXX");

        assertEquals(button1, button2);
        assertEquals(button1, button1);

        assertFalse(button1.equals(button3));
    }
}
