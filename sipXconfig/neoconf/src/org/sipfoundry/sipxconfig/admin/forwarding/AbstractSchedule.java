/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.forwarding;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.TimeZone;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.ScheduledDay;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime.WorkingHours;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;

public abstract class AbstractSchedule extends BeanWithId {
    private User m_user;
    private String m_name;
    private String m_description;
    private WorkingTime m_workingTime;

    public AbstractSchedule() {

    }

    public User getUser() {
        return m_user;
    }

    public void setUser(User user) {
        m_user = user;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        this.m_name = name;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        this.m_description = description;
    }

    public WorkingTime getWorkingTime() {
        return m_workingTime;
    }

    public void setWorkingTime(WorkingTime workingTime) {
        this.m_workingTime = workingTime;
    }

    public String calculateValidTime() {
        List<String> validTimeList = null;
        String validTime = null;
        TimeZone timeZone = TimeZone.getDefault();
        int timeZoneOffset = timeZone.getOffset((new Date()).getTime());
        Integer timeZoneOffsetInMinutes = new Integer(timeZoneOffset / 1000 / 60);
        validTimeList = new ArrayList<String>();
        WorkingTime workingTime = getWorkingTime();
        WorkingHours[] workingHours = workingTime.getWorkingHours();
        for (WorkingHours wk : workingHours) {
            if (wk != null) {
                ScheduledDay day = wk.getDay();
                if (day != null) {
                    // days here are numbered from 1 to 7 with 1 being Sunday and 7 being
                    // Saturday
                    Integer minutesFromSunday = (day.getDayOfWeek() - 1) * 24 * 60;
                    Integer startHour = Integer.valueOf(wk.getStartHour());
                    Integer stopHour = Integer.valueOf(wk.getStopHour());
                    Integer startMinute = Integer.valueOf(wk.getStartMinute());
                    Integer stopMinute = Integer.valueOf(wk.getStopMinute());
                    Integer startWithoutTimezone = minutesFromSunday + startHour * 60
                            + startMinute;
                    Integer stopWithoutTimezone = minutesFromSunday + stopHour * 60 + stopMinute;

                    if (timeZoneOffsetInMinutes > 0) { // GMT+xx - split into 2 periods if case
                        validTimeList.addAll(getPeriodsIfGMTPlusTimezone(startWithoutTimezone,
                                stopWithoutTimezone, timeZoneOffsetInMinutes));
                    } else if (timeZoneOffsetInMinutes < 0) { // GMT-xx - split into 2 periods
                                                                // if case,
                        validTimeList.addAll(getPeriodsIfGMTMinusTimezone(startWithoutTimezone,
                                stopWithoutTimezone, -timeZoneOffsetInMinutes));
                    } else { // GMT
                        validTimeList.add(Integer.toHexString(startWithoutTimezone));
                        validTimeList.add(Integer.toHexString(stopWithoutTimezone));
                    }
                }
            }
        }
        validTime = StringUtils.join(validTimeList, ":");
        return validTime;
    }

    // for GMT+xx
    // returns the rolled periods if roll happends or the normal period if not
    private List<String> getPeriodsIfGMTPlusTimezone(Integer start, Integer stop,
            Integer timezoneOffset) {
        List<String> result = new ArrayList<String>();
        if (start - timezoneOffset < WorkingHours.MIN_MINUTES) {
            Integer rolledStartMinutes = start - timezoneOffset + WorkingHours.MIN_MINUTES;
            Integer startRolled = WorkingHours.MAX_MINUTES + rolledStartMinutes;
            if (stop - timezoneOffset < WorkingHours.MIN_MINUTES) {
                Integer rolledStopMinutes = stop - timezoneOffset + WorkingHours.MIN_MINUTES;
                Integer stopRolled = WorkingHours.MAX_MINUTES + rolledStopMinutes;
                result.add(Integer.toHexString(startRolled));
                result.add(Integer.toHexString(stopRolled));
            } else {
                result.add(Integer.toHexString(startRolled));
                result.add(Integer.toHexString(WorkingHours.MAX_MINUTES));
                result.add(Integer.toHexString(WorkingHours.MIN_MINUTES));
                result.add(Integer.toHexString(stop - timezoneOffset));
            }
        } else {
            result.add(Integer.toHexString(start - timezoneOffset));
            result.add(Integer.toHexString(stop - timezoneOffset));
        }
        return result;
    }

