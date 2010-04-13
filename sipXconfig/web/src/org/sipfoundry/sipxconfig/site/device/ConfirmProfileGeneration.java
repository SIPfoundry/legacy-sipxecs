/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.device;

import java.util.Collection;
import java.util.Date;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.valid.IValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.device.ProfileManager;

public abstract class ConfirmProfileGeneration extends BaseComponent {
    @Parameter(required = true)
    public abstract ProfileManager getProfileManager();

    @Parameter(required = true)
    public abstract Collection<Integer> getDeviceIds();

    @Parameter(required = true)
    public abstract IValidationDelegate getValidator();

    public abstract void setDeviceIds(Collection<Integer> ids);

    @InitialValue("true")
    public abstract boolean getRestart();

    public abstract void setRestartDate(Date restartDate);

    public abstract Date getRestartDate();

    @Override
    protected void prepareForRender(IRequestCycle cycle) {
        if (getRestartDate() == null) {
            setRestartDate(new Date());
        }
    }

    public void generate() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        Collection<Integer> deviceIds = getDeviceIds();
        getProfileManager().generateProfiles(deviceIds, getRestart(), getRestartDate());
        String msg = getMessages().format("msg.success.profiles", deviceIds.size());
        TapestryUtils.recordSuccess(this, msg);
        setDeviceIds(null);
    }

    public void cancel() {
        setDeviceIds(null);
    }

    public String getPrompt() {
        Collection<Integer> deviceIds = getDeviceIds();
        return getMessages().format("prompt", deviceIds.size());
    }
}
