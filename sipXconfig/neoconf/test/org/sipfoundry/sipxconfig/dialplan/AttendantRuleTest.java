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
import static org.easymock.EasyMock.createNiceMock;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

import java.io.InputStream;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.List;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.dialplan.attendant.Holiday;
import org.sipfoundry.sipxconfig.dialplan.attendant.ScheduledAttendant;
import org.sipfoundry.sipxconfig.dialplan.attendant.WorkingTime;
import org.sipfoundry.sipxconfig.dialplan.config.MappingRules;
import org.sipfoundry.sipxconfig.dialplan.config.RulesXmlFile;
import org.sipfoundry.sipxconfig.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.dialplan.config.UrlTransform;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchMediaServer;
import org.sipfoundry.sipxconfig.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class AttendantRuleTest extends TestCase {
    private DomainManager m_domainManager;

    @Override
    public void setUp() {
        m_domainManager = TestHelper.getMockDomainManager();
        replay(m_domainManager);
    }

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
        AttendantRule rule = new AttendantRule();
        FreeswitchMediaServer mediaServer = new FreeswitchMediaServer();

        AddressManager addressManager = createMock(AddressManager.class);
        addressManager.getSingleAddress(FreeswitchFeature.SIP_ADDRESS);
        expectLastCall().andReturn(new Address(FreeswitchFeature.SIP_ADDRESS, "ivr.example.org", 3333)).anyTimes();
        replay(addressManager);
        mediaServer.setAddressManager(addressManager);

        FeatureManager featureManager = createMock(FeatureManager.class);
        featureManager.isFeatureEnabled(FreeswitchFeature.FEATURE);
        expectLastCall().andReturn(true).anyTimes();
        replay(featureManager);
        rule.setFeatureManager(featureManager);
        LocalizationContext lc = createNiceMock(LocalizationContext.class);
        replay(lc);
        mediaServer.setLocalizationContext(lc);

        rule.setMediaServer(mediaServer);
        rule.setName("abc");
        rule.setExtension("100");
        rule.setAttendantAliases("0 operator");
        rule.setDid("+123456789");
        rule.setEnabled(true);

        List<DialingRule> list = new ArrayList<DialingRule>();
        rule.appendToGenerationRules(list);
        // do not add anything if rule is disabled and it is disabled by default
        assertFalse(list.isEmpty());

        Object firstRule = list.get(0);
        assertTrue(firstRule instanceof MappingRule.Operator);

        DialingRule dr = (DialingRule) firstRule;
        String[] patterns = dr.getPatterns();
        assertEquals(4, patterns.length);
        assertEquals("100", patterns[0]);
        assertEquals("0", patterns[1]);
        assertEquals("operator", patterns[2]);
        assertEquals("+123456789", patterns[3]);

        Transform[] transforms = dr.getTransforms();
        assertEquals(1, transforms.length);
        assertTrue(transforms[0] instanceof UrlTransform);
        UrlTransform urlTransform = (UrlTransform) transforms[0];

        assertEquals("<sip:IVR@ivr.example.org:3333;action=autoattendant;schedule_id=aa_-1>", urlTransform.getUrl());
    }

    public void testMappingRulesIvr() throws Exception {
        AttendantRule rule = new AttendantRule();
        FeatureManager featureManager = createMock(FeatureManager.class);
        featureManager.isFeatureEnabled(FreeswitchFeature.FEATURE);
        expectLastCall().andReturn(true).anyTimes();
        replay(featureManager);
        rule.setFeatureManager(featureManager);
        FreeswitchMediaServer mediaServer = new FreeswitchMediaServer();

        LocalizationContext lc = createMock(LocalizationContext.class);
        expect(lc.getCurrentLanguage()).andReturn("en_US");
        replay(lc);
        mediaServer.setLocalizationContext(lc);
        
        AddressManager addressManager = createMock(AddressManager.class);
        addressManager.getSingleAddress(FreeswitchFeature.SIP_ADDRESS);
        expectLastCall().andReturn(new Address(FreeswitchFeature.SIP_ADDRESS, "zzz.example.org", 4444)).anyTimes();
        replay(addressManager);        
        mediaServer.setAddressManager(addressManager);

        rule.setMediaServer(mediaServer);
        rule.setName("abc");
        rule.setExtension("100");
        rule.setAttendantAliases("0 operator");
        rule.setEnabled(true);

        RulesXmlFile mappingRules = new MappingRules();
        mappingRules.setDomainName("example.org");
        List<DialingRule> rules = new ArrayList<DialingRule>();
        rule.appendToGenerationRules(rules);
        mappingRules.begin();
        for (DialingRule r : rules) {
            mappingRules.generate(r);
        }
        mappingRules.end();
        
        StringWriter actual = new StringWriter();
        mappingRules.write(actual);
        InputStream referenceXmlStream = getClass().getResourceAsStream("sipxivr-aa-rules.test.xml");
        assertEquals(IOUtils.toString(referenceXmlStream).trim(), actual.toString().trim());
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
