/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.phone;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.device.DeviceDescriptor;

import static org.apache.commons.lang.StringUtils.join;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class PhoneModelLabels extends BaseComponent {
    @Parameter(required = true)
    public abstract Collection<DeviceDescriptor> getModels();

    public String getModelLabelsAsString() {
        return join(getModelLabels(), ", ");
    }

    public List<String> getModelLabels() {
        List<String> labels = new ArrayList<String>();
        for (DeviceDescriptor descriptor : getModels()) {
            labels.add(descriptor.getLabel());
        }
        return labels;
    }
}
