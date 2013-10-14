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

public class FeaturesConfigurationTest extends PolycomXmlTestCase {

    @Override
    protected void setUp() throws Exception {
        setUp404150Tests();
    }

    public void testGenerateProfile41() throws Exception {
        FeaturesConfiguration app = new FeaturesConfiguration(phone41);

        m_pg.generate(location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-sipx-features.cfg");
        assertPolycomXmlEquals(new InputStreamReader(expectedPhoneStream), location.getReader());

        expectedPhoneStream.close();
    }

    public void testGenerateProfile40() throws Exception {

        FeaturesConfiguration app = new FeaturesConfiguration(phone40);

        m_pg.generate(location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-sipx-features.cfg");
        assertPolycomXmlEquals(new InputStreamReader(expectedPhoneStream), location.getReader());

        expectedPhoneStream.close();
    }

    public void testGenerateProfile50() throws Exception {

        FeaturesConfiguration app = new FeaturesConfiguration(phone50);

        m_pg.generate(location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-sipx-features.cfg");
        assertPolycomXmlEquals(new InputStreamReader(expectedPhoneStream), location.getReader());

        expectedPhoneStream.close();
    }
}
