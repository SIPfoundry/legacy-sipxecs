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

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;

import org.dom4j.Document;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantRule;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.Holiday;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.ScheduledAttendant;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime.WorkingHours;

public class AttendantScheduleFile extends XmlFile {
    private static final DateFormat DATE_FORMAT = new SimpleDateFormat("dd-MMM-yyyy", Locale.US);
    private static final String SUFFIX = "-schedule.xml";

    private Document m_document;

    public Document getDocument() {
        if (m_document == null) {
            Element root = XmlFile.FACTORY.createElement("schedules");
            m_document = XmlFile.FACTORY.createDocument(root);
        }
        return m_document;
    }

    void generate(AttendantRule attendantRule) {
        String name = attendantRule.getSystemName() + SUFFIX;
        setName(name);
        Element root = getDocument().getRootElement();
        Element schedule = root.addElement("autoattendant");
        Holiday holidayAttendant = attendantRule.getHolidayAttendant();
        addHoliday(schedule, holidayAttendant);
        WorkingTime workingTimeAttendant = attendantRule.getWorkingTimeAttendant();
        addRegularHours(schedule, workingTimeAttendant);
        ScheduledAttendant afterHoursAttendant = attendantRule.getAfterHoursAttendant();
        addAttendant(schedule, afterHoursAttendant, "afterhours");
    }

    private void addHoliday(Element schedule, Holiday holiday) {
        if (holiday.getAttendant() == null) {
            return;
        }
        Element holidays = addAttendant(schedule, holiday, "holidays");
        List dates = holiday.getDates();
        for (Iterator i = dates.iterator(); i.hasNext();) {
            Date date = (Date) i.next();
            holidays.addElement("date").setText(DATE_FORMAT.format(date));
        }
    }

    private void addRegularHours(Element schedule, WorkingTime workingTime) {
        if (workingTime.getAttendant() == null) {
            return;
        }
        Element regularHours = addAttendant(schedule, workingTime, "regularhours");
        WorkingHours[] workingHours = workingTime.getWorkingHours();
        for (int i = 0; i < workingHours.length; i++) {
            WorkingHours whs = workingHours[i];
            if (!whs.isEnabled()) {
                continue;
            }
            Element day = regularHours.addElement(whs.getDay().getName().toLowerCase());
            day.addElement("from").setText(whs.getStartTime());
            day.addElement("to").setText(whs.getStopTime());
        }
    }

    private Element addAttendant(Element parent, ScheduledAttendant sa, String name) {
        if (sa.getAttendant() == null) {
            return null;
        }
        Element ah = parent.addElement(name);
        ah.addElement("filename").setText(sa.getAttendant().getSystemName());
        return ah;
    }
}
