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

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.job.JobContext;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class StatusWarning extends BaseComponent {
    @InjectObject(value = "spring:jobContext")
    public abstract JobContext getJobContext();

    @InjectObject(value = "spring:sipxProcessContext")
    public abstract SipxProcessContext getSipxProcessContext();

    /**
     * Show only if there was a failure AND we are NOT on JobStatus page
     *
     * @return true if error should be shown
     */
    public boolean isShow() {
        return getJobContext().isFailure() && !JobStatusPage.PAGE.equals(getPage().getPageName());
    }
}
