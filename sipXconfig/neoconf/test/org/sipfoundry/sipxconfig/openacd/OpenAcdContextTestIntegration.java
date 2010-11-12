/*
 *
 *
 * Copyright (c) 2010 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.openacd;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.admin.NameInUseException;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.common.BeanId;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchAction;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchCondition;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.service.freeswitch.DefaultContextConfigurationTest;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class OpenAcdContextTestIntegration extends IntegrationTestCase {
    private OpenAcdContext m_openAcdContext;
    private OpenAcdContextImpl m_openAcdContextImpl;

    public void testOpenAcdExtensionCrud() throws Exception {
        // test save open acd extension
        assertEquals(0, m_openAcdContext.getFreeswitchExtensions().size());
        OpenAcdExtension extension = DefaultContextConfigurationTest.createOpenAcdExtension("example");
        m_openAcdContext.saveExtension(extension);
        assertEquals(1, m_openAcdContext.getFreeswitchExtensions().size());

        // test save extension with same name
        try {
            OpenAcdExtension sameNameExtension = new OpenAcdExtension();
            sameNameExtension.setName("example");
            m_openAcdContext.saveExtension(sameNameExtension);
            fail();
        } catch (NameInUseException ex) {
        }

        // test get extension by name
        OpenAcdExtension savedExtension = m_openAcdContext.getExtensionByName("example");
        assertNotNull(extension);
        assertEquals("example", savedExtension.getName());
        // test modify extension without changing name
        try {
            m_openAcdContext.saveExtension(savedExtension);
        } catch (NameInUseException ex) {
            fail();
        }

        // test get extension by id
        Integer id = savedExtension.getId();
        OpenAcdExtension extensionById = m_openAcdContext.getExtensionById(id);
        assertNotNull(extensionById);
        assertEquals("example", extensionById.getName());

        // test saved conditions and actions
        Set<FreeswitchCondition> conditions = extensionById.getConditions();
        assertEquals(1, conditions.size());
        for (FreeswitchCondition condition : conditions) {
            assertEquals(8, condition.getActions().size());
            List<FreeswitchAction> actions = new LinkedList<FreeswitchAction>();
            actions.addAll(condition.getActions());
            assertEquals("answer", actions.get(0).getApplication());
            assertEquals("", actions.get(0).getData());
            assertEquals("set", actions.get(1).getApplication());
            assertEquals("domain_name=$${domain}", actions.get(1).getData());
            assertEquals("set", actions.get(2).getApplication());
            assertEquals("brand=1", actions.get(2).getData());
            assertEquals("set", actions.get(3).getApplication());
            assertEquals("queue=Sales", actions.get(3).getData());
            assertEquals("set", actions.get(4).getApplication());
            assertEquals("allow_voicemail=true", actions.get(4).getData());
            assertEquals("erlang_sendmsg", actions.get(5).getApplication());
            assertEquals("freeswitch_media_manager testme@192.168.1.1 inivr ${uuid}", actions.get(5).getData());
            assertEquals("playback", actions.get(6).getApplication());
            assertEquals("/usr/share/www/doc/stdprompts/welcome.wav", actions.get(6).getData());
            assertEquals("erlang", actions.get(7).getApplication());
            assertEquals("freeswitch_media_manager:! testme@192.168.1.1", actions.get(7).getData());
        }

        // test remove extension
        assertEquals(1, m_openAcdContext.getFreeswitchExtensions().size());
        m_openAcdContext.removeExtensions(Collections.singletonList(id));
        assertEquals(0, m_openAcdContext.getFreeswitchExtensions().size());
    }

    public void testOpenAcdExtensionAliasProvider() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        SipxFreeswitchService service = new MockSipxFreeswitchService();
        service.setBeanId(SipxFreeswitchService.BEAN_ID);
        IMocksControl mc = EasyMock.createControl();
        LocationsManager locationsManager = mc.createMock(LocationsManager.class);
        locationsManager.getLocationsForService(service);
        Location location = new Location();
        location.setAddress("example.org");
        List<Location> locations = new ArrayList<Location>();
        locations.add(location);
        mc.andReturn(locations);
        mc.replay();
        service.setLocationsManager(locationsManager);

        SipxServiceManager sm = TestUtil.getMockSipxServiceManager(true, service);
        m_openAcdContextImpl.setSipxServiceManager(sm);

        OpenAcdExtension extension = DefaultContextConfigurationTest.createOpenAcdExtension("provider");
        m_openAcdContextImpl.saveExtension(extension);
        assertEquals(1, m_openAcdContextImpl.getBeanIdsOfObjectsWithAlias("provider").size());

        assertFalse(m_openAcdContextImpl.isAliasInUse("test"));
        assertTrue(m_openAcdContextImpl.isAliasInUse("provider"));

        Collection<AliasMapping> mappings = m_openAcdContextImpl.getAliasMappings();
        assertEquals(1, mappings.size());
        for (AliasMapping mapping : mappings) {
            assertEquals("openacd", mapping.getRelation());
            assertEquals("provider@example.org", mapping.getIdentity());
            assertEquals("sip:provider@example.org:50", mapping.getContact());
        }
    }

    public void setOpenAcdContext(OpenAcdContext openAcdContext) {
        m_openAcdContext = openAcdContext;
    }

    public void setOpenAcdContextImpl(OpenAcdContextImpl openAcdContext) {
        m_openAcdContextImpl = openAcdContext;
    }

    private class MockSipxFreeswitchService extends SipxFreeswitchService {
        @Override
        public int getFreeswitchSipPort() {
            return 50;
        }
    }
}
