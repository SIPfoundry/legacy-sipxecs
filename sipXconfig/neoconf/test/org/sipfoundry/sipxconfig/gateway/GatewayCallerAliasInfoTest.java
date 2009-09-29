/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.gateway;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.common.User;

public class GatewayCallerAliasInfoTest extends TestCase {

    protected void setUp() throws Exception {
        super.setUp();
    }

    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void testGetTransformedNumber() {
        GatewayCallerAliasInfo info = new GatewayCallerAliasInfo();
        User user = new User();
        user.setUserName("abc");
        info.setTransformUserExtension(false);
        assertNull(info.getTransformedNumber(user));

        info.setTransformUserExtension(true);
        assertNull(info.getTransformedNumber(user));

        user.setUserName("123456789");
        assertEquals("123456789", info.getTransformedNumber(user));

        info.setKeepDigits(4);
        assertEquals("6789", info.getTransformedNumber(user));

        info.setKeepDigits(20);
        info.setAddPrefix("777");
        assertEquals("777123456789", info.getTransformedNumber(user));

        info.setKeepDigits(2);
        info.setAddPrefix("777");
        assertEquals("77789", info.getTransformedNumber(user));

        info.setKeepDigits(-1);
        info.setAddPrefix("2");
        assertEquals("2123456789", info.getTransformedNumber(user));
    }
}
