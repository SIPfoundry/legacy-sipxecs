/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.admin.dialplan.sbc.bridge;

import java.io.IOException;
import java.io.Writer;

import org.sipfoundry.sipxconfig.admin.AbstractConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.springframework.beans.factory.annotation.Required;

public class BridgeSbcConfigurationFile extends AbstractConfigurationFile {
    private SbcDeviceManager m_sbcDeviceManager;

    public void write(Writer writer, Location location) throws IOException {
        BridgeSbc bridgeSbc = m_sbcDeviceManager.getBridgeSbc(location);
        if (bridgeSbc == null) {
            throw new IllegalArgumentException("Cannot generate SBC configuration for location:"
                    + location.getFqdn());
        }

        MemoryProfileLocation profileLocation = new MemoryProfileLocation();
        bridgeSbc.generateProfiles(profileLocation);

        writer.write(profileLocation.toString(getName()));
    }

    @Override
    public boolean isReplicable(Location location) {
        BridgeSbc bridgeSbc = m_sbcDeviceManager.getBridgeSbc(location);
        return bridgeSbc != null;
    }

    @Required
    public void setSbcDeviceManager(SbcDeviceManager sbcDeviceManager) {
        m_sbcDeviceManager = sbcDeviceManager;
    }
}
