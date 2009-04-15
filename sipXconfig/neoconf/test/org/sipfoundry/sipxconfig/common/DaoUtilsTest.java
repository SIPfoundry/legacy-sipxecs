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
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import junit.framework.TestCase;

public class DaoUtilsTest extends TestCase {
    
    BeanWithId subject = new BeanWithId();
        
    List objs = new ArrayList();

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
            Collection c = Arrays.asList(new String[] {"one", "two"});
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
}
