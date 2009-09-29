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
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.admin.CronSchedule;
import org.sipfoundry.sipxconfig.admin.ScheduledDay;

public abstract class ScheduleEditor extends BaseComponent {

    public abstract IPropertySelectionModel getTypeModel();

    public abstract void setTypeModel(IPropertySelectionModel model);

    public abstract IPropertySelectionModel getDayOfWeekModel();

    public abstract void setDayOfWeekModel(IPropertySelectionModel model);

    protected void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
        if (getTypeModel() == null) {
            NewEnumPropertySelectionModel typeModel = new NewEnumPropertySelectionModel();
            typeModel.setEnumType(CronSchedule.Type.class);

            LocalizedOptionModelDecorator decoratedModel = new LocalizedOptionModelDecorator();
            decoratedModel.setMessages(getMessages());
            decoratedModel.setModel(typeModel);
            decoratedModel.setResourcePrefix("type.");
            setTypeModel(decoratedModel);
        }
        if (getDayOfWeekModel() == null) {
            EnumPropertySelectionModel dayModel = new EnumPropertySelectionModel();
            dayModel.setOptions(ScheduledDay.DAYS_OF_WEEK);
            setDayOfWeekModel(dayModel);
        }
    }

    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        super.renderComponent(writer, cycle);
        if (!cycle.isRewinding()) {
            return;
        }
    }
}