    // for GMT-xx
    // returns the rolled periods if roll happends or the normal period if not
    private List<String> getPeriodsIfGMTMinusTimezone(Integer start, Integer stop,
            Integer timezoneOffset) {
        List<String> result = new ArrayList<String>();
        if (stop + timezoneOffset > WorkingHours.MAX_MINUTES) {
            Integer rolledStopMinutes = stop + timezoneOffset - WorkingHours.MAX_MINUTES;
            Integer stopRolled = WorkingHours.MIN_MINUTES + rolledStopMinutes;
            if (start + timezoneOffset > WorkingHours.MAX_MINUTES) {
                Integer rolledStartMinutes = start + timezoneOffset - WorkingHours.MAX_MINUTES;
                Integer startRolled = WorkingHours.MIN_MINUTES + rolledStartMinutes;
                result.add(Integer.toHexString(startRolled));
                result.add(Integer.toHexString(stopRolled));
            } else {
                result.add(Integer.toHexString(start + timezoneOffset));
                result.add(Integer.toHexString(WorkingHours.MAX_MINUTES));
                result.add(Integer.toHexString(WorkingHours.MIN_MINUTES));
                result.add(Integer.toHexString(stopRolled));
            }
        } else {
            result.add(Integer.toHexString(start + timezoneOffset));
            result.add(Integer.toHexString(stop + timezoneOffset));
        }
        return result;
    }

    public void checkForValidSchedule() {
        WorkingHours[] workingHours = getWorkingTime().getWorkingHours();
        if (workingHours == null || workingHours.length == 0) {
            throw new ScheduleException();
        }
        for (WorkingHours workingHour : workingHours) {
            if (workingHour.isInvalidPeriod()) {
                throw new InvalidPeriodException();
            }
            if (workingHour.isTheSameHour()) {
                throw new SameStartAndStopHoursException();
            }
        }
        if (overlappingPeriods(workingHours)) {
            throw new OverlappingPeriodsException();
        }
    }

    public boolean overlappingPeriods(WorkingHours[] workingHours) {
        int[] startHours = new int[workingHours.length];
        int[] stopHours = new int[workingHours.length];
        int i = 0;
        for (WorkingHours workingHour : workingHours) {
            ScheduledDay day = workingHour.getDay();
            // days here are numbered from 1 to 7 with 1 being Sunday and 7 being
            // Saturday
            int minutesFromSunday = (day.getDayOfWeek() - 1) * 24 * 60;
            int startHour = Integer.parseInt(workingHour.getStartHour());
            int stopHour = Integer.parseInt(workingHour.getStopHour());
            int startMinute = Integer.parseInt(workingHour.getStartMinute());
            int stopMinute = Integer.parseInt(workingHour.getStopMinute());
            int start = minutesFromSunday + startHour * 60 + startMinute;
            int stop = minutesFromSunday + stopHour * 60 + stopMinute;
            startHours[i] = start;
            stopHours[i] = stop;
            i++;
        }

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

    static class OverlappingPeriodsException extends UserException {
        private static final String ERROR = 
            "Periods defined overlap. Please modify them in order to proceed";

        OverlappingPeriodsException() {
            super(ERROR);
        }
    }

    static class InvalidPeriodException extends UserException {
        private static final String ERROR = 
            "Invalid period.The start time of one of the periods is after the stop time.";

        InvalidPeriodException() {
            super(ERROR);
        }
    }

    static class SameStartAndStopHoursException extends UserException {
        private static final String ERROR = 
            "Invalid period.The start time equals the stop time.";

        SameStartAndStopHoursException() {
            super(ERROR);
        }
    }

    static class ScheduleException extends UserException {
        private static final String ERROR = 
            "Invalid schedule. Schedule must have at least one period defined.";

        ScheduleException() {
            super(ERROR);
        }
    }
}
