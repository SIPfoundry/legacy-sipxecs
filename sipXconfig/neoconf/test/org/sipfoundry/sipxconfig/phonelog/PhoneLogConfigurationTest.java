/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.phonelog;

import static org.junit.Assert.assertEquals;

import java.io.StringWriter;

import org.junit.Test;

public class PhoneLogConfigurationTest {

    @Test
    public void testWrite() throws Exception {
        PhoneLogConfiguration out = new PhoneLogConfiguration();
        StringWriter actual = new StringWriter();
        out.write(actual);
        assertEquals("PHONELOG_ENABLED : TRUE\n", actual.toString());
    }
}
