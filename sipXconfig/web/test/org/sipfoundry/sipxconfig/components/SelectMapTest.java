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

import java.util.Collection;

import junit.framework.TestCase;

/**
 * SelectMapTest
 */
public class SelectMapTest extends TestCase {

    public void testGetSelectedObject() {
        SelectMap map = new SelectMap();
        Collection selections = map.getAllSelected();
        assertEquals(0, selections.size());
        assertFalse(map.getSelected("kuku"));
        map.setSelected("kuku", true);
        map.setSelected("bongo", true);
        assertTrue(map.getSelected("kuku"));
        assertTrue(map.getSelected("bongo"));
        assertFalse(map.getSelected("xyz"));
        selections = map.getAllSelected();
        assertEquals(2, selections.size());
        assertTrue(selections.contains("kuku"));
        assertTrue(selections.contains("bongo"));
        map.setSelected("bongo", false);
        map.setSelected("xyz", false);
        assertTrue(map.getSelected("kuku"));
        assertFalse(map.getSelected("bongo"));
        assertFalse(map.getSelected("xyz"));
        selections = map.getAllSelected();
        assertEquals(1, selections.size());
        assertTrue(selections.contains("kuku"));
        assertFalse(selections.contains("bongo"));
    }
}
