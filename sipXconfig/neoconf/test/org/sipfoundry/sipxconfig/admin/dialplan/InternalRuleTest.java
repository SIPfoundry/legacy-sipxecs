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

import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;
import java.util.TimeZone;

import junit.framework.TestCase;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.StringUtils;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.admin.ScheduledDay;
import org.sipfoundry.sipxconfig.admin.dialplan.MappingRule.Operator;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime.WorkingHours;
import org.sipfoundry.sipxconfig.admin.dialplan.config.UrlTransform;
import org.sipfoundry.sipxconfig.admin.forwarding.GeneralSchedule;
import org.sipfoundry.sipxconfig.admin.forwarding.Schedule;
import org.sipfoundry.sipxconfig.admin.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.springframework.beans.factory.BeanFactory;

/**
 * InternalRuleTest
 */
public class InternalRuleTest extends TestCase {
    private static final String TEST_DESCRIPTION = "kuku description";
    private static final String TEST_NAME = "kuku name";
    private Schedule m_schedule;
    private BeanFactory m_beanFactory;
    private LocalizationContext m_localizationContext;
    private FreeswitchMediaServer m_mediaServer;

    @Override
    protected void setUp() throws Exception {
        m_schedule = new GeneralSchedule();
        m_schedule.setName("Custom schedule");
        WorkingHours[] hours = new WorkingHours[1];
        WorkingTime wt = new WorkingTime();
        hours[0] = new WorkingHours();
        Calendar cal = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
        cal.set(2006, Calendar.DECEMBER, 31, 10, 12);
        hours[0].setStart(cal.getTime());
        cal.set(2006, Calendar.DECEMBER, 31, 11, 88);
        hours[0].setStop(cal.getTime());
        hours[0].setEnabled(true);
        hours[0].setDay(ScheduledDay.WEDNESDAY);
        wt.setWorkingHours(hours);
        wt.setEnabled(true);
        m_schedule.setWorkingTime(wt);

        m_mediaServer = new FreeswitchMediaServer();
        FreeswitchMediaServerTest.configureMediaServer(m_mediaServer);

        m_localizationContext = EasyMock.createNiceMock(LocalizationContext.class);
        m_mediaServer.setLocalizationContext(m_localizationContext);

        m_beanFactory = EasyMock.createNiceMock(BeanFactory.class);
        m_beanFactory.getBean("freeswitchMediaServer", MediaServer.class);
        EasyMock.expectLastCall().andReturn(m_mediaServer);
    }

    /**
     * Call this method before using any mocks in test, and after any optional mock config is
     * complete
     */
    private void replayMocks() {
        EasyMock.replay(m_beanFactory, m_localizationContext);
    }

    public void testAppendToGenerationRules() throws Exception {
        InternalRule ir = new InternalRule();
        ir.setName("kuku");
        ir.setDescription(TEST_DESCRIPTION);
        ir.setLocalExtensionLen(5);
        ir.setVoiceMail("20004");
        ir.setVoiceMailPrefix("7");
        ir.setEnabled(true);

        replayMocks();
        MediaServerFactory mediaServerFactory = new MediaServerFactory();
        mediaServerFactory.setBeanFactory(m_beanFactory);
        ir.setMediaServerFactory(mediaServerFactory);
        ir.setMediaServerType("freeswitchMediaServer");
        ir.setMediaServerHostname("example");

        List<DialingRule> rules = new ArrayList<DialingRule>();
        ir.appendToGenerationRules(rules);

        assertEquals(4, rules.size());

        MappingRule v = (MappingRule) rules.get(0);
        MappingRule vt = (MappingRule) rules.get(1);
        MappingRule vf = (MappingRule) rules.get(2);
        MappingRule faxRule = (MappingRule) rules.get(3);

        assertEquals(TEST_DESCRIPTION, v.getDescription());
        assertEquals("20004", v.getPatterns()[0]);
        assertEquals(0, v.getPermissions().size());
        UrlTransform tv = (UrlTransform) v.getTransforms()[0];
        assertEquals("<sip:IVR@192.168.1.1:0;action=retrieve>", tv.getUrl());

        assertEquals(TEST_DESCRIPTION, vt.getDescription());
        assertEquals("7xxxxx", vt.getPatterns()[0]);
        assertEquals(0, vt.getPermissions().size());
        UrlTransform tvt = (UrlTransform) vt.getTransforms()[0];
        assertEquals("<sip:IVR@192.168.1.1:0;mailbox={vdigits};action=deposit>", tvt.getUrl());

        assertEquals(TEST_DESCRIPTION, vf.getDescription());
        assertEquals("~~vm~.", vf.getPatterns()[0]);
        assertEquals(PermissionName.FREESWITH_VOICEMAIL.getName(), vf.getPermissionNames().get(0));
        UrlTransform tvf = (UrlTransform) vf.getTransforms()[0];
        assertEquals("<sip:IVR@192.168.1.1:0;mailbox={vdigits};action=deposit>;q=0.1", tvf.getUrl());

        assertEquals("Fax Routing rule", faxRule.getDescription());
        assertEquals("~~ff~.", faxRule.getPatterns()[0]);
        assertEquals(0 == faxRule.getPermissionNames().size(), true);
        UrlTransform faxRuleUrl = (UrlTransform) faxRule.getTransforms()[0];
        assertEquals("<sip:IVR@192.168.1.1:0;mailbox={vdigits};action=faxrx>", faxRuleUrl.getUrl());

        EasyMock.verify(m_beanFactory, m_localizationContext);
    }

