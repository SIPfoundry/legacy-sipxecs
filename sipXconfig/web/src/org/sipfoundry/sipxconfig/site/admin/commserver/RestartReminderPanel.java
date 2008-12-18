/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin.commserver;

import java.util.Arrays;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.InjectObject;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;

public abstract class RestartReminderPanel extends BaseComponent {
    public static final String PAGE = "admin/commserver/RestartReminder";

    public abstract boolean getRestartLater();

    public abstract void setRestartLater(boolean restartLater);

    public abstract SipxProcessContext getSipxProcessContext();

    public abstract Object[] getProcesses();

    public abstract void setProcesses(Object[] processes);

    public abstract Class getEventClass();

    public abstract void setEventClass(Class eventClass);

    @InjectObject("spring:sipxServiceManager")
    public abstract SipxServiceManager getSipxServiceManager();

    public void restart() {
        List procsToRestart = getProcessesToRestart();
        if (procsToRestart == null) {
            return;
        }
        SipxProcessContext processContext = getSipxProcessContext();
        Class eventClass = getEventClass();
        if (eventClass == null) {
            processContext.manageServices(procsToRestart, SipxProcessContext.Command.RESTART);
        } else {
            processContext.restartOnEvent(procsToRestart, eventClass);
        }
    }

    public List getProcessesToRestart() {
        if (getRestartLater()) {
            return null;
        }
        Object[] processes = getProcesses();
        if (processes != null) {
            return Arrays.asList(processes);
        }

        return getSipxServiceManager().getRestartable();
    }
}
