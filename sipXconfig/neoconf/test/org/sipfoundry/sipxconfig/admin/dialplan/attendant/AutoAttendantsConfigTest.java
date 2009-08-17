/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin.dialplan.attendant;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Collections;
import java.util.Locale;

import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantMenu;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantMenuAction;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantRule;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendant;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendantManager;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.common.DialPad;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.sipfoundry.sipxconfig.admin.AbstractConfigurationFile.getFileContent;

public class AutoAttendantsConfigTest extends XMLTestCase {

    private DomainManager m_domainManager;

    @Override
    protected void setUp() throws Exception {
        XMLUnit.setIgnoreWhitespace(true);

        Domain domain = new Domain();
        domain.setName("example.org");
        m_domainManager = createMock(DomainManager.class);
        m_domainManager.getDomain();
        expectLastCall().andReturn(domain).atLeastOnce();
        replay(m_domainManager);
    }

    public void testGenerateEmpty() throws Exception {
        DialPlanContext dialPlanContext = createMock(DialPlanContext.class);
        dialPlanContext.getAttendantRules();
        expectLastCall().andReturn(Collections.emptyList());

        AutoAttendantManager aam = createMock(AutoAttendantManager.class);
        aam.getAutoAttendants();
        expectLastCall().andReturn(Collections.emptyList());

        replay(dialPlanContext, aam);

        AutoAttendantsConfig autoAttendantsConfig = new AutoAttendantsConfig();
        autoAttendantsConfig.setDomainManager(m_domainManager);
        autoAttendantsConfig.setDialPlanContext(dialPlanContext);
        autoAttendantsConfig.setAutoAttendantManager(aam);

        String generatedXml = getFileContent(autoAttendantsConfig, null);
        InputStream referenceXml = getClass().getResourceAsStream("empty-autoattendants.test.xml");

        assertXMLEqual(new InputStreamReader(referenceXml), new StringReader(generatedXml));

        verify(dialPlanContext, aam);
    }

    public void testGenerateAutoAttendants() throws Exception {
        AutoAttendant operator = new AutoAttendant();
        operator.setUniqueId();
        operator.setPromptsDirectory("prompts/");
        operator.setModelFilesContext(TestHelper.getModelFilesContext());
        operator.setSystemId(AutoAttendant.OPERATOR_ID);
        operator.setName("operator");
        operator.setPrompt("operator.wav");
        operator.resetToFactoryDefault();

        AutoAttendant aa = new AutoAttendant();
        aa.setName("abc");
        aa.setModelFilesContext(TestHelper.getModelFilesContext());
        aa.setPromptsDirectory("prompts/");
        aa.setPrompt("prompt.wav");

        AttendantMenu menu = new AttendantMenu();
        menu.addMenuItem(DialPad.NUM_0, AttendantMenuAction.AUTO_ATTENDANT, "afterhours");
        menu.addMenuItem(DialPad.NUM_1, AttendantMenuAction.DISCONNECT);
        menu.addMenuItem(DialPad.NUM_2, AttendantMenuAction.TRANSFER_OUT, "1234");
        aa.setMenu(menu);

        aa.setSettingValue("onfail/transfer", "1");
        aa.setSettingValue("onfail/transfer-extension", "999");
        aa.setSettingValue("onfail/transfer-prompt", "test.wav");

        AutoAttendantManager aam = createMock(AutoAttendantManager.class);
        aam.getAutoAttendants();
        expectLastCall().andReturn(Arrays.asList(operator, aa));
        aam.getSpecialMode();
        expectLastCall().andReturn(true).anyTimes();
        aam.getSelectedSpecialAttendant();
        expectLastCall().andReturn(aa).anyTimes();

        DialPlanContext dialPlanContext = createMock(DialPlanContext.class);
        dialPlanContext.getAttendantRules();
        expectLastCall().andReturn(Collections.emptyList());
        dialPlanContext.getVoiceMail();
        expectLastCall().andReturn("101");

        replay(dialPlanContext, aam);

        AutoAttendantsConfig autoAttendantsConfig = new AutoAttendantsConfig();
        autoAttendantsConfig.setDomainManager(m_domainManager);
        autoAttendantsConfig.setDialPlanContext(dialPlanContext);
        autoAttendantsConfig.setAutoAttendantManager(aam);

        String generatedXml = getFileContent(autoAttendantsConfig, null);
        InputStream referenceXml = getClass().getResourceAsStream("autoattendants.test.xml");
        assertXMLEqual(new InputStreamReader(referenceXml), new StringReader(generatedXml));

        verify(dialPlanContext, aam);
    }

