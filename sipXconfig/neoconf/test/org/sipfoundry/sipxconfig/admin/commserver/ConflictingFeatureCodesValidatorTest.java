package org.sipfoundry.sipxconfig.admin.commserver;

import java.io.InputStream;
import java.util.Arrays;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.service.SipxRegistrarService;
import org.sipfoundry.sipxconfig.setting.Setting;

public class ConflictingFeatureCodesValidatorTest extends TestCase {
    
    private SipxServer m_server;
    private SipxRegistrarService m_registrarService;
    private ConflictingFeatureCodeValidator m_out;
    
    public void setUp() throws Exception {
        m_out = new ConflictingFeatureCodeValidator();
        m_server = SipxServerTest.setUpSipxServer();
        m_registrarService = new SipxRegistrarService();
        m_registrarService.setModelDir("sipxregistrar");
        m_registrarService.setModelName("sipxregistrar.xml");
        m_registrarService.setModelFilesContext(TestHelper.getModelFilesContext());

        // set up files for sipxserver to read - copied from SipxServerTest
        InputStream configDefs = SipxServerTest.class.getResourceAsStream("config.defs");
        TestHelper
                .copyStreamToDirectory(configDefs, TestHelper.getTestDirectory(), "config.defs");
        InputStream sipxpresence = SipxServerTest.class
                .getResourceAsStream("sipxpresence-config.test.in");
        TestHelper.copyStreamToDirectory(sipxpresence, TestHelper.getTestDirectory(),
                "sipxpresence-config.in");
        InputStream registrar = SipxServerTest.class
                .getResourceAsStream("registrar-config.test.in");
        TestHelper.copyStreamToDirectory(registrar, TestHelper.getTestDirectory(),
                "registrar-config");
        // we read server location from sipxpresence-config
        sipxpresence = SipxServerTest.class.getResourceAsStream("sipxpresence-config.test.in");
        TestHelper.copyStreamToDirectory(sipxpresence, TestHelper.getTestDirectory(),
                "sipxpresence-config");
        registrar = SipxServerTest.class.getResourceAsStream("registrar-config.test.in");
        TestHelper.copyStreamToDirectory(registrar, TestHelper.getTestDirectory(),
                "registrar-config.in");
    }

    public void testValidate() {
        Setting settings = m_server.getSettings().copy();        
        m_out.validate(settings);
        
        settings.getSetting("presence/SIP_PRESENCE_SIGN_OUT_CODE").setValue("*123");
        settings.getSetting("presence/SIP_PRESENCE_SIGN_IN_CODE").setValue("*123");
        try {
            m_out.validate(settings);
            fail();
        } catch (ConflictingFeatureCodeException expected) {
            assertTrue(true);
        }
        
        settings.getSetting("presence/SIP_PRESENCE_SIGN_IN_CODE").setValue("*121");
        try {
            m_out.validate(settings);
        } catch (ConflictingFeatureCodeException expected) {
            fail();
        }
    }
    
    public void testValidateWithMultipleSettingSets() {
        Setting serverSettings = m_server.getSettings().copy();
        Setting registrarSettings = m_registrarService.getSettings().copy();
        m_out.validate(Arrays.asList(new Setting[] {serverSettings, registrarSettings}));
        
        serverSettings.getSetting("presence/SIP_PRESENCE_SIGN_IN_CODE").setValue("*123");
        registrarSettings.getSetting("call-pick-up/SIP_REDIRECT.180-PICKUP.DIRECTED_CALL_PICKUP_CODE").setValue("*123");
        try {
            m_out.validate(Arrays.asList(new Setting[] {serverSettings, registrarSettings}));
            fail();
        } catch (ConflictingFeatureCodeException expected) {
            assertTrue(true);
        }
        
        serverSettings.getSetting("presence/SIP_PRESENCE_SIGN_IN_CODE").setValue("*121");
        try {
            m_out.validate(Arrays.asList(new Setting[] {serverSettings, registrarSettings}));
        } catch (ConflictingFeatureCodeException expected) {
            fail();
        }
    }
}
