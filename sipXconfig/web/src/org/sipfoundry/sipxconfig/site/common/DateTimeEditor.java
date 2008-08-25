/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.common;

import java.util.Calendar;
import java.util.Date;
import java.util.Locale;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.admin.TimeOfDay;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

/*
 * Date and time picker
 */
@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class DateTimeEditor extends BaseComponent {
    private static final int[] TIME_FIELDS = {
        Calendar.HOUR_OF_DAY, Calendar.MINUTE, Calendar.SECOND
    };

    @Parameter(required = true)
    public abstract Date getDatetime();

    public abstract void setDatetime(Date datetime);

    @Parameter(required = true)
    public abstract String getLabel();

    public abstract TimeOfDay getTime();

    public abstract void setTime(TimeOfDay time);

    public abstract Date getDate();

    public abstract void setDate(Date date);

    @Override
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        if (!TapestryUtils.isRewinding(cycle, this)) {
            Date datetime = getDatetime();
            setDate(new Date(datetime.getTime()));

            Calendar calendar = Calendar.getInstance(getPage().getLocale());
            calendar.setTime(datetime);
            setTime(new TimeOfDay(calendar.get(Calendar.HOUR_OF_DAY), calendar.get(Calendar.MINUTE)));
        }

        super.renderComponent(writer, cycle);

        if (TapestryUtils.isRewinding(cycle, this) && TapestryUtils.isValid(this)) {
            Date  datetime = toDateTime(getDate(), getTime(), getPage().getLocale());
            setDatetime(datetime);
        }
    }

    public static Date toDateTime(Date date, TimeOfDay timeOfDay, Locale locale) {
        Calendar calDate = Calendar.getInstance(locale);
        calDate.setTime(date);

        Calendar calTime = Calendar.getInstance(locale);
        calTime.set(Calendar.HOUR_OF_DAY, timeOfDay.getHrs());
        calTime.set(Calendar.MINUTE, timeOfDay.getMin());

        for (int field : TIME_FIELDS) {
            calDate.set(field, calTime.get(field));
        }

        Date datetime2 = calDate.getTime();
        return datetime2;
    }
}
