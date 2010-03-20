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

import java.text.DateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.List;

import static java.text.DateFormat.SHORT;

import org.apache.tapestry.IAsset;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.EventListener;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.Snapshot;
import org.sipfoundry.sipxconfig.admin.Snapshot.SnapshotResult;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class SnapshotPage extends SipxBasePage implements PageBeginRenderListener {
    @Asset("/images/loading.gif")
    public abstract IAsset getLoadingImage();

    public abstract Date getStartDate();

    public abstract void setStartDate(Date date);

    public abstract Date getEndDate();

    public abstract void setEndDate(Date date);

    public abstract void setGenerationDate(String date);

    public abstract String getGenerationDate();

    @InjectObject("spring:snapshot")
    public abstract Snapshot getSnapshot();

    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract void setResult(Snapshot.SnapshotResult snapshotResult);

    public void pageBeginRender(PageEvent event_) {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        Calendar now = Calendar.getInstance();
        if (getEndDate() == null) {
            // End date: When the page is rendered.
            setEndDate(now.getTime());
        }

        if (getStartDate() == null) {
            // Start date: 3 hours before the End date.
            now.add(Calendar.HOUR, -3);
            setStartDate(now.getTime());
        }

        if (!getSnapshot().isRefreshing()) {
            DateFormat dateFormat = DateFormat.getDateTimeInstance(SHORT, SHORT, getPage().getLocale());
            Date lastDate = getSnapshot().getGeneratedDate();
            if (lastDate != null) {
                String date = dateFormat.format(lastDate);
                setGenerationDate(date);
            }
        }

        List<SnapshotResult> results = getSnapshot().getResults();
        for (SnapshotResult result : results) {
            if (!result.isSuccess()) {
                UserException exception = result.getUserException();
                getValidator().record(exception, getMessages());
            }
        }
    }

    public void createSnapshot() {
        if (!getSnapshot().isLogs() && getSnapshot().isFilterTime()) {
            // 'Time filter' may only be specified with 'Log files'
            throw new UserException("&message.invalidSelection");
        }

        if (getStartDate().compareTo(getEndDate()) > 0) {
            // 'Start date' should not be higher that 'End date'
            throw new UserException("&message.invalidDates");
        }

        getSnapshot().perform(getStartDate(), getEndDate(), getLocationsManager().getLocations());
    }

    public String getGenerationSuccessMsg() {
        return getMessages().format("generated.time", getGenerationDate());
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
