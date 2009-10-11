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

@ComponentClass
public abstract class ImAccountPanel extends BaseComponent {

    @Parameter(required = true)
    public abstract User getUser();
    @Parameter
    public abstract Setting getActiveSetting();
    @Parameter(defaultValue = "false")
    public abstract boolean isImIdDisabled();

    public abstract String getDefaultImId();
    public abstract void setDefaultImId(String imId);
    public abstract String getDefaultImDisplayName();
    public abstract void setDefaultImDisplayName(String imDisplayName);

    @Override
    protected void prepareForRender(IRequestCycle cycle) {
        ImAccount imAccount = new ImAccount(getUser());
        setDefaultImId(imAccount.getDefaultImId());
        setDefaultImDisplayName(imAccount.getDefaultImDisplayName());
    }
}
