/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.user;

import java.io.OutputStream;
import java.net.URL;
import java.net.URLConnection;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class PinTokenChangeServletTestUi extends WebTestCase {
    
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(PinTokenChangeServletTestUi.class);
    }

    protected void setUp() throws Exception {
        super.setUp();
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        SiteTestHelper.seedUser(getTester());
    }
    
    public void testUserPinChange() throws Exception {
        String baseurl = getTestContext().getBaseUrl();
        String urlString = baseurl + "api/change-pintoken";
        URL url = new URL(urlString);
        URLConnection connection = url.openConnection();
        connection.setDoOutput(true);
        connection.setDoInput(true);
        connection.setAllowUserInteraction(true);        
        OutputStream out = connection.getOutputStream();
        out.write("testuser;c19d0229f2904a5a235c4a649222c258;newpin".getBytes());
        out.close();
        connection.getContent();
    }
}
