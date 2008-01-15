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
import org.sipfoundry.sipxconfig.admin.configdiag.ConfigurationDiagnosticContext;
import org.sipfoundry.sipxconfig.job.JobContext;
import org.sipfoundry.sipxconfig.site.admin.configdiag.ConfigurationDiagnosticPage;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class StatusWarning extends BaseComponent {
    @InjectObject(value = "spring:jobContext")
    public abstract JobContext getJobContext();

    @InjectObject(value = "spring:configurationDiagnosticContext")
    public abstract ConfigurationDiagnosticContext getConfigurationDiagnosticContext();

    /**
     * Show only if there was a failure AND we are NOT on JobStatus page
     * 
     * @return true if error should be showns
     */
    public boolean isShow() {
        String currentPageName = getPage().getPageName();
        return getJobContext().isFailure() && !JobStatusPage.PAGE.equals(currentPageName);
    }

    /**
     * Show only if there was a diagnostic failure AND we are NOT on Configuration Diagnostic page
     * 
     * @return true if diagnostic failed error should be showns
     */
    public boolean isShowDiagnosticFailing() {
        String currentPageName = getPage().getPageName();
        return getConfigurationDiagnosticContext().anyFailures()
                && !ConfigurationDiagnosticPage.PAGE.equals(currentPageName);
    }
}
