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
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.collections.Closure;
import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.annotations.InjectState;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.components.IPrimaryKeyConverter;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.valid.IValidationDelegate;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DataObjectSource;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.ObjectSourceDataSqueezer;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.conference.ActiveConferenceContext;
import org.sipfoundry.sipxconfig.conference.ActiveConferenceMember;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.conference.DimDimConference;
import org.sipfoundry.sipxconfig.sip.SipService;
import org.sipfoundry.sipxconfig.site.UserSession;

public abstract class ActiveConferenceControl extends BaseComponent implements PageBeginRenderListener {
    private static final Log LOG = LogFactory.getLog(ActiveConferenceControl.class);

    private static final String TEXT_NUMBER = "text.number";

    @Bean
    public abstract SelectMap getSelections();

    @InjectObject(value = "spring:activeConferenceContext")
    public abstract ActiveConferenceContext getActiveConferenceContext();

    @InjectObject(value = "spring:conferenceBridgeContext")
    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    @InjectObject(value = "spring:sip")
    public abstract SipService getSipService();

    @InjectObject(value = "spring:coreContext")
    public abstract CoreContext getCoreContext();

    @InjectState(value = "userSession")
    public abstract UserSession getUserSession();

    @InjectPage(EditActiveConferenceMember.PAGE)
    public abstract EditActiveConferenceMember getEditActiveConferenceMemberPage();

    @Parameter
    public abstract Conference getConference();

    @Parameter
    public abstract SipxValidationDelegate getValidator();

    public abstract List<ActiveConferenceMember> getMembersCached();

    public abstract void setMembersCached(List<ActiveConferenceMember> members);

    public abstract ActiveConferenceMember getCurrentMember();

    public abstract String getInviteNumber();

    public abstract void setInviteNumber(String inviteNumber);

    @Persist
    public abstract Map<String, String> getConferenceMemberNicknameMap();

    public abstract void setConferenceMemberNicknameMap(Map nicknameMap);

    public abstract DimDimConference getDimDimConference();

    public abstract void setDimDimConference(DimDimConference dim);

    public void pageBeginRender(PageEvent event) {
        if (getConferenceMemberNicknameMap() == null) {
            setConferenceMemberNicknameMap(new HashMap<String, String>());
        }

        if (getDimDimConference() == null) {
            setDimDimConference(new DimDimConference(getConference()));
        }
    }

    public String getInviteFieldValue() {
        String placeholder = getMessages().getMessage(TEXT_NUMBER);
        String rawValue = getInviteNumber();

        return StringUtils.defaultIfEmpty(rawValue, placeholder);
    }

    public void setInviteFieldValue(String inviteFieldValue) {
        String placeholder = getMessages().getMessage(TEXT_NUMBER);
        if (inviteFieldValue == null || inviteFieldValue.equals(placeholder)) {
            setInviteNumber(null);
        } else {
            setInviteNumber(inviteFieldValue);
        }
    }

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

    @Asset(value = "context:/WEB-INF/conference/ActiveConferenceControl.script")
    public abstract IAsset getScript();

    @Asset(value = "context:/WEB-INF/conference/ControlCheckbox.script")
    public abstract IAsset getControlCheckboxScript();

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
        LOG.debug("Getting member list for table display");
        List<ActiveConferenceMember> members = getMembersCached();
        if (members != null) {
            LOG.debug("Using cached member list");
            return members;
        }
        members = Collections.emptyList();
        try {
            Conference conference = getConference();
            if (conference != null && conference.isEnabled()) {
                LOG.debug("Requesting latest member list");
                members = getActiveConferenceContext().getConferenceMembers(conference);
            }
        } catch (UserException e) {
            recordFailure(e);
        }

        LOG.debug("Using member list: " + members);

        setMembersCached(members);
        return members;
    }

    public String getNicknameForCurrentMember() {
        String nickname = getConferenceMemberNicknameMap().get(getCurrentMember().getUuid());
        // FIXME: needs to be localized
        return StringUtils.defaultIfEmpty(nickname, "Add a nickname");
    }

    public abstract class Action implements Closure {
        private final String m_msgSuccess;

        public Action(String msgSuccess) {
            m_msgSuccess = getMessages().getMessage(msgSuccess);
        }

        public void execute(Object id) {
            if (id != null) {
                ActiveConferenceMember member = getMemberById((Integer) id);

                if (member != null) {
                    execute(getConference(), member);
                    TapestryUtils.recordSuccess(ActiveConferenceControl.this, m_msgSuccess);
                }
            } else {
                recordFailure(new UserException(getMessages().getMessage("error.noSuchMemberNullId")));
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

    public void inviteParticipant() {
        if (TapestryUtils.isValid(this)) {
            String inviteNumber = getInviteNumber();
            User user = getUserSession().getUser(getCoreContext());
            getActiveConferenceContext().inviteParticipant(user, getConference(), inviteNumber);
            getValidator().recordSuccess(inviteNumber + " has been invited to this conference.");

            setInviteNumber(null);
        }
    }

    public void deafUsers() {
        Closure deaf = new Action("msg.success.deaf") {
            @Override
            public void execute(Conference conference, ActiveConferenceMember member) {
                getActiveConferenceContext().deafUser(conference, member);
            }
        };
        forAllMembers(deaf);
    }

    public void undeafUsers() {
        Closure deaf = new Action("msg.success.undeaf") {
            @Override
            public void execute(Conference conference, ActiveConferenceMember member) {
                getActiveConferenceContext().undeafUser(conference, member);
            }
        };
        forAllMembers(deaf);
    }

    public void muteUsers() {
        Closure deaf = new Action("msg.success.mute") {
            @Override
            public void execute(Conference conference, ActiveConferenceMember member) {
                getActiveConferenceContext().muteUser(conference, member);
            }
        };
        forAllMembers(deaf);
    }

    public void unmuteUsers() {
        Closure deaf = new Action("msg.success.unmute") {
            @Override
            public void execute(Conference conference, ActiveConferenceMember member) {
                getActiveConferenceContext().unmuteUser(conference, member);
            }
        };
        forAllMembers(deaf);
    }

    public void kickUsers() {
        Closure kick = new Action("msg.success.kick") {
            @Override
            public void execute(Conference conference, ActiveConferenceMember member) {
                getActiveConferenceContext().kickUser(conference, member);
            }
        };
        forAllMembers(kick);
    }

    public IPage editCurrentMember(String conferenceMemberUuid) {
        EditActiveConferenceMember editMemberPage = getEditActiveConferenceMemberPage();
        editMemberPage.setReturnPage(getPage());
        editMemberPage.setConferenceMemberUuid(conferenceMemberUuid);
        editMemberPage.setConferenceMemberNicknameMap(getConferenceMemberNicknameMap());
        return editMemberPage;
    }
}
