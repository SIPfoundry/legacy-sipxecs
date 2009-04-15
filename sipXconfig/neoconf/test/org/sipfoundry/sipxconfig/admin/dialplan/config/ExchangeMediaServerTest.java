/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.dialplan.ExchangeMediaServer;
import org.sipfoundry.sipxconfig.admin.localization.LocalizationContext;

import static org.easymock.EasyMock.createNiceMock;
import static org.easymock.EasyMock.replay;

public class ExchangeMediaServerTest extends TestCase {

    private ExchangeMediaServer m_out;
    private static final String VOICEMAIL_EXTENSION = "101";
    private static final String HOSTNAME = "example.com";

    @Override
    public void setUp() {
        LocalizationContext lc = createNiceMock(LocalizationContext.class);
        replay(lc);

        m_out = new ExchangeMediaServer();
        m_out.setHostname(HOSTNAME);
        m_out.setServerExtension(VOICEMAIL_EXTENSION);
        m_out.setLocalizationContext(lc);
    }

    public void testConstructorWithArgs() {
        ExchangeMediaServer out = new ExchangeMediaServer(HOSTNAME, VOICEMAIL_EXTENSION);
        assertEquals("Wrong value for hostname.", HOSTNAME, out.getHostname());
        assertEquals("Wrong value for extension.", VOICEMAIL_EXTENSION, out.getServerExtension());
    }

    public void testBuildVoicemailDepositUrl() {
        String uri = m_out.buildVoicemailDepositUrl("q=0.1");
        assertEquals(
                "<sip:101@example.com;transport=tcp?Diversion=%3Csip:{vdigits}%40{host}%3E%3Breason%3Dno-answer%3Bscreen%3Dno%3Bprivacy%3Doff>;q=0.1",
                uri);

        uri = m_out.buildVoicemailDepositUrl(null);
        assertEquals(
                "<sip:101@example.com;transport=tcp?Diversion=%3Csip:{vdigits}%40{host}%3E%3Breason%3Dno-answer%3Bscreen%3Dno%3Bprivacy%3Doff>",
                uri);
    }

    public void testBuildVoicemailRetrieveUrl() {
        String uri = m_out.buildVoicemailRetrieveUrl();
        assertEquals("<sip:101@example.com;transport=tcp>", uri);
    }

    public void testGetAddress() {
        assertEquals("Wrong server address.", HOSTNAME, m_out.getHostname());
    }
}
