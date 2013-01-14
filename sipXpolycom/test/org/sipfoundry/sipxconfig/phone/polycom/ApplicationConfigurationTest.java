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

import org.custommonkey.xmlunit.XMLUnit;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.device.ProfileGenerator;
import org.sipfoundry.sipxconfig.device.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.test.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class ApplicationConfigurationTest extends PolycomXmlTestCase {

    private PolycomPhone phone32;
    private PolycomPhone phone40;
    private PolycomPhone phone31;
    private ProfileGenerator m_pg;

    @Override
    protected void setUp() throws Exception {
        XMLUnit.setIgnoreWhitespace(true);
        phone32 = new PolycomPhone();
        phone32.setModelId("335");
        PhoneModel model = new PolycomModel();
        ModelSource<PhoneModel> phoneModelSource = createMock(ModelSource.class);
        phone32.setPhoneModelSource(phoneModelSource);
        model.setMaxLineCount(6);
        phone32.setModel(model);
        phone32.setDeviceVersion(PolycomModel.VER_3_2_X);
        PhoneTestDriver.supplyTestData(phone32);

        phone40 = new PolycomPhone();
        phone40.setModelId("335");
        phone40.setPhoneModelSource(phoneModelSource);
        phone40.setModel(model);
        phone40.setDeviceVersion(PolycomModel.VER_4_0_X);
        PhoneTestDriver.supplyTestData(phone40);

        phone31 = new PolycomPhone();
        phone31.setModelId("335");
        phone31.setPhoneModelSource(phoneModelSource);
        phone31.setModel(model);
        phone31.setDeviceVersion(PolycomModel.VER_3_1_X);
        PhoneTestDriver.supplyTestData(phone31);
        
    }

    public void testGenerateProfile() throws Exception {
        MemoryProfileLocation location = new MemoryProfileLocation();

        VelocityProfileGenerator pg = new VelocityProfileGenerator();
        pg.setVelocityEngine(TestHelper.getVelocityEngine());
        m_pg = pg;

        ApplicationConfiguration app = new ApplicationConfiguration(phone32);

        m_pg.generate(location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-macaddress.cfg");

        assertPolycomXmlEquals(expectedPhoneStream, location.getReader());
        expectedPhoneStream.close();
    }

    public void testGenerateProfile40() throws Exception {
        MemoryProfileLocation location = new MemoryProfileLocation();

        VelocityProfileGenerator pg = new VelocityProfileGenerator();
        pg.setVelocityEngine(TestHelper.getVelocityEngine());
        m_pg = pg;
        
        ApplicationConfiguration app = new ApplicationConfiguration(phone40);

        m_pg.generate(location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-macaddress40.cfg");

        assertPolycomXmlEquals(expectedPhoneStream, location.getReader());
        expectedPhoneStream.close();
    }

    public void testGenerateProfile31() throws Exception {
        MemoryProfileLocation location = new MemoryProfileLocation();

        VelocityProfileGenerator pg = new VelocityProfileGenerator();
        pg.setVelocityEngine(TestHelper.getVelocityEngine());
        m_pg = pg;
        
        ApplicationConfiguration app = new ApplicationConfiguration(phone31,"192.168.1.20");

        m_pg.generate(location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-macaddress31.cfg");

        assertPolycomXmlEquals(expectedPhoneStream, location.getReader());
        expectedPhoneStream.close();
    }
    
    public void testNonBlankEndsInComma() {
        assertEquals("", ApplicationConfiguration.nonBlankEndsInComma(null));
        assertEquals("", ApplicationConfiguration.nonBlankEndsInComma(""));
        assertEquals("goose,", ApplicationConfiguration.nonBlankEndsInComma("goose"));
        assertEquals("goose,", ApplicationConfiguration.nonBlankEndsInComma("goose,"));
        assertEquals("goose,", ApplicationConfiguration.nonBlankEndsInComma("goose, "));
    }
}
