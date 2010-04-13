/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.admin.configdiag;

import java.text.Format;
import java.util.Date;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Message;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.annotations.Persist;
import org.sipfoundry.sipxconfig.admin.configdiag.ConfigurationDiagnostic;
import org.sipfoundry.sipxconfig.admin.configdiag.ConfigurationDiagnosticResult;
import org.sipfoundry.sipxconfig.admin.configdiag.ConfigurationDiagnosticResult.Status;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class DiagnosticDetail extends BaseComponent {

    @Asset("context:/WEB-INF/admin/configdiag/DiagnosticDetail.script")
    public abstract IAsset getScript();

    @Parameter
    public abstract ConfigurationDiagnostic getTest();

    @Parameter
    public abstract Format getDateFormat();

    @Parameter
    public abstract String getRowClass();

    @Parameter
    public abstract Integer getRowId();

    @Message(value = "notRun")
    public abstract String getNotRunMsg();

    public Object getLastRunTime() {
        Date lastRunTime = getTest().getStartTime();
        if (lastRunTime != null) {
            return getDateFormat().format(lastRunTime);
        }
        return getNotRunMsg();
    }

    /**
     * Determines if more information should be presented about the test
     */
    public boolean getRenderResult() {
        ConfigurationDiagnosticResult result = getTest().getResult();
        return Status.hasDetails(result.getStatus());
    }

    @Persist
    public abstract boolean getShowTestResults();

    @Persist
    public abstract boolean isShowDetailedHelp();

    public abstract void setShowTestResults(boolean showTestResults);
}
