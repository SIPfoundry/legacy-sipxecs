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

import static org.easymock.EasyMock.createNiceMock;
import static org.easymock.EasyMock.replay;
import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.service.SipxMediaService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.test.TestUtil;

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
        assertEquals("Wrong server address.", "192.168.1.1;transport=tcp", m_out.getHostname());
    }

    public void testBuildVoicemailDepositUrl() {
        String uri = m_out.buildVoicemailDepositUrl("q=0.1");
        assertEquals(
                "<sip:{vdigits}@192.168.1.1;transport=tcp;voicexml=https%3A%2F%2F192.168.1.1%3A443%2Fcgi-bin%2Fvoicemail%2Fmediaserver.cgi%3Faction%3Ddeposit%26mailbox%3D{vdigits-escaped}>;q=0.1",
                uri);

        uri = m_out.buildVoicemailDepositUrl(null);
        assertEquals(
                "<sip:{vdigits}@192.168.1.1;transport=tcp;voicexml=https%3A%2F%2F192.168.1.1%3A443%2Fcgi-bin%2Fvoicemail%2Fmediaserver.cgi%3Faction%3Ddeposit%26mailbox%3D{vdigits-escaped}>",
                uri);
    }

    public void testBuildVoicemailRetrieveUrl() {
        String uri = m_out.buildVoicemailRetrieveUrl();
        assertEquals("<sip:{digits}@192.168.1.1;transport=tcp;voicexml=https%3A%2F%2F192.168.1.1%3A443%2Fcgi-bin%2Fvoicemail%2Fmediaserver.cgi%3Faction%3Dretrieve>", uri);
    }

    public void testBuildAttendantUrl() {
        String uri = m_out.buildAttendantUrl("operator");
        assertEquals("<sip:{digits}@192.168.1.1;transport=tcp;voicexml=https%3A%2F%2F192.168.1.1%3A443%2Fcgi-bin%2Fvoicemail%2Fmediaserver.cgi%3Faction%3Dautoattendant%26name%3Doperator>", uri);
    }

    /**
     * Configure an instance of SipXMediaServer with necessary dependencies for test purposes only.
     */
    public static final void configureMediaServer(SipXMediaServer mediaServer) {
        LocationsManager locationsManager = EasyMock.createMock(LocationsManager.class);
        locationsManager.getPrimaryLocation();
        EasyMock.expectLastCall().andReturn(TestUtil.createDefaultLocation()).anyTimes();
        EasyMock.replay(locationsManager);

        SipxMediaService mediaService = new SipxMediaService();
        mediaService.setBeanName(SipxMediaService.BEAN_ID);
        mediaService.setLocationsManager(locationsManager);
        mediaService.setVoicemailHttpsPort(443);

        SipxServiceManager sipxServiceManager = TestUtil.getMockSipxServiceManager(true, mediaService);
        mediaServer.setSipxServiceManager(sipxServiceManager);
    }
}
