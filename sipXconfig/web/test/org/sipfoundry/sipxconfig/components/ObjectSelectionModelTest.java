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

import java.util.ArrayList;
import java.util.List;

import junit.framework.TestCase;

public class ObjectSelectionModelTest extends TestCase {

    List m_objects;

    protected void setUp() {
        m_objects = new ArrayList();
        m_objects.add(new ExampleBusinessObject("voltar", new Integer(100)));
        m_objects.add(new ExampleBusinessObject("kuku", new Integer(101)));
        m_objects.add(new ExampleBusinessObject("bongo", new Integer(102)));
    }

    public void testSetArray() {
        ObjectSelectionModel model = new ObjectSelectionModel();
        model.setArray(m_objects.toArray());
        assertSame(m_objects.get(0), model.getOption(0));
    }

    public void testGetLabel() {
        ObjectSelectionModel model = new ObjectSelectionModel();
        model.setCollection(m_objects);
        model.setLabelExpression("something");

        assertEquals("voltar", model.getLabel(0));
        assertEquals("kuku", model.getLabel(1));
        assertEquals("bongo", model.getLabel(2));
    }

    public void testGetOption() {
        ObjectSelectionModel model = new ObjectSelectionModel();
        model.setCollection(m_objects);
        model.setValueExpression("somethingElse");

        assertEquals(new Integer(100), model.getOption(0));
        assertEquals(new Integer(101), model.getOption(1));
        assertEquals(new Integer(102), model.getOption(2));
    }

    public void testNullLabel() {
        ObjectSelectionModel model = new ObjectSelectionModel();
        model.setCollection(m_objects);
        assertEquals("", model.getLabel(0));
    }

    public void testInvalidLabel() {
        ObjectSelectionModel model = new ObjectSelectionModel();
        model.setCollection(m_objects);
        model.setLabelExpression("bogus");
        try {
            model.getLabel(0);
            fail();
        } catch (RuntimeException e) {
            assertTrue(true);
        }
    }

    static class ExampleBusinessObject {

        String m_something;

        Integer m_somethingElse;

        ExampleBusinessObject(String something, Integer somethingElse) {
            m_something = something;
            m_somethingElse = somethingElse;
        }

        public String getSomething() {
            return m_something;
        }

        public Integer getSomethingElse() {
            return m_somethingElse;
        }
    }
}
