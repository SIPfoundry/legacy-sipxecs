/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.registrar;

import static org.junit.Assert.assertEquals;

import java.io.StringWriter;
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.io.IOUtils;
import org.easymock.classextension.EasyMock;
import org.junit.Test;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureManagerImpl;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.im.ImManager;
import org.sipfoundry.sipxconfig.parkorbit.ParkOrbitContext;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.springframework.context.ApplicationContext;


public class RegistrarConfigurationTest {
    private static String FEATURE_ID = "featureId";
    @Test
    public void testConfigWithNoPlugins() throws Exception {
        RegistrarConfiguration config = new RegistrarConfiguration();
        ApplicationContext context = EasyMock.createMock(ApplicationContext.class);
        context.getBeansOfType(RegistrarConfigurationPlugin.class);
        EasyMock.expectLastCall().andReturn(null).anyTimes();
        EasyMock.replay(context);
        config.setApplicationContext(context);
        StringWriter actual = new StringWriter();
        RegistrarSettings settings = new RegistrarSettings();
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        //test allow unbound config value replication
        settings.setSettingTypedValue("registrar-config/SIP_REDIRECT.140-FALLBACK.ALLOW_UNBOUND", true);
        Domain domain = new Domain("example.org");
        domain.setSipRealm("grapefruit");
        Address imApi = new Address(ImManager.XMLRPC_ADDRESS, "im.example.org", 100);
        Address presenceApi = new Address(Registrar.PRESENCE_MONITOR_ADDRESS, "presence.example.org", 101);
        Address park = new Address(ParkOrbitContext.SIP_TCP_PORT, "park.example.org", 102);
        Address proxy = new Address(ProxyManager.TCP_ADDRESS, "proxy.example.org", 103);
        Location location = TestHelper.createDefaultLocation();
        config.write(actual, settings, domain, location, proxy, imApi, presenceApi, park, new FeatureManagerImpl());
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-registrar-config"));
        assertEquals(expected, actual.toString());
    }

    @Test
    public void testConfigWithPlugins() throws Exception {
        RegistrarConfiguration config = new RegistrarConfiguration();
        ApplicationContext context = EasyMock.createMock(ApplicationContext.class);
        context.getBeansOfType(RegistrarConfigurationPlugin.class);
        RegistrarConfigurationPlugin regPlugin = new RegistrarConfigurationPlugin();
        Map<String, String> plugins = new HashMap<String, String>();
        plugins.put("SIP_REGISTRAR_HOOK_LIBRARY.TEST_PLUGIN", "/usr/lib/testPlugin.so");
        plugins.put("SIP_REGISTRAR_HOOK_LIBRARY.TEST1_PLUGIN", "/usr/lib/test1Plugin.so");
        regPlugin.setRegistrarPlugins(plugins);
        Map<String, RegistrarConfigurationPlugin> beans = new HashMap<String, RegistrarConfigurationPlugin>();
        beans.put("test", regPlugin);
        EasyMock.expectLastCall().andReturn(beans).anyTimes();
        EasyMock.replay(context);
        config.setApplicationContext(context);
        StringWriter actual = new StringWriter();
        RegistrarSettings settings = new RegistrarSettings();
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        Domain domain = new Domain("example.org");
        domain.setSipRealm("grapefruit");
        Address imApi = new Address(ImManager.XMLRPC_ADDRESS, "im.example.org", 100);
        Address presenceApi = new Address(Registrar.PRESENCE_MONITOR_ADDRESS, "presence.example.org", 101);
        Address park = new Address(ParkOrbitContext.SIP_TCP_PORT, "park.example.org", 102);
        Address proxy = new Address(ProxyManager.TCP_ADDRESS, "proxy.example.org", 103);
        Location location = TestHelper.createDefaultLocation();
        config.write(actual, settings, domain, location, proxy, imApi, presenceApi, park, new FeatureManagerImpl());
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-registrar-config-with-plugins"));
        assertEquals(expected, actual.toString());
    }

