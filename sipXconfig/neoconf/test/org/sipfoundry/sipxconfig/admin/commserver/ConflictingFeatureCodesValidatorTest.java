/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.Arrays;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.service.SipxPresenceService;
import org.sipfoundry.sipxconfig.service.SipxRegistrarService;
import org.sipfoundry.sipxconfig.setting.Setting;

public class ConflictingFeatureCodesValidatorTest extends TestCase {

    private SipxRegistrarService m_registrarService;
    private SipxPresenceService m_presenceService;
    private ConflictingFeatureCodeValidator m_out;

    public void setUp() throws Exception {
        m_out = new ConflictingFeatureCodeValidator();

        m_registrarService = new SipxRegistrarService();
        m_registrarService.setModelDir("sipxregistrar");
        m_registrarService.setModelName("sipxregistrar.xml");
        m_registrarService.setModelFilesContext(TestHelper.getModelFilesContext());
        IMocksControl domainManagerControl = EasyMock.createControl();
        DomainManager domainManager = domainManagerControl.createMock(DomainManager.class);
        domainManager.getDomain();
        EasyMock.expectLastCall().andReturn(new Domain()).anyTimes();
        EasyMock.replay(domainManager);
        m_registrarService.setDomainManager(domainManager);

        m_presenceService = new SipxPresenceService();
        m_presenceService.setModelDir("sipxpresence");
        m_presenceService.setModelName("sipxpresence.xml");
        m_presenceService.setModelFilesContext(TestHelper.getModelFilesContext());
    }

    public void testValidate() {
        Setting settings = m_presenceService.getSettings().copy();
        m_out.validate(settings);

        settings.getSetting("presence-config/SIP_PRESENCE_SIGN_OUT_CODE").setValue("*123");
        settings.getSetting("presence-config/SIP_PRESENCE_SIGN_IN_CODE").setValue("*123");
        try {
            m_out.validate(settings);
            fail();
        } catch (ConflictingFeatureCodeException expected) {
            assertTrue(true);
        }

        settings.getSetting("presence-config/SIP_PRESENCE_SIGN_IN_CODE").setValue("*121");
        try {
            m_out.validate(settings);
        } catch (ConflictingFeatureCodeException expected) {
            fail();
        }
    }

    public void testValidateWithMultipleSettingSets() {
        Setting presenceSettings = m_presenceService.getSettings().copy();
        Setting registrarSettings = m_registrarService.getSettings().copy();
        m_out.validate(Arrays.asList(new Setting[] {presenceSettings, registrarSettings}));

        presenceSettings.getSetting("presence-config/SIP_PRESENCE_SIGN_IN_CODE").setValue("*123");
        registrarSettings.getSetting("call-pick-up/SIP_REDIRECT.100-PICKUP.DIRECTED_CALL_PICKUP_CODE").setValue("*123");
        try {
            m_out.validate(Arrays.asList(new Setting[] {presenceSettings, registrarSettings}));
            fail();
        } catch (ConflictingFeatureCodeException expected) {
            assertTrue(true);
        }

        presenceSettings.getSetting("presence-config/SIP_PRESENCE_SIGN_IN_CODE").setValue("*121");
        try {
            m_out.validate(Arrays.asList(new Setting[] {presenceSettings, registrarSettings}));
        } catch (ConflictingFeatureCodeException expected) {
            fail();
        }
    }
}