    public void testAppendToGenerationRulesWithSchedule() throws Exception {
        InternalRule ir = new InternalRule();
        ir.setName("kuku");
        ir.setDescription(TEST_DESCRIPTION);
        ir.setLocalExtensionLen(5);
        ir.setVoiceMail("20004");
        ir.setVoiceMailPrefix("7");
        ir.setEnabled(true);

        replayMocks();
        MediaServerFactory mediaServerFactory = new MediaServerFactory();
        mediaServerFactory.setBeanFactory(m_beanFactory);
        ir.setMediaServerFactory(mediaServerFactory);
        ir.setMediaServerType("freeswitchMediaServer");
        ir.setMediaServerHostname("example");

        List<DialingRule> rules = new ArrayList<DialingRule>();
        ir.appendToGenerationRules(rules);

        assertEquals(4, rules.size());

        MappingRule v = (MappingRule) rules.get(0);
        v.setSchedule(m_schedule);
        MappingRule vt = (MappingRule) rules.get(1);
        vt.setSchedule(m_schedule);
        MappingRule vf = (MappingRule) rules.get(2);
        vf.setSchedule(m_schedule);

        assertEquals(TEST_DESCRIPTION, v.getDescription());
        assertEquals("20004", v.getPatterns()[0]);
        assertEquals(0, v.getPermissions().size());
        UrlTransform tv = (UrlTransform) v.getTransforms()[0];
        assertTrue(tv.getUrl().startsWith("<sip:IVR@192.168.1.1:0;action=retrieve>;sipx-ValidTime="));

        assertEquals(TEST_DESCRIPTION, vt.getDescription());
        assertEquals("7xxxxx", vt.getPatterns()[0]);
        assertEquals(0, vt.getPermissions().size());
        UrlTransform tvt = (UrlTransform) vt.getTransforms()[0];
        assertTrue(tvt.getUrl().startsWith("<sip:IVR@192.168.1.1:0;mailbox={vdigits};action=deposit>;sipx-ValidTime="));

        assertEquals(TEST_DESCRIPTION, vf.getDescription());
        assertEquals("~~vm~.", vf.getPatterns()[0]);
        assertEquals(PermissionName.FREESWITH_VOICEMAIL.getName(), vf.getPermissionNames().get(0));
        UrlTransform tvf = (UrlTransform) vf.getTransforms()[0];
        assertTrue(tvf.getUrl().startsWith("<sip:IVR@192.168.1.1:0;mailbox={vdigits};action=deposit>;q=0.1;sipx-ValidTime="));

        EasyMock.verify(m_beanFactory, m_localizationContext);
    }

