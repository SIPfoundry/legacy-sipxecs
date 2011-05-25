/*
 *
 *
 * Copyright (C) 2010 Karel Elektronik, A.S. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.phone.karel_ip11x;

import org.sipfoundry.sipxconfig.device.ProfileContext;

/**
 * Responsible for generating mac.cfg
 */
public class PhoneConfiguration extends ProfileContext {

    private static final String DEVICE_TEMPLATE = "karel-ip11x/config.vm";

    public PhoneConfiguration(KarelIP11xPhone device) {
        super(device, DEVICE_TEMPLATE);
    }
}

