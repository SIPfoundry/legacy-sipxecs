/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin.time;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.time.TimeManager;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;

public abstract class EditManualTimeSettings extends BaseComponent implements PageBeginRenderListener {
    @Parameter(required = true)
    public abstract TimeManager getTimeManager();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract Date getNewDate();

    public abstract void setNewDate(Date date);

    public void pageBeginRender(PageEvent event_) {
        setNewDate(new Date());
    }

    public void setDate() {
        SimpleDateFormat changeDateFormat = new SimpleDateFormat("MMddHHmmyyyy");
        getTimeManager().setSystemDate(changeDateFormat.format(getNewDate()));
    }
}
