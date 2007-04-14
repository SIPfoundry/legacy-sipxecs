/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.conference;

import java.util.List;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;

public class ConferenceTest extends TestCase {
    private Conference m_conf;

    protected void setUp() throws Exception {
        m_conf = (Conference) TestHelper.getApplicationContext().getBean("conferenceConference",
                Conference.class);
    }

    public void testGenerateAccessCodes() {
        m_conf.generateAccessCodes();

        assertTrue(m_conf.getSettingValue(Conference.ORGANIZER_CODE).toString().length() > 0);
        assertTrue(m_conf.getSettingValue(Conference.PARTICIPANT_CODE).toString().length() > 0);
    }

    public void testGetUri() {
        m_conf.setName("weekly.marketing");

        Bridge bridge = new Bridge();
        bridge.setHost("bridge1.sipfoundry.org");
        bridge.addConference(m_conf);

        assertEquals("sip:weekly.marketing@bridge1.sipfoundry.org", m_conf.getUri());

        bridge.setHost("abc.domain.com");
        assertEquals("sip:weekly.marketing@abc.domain.com", m_conf.getUri());
    }

    public void testGenerateRemoteAdmitSecret() {
        assertNull(m_conf.getRemoteAdmitSecret());
        m_conf.generateRemoteAdmitSecret();
        assertTrue(m_conf.getRemoteAdmitSecret().length() > 0);
    }

    public void testGenerateAliases() {
        Bridge bridge = new Bridge();
        bridge.initialize();
        bridge.setAdmissionServer("media.sipfoundry.org:5100");

        bridge.addConference(m_conf);
        // empty for disabled conference
        m_conf.setName("conf1");
        m_conf.setAdmissionScriptServer("localhost:8091");
        List aliasMappings = m_conf.generateAliases("sipfoundry.org");

        assertTrue(aliasMappings.isEmpty());

        // 1 alias for conference without extension
        m_conf.setEnabled(true);
        aliasMappings = m_conf.generateAliases("sipfoundry.org");
        assertEquals(1, aliasMappings.size());

        AliasMapping am = (AliasMapping) aliasMappings.get(0);
        assertEquals("conf1@sipfoundry.org", am.getIdentity());
        assertEquals("<sip:conf1@media.sipfoundry.org:5100;"
                + "play=https%3A%2F%2Flocalhost%3A8091%2Fcgi-bin%2Fcbadmission%2Fcbadmission.cgi"
                + "%3F" + "action%3Dconferencebridge" + "%26" + "confid%3Dconf1" + "%26"
                + "name%3Dcbadmission>", am.getContact());

        // 2 aliases for conference with extension
        m_conf.setExtension("1111");
        aliasMappings = m_conf.generateAliases("sipfoundry.org");
        assertEquals(2, aliasMappings.size());

        AliasMapping am0 = (AliasMapping) aliasMappings.get(0);
        assertEquals("1111@sipfoundry.org", am0.getIdentity());
        assertEquals("sip:conf1@sipfoundry.org", am0.getContact());

        AliasMapping am1 = (AliasMapping) aliasMappings.get(1);
        assertEquals(am1, am);
    }
}
