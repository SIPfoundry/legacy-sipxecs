/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.admin;

import static org.junit.Assert.assertEquals;

import java.io.StringWriter;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.io.IOUtils;
import org.easymock.EasyMock;
import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.mwi.Mwi;
import org.sipfoundry.sipxconfig.mwi.MwiSettings;
import org.sipfoundry.sipxconfig.parkorbit.ParkOrbitContext;
import org.sipfoundry.sipxconfig.parkorbit.ParkSettings;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.sipfoundry.sipxconfig.proxy.ProxySettings;
import org.sipfoundry.sipxconfig.registrar.Registrar;
import org.sipfoundry.sipxconfig.registrar.RegistrarSettings;
import org.sipfoundry.sipxconfig.rls.Rls;
import org.sipfoundry.sipxconfig.rls.RlsSettings;
import org.sipfoundry.sipxconfig.saa.SaaManager;
import org.sipfoundry.sipxconfig.saa.SaaSettings;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingImpl;
import org.sipfoundry.sipxconfig.setting.SettingSet;
import org.sipfoundry.sipxconfig.setting.type.BooleanSetting;
import org.sipfoundry.sipxconfig.setting.type.IntegerSetting;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.springframework.beans.factory.ListableBeanFactory;

public class ResLimitsConfigurationTest {
    private ResLimitsConfiguration m_config;

    @Before
    public void setUp() {
        m_config = new ResLimitsConfiguration();

        PluginSettings pluginSettings = new PluginSettings();
        pluginSettings.setSettingTypedValue("resource-limits/fd-soft", "11119");
        pluginSettings.setSettingTypedValue("resource-limits/fd-hard", "11118");
        pluginSettings.setSettingTypedValue("resource-limits/core-enabled", false);

        ResLimitPluginConfig config1 = EasyMock.createMock(ResLimitPluginConfig.class);
        config1.getLimitsConfig();
        EasyMock.expectLastCall().andReturn(new PluginResLimitsConfig());
        config1.getLocationFeature();
        EasyMock.expectLastCall().andReturn(new LocationFeature("PLUGIN_FEATURE"));
        config1.getSettings();
        EasyMock.expectLastCall().andReturn(pluginSettings);

        ListableBeanFactory beanFactory = EasyMock.createMock(ListableBeanFactory.class);
        Map<String, ResLimitPluginConfig> resLimitsConfigsMap = new HashMap<String, ResLimitPluginConfig>();
        resLimitsConfigsMap.put("config1", config1);
        beanFactory.getBeansOfType(ResLimitPluginConfig.class, false, false);
        EasyMock.expectLastCall().andReturn(resLimitsConfigsMap);

        m_config.setBeanFactory(beanFactory);

        Collection<AbstractResLimitsConfig> resLimitsConfigs = new ArrayList<AbstractResLimitsConfig>();
        ResLimitsConfigImpl proxyResLimits = new ResLimitsConfigImpl();
        proxyResLimits.setPrefix("sipxproxy-");
        ResLimitsConfigImpl registrarResLimits = new ResLimitsConfigImpl();
        registrarResLimits.setPrefix("sipxregistry-");
        ResLimitsConfigImpl parkResLimits = new ResLimitsConfigImpl();
        parkResLimits.setPrefix("sipxpark-");
        ResLimitsConfigImpl mwiResLimits = new ResLimitsConfigImpl();
        mwiResLimits.setPrefix("sipxpublisher-");
        ResLimitsConfigImpl rlsResLimits = new ResLimitsConfigImpl();
        rlsResLimits.setPrefix("sipxrls-");
        ResLimitsConfigImpl sipxsaaResLimits = new ResLimitsConfigImpl();
        sipxsaaResLimits.setPrefix("sipxsaa-");
        resLimitsConfigs.add(proxyResLimits);
        resLimitsConfigs.add(registrarResLimits);
        resLimitsConfigs.add(parkResLimits);
        resLimitsConfigs.add(mwiResLimits);
        resLimitsConfigs.add(rlsResLimits);
        resLimitsConfigs.add(sipxsaaResLimits);
        m_config.setResLimitsConfigs(resLimitsConfigs);

        m_config.setParkLimitsConfig(parkResLimits);
        m_config.setRegistrarLimitsConfig(registrarResLimits);
        m_config.setProxyLimitsConfig(proxyResLimits);
        m_config.setSaaLimitsConfig(sipxsaaResLimits);
        m_config.setPublisherLimitsConfig(mwiResLimits);

        AdminSettings settings = new AdminSettings();
        settings.setPasswordPolicy(new PasswordPolicyImpl());
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        //settings.setSettingTypedValue("configserver-config/fd-soft", "65656");
        //settings.setSettingTypedValue("configserver-config/fd-hard", "65657");
        //settings.setSettingTypedValue("configserver-config/core-enabled", true);
        AdminContext adminContext = EasyMock.createMock(AdminContext.class);
        adminContext.getSettings();
        EasyMock.expectLastCall().andReturn(settings);

        ProxySettings proxySettings = new ProxySettings();
        proxySettings.setModelFilesContext(TestHelper.getModelFilesContext());
        proxySettings.setSettingTypedValue("resource-limits/fd-soft", "11111");
        proxySettings.setSettingTypedValue("resource-limits/fd-hard", "11112");
        proxySettings.setSettingTypedValue("resource-limits/core-enabled", false);
        ProxyManager proxyManager = EasyMock.createMock(ProxyManager.class);
        proxyManager.getSettings();
        EasyMock.expectLastCall().andReturn(proxySettings);
        proxyManager.saveSettings(proxySettings);
        EasyMock.expectLastCall().anyTimes();

        MwiSettings mwiSettings = new MwiSettings();
        mwiSettings.setModelFilesContext(TestHelper.getModelFilesContext());
        mwiSettings.setSettingTypedValue("resource-limits/fd-soft", "22222");
        mwiSettings.setSettingTypedValue("resource-limits/fd-hard", "22223");
        mwiSettings.setSettingTypedValue("resource-limits/core-enabled", true);
        Mwi mwi = EasyMock.createMock(Mwi.class);
        mwi.getSettings();
        EasyMock.expectLastCall().andReturn(mwiSettings);
        mwi.saveSettings(mwiSettings);
        EasyMock.expectLastCall().anyTimes();

        RegistrarSettings registrarSettings = new RegistrarSettings();
        registrarSettings.setModelFilesContext(TestHelper.getModelFilesContext());
        registrarSettings.setSettingTypedValue("resource-limits/fd-soft", "33333");
        registrarSettings.setSettingTypedValue("resource-limits/fd-hard", "33334");
        registrarSettings.setSettingTypedValue("resource-limits/core-enabled", false);
        Registrar registrar = EasyMock.createMock(Registrar.class);
        registrar.getSettings();
        EasyMock.expectLastCall().andReturn(registrarSettings);
        registrar.saveSettings(registrarSettings);
        EasyMock.expectLastCall().anyTimes();

        RlsSettings rlsSettings = new RlsSettings();
        rlsSettings.setModelFilesContext(TestHelper.getModelFilesContext());
        rlsSettings.setSettingTypedValue("resource-limits/fd-soft", "44444");
        rlsSettings.setSettingTypedValue("resource-limits/fd-hard", "44445");
        rlsSettings.setSettingTypedValue("resource-limits/core-enabled", false);
        Rls rls = EasyMock.createMock(Rls.class);
        rls.getSettings();
        EasyMock.expectLastCall().andReturn(rlsSettings);
        rls.saveSettings(rlsSettings);
        EasyMock.expectLastCall().anyTimes();

        SaaSettings saaSettings = new SaaSettings();
        saaSettings.setModelFilesContext(TestHelper.getModelFilesContext());
        saaSettings.setSettingTypedValue("resource-limits/fd-soft", "55555");
        saaSettings.setSettingTypedValue("resource-limits/fd-hard", "55556");
        saaSettings.setSettingTypedValue("resource-limits/core-enabled", false);
        SaaManager saaManager = EasyMock.createMock(SaaManager.class);
        saaManager.getSettings();
        EasyMock.expectLastCall().andReturn(saaSettings);
        saaManager.saveSettings(saaSettings);
        EasyMock.expectLastCall().anyTimes();

        ParkSettings parkSettings = new ParkSettings();
        parkSettings.setModelFilesContext(TestHelper.getModelFilesContext());
        parkSettings.setSettingTypedValue("resource-limits/fd-soft", "6666");
        parkSettings.setSettingTypedValue("resource-limits/fd-hard", "66667");
        parkSettings.setSettingTypedValue("resource-limits/core-enabled", false);
        ParkOrbitContext parkOrbitContext = EasyMock.createMock(ParkOrbitContext.class);
        parkOrbitContext.getSettings();
        EasyMock.expectLastCall().andReturn(parkSettings);
        parkOrbitContext.saveSettings(parkSettings);
        EasyMock.expectLastCall().anyTimes();

        EasyMock.replay(config1, beanFactory, adminContext,
            proxyManager, mwi, registrar, rls, saaManager, parkOrbitContext);

        m_config.setAdminContext(adminContext);
        m_config.setMwi(mwi);
        m_config.setRegistrar(registrar);
        m_config.setParkOrbitContext(parkOrbitContext);
        m_config.setSaaManager(saaManager);
        m_config.setProxyManager(proxyManager);
    }

