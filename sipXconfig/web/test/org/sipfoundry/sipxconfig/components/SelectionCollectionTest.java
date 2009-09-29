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
import java.util.Arrays;
import java.util.Collection;

import junit.framework.TestCase;

public class SelectionCollectionTest extends TestCase {

    SelectionCollection selections;

    Collection collection;

    String[] items = { "one", "two", "three" };

    protected void setUp() {
        selections = new SelectionCollection();
        collection = new ArrayList(Arrays.asList(items));
        selections.setCollection(collection);
    }

    public void testUnSelect() {
        assertTrue(collection.contains("one"));
        selections.setSelected("one", false);
        assertFalse(collection.contains("one"));
        assertEquals(2, collection.size());
    }

    public void testSelect() {
        assertFalse(collection.contains("four"));
        selections.setSelected("four", true);
        assertTrue(collection.contains("four"));
        assertEquals(4, collection.size());
    }
}
