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

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.rls.Rls;

public class FeaturesConfigurationTest extends PolycomXmlTestCase {

    @Override
    protected void setUp() throws Exception {
        setUp404150TestsMwiMohRlsEnabled();
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

    /**
     * test with a line configured and rls enabled
     * @throws Exception
     */
    public void testGenerateProfilesWithRlsEnabled() throws Exception {
        PolycomModel model = phoneModelBuilder("polycomVVX500", getClass());
        ModelSource<PhoneModel> phoneModelSource = createMock(ModelSource.class);
        FeatureManager featureManagerMock = createMock(FeatureManager.class);

        featureManagerMock.isFeatureEnabled(Rls.FEATURE);
        EasyMock.expectLastCall().andReturn(true).anyTimes();


        PolycomPhone phone = new PolycomPhone();
        phone.setModelId("polycomVVX500");
        phone.setPhoneModelSource(phoneModelSource);
        phone.setBeanId("polycomVVX500");
        phone.setModel(model);
        phone.setDeviceVersion(PolycomModel.VER_4_1_X);
        phone.setFeatureManager(featureManagerMock);
        PhoneTestDriver.supplyTestData(phone, false, true, false, false);

        FeaturesConfiguration app = new FeaturesConfiguration(phone);

        EasyMock.replay(featureManagerMock);

        m_pg.generate(location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-sipx-features-rls.cfg");
        assertPolycomXmlEquals(new InputStreamReader(expectedPhoneStream), location.getReader());

    }

    //rls disabled but with speeddials with blf
    public void testGenerateProfilesWithRlsDisabled() throws Exception {
        PolycomModel model = phoneModelBuilder("polycomVVX500", getClass());
        ModelSource<PhoneModel> phoneModelSource = createMock(ModelSource.class);
        FeatureManager featureManagerMock = createMock(FeatureManager.class);

        featureManagerMock.isFeatureEnabled(Rls.FEATURE);
        EasyMock.expectLastCall().andReturn(false).anyTimes();


        PolycomPhone phone = new PolycomPhone();
        phone.setModelId("polycomVVX500");
        phone.setPhoneModelSource(phoneModelSource);
        phone.setBeanId("polycomVVX500");
        phone.setModel(model);
        phone.setDeviceVersion(PolycomModel.VER_4_1_X);
        phone.setFeatureManager(featureManagerMock);
        PhoneTestDriver.supplyTestData(phone, false, true, false, false);

        FeaturesConfiguration app = new FeaturesConfiguration(phone);

        EasyMock.replay(featureManagerMock);

        m_pg.generate(location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-sipx-features.cfg");
        assertPolycomXmlEquals(new InputStreamReader(expectedPhoneStream), location.getReader());

    }
}
