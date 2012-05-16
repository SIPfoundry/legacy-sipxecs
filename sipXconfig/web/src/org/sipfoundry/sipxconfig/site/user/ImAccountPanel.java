/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.user;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.im.ImAccount;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.sipfoundry.sipxconfig.setting.Setting;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class ImAccountPanel extends BaseComponent {

    @InjectObject(value = "spring:featureManager")
    public abstract FeatureManager getFeatureManager();

    @Parameter(required = true)
    public abstract User getUser();

    @Parameter(defaultValue = "true")
    public abstract boolean isAdminMode();

    public abstract ImAccount getImAccount();

    public abstract void setImAccount(ImAccount imAccount);

    public abstract Setting getImSettings();

    public abstract void setImSettings(Setting imSetting);

    @Override
    protected void prepareForRender(IRequestCycle cycle) {
        if (getImAccount() == null) {
            setImAccount(new ImAccount(getUser()));
        }
        if (getImSettings() == null) {
            setImSettings(getUser().getSettings().getSetting("im"));
        }
    }

    public String getSettingsToHide() {
        String defaultSettingsToHide = "im-account, im-group, add-pa-to-group";
        if (isVoicemailEnabled()) {
            return defaultSettingsToHide;
        } else {
            return defaultSettingsToHide + ", fwd-vm-on-dnd";
        }
    }

    private boolean isVoicemailEnabled() {
        return (getFeatureManager().isFeatureEnabled(Ivr.FEATURE) ? true : false);
    }
}
