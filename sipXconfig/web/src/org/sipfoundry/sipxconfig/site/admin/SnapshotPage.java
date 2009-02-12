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

import java.io.File;
import java.util.Date;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.EventListener;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.Snapshot;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.site.cdr.CdrHistory;

public abstract class SnapshotPage extends BasePage implements PageBeginRenderListener {
    private static final String CLIENT = "client";

    @Persist(CLIENT)
    public abstract Date getStartDate();

    public abstract void setStartDate(Date date);

    @Persist(CLIENT)
    public abstract Date getEndDate();

    public abstract void setEndDate(Date date);

    public abstract void setSnapshotFile(File file);

    @InjectObject("spring:snapshot")
    public abstract Snapshot getSnapshot();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public void pageBeginRender(PageEvent event_) {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        if (getEndDate() == null) {
            setEndDate(CdrHistory.getDefaultEndTime());
        }

        if (getStartDate() == null) {
            Date startTime = CdrHistory.getDefaultStartTime(getEndDate());
            setStartDate(startTime);
        }
    }

    public void createSnapshot() {
        if (!TapestryUtils.isValid(this)) {
            // do nothing on errors
            return;
        }

        if (!getSnapshot().isLogs() && getSnapshot().isFilterTime()) {
            // 'Time filter' may only be specified with 'Log files'
            throw new UserException(false, "message.invalidSelection");
        }

        if (getStartDate().compareTo(getEndDate()) > 0) {
            // 'Start date' should not be higher that 'End date'
            throw new UserException(false, "message.invalidDates");
        }

        File file = getSnapshot().perform(getStartDate(), getEndDate());
        setSnapshotFile(file);
    }

    @EventListener(targets = "profilesCheck", events = "onclick")
    public void adjustCredentials(IRequestCycle cycle) {
        Snapshot snapshot = getSnapshot();
        if (snapshot.isProfiles()) {
            snapshot.setCredentials(true);
        }
        cycle.getResponseBuilder().updateComponent("snapshotForm");
    }
}
