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

import static org.easymock.EasyMock.createMock;

import java.io.InputStream;
import java.io.InputStreamReader;

import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class RegAdvancedConfigurationTest extends PolycomXmlTestCase {

    @Override
    protected void setUp() throws Exception {
        setUp404150Tests();
    }

    public void testGenerateProfile41() throws Exception {

        RegAdvancedConfiguration app = new RegAdvancedConfiguration(phone41);

        m_pg.generate(location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-sipx-reg-advanced.cfg");
        assertPolycomXmlEquals(new InputStreamReader(expectedPhoneStream), location.getReader());

        expectedPhoneStream.close();
    }

    public void testGenerateProfile40() throws Exception {
        RegAdvancedConfiguration app = new RegAdvancedConfiguration(phone40);

        m_pg.generate(location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-sipx-reg-advanced.cfg");
        assertPolycomXmlEquals(new InputStreamReader(expectedPhoneStream), location.getReader());

        expectedPhoneStream.close();
    }

    public void testGenerateProfile50() throws Exception {
        RegAdvancedConfiguration app = new RegAdvancedConfiguration(phone50);

        m_pg.generate(location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-sipx-reg-advanced-50.cfg");
        assertPolycomXmlEquals(new InputStreamReader(expectedPhoneStream), location.getReader());

        expectedPhoneStream.close();
    }
    
    public void testGenerateProfile50VVX600() throws Exception {
        PolycomPhone phoneVVX600 = new PolycomPhone();
        ModelSource<PhoneModel> phoneModelSource = createMock(ModelSource.class);
        phoneVVX600.setModelId("polycomVVX600");
        phoneVVX600.setBeanId("polycomVVX600");
        phoneVVX600.setPhoneModelSource(phoneModelSource);
        phoneVVX600.setModel(phoneModelBuilder("polycomVVX600", getClass()));
        phoneVVX600.setDeviceVersion(PolycomModel.VER_5_0_0);
        PhoneTestDriver.supplyTestData(phoneVVX600);
        
        RegAdvancedConfiguration app = new RegAdvancedConfiguration(phoneVVX600);

        m_pg.generate(location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-sipx-reg-advanced-50-vvx600.cfg");
        assertPolycomXmlEquals(new InputStreamReader(expectedPhoneStream), location.getReader());

        expectedPhoneStream.close();
    }
    
    
}
