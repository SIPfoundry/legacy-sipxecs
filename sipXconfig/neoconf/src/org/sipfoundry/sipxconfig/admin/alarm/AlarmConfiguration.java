/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.alarm;

import java.util.List;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.TemplateConfigurationFile;

public class AlarmConfiguration extends TemplateConfigurationFile {
    private List<Alarm> m_alarms;

    public void generate(List<Alarm> alarms) {
        this.m_alarms = alarms;
    }

    protected VelocityContext setupContext() {
        VelocityContext context = super.setupContext();
        context.put("alarms", m_alarms);
        return context;
    }
}
