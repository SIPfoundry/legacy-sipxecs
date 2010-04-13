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

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.sipfoundry.sipxconfig.admin.monitoring.MRTGTarget;
import org.sipfoundry.sipxconfig.admin.monitoring.MonitoringContext;
import org.sipfoundry.sipxconfig.admin.monitoring.MonitoringUtil;
import org.sipfoundry.sipxconfig.components.SelectMap;

public abstract class TargetsTable extends BaseComponent {
    // private static final String FAILURES = "error.snmpCommunityString";

    @InjectObject(value = "spring:monitoringContext")
    public abstract MonitoringContext getMonitoringContext();

    public abstract List getTargets();

    public abstract SelectMap getSelections();

    public abstract void setSelections(SelectMap selections);

    public abstract String getHost();

    public abstract void setHost(String host);

    @InitialValue(value = "literal:sipxecs")
    public abstract String getSnmpCommunityString();

    public abstract void setSnmpCommunityString(String snmpCommunityString);

    protected void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);

        List<MRTGTarget> targets = getMonitoringContext().getTargetsForHost(getHost());
        if (targets.size() > 0) {
            MRTGTarget target = targets.get(0);
            String expression = target.getExpression();
            String snmpCommunityString = expression.substring(expression
                    .lastIndexOf(MonitoringUtil.COLON) + 1, expression.lastIndexOf("@"));
            setSnmpCommunityString(snmpCommunityString);
        }
    }

    public void commit() {
        List<String> selectedTargetNames = new ArrayList<String>();
        Collection selections = getSelections().getAllSelected();
        for (Object selection : selections) {
            selectedTargetNames.add(selection.toString());
        }

        getMonitoringContext().generateConfigFiles(getHost(), selectedTargetNames);
    }
}
