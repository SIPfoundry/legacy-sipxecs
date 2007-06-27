/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan.attendant;

import java.io.Serializable;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.List;
import java.util.Locale;
import java.util.TimeZone;

import org.sipfoundry.sipxconfig.admin.ScheduledDay;
import org.sipfoundry.sipxconfig.common.UserException;

public class WorkingTime extends ScheduledAttendant {
    private WorkingHours[] m_workingHours;

    /**
     * Initialization is a bit tricky - days here are numbered from 0 to 6, with - 0 being Monday
     * and 6 being Sunday. Days in getScheduleDay and in Calenar object are number from 1 to 7
     * with 1 being Sunday and 7 being Saturday.
     * 
     */
    public WorkingTime() {
        final int days = ScheduledDay.DAYS_OF_WEEK.length;
        final int lastWorkingDay = Calendar.FRIDAY - Calendar.MONDAY;
        m_workingHours = new WorkingHours[days];
        for (int i = 0; i < days; i++) {
            WorkingHours whs = new WorkingHours();
            int dayOfWeek = (i + Calendar.SUNDAY) % days + 1;
            whs.setDay(ScheduledDay.getScheduledDay(dayOfWeek));
            whs.setEnabled(i <= lastWorkingDay);
            m_workingHours[i] = whs;
        }
    }

    public WorkingHours[] getWorkingHours() {
        return m_workingHours;
    }

    public void setWorkingHours(WorkingHours[] workingHours) {
        m_workingHours = workingHours;
    }

    public Object clone() throws CloneNotSupportedException {
        WorkingTime clone = (WorkingTime) super.clone();
        clone.m_workingHours = m_workingHours.clone();
        return clone;
    }

    public List<Integer> calculateValidTime(TimeZone timeZone) {
        int timeZoneOffset = timeZone.getOffset((new Date()).getTime());
        int timeZoneOffsetInMinutes = timeZoneOffset / 1000 / 60;
        return calculateValidTimes(timeZoneOffsetInMinutes);
    }

    private List<Integer> calculateValidTimes(int timeZoneOffsetInMinutes) {
        WorkingHours[] workingHours = getWorkingHours();
        List<Integer> validTimeList = new ArrayList<Integer>();
        for (WorkingHours wk : workingHours) {
            wk.addMinutesFromSunday(validTimeList, timeZoneOffsetInMinutes);
        }
        return validTimeList;
    }

    public void checkValid() {
        for (WorkingHours workingHour : m_workingHours) {
            if (workingHour.isInvalidPeriod()) {
                throw new InvalidPeriodException();
            }
            if (workingHour.isTheSameHour()) {
                throw new SameStartAndStopHoursException();
            }
        }
        if (overlappingPeriods()) {
            throw new OverlappingPeriodsException();
        }
    }

