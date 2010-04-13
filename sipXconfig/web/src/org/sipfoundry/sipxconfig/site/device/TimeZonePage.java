/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.device;

import java.util.Calendar;
import java.util.HashMap;
import java.util.Map;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.admin.TimeOfDay;
import org.sipfoundry.sipxconfig.components.MapSelectionModel;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.device.DeviceTimeZone;
import org.sipfoundry.sipxconfig.device.TimeZoneManager;

public abstract class TimeZonePage extends SipxBasePage implements PageBeginRenderListener {
    @InjectObject(value = "spring:timeZoneManager")
    public abstract TimeZoneManager getTimeZoneManager();

    public abstract void setDeviceTimeZone(DeviceTimeZone dtz);

    public abstract DeviceTimeZone getDeviceTimeZone();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    public abstract boolean isAdvanced();

    public void pageBeginRender(PageEvent event_) {
        setDeviceTimeZone(getTimeZoneManager().getDeviceTimeZone());
        setStartMonth(getDeviceTimeZone().getStartMonth());
        setStopMonth(getDeviceTimeZone().getStopMonth());
        setStartWeek(getDeviceTimeZone().getStartWeek());
        setStopWeek(getDeviceTimeZone().getStopWeek());
        setStartDayOfWeek(getDeviceTimeZone().getStartDayOfWeek());
        setStopDayOfWeek(getDeviceTimeZone().getStopDayOfWeek());
        // construct the time of day
        int time = getDeviceTimeZone().getStartTime();
        setStartTime(new TimeOfDay(time / 60, time % 60));
        time = getDeviceTimeZone().getStopTime();
        setStopTime(new TimeOfDay(time / 60, time % 60));
    }

    public void submit() {
        getDeviceTimeZone().setStartWeek(getStartWeek());
        getDeviceTimeZone().setStopWeek(getStopWeek());
        getDeviceTimeZone().setStartDayOfWeek(getStartDayOfWeek());
        getDeviceTimeZone().setStopDayOfWeek(getStopDayOfWeek());
        getDeviceTimeZone().setStartMonth(getStartMonth());
        getDeviceTimeZone().setStopMonth(getStopMonth());
        getDeviceTimeZone().setStartTime(getStartTime().getHrs() * 60 + getStartTime().getMin());
        getDeviceTimeZone().setStopTime(getStopTime().getHrs() * 60 + getStopTime().getMin());
        getTimeZoneManager().setDeviceTimeZone(getDeviceTimeZone());
    }

    public IPropertySelectionModel getSelectWeekModel() {
        Map<Integer, String> map = new HashMap<Integer, String>();
        map.put(new Integer(1), getMessages().getMessage("label.firstWeek"));
        map.put(new Integer(2), getMessages().getMessage("label.secondWeek"));
        map.put(new Integer(3), getMessages().getMessage("label.thirdWeek"));
        map.put(new Integer(4), getMessages().getMessage("label.fourthWeek"));
        map.put(new Integer(-1), getMessages().getMessage("label.lastWeek"));
        return new MapSelectionModel(map);
    }

    public IPropertySelectionModel getSelectDayOfWeekModel() {
        Map<Integer, String> map = new HashMap<Integer, String>();
        map.put(Calendar.SUNDAY, getMessages().getMessage("label.Sunday"));
        map.put(Calendar.MONDAY, getMessages().getMessage("label.Monday"));
        map.put(Calendar.TUESDAY, getMessages().getMessage("label.Tuesday"));
        map.put(Calendar.WEDNESDAY, getMessages().getMessage("label.Wednesday"));
        map.put(Calendar.THURSDAY, getMessages().getMessage("label.Thursday"));
        map.put(Calendar.FRIDAY, getMessages().getMessage("label.Friday"));
        map.put(Calendar.SATURDAY, getMessages().getMessage("label.Saturday"));
        return new MapSelectionModel(map);
    }

    public IPropertySelectionModel getSelectMonthModel() {
        Map<Integer, String> map = new HashMap<Integer, String>();
        map.put(Calendar.JANUARY, getMessages().getMessage("label.January"));
        map.put(Calendar.FEBRUARY, getMessages().getMessage("label.February"));
        map.put(Calendar.MARCH, getMessages().getMessage("label.March"));
        map.put(Calendar.APRIL, getMessages().getMessage("label.April"));
        map.put(Calendar.MAY, getMessages().getMessage("label.May"));
        map.put(Calendar.JUNE, getMessages().getMessage("label.June"));
        map.put(Calendar.JULY, getMessages().getMessage("label.July"));
        map.put(Calendar.AUGUST, getMessages().getMessage("label.August"));
        map.put(Calendar.SEPTEMBER, getMessages().getMessage("label.September"));
        map.put(Calendar.OCTOBER, getMessages().getMessage("label.October"));
        map.put(Calendar.NOVEMBER, getMessages().getMessage("label.November"));
        map.put(Calendar.DECEMBER, getMessages().getMessage("label.December"));
        return new MapSelectionModel(map);
    }

    public abstract Integer getStartWeek();

    public abstract void setStartWeek(Integer startWeek);

    public abstract Integer getStopWeek();

    public abstract void setStopWeek(Integer stopWeek);

    public abstract Integer getStartDayOfWeek();

    public abstract void setStartDayOfWeek(Integer startDayOfWeek);

    public abstract Integer getStopDayOfWeek();

    public abstract void setStopDayOfWeek(Integer stopDayOfWeek);

    public abstract Integer getStartMonth();

    public abstract void setStartMonth(Integer startMonth);

    public abstract Integer getStopMonth();

    public abstract void setStopMonth(Integer stopMonth);

    public abstract TimeOfDay getStartTime();

    public abstract void setStartTime(TimeOfDay startTime);

    public abstract TimeOfDay getStopTime();

    public abstract void setStopTime(TimeOfDay stopTime);
}
