/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.dialplan;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchMediaServer;
import org.sipfoundry.sipxconfig.localization.LocalizationContext;

public class FreeswitchMediaServerTest extends TestCase {
    private FreeswitchMediaServer m_mediaServer;
    private AddressManager m_addressManager;

    @Override
    protected void setUp() throws Exception {
        m_mediaServer = new FreeswitchMediaServer();
        
        m_addressManager = createMock(AddressManager.class);
        m_mediaServer.setAddressManager(m_addressManager);
        m_addressManager.getSingleAddress(FreeswitchFeature.SIP_ADDRESS);
        expectLastCall().andReturn(new Address(FreeswitchFeature.SIP_ADDRESS, "ivr.example.org", 3333)).anyTimes();
        replay(m_addressManager);

        LocationsManager locationsManager = createMock(LocationsManager.class);
        LocalizationContext localizationContext = createMock(LocalizationContext.class);
        localizationContext.getCurrentLanguage();
        expectLastCall().andReturn("en_US").anyTimes();
        replay(locationsManager, localizationContext);
        
        m_mediaServer.setLocalizationContext(localizationContext);
    }

    public void testBuildAttendantUrl() {        
        String uri = m_mediaServer.buildAttendantUrl("operator");
        assertEquals("<sip:IVR@ivr.example.org:3333;action=autoattendant;schedule_id=operator;locale=en_US>", uri);
    }

    public void testBuildVoicemailDepositUrl() {
        String uri = m_mediaServer.buildVoicemailDepositUrl("q=0.1");
        assertEquals("<sip:IVR@ivr.example.org:3333;mailbox={vdigits};action=deposit;locale=en_US>;q=0.1", uri);

        uri = m_mediaServer.buildVoicemailDepositUrl(null);
        assertEquals("<sip:IVR@ivr.example.org:3333;mailbox={vdigits};action=deposit;locale=en_US>", uri);
    }

    public void testBuildVoicemailRetrieveUrl() {
        String uri = m_mediaServer.buildVoicemailRetrieveUrl();
        assertEquals("<sip:IVR@ivr.example.org:3333;action=retrieve;locale=en_US>", uri);
    }
}
