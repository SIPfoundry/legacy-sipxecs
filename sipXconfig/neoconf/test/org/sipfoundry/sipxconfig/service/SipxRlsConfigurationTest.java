package org.sipfoundry.sipxconfig.service;

import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;
import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.setting.Setting;

public class SipxRlsConfigurationTest extends SipxServiceTestBase {
    
    public void testWrite() throws Exception {
       SipxRlsConfiguration config = new SipxRlsConfiguration();
       config.setVelocityEngine(TestHelper.getVelocityEngine());
       config.setTemplate("sipxrls/sipxrls-config.vm");
       
       SipxRlsService service = new SipxRlsService();
       service.setRlsPort("5140");
       Setting settings = TestHelper.loadSettings("sipxrls/sipxrls.xml");
       service.setSettings(settings);
       initCommonAttributes(service);
       
       Setting parkSettings = service.getSettings().getSetting("rls-config");
       parkSettings.getSetting("SIP_RLS_LOG_LEVEL").setValue("WARN");
       
       config.generate(service);
       StringWriter actualConfigWriter = new StringWriter();
       config.write(actualConfigWriter);
       
       
       Reader referenceConfigReader = new InputStreamReader(getClass().getResourceAsStream("expected-rls-config"));
       String referenceConfig = IOUtils.toString(referenceConfigReader);

       Reader actualConfigReader = new StringReader(actualConfigWriter.toString());
       String actualConfig = IOUtils.toString(actualConfigReader);

       assertEquals(referenceConfig, actualConfig);
    }
}
