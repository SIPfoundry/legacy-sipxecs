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
import java.util.Collection;

import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.conference.Bridge;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;

public abstract class EditConference extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "conference/EditConference";

    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    public abstract Serializable getBridgeId();

    public abstract void setBridgeId(Serializable bridgeId);

    public abstract Serializable getConferenceId();

    public abstract void setConferenceId(Serializable id);

    public abstract Conference getConference();

    public abstract void setConference(Conference acdServer);

    public abstract boolean getChanged();

    public abstract CoreContext getCoreContext();
    
    public abstract void setSelectedUsers(Collection<Integer> selectedUsers);
    public abstract Collection<Integer> getSelectedUsers();
    
    public void pageBeginRender(PageEvent event_) {
        if (getConference() != null) {
            return;
        }
        Conference conference = null;
        if (getConferenceId() != null) {
            conference = getConferenceBridgeContext().loadConference(getConferenceId());
        } else {
            conference = getConferenceBridgeContext().newConference();
        }
        
        setConference(conference);
    }

    public void apply() {
        if (TapestryUtils.isValid(this)) {
            saveValid();
        }
    }

    private void saveValid() {
        Conference conference = getConference();
        
        // Make sure the conference is OK to save before we save it.
        // Since the database is not locked, there is a race condition here, but at least
        // we are reducing the likelihood of a problem significantly.
        getConferenceBridgeContext().validate(conference);
        
        if (conference.isNew()) {
            // associate with bridge
            Bridge bridge = getConferenceBridgeContext().loadBridge(getBridgeId());
            bridge.addConference(conference);
            getConferenceBridgeContext().store(bridge);
            Integer id = conference.getId();
            setConferenceId(id);
        } else {
            getConferenceBridgeContext().store(conference);
        }
    }

    public void formSubmit() {
        if (getChanged()) {
            setConference(null);
        }
    }
}
