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

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.conference.Bridge;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;

public abstract class EditBridge extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "conference/EditBridge";

    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    public abstract Serializable getBridgeId();

    public abstract void setBridgeId(Serializable id);

    public abstract Bridge getBridge();

    public abstract void setBridge(Bridge acdServer);

    public abstract boolean getChanged();

    public void pageBeginRender(PageEvent event_) {
        if (getBridge() != null) {
            return;
        }
        Bridge bridge = null;
        if (getBridgeId() != null) {
            bridge = getConferenceBridgeContext().loadBridge(getBridgeId());
        } else {
            bridge = getConferenceBridgeContext().newBridge();
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
        boolean isNew = bridge.isNew();
        getConferenceBridgeContext().store(bridge);
        if (isNew) {
            Integer id = bridge.getId();
            setBridgeId(id);
        }
    }

    public void formSubmit() {
        if (getChanged()) {
            setBridge(null);
        }
    }

    public IPage addConference(IRequestCycle cycle) {
        apply();
        EditConference editConference = (EditConference) cycle.getPage(EditConference.PAGE);
        editConference.setBridgeId(getBridgeId());
        editConference.setConferenceId(null);
        editConference.setReturnPage(PAGE);
        return editConference;
    }

    public IPage editConference(IRequestCycle cycle, Integer id) {
        EditConference editConference = (EditConference) cycle.getPage(EditConference.PAGE);
        editConference.setBridgeId(getBridgeId());
        editConference.setConferenceId(id);
        editConference.setReturnPage(PAGE);
        return editConference;
    }
}
