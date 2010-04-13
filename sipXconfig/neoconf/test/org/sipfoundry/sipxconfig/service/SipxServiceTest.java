/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;

import java.io.IOException;
import java.io.Writer;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;

import junit.framework.TestCase;
import org.sipfoundry.sipxconfig.admin.AbstractConfigurationFile;
import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class SipxServiceTest extends TestCase {

    public void testIsAutoEnabled() {
        SipxServiceBundle autoEnabledBundle = new SipxServiceBundle("autoEnabled");
        autoEnabledBundle.setAutoEnable(true);

        SipxServiceBundle standardBundle = new SipxServiceBundle("standard");

        SipxService service = new SipxService() {
        };

        assertFalse(service.isAutoEnabled());

        service.setBundles(new HashSet(Arrays.asList(autoEnabledBundle, standardBundle)));
        assertTrue(service.isAutoEnabled());

        service.setBundles(Collections.singleton(autoEnabledBundle));
        assertTrue(service.isAutoEnabled());

        service.setBundles(Collections.singleton(standardBundle));
        assertFalse(service.isAutoEnabled());
    }

    public void testGetLogSetting() {
        // getLogSetting should return the path to the log setting for a service that implements
        // LoggingEntity
        SipxProxyService proxyService = new SipxProxyService();
        String logSetting = proxyService.getLogSetting();
        assertEquals(SipxProxyService.LOG_SETTING, logSetting);

        // getLogSetting should return null for a service that does not implement LoggingEntity
        SipxMrtgService mrtgService = new SipxMrtgService();
        logSetting = mrtgService.getLogSetting();
        assertNull(logSetting);
    }

    public void testGetBundlesForLocation() {
        SipxServiceBundle b1 = new SipxServiceBundle("b1");
        b1.setBeanName("b1");
        SipxServiceBundle b2 = new SipxServiceBundle("b2");
        b2.setBeanName("b2");
        SipxServiceBundle b3 = new SipxServiceBundle("b3");
        b3.setBeanName("b3");

        SipxService service = new SipxService() {
        };

        service.setBundles(new HashSet(Arrays.asList(b1, b2, b3)));

        Location location = new Location();

        List<SipxServiceBundle> bundles = service.getBundles(location);
        assertTrue(bundles.isEmpty());

        location.setInstalledBundles(Arrays.asList("b1", "b3"));
        bundles = service.getBundles(location);
        assertEquals(2, bundles.size());
        assertTrue(bundles.contains(b1));
        assertFalse(bundles.contains(b2));
        assertTrue(bundles.contains(b3));
    }

    public void testGetConfigurations() {
        SipxProxyService proxyService = new SipxProxyService();

        assertTrue(proxyService.getConfigurations().isEmpty());
        assertTrue(proxyService.getConfigurations(true).isEmpty());
        assertTrue(proxyService.getConfigurations(false).isEmpty());

        ConfigurationFile a = new DummyConfig("a", true);
        ConfigurationFile b = new DummyConfig("b", false);
        ConfigurationFile c = new DummyConfig("c", true);

        proxyService.setConfigurations(Arrays.asList(a, b, c));
        assertEquals(3, proxyService.getConfigurations().size());
        assertEquals(3, proxyService.getConfigurations(false).size());
        assertEquals(1, proxyService.getConfigurations(true).size());

        assertSame(b, proxyService.getConfigurations(true).get(0));
    }

    public void testGetFqdn() {
        SipxProxyService proxy = new SipxProxyService();
        proxy.setBeanId(SipxProxyService.BEAN_ID);

        Location l1 = new Location();
        l1.setFqdn("l1.example.com");

        Location l2 = new Location();
        l2.setFqdn("l2.example.com");

        LocationsManager lm = createMock(LocationsManager.class);
        lm.getLocationsForService(proxy);
        expectLastCall().andReturn(Collections.emptyList());
        lm.getLocationsForService(proxy);
        expectLastCall().andReturn(Arrays.asList(l2));
        lm.getLocationsForService(proxy);
        expectLastCall().andReturn(Arrays.asList(l1, l2));
        replay(lm);

        proxy.setLocationsManager(lm);
        assertNull(proxy.getFqdn());
        assertEquals("l2.example.com", proxy.getFqdn());
        assertEquals("l1.example.com", proxy.getFqdn());

        verify(lm);
    }

    static class DummyConfig extends AbstractConfigurationFile {
        public DummyConfig(String name, boolean restartRequired) {
            setName(name);
            setRestartRequired(restartRequired);
        }

        public String getFileContent() {
            return null;
        }

        public void write(Writer writer, Location location) throws IOException {
        }
    }
}
