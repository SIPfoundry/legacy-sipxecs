/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import org.sipfoundry.sipxconfig.device.DeviceDescriptor;

public class ServiceDescriptor extends DeviceDescriptor {

    public ServiceDescriptor() {
    }

    public ServiceDescriptor(String beanId) {
        setBeanId(beanId);
    }

    public ServiceDescriptor(String beanId, String modelId) {
        this(beanId);
        setModelId(modelId);
    }

    public ServiceDescriptor(String beanId, String modelId, String label) {
        this(beanId);
        setModelId(modelId);
        setLabel(label);
    }
}
