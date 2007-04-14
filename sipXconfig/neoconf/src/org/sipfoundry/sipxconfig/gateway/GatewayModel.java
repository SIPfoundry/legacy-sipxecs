/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.gateway;

import org.sipfoundry.sipxconfig.device.DeviceDescriptor;

public class GatewayModel extends DeviceDescriptor {

    public GatewayModel() {
    }

    public GatewayModel(String beanId) {
        super(beanId);
    }

    public GatewayModel(String beanId, String modelId) {
        super(beanId, modelId);
    }
}
