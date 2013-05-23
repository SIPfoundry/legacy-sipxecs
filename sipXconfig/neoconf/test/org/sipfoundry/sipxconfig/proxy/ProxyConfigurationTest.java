/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.proxy;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.replay;
import static org.junit.Assert.assertEquals;

import java.io.StringWriter;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.io.IOUtils;
import org.dom4j.Document;
import org.easymock.classextension.EasyMock;
import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.common.InternalUser;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.sipfoundry.sipxconfig.tls.TlsPeer;
import org.sipfoundry.sipxconfig.tls.TlsPeerManager;
import org.springframework.context.ApplicationContext;


public class ProxyConfigurationTest {
    private Domain m_domain;
    private Location m_location;
    private ProxyConfiguration m_config;
    private ProxySettings m_settings;
        
    @Before
    public void setUp() {
        m_config = new ProxyConfiguration();
        m_settings = new ProxySettings();
        m_settings.setModelFilesContext(TestHelper.getModelFilesContext());
        m_location = TestHelper.createDefaultLocation();
        m_domain = new Domain("example.org");
        m_domain.setSipRealm("realm.example.org");
    }

    @Test
    public void testConfig() throws Exception {
        ApplicationContext context = EasyMock.createMock(ApplicationContext.class);
        context.getBeansOfType(ProxyHookPlugin.class);
        ProxyHookPlugin plugin = new ProxyHookPlugin() {
            
            @Override
            public boolean isEnabled() {
                return true;
            }

            @Override
            public String getProxyHookName() {
                return "SIPX_PROXY_HOOK_LIBRARY.111_test";
            }

            @Override
            public String getProxyHookValue() {
                return "$(sipx.SIPX_LIBDIR)/authplugins/libTest.so";
            }
        };
        Map<String, ProxyHookPlugin> beans = new HashMap<String, ProxyHookPlugin>();
        beans.put("test", plugin);
        EasyMock.expectLastCall().andReturn(beans).anyTimes();
        EasyMock.replay(context);
        m_config.setApplicationContext(context);
        StringWriter actual = new StringWriter();
        m_config.write(actual, m_settings, m_location, m_domain, true);
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-proxy-config"));
        assertEquals(expected, actual.toString());
    }
    
    @Test
    public void testPeersConfig() throws Exception {        
        TlsPeer peer1 = new TlsPeer();
        peer1.setName("trusteddomain.com");
        InternalUser user1 = new InternalUser();
        user1.setUserName("~~tp~trusteddomain.com");
        peer1.setInternalUser(user1);

        TlsPeer peer2 = new TlsPeer();
        peer2.setName("10.10.1.2");
        InternalUser user2 = new InternalUser();
        user2.setUserName("~~tp~10.10.1.2");
        peer2.setInternalUser(user2);
        
        Collection<TlsPeer> peers = Arrays.asList(peer1, peer2);
        
        TlsPeerManager tlsPeerManager = createMock(TlsPeerManager.class);
        replay(tlsPeerManager);
        Document doc = m_config.getDocument(peers);
        String actual = TestHelper.asString(doc);
        String expected = IOUtils.toString(getClass().getResourceAsStream("peeridentities.test.xml"));
        assertEquals(expected, actual);
    }
}
