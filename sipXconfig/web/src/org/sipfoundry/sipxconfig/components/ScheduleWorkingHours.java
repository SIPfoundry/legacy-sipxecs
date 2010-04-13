/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.admin.ScheduledDay;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime.WorkingHours;

public abstract class ScheduleWorkingHours extends BaseComponent {

    public abstract IPropertySelectionModel getDayOfWeekModel();

    public abstract void setDayOfWeekModel(IPropertySelectionModel model);

    public abstract WorkingHours getWorkingHour();

    public abstract void setWorkingHour(WorkingHours hours);

    protected void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
        if (getDayOfWeekModel() == null) {
            setDayOfWeekModel(createDayOfWeekModel());
        }
    }

    private IPropertySelectionModel createDayOfWeekModel() {
        EnumPropertySelectionModel dayModel = new EnumPropertySelectionModel();
        dayModel.setEnumClass(ScheduledDay.class);

        LocalizedOptionModelDecorator decoratedModel = new LocalizedOptionModelDecorator();
        decoratedModel.setModel(dayModel);
        decoratedModel.setMessages(getMessages());
        decoratedModel.setResourcePrefix("scheduleDay.");
        return decoratedModel;
    }
}
