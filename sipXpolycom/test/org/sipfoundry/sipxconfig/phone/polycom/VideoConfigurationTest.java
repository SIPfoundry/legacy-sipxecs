/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.polycom;

import java.io.InputStream;
import java.io.InputStreamReader;

public class VideoConfigurationTest extends PolycomXmlTestCase {

    @Override
    protected void setUp() throws Exception {
        setUp404150Tests();
    }

    public void testGenerateProfile41() throws Exception {

        VideoConfiguration app = new VideoConfiguration(phone41);

        m_pg.generate(location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-sipx-video.cfg");
        assertPolycomXmlEquals(new InputStreamReader(expectedPhoneStream), location.getReader());

        expectedPhoneStream.close();
    }

    public void testGenerateProfile40() throws Exception {
        VideoConfiguration app = new VideoConfiguration(phone40);

        m_pg.generate(location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-sipx-video.cfg");
        assertPolycomXmlEquals(new InputStreamReader(expectedPhoneStream), location.getReader());

        expectedPhoneStream.close();
    }

}
