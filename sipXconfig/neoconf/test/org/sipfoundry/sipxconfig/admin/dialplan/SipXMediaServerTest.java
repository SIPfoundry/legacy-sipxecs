/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.admin.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.service.SipxRegistrarService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;

import static org.easymock.EasyMock.createNiceMock;
import static org.easymock.EasyMock.replay;

public class SipXMediaServerTest extends TestCase {

    private SipXMediaServer m_out;

    @Override
    public void setUp() {
        m_out = new SipXMediaServer();
        SipXMediaServerTest.configureMediaServer(m_out);
        LocalizationContext lc = createNiceMock(LocalizationContext.class);
        replay(lc);
        m_out.setLocalizationContext(lc);
    }

    public void testGetHostname() {
        m_out.setHostname("foo");
        assertEquals("Wrong server address.", "localhost;transport=tcp", m_out.getHostname());
    }


    public void testBuildVoicemailDepositUrl() {
        String uri = m_out.buildVoicemailDepositUrl("q=0.1");
        assertEquals(
                "<sip:{vdigits}@localhost;transport=tcp;voicexml=https%3A%2F%2Flocalhost%3A443%2Fcgi-bin%2Fvoicemail%2Fmediaserver.cgi%3Faction%3Ddeposit%26mailbox%3D{vdigits-escaped}>;q=0.1",
                uri);

        uri = m_out.buildVoicemailDepositUrl(null);
        assertEquals(
                "<sip:{vdigits}@localhost;transport=tcp;voicexml=https%3A%2F%2Flocalhost%3A443%2Fcgi-bin%2Fvoicemail%2Fmediaserver.cgi%3Faction%3Ddeposit%26mailbox%3D{vdigits-escaped}>",
                uri);
    }

    public void testBuildVoicemailRetrieveUrl() {
        String uri = m_out.buildVoicemailRetrieveUrl();
        assertEquals("<sip:{digits}@localhost;transport=tcp;voicexml=https%3A%2F%2Flocalhost%3A443%2Fcgi-bin%2Fvoicemail%2Fmediaserver.cgi%3Faction%3Dretrieve>", uri);
    }

    public void testBuildAttendantUrl() {
        String uri = m_out.buildAttendantUrl("operator");
        assertEquals("<sip:{digits}@localhost;transport=tcp;voicexml=https%3A%2F%2Flocalhost%3A443%2Fcgi-bin%2Fvoicemail%2Fmediaserver.cgi%3Faction%3Dautoattendant%26name%3Doperator>", uri);
    }

    /**
     * Configure an instance of SipXMediaServer with necessary dependencies for test purposes only.
     */
    public static final void configureMediaServer(SipXMediaServer mediaServer) {
        SipxRegistrarService registrarService = new SipxRegistrarService();
        registrarService.setMediaServerSipSrvOrHostport("localhost");
        registrarService.setVoicemailHttpsPort("443");
        SipxServiceManager sipxServiceManager = EasyMock.createMock(SipxServiceManager.class);
        sipxServiceManager.getServiceByBeanId(SipxRegistrarService.BEAN_ID);
        EasyMock.expectLastCall().andReturn(registrarService).anyTimes();
        EasyMock.replay(sipxServiceManager);
        mediaServer.setSipxServiceManager(sipxServiceManager);
    }
}
