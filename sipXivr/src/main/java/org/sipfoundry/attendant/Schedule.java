/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.attendant;

import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;

import org.apache.log4j.Logger;
import org.w3c.dom.Node;

public class Schedule {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");

    class TimeRange {
        private Date m_from; // Starts at this time (inclusive) (Only time part is used, date is ignored)
        private Date m_to; // Ends one second before this time
        public Date getFrom() {
            return m_from;
        }
        public void setFrom(Date from) {
            m_from = from;
        }
        public Date getTo() {
            return m_to;
        }
        public void setTo(Date to) {
            m_to = to;
        }
    }

    class Day {
        private int m_dayOfWeek; // As defined in Calendar
        private TimeRange m_range; // Time during that day
        public int getDayOfWeek() {
            return m_dayOfWeek;
        }
        public void setDayOfWeek(int dayOfWeek) {
            m_dayOfWeek = dayOfWeek;
        }
        public TimeRange getRange() {
            return m_range;
        }
        public void setRange(TimeRange range) {
            m_range = range;
        }
    }

    class Holidays {
        private ArrayList<Date> m_dates = new ArrayList<Date>(); // Dates that are holidays
        private String m_id; // Attendant id to run
        
        public void add(Date date) {
            m_dates.add(date);
        }
        public ArrayList<Date> getDates() {
            return m_dates;
        }
        public void setDates(ArrayList<Date> dates) {
            m_dates = dates;
        }
        public String getAttendantId() {
            return m_id;
        }
        public void setAttendantId(String attendantId) {
            m_id = attendantId;
        }
    }

    class Hours {
        private ArrayList<Day> m_days = new ArrayList<Day>(); // The regular hours
        private String m_regularHoursAttendantId; // Attendant name to run during regular hours
        private String m_afterHoursAttendantId; // Attendant name to run not during regular hours
        public void add(Day day) {
            m_days.add(day);
        }
        public ArrayList<Day> getDays() {
            return m_days;
        }
        public void setDays(ArrayList<Day> days) {
            m_days = days;
        }
        public String getRegularHoursAttendantId() {
            return m_regularHoursAttendantId;
        }
        public void setRegularHoursAttendantId(String regularHoursAttendantId) {
            m_regularHoursAttendantId = regularHoursAttendantId;
        }
        public String getAfterHoursAttendantId() {
            return m_afterHoursAttendantId;
        }
        public void setAfterHoursAttendantId(String afterHoursAttendantId) {
            m_afterHoursAttendantId = afterHoursAttendantId;
        }
    }

	private String m_id;
    private Holidays m_holidays;
    private Hours m_hours;

    Schedule() {
        m_holidays = new Holidays();
        m_hours = new Hours();
    }

    /**
     * Load the schedule for a particular day
     * 
     * @param dayOfWeek
     * @param node
     * @throws ParseException
     */
    private void loadDay(int dayOfWeek, Node node) throws ParseException {
        DateFormat timeFormat = new SimpleDateFormat("HH:mm");

        Day day = new Day();
        day.m_range = new TimeRange();
        day.m_dayOfWeek = dayOfWeek;

        Node fromTo = node.getFirstChild();
        while (fromTo != null) {
            if (fromTo.getNodeType() == Node.ELEMENT_NODE) {
                String name = fromTo.getNodeName();
                if (name.contentEquals("from")) {
                    String fromString = fromTo.getTextContent().trim();
                    day.m_range.m_from = timeFormat.parse(fromString);
                } else if (name.contentEquals("to")) {
                    String toString = fromTo.getTextContent().trim();
                    day.m_range.m_to = timeFormat.parse(toString);
                }
            }
            fromTo = fromTo.getNextSibling();
        }
        m_hours.m_days.add(day);
    }

