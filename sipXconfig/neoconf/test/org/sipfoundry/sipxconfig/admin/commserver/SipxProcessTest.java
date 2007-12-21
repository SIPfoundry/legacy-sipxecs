package org.sipfoundry.sipxconfig.admin.commserver;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import junit.framework.TestCase;

public class SipxProcessTest extends TestCase {

    SipxProcessContext m_sipxProcessContext;
    IMocksControl m_contextCtrl;
    SipxProcess m_process;

    protected void setUp() throws Exception {
        m_contextCtrl = EasyMock.createControl();
        m_sipxProcessContext = m_contextCtrl.createMock(SipxProcessContext.class);
        m_process = new SipxProcess();
        Location location = new Location();
        Location[] locations = new Location[] {
            location
        };
        ServiceStatus servStatus1 = new ServiceStatus(new Process("nameProcess1"),
                ServiceStatus.Status.STARTED);
        ServiceStatus servStatus2 = new ServiceStatus(new Process("nameProcess2"),
                ServiceStatus.Status.STARTED);
        ServiceStatus[] serviceStatusList = new ServiceStatus[] {
            servStatus1, servStatus2
        };
        m_sipxProcessContext.getLocations();
        m_contextCtrl.andReturn(locations);
        m_contextCtrl.anyTimes();
        m_sipxProcessContext.getStatus(location);
        m_contextCtrl.andReturn(serviceStatusList);
        m_contextCtrl.anyTimes();
        m_contextCtrl.replay();
        m_process.setSipxProcessContext(m_sipxProcessContext);
    }

    public void testProcessEnabled() {
        m_process.setProcessName("nameProcess1");
        assertTrue(m_process.isEnabled());
        m_contextCtrl.verify();
    }

    public void testProcessDisabled() {
        m_process.setProcessName("nameProcessDummy");
        assertFalse(m_process.isEnabled());
        m_contextCtrl.verify();
    }
}
