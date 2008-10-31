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

import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.Holiday;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.ScheduledAttendant;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime;
import org.sipfoundry.sipxconfig.admin.dialplan.config.MappingRules;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.UrlTransform;
import org.sipfoundry.sipxconfig.admin.localization.LocalizationContext;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.createNiceMock;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;

public class AttendantRuleTest extends TestCase {
    private static final String VOICEMAIL_SERVER = "https%3A%2F%2Flocalhost%3A443";
    private static final String URL_PARAMS = ";voicexml=" + VOICEMAIL_SERVER
            + "%2Fcgi-bin%2Fvoicemail%2Fmediaserver.cgi%3Faction%3D";
    private static final String OPERATOR_URL = "<sip:{digits}@localhost;transport=tcp" + URL_PARAMS
            + "autoattendant%26name%3Daa_-1>";

    public void testNotImplemented() {
        AttendantRule rule = new AttendantRule();
        assertNull(rule.getPatterns());
        assertNull(rule.getTransforms());
    }

    public void testAppendToGenerationRulesDisabled() {
        AttendantRule rule = new AttendantRule();
        List<DialingRule> list = new ArrayList<DialingRule>();
        rule.appendToGenerationRules(list);
        // do not add anything if rule is disabled and it is disabled by default
        assertTrue(list.isEmpty());
    }

    public void testAppendToGenerationRules() {
        LocalizationContext lc = createNiceMock(LocalizationContext.class);
        replay(lc);

        AttendantRule rule = new AttendantRule();
        SipXMediaServer mediaServer = new SipXMediaServer();
        SipXMediaServerTest.configureMediaServer(mediaServer);
        mediaServer.setLocalizationContext(lc);

        rule.setMediaServer(mediaServer);
        rule.setName("abc");
        rule.setExtension("100");
        rule.setAttendantAliases("0 operator");
        rule.setEnabled(true);

        List<DialingRule> list = new ArrayList<DialingRule>();
        rule.appendToGenerationRules(list);
        // do not add anything if rule is disabled and it is disabled by default
        assertFalse(list.isEmpty());

        Object firstRule = list.get(0);
        assertTrue(firstRule instanceof MappingRule.Operator);

        DialingRule dr = (DialingRule) firstRule;
        String[] patterns = dr.getPatterns();
        assertEquals(3, patterns.length);
        assertEquals("100", patterns[0]);
        assertEquals("0", patterns[1]);
        assertEquals("operator", patterns[2]);

        Transform[] transforms = dr.getTransforms();
        assertEquals(1, transforms.length);
        assertTrue(transforms[0] instanceof UrlTransform);
        UrlTransform urlTransform = (UrlTransform) transforms[0];

        assertEquals(OPERATOR_URL, urlTransform.getUrl());
    }

    public void testMappingRulesVxml() throws Exception {
        LocalizationContext lc = createNiceMock(LocalizationContext.class);
        replay(lc);

        AttendantRule rule = new AttendantRule();
        SipXMediaServer mediaServer = new SipXMediaServer();
        SipXMediaServerTest.configureMediaServer(mediaServer);
        mediaServer.setLocalizationContext(lc);

        rule.setMediaServer(mediaServer);
        rule.setName("abc");
        rule.setExtension("100");
        rule.setAttendantAliases("0 operator");
        rule.setEnabled(true);

        MappingRules mappingRules = new MappingRules();
        List<DialingRule> rules = new ArrayList<DialingRule>();
        rule.appendToGenerationRules(rules);
        mappingRules.begin();
        for (DialingRule r : rules) {
            mappingRules.generate(r);
        }
        mappingRules.end();

        String generatedXml = mappingRules.getFileContent();
        InputStream referenceXmlStream = getClass().getResourceAsStream("sipxvxml-aa-rules.test.xml");

        assertEquals(IOUtils.toString(referenceXmlStream), generatedXml);
    }

    public void testMappingRulesIvr() throws Exception {
        LocalizationContext lc = createMock(LocalizationContext.class);
        expect(lc.getCurrentLanguage()).andReturn("en_US");
        replay(lc);

        AttendantRule rule = new AttendantRule();
        FreeswitchMediaServer mediaServer = new FreeswitchMediaServer();
        mediaServer.setLocalizationContext(lc);
        mediaServer.setHostname("localhost");
        mediaServer.setPort(8084);

        rule.setMediaServer(mediaServer);
        rule.setName("abc");
        rule.setExtension("100");
        rule.setAttendantAliases("0 operator");
        rule.setEnabled(true);

        MappingRules mappingRules = new MappingRules();
        List<DialingRule> rules = new ArrayList<DialingRule>();
        rule.appendToGenerationRules(rules);
        mappingRules.begin();
        for (DialingRule r : rules) {
            mappingRules.generate(r);
        }
        mappingRules.end();

        String generatedXml = mappingRules.getFileContent();
        InputStream referenceXmlStream = getClass().getResourceAsStream("sipxivr-aa-rules.test.xml");

        assertEquals(IOUtils.toString(referenceXmlStream), generatedXml);
    }

    public void testIsInternal() {
        AttendantRule rule = new AttendantRule();
        // this is internal rule
        assertTrue(rule.isInternal());
    }

    public void testCheckAttendant() {
        AutoAttendant attendant = new AutoAttendant();
        attendant.setUniqueId();

        WorkingTime wt = new WorkingTime();
        wt.setAttendant(attendant);

        Holiday holiday = new Holiday();
        holiday.setAttendant(attendant);

        ScheduledAttendant sa = new ScheduledAttendant();
        sa.setAttendant(attendant);

        AttendantRule r1 = new AttendantRule();
        assertFalse(r1.checkAttendant(attendant));

        r1.setAfterHoursAttendant(sa);
        assertTrue(r1.checkAttendant(attendant));

        AttendantRule r2 = new AttendantRule();
        r2.setWorkingTimeAttendant(wt);
        r2.setHolidayAttendant(holiday);
        assertTrue(r2.checkAttendant(attendant));
    }

    public void testGetAttendantAliasesAsArray() {
        String[] attendantAliases = AttendantRule.getAttendantAliasesAsArray(null);
        assertEquals(0, attendantAliases.length);
        attendantAliases = AttendantRule.getAttendantAliasesAsArray("");
        assertEquals(0, attendantAliases.length);
        attendantAliases = AttendantRule.getAttendantAliasesAsArray("0 operator");
        assertEquals(2, attendantAliases.length);
        assertEquals("0", attendantAliases[0]);
        assertEquals("operator", attendantAliases[1]);
    }
}
