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

import org.sipfoundry.sipxconfig.site.common.AssetSelector;

import junit.framework.TestCase;

public class AssetSelectorTest extends TestCase {

    public void testGetSystemIndependentFileName() {
        assertEquals("kuku.txt", AssetSelector.getSystemIndependentFileName("kuku.txt"));
        assertEquals("kuku.txt", AssetSelector.getSystemIndependentFileName("/kuku.txt"));
        assertEquals("kuku.txt", AssetSelector.getSystemIndependentFileName("c:kuku.txt"));
        assertEquals("kuku.txt", AssetSelector.getSystemIndependentFileName("c:\\dir\\kuku.txt"));
        assertEquals("kuku.txt", AssetSelector.getSystemIndependentFileName("dir/kuku.txt"));
        assertEquals("kuku.txt", AssetSelector.getSystemIndependentFileName("/d\\ir/kuku.txt"));
        assertEquals("", AssetSelector.getSystemIndependentFileName(""));
        assertEquals("", AssetSelector.getSystemIndependentFileName(null));
    }
}
