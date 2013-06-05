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

import java.util.LinkedList;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.LocalizationUtils;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.im.ImAccount;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;

public abstract class UserSettings extends UserBasePage implements PageBeginRenderListener {

    public static final String PAGE = "user/UserSettings";

    @InjectObject("spring:featureManager")
    public abstract FeatureManager getFeatureManager();

    public abstract User getUser();

    public abstract void setUser(User user);

    @Persist
    public abstract String getParentSettingName();

    public abstract void setParentSettingName(String settingName);

    /** REQUIRED PAGE PARAMETER */
    public abstract Setting getParentSetting();

    public abstract void setParentSetting(Setting parent);

    @InjectObject("spring:ldapManager")
    public abstract LdapManager getLdapManager();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Override
    public void pageBeginRender(PageEvent event_) {
        User user = getUser();
        if (user != null) {
            return;
        }

        user = getCoreContext().loadUser(getUserId());
        setUser(user);
        Setting root = user.getSettings();
        Setting parent = root.getSetting(getParentSettingName());
        setParentSetting(parent);
    }

    public void apply() {
        ImAccount imAccount = new ImAccount(getUser());
        if (imAccount.getDefaultImDisplayName().equals(getUser().getImDisplayName())) {
            getUser().setImDisplayName(null);
        }

        User savedUser = getCoreContext().loadUser(getUserId());
        boolean adminChanged = getUser().isAdmin() != savedUser.isAdmin();

        CoreContext dao = getCoreContext();
        dao.saveUser(getUser());
//        if (adminChanged) {
//            getLdapManager().replicateOpenfireConfig();
//        }
    }

    public String getParentSettingLabel() {
        Setting setting = getParentSetting();
        return LocalizationUtils.getModelMessage(this, setting.getMessageSource(), setting.getLabelKey(),
                StringUtils.EMPTY);
    }

    public String getParentSettingDescription() {
        Setting setting = getParentSetting();
        return LocalizationUtils.getModelMessage(this, setting.getMessageSource(), setting
                .getDescriptionKey(), StringUtils.EMPTY);
    }

    public String getSettingsToHide() {
        List<String> names = new LinkedList<String>();
        if (!getFeatureManager().isFeatureEnabled(Ivr.FEATURE)) {
            names.add("leaveMsgBeginIM, leaveMsgEndIM");
        }
        return StringUtils.join(names, ",");
    }

    @Override
    public String getBorderTitle() {
        return getParentSettingLabel();
    }
}
