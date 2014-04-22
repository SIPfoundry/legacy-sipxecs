/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.phone.polycom;

import java.util.Map;

import org.sipfoundry.sipxconfig.device.ProfileContext;

/**
 * Responsible for generating ipmid.cfg
 */
public class ApplicationsConfiguration extends ProfileContext {

    public ApplicationsConfiguration(PolycomPhone device) {
        super(device, device.getTemplateDir() + "/applications.cfg.vm");
    }

    @Override
    public Map<String, Object> getContext() {
        Map<String, Object> context = super.getContext();
        getDevice().getSettings();
        context.put("ver416", PolycomModel.VER_4_1_6);

        return context;
    }
}
