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

public interface JobContext {
    Serializable schedule(String name);

    void start(Serializable jobId);

    void success(Serializable jobId);

    void warning(Serializable jobId, String warnMsg);

    void failure(Serializable jobId, String errorMsg, Throwable exception);

    int removeCompleted();

    void clear();

    List<Job> getJobs();

    /**
     * Returns true if there is a least one failed job on the list.
     */
    boolean isFailure();
}
