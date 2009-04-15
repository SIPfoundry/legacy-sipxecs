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
import java.text.DateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.List;
import java.util.Map;

import static java.text.DateFormat.SHORT;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.EventListener;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.Snapshot;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class SnapshotPage extends BasePage implements PageBeginRenderListener {
    public abstract Date getStartDate();

    public abstract void setStartDate(Date date);

    public abstract Date getEndDate();

    public abstract void setEndDate(Date date);

    @Persist
    public abstract void setSnapshotFileList(File [] file);

    @Persist
    public abstract void setFileToDomain(Map<File, String> fileToDomain);

    @Persist
    public abstract void setGenerationDate(String date);

    public abstract Map<File, String> getFileToDomain();

    @InjectObject("spring:snapshot")
    public abstract Snapshot getSnapshot();

    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract void setSnapshotFile(File snapshot);

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
    }

    public void createSnapshot() {
        if (!TapestryUtils.isValid(this)) {
            // do nothing on errors
            return;
        }

        if (!getSnapshot().isLogs() && getSnapshot().isFilterTime()) {
            // 'Time filter' may only be specified with 'Log files'
            throw new UserException("&message.invalidSelection");
        }

        if (getStartDate().compareTo(getEndDate()) > 0) {
            // 'Start date' should not be higher that 'End date'
            throw new UserException("&message.invalidDates");
        }

        Map<File, String> logFileToLocationFqdn = getSnapshot().perform(getStartDate(), getEndDate(),
                getLocationsManager().getLocations());
        List<File> outputFiles = new ArrayList<File>();

        for (Map.Entry<File, String> entry : logFileToLocationFqdn.entrySet()) {
            outputFiles.add(entry.getKey());
        }

        DateFormat dateFormat = DateFormat.getDateTimeInstance(SHORT, SHORT, getPage().getLocale());
        String now = dateFormat.format(new Date());
        setGenerationDate(now);
        setSnapshotFileList(outputFiles.toArray(new File[outputFiles.size()]));
        setFileToDomain(logFileToLocationFqdn);
    }

    @EventListener(targets = "profilesCheck", events = "onclick")
    public void adjustCredentials(IRequestCycle cycle) {
        Snapshot snapshot = getSnapshot();
        if (snapshot.isProfiles()) {
            snapshot.setCredentials(true);
        }
        cycle.getResponseBuilder().updateComponent("snapshotForm");
    }

    public String domainForFile(File file) {
        return getFileToDomain().get(file);
    }
}
