/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.Arrays;

import junit.framework.TestCase;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.service.SipxIvrService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.test.TestUtil;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

public class FreeswitchMediaServerTest extends TestCase {
    private FreeswitchMediaServer m_mediaServer;

    @Override
    protected void setUp() throws Exception {
        m_mediaServer = new FreeswitchMediaServer();
        m_mediaServer.setPort(15060);

        LocationsManager locationsManager = createMock(LocationsManager.class);
        Location serviceLocation = TestUtil.createDefaultLocation();

        SipxIvrService service = new SipxIvrService();
        service.setBeanName(SipxIvrService.BEAN_ID);
        service.setLocationsManager(locationsManager);

        locationsManager.getLocationsForService(service);
        expectLastCall().andReturn(Arrays.asList(serviceLocation)).anyTimes();

        LocalizationContext localizationContext = createMock(LocalizationContext.class);
        localizationContext.getCurrentLanguage();
        expectLastCall().andReturn("en_US").anyTimes();

        replay(locationsManager, localizationContext);

        SipxServiceManager sipxServiceManager = TestUtil.getMockSipxServiceManager(true, service);
        m_mediaServer.setSipxServiceManager(sipxServiceManager);
        m_mediaServer.setLocalizationContext(localizationContext);
    }

    public void testBuildAttendantUrl() {
        String uri = m_mediaServer.buildAttendantUrl("operator");
        assertEquals("<sip:IVR@192.168.1.1:15060;action=autoattendant;schedule_id=operator;locale=en_US>", uri);
    }

    public void testBuildVoicemailDepositUrl() {
        String uri = m_mediaServer.buildVoicemailDepositUrl("q=0.1");
        assertEquals("<sip:IVR@192.168.1.1:15060;mailbox={vdigits};action=deposit;locale=en_US>;q=0.1", uri);

        uri = m_mediaServer.buildVoicemailDepositUrl(null);
        assertEquals("<sip:IVR@192.168.1.1:15060;mailbox={vdigits};action=deposit;locale=en_US>", uri);
    }

    public void testBuildVoicemailRetrieveUrl() {
        String uri = m_mediaServer.buildVoicemailRetrieveUrl();
        assertEquals("<sip:IVR@192.168.1.1:15060;action=retrieve;locale=en_US>", uri);
    }

    /**
     * Configure an instance of media server with necessary dependencies for test purposes only.
     */
    public static final void configureMediaServer(FreeswitchMediaServer mediaServer) {
        LocationsManager locationsManager = createMock(LocationsManager.class);

        Location serviceLocation = TestUtil.createDefaultLocation();

        SipxIvrService mediaService = new SipxIvrService();
        mediaService.setBeanName(SipxIvrService.BEAN_ID);
        mediaService.setLocationsManager(locationsManager);

        locationsManager.getLocationsForService(mediaService);
        expectLastCall().andReturn(Arrays.asList(serviceLocation)).anyTimes();
        replay(locationsManager);

        SipxServiceManager sipxServiceManager = TestUtil.getMockSipxServiceManager(true, mediaService);
        mediaServer.setSipxServiceManager(sipxServiceManager);
    }
}
