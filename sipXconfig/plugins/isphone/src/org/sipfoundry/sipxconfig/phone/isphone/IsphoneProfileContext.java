/*
 *
 *
 * Copyright (C) 2004-2009 iscoord ltd.
 * Beustweg 12, 8032 Zurich, Switzerland
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxconfig.phone.isphone;

import java.util.Map;

import org.sipfoundry.sipxconfig.device.ProfileContext;

public class IsphoneProfileContext extends ProfileContext<IsphonePhone> {

    public IsphoneProfileContext(IsphonePhone device, String profileTemplate) {
        super(device, profileTemplate);
    }

    @Override
    public Map<String, Object> getContext() {
        Map<String, Object> context = super.getContext();
        return context;
    }
}
