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


import java.io.InputStream;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.site.common.AssetSelector;
import org.sipfoundry.sipxconfig.site.dialplan.PatternsEditorTest;

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

    public void testAudioFormat() throws Exception {
        InputStream wav = PatternsEditorTest.class.getResourceAsStream("thankyou_goodbye.wav");
        assertTrue(AssetSelector.isAcceptedAudioFormat(wav));

        InputStream notWav = getClass().getResourceAsStream("unused.png");
        assertFalse(AssetSelector.isAcceptedAudioFormat(notWav));
    }
}
