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

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.apache.tapestry.IAsset;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.conference.ActiveConference;
import org.sipfoundry.sipxconfig.conference.ActiveConferenceContext;
import org.sipfoundry.sipxconfig.conference.Bridge;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.conference.FreeswitchApiException;

public abstract class ListActiveConferences extends BasePage implements PageBeginRenderListener {
    public static final String PAGE = "conference/ListActiveConferences";

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Bean
    public abstract SelectMap getSelections();

    @InjectObject(value = "spring:activeConferenceContext")
    public abstract ActiveConferenceContext getActiveConferenceContext();

    @InjectObject(value = "spring:conferenceBridgeContext")
    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    @Persist
    public abstract Integer getBridgeId();
    public abstract void setBridgeId(Integer bridgeId);

    public abstract void setBridge(Bridge bridge);
    public abstract Bridge getBridge();

    public abstract void setCurrentConference(ActiveConference conference);
    public abstract ActiveConference getCurrentConference();

    @Asset("/images/user.png")
    public abstract IAsset getUserIcon();

    @Asset("/images/lock.png")
    public abstract IAsset getLockedIcon();

    @Persist
    public abstract void setActiveConferences(List<ActiveConference> conferences);
    public abstract List<ActiveConference> getActiveConferences();

    public void pageBeginRender(PageEvent event_) {
        Bridge bridge = getConferenceBridgeContext().loadBridge(getBridgeId());
        setBridge(bridge);

        if (!getRequestCycle().isRewinding()) {
            List<ActiveConference> activeConferences = new ArrayList<ActiveConference>();
            try {
                activeConferences = getActiveConferenceContext().getActiveConferences(bridge);
            } catch (FreeswitchApiException fae) {
                getValidator().record(fae, getMessages());
            }

            setActiveConferences(activeConferences);
        }
    }

    public void lockConferences() {
        Collection<Integer> selectedConferences = getSelections().getAllSelected();
        ConferenceBridgeContext conferenceBridgeContext = getConferenceBridgeContext();
        SipxValidationDelegate validator = getValidator();
        ActiveConferenceContext activeConferenceContext = getActiveConferenceContext();
        for (Integer conferenceId : selectedConferences) {
            Conference conference = conferenceBridgeContext.loadConference(conferenceId);
            try {
                activeConferenceContext.lockConference(conference);
                validator.recordSuccess(getMessages().getMessage("msg.success.lock"));
            } catch (FreeswitchApiException fae) {
                validator.record(fae, getMessages());
            }
        }
    }

    public void unlockConferences() {
        Collection<Integer> selectedConferences = getSelections().getAllSelected();
        ConferenceBridgeContext conferenceBridgeContext = getConferenceBridgeContext();
        SipxValidationDelegate validator = getValidator();
        ActiveConferenceContext activeConferenceContext = getActiveConferenceContext();
        for (Integer conferenceId : selectedConferences) {
            Conference conference = conferenceBridgeContext.loadConference(conferenceId);
            try {
                activeConferenceContext.unlockConference(conference);
                validator.recordSuccess(getMessages().getMessage("msg.success.unlock"));
            } catch (FreeswitchApiException fae) {
                validator.record(fae, getMessages());
            }
        }
    }

    public IPage activeConferenceControl(Conference conference) {
        IRequestCycle cycle = getRequestCycle();
        ActiveConferenceControl controlPage = (ActiveConferenceControl) cycle.getPage(ActiveConferenceControl.PAGE);
        controlPage.setConference(conference);
        return controlPage;
    }

}
