package org.sipfoundry.sipxconfig.tunnel;

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.device.BeanFactoryModelSource;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.tunnel.TunnelService;

public class TunnelServiceTestIntegration extends IntegrationTestCase  {
    
    private BeanFactoryModelSource<SipxService> m_sipxServiceModelSource;

    public BeanFactoryModelSource<SipxService> getSipxServiceModelSource() {
        return m_sipxServiceModelSource;
    }

    public void setSipxServiceModelSource(BeanFactoryModelSource<SipxService> sipxServiceModelSource) {
        m_sipxServiceModelSource = sipxServiceModelSource;
    }

    public void testFindService() {
        assertNotNull(m_sipxServiceModelSource.getModel(TunnelService.BEAN_ID));
    }
}
