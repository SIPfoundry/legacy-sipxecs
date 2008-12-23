package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.Arrays;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxRegistrarService;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.sipfoundry.sipxconfig.admin.commserver.ServiceStatus.Status.Disabled;
import static org.sipfoundry.sipxconfig.admin.commserver.ServiceStatus.Status.Running;

public class SipxProcessTest extends TestCase {

    SipxProcessContext m_sipxProcessContext;
    LocationsManager m_locationsManager;

    @Override
    protected void setUp() throws Exception {
        m_sipxProcessContext = createMock(SipxProcessContext.class);
        m_locationsManager = createMock(LocationsManager.class);

        Location location = new Location();
        SipxRegistrarService sipxRegistrarService = new SipxRegistrarService();
        sipxRegistrarService.setModelId(SipxRegistrarService.BEAN_ID);
        SipxProxyService sipxProxyService = new SipxProxyService();
        sipxProxyService.setModelId(SipxProxyService.BEAN_ID);
        location.setServiceDefinitions(Arrays.asList(sipxRegistrarService, sipxProxyService));

        Location[] locations = new Location[] {
            location
        };

        ServiceStatus servStatus1 = new ServiceStatus("ACDServer", Running);
        ServiceStatus servStatus2 = new ServiceStatus("PresenceServer", Disabled);
        ServiceStatus[] serviceStatusList = new ServiceStatus[] {
            servStatus1, servStatus2
        };
        expect(m_locationsManager.getLocations()).andReturn(locations).once();
        expect(m_sipxProcessContext.getStatus(locations[0], false)).andReturn(serviceStatusList).once();

        replay(m_sipxProcessContext, m_locationsManager);
    }

    @Override
    protected void tearDown() throws Exception {
        verify(m_sipxProcessContext, m_locationsManager);
    }

    public void testProcessEnabled() {
        SipxProcess process = new SipxProcess(m_locationsManager, m_sipxProcessContext, "ACDServer");
        assertTrue(process.isEnabled());

        // make status is only computed once
        assertTrue(process.isEnabled());
    }

    public void testProcessDisabled() {
        SipxProcess process = new SipxProcess(m_locationsManager, m_sipxProcessContext, "PresenceServer");
        assertFalse(process.isEnabled());
    }

    public void testProcessNotInstalled() {
        SipxProcess process = new SipxProcess(m_locationsManager, m_sipxProcessContext, "ParkServer");
        assertFalse(process.isEnabled());
    }
}
