/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.user;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.callback.ICallback;
import org.sipfoundry.sipxconfig.common.AbstractUser;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.setting.Setting;

public abstract class UserSettingsPanel extends BaseComponent {
    @InjectObject("spring:coreContext")
    public abstract CoreContext getCoreContext();

    @Parameter
    public abstract ICallback getCallback();

    @Parameter(required = true)
    public abstract AbstractUser getUser();

    public abstract void setUser(AbstractUser user);

    @Parameter(required = true)
    public abstract String getGroupToDisplay();

    @Parameter(required = false)
    public abstract String getSettingsToHide();

    @Persist
    public abstract String getParentSettingName();

    public abstract void setParentSettingName(String settingName);

    /** REQUIRED PAGE PARAMETER */
    public abstract Setting getParentSetting();

    public abstract void setParentSetting(Setting parent);

    @Override
    protected void prepareForRender(IRequestCycle cycle) {
        if (getUser() == null) {
            setUser(getCoreContext().newUser());
        }
        Setting root = getUser().getSettings();
        Setting parent = root.getSetting(getGroupToDisplay());
        setParentSetting(parent);
    }
}
