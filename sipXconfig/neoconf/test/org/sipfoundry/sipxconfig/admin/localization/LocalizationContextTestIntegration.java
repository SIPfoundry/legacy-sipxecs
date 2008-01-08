/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.sipxconfig.admin.localization;

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.domain.DomainManager;

public class LocalizationContextTestIntegration extends IntegrationTestCase {

    private LocalizationContext m_localizationContext;
    private DomainManager m_domainManager;

    public void testUpdateRegion() throws Exception {
        assertEquals(-1, m_localizationContext.updateRegion("PL"));
    }

    public void testUpdateLanguage() throws Exception {
        m_domainManager.initialize();
        assertEquals(1, m_localizationContext.updateLanguage("stdprompts_pl"));
        flush();
        assertEquals(1, getConnection().getRowCount("localization", "where language = 'pl'"));
    }

    public void testDefaults() {
        assertEquals("na", m_localizationContext.getCurrentRegionId());
        assertEquals("en", m_localizationContext.getCurrentLanguage());
    }

    public void setLocalizationContext(LocalizationContext localizationContext) {
        m_localizationContext = localizationContext;
    }

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }
}
