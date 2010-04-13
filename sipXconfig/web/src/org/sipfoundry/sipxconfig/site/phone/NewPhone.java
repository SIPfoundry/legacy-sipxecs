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

import java.util.Collections;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.PageRedirectException;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.site.setting.EditGroup;

/**
 * First page of wizard-like UI for creating a new phone
 */
public abstract class NewPhone extends PageWithCallback implements PageBeginRenderListener {

    public static final String PAGE = "phone/NewPhone";

    @InjectObject(value = "spring:phoneContext")
    public abstract PhoneContext getPhoneContext();

    @InjectObject(value = "spring:settingDao")
    public abstract SettingDao getSettingDao();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    public abstract PhoneModel getPhoneModel();

    public abstract void setPhoneModel(PhoneModel model);

    /**
     * If set we will add the line for this user after creating a new phone
     */
    @Persist
    public abstract Integer getUserId();

    public abstract void setUserId(Integer userId);

    public abstract Phone getPhone();

    public abstract void setPhone(Phone phone);

    @InitialValue("false")
    public abstract boolean isStay();

    public void addPhone(IRequestCycle cycle) {
        if (!TapestryUtils.isValid(cycle, this)) {
            return;
        }

        Phone phone = getPhone();
        EditGroup.saveGroups(getSettingDao(), phone.getGroups());
        getPhoneContext().storePhone(phone);
        Integer userId = getUserId();
        if (userId != null) {
            getPhoneContext().addUsersToPhone(phone.getId(), Collections.singleton(userId));
        }

        if (isStay()) {
            // triggers form to clear
            Phone nextPhone = getPhoneContext().newPhone(getPhoneModel());
            setPhone(nextPhone);
            // if no exception is throw the callback will be executed
            throw new PageRedirectException(this);
        }
    }

    public void pageBeginRender(PageEvent event) {
        Phone phone = getPhone();
        if (phone == null) {
            setPhone(getPhoneContext().newPhone(getPhoneModel()));
        }
    }
}
