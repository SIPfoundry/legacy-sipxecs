/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.phone;

import java.util.Collection;
import java.util.Collections;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.device.ProfileManager;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.site.setting.EditGroup;

/**
 * Tapestry Page support for editing and creating new phones
 */
public abstract class EditPhone extends PageWithCallback implements PageBeginRenderListener {

    public static final String PAGE = "phone/EditPhone";

    public abstract Phone getPhone();

    public abstract void setPhone(Phone phone);

    /** REQUIRED PROPERTY */
    @Persist
    public abstract Integer getPhoneId();

    public abstract void setPhoneId(Integer id);

    @InjectObject(value = "spring:phoneContext")
    public abstract PhoneContext getPhoneContext();

    @InjectObject(value = "spring:settingDao")
    public abstract SettingDao getSettingDao();

    @InjectObject(value = "spring:phoneProfileManager")
    public abstract ProfileManager getPhoneProfileManager();

    public abstract String getActiveTab();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    public abstract Collection<Integer> getGenerateProfileIds();

    public abstract void setGenerateProfileIds(Collection<Integer> ids);

    public IPage addLine(IRequestCycle cycle, Integer phoneId) {
        AddPhoneUser page = (AddPhoneUser) cycle.getPage(AddPhoneUser.PAGE);
        page.setReturnToEditPhone(true);
        page.setPhoneId(phoneId);
        return page;
    }

    public void commit() {
        save();
    }

    private boolean save() {
        boolean valid = TapestryUtils.isValid(this);
        if (valid) {
            PhoneContext dao = getPhoneContext();
            Phone phone = getPhone();
            EditGroup.saveGroups(getSettingDao(), phone.getGroups());
            dao.storePhone(phone);
        }

        return valid;
    }

    public void generateProfile() {
        Collection<Integer> phoneIds = Collections.singleton(getPhone().getId());
        setGenerateProfileIds(phoneIds);
    }

    public void pageBeginRender(PageEvent event_) {
        if (getPhone() != null) {
            return;
        }

        // Load the phone with the ID that was passed in
        PhoneContext context = getPhoneContext();
        setPhone(context.loadPhone(getPhoneId()));
    }
}
