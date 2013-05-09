/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.admin;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.job.JobContext;
import org.sipfoundry.sipxconfig.mongo.MongoManager;
import org.sipfoundry.sipxconfig.site.admin.commserver.LocationsPage;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class StatusWarning extends BaseComponent {
    @InjectObject(value = "spring:jobContext")
    public abstract JobContext getJobContext();

    @InjectObject(value = "spring:mongoManager")
    public abstract MongoManager getMongoManager();

    @InjectObject("spring:featureManager")
    public abstract FeatureManager getFeatureManager();

    /**
     * Show only if there was a failure AND we are NOT on JobStatus page
     *
     * @return true if error should be shown
     */
    public boolean isShow() {
        return getJobContext().isFailure() && !JobStatusPage.PAGE.equals(getPage().getPageName());
    }

    /**
     * Show only if there is package(s) need to be updated AND we are NOT on
     * RestartNeededServicesPage page
     *
     * @return true if warning should be shown
     */
    public boolean showReplicaSetWarning() {
        try {
            return getMongoManager().isMisconfigured();
        } catch (Exception ex) {
            return true;
        }
    }

    public IPage getDatabasesPage() {
        LocationsPage page =  (LocationsPage) getPage().getRequestCycle().getPage(LocationsPage.PAGE);
        page.setTab("databases");
        return page;
    }

    public IPage getStatusPage() {
        JobStatusPage page =  (JobStatusPage) getPage().getRequestCycle().getPage(JobStatusPage.PAGE);
        page.setTab("failedJobs");
        return page;
    }
}