    public boolean overlappingPeriods() {
        List<Integer> times = calculateValidTimes(0);
        int hoursLen = times.size() / 2;
        int[] startHours = new int[hoursLen];
        int[] stopHours = new int[hoursLen];
        for (int i = 0; i < hoursLen; i++) {
            int j = 2 * i;
            startHours[i] = times.get(j);
            stopHours[i] = times.get(j + 1);
        }

        // What's this!?
        for (int j = 0; j < startHours.length; j++) {
            for (int k = 0; k < startHours.length; k++) {
                if (j != k) {
                    if ((startHours[j] >= startHours[k] && startHours[j] < stopHours[k])
                            || (stopHours[j] > startHours[k] && stopHours[j] <= stopHours[k])) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    public static class WorkingHours implements Serializable {
        public static final int DEFAULT_START = 9;
        public static final int DEFAULT_STOP = 18;
        public static final Integer MAX_MINUTES = 10080;

        public static final DateFormat TIME_FORMAT = new SimpleDateFormat("HH:mm", Locale.US);

        static {
            TIME_FORMAT.setTimeZone(getGmtTimeZone());
        }

        private boolean m_enabled;
        private ScheduledDay m_day;
        private Date m_start;
        private Date m_stop;

        public WorkingHours() {
            Calendar calendar = Calendar.getInstance(getGmtTimeZone());
            calendar.set(Calendar.HOUR_OF_DAY, DEFAULT_START);
            calendar.set(Calendar.MINUTE, 0);
            m_start = calendar.getTime();
            calendar.set(Calendar.HOUR_OF_DAY, DEFAULT_STOP);
            m_stop = calendar.getTime();
        }

        public void setEnabled(boolean enabled) {
            m_enabled = enabled;
        }

        public boolean isEnabled() {
            return m_enabled;
        }

        public void setDay(ScheduledDay day) {
            m_day = day;
        }

        public ScheduledDay getDay() {
            return m_day;
        }

        public String getStartTime() {
            return formatTime(m_start);
        }

        public String getStopTime() {
            return formatTime(m_stop);
        }

        /**
         * @return UTC time
         */
        public Date getStart() {
            return m_start;
        }

        /**
         * @param start UTC time
         */
        public void setStart(Date start) {
            m_start = start;
        }

        public Date getStop() {
            return m_stop;
        }

        public void setStop(Date stop) {
            m_stop = stop;
        }

        /**
         * Formats time as 24 hours (00:00-23:59), GMT time.
         * 
         * @param date date to format - needs to be GMT time zon
         * @return formatted time string
         */
        private static String formatTime(Date date) {
            return TIME_FORMAT.format(date);
        }

        private static TimeZone getGmtTimeZone() {
            return TimeZone.getTimeZone("GMT");
        }

        public static int getHour(Date date) {
            Calendar cal = Calendar.getInstance(getGmtTimeZone());
            cal.setTime(date);
            return cal.get(Calendar.HOUR_OF_DAY);
        }

        public static int getMinute(Date date) {
            Calendar cal = Calendar.getInstance(getGmtTimeZone());
            cal.setTime(date);
            return cal.get(Calendar.MINUTE);
        }

        public boolean isInvalidPeriod() {
            if (getStart().after(getStop())) {
                return true;
            }
            return false;
        }

        public boolean isTheSameHour() {
            if (getStart().getTime() == getStop().getTime()) {
                return true;
            }
            return false;
        }

        int getMinutesFromMidnight(Date date) {
            return getHour(date) * 60 + getMinute(date);
        }

        void addMinutesFromSunday(List<Integer> minutes, int timeZoneOffsetInMinutes) {
            ScheduledDay day = getDay();
            // days here are numbered from 1 to 7 with 1 being Sunday and 7 being Saturday
            int minutesFromSunday = (day.getDayOfWeek() - 1) * 24 * 60;
            int startMinutes = minutesFromSunday + getMinutesFromMidnight(m_start);
            int stopMinutes = minutesFromSunday + getMinutesFromMidnight(m_stop);

            if (timeZoneOffsetInMinutes > 0) {
                minutes.addAll(getPeriodsIfGMTPlusTimezone(startMinutes, stopMinutes,
                        timeZoneOffsetInMinutes));
            } else if (timeZoneOffsetInMinutes < 0) {
                minutes.addAll(getPeriodsIfGMTMinusTimezone(startMinutes, stopMinutes,
                        -timeZoneOffsetInMinutes));
            } else {
                minutes.add(startMinutes);
                minutes.add(stopMinutes);
            }
        }

        // for GMT-xx
        // returns the rolled periods if roll happends or the normal period if not
        public static List<Integer> getPeriodsIfGMTMinusTimezone(int start, int stop,
                int timezoneOffset) {
            List<Integer> result = new ArrayList<Integer>();
            if (stop + timezoneOffset > MAX_MINUTES) {
                int rolledStopMinutes = stop + timezoneOffset - MAX_MINUTES;
                int stopRolled = rolledStopMinutes;
                if (start + timezoneOffset > MAX_MINUTES) {
                    int rolledStartMinutes = start + timezoneOffset - MAX_MINUTES;
                    int startRolled = rolledStartMinutes;
                    result.add(startRolled);
                    result.add(stopRolled);
                } else {
                    result.add(start + timezoneOffset);
                    result.add(MAX_MINUTES);
                    result.add(0);
                    result.add(stopRolled);
                }
            } else {
                result.add(start + timezoneOffset);
                result.add(stop + timezoneOffset);
            }
            return result;
        }

        // for GMT+xx
        // returns the rolled periods if roll happends or the normal period if not
        public static List<Integer> getPeriodsIfGMTPlusTimezone(int start, int stop,
                int timezoneOffset) {
            List<Integer> result = new ArrayList<Integer>();
            if (start - timezoneOffset < 0) {
                int rolledStartMinutes = start - timezoneOffset;
                int startRolled = MAX_MINUTES + rolledStartMinutes;
                if (stop - timezoneOffset < 0) {
                    int rolledStopMinutes = stop - timezoneOffset;
                    int stopRolled = MAX_MINUTES + rolledStopMinutes;
                    result.add(startRolled);
                    result.add(stopRolled);
                } else {
                    result.add(startRolled);
                    result.add(MAX_MINUTES);
                    result.add(0);
                    result.add(stop - timezoneOffset);
                }
            } else {
                result.add(start - timezoneOffset);
                result.add(stop - timezoneOffset);
            }
            return result;
        }
    }

    public static class OverlappingPeriodsException extends UserException {
        private static final String ERROR = "Periods defined overlap. Please modify them in order to proceed";

        public OverlappingPeriodsException() {
            super(ERROR);
        }
    }

    public static class InvalidPeriodException extends UserException {
        private static final String ERROR = "The start time of one of the periods is after the stop time.";

        public InvalidPeriodException() {
            super(ERROR);
        }
    }

    public static class SameStartAndStopHoursException extends UserException {
        private static final String ERROR = "The start time equals the stop time.";

        public SameStartAndStopHoursException() {
            super(ERROR);
        }
    }
}
