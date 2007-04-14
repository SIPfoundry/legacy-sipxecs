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

import org.apache.tapestry.html.BasePage;

public abstract class RestartReminder extends BasePage {
    public static final String PAGE = "RestartReminder";

    public abstract void setNextPage(String nextPage);

    public abstract String getNextPage();

    public abstract void setProcesses(Object[] processes);

    public abstract Object[] getProcesses();

    public String proceed() {
        RestartReminderPanel reminder = (RestartReminderPanel) getComponent("reminder");
        reminder.restart();
        return getNextPage();
    }
}
