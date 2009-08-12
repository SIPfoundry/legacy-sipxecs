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

import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.device.DiscoveredDevice;
import org.sipfoundry.sipxconfig.phone.DeviceFinder;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;

public abstract class DiscoverDevices extends UserBasePage {
    public static final String PAGE = "DiscoverDevices";

    private static final String ACTION_REDISCOVER = "discover";
    private static final String ACTION_CLEAR = "clear";

    @InjectObject(value = "spring:deviceFinder")
    public abstract DeviceFinder getTaskExecutor();

    public abstract void setTargets(List<DiscoveredDevice> targets);

    public abstract List<DiscoveredDevice> getTargets();

    public abstract void setSelections(SelectMap selections);

    public abstract SelectMap getSelections();

    public abstract String getAction();

    public abstract boolean isDiscovering();

    public abstract void setDiscovering(boolean discovering);

    public abstract boolean isDiscoveryExecuted();

    public abstract void setDiscoveryExecuted(boolean discoveryExecuted);

    public abstract boolean isDiscoveryFailed();

    public abstract void setDiscoveryFailed(boolean discoveryFailed);

    public abstract boolean isDiscoveryNotStarted();

    public abstract void setDiscoveryNotStarted(boolean discoveryFailed);

    @Override
    public void pageBeginRender(PageEvent event) {
        setDiscovering(false);
        setDiscoveryExecuted(false);
        setDiscoveryFailed(false);
        setDiscoveryNotStarted(false);
        String state = getTaskExecutor().getState();
        if (state.equals(DeviceFinder.RUNNING)) {
            setDiscovering(true);
        } else if (state.equals(DeviceFinder.FINISHED)) {
            setDiscoveryExecuted(true);
            setTargets(getTaskExecutor().getDiscoveredDevices());
        } else if (state.equals(DeviceFinder.FAILED)) {
            setDiscoveryFailed(true);
        } else if (state.equals(DeviceFinder.NOT_STARTED)) {
            setDiscoveryNotStarted(true);
        }
        setSelections(new SelectMap());
    }

    public void editTargets(String returnPage) {
        setReturnPage(returnPage);
    }

    public void submit() {
        if (ACTION_REDISCOVER.equals(getAction())) {
            getTaskExecutor().start();
            return;
        } else if (ACTION_CLEAR.equals(getAction())) {
            // clears list
            setSelections(null);
        }
    }

    public void save() {
        List<DiscoveredDevice> devicesToSave = new ArrayList<DiscoveredDevice>();
        List<String> selectedTargetNames = new ArrayList<String>();

        for (Object selection : getSelections().getAllSelected()) {
            selectedTargetNames.add(selection.toString());
        }

        List<String> devicesWithoutModel = new ArrayList<String>();
        for (Object deviceObject : getTargets()) {
            DiscoveredDevice device = (DiscoveredDevice) deviceObject;
            if (selectedTargetNames.contains(device.getMacAddress())) {
                if (device.getModel() != null) {
                    devicesToSave.add(device);
                } else {
                    devicesWithoutModel.add(device.getMacAddress());
                }
            }
        }

        getTaskExecutor().saveDiscoveredDevices(devicesToSave);

        if (devicesWithoutModel.size() > 0) {
            StringBuilder error = new StringBuilder(getMessages().getMessage(
                    "error.modelNotSelected"));
            error.append(StringUtils.join(devicesWithoutModel, ", "));
            TapestryUtils.getValidator(getPage())
                    .record(new ValidatorException(error.toString()));
        } else {
            TapestryUtils.recordSuccess(getPage(), getMessages().getMessage("msg.actionSuccess"));
        }
    }
}
