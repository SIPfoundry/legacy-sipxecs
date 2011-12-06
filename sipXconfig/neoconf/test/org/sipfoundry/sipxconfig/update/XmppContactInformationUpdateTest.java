/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.update;

import java.util.Arrays;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.admin.commserver.ContactInfoChangeApi;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.ServiceStatus;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;

public class XmppContactInformationUpdateTest extends TestCase {

    private ApiProvider<ContactInfoChangeApi> m_contactInfoChangeApiProvider;
    private SipxServiceManager m_sipxServiceManager;
    private LocationsManager m_locationsManager;
    private SipxProcessContext m_sipxProcessContext;
    private ContactInfoChangeApi m_contactInfoChangeApi;
    private Location m_location;
    private SipxService m_service;
    private XmppContactInformationUpdate m_xmppContactInformationUpdate;

    @Override
    public void setUp() {
        m_service = new SipxService() {
        };

        m_location = new Location();
        m_location.setFqdn("test.fqdn");

        m_sipxServiceManager = EasyMock.createMock(SipxServiceManager.class);
        m_sipxServiceManager.getServiceByBeanId("sipxOpenfireService");
        EasyMock.expectLastCall().andReturn(m_service);

        m_locationsManager = EasyMock.createMock(LocationsManager.class);
        m_sipxProcessContext = EasyMock.createMock(SipxProcessContext.class);
        m_contactInfoChangeApi = EasyMock.createMock(ContactInfoChangeApi.class);
        m_contactInfoChangeApiProvider = EasyMock.createMock(ApiProvider.class);

        m_xmppContactInformationUpdate = new XmppContactInformationUpdate();
        m_xmppContactInformationUpdate.setContactInfoChangeApiProvider(m_contactInfoChangeApiProvider);
        m_xmppContactInformationUpdate.setLocationsManager(m_locationsManager);
        m_xmppContactInformationUpdate.setSipxProcessContext(m_sipxProcessContext);
        m_xmppContactInformationUpdate.setSipxServiceManager(m_sipxServiceManager);
    }

    public void testNotifyChangeServiceRunning() {
        User user = new User() {
            @Override
            public Object getSettingTypedValue(String path) {
                if ("im/im-account" == path) {
                    return true;
                }
                return getSettings().getSetting(path).getTypedValue();
            }
        };
        user.setUserName("userName");
        user.setImId("imId");

        m_locationsManager.getLocationsForService(m_service);
        EasyMock.expectLastCall().andReturn(Arrays.asList( new Location [] {m_location}));

        m_sipxProcessContext.getStatus(m_location, m_service);
        EasyMock.expectLastCall().andReturn(ServiceStatus.Status.Running);

        m_contactInfoChangeApiProvider.getApi(m_location.getXmppContactInfoUpdateUrl());
        EasyMock.expectLastCall().andReturn(m_contactInfoChangeApi);

        m_contactInfoChangeApi.notifyContactChange(user.getImId());
        EasyMock.expectLastCall();

        EasyMock.replay(m_sipxServiceManager, m_locationsManager, m_sipxProcessContext,
                m_contactInfoChangeApiProvider, m_contactInfoChangeApi);

        m_xmppContactInformationUpdate.notifyChange(user);
        EasyMock.verify(m_sipxServiceManager, m_locationsManager, m_sipxProcessContext,
                m_contactInfoChangeApiProvider, m_contactInfoChangeApi);
    }

    public void testNotifyChangeServiceNotRunning() {
        User user = new User() {
            @Override
            public Object getSettingTypedValue(String path) {
                if ("im/im-account" == path) {
                    return true;
                }
                return getSettings().getSetting(path).getTypedValue();
            }
        };
        user.setUserName("userName");
        user.setImId("imId");

        m_locationsManager.getLocationsForService(m_service);
        EasyMock.expectLastCall().andReturn(Arrays.asList( new Location [] {m_location}));

        m_sipxProcessContext.getStatus(m_location, m_service);
        EasyMock.expectLastCall().andReturn(ServiceStatus.Status.ResourceRequired);

        EasyMock.replay(m_sipxServiceManager, m_locationsManager, m_sipxProcessContext,
                m_contactInfoChangeApiProvider, m_contactInfoChangeApi);

        m_xmppContactInformationUpdate.notifyChange(user);
        EasyMock.verify(m_sipxServiceManager, m_locationsManager, m_sipxProcessContext,
                m_contactInfoChangeApiProvider, m_contactInfoChangeApi);
    }

    public void testNotifyChangeImDisabled() {
        User user = new User() {
            @Override
            public Object getSettingTypedValue(String path) {
                if ("im/im-account" == path) {
                    return false;
                }
                return getSettings().getSetting(path).getTypedValue();
            }
        };
        user.setUserName("userName");
        user.setImId("imId");

        EasyMock.replay(m_sipxServiceManager, m_locationsManager, m_sipxProcessContext,
                m_contactInfoChangeApiProvider, m_contactInfoChangeApi);

        m_xmppContactInformationUpdate.notifyChange(user);
        EasyMock.verify(m_sipxServiceManager, m_locationsManager, m_sipxProcessContext,
                m_contactInfoChangeApiProvider, m_contactInfoChangeApi);
    }
}