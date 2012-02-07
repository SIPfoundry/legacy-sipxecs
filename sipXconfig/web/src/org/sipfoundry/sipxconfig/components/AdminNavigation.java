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
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.update.PackageUpdateManager;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class AdminNavigation extends BaseComponent {

    @InjectObject("spring:coreContext")
    public abstract CoreContext getContext();

    @InjectObject("spring:packageUpdateManager")
    public abstract PackageUpdateManager getPackageUpdateManager();

    @InjectObject("spring:sbcDeviceManager")
    public abstract SbcDeviceManager getSbcDeviceManager();
}
