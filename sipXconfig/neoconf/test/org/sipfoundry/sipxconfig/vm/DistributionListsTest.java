/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.vm;

import java.util.Collection;

import junit.framework.TestCase;

import org.apache.commons.lang.StringUtils;

public class DistributionListsTest extends TestCase {

    protected void setUp() {
    }

    public void testGetUniqueExtensions() throws Exception {
        DistributionList[] dls = DistributionList.createBlankList();
        assertEquals(0, DistributionList.getUniqueExtensions(dls).size());

        DistributionList dl1 = new DistributionList();
        dl1.setExtensions(StringUtils.split("123 234 345 456"));
        Collection<String> uniqueExtensions = DistributionList.getUniqueExtensions(dl1);
        assertEquals(4, uniqueExtensions.size());
        assertTrue(uniqueExtensions.contains("123"));
        assertFalse(uniqueExtensions.contains("567"));

        DistributionList dl2 = new DistributionList();
        dl2.setExtensions(StringUtils.split("234 345 456 567"));
        Collection<String> uniqueExtensions2 = DistributionList.getUniqueExtensions(dl1, dl2);
        assertEquals(5, uniqueExtensions2.size());
        assertTrue(uniqueExtensions2.contains("123"));
        assertTrue(uniqueExtensions2.contains("567"));
    }
}
