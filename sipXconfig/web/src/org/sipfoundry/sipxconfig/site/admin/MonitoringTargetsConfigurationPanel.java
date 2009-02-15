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

import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.valid.IValidationDelegate;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.monitoring.MRTGTarget;
import org.sipfoundry.sipxconfig.admin.monitoring.MonitoringContext;
import org.sipfoundry.sipxconfig.components.SelectMap;

public abstract class MonitoringTargetsConfigurationPanel extends BaseComponent {

    @InjectObject("spring:monitoringContext")
    public abstract MonitoringContext getMonitoringContext();

    @Parameter
    public abstract ICallback getCallback();

    @Parameter
    public abstract IValidationDelegate getValidator();

    @Parameter(required = true)
    public abstract Location getLocationBean();

    public List<MRTGTarget> getTargets() {
        return getMonitoringContext().getTargetsFromTemplate();

    }

    public String getHost() {
        return getLocationBean().getFqdn();
    }

    public SelectMap getSelections() {
        String hostFqdn = getLocationBean().getFqdn();
        List<MRTGTarget> selectedTargets = getMonitoringContext().getTargetsForHost(hostFqdn);
        SelectMap selections = new SelectMap();
        for (MRTGTarget selectedTarget : selectedTargets) {
            selections.setSelected(selectedTarget.getTitle(), true);
        }
        return selections;
    }
}
