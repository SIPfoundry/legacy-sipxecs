/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.skin;

import java.util.HashSet;
import java.util.Set;

import junit.framework.TestCase;

import org.apache.tapestry.IAsset;
import org.easymock.classextension.EasyMock;

public class SkinControlTest extends TestCase {

    public void testIe() {
        assertNotIe("Opera/9.0 (Windows NT 5.0; U; en)");
        assertNotIe("Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.8.0.10) Gecko/20070313 Fedora/1.5.0.10-5.fc6 Firefox/1.5.0.10");
        assertNotIe("Foo/1.0 (NOTMSIE 1.0)");
        assertIe("Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 1.1.4322; .NET CLR 2.0.50727)");
        assertIe("Mozilla/4.0 (compatible; MSIE 7.0b; Windows NT 6.0)");
        assertIe("Foo/1.0 (MSIE 1.0)");
    }

    private void assertIe(String userAgent) {
        assertTrue(SkinControl.IE_REGEX.matcher(userAgent).matches());
    }

    private void assertNotIe(String userAgent) {
        assertFalse(SkinControl.IE_REGEX.matcher(userAgent).matches());
    }

    public void testGetStylesheetAssets() {
        DummySkinControl skin = new DummySkinControl();
        IAsset[] firefox = skin.getStylesheetAssets("foo");
        assertEquals(2, firefox.length);

        skin = new DummySkinControl();
        IAsset[] ie6 = skin.getStylesheetAssets("Mozilla/4.0 (compatible; MSIE 6.0)");
        assertEquals(3, ie6.length);
        skin.m_paths.contains("ie6-hacks.css");

        skin = new DummySkinControl();
        IAsset[] ie7 = skin.getStylesheetAssets("Mozilla/4.0 (compatible; MSIE 7.0)");
        assertEquals(3, ie7.length);
        skin.m_paths.contains("ie7-hacks.css");
    }

    static class DummySkinControl extends SkinControl {
        Set m_paths = new HashSet();
        public IAsset getAsset(String path) {
            m_paths.add(path);
            return EasyMock.createNiceControl().createMock(IAsset.class);
        }
    }
}
