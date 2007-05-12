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
import org.sipfoundry.sipxconfig.device.Device;

@ComponentClass(allowInformalParameters = false, allowBody = false)
public abstract class ProfilePreview extends BaseComponent {
    @Parameter
    public abstract Device getDevice();
    
    public abstract String getProfileType();

    public boolean getRender() {
        String[] profileTypes = getDevice().getProfileTypes();
        return profileTypes != null && profileTypes.length > 0;
    }
}
