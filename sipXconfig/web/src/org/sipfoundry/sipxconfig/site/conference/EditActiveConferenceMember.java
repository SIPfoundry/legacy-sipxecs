/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */

package org.sipfoundry.sipxconfig.site.conference;

import java.util.Map;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.conference.ActiveConferenceContext;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;

public abstract class EditActiveConferenceMember extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "conference/EditActiveConferenceMember";

    @InjectObject(value = "spring:activeConferenceContext")
    public abstract ActiveConferenceContext getActiveConferenceContext();

    @InjectObject(value = "spring:conferenceBridgeContext")
    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    public abstract Map<String, String> getConferenceMemberNicknameMap();

    public abstract void setConferenceMemberNicknameMap(Map<String, String> nicknameMap);

    @Persist
    public abstract String getConferenceMemberUuid();

    public abstract void setConferenceMemberUuid(String id);

    public abstract String getNickname();

    public abstract void setNickname(String nickname);

    public void pageBeginRender(PageEvent event) {
        if (getNickname() == null && !event.getRequestCycle().isRewinding()) {
            setNickname(getConferenceMemberNicknameMap().get(getConferenceMemberUuid()));
        }
    }

    public void save() {
        getConferenceMemberNicknameMap().put(getConferenceMemberUuid(), getNickname());
    }
}
