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
import java.util.Iterator;

import junit.framework.TestCase;

public class SipxCollectionUtilsTest extends TestCase {

    private Collection nonEmptyCollection;

    @Override
    protected void setUp() throws Exception {
        super.setUp();

        nonEmptyCollection = new ArrayList();
        nonEmptyCollection.add(new Object());
    }

    public void testSafeSize() {
        assertEquals(0, SipxCollectionUtils.safeSize(null));
        assertEquals(1, SipxCollectionUtils.safeSize(nonEmptyCollection));
    }

    public void testSafeIterator() {
        Iterator iter = SipxCollectionUtils.safeIterator(null);
        assertFalse(iter.hasNext());

        iter = SipxCollectionUtils.safeIterator(nonEmptyCollection);
        assertTrue(iter.hasNext());
    }

    public void testArrayToShortStrings() {
        Object[] arr = {
            1, "s", 22
        };
        assertEquals("[1, s, 22]", SipxCollectionUtils.arrayToShortString(arr, 4));

        arr[1] = "012345678901234567890";
        assertEquals("[1, 012345678901..., 22]", SipxCollectionUtils.arrayToShortString(arr, 15));

        arr[1] = new Object[] {
            "aaaaa", "bbbbb", "ccccc", "ddddd"
        };
        assertEquals("[1, [aaaaa, bb..., 22]", SipxCollectionUtils.arrayToShortString(arr, 13));
    }
}
