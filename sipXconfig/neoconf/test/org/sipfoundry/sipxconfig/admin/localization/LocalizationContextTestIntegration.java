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
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class LocalizationContextTestIntegration extends IntegrationTestCase {

    // Object Under Test
    private LocalizationContext m_out;
    private ServiceConfigurator m_origServiceConfigurator;
    private LocalizationContextImpl m_localizationContextImpl;

    public void testUpdateRegion() throws Exception {
        assertEquals(-1, m_out.updateRegion("PL"));
    }

    public void testUpdateLanguage() throws Exception {
        ServiceConfigurator sc = createMock(ServiceConfigurator.class);
        sc.initLocations();
        replay(sc);

        modifyContext(m_localizationContextImpl, "serviceConfigurator", m_origServiceConfigurator, sc);

        assertEquals(1, m_out.updateLanguage("stdprompts_pl"));
        flush();
        assertEquals(1, getConnection().getRowCount("localization", "where language = 'pl'"));

        verify(sc);
    }

    public void testDefaults() {
        assertEquals("na", m_out.getCurrentRegionId());
        assertEquals("en", m_out.getCurrentLanguage());
    }

    public void setLocalizationContext(LocalizationContext localizationContext) {
        m_out = localizationContext;
    }

    public void setLocalizationContextImpl(LocalizationContextImpl localizationContextImpl) {
        m_localizationContextImpl = localizationContextImpl;
    }

    public void setServiceConfigurator(ServiceConfigurator serviceConfigurator) {
        m_origServiceConfigurator = serviceConfigurator;
    }
}