    @Test
    public void testConfigWithPluginsFeatureNotEnabled() throws Exception {
        RegistrarConfiguration config = new RegistrarConfiguration();
        ApplicationContext context = EasyMock.createMock(ApplicationContext.class);
        context.getBeansOfType(RegistrarConfigurationPlugin.class);
        RegistrarConfigurationPlugin regPlugin = new RegistrarConfigurationPlugin();
        Map<String, String> plugins = new HashMap<String, String>();
        plugins.put("SIP_REGISTRAR_HOOK_LIBRARY.TEST_PLUGIN", "/usr/lib/testPlugin.so");
        plugins.put("SIP_REGISTRAR_HOOK_LIBRARY.TEST1_PLUGIN", "/usr/lib/test1Plugin.so");
        regPlugin.setRegistrarPlugins(plugins);
        regPlugin.setFeatureId(FEATURE_ID);
        Map<String, RegistrarConfigurationPlugin> beans = new HashMap<String, RegistrarConfigurationPlugin>();
        beans.put("test", regPlugin);
        EasyMock.expectLastCall().andReturn(beans).anyTimes();
        EasyMock.replay(context);

        FeatureManager fm = EasyMock.createMock(FeatureManager.class);
        fm.isFeatureEnabled(new LocationFeature(FEATURE_ID));
        EasyMock.expectLastCall().andReturn(false);
        fm.isFeatureEnabled(new GlobalFeature(FEATURE_ID));
        EasyMock.expectLastCall().andReturn(false);
        EasyMock.replay(fm);

        RegistrarSettings settings = new RegistrarSettings();
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        //test allow unbound config value replication
        settings.setSettingTypedValue("registrar-config/SIP_REDIRECT.140-FALLBACK.ALLOW_UNBOUND", true);

        config.setApplicationContext(context);
        StringWriter actual = new StringWriter();
        Domain domain = new Domain("example.org");
        domain.setSipRealm("grapefruit");
        Address imApi = new Address(ImManager.XMLRPC_ADDRESS, "im.example.org", 100);
        Address presenceApi = new Address(Registrar.PRESENCE_MONITOR_ADDRESS, "presence.example.org", 101);
        Address park = new Address(ParkOrbitContext.SIP_TCP_PORT, "park.example.org", 102);
        Address proxy = new Address(ProxyManager.TCP_ADDRESS, "proxy.example.org", 103);
        Location location = TestHelper.createDefaultLocation();
        config.write(actual, settings, domain, location, proxy, imApi, presenceApi, park, fm);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-registrar-config"));
        assertEquals(expected, actual.toString());
    }

    @Test
    public void testConfigWithPluginsFeatureEnabled() throws Exception {
        RegistrarConfiguration config = new RegistrarConfiguration();
        ApplicationContext context = EasyMock.createMock(ApplicationContext.class);
        context.getBeansOfType(RegistrarConfigurationPlugin.class);
        RegistrarConfigurationPlugin regPlugin = new RegistrarConfigurationPlugin();
        Map<String, String> plugins = new HashMap<String, String>();
        plugins.put("SIP_REGISTRAR_HOOK_LIBRARY.TEST_PLUGIN", "/usr/lib/testPlugin.so");
        plugins.put("SIP_REGISTRAR_HOOK_LIBRARY.TEST1_PLUGIN", "/usr/lib/test1Plugin.so");
        regPlugin.setRegistrarPlugins(plugins);
        regPlugin.setFeatureId(FEATURE_ID);
        Map<String, RegistrarConfigurationPlugin> beans = new HashMap<String, RegistrarConfigurationPlugin>();
        beans.put("test", regPlugin);
        EasyMock.expectLastCall().andReturn(beans).anyTimes();
        EasyMock.replay(context);

        FeatureManager fm = EasyMock.createMock(FeatureManager.class);
        fm.isFeatureEnabled(new LocationFeature(FEATURE_ID));
        EasyMock.expectLastCall().andReturn(true);
        fm.isFeatureEnabled(new GlobalFeature(FEATURE_ID));
        EasyMock.expectLastCall().andReturn(false);
        EasyMock.replay(fm);

        config.setApplicationContext(context);
        StringWriter actual = new StringWriter();
        RegistrarSettings settings = new RegistrarSettings();
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        Domain domain = new Domain("example.org");
        domain.setSipRealm("grapefruit");
        Address imApi = new Address(ImManager.XMLRPC_ADDRESS, "im.example.org", 100);
        Address presenceApi = new Address(Registrar.PRESENCE_MONITOR_ADDRESS, "presence.example.org", 101);
        Address park = new Address(ParkOrbitContext.SIP_TCP_PORT, "park.example.org", 102);
        Address proxy = new Address(ProxyManager.TCP_ADDRESS, "proxy.example.org", 103);
        Location location = TestHelper.createDefaultLocation();
        config.write(actual, settings, domain, location, proxy, imApi, presenceApi, park, fm);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-registrar-config-with-plugins"));
        assertEquals(expected, actual.toString());
    }
}
