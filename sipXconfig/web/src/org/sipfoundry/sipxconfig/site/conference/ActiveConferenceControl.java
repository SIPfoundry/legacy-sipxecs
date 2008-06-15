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

import java.io.Serializable;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.apache.commons.collections.Closure;
import org.apache.commons.collections.CollectionUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.components.IPrimaryKeyConverter;
import org.apache.tapestry.valid.IValidationDelegate;
import org.sipfoundry.sipxconfig.common.DataObjectSource;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.ObjectSourceDataSqueezer;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.conference.ActiveConferenceContext;
import org.sipfoundry.sipxconfig.conference.ActiveConferenceMember;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;

public abstract class ActiveConferenceControl extends BaseComponent {
    @Bean
    public abstract SelectMap getSelections();

    @InjectObject(value = "spring:activeConferenceContext")
    public abstract ActiveConferenceContext getActiveConferenceContext();

    @InjectObject(value = "spring:conferenceBridgeContext")
    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    @Parameter
    public abstract Conference getConference();

    public abstract List<ActiveConferenceMember> getMembersCached();

    public abstract void setMembersCached(List<ActiveConferenceMember> members);

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

    void recordFailure(UserException ue) {
        IValidationDelegate validator = TapestryUtils.getValidator(this);
        if (validator instanceof SipxValidationDelegate) {
            SipxValidationDelegate v = (SipxValidationDelegate) validator;
            v.record(ue, getMessages());
        }
    }

    public IPrimaryKeyConverter getConverter() {
        DataObjectSource< ? > source = new DataObjectSource<ActiveConferenceMember>() {
            public ActiveConferenceMember load(Class<ActiveConferenceMember> c, Serializable serializable) {
                return getMemberById((Integer) serializable);
            }
        };
        ObjectSourceDataSqueezer squeezer = new ObjectSourceDataSqueezer();
        squeezer.setClass(ActiveConferenceMember.class);
        squeezer.setSource(source);
        return squeezer;
    }

    public List<ActiveConferenceMember> getMembers() {
        List<ActiveConferenceMember> members = getMembersCached();
        if (members != null) {
            return members;
        }
        members = Collections.emptyList();
        try {
            Conference conference = getConference();
            if (conference != null && conference.isEnabled()) {
                members = getActiveConferenceContext().getConferenceMembers(conference);
            }
        } catch (UserException e) {
            recordFailure(e);
        }
        setMembersCached(members);
        return Collections.emptyList();
    }

    public abstract class Action implements Closure {
        private String m_msgSuccess;

        public Action(String msgSuccess) {
            m_msgSuccess = getMessages().getMessage(msgSuccess);
        }

        public void execute(Object id) {
            ActiveConferenceMember member = getMemberById((Integer) id);
            if (member != null) {
                execute(getConference(), member);
                TapestryUtils.recordSuccess(ActiveConferenceControl.this, m_msgSuccess);
            }
        }

        public abstract void execute(Conference conference, ActiveConferenceMember member);
    }

    private ActiveConferenceMember getMemberById(int memberId) {
        for (ActiveConferenceMember member : getMembers()) {
            if (member.getId() == memberId) {
                return member;
            }
        }

        return null;
    }

    private void forAllMembers(Closure closure) {
        Collection<Integer> selectedUsers = getSelections().getAllSelected();
        if (selectedUsers.isEmpty()) {
            return;
        }
        CollectionUtils.forAllDo(selectedUsers, closure);
        // probably some status changes - force new read
        setMembersCached(null);
    }

    public void deafUsers() {
        Closure deaf = new Action("msg.success.deaf") {
            public void execute(Conference conference, ActiveConferenceMember member) {
                getActiveConferenceContext().deafUser(conference, member);
            }
        };
        forAllMembers(deaf);
    }

    public void undeafUsers() {
        Closure deaf = new Action("msg.success.undeaf") {
            public void execute(Conference conference, ActiveConferenceMember member) {
                getActiveConferenceContext().undeafUser(conference, member);
            }
        };
        forAllMembers(deaf);
    }

    public void muteUsers() {
        Closure deaf = new Action("msg.success.mute") {
            public void execute(Conference conference, ActiveConferenceMember member) {
                getActiveConferenceContext().muteUser(conference, member);
            }
        };
        forAllMembers(deaf);
    }

    public void unmuteUsers() {
        Closure deaf = new Action("msg.success.unmute") {
            public void execute(Conference conference, ActiveConferenceMember member) {
                getActiveConferenceContext().unmuteUser(conference, member);
            }
        };
        forAllMembers(deaf);
    }

    public void kickUsers() {
        Closure kick = new Action("msg.success.kick") {
            public void execute(Conference conference, ActiveConferenceMember member) {
                getActiveConferenceContext().kickUser(conference, member);
            }
        };
        forAllMembers(kick);
    }
}