    public void testAppendToGenerationRulesLang() throws Exception {
        InternalRule ir = new InternalRule();
        ir.setName("kuku");
        ir.setDescription(TEST_DESCRIPTION);
        ir.setLocalExtensionLen(5);
        ir.setVoiceMail("20004");
        ir.setVoiceMailPrefix("7");
        ir.setEnabled(true);

        EasyMock.expect(m_localizationContext.getCurrentLanguage()).andReturn("pl").atLeastOnce();
        replayMocks();
        MediaServerFactory mediaServerFactory = new MediaServerFactory();
        mediaServerFactory.setBeanFactory(m_beanFactory);
        ir.setMediaServerFactory(mediaServerFactory);
        ir.setMediaServerType("freeswitchMediaServer");
        ir.setMediaServerHostname("example");

        List rules = new ArrayList();
        ir.appendToGenerationRules(rules);

        assertEquals(4, rules.size());

        MappingRule v = (MappingRule) rules.get(0);
        MappingRule vt = (MappingRule) rules.get(1);
        MappingRule vf = (MappingRule) rules.get(2);
        MappingRule faxRule = (MappingRule) rules.get(3);

        assertEquals(TEST_DESCRIPTION, v.getDescription());
        assertEquals("20004", v.getPatterns()[0]);
        assertEquals(0, v.getPermissions().size());
        UrlTransform tv = (UrlTransform) v.getTransforms()[0];
        assertEquals("<sip:IVR@192.168.1.1:0;action=retrieve;locale=pl>", tv.getUrl());

        assertEquals(TEST_DESCRIPTION, vt.getDescription());
        assertEquals("7xxxxx", vt.getPatterns()[0]);
        assertEquals(0, vt.getPermissions().size());
        UrlTransform tvt = (UrlTransform) vt.getTransforms()[0];
        assertEquals("<sip:IVR@192.168.1.1:0;mailbox={vdigits};action=deposit;locale=pl>", tvt.getUrl());

        assertEquals(TEST_DESCRIPTION, vf.getDescription());
        assertEquals("~~vm~.", vf.getPatterns()[0]);
        assertEquals(PermissionName.FREESWITH_VOICEMAIL.getName(), vf.getPermissionNames().get(0));
        UrlTransform tvf = (UrlTransform) vf.getTransforms()[0];
        assertEquals("<sip:IVR@192.168.1.1:0;mailbox={vdigits};action=deposit;locale=pl>;q=0.1", tvf.getUrl());

        assertEquals("Fax Routing rule", faxRule.getDescription());
        assertEquals("~~ff~.", faxRule.getPatterns()[0]);
        assertEquals(0 == faxRule.getPermissionNames().size(), true);
        UrlTransform faxRuleUrl = (UrlTransform) faxRule.getTransforms()[0];
        assertEquals("<sip:IVR@192.168.1.1:0;mailbox={vdigits};action=faxrx;locale=pl>", faxRuleUrl.getUrl());

        EasyMock.verify(m_beanFactory, m_localizationContext);
    }

    public void testOperator() {
        AutoAttendant aa = new AutoAttendant();
        aa.setName(TEST_NAME);
        aa.setDescription(TEST_DESCRIPTION);

        Operator o = new MappingRule.Operator(aa, "100", ArrayUtils.EMPTY_STRING_ARRAY, m_mediaServer);
        assertEquals("100", StringUtils.join(o.getPatterns(), "|"));
        assertEquals(TEST_NAME, o.getName());
        assertEquals(TEST_DESCRIPTION, o.getDescription());
    }

    public void testOperatorWithAliases() {
        AutoAttendant aa = new AutoAttendant();
        aa.setName(TEST_NAME);
        aa.setDescription(TEST_DESCRIPTION);

        Operator o = new MappingRule.Operator(aa, "100", new String[] {
            "0", "operator"
        }, m_mediaServer);
        assertEquals("100|0|operator", StringUtils.join(o.getPatterns(), "|"));
        assertEquals(TEST_NAME, o.getName());
        assertEquals(TEST_DESCRIPTION, o.getDescription());
    }

    public void testOperatorNoExtension() {
        AutoAttendant aa = new AutoAttendant();
        aa.setName(TEST_NAME);
        aa.setDescription(TEST_DESCRIPTION);

        Operator o = new MappingRule.Operator(aa, null, new String[] {
            "0", "operator"
        }, m_mediaServer);
        assertEquals("0|operator", StringUtils.join(o.getPatterns(), "|"));
        assertEquals(TEST_NAME, o.getName());
        assertEquals(TEST_DESCRIPTION, o.getDescription());
    }
}
