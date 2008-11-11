/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import junit.framework.TestCase;

public class AutoAttendantTest extends TestCase {

    public void testGetSystemName() {
        AutoAttendant aa = new AutoAttendant();
        assertEquals("xcf-1", aa.getSystemName());
        assertFalse(aa.isAfterhour());
        assertFalse(aa.isOperator());
        assertFalse(aa.isPermanent());

        AutoAttendant operator = new AutoAttendant();
        operator.setSystemId(AutoAttendant.OPERATOR_ID);
        assertEquals("operator", operator.getSystemName());
        assertFalse(operator.isAfterhour());
        assertTrue(operator.isOperator());
        assertTrue(operator.isPermanent());

        AutoAttendant afterhour = new AutoAttendant();
        afterhour.setSystemId(AutoAttendant.AFTERHOUR_ID);
        assertEquals("afterhour", afterhour.getSystemName());
        assertTrue(afterhour.isAfterhour());
        assertFalse(afterhour.isOperator());
        assertTrue(afterhour.isPermanent());
    }
}
