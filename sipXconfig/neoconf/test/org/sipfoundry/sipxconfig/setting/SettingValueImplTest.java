/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import junit.framework.TestCase;

public class SettingValueImplTest extends TestCase {

    public void testNullEquals() {
        SettingValue v1 = new SettingValueImpl(null);
        SettingValue v2 = new SettingValueImpl(null);
        assertTrue(v1.equals(v2));
    }

}
