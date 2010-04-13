/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */

package org.sipfoundry.sipxconfig.conference;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.common.InitTaskListener;
import org.sipfoundry.sipxconfig.service.LocationSpecificService;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;

public class ConferenceBridgeMigrationTask extends InitTaskListener {

    private ConferenceBridgeContext m_conferenceBridgeContext;
    private LocationsManager m_locationsManager;

    public void setConferenceBridgeContext(ConferenceBridgeContext conferenceBridgeContext) {
        m_conferenceBridgeContext = conferenceBridgeContext;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    @Override
    public void onInitTask(String task) {
        for (Location location : m_locationsManager.getLocations()) {
            LocationSpecificService freeswitchService =
                location.getService(SipxFreeswitchService.BEAN_ID);

            if (freeswitchService != null) {
                Bridge newBridge = m_conferenceBridgeContext.newBridge();
                newBridge.setService(freeswitchService);
                m_conferenceBridgeContext.store(newBridge);
            }
        }

    }
}
