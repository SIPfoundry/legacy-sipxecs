/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import java.util.Arrays;
import java.util.List;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.setting.Group;

public class TapestryContextTest extends TestCase {
    public void testJoinNamed() {
        Group[] groups = new Group[] { new Group(), new Group()};
        groups[0].setName("robin");
        groups[1].setName("crow");
        List asList = Arrays.asList(groups);
        assertEquals("robin, crow", new TapestryContext().joinNamed(asList, ", "));
    }
}
