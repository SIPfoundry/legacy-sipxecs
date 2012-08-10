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

import static java.text.DateFormat.SHORT;

import java.text.DateFormat;
import java.util.Date;
import java.util.List;

import org.apache.tapestry.IAsset;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.EventListener;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.backup.Snapshot;
import org.sipfoundry.sipxconfig.backup.Snapshot.SnapshotResult;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class SnapshotPage extends SipxBasePage implements PageBeginRenderListener {
    @Asset("/images/loading.gif")
    public abstract IAsset getLoadingImage();

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
            if (result == null || !result.isSuccess()) {
                UserException exception = result.getUserException();
                getValidator().record(exception, getMessages());
            }
        }
    }

    public void createSnapshot() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        if (!getSnapshot().isLogs() && getSnapshot().isLogFilter()) {
            // 'Log filter' may only be specified with 'Logs'
            throw new UserException("&message.invalidSelection");
        }

        int numberOfLines = getSnapshot().getLines();
        if (getSnapshot().isLogFilter() && numberOfLines == 0) {
            throw new UserException("&message.invalidNumberOfLines");
        }

        getSnapshot().perform(getLocationsManager().getLocations(), numberOfLines);
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
