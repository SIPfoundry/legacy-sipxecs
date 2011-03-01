/*
 *
 *
 * Copyright (C) 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.service;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.admin.commserver.Location;

public class SipxOpenAcdVmArgsConfiguration extends SipxServiceConfiguration {

    private String m_etcDir;

    @Override
    protected VelocityContext setupContext(Location location) {
        VelocityContext context = super.setupContext(location);
        context.put("etc_dir", m_etcDir);

        return context;
    }

    public void setEtcDir(String path) {
        m_etcDir = path;
    }
}
