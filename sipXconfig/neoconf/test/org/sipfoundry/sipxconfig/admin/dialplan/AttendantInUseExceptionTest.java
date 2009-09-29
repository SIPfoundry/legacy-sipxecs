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

import java.util.ArrayList;
import java.util.List;

import junit.framework.TestCase;

public class AttendantInUseExceptionTest extends TestCase {

    /*
     * Class under test for void AttendantInUseException()
     */
    public void testAttendantInUseException() {
        Exception exception = new AttendantInUseException();
        assertEquals(AttendantInUseException.OPERATOR_DELETE, exception.getMessage());
    }

    /*
     * Class under test for void AttendantInUseException(Collection)
     */
    public void testAttendantInUseExceptionRules() {
        List rules = new ArrayList();
        DialingRule rule1 = new CustomDialingRule();
        rule1.setName("abc");
        rules.add(rule1);
        DialingRule rule2 = new CustomDialingRule();
        rule2.setName("bc");
        rules.add(rule2);
        DialingRule rule3 = new CustomDialingRule();
        rule3.setName("def");
        rules.add(rule3);
        Exception exception = new AttendantInUseException(rules);
        assertTrue(exception.getMessage().indexOf("abc, bc, def") > 0);
        assertFalse(exception.getMessage().indexOf("abc, bc, def,") > 0);
    }

}
