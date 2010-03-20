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

import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Edit vendor specific phone setttings in abstract manor using setting model of meta data
 */
public abstract class PhoneSettings extends SipxBasePage implements PageBeginRenderListener {

    public static final String PAGE = "phone/PhoneSettings";

    public abstract Integer getPhoneId();

    /** REQUIRED PAGE PARAMETER */
    public abstract void setPhoneId(Integer id);

    public abstract Phone getPhone();

    public abstract void setPhone(Phone phone);

    public abstract String getParentSettingName();

    /** REQUIRED PAGE PARAMETER */
    public abstract void setParentSettingName(String name);

    public abstract Setting getParentSetting();

    public abstract void setParentSetting(Setting parent);

    public abstract PhoneContext getPhoneContext();

    public void pageBeginRender(PageEvent event_) {
        Phone phone = getPhone();
        if (phone != null) {
            return;
        }

        phone = getPhoneContext().loadPhone(getPhoneId());
        setPhone(phone);
        Setting root = phone.getSettings();
        Setting parent = root.getSetting(getParentSettingName());
        setParentSetting(parent);
    }

    public String ok() {
        apply();
        return ManagePhones.PAGE;
    }

    public void apply() {
        PhoneContext dao = getPhoneContext();
        dao.storePhone(getPhone());
    }

    public String cancel() {
        return ManagePhones.PAGE;
    }
}
