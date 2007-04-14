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
}
