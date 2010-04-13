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
import java.util.Collection;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

import junit.framework.TestCase;

public class BeanIdTest extends TestCase {
    static class Dummy extends BeanWithId {
        public Dummy(Integer id) {
            setId(id);
        }
    }
    static class Dummy2 extends BeanWithId {
        public Dummy2(Integer id) {
            setId(id);
        }
    }

    private Collection m_ids = new ArrayList(2);
    private Integer m_id1 = new Integer(1);
    private Integer m_id2 = new Integer(2);
    BeanId m_bid1Dummy = new BeanId(m_id1, Dummy.class);
    BeanId m_bid1OtherClass = new BeanId(m_id1, Integer.class);
    BeanId m_bid2Dummy = new BeanId(m_id2, Dummy.class);
    BeanId m_bid1DummyAgain = new BeanId(m_id1, Dummy.class);

    protected void setUp() throws Exception {
        m_ids.add(m_id1);
        m_ids.add(m_id2);
    }

    public void testCreateBeanIdCollection() {
        Collection bids = BeanId.createBeanIdCollection(m_ids, Dummy.class);
        assertTrue(bids.size() == 2);
        Iterator it = bids.iterator();
        BeanId bid1 = (BeanId) it.next();
        assertTrue(bid1.getId().equals(m_id1));
        assertTrue(bid1.getBeanClass() == Dummy.class);
        BeanId bid2 = (BeanId) it.next();
        assertTrue(bid2.getId().equals(m_id2));
        assertTrue(bid2.getBeanClass() == Dummy.class);
    }

    public void testCreateBeanIdCollectionExceptions() {
        // test null ID
        try {
            m_ids.add(null);
            BeanId.createBeanIdCollection(m_ids, Dummy.class);
            fail("Should throw exception because of null ID value");
        }
        catch (IllegalArgumentException e) {
            // expected
        }

        // test negative ID
        try {
            m_ids.clear();
            m_ids.add(new Integer(-1));
            BeanId.createBeanIdCollection(m_ids, Dummy.class);
            fail("Should throw exception because of negative ID value");
        }
        catch (IllegalArgumentException e) {
            // expected
        }

        // test non-unique ID
        try {
            m_ids.clear();
            m_ids.add(m_id1);
            m_ids.add(m_id1);
            BeanId.createBeanIdCollection(m_ids, Dummy.class);
            fail("Should throw exception because of non-unique ID value");
        }
        catch (IllegalArgumentException e) {
            // expected
        }
    }

    public void testEqualsAndHashCode() {
        assertFalse(m_bid1Dummy.equals(m_bid1OtherClass));
        assertFalse(m_bid1Dummy.equals(m_bid2Dummy));
        assertTrue(m_bid1Dummy.equals(m_bid1DummyAgain));
        assertFalse(m_bid1Dummy.equals(null));
        assertFalse(m_bid1Dummy.equals(new Object()));

        // Add these four objects to a Set, should end up with three objects in the Set
        // because one of them is a dup according to the hashCode.
        Set set = new HashSet();
        set.add(m_bid1Dummy);
        set.add(m_bid1OtherClass);
        set.add(m_bid2Dummy);
        set.add(m_bid1DummyAgain);
        assertEquals(3, set.size());
    }

    public void testIsIdOfBean() {
        BeanWithId bean1 = new Dummy(m_id1);
        BeanWithId bean2 = new Dummy(m_id2);
        BeanWithId bean3 = new Dummy2(m_id1);
        BeanId bid1 = new BeanId(bean1);

        assertTrue(bid1.isIdOfBean(bean1));
        assertFalse(bid1.isIdOfBean(bean2));
        assertFalse(bid1.isIdOfBean(bean3));
    }

}
