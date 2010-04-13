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
import java.net.URL;

import junit.framework.Test;
import net.sourceforge.jwebunit.junit.WebTestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class SkinServletTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(SkinServletTestUi.class);
    }

    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
    }

    public void testLogoAsset() throws Exception {
        assertAssetEquals("/skin/logo.png", "test-logo.png");
        assertAssetEquals("/skin/unused.png", "unused.png");
    }

    private void assertAssetEquals(String assetPath, String expectedAsset) throws Exception {
        URL logoUrl = new URL(SiteTestHelper.getBaseUrl() + assetPath);
        InputStream actual = logoUrl.openStream();
        InputStream expected = getClass().getResourceAsStream(expectedAsset);
        assertTrue(IOUtils.contentEquals(expected, actual));
    }
}
