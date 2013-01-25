/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.job;

import java.io.Serializable;
import java.util.List;

import org.sipfoundry.sipxconfig.commserver.Location;

public interface JobContext {
    Serializable schedule(String name);
    Serializable schedule(String name, Location location);

    void checkAndThrowErrorOnFailedJobs();

    void start(Serializable jobId);

    void success(Serializable jobId);

    void warning(Serializable jobId, String warnMsg);

    void failure(Serializable jobId, String errorMsg, Throwable exception);

    void clear();
    void clearFailed();

    List<Job> getJobs();
    List<Job> getFailedJobs();
    List<Job> getNotFailedJobs();

    /**
     * Returns true if there is a least one failed job on the list.
     */
    boolean isFailure();
    void removeCompleted();
}
