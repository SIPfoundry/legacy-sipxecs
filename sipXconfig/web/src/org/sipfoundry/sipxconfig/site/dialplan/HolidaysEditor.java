/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.dialplan;

import java.util.Calendar;
import java.util.Collections;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.form.ListEditMap;
import org.sipfoundry.commons.util.HolidayPeriod;
import org.sipfoundry.sipxconfig.dialplan.attendant.Holiday;

public abstract class HolidaysEditor extends BaseComponent {
    public abstract Holiday getHoliday();

    public abstract int getDayIndex();

    public abstract void setDayIndex(int i);

    public abstract int getMaxDayIndex();

    public abstract void setMaxDayIndex(int i);

    public abstract HolidayPeriod getNewPeriod();

    public abstract void setNewDay(Date day);

    public abstract ListEditMap getListEditMap();

    public abstract void setListEditMap(ListEditMap map);

    public Date getHolidayStartDay() {
        return getHoliday().getPeriod(getDayIndex()).getStartDate();
    }

    public void setHolidayStartDay(Date day) {
        synchronizeDateTime(getHolidayStartDay(), day);
        getHoliday().getPeriod(getDayIndex()).setStartDate(day);
    }

    public Date getHolidayStartTime() {
        return getHolidayStartDay();
    }

    public void setHolidayStartTime(Date time) {
        Calendar calendar = synchronizeDateTime(getHolidayStartDay(), time);
        setHolidayStartDay(calendar.getTime());
    }

    public Date getHolidayEndDay() {
        return getHoliday().getPeriod(getDayIndex()).getEndDate();
    }

    public void setHolidayEndDay(Date day) {
        synchronizeDateTime(getHolidayEndDay(), day);
        getHoliday().getPeriod(getDayIndex()).setEndDate(day);
    }

    public Date getHolidayEndTime() {
        return getHolidayEndDay();
    }

    public void setHolidayEndTime(Date time) {
        Calendar calendar = synchronizeDateTime(getHolidayEndDay(), time);
        setHolidayEndDay(calendar.getTime());
    }

    private Calendar synchronizeDateTime(Date currentDate, Date newDate) {
        Calendar holidayCal = new GregorianCalendar();
        holidayCal.setTime(currentDate);
        Calendar currentCal = new GregorianCalendar();
        currentCal.setTime(newDate);

        holidayCal.set(Calendar.HOUR_OF_DAY,
                currentCal.get(Calendar.HOUR_OF_DAY));
        holidayCal.set(Calendar.MINUTE, currentCal.get(Calendar.MINUTE));
        return holidayCal;
    }

    /**
     * This listener is used to adjuse max day index. If we are rewinding we could have started
     * with Holiday object that had more days that we needed. Keeping track of max index lets us
     * remove unnecessary days.
     *
     */
    public void onDayRender() {
        Integer index = (Integer) getListEditMap().getKey();
        setMaxDayIndex(index.intValue());
    }

    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        setNewDay(null);
        setMaxDayIndex(-1);
        initListEditMap();
        super.renderComponent(writer, cycle);
        if (cycle.isRewinding()) {
            adjustPeriods();
            addPeriod();
            removePeriod();
        }
    }

    private void adjustPeriods() {
        Holiday holiday = getHoliday();
        holiday.chop(getMaxDayIndex());
    }

    private void removePeriod() {
        Holiday holiday = getHoliday();
        ListEditMap map = getListEditMap();
        List deletedKeys = map.getDeletedKeys();
        Collections.sort(deletedKeys);
        for (int i = deletedKeys.size() - 1; i >= 0; i--) {
            Integer index = (Integer) deletedKeys.get(i);
            holiday.removeDay(index.intValue());
        }
    }

    private void addPeriod() {
        Holiday holiday = getHoliday();
        HolidayPeriod holidayPeriod = getNewPeriod();
        if (holidayPeriod != null) {
            holiday.addPeriod(holidayPeriod);
        }
    }

    private void initListEditMap() {
        ListEditMap map = new ListEditMap();
        List<HolidayPeriod> dates = getHoliday().getPeriods();
        for (int i = 0; i < dates.size(); i++) {
            map.add(new Integer(i), dates.get(i));
        }
        setListEditMap(map);
    }
}
