/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin.commserver;

import java.util.Collection;

import junit.framework.TestCase;

import org.apache.hivemind.util.PropertyUtils;
import org.apache.tapestry.test.Creator;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Command;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;

import static org.easymock.EasyMock.*;

public class ServicesTableTest extends TestCase {

    private ServicesTable m_out;
    private Location m_location;
    private SipxProcessContext m_sipxProcessContext;

    @Override
    public void setUp() {
        Creator pageCreator = new Creator();
        m_out = (ServicesTable) pageCreator.newInstance(ServicesTable.class);

        SipxProxyService proxyService = new SipxProxyService();
        proxyService.setBeanName(SipxProxyService.BEAN_ID);
        proxyService.setProcessName("SIPXProxy");
        m_location = new Location();
        m_location.addService(proxyService);
        PropertyUtils.write(m_out, "serviceLocation", m_location);

        m_sipxProcessContext = EasyMock.createMock(SipxProcessContext.class);
        PropertyUtils.write(m_out, "sipxProcessContext", m_sipxProcessContext);

        SelectMap selectMap = new SelectMap();
        selectMap.setSelected(SipxProxyService.BEAN_ID, true);
        PropertyUtils.write(m_out, "selections", selectMap);

        SipxServiceManager serviceManager = createMock(SipxServiceManager.class);
        serviceManager.getServiceByBeanId(SipxProxyService.BEAN_ID);
        expectLastCall().andReturn(proxyService).atLeastOnce();
        replay(serviceManager);
        PropertyUtils.write(m_out, "sipxServiceManager", serviceManager);
    }

    public void testRestart() {
        configureSipxProcessContext(Command.RESTART);
        m_out.restart();
        EasyMock.verify(m_sipxProcessContext);
    }

    public void testStart() {
        configureSipxProcessContext(Command.START);
        m_out.start();
        EasyMock.verify(m_sipxProcessContext);
    }

    public void testStop() {
        configureSipxProcessContext(Command.STOP);
        m_out.stop();
        EasyMock.verify(m_sipxProcessContext);
    }

    public void testRemoveService() {
        LocationsManager locationsManager = EasyMock.createMock(LocationsManager.class);
        locationsManager.storeLocation(m_location);
        EasyMock.expectLastCall();
        EasyMock.replay(locationsManager);
        PropertyUtils.write(m_out, "locationsManager", locationsManager);

        configureSipxProcessContext(Command.STOP);
        m_out.removeService();
        assertTrue(m_location.getServices().isEmpty());
        EasyMock.verify(m_sipxProcessContext, locationsManager);
    }

    private void configureSipxProcessContext(Command expectedCommand) {
        m_sipxProcessContext.manageServices(EasyMock.eq(m_location), EasyMock.isA(Collection.class),
                EasyMock.eq(expectedCommand));
        EasyMock.expectLastCall();
        EasyMock.replay(m_sipxProcessContext);
    }
}
