/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.service.freeswitch;

import java.util.Arrays;
import java.util.Comparator;
import java.util.Set;
import java.util.TreeSet;

import org.sipfoundry.sipxconfig.acccode.AccCodeContext;
import org.sipfoundry.sipxconfig.conference.Bridge;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchAction;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchCondition;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchExtension;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchExtensionCollector;
import org.sipfoundry.sipxconfig.openacd.OpenAcdLine;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;
import org.sipfoundry.sipxconfig.service.SipxServiceTestBase;

import static org.easymock.EasyMock.expect;
import static org.easymock.classextension.EasyMock.createMock;
import static org.easymock.classextension.EasyMock.createNiceMock;
import static org.easymock.classextension.EasyMock.replay;
import static org.easymock.classextension.EasyMock.verify;

public class DefaultContextConfigurationTest extends SipxServiceTestBase {

    private ConferenceBridgeContext m_conferenceContext;
    private AccCodeContext m_accCodeContext;
    private FreeswitchExtensionCollector m_freeswitchExtensionCollector;
    private DefaultContextConfiguration m_configuration;

    private static String[][] DATA = {
        {
            "disable", "101", "0000", "000", "000", ""
        }, {
            "sales", "400", "400111", "400222", "400AAA", "sip:sales@bridge.sipfoundry.org"
        }, {
            "marketing", "500", "500111", "500222", "500AAA", "sip:marketing@bridge.sipfoundry.org"
        }
    };

    private static String[][] ACTIONS = {
        {
            "answer", ""
        }, {
            "set", "domain_name=$${domain}"
        }, {
            "set", "brand=1"
        }, {
            "set", "queue=Sales"
        }, {
            "set", "allow_voicemail=true"
        }, {
            "erlang_sendmsg", "freeswitch_media_manager testme@192.168.1.1 inivr ${uuid}"
        }, {
            "playback", "/usr/share/www/doc/stdprompts/welcome.wav"
        }, {
            "erlang", "freeswitch_media_manager:! testme@192.168.1.1"
        },
    };

    public void setUp() {
        m_conferenceContext = createNiceMock(ConferenceBridgeContext.class);
        m_accCodeContext = createNiceMock(AccCodeContext.class);
        m_freeswitchExtensionCollector = createNiceMock(FreeswitchExtensionCollector.class);

        m_configuration = new DefaultContextConfiguration();
        m_configuration.setTemplate("freeswitch/default_context.xml.vm");
        m_configuration.setConferenceContext(m_conferenceContext);
        m_configuration.setAccCodeContext(m_accCodeContext);
        m_configuration.setFreeswitchExtensionCollector(m_freeswitchExtensionCollector);
    }

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

    public static OpenAcdLine createOpenAcdLine(String extensionName) {
        OpenAcdLine extension = new OpenAcdLine();
        extension.setName(extensionName);

        FreeswitchCondition condition = new FreeswitchCondition();
        condition.setField("destination_number");
        condition.setExpression("^300$");
        extension.addCondition(condition);

        for (int i = 0; i < ACTIONS.length; i++) {
            FreeswitchAction action = new FreeswitchAction();
            action.setApplication(ACTIONS[i][0]);
            action.setData(ACTIONS[i][1]);
            condition.addAction(action);
        }
        return extension;
    }

    public void testWriteNoBridge() throws Exception {
        SipxFreeswitchService service = new SipxFreeswitchService();
        initCommonAttributes(service);

        replay(m_conferenceContext, m_accCodeContext, m_freeswitchExtensionCollector);

        assertCorrectFileGeneration(m_configuration, "default_context-no-conferences.test.xml");
        verify(m_conferenceContext);
    }

    public void testWrite() throws Exception {
        SipxFreeswitchService service = new SipxFreeswitchService();
        initCommonAttributes(service);

        expect(m_conferenceContext.getBridgeByServer("sipx.example.org")).andReturn(createBridge());
        expect(m_accCodeContext.isEnabled()).andReturn(false);
        replay(m_conferenceContext, m_accCodeContext, m_freeswitchExtensionCollector);

        assertCorrectFileGeneration(m_configuration, "default_context.test.xml");
        verify(m_conferenceContext);
    }

    public void testWriteWithOpenAcdExtensions() throws Exception {
        SipxFreeswitchService service = new SipxFreeswitchService();
        initCommonAttributes(service);

        expect(m_freeswitchExtensionCollector.getExtensions()).andReturn(
                Arrays.asList((FreeswitchExtension) createOpenAcdLine("sales")));
        replay(m_conferenceContext, m_accCodeContext, m_freeswitchExtensionCollector);

        assertCorrectFileGeneration(m_configuration, "default_context_freeswitch_extensions.test.xml");
        verify(m_freeswitchExtensionCollector);
    }
}
