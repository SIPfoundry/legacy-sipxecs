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

import org.custommonkey.xmlunit.XMLUnit;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.device.ProfileGenerator;
import org.sipfoundry.sipxconfig.device.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class ApplicationConfigurationTest extends PolycomXmlTestCase {

    private PolycomPhone phone;
    private ProfileGenerator m_pg;
    private MemoryProfileLocation m_location;

    @Override
    protected void setUp() throws Exception {
        XMLUnit.setIgnoreWhitespace(true);
        phone = new PolycomPhone();
        PolycomModel model = new PolycomModel();
        model.setMaxLineCount(6);
        phone.setModel(model);
        PhoneTestDriver.supplyTestData(phone);

        m_location = new MemoryProfileLocation();

        VelocityProfileGenerator pg = new VelocityProfileGenerator();
        pg.setVelocityEngine(TestHelper.getVelocityEngine());
        m_pg = pg;
    }

    public void testGenerateProfile() throws Exception {
        ApplicationConfiguration app = new ApplicationConfiguration(phone);

        m_pg.generate(m_location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-macaddress.cfg");

        assertPolycomXmlEquals(expectedPhoneStream, m_location.getReader());
        expectedPhoneStream.close();
    }
}