    @Test
    public void testFeaturedConfig() throws Exception {
        StringWriter actual = new StringWriter();
        m_config.writeFeaturedResourceLimits(actual);
        String expected = IOUtils.toString(getClass().getResourceAsStream("featured-resource-limits"));
        assertEquals(expected, actual.toString());
    }

    //@Test
    //public void testDefaultConfig() throws Exception {
        //StringWriter actual = new StringWriter();
        //m_config.writeDefaultsResourceLimits(actual);
        //String expected = IOUtils.toString(getClass().getResourceAsStream("default-resource-limits"));
        //assertEquals(expected, actual.toString());
    //}

    private class PluginResLimitsConfig extends AbstractResLimitsConfig {
        @Override
        public String getPrefix() {
            return "plugin-";
        }
    }

    private class PluginSettings extends PersistableSettings {

        @Override
        protected Setting loadSettings() {
            Setting s = new SettingSet();
            s.setName("resource-limits/fd-soft");
            Setting s1 = new SettingSet("resource-limits");
            Setting s1_1 = new SettingImpl("fd-soft");
            Setting s1_2 = new SettingImpl("fd-hard");
            Setting s1_3 = new SettingImpl("core-enabled");
            s1_1.setType(new IntegerSetting());
            s1_2.setType(new IntegerSetting());
            BooleanSetting trueFalse = new BooleanSetting();
            trueFalse.setTrueValue("true");
            trueFalse.setFalseValue("false");
            s1_3.setType(trueFalse);
            s.addSetting(s1);
            s1.addSetting(s1_1);
            s1.addSetting(s1_2);
            s1.addSetting(s1_3);

            return s;
        }
        @Override
        public String getBeanId() {
            return "pluginSettings";
        }

    }
}