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

import java.util.Collections;
import java.util.Date;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.form.ListEditMap;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.Holiday;

public abstract class HolidaysEditor extends BaseComponent {
    public abstract Holiday getHoliday();

    public abstract int getDayIndex();

    public abstract void setDayIndex(int i);

    public abstract int getMaxDayIndex();

    public abstract void setMaxDayIndex(int i);

    public abstract Date getNewDay();

    public abstract void setNewDay(Date day);

    public abstract ListEditMap getListEditMap();

    public abstract void setListEditMap(ListEditMap map);

    public Date getHolidayDay() {
        return getHoliday().getDay(getDayIndex());
    }

    public void setHolidayDay(Date day) {
        getHoliday().setDay(getDayIndex(), day);
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
            adjustDays();
            addDay();
            removeDay();
        }
    }

    private void adjustDays() {
        Holiday holiday = getHoliday();
        holiday.chop(getMaxDayIndex());
    }

    private void removeDay() {
        Holiday holiday = getHoliday();
        ListEditMap map = getListEditMap();
        List deletedKeys = map.getDeletedKeys();
        Collections.sort(deletedKeys);
        for (int i = deletedKeys.size() - 1; i >= 0; i--) {
            Integer index = (Integer) deletedKeys.get(i);
            holiday.removeDay(index.intValue());
        }
    }

    private void addDay() {
        Holiday holiday = getHoliday();
        Date date = getNewDay();
        if (date != null) {
            holiday.addDay(date);
        }
    }

    private void initListEditMap() {
        ListEditMap map = new ListEditMap();
        List dates = getHoliday().getDates();
        for (int i = 0; i < dates.size(); i++) {
            map.add(new Integer(i), dates.get(i));
        }
        setListEditMap(map);
    }
}
