/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.util.Date;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;

public abstract class TimeSettingsPage extends BasePage implements PageBeginRenderListener {
    @InjectObject(value = "spring:adminContext")
    public abstract AdminContext getAdminContext();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract Date getNewDate();

    public abstract void setNewDate(Date date);

    public void pageBeginRender(PageEvent event_) {
        setNewDate(new Date());
    }

    public void setDate() {
        getAdminContext().setSystemDate(getNewDate());
    }
}
