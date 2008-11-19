package org.sipfoundry.sipxconfig.service;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.setting.Setting;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class SipxCallResolverConfigurationTest extends SipxServiceTestBase {

    public void testWrite() throws Exception {
        LocationsManager locationManager = createMock(LocationsManager.class);
        locationManager.getPrimaryLocation();
        expectLastCall().andReturn(createDefaultLocation());

        SipxCallResolverService callResolverService = new SipxCallResolverService();
        initCommonAttributes(callResolverService);
        Setting settings = TestHelper.loadSettings("sipxcallresolver/sipxcallresolver.xml");
        callResolverService.setSettings(settings);
        callResolverService.setLocationManager(locationManager);

        Setting callresolverSettings = callResolverService.getSettings().getSetting("callresolver");
        callresolverSettings.getSetting("SIP_CALLRESOLVER_PURGE").setValue("DISABLE");
        callresolverSettings.getSetting("SIP_CALLRESOLVER_PURGE_AGE_CDR").setValue("40");
        callresolverSettings.getSetting("SIP_CALLRESOLVER_PURGE_AGE_CSE").setValue("10");
        callresolverSettings.getSetting("SIP_CALLRESOLVER_LOG_LEVEL").setValue("CRIT");

        callResolverService.setAgentPort(8090);

        SipxServiceManager sipxServiceManager = createMock(SipxServiceManager.class);
        sipxServiceManager.getServiceByBeanId(SipxCallResolverService.BEAN_ID);
        expectLastCall().andReturn(callResolverService).atLeastOnce();
        replay(locationManager, sipxServiceManager);

        SipxCallResolverConfiguration out = new SipxCallResolverConfiguration();
        out.setSipxServiceManager(sipxServiceManager);
        out.setTemplate("sipxcallresolver/callresolver-config.vm");

        assertCorrectFileGeneration(out, "expected-callresolver-config");

        verify(locationManager, sipxServiceManager);
    }
}
