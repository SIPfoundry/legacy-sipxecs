package org.sipfoundry.sipxconfig.admin.commserver;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessModel.ProcessName;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class SipxProcessTest extends TestCase {

    SipxProcessContext m_sipxProcessContext;
    LocationsManager m_locationsManager;

    protected void setUp() throws Exception {
        m_sipxProcessContext = createMock(SipxProcessContext.class);
        m_locationsManager = createMock(LocationsManager.class);
        Location[] locations = new Location[] {
            new Location()
        };
        ServiceStatus servStatus1 = new ServiceStatus(new Process(ProcessName.ACD_SERVER), ServiceStatus.Status.STARTED);
        ServiceStatus servStatus2 = new ServiceStatus(new Process(ProcessName.PRESENCE_SERVER),
                ServiceStatus.Status.STOPPED);
        ServiceStatus[] serviceStatusList = new ServiceStatus[] {
            servStatus1, servStatus2
        };
        expect(m_locationsManager.getLocations()).andReturn(locations).once();
        expect(m_sipxProcessContext.getStatus(locations[0])).andReturn(serviceStatusList).once();

        replay(m_sipxProcessContext, m_locationsManager);
    }

    protected void tearDown() throws Exception {
        verify(m_sipxProcessContext, m_locationsManager);
    }

    public void testProcessEnabled() {
        SipxProcess process = new SipxProcess(m_locationsManager, m_sipxProcessContext, ProcessName.ACD_SERVER);
        assertTrue(process.isEnabled());

        // make status is only computed once
        assertTrue(process.isEnabled());
    }

    public void testProcessDisabled() {
        SipxProcess process = new SipxProcess(m_locationsManager, m_sipxProcessContext, ProcessName.PRESENCE_SERVER);
        assertFalse(process.isEnabled());
    }

    public void testProcessNotInstalled() {
        SipxProcess process = new SipxProcess(m_locationsManager, m_sipxProcessContext, ProcessName.PARK_SERVER);
        assertFalse(process.isEnabled());
    }
}
