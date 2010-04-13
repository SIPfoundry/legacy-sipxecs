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

import junit.framework.TestCase;

public class NewEnumPropertySelectionModelTest extends TestCase {

    private NewEnumPropertySelectionModel<Numbers> m_model;

    enum Numbers {
        ZERO, ONE, TWO, THREE;
    }

    protected void setUp() throws Exception {
        m_model = new NewEnumPropertySelectionModel<Numbers>();
        m_model.setEnumType(Numbers.class);
    }

    public void testGetOptionCount() {
        assertEquals(4, m_model.getOptionCount());
    }

    public void testGetOption() {
        assertEquals(Numbers.TWO, m_model.getOption(2));
    }

    public void testGetLabel() {
        assertEquals("THREE", m_model.getLabel(3));
    }

    public void testGetValue() {
        assertEquals("0", m_model.getValue(0));
    }

    public void testTranslateValue() {
        assertEquals(Numbers.TWO, m_model.translateValue("2"));
    }
}