    public void testGenerateSchedules() throws Exception {
        AutoAttendant operator = new AutoAttendant();
        operator.setPromptsDirectory("prompts/");
        operator.setModelFilesContext(TestHelper.getModelFilesContext());
        operator.setSystemId(AutoAttendant.OPERATOR_ID);
        operator.setName("operator");
        operator.setPrompt("operator.wav");

        AttendantRule attendantRule = new AttendantRule();
        ScheduledAttendant sa = new ScheduledAttendant();
        sa.setAttendant(operator);
        attendantRule.setAfterHoursAttendant(sa);

        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd", Locale.US);
        Holiday holiday = new Holiday();
        holiday.addDay(format.parse("2010-05-03"));
        holiday.addDay(format.parse("2010-07-04"));
        holiday.setAttendant(operator);
        attendantRule.setHolidayAttendant(holiday);

        WorkingTime workingTime = new WorkingTime();
        workingTime.setAttendant(operator);
        attendantRule.setWorkingTimeAttendant(workingTime);

        AutoAttendantManager aam = createMock(AutoAttendantManager.class);
        aam.getAutoAttendants();
        expectLastCall().andReturn(Arrays.asList(operator));
        aam.getSpecialMode();
        expectLastCall().andReturn(false).anyTimes();

        DialPlanContext dialPlanContext = createMock(DialPlanContext.class);
        dialPlanContext.getAttendantRules();
        expectLastCall().andReturn(Arrays.asList(attendantRule));

        replay(dialPlanContext, aam);

        AutoAttendantsConfig autoAttendantsConfig = new AutoAttendantsConfig();
        autoAttendantsConfig.setDomainManager(m_domainManager);
        autoAttendantsConfig.setAutoAttendantManager(aam);
        autoAttendantsConfig.setDialPlanContext(dialPlanContext);

        String generatedXml = getFileContent(autoAttendantsConfig, null);
        InputStream referenceXml = getClass().getResourceAsStream("schedules-autoattendants.test.xml");

        assertXMLEqual(new InputStreamReader(referenceXml), new StringReader(generatedXml));

        verify(dialPlanContext, aam);
    }

    public void testGenerateAutoAttendantsWithNullMenuEntry() throws Exception {

        AutoAttendant operator = new AutoAttendant();
        operator.setUniqueId();
        operator.setPromptsDirectory("prompts/");
        operator.setModelFilesContext(TestHelper.getModelFilesContext());
        operator.setSystemId(AutoAttendant.OPERATOR_ID);
        operator.setName("operator");
        operator.setPrompt("operator.wav");
        operator.resetToFactoryDefault();

        AutoAttendant aa = new AutoAttendant();
        aa.setName("abc");
        aa.setModelFilesContext(TestHelper.getModelFilesContext());
        aa.setPromptsDirectory("prompts/");
        aa.setPrompt("prompt.wav");

        AttendantMenu menu = new AttendantMenu();
        menu.addMenuItem(DialPad.NUM_0, AttendantMenuAction.AUTO_ATTENDANT, null);
        menu.addMenuItem(DialPad.NUM_1, AttendantMenuAction.DISCONNECT);
        menu.addMenuItem(DialPad.NUM_2, AttendantMenuAction.TRANSFER_OUT, null);
        aa.setMenu(menu);

        aa.setSettingValue("onfail/transfer", "1");
        aa.setSettingValue("onfail/transfer-extension", "999");
        aa.setSettingValue("onfail/transfer-prompt", "test.wav");

        AutoAttendantManager aam = createMock(AutoAttendantManager.class);
        aam.getAutoAttendants();
        expectLastCall().andReturn(Arrays.asList(operator, aa));
        aam.getSpecialMode();
        expectLastCall().andReturn(true).anyTimes();
        aam.getSelectedSpecialAttendant();
        expectLastCall().andReturn(aa).anyTimes();

        DialPlanContext dialPlanContext = createMock(DialPlanContext.class);
        dialPlanContext.getAttendantRules();
        expectLastCall().andReturn(Collections.emptyList());
        dialPlanContext.getVoiceMail();
        expectLastCall().andReturn("101");

        replay(dialPlanContext, aam);

        AutoAttendantsConfig autoAttendantsConfig = new AutoAttendantsConfig();
        autoAttendantsConfig.setDomainManager(m_domainManager);
        autoAttendantsConfig.setAutoAttendantManager(aam);
        autoAttendantsConfig.setDialPlanContext(dialPlanContext);

        String generatedXml = getFileContent(autoAttendantsConfig, null);
        InputStream referenceXml = getClass().getResourceAsStream("nullparameter-autoattendants.test.xml");
        assertXMLEqual(new InputStreamReader(referenceXml), new StringReader(generatedXml));
        verify(dialPlanContext, aam);
    }
}
