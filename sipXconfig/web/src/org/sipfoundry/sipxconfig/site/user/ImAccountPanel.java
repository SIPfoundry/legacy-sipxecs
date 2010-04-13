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
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.im.ImAccount;
import org.sipfoundry.sipxconfig.setting.Setting;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class ImAccountPanel extends BaseComponent {
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
}
