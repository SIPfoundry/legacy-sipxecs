/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.dialplan.attendant;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.io.StringWriter;
import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Collections;
import java.util.Date;
import java.util.Locale;

import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.sipfoundry.commons.util.HolidayPeriod;
import org.sipfoundry.sipxconfig.common.DialPad;
import org.sipfoundry.sipxconfig.dialplan.AttendantMenu;
import org.sipfoundry.sipxconfig.dialplan.AttendantMenuAction;
import org.sipfoundry.sipxconfig.dialplan.AttendantRule;
import org.sipfoundry.sipxconfig.dialplan.AutoAttendant;
import org.sipfoundry.sipxconfig.dialplan.AutoAttendantManager;
import org.sipfoundry.sipxconfig.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.dialplan.config.XmlFile;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class AutoAttendantsConfigTest extends XMLTestCase {

    private DomainManager m_domainManager;

    @Override
    protected void setUp() throws Exception {
        XMLUnit.setIgnoreWhitespace(true);
        m_domainManager = createMock(DomainManager.class);
        m_domainManager.getDomainName();
        expectLastCall().andReturn("example.org").atLeastOnce();
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

        AutoAttendantXmlConfig autoAttendantsConfig = new AutoAttendantXmlConfig();
        autoAttendantsConfig.setDomainManager(m_domainManager);
        autoAttendantsConfig.setDialPlanContext(dialPlanContext);
        autoAttendantsConfig.setAutoAttendantManager(aam);
        
        String actual = toString(autoAttendantsConfig);
        InputStream referenceXml = getClass().getResourceAsStream("empty-autoattendants.test.xml");

        assertXMLEqual(new InputStreamReader(referenceXml), new StringReader(actual.toString()));

        verify(dialPlanContext, aam);
    }
    
    private String toString(AutoAttendantXmlConfig config) throws IOException {
        StringWriter w = new StringWriter();
        XmlFile f = new XmlFile(w);
        f.write(config.getDocument());
        return w.toString();        
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

        aa.setSettingValue(AutoAttendant.ON_TRANSFER_PLAY_PROMPT, "1");

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

        AutoAttendantXmlConfig autoAttendantsConfig = new AutoAttendantXmlConfig();
        autoAttendantsConfig.setDomainManager(m_domainManager);
        autoAttendantsConfig.setDialPlanContext(dialPlanContext);
        autoAttendantsConfig.setAutoAttendantManager(aam);

        String generatedXml = toString(autoAttendantsConfig);
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

        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd HH:mm", Locale.US);
        Holiday holiday = new Holiday();
        holiday.addPeriod(getNewHolidayPeriod(format.parse("2010-05-03 00:00"), format.parse("2010-05-03 23:59")));
        holiday.addPeriod(getNewHolidayPeriod(format.parse("2010-07-04 00:00"), format.parse("2010-07-04 23:59")));
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

        AutoAttendantXmlConfig autoAttendantsConfig = new AutoAttendantXmlConfig();
        autoAttendantsConfig.setDomainManager(m_domainManager);
        autoAttendantsConfig.setAutoAttendantManager(aam);
        autoAttendantsConfig.setDialPlanContext(dialPlanContext);

        String generatedXml = toString(autoAttendantsConfig);
        InputStream referenceXml = getClass().getResourceAsStream("schedules-autoattendants.test.xml");

        assertXMLEqual(new InputStreamReader(referenceXml), new StringReader(generatedXml));

        verify(dialPlanContext, aam);
    }

    private HolidayPeriod getNewHolidayPeriod(Date startDate, Date endDate) {
        HolidayPeriod holidayPeriod = new HolidayPeriod();
        holidayPeriod.setStartDate(startDate);
        holidayPeriod.setEndDate(endDate);
        return holidayPeriod;
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

        aa.setSettingValue(AutoAttendant.ON_TRANSFER_PLAY_PROMPT, "0");

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

        AutoAttendantXmlConfig autoAttendantsConfig = new AutoAttendantXmlConfig();
        autoAttendantsConfig.setDomainManager(m_domainManager);
        autoAttendantsConfig.setAutoAttendantManager(aam);
        autoAttendantsConfig.setDialPlanContext(dialPlanContext);

        String generatedXml = toString(autoAttendantsConfig);
        InputStream referenceXml = getClass().getResourceAsStream("nullparameter-autoattendants.test.xml");
        assertXMLEqual(new InputStreamReader(referenceXml), new StringReader(generatedXml));
        verify(dialPlanContext, aam);
    }
}
