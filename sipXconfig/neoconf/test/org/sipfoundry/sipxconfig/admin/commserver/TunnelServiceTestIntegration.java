package org.sipfoundry.sipxconfig.admin.commserver;

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.device.BeanFactoryModelSource;
import org.sipfoundry.sipxconfig.service.SipxService;

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