    /**
     * Load the schedules from an XML node
     * 
     * @param schedule
     */
    void loadSchedule(Node schedule) {
        String parm = "unknown";
        DateFormat dateFormat = new SimpleDateFormat("dd-MMM-yyyy");
        try {
        	m_id = schedule.getAttributes().getNamedItem("id").getNodeValue();

            for (Node scheduleNode = schedule.getFirstChild(); scheduleNode != null; scheduleNode = scheduleNode
                    .getNextSibling()) {

                if (scheduleNode.getNodeType() != Node.ELEMENT_NODE) {
                    continue;
                }
                // First find the Holidays
                if (scheduleNode.getNodeName().equals(parm = "holiday")) {
                    for (Node next = scheduleNode.getFirstChild(); next != null; next = next
                            .getNextSibling()) {
                        if (next.getNodeType() == Node.ELEMENT_NODE) {
                            String name = next.getNodeName();
                            if (name.contentEquals("id")) {
                                m_holidays.m_id = next.getTextContent().trim();
                            } else if (name.contentEquals("date")) {
                                String dateString = next.getTextContent().trim();
                                Date date = dateFormat.parse(dateString);
                                m_holidays.m_dates.add(date);
                            }
                        }
                    }
                    continue;
                }

                // Then the regular hours
                if (scheduleNode.getNodeName().equals(parm = "regularhours")) {
                    for (Node next = scheduleNode.getFirstChild(); next != null; next = next
                            .getNextSibling()) {
                        if (next.getNodeType() == Node.ELEMENT_NODE) {
                            String name = next.getNodeName();
                            if (name.contentEquals("id")) {
                                m_hours.m_regularHoursAttendantId = next.getTextContent()
                                        .trim();
                            } else if (name.contentEquals("monday")) {
                                loadDay(Calendar.MONDAY, next);
                            } else if (name.contentEquals("tuesday")) {
                                loadDay(Calendar.TUESDAY, next);
                            } else if (name.contentEquals("wednesday")) {
                                loadDay(Calendar.WEDNESDAY, next);
                            } else if (name.contentEquals("thursday")) {
                                loadDay(Calendar.THURSDAY, next);
                            } else if (name.contentEquals("friday")) {
                                loadDay(Calendar.FRIDAY, next);
                            } else if (name.contentEquals("saturday")) {
                                loadDay(Calendar.SATURDAY, next);
                            } else if (name.contentEquals("sunday")) {
                                loadDay(Calendar.SUNDAY, next);
                            }
                        }
                    }
                    continue;
                }

                // Then the after hours
                if (scheduleNode.getNodeName().equals(parm = "afterhours")) {
                    for (Node next = scheduleNode.getFirstChild(); next != null; next = next
                            .getNextSibling()) {
                        if (next.getNodeType() == Node.ELEMENT_NODE) {
                            String name = next.getNodeName();
                            if (name.contentEquals("id")) {
                                m_hours.m_afterHoursAttendantId = next.getTextContent().trim();
                            }
                        }
                    }
                    continue;
                }
            }
        } catch (Throwable t) {
            LOG.error("Schedule::loadSchedule Trouble with schedules section " + parm, t);
        }
    }


    public String getId() {
    	return m_id;
    }
    
    /**
     * Determine which Attendant name is in effect at a particular Date or time
     * 
     * @param date
     * @return
     */
    public String getAttendant(Date date) {
        // Check the schedule to see which AA to use.
        Calendar nowCalendar = Calendar.getInstance();
        nowCalendar.setTime(date);

        // First check if date is a holiday
        for (Date from : m_holidays.m_dates) {
            // Test if date >= from
            if (!date.before(from)) {
                // Add a day to the start to get the end
                Calendar calendar = Calendar.getInstance();
                calendar.setTime(from);
                calendar.add(Calendar.DAY_OF_WEEK, 1);
                Date to = calendar.getTime();
                // Test if nowDate < to 
                if (date.before(to)) {
                    // Yep, it is
                    DateFormat dateFormat = new SimpleDateFormat("dd-MMM-yyyy");

                    LOG.info("Schedule::getAttendant Using holiday AutoAttendant as " + dateFormat.format(from)
                            + " is a holiday");
                    return m_holidays.m_id;
                }
            }
        }

        DateFormat timeFormat = new SimpleDateFormat("EEE HH:mm");
        Calendar justTime = Calendar.getInstance();
        justTime.clear();
        justTime.set(Calendar.HOUR_OF_DAY, nowCalendar.get(Calendar.HOUR_OF_DAY));
        justTime.set(Calendar.MINUTE, nowCalendar.get(Calendar.MINUTE));
        Date time = justTime.getTime();
        // Next, check if date is within regular hours
        for (Day day : m_hours.m_days) {
            // Check if the day is the same
            if (day.m_dayOfWeek == nowCalendar.get(Calendar.DAY_OF_WEEK)) {
                // Then check if the hours match too.
                if (!time.before(day.m_range.m_from)) {
                    if (time.before(day.m_range.m_to)) {
                        // Yep, it does
                        LOG.info("Schedule::getAttendant Using regular hours AutoAttendant as "
                                + timeFormat.format(date) + " is regular hours");
                        return m_hours.m_regularHoursAttendantId;
                    }
                }

            }
        }

        // Nope.  Must be after hours
        LOG.info("Schedule::getAttendant Using after hours AutoAttendant as " + timeFormat.format(date)
                + " is after hours");
        return m_hours.m_afterHoursAttendantId;

    }

    public Holidays getHolidays() {
        return m_holidays;
    }

    public void setHolidays(Holidays holidays) {
        m_holidays = holidays;
    }

    public Hours getHours() {
        return m_hours;
    }

    public void setHours(Hours hours) {
        m_hours = hours;
    }
}
