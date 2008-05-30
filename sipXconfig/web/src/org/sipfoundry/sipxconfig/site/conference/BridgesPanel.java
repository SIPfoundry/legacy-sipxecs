/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.conference;

import java.util.Collection;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.components.TablePanel;
import org.sipfoundry.sipxconfig.conference.ActiveConferenceContext;
import org.sipfoundry.sipxconfig.conference.Bridge;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.conference.FreeswitchApiException;

public abstract class BridgesPanel extends TablePanel {
    private static final Log LOG = LogFactory.getLog(BridgesPanel.class);

    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    public abstract ActiveConferenceContext getActiveConferenceContext();

    protected void removeRows(Collection selectedRows) {
        getConferenceBridgeContext().removeBridges(selectedRows);
    }

    public int getActiveConferenceCount(Bridge bridge) {
        int activeConferences = 0;
        try {
            activeConferences = getActiveConferenceContext().getActiveConferenceCount(bridge);
        } catch (FreeswitchApiException fae) {
            // TODO this is a temporary measure - eventually want better UI to mark
            // the bridge in the UI as unreachable
            LOG.error("Couldn't connect to FreeSWITCH to fetch active conferences", fae.getCause());
        }
        return activeConferences;
    }
}
