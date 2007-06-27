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

import java.util.List;

import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.admin.forwarding.AbstractSchedule;

public class ScheduleListPropertySelectionModel implements IPropertySelectionModel {

    private List m_options;

    public void setOptions(List options) {
        m_options = options;
    }

    public int getOptionCount() {
        return m_options.size();
    }

    public Object getOption(int index) {
        return (AbstractSchedule) m_options.get(index);
    }

    public String getLabel(int index) {
        return ((AbstractSchedule) m_options.get(index)).getName();
    }

    public String getValue(int index) {
        return ((AbstractSchedule) m_options.get(index)).getId().toString();
    }

    public Object translateValue(String value) {
        int length = m_options.size();
        for (int i = 0; i < length; i++) {
            AbstractSchedule sch = (AbstractSchedule) m_options.get(i);
            if (sch.getId().toString().equals(value)) {
                return sch;
            }
        }
        return null;
    }
}
