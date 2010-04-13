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
import java.util.Iterator;
import java.util.List;

import junit.framework.TestCase;
import org.apache.commons.lang.ArrayUtils;

public class DataCollectionUtilTest extends TestCase {

    private List<PrimaryKeySource> m_items;

    private Object[] m_primaryKeys;

    private int m_originalSize;

    @Override
    protected void setUp() {
        m_items = new ArrayList();
        for (int i = 0; i < 40; i++) {
            m_items.add(new TestCollectionItem(i));
        }

        // out of order on purpose
        m_primaryKeys = new Integer[] {
            new Integer(30), new Integer(10), new Integer(20)
        };

        m_originalSize = m_items.size();
    }

    public void testFindByPrimaryKey() {
        Collection<PrimaryKeySource> found = DataCollectionUtil.findByPrimaryKey(m_items, m_primaryKeys);
        assertEquals(m_primaryKeys.length, found.size());
        Iterator<PrimaryKeySource> ifound = found.iterator();
        Arrays.sort(m_primaryKeys);
        for (int i = 0; ifound.hasNext(); i++) {
            PrimaryKeySource item = ifound.next();
            assertTrue(Arrays.binarySearch(m_primaryKeys, item.getPrimaryKey()) >= 0);
        }
    }

    public void testRemoveByPrimaryKey() {
        // assumes findByPrimaryKey works
        Iterator find = DataCollectionUtil.findByPrimaryKey(m_items, m_primaryKeys).iterator();

        DataCollectionUtil.removeByPrimaryKey(m_items, m_primaryKeys);
        while (find.hasNext()) {
            assertFalse(m_items.contains(find.next()));
        }
        assertEquals(m_originalSize - m_primaryKeys.length, m_items.size());
    }

    public void testMoveUpByPrimaryKey() {
        DataCollectionUtil.moveByPrimaryKey(m_items, m_primaryKeys, -1);
        PrimaryKeySource[] items = m_items.toArray(new PrimaryKeySource[0]);
        assertEquals(new Integer(10), items[9].getPrimaryKey());
        assertEquals(new Integer(9), items[10].getPrimaryKey());
        assertEquals(new Integer(20), items[19].getPrimaryKey());
        assertEquals(new Integer(19), items[20].getPrimaryKey());
        assertEquals(new Integer(30), items[29].getPrimaryKey());
        assertEquals(new Integer(29), items[30].getPrimaryKey());
    }

    public void testMoveDownByPrimaryKey() {
        DataCollectionUtil.moveByPrimaryKey(m_items, m_primaryKeys, 1);
        PrimaryKeySource[] items = m_items.toArray(new PrimaryKeySource[0]);
        assertEquals(new Integer(10), items[11].getPrimaryKey());
        assertEquals(new Integer(11), items[10].getPrimaryKey());
        assertEquals(new Integer(20), items[21].getPrimaryKey());
        assertEquals(new Integer(21), items[20].getPrimaryKey());
        assertEquals(new Integer(30), items[31].getPrimaryKey());
        assertEquals(new Integer(31), items[30].getPrimaryKey());
    }

    public void testExtractPrimaryKeys() {
        Collection c = DataCollectionUtil.extractPrimaryKeys(m_items);
        Integer[] pks = (Integer[]) c.toArray(new Integer[0]);
        Arrays.equals(m_primaryKeys, pks);
    }

    public void testMoveUpByPrimaryKeySqueezeHoles() {
        int keys[] = {
            0, 1, 3
        };
        DataCollectionUtil.moveByPrimaryKey(m_items, ArrayUtils.toObject(keys), -1);
        PrimaryKeySource[] items = m_items.toArray(new PrimaryKeySource[0]);
        assertEquals(new Integer(0), items[0].getPrimaryKey());
        assertEquals(new Integer(1), items[1].getPrimaryKey());
        assertEquals(new Integer(3), items[2].getPrimaryKey());
    }

    public void testMoveDownByPrimaryKeySqueezeHoles() {
        Integer keys[] = {
            35, 38, 39
        };
        DataCollectionUtil.moveByPrimaryKey(m_items, keys, 10);
        PrimaryKeySource[] items = m_items.toArray(new PrimaryKeySource[0]);
        assertEquals(new Integer(35), items[37].getPrimaryKey());
        assertEquals(new Integer(38), items[38].getPrimaryKey());
        assertEquals(new Integer(39), items[39].getPrimaryKey());
    }

    public void testDuplicate() {
        int size = 5;
        Collection from = new ArrayList();
        for (int i = 0; i < size; i++) {
            BeanWithId bean = new BeanWithId();
            bean.setUniqueId();
            from.add(bean);
        }
        Collection to = new ArrayList();
        DataCollectionUtil.duplicate(from, to);
        assertEquals(size, to.size());
        for (Iterator i = to.iterator(); i.hasNext();) {
            BeanWithId clonedBean = (BeanWithId) i.next();
            assertTrue(clonedBean.isNew());
        }
        // check that we cannot do it again
        try {
            DataCollectionUtil.duplicate(from, to);
            fail("should throw illegal argument exception");
        } catch (IllegalArgumentException e) {
            // OK
        }
    }

    public void testMove() throws Exception {
        List<Character> list = new ArrayList<Character>();
        for (char c = 'a'; c < 'd'; c++) {
            list.add(c);
        }

        assertEquals(0, DataCollectionUtil.move(list, 0, -1));
        assertEquals(0, DataCollectionUtil.move(list, 0, -2));
        assertEquals(0, DataCollectionUtil.move(list, 1, -1));
        assertEquals(new Character('b'), list.get(0));
        assertEquals(new Character('a'), list.get(1));
        assertEquals(2, DataCollectionUtil.move(list, 0, 3));
        assertEquals(new Character('b'), list.get(2));
        assertEquals(2, DataCollectionUtil.move(list, 0, 2));
        assertEquals(new Character('a'), list.get(2));
        assertEquals(new Character('b'), list.get(1));
        assertEquals(new Character('c'), list.get(0));
    }

    static class TestCollectionItem implements PrimaryKeySource {

        private final Integer m_id;

        TestCollectionItem(int id) {
            m_id = new Integer(id);
        }

        public Object getPrimaryKey() {
            return m_id;
        }
    }
}
