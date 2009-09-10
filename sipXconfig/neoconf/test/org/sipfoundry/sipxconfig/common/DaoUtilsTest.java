/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.common;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import junit.framework.TestCase;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class DaoUtilsTest extends TestCase {

    BeanWithId subject = new BeanWithId();

    List objs = new ArrayList();

    @Override
    protected void setUp() {
        subject.setUniqueId();
    }

    public void testCheckDuplicatesFoundItself() {
        objs.add(new Integer(subject.getId().intValue()));
        DaoUtils.checkDuplicates(subject, objs, new UserException());
    }

    public void testCheckDuplicatesFoundItselfWithoutException() {
        objs.add(new Integer(subject.getId().intValue()));
        assertFalse(DaoUtils.checkDuplicates(subject, objs, null));
    }

    public void testCheckDuplicatesFoundDuplicate() {
        objs.add(new Integer(subject.getId().intValue() + 1));
        try {
            DaoUtils.checkDuplicates(subject, objs, new UserException());
            fail();
        } catch (UserException expected) {
            assertTrue(true);
        }
    }

    public void testRequireOneOrZero() {
        String query = "my query";
        DaoUtils.requireOneOrZero(Collections.EMPTY_LIST, query);
        DaoUtils.requireOneOrZero(Collections.singleton(new Object()), query);
        try {
            Collection c = Arrays.asList(new String[] {
                "one", "two"
            });
            DaoUtils.requireOneOrZero(c, query);
            fail();
        } catch (IllegalStateException expected) {
            assertTrue(expected.getMessage().indexOf(query) > 0);
        }
    }

    public void testCheckDuplicatesFoundDuplicateWithoutException() {
        objs.add(new Integer(subject.getId().intValue() + 1));
        assertTrue(DaoUtils.checkDuplicates(subject, objs, null));
    }

    public void testForAllUsersDoEmpty() {
        CoreContext coreContext = createMock(CoreContext.class);
        coreContext.loadUsersByPage(null, null, null, 0, DaoUtils.PAGE_SIZE, "id", true);
        expectLastCall().andReturn(Collections.EMPTY_LIST);
        replay(coreContext);

        Closure<User> closure = new Closure<User>() {
            @Override
            public void execute(User item) {
                fail("Should not be called for an empty list");
            }
        };
        DaoUtils.forAllUsersDo(coreContext, closure);
        verify(coreContext);
    }

    public void testForAllUsersDo() {
        User[] firstPage = new User[DaoUtils.PAGE_SIZE];
        User[] secondPage = new User[DaoUtils.PAGE_SIZE - 1];

        CoreContext coreContext = createMock(CoreContext.class);
        coreContext.loadUsersByPage(null, null, null, 0, DaoUtils.PAGE_SIZE, "id", true);
        expectLastCall().andReturn(Arrays.asList(firstPage)).andReturn(Arrays.asList(secondPage));
        replay(coreContext);

        CountingClosure closure = new CountingClosure();
        DaoUtils.forAllUsersDo(coreContext, closure);
        assertEquals(firstPage.length + secondPage.length, closure.getCount());
        verify(coreContext);
    }

    private static class CountingClosure implements Closure<User> {
        private int m_count = 0;

        public int getCount() {
            return m_count;
        }

        @Override
        public void execute(User item) {
            assertNull(item);
            m_count++;
        }
    }
}
