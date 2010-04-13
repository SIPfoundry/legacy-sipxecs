/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.conference;

import junit.framework.TestCase;

public class FreeswitchApiMarshallerTest extends TestCase {
    public void testMarshallerNoParams() throws Exception {
        FreeswitchApiMarshaller m = new FreeswitchApiMarshaller();
        Object[] parameters = m.parameters("reloadXml");
        assertEquals(2, parameters.length);
        assertEquals("reloadxml", parameters[0]);
        assertEquals("", parameters[1]);
    }

    public void testMarshaller() throws Exception {
        FreeswitchApiMarshaller m = new FreeswitchApiMarshaller();
        Object[] parameters = m.parameters("someOtherMethod", "param1", 2);
        assertEquals(3, parameters.length);
        assertEquals("someothermethod", parameters[0]);
        assertEquals("param1", parameters[1]);
        assertEquals(2, parameters[2]);
    }

}
