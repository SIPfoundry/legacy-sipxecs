/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.conference;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.EventListener;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.components.Block;
import org.apache.tapestry.event.BrowserEvent;
import org.sipfoundry.sipxconfig.conference.ActiveConferenceContext;
import org.sipfoundry.sipxconfig.conference.Bridge;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.conference.FreeswitchApiException;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class ActiveConferencesValue extends BaseComponent {
    private static final Log LOG = LogFactory.getLog(ActiveConferencesValue.class);

    @Parameter(required = true)
    public abstract int getBridgeId();

    @Asset("/images/go.png")
    public abstract IAsset getGoIcon();

    @Asset("/images/loading.gif")
    public abstract IAsset getLoadingImage();

    @Asset(value = "context:/WEB-INF/conference/ActiveConferencesValue.script")
    public abstract IAsset getScript();

    @InjectPage(value = ListActiveConferences.PAGE)
    public abstract ListActiveConferences getListActiveConferencesPage();

    @InjectObject(value = "spring:activeConferenceContext")
    public abstract ActiveConferenceContext getActiveConferenceContext();

    @InjectObject(value = "spring:conferenceBridgeContext")
    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    @InitialValue(value = "-1")
    public abstract void setActiveConferenceCount(int count);

    public abstract int getActiveConferenceCount();

    public IPage activeConferences(Integer bridgeId) {
        ListActiveConferences activePage = getListActiveConferencesPage();
        activePage.setBridgeId(bridgeId);
        return activePage;
    }

    @EventListener(events = "startRendering", targets = "throbber")
    public void retrieveActiveConferenceCount(IRequestCycle cycle, BrowserEvent event) {
        // reset the number in case we cannot connect to freeswitch
        setActiveConferenceCount(0);

        int bridgeId = event.getMethodArguments().getInt(0);
        Bridge bridge = getConferenceBridgeContext().loadBridge(bridgeId);

        try {
            int activeConferences = getActiveConferenceContext().getActiveConferenceCount(bridge);
            LOG.info("Bridge: " + bridge.getName() + " Conference: " + activeConferences);
            setActiveConferenceCount(activeConferences);
        } catch (FreeswitchApiException fae) {
            // TODO this is a temporary measure - eventually want better UI to mark
            // the bridge in the UI as unreachable
            LOG.error("Couldn't connect to FreeSWITCH to fetch active conferences", fae);
        }

        cycle.getResponseBuilder().updateComponent("cell");
    }

    public String getActiveLabel() {
        return getMessages().format("label.active", getActiveConferenceCount());
    }

    public Block getBlock() {
        String blockName = "loading";
        int count = getActiveConferenceCount();
        if (count > 0) {
            blockName = "active";
        } else if (count == 0) {
            blockName = "inactive";
        }
        return (Block) getComponent(blockName);
    }
}
