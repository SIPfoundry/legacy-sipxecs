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

public class LocalizationContextTestIntegration extends IntegrationTestCase {

    LocalizationContext m_localizationContext;

    public void testUpdateRegion() throws Exception {
        assertEquals(-1, m_localizationContext.updateRegion("PL"));
    }

    public void testUpdateLanguage() throws Exception {
        assertEquals(1, m_localizationContext.updateLanguage("pl"));
        flush();
        assertEquals(1, getConnection().getRowCount("localization", "where language = 'pl'"));
    }

    public void testDefaults() {
        assertEquals("na", m_localizationContext.getCurrentRegionId());
        assertEquals("en", m_localizationContext.getCurrentLanguageId());
    }

    public void setLocalizationContext(LocalizationContext localizationContext) {
        m_localizationContext = localizationContext;
    }
}
