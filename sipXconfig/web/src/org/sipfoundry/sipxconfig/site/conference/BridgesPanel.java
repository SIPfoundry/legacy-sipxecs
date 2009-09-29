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
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.engine.RequestCycle;
import org.sipfoundry.sipxconfig.components.TablePanel;
import org.sipfoundry.sipxconfig.conference.ActiveConferenceContext;
import org.sipfoundry.sipxconfig.conference.Bridge;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.conference.FreeswitchApiException;

public abstract class BridgesPanel extends TablePanel {
    private static final Log LOG = LogFactory.getLog(ConferencesPanel.class);

    @InjectObject("spring:conferenceBridgeContext")
    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    @InjectObject("spring:activeConferenceContext")
    public abstract ActiveConferenceContext getActiveConferenceContext();

    protected void removeRows(Collection selectedRows) {

    }


    public void calculateActiveValue(RequestCycle cycle, int id) {
        Bridge bridge = getConferenceBridgeContext().loadBridge(id);

        try {
            int activeCount = getActiveConferenceContext().getActiveConferenceCount(bridge);
            LOG.info("Bridge: " + bridge.getName() + " Conference: " + activeCount);
            ActiveValue.setActiveCount(cycle, activeCount);
        } catch (FreeswitchApiException fae) {
            // TODO better UI to mark the bridge in the UI as unreachable
            LOG.error("Couldn't connect to FreeSWITCH to fetch active conferences", fae);
        }
    }
}
