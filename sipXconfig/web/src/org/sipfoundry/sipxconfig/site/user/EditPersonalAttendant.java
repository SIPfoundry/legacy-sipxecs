/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.user;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.attendant.PersonalAttendant;

public abstract class EditPersonalAttendant extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "user/EditPersonalAttendant";

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectObject(value = "spring:coreContext")
    public abstract CoreContext getCoreContext();

    @InjectObject(value = "spring:mailboxManager")
    public abstract MailboxManager getMailboxManager();

    @Persist
    public abstract Integer getUserId();
    public abstract void setUserId(Integer userId);

    public abstract Setting getParentSetting();
    public abstract void setParentSetting(Setting setting);

    public abstract User getUser();
    public abstract void setUser(User user);

    @Persist
    public abstract PersonalAttendant getPersonalAttendant();
    public abstract void setPersonalAttendant(PersonalAttendant personalAttendant);

    public abstract IPropertySelectionModel getLanguagesSelectionModel();
    public abstract void setLanguagesSelectionModel(IPropertySelectionModel languageList);

    public void pageBeginRender(PageEvent event) {
        if (getUser() == null) {
            User user = getCoreContext().loadUser(getUserId());
            setUser(user);

            Setting personalAttendantSetting = user.getSettings().getSetting("personal-attendant");
            setParentSetting(personalAttendantSetting);
        }

        if (getPersonalAttendant() == null || !getPersonalAttendant().getUser().equals(getUser())) {
            PersonalAttendant attendant = getMailboxManager().loadPersonalAttendantForUser(getUser());
            setPersonalAttendant(attendant);
        }
    }

    public void commit() {
        getMailboxManager().storePersonalAttendant(getPersonalAttendant());
        getCoreContext().saveUser(getUser());
    }
}
