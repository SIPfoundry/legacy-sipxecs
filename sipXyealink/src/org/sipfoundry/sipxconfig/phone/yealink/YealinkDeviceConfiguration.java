/*
 * Copyright (c) 2013 SibTelCom, JSC (SIPLABS Communications). All rights reserved.
 * Contributed to SIPfoundry and eZuce, Inc. under a Contributor Agreement.
 *
 * Developed by Konstantin S. Vishnivetsky
 *
 * This library or application is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License (AGPL) as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option) any later version.
 *
 * This library or application is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License (AGPL) for
 * more details.
 *
*/

package org.sipfoundry.sipxconfig.phone.yealink;

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.speeddial.Button;

/**
 * Responsible for generating ipmid.cfg
 */
public class YealinkDeviceConfiguration extends ProfileContext {

    public YealinkDeviceConfiguration(YealinkPhone device, String profileTemplate) {
        super(device, profileTemplate);
    }

    public Collection<Button> getSpeedDial() {
        YealinkPhone phone = (YealinkPhone) getDevice();
        if (null == phone.getSpeedDial()) {
            return Collections.emptyList();
        } else {
            return phone.getSpeedDial().getButtons();
        }
    }
}
