/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common;

import java.util.ArrayList;
import java.util.List;

import junit.framework.TestCase;

public class CoreContextImplTest extends TestCase {
    private static final int NUM_USERS = 10;

    public void testMaxUsers() throws Exception {
        CoreContextImpl m_core = new CoreContextImpl() {
            public int getBeansInGroupCount(Class beanClass, Integer groupId) {
                return NUM_USERS;
            }

            public User newUser() {
                return null;
            }

            public InternalUser newInternalUser() {
                return null;
            }
        };
        try {
            m_core.checkMaxUsers(new User(), -1);
        } catch (UserException e) {
            fail();
        }

        try {
            m_core.checkMaxUsers(new User(), NUM_USERS + 1);
        } catch (UserException e) {
            fail();
        }

        try {
            m_core.checkMaxUsers(new User(), NUM_USERS);
            fail();
        } catch (UserException e) {
            assertTrue(true);
        }

        try {
            m_core.checkMaxUsers(new ExistingUser(), NUM_USERS);
        } catch (UserException e) {
            fail();
        }
    }

    public void testCheckForDuplicateString() {
        CoreContextImpl core = new CoreContextImpl() {
            public User newUser() {
                return null;
            }

            public InternalUser newInternalUser() {
                return null;
            }
        };
        // An empty collection should have no duplicates
        List strings = new ArrayList();
        assertNull(core.checkForDuplicateString(strings));

        // Still no duplicates
        strings.add(new String("a"));
        strings.add(new String("b"));
        strings.add(new String("c"));
        assertNull(core.checkForDuplicateString(strings));

        // Now we have a duplicate
        strings.add(new String("b"));
        assertEquals("b", core.checkForDuplicateString(strings));

        // Try multiple duplicates (result is indeterminate but non-null)
        strings.add(new String("c"));
        assertNotNull(core.checkForDuplicateString(strings));
    }

    private static class ExistingUser extends User {
        public boolean isNew() {
            return false;
        }
    }
}
