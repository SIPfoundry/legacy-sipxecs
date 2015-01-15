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
import java.util.ArrayList;
import java.util.List;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.moh.MusicOnHoldManager;
import org.sipfoundry.sipxconfig.mwi.Mwi;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.rls.Rls;

public class RegAdvancedConfigurationTest extends PolycomXmlTestCase {

    @Override
    protected void setUp() throws Exception {
        setUp404150TestsMwiMohRlsEnabled();
    }

    public void testGenerateProfile41() throws Exception {

        RegAdvancedConfiguration app = new RegAdvancedConfiguration(phone41);

        m_pg.generate(location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-sipx-reg-advanced.cfg");
        dumpXml(location.getReader(), System.out);
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
        FeatureManager featureManagerMock = createMock(FeatureManager.class);
        featureManagerMock.isFeatureEnabled(Mwi.FEATURE);
        EasyMock.expectLastCall().andReturn(true).anyTimes();

        featureManagerMock.isFeatureEnabled(MusicOnHoldManager.FEATURE);
        EasyMock.expectLastCall().andReturn(true).anyTimes();

        featureManagerMock.isFeatureEnabled(Rls.FEATURE);
        EasyMock.expectLastCall().andReturn(true).anyTimes();
        PolycomPhone phoneVVX600 = new PolycomPhone();
        ModelSource<PhoneModel> phoneModelSource = createMock(ModelSource.class);
        phoneVVX600.setModelId("polycomVVX600");
        phoneVVX600.setBeanId("polycomVVX600");
        phoneVVX600.setPhoneModelSource(phoneModelSource);
        phoneVVX600.setModel(phoneModelBuilder("polycomVVX600", getClass()));
        phoneVVX600.setDeviceVersion(PolycomModel.VER_5_0_0);
        PhoneTestDriver.supplyTestData(phoneVVX600);
        phoneVVX600.setFeatureManager(featureManagerMock);
        EasyMock.replay(featureManagerMock);
        RegAdvancedConfiguration app = new RegAdvancedConfiguration(phoneVVX600);

        m_pg.generate(location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-sipx-reg-advanced-50-vvx600.cfg");
        assertPolycomXmlEquals(new InputStreamReader(expectedPhoneStream), location.getReader());

        expectedPhoneStream.close();
    }


    public void testGenerateProfile501VVX600() throws Exception {
        FeatureManager featureManagerMock = createMock(FeatureManager.class);
        featureManagerMock.isFeatureEnabled(Mwi.FEATURE);
        EasyMock.expectLastCall().andReturn(true).anyTimes();

        featureManagerMock.isFeatureEnabled(MusicOnHoldManager.FEATURE);
        EasyMock.expectLastCall().andReturn(true).anyTimes();

        featureManagerMock.isFeatureEnabled(Rls.FEATURE);
        EasyMock.expectLastCall().andReturn(true).anyTimes();

        PolycomPhone phoneVVX600 = new PolycomPhone();
        ModelSource<PhoneModel> phoneModelSource = createMock(ModelSource.class);
        phoneVVX600.setModelId("polycomVVX600");
        phoneVVX600.setBeanId("polycomVVX600");
        phoneVVX600.setPhoneModelSource(phoneModelSource);
        phoneVVX600.setModel(phoneModelBuilder("polycomVVX600", getClass()));
        phoneVVX600.setDeviceVersion(PolycomModel.VER_5_0_1);
        PhoneTestDriver.supplyTestData(phoneVVX600);
        phoneVVX600.setFeatureManager(featureManagerMock);
        EasyMock.replay(featureManagerMock);
        RegAdvancedConfiguration app = new RegAdvancedConfiguration(phoneVVX600);

        m_pg.generate(location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-sipx-reg-advanced-50-vvx600.cfg");
        assertPolycomXmlEquals(new InputStreamReader(expectedPhoneStream), location.getReader());

        expectedPhoneStream.close();
    }

    public void testGenerateProfile501() throws Exception {
        RegAdvancedConfiguration app = new RegAdvancedConfiguration(phone501);

        m_pg.generate(location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-sipx-reg-advanced-50.cfg");
        assertPolycomXmlEquals(new InputStreamReader(expectedPhoneStream), location.getReader());

        expectedPhoneStream.close();
    }
    public void testGenerateProfile502() throws Exception {
        RegAdvancedConfiguration app = new RegAdvancedConfiguration(phone502);

        m_pg.generate(location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-sipx-reg-advanced-50.cfg");
        assertPolycomXmlEquals(new InputStreamReader(expectedPhoneStream), location.getReader());

        expectedPhoneStream.close();
    }
    public void testGenerateProfile416() throws Exception {
        RegAdvancedConfiguration app = new RegAdvancedConfiguration(phone416);

        m_pg.generate(location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-sipx-reg-advanced-416.cfg");
        assertPolycomXmlEquals(new InputStreamReader(expectedPhoneStream), location.getReader());

        expectedPhoneStream.close();
    }

    public void testGenerateProfile520() throws Exception {
        RegAdvancedConfiguration app = new RegAdvancedConfiguration(phone520);

        m_pg.generate(location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-sipx-reg-advanced-520.cfg");

        dumpXml(location.getReader(), System.out);

        assertPolycomXmlEquals(new InputStreamReader(expectedPhoneStream), location.getReader());

        expectedPhoneStream.close();
    }

    //502 profiles
    public void testGenerateProfileMohDisabled() throws Exception {
        PolycomModel model = phoneModelBuilder("polycomVVX500", getClass());
        ModelSource<PhoneModel> phoneModelSource = createMock(ModelSource.class);
        FeatureManager featureManagerMock = createMock(FeatureManager.class);

        featureManagerMock.isFeatureEnabled(MusicOnHoldManager.FEATURE);
        EasyMock.expectLastCall().andReturn(false).anyTimes();
        featureManagerMock.isFeatureEnabled(Mwi.FEATURE);
        EasyMock.expectLastCall().andReturn(false).anyTimes();

        featureManagerMock.isFeatureEnabled(Rls.FEATURE);
        EasyMock.expectLastCall().andReturn(true).anyTimes();

        PolycomPhone phone = new PolycomPhone();
        phone.setModelId("polycomVVX500");
        phone.setPhoneModelSource(phoneModelSource);
        phone.setBeanId("polycomVVX500");
        phone.setModel(model);
        phone.setDeviceVersion(PolycomModel.VER_5_0_2);
        phone.setFeatureManager(featureManagerMock);
        PhoneTestDriver.supplyTestData(phone);

        RegAdvancedConfiguration app = new RegAdvancedConfiguration(phone);
        EasyMock.replay(featureManagerMock);
        m_pg.generate(location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-sipx-reg-advanced-moh-disabled.cfg");
        assertPolycomXmlEquals(new InputStreamReader(expectedPhoneStream), location.getReader());

        expectedPhoneStream.close();
    }

    //502 profiles
    //Also tests sipXprovision AOR
    public void testGenerateProfileMohEnabledUnassigned() throws Exception {
        PolycomModel model = phoneModelBuilder("polycomVVX500", getClass());
        ModelSource<PhoneModel> phoneModelSource = createMock(ModelSource.class);
        FeatureManager featureManagerMock = createMock(FeatureManager.class);

        featureManagerMock.isFeatureEnabled(MusicOnHoldManager.FEATURE);
        EasyMock.expectLastCall().andReturn(true).anyTimes();
        featureManagerMock.isFeatureEnabled(Mwi.FEATURE);
        EasyMock.expectLastCall().andReturn(true).anyTimes();

        featureManagerMock.isFeatureEnabled(Rls.FEATURE);
        EasyMock.expectLastCall().andReturn(true).anyTimes();

        List<Line> lines = new ArrayList<Line>();

        PolycomPhone phone = new PolycomPhone();
        phone.setLines(lines);
        phone.setModelId("polycomVVX500");
        phone.setPhoneModelSource(phoneModelSource);
        phone.setBeanId("polycomVVX500");
        phone.setModel(model);
        phone.setDeviceVersion(PolycomModel.VER_5_0_2);
        phone.setFeatureManager(featureManagerMock);
        PhoneTestDriver.supplyTestData(phone, new ArrayList<User>());

        RegAdvancedConfiguration app = new RegAdvancedConfiguration(phone);
        EasyMock.replay(featureManagerMock);
        m_pg.generate(location, app, null, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-sipx-reg-advanced-moh-enabled-unassigned.cfg");
        assertPolycomXmlEquals(new InputStreamReader(expectedPhoneStream), location.getReader());

        expectedPhoneStream.close();
    }
}
