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
import java.util.List;

import static java.util.Arrays.asList;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.IPage;
import org.apache.tapestry.PageRedirectException;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.annotations.InjectState;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.conference.Bridge;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.site.UserSession;
import org.sipfoundry.sipxconfig.site.vm.ManageVoicemail;

public abstract class EditConference extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "conference/EditConference";
    public static final String TAB_CONFIG = "config";
    public static final String TAB_PARTICIPANTS = "participants";

    private static final String TAB_CHAT = "chat";
    private static final String TAB_WEB = "dimdim";

    private static final Log LOG = LogFactory.getLog(EditConference.class);

    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    public abstract Serializable getBridgeId();

    public abstract void setBridgeId(Serializable bridgeId);

    public abstract Bridge getBridge();

    public abstract void setBridge(Bridge bridge);

    public abstract Bridge getTestBridge();

    public abstract void setTestBridge(Bridge testBridge);

    public abstract Integer getOwnerId();

    public abstract void setOwnerId(Integer ownerId);

    public abstract Serializable getConferenceId();

    public abstract void setConferenceId(Serializable id);

    @Persist("session")
    public abstract Conference getTransientConference();

    public abstract void setTransientConference(Conference transientConference);

    public abstract Conference getConference();

    public abstract void setConference(Conference conference);

    public abstract boolean getChanged();

    public abstract CoreContext getCoreContext();

    public abstract void setSelectedUsers(Collection<Integer> selectedUsers);

    public abstract Collection<Integer> getSelectedUsers();

    @Asset("/images/breadcrumb_separator.png")
    public abstract IAsset getBreadcrumbSeparator();

    @InjectState(value = "userSession")
    public abstract UserSession getUserSession();

    @InjectPage(EditBridge.PAGE)
    public abstract EditBridge getEditBridgePage();

    @Persist
    @InitialValue(value = "literal:config")
    public abstract void setTab(String tab);

    public void pageBeginRender(PageEvent event_) {
        if (getTransientConference() != null) {
            setConference(getTransientConference());
            setTransientConference(null);
        }

        Conference conference = getConference();
        if (conference == null) {
            if (getConferenceId() != null) {
                conference = getConferenceBridgeContext().loadConference(getConferenceId());
            } else {
                conference = getConferenceBridgeContext().newConference();
                setTab(TAB_CONFIG);
                if (getOwnerId() != null) {
                    conference.setOwner(getCoreContext().loadUser(getOwnerId()));
                }
            }
        }

        Bridge bridge = getBridge();
        if (bridge == null && conference != null) {
            bridge = conference.getBridge();
        }
        if (bridge == null && getBridgeId() != null) {
            bridge = getConferenceBridgeContext().loadBridge(getBridgeId());
        }

        setConference(conference);
        setBridge(bridge);

        UserSession currentUser = getUserSession();
        if (!isAuthorized(currentUser, conference)) {
            LOG.warn(String.format("Unauthorized attempt to edit conference \"%s\" by user %s",
                    conference.getName(), currentUser.getUser(getCoreContext()).getUserName()));
            throw new PageRedirectException(ManageVoicemail.PAGE);
        }
    }

    public boolean isAuthorized(UserSession user, Conference conference) {
        User conferenceOwner = conference.getOwner();
        return (user.isAdmin() || (conferenceOwner != null && conferenceOwner.getId().equals(user.getUserId())));
    }

    public List<String> getTabNames() {
        Conference conference = getConference();
        if (conference.isNew()) {
            return asList(TAB_CONFIG);
        }
        if (!conference.isEnabled()) {
            // participants are only visible for enabled conferences
            return asList(TAB_CONFIG, TAB_WEB, TAB_CHAT);
        }
        return asList(TAB_CONFIG, TAB_PARTICIPANTS, TAB_WEB, TAB_CHAT);
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
            Bridge bridge = getBridge();
            if (bridge == null) {
                bridge = getConferenceBridgeContext().loadBridge(getBridgeId());
            }
            bridge.addConference(conference);
            conference.setBridge(bridge);
        }
        getConferenceBridgeContext().store(conference);
        Integer id = conference.getId();
        setConferenceId(id);
    }

    public void formSubmit() {
        if (getChanged()) {
            setConference(null);
        }
    }

    public IPage viewBridge(Integer bridgeId) {
        EditBridge page = getEditBridgePage();
        page.setBridgeId(bridgeId);
        return page;
    }
}
