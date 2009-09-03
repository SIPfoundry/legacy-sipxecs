/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.components;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.admin.monitoring.MonitoringContext;
import org.sipfoundry.sipxconfig.admin.update.PackageUpdateManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.presence.PresenceServer;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class AdminNavigation extends BaseComponent {

    @InjectObject("spring:presenceServer")
    public abstract PresenceServer getPresenceServer();

    @InjectObject("spring:acdContext")
    public abstract AcdContext getAcdContext();

    @InjectObject("spring:monitoringContext")
    public abstract MonitoringContext getMonitoringContext();

    @InjectObject("spring:coreContext")
    public abstract CoreContext getContext();

    @InjectObject("spring:packageUpdateManager")
    public abstract PackageUpdateManager getPackageUpdateManager();

    @InjectObject("spring:sbcDeviceManager")
    public abstract SbcDeviceManager getSbcDeviceManager();

    @InjectObject("spring:sipxServiceManager")
    public abstract SipxServiceManager getSipxServiceManager();

    public boolean isOpenFireEnabled() {
        // it uses the service name defined in openfire plugin
        return getSipxServiceManager().isServiceInstalled("sipxOpenfireService");
    }
}
