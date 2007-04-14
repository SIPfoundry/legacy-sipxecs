/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;

import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.sipfoundry.sipxconfig.XmlUnitHelper;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantRule;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendant;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.Holiday;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.ScheduledAttendant;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime.WorkingHours;

public class AttendantScheduleFileTest extends XMLTestCase {
    public AttendantScheduleFileTest() {
        XmlUnitHelper.setNamespaceAware(false);
        XMLUnit.setIgnoreWhitespace(true);
    }

    public void testGenerateEmpty() throws Exception {
        AttendantRule rule = new AttendantRule();
        rule.setName("abc");
        AttendantScheduleFile file = new AttendantScheduleFile();
        file.generate(rule);

        String generatedXml = file.getFileContent();

        String expected = "<schedules><autoattendant/></schedules>";

        assertXMLEqual(expected, generatedXml);
    }

    public void testGenerate() throws Exception {
        AutoAttendant operator = new AutoAttendant();
        operator.setSystemId("operator");

        AutoAttendant after = new AutoAttendant();
        after.setSystemId("afterhour");

        ScheduledAttendant sa = new ScheduledAttendant();
        sa.setAttendant(after);

        DateFormat format = new SimpleDateFormat("MM/dd/yyy");

        Holiday holiday = new Holiday();
        holiday.setAttendant(after);
        holiday.addDay(format.parse("01/01/2005"));
        holiday.addDay(format.parse("06/06/2005"));
        holiday.addDay(format.parse("12/24/2005"));

        WorkingTime wt = new WorkingTime();
        wt.setAttendant(operator);
        WorkingHours[] workingHours = wt.getWorkingHours();
        Date stop = workingHours[4].getStop();
        Calendar calendar = Calendar.getInstance();
        calendar.setTime(stop);
        calendar.add(Calendar.HOUR_OF_DAY, -1);
        workingHours[4].setStop(calendar.getTime());

        AttendantRule rule = new AttendantRule();
        rule.setAfterHoursAttendant(sa);
        rule.setHolidayAttendant(holiday);
        rule.setWorkingTimeAttendant(wt);

        AttendantScheduleFile file = new AttendantScheduleFile();
        file.generate(rule);

        assertEquals("aa_-1-schedule.xml", file.getFileBaseName());

        String generatedXml = file.getFileContent();

        InputStream referenceXmlStream = AttendantScheduleFileTest.class
                .getResourceAsStream("attendant_schedule.test.xml");

        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(generatedXml));
    }
}
