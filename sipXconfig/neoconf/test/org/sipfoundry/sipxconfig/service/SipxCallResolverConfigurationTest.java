package org.sipfoundry.sipxconfig.service;

import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;
import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.setting.Setting;

public class SipxCallResolverConfigurationTest extends SipxServiceTestBase {

    public void testWrite() throws Exception {
        SipxCallResolverConfiguration out = new SipxCallResolverConfiguration();
        out.setVelocityEngine(TestHelper.getVelocityEngine());
        out.setTemplate("sipxcallresolver/callresolver-config.vm");

        SipxCallResolverService callResolverService = new SipxCallResolverService();
        initCommonAttributes(callResolverService);
        Setting settings = TestHelper.loadSettings("sipxcallresolver/sipxcallresolver.xml");
        callResolverService.setSettings(settings);

        Setting callresolverSettings = callResolverService.getSettings().getSetting("callresolver");
        callresolverSettings.getSetting("SIP_CALLRESOLVER_PURGE").setValue("DISABLE");
        callresolverSettings.getSetting("SIP_CALLRESOLVER_PURGE_AGE_CDR").setValue("40");
        callresolverSettings.getSetting("SIP_CALLRESOLVER_PURGE_AGE_CSE").setValue("10");
        callresolverSettings.getSetting("SIP_CALLRESOLVER_LOG_LEVEL").setValue("CRIT");

        callResolverService.setAgentPort(8090);

        out.generate(callResolverService);

        Reader referenceConfigReader = new InputStreamReader(SipxProxyConfigurationTest.class
                .getResourceAsStream("expected-callresolver-config"));
        String expectedConfig = IOUtils.toString(referenceConfigReader);

        StringWriter actualConfigWriter = new StringWriter();
        out.write(actualConfigWriter, null);

        Reader actualConfigReader = new StringReader(actualConfigWriter.toString());
        String actualConfig = IOUtils.toString(actualConfigReader);

        assertEquals(expectedConfig, actualConfig);
    }
}
