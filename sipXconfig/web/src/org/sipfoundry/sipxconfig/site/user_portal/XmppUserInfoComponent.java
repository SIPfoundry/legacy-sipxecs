/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.user_portal;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.Setting;

public abstract class XmppUserInfoComponent extends BaseComponent {
    @Parameter(required = true)
    public abstract User getUser();

    @InjectObject(value = "spring:coreContext")
    public abstract CoreContext getCoreContext();

    public void save() {
        if (TapestryUtils.isValid(this)) {
            getCoreContext().saveUser(getUser());
        }
    }

    public Setting getOpenfireSettings() {
        return getUser().getSettings().getSetting("openfire");
    }
}
