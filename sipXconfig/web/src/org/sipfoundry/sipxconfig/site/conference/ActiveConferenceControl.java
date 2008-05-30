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

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.apache.tapestry.IAsset;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.conference.ActiveConferenceContext;
import org.sipfoundry.sipxconfig.conference.ActiveConferenceMember;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.conference.FreeswitchApiException;

public abstract class ActiveConferenceControl extends BasePage implements PageBeginRenderListener {
    public static final String PAGE = "conference/ActiveConferenceControl";

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Bean
    public abstract SelectMap getSelections();

    @InjectObject(value = "spring:activeConferenceContext")
    public abstract ActiveConferenceContext getActiveConferenceContext();

    @InjectObject(value = "spring:conferenceBridgeContext")
    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    @Persist
    public abstract void setConference(Conference conference);
    public abstract Conference getConference();

    @Persist
    public abstract List<ActiveConferenceMember> getMembers();
    public abstract void setMembers(List<ActiveConferenceMember> members);

    public abstract ActiveConferenceMember getCurrentMember();

    @Asset("/images/user.png")
    public abstract IAsset getUserIcon();

    @Asset("/images/sound.png")
    public abstract IAsset getSoundIcon();

    @Asset("/images/deaf.png")
    public abstract IAsset getDeafIcon();

    @Asset("/images/talk.png")
    public abstract IAsset getTalkIcon();

    @Asset("/images/no-talk.png")
    public abstract IAsset getMuteIcon();

    public void pageBeginRender(PageEvent event) {
        if (!getRequestCycle().isRewinding()) {
            List<ActiveConferenceMember> members = new ArrayList<ActiveConferenceMember>();
            try {
                members = getActiveConferenceContext().getConferenceMembers(getConference());
            } catch (FreeswitchApiException fae) {
                getValidator().record(fae, getMessages());
            }

            setMembers(members);
        }
    }

    public void deafUsers() {
        Collection<Integer> selectedUsers = getSelections().getAllSelected();
        SipxValidationDelegate validator = getValidator();
        ActiveConferenceContext activeConferenceContext = getActiveConferenceContext();
        Conference conference = getConference();
        for (Integer userId : selectedUsers) {
            ActiveConferenceMember member = getMemberById(userId);
            try {
                activeConferenceContext.deafUser(conference, member);
                validator.recordSuccess(getMessages().getMessage("msg.success.deaf"));
            } catch (FreeswitchApiException fae) {
                validator.record(fae, getMessages());
            }
        }
    }

    public void undeafUsers() {
        Collection<Integer> selectedUsers = getSelections().getAllSelected();
        SipxValidationDelegate validator = getValidator();
        ActiveConferenceContext activeConferenceContext = getActiveConferenceContext();
        Conference conference = getConference();
        for (Integer userId : selectedUsers) {
            ActiveConferenceMember member = getMemberById(userId);
            try {
                activeConferenceContext.undeafUser(conference, member);
                validator.recordSuccess(getMessages().getMessage("msg.success.undeaf"));
            } catch (FreeswitchApiException fae) {
                validator.record(fae, getMessages());
            }
        }
    }

    public void muteUsers() {
        Collection<Integer> selectedUsers = getSelections().getAllSelected();
        SipxValidationDelegate validator = getValidator();
        ActiveConferenceContext activeConferenceContext = getActiveConferenceContext();
        Conference conference = getConference();
        for (Integer userId : selectedUsers) {
            ActiveConferenceMember member = getMemberById(userId);
            try {
                activeConferenceContext.muteUser(conference, member);
                validator.recordSuccess(getMessages().getMessage("msg.success.mute"));
            } catch (FreeswitchApiException fae) {
                validator.record(fae, getMessages());
            }
        }
    }

    public void unmuteUsers() {
        Collection<Integer> selectedUsers = getSelections().getAllSelected();
        SipxValidationDelegate validator = getValidator();
        ActiveConferenceContext activeConferenceContext = getActiveConferenceContext();
        Conference conference = getConference();
        for (Integer userId : selectedUsers) {
            ActiveConferenceMember member = getMemberById(userId);
            try {
                activeConferenceContext.unmuteUser(conference, member);
                validator.recordSuccess(getMessages().getMessage("msg.success.unmute"));
            } catch (FreeswitchApiException fae) {
                validator.record(fae, getMessages());
            }
        }
    }

    public void kickUsers() {
        Collection<Integer> selectedUsers = getSelections().getAllSelected();
        SipxValidationDelegate validator = getValidator();
        ActiveConferenceContext activeConferenceContext = getActiveConferenceContext();
        Conference conference = getConference();
        for (Integer userId : selectedUsers) {
            ActiveConferenceMember member = getMemberById(userId);
            try {
                activeConferenceContext.kickUser(conference, member);
                validator.recordSuccess(getMessages().getMessage("msg.success.kick"));
            } catch (FreeswitchApiException fae) {
                validator.record(fae, getMessages());
            }
        }
    }

    private ActiveConferenceMember getMemberById(Integer memberId) {
        for (ActiveConferenceMember member : getMembers()) {
            if (member.getId() == memberId.intValue()) {
                return member;
            }
        }

        return null;
    }
}
