/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.sipxconfig.service;

import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;
import java.io.StringWriter;
import java.util.Arrays;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.setting.Setting;

public class SipxRegistrarConfigurationTest extends SipxServiceTestBase {

    public void testWrite() throws Exception {
        SipxRegistrarConfiguration out = new SipxRegistrarConfiguration();
        out.setVelocityEngine(TestHelper.getVelocityEngine());
        out.setTemplate("sipxregistrar/registrar-config.vm");

        SipxRegistrarService registrarService = new SipxRegistrarService();
        Setting settings = TestHelper.loadSettings("sipxregistrar/sipxregistrar.xml");
        registrarService.setSettings(settings);
        initCommonAttributes(registrarService);
        
        setSettingValuesForGroup(registrarService, "logging", new String[] {
            "SIP_REGISTRAR_LOG_LEVEL"
        }, new String[] {
            "WARNING"
        });
        setSettingValuesForGroup(registrarService, "call-pick-up", new String[] {
            "SIP_REDIRECT.180-PICKUP.DIRECTED_CALL_PICKUP_CODE",
            "SIP_REDIRECT.180-PICKUP.CALL_RETRIEVE_CODE",
            "SIP_REDIRECT.180-PICKUP.CALL_PICKUP_WAIT"
        }, new String[] {
            "*42", "*43", "15.0"
        });
        setSettingValuesForGroup(registrarService, "isn", new String[] {
            "SIP_REDIRECT.190-ISN.BASE_DOMAIN", "SIP_REDIRECT.190-ISN.PREFIX"
        }, new String[] {
            "myisndomain.org", null
        });
        setSettingValuesForGroup(registrarService, "enum", new String[] {
            "SIP_REDIRECT.200-ENUM.BASE_DOMAIN", "SIP_REDIRECT.200-ENUM.DIAL_PREFIX",
            "SIP_REDIRECT.200-ENUM.ADD_PREFIX", "SIP_REDIRECT.200-ENUM.PREFIX_PLUS"
        }, new String[] {
            "myenumdomain.org", null, "*66", "Y"
        });
        
        registrarService.setMediaServerSipSrvOrHostport("media.example.org");
        registrarService.setOrbitServerSipSrvOrHostport("orbit.example.org");
        registrarService.setProxyServerSipHostport("proxy.example.org");
        registrarService.setVoicemailHttpsPort("443");
        registrarService.setRegistrarSipPort("5070");
        registrarService.setRegistrarEventSipPort("5075");
        registrarService.setRegistrarDomainAliases(Arrays.asList(new String[] {"another.example.org"}));

        out.generate(registrarService);

        StringWriter actualConfigWriter = new StringWriter();
        out.write(actualConfigWriter);

        Reader referenceConfigReader = new InputStreamReader(SipxProxyConfigurationTest.class
                .getResourceAsStream("expected-registrar-config"));
        String referenceConfig = IOUtils.toString(referenceConfigReader);

        Reader actualConfigReader = new StringReader(actualConfigWriter.toString());
        String actualConfig = IOUtils.toString(actualConfigReader);

        assertEquals(referenceConfig, actualConfig);
    }

    private void setSettingValuesForGroup(SipxRegistrarService registrarService, String group,
            String[] settingNames, String[] values) {
        for (int i = 0; i < settingNames.length; i++) {
            registrarService.getSettings().getSetting(group).getSetting(settingNames[i])
                    .setValue(values[i]);
        }
    }
}
