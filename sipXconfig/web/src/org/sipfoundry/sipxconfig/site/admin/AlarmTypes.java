/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.admin.alarm.Alarm;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;

@ComponentClass
public abstract class AlarmTypes extends BaseComponent {
    @Bean
    public abstract AlarmSqueezeAdapter getAlarmConverter();

    @Parameter
    public abstract SipxValidationDelegate getValidator();

    @Parameter
    public abstract List<Alarm> getAlarms();

    @Parameter
    public abstract IPropertySelectionModel getAlarmGroupModel();

    public abstract Alarm getCurrentRow();

    public abstract void setCurrentRow(Alarm alarm);
}
