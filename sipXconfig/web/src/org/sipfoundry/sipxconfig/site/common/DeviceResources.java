/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.common;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.device.DeviceDescriptor;
import org.sipfoundry.sipxconfig.device.Resource;

@ComponentClass(allowInformalParameters = false, allowBody = false)
public abstract class DeviceResources extends BaseComponent {
    @Parameter
    public abstract DeviceDescriptor getDeviceDescriptor();

    public abstract Resource getResource();

    public boolean getRender() {
        Resource[] profileTypes = getDeviceDescriptor().getResources();
        return profileTypes != null && profileTypes.length > 0;
    }
}
