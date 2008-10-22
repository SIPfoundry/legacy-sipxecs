/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.service.freeswitch;

import java.util.Comparator;
import java.util.Set;
import java.util.TreeSet;

import org.sipfoundry.sipxconfig.conference.Bridge;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;
import org.sipfoundry.sipxconfig.service.SipxServiceTestBase;

import static org.easymock.EasyMock.expect;
import static org.easymock.classextension.EasyMock.createMock;
import static org.easymock.classextension.EasyMock.createNiceMock;
import static org.easymock.classextension.EasyMock.replay;
import static org.easymock.classextension.EasyMock.verify;

public class DefaultContextConfigurationTest extends SipxServiceTestBase {
    private static String[][] DATA = {
        {
            "disable", "101", "0000", "000", "000", ""
        },
        {
            "sales", "400", "400111", "400222", "400AAA", "sip:sales@bridge.sipfoundry.org"
        },
        {
            "marketing", "500", "500111", "500222", "500AAA",
            "sip:marketing@bridge.sipfoundry.org"
        }
    };

    private Bridge createBridge() {

        Comparator<Conference> comparator = new Comparator<Conference>() {
            public int compare(Conference o1, Conference o2) {
                return o1.getId().compareTo(o2.getId());
            }};
        Set<Conference> conferences = new TreeSet<Conference>(comparator);
        for (int i = 0; i < DATA.length; i++) {
            Conference conference = createMock(Conference.class);

            expect(conference.getId()).andReturn(i).anyTimes();
            expect(conference.getName()).andReturn(DATA[i][0]).anyTimes();
            expect(conference.getExtension()).andReturn(DATA[i][1]).anyTimes();
            expect(conference.getOrganizerAccessCode()).andReturn(DATA[i][2]).anyTimes();
            expect(conference.getParticipantAccessCode()).andReturn(DATA[i][3]).anyTimes();
            expect(conference.getRemoteAdmitSecret()).andReturn(DATA[i][4]).anyTimes();
            expect(conference.getUri()).andReturn(DATA[i][5]).anyTimes();

            // the first one is disabled
            expect(conference.isEnabled()).andReturn(i > 0);
            replay(conference);

            conferences.add(conference);
        }
        Bridge bridge = new Bridge();
        bridge.setConferences(conferences);
        return bridge;
    }


    public void testWriteNoBridge() throws Exception {
        SipxFreeswitchService service = new SipxFreeswitchService();
        initCommonAttributes(service);

        DefaultContextConfiguration configuration = new DefaultContextConfiguration();
        configuration.setTemplate("freeswitch/default_context.xml.vm");

        ConferenceBridgeContext conferenceContext = createNiceMock(ConferenceBridgeContext.class);
        configuration.setConferenceContext(conferenceContext);
        replay(conferenceContext);

        assertCorrectFileGeneration(configuration, "default_context-no-conferences.test.xml");

        verify(conferenceContext);
    }

    public void testWrite() throws Exception {
        SipxFreeswitchService service = new SipxFreeswitchService();
        initCommonAttributes(service);

        DefaultContextConfiguration configuration = new DefaultContextConfiguration();
        configuration.setTemplate("freeswitch/default_context.xml.vm");

        ConferenceBridgeContext conferenceContext = createMock(ConferenceBridgeContext.class);
        expect(conferenceContext.getBridgeByServer("sipx.example.org")).andReturn(createBridge());
        replay(conferenceContext);

        configuration.setConferenceContext(conferenceContext);

        assertCorrectFileGeneration(configuration, "default_context.test.xml");

        verify(conferenceContext);
    }
}
