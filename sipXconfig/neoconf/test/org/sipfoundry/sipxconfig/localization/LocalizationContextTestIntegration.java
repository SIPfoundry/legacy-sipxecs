/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.localization;


import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.isA;
import static org.easymock.EasyMock.replay;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.dialplan.DialPlan;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class LocalizationContextTestIntegration extends IntegrationTestCase {
    private LocalizationContext m_l8n;

    public void testUpdateRegion() throws Exception {
        DaoEventListener eventListener = createMock(DaoEventListener.class);        
        eventListener.onSave(isA(DialPlan.class));
        expectLastCall().once();        
        eventListener.onSave(m_l8n.getLocalization());
        expectLastCall().once();
        replay(eventListener);
        divertDaoEvents(eventListener);
        try {
            m_l8n.updateRegion("xx");
            fail("bad region error expected");
        } catch (UserException expectedE) {
            assertTrue("Bad region correctly rejected", true);
        }

        m_l8n.updateRegion("na.dialPlan");
        EasyMock.verify(eventListener);
    }

    private void updateLanguage(String language, String languageDirectory) throws Exception {
        assertEquals(1, m_l8n.updateLanguage(language));
        flush();
        assertEquals(1, getConnection().getRowCount("localization", "where language = '" + language + "'"));
        assertEquals(languageDirectory, m_l8n.getCurrentLanguageDir());
    }

    public void testUpdateLanguage() throws Exception {
        updateLanguage("pl", "stdprompts_pl");
        // update back to default
        updateLanguage(LocalizationContext.DEFAULT, LocalizationContext.PROMPTS_DEFAULT);
    }

    public void testDefaults() {
        assertEquals("na", m_l8n.getCurrentRegionId());
        assertEquals("en", m_l8n.getCurrentLanguage());
    }

    public void setLocalizationContext(LocalizationContext localizationContext) {
        m_l8n = localizationContext;
    }
}
