/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin;

import java.util.Collection;

import org.sipfoundry.sipxconfig.alias.AliasManager;
import org.sipfoundry.sipxconfig.service.LoggingEntity;
import org.sipfoundry.sipxconfig.service.SipxRegistrarService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class LoggingManagerImplTestIntegration extends IntegrationTestCase {
    private static final String TEST_SETTING = "call-pick-up/SIP_REDIRECT.100-PICKUP.DIRECTED_CALL_PICKUP_CODE";
    private LoggingManager m_out;
    private SipxServiceManager m_sipxServiceManager;
    private AliasManager m_aliasManager;

    public void testGetLoggingEntities() throws Exception {
        loadDataSetXml("admin/commserver/seedLocations.xml");
        SipxRegistrarService registrarService = (SipxRegistrarService) m_sipxServiceManager
                .getServiceByBeanId(SipxRegistrarService.BEAN_ID);
        registrarService.getSettings().getSetting(TEST_SETTING).setValue("999");
        registrarService.setSipxServiceManager(m_sipxServiceManager);
        registrarService.setAliasManager(m_aliasManager);
        m_sipxServiceManager.storeService(registrarService);
        Collection<LoggingEntity> loggingEntities = m_out.getLoggingEntities();
        assertNotNull(loggingEntities);

        boolean foundRegistrarService = false;
        for (LoggingEntity loggingEntity : loggingEntities) {
            SipxService logEnabledService = (SipxService) loggingEntity;
            if (logEnabledService == null) {
                fail("Found null LoggingEntity");
            }
            if (logEnabledService.getBeanId().equals(SipxRegistrarService.BEAN_ID)) {
                Setting testSetting = logEnabledService.getSettings().getSetting(TEST_SETTING);
                assertEquals("999", testSetting.getValue());
                foundRegistrarService = true;
            }
        }

        assertTrue(foundRegistrarService);
    }

    public void setLoggingManager(LoggingManager loggingManager) {
        m_out = loggingManager;
    }

    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    public void setAliasManager(AliasManager aliasManager) {
        m_aliasManager = aliasManager;
    }    
}
