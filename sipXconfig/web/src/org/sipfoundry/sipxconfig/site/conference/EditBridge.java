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

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

import org.apache.tapestry.IAsset;
import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.components.Block;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.conference.Bridge;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;

public abstract class EditBridge extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "conference/EditBridge";

    public static final String TAB_CONFERENCES = "conferences";

    public abstract Serializable getBridgeId();

    @Persist
    public abstract void setBridgeId(Serializable id);

    public abstract Bridge getBridge();

    public abstract void setBridge(Bridge acdServer);

    public abstract boolean getChanged();

    @InjectObject("spring:conferenceBridgeContext")
    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    @Persist
    @InitialValue("literal:config")
    public abstract void setTab(String tab);

    public abstract String getTab();

    @Asset("/images/breadcrumb_separator.png")
    public abstract IAsset getBreadcrumbSeparator();

    public void pageBeginRender(PageEvent event_) {
        if (getBridge() != null) {
            return;
        }
        Bridge bridge = null;
        if (getBridgeId() != null) {
            bridge = getConferenceBridgeContext().loadBridge(getBridgeId());
        }

        setBridge(bridge);
    }

    public void apply() {
        if (TapestryUtils.isValid(this)) {
            saveValid();
        }
    }

    private void saveValid() {
        Bridge bridge = getBridge();
        getConferenceBridgeContext().store(bridge);
    }

    public void formSubmit() {
        if (getChanged()) {
            setBridge(null);
        }
    }

    @InjectPage(EditConference.PAGE)
    public abstract EditConference getEditConferencePage();

    public IPage addConference() {
        apply();
        return activateEditConferencePage(null, null);
    }

    public List<String> getTabNames() {
        List<String> tabNames = new ArrayList<String>();
        tabNames.add(EditConference.TAB_CONFIG);
        if (!getBridge().isNew()) {
            tabNames.add(TAB_CONFERENCES);
        }

        return tabNames;
    }

    public IPage editConference(Integer id) {
        return activateEditConferencePage(id, EditConference.TAB_CONFIG);
    }

    public IPage activeConferences(Integer id) {
        return activateEditConferencePage(id, EditConference.TAB_PARTICIPANTS);
    }

    private IPage activateEditConferencePage(Integer id, String tab) {
        EditConference editConference = getEditConferencePage();
        editConference.setBridgeId(getBridgeId());
        editConference.setTestBridge(getBridge());
        editConference.setConferenceId(id);
        if (tab != null) {
            editConference.setTab(tab);
        }
        editConference.setReturnPage(PAGE);
        return editConference;
    }

    public Block getActionBlockForTab() {
        if (getTab().equals(TAB_CONFERENCES)) {
            return (Block) getComponent("conferencesPanel").getComponent("conferenceActions");
        }
        return (Block) getComponent("formActionsBlock");
    }
}
