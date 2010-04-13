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

import java.util.Collection;

import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.device.ProfileManager;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phonebook.Phonebook;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;

public abstract class UserPhones extends UserBasePage {
    public static final String PAGE = "user/UserPhones";

    @InjectObject(value = "spring:phoneContext")
    public abstract PhoneContext getPhoneContext();

    @InjectObject(value = "spring:phoneProfileManager")
    public abstract ProfileManager getPhoneProfileManager();

    @InjectObject(value = "spring:phonebookManager")
    public abstract PhonebookManager getPhonebookManager();

    @InjectPage(value = AddExistingPhone.PAGE)
    public abstract AddExistingPhone getAddExistingPhonePage();

    @Bean
    public abstract SelectMap getSelections();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    public abstract Collection<Integer> getGenerateProfileIds();

    public abstract void setPrivatePhonebook(Phonebook privatePhonebook);

    public abstract Phonebook getPrivatePhonebook();

    public abstract boolean getIsShared();

    public abstract void setIsShared(boolean shared);

    public Collection<Phone> getPhones() {
        return getPhoneContext().getPhonesByUserId(getUserId());
    }

    public IPage addExistingPhones() {
        AddExistingPhone page = getAddExistingPhonePage();
        page.setUserId(getUser().getId());
        page.setReturnPage(this);
        return page;
    }

    @Override
    public void pageBeginRender(PageEvent event) {
        super.pageBeginRender(event);
        setPrivatePhonebook(getPhonebookManager().getPrivatePhonebook(getUser()));
        setIsShared(getUser().getIsShared());
    }

    public void savePrivatePhonebook() {
        getPhonebookManager().savePhonebook(getPrivatePhonebook());
        TapestryUtils.recordSuccess(this, getMessages().getMessage("showOnPhone.success"));
    }

    public void saveSharedLine() {
        User user = getUser();
        user.setIsShared(getIsShared());
        getCoreContext().saveUser(user);
        TapestryUtils.recordSuccess(this, getMessages().getMessage("userSaved.success"));
    }
}
