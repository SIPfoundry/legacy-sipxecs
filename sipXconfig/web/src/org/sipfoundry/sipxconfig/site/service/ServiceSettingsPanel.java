/*
 *
 *
 * Copyright (C) 2011 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.site.service;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.callback.ICallback;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.setting.Setting;

public abstract class ServiceSettingsPanel extends BaseComponent {
    @InjectObject("spring:coreContext")
    public abstract CoreContext getCoreContext();

    @Parameter
    public abstract ICallback getCallback();

    @Parameter(required = true)
    public abstract SipxService getService();

    public abstract void setService(SipxService service);

    @Parameter(required = true)
    public abstract String getGroupToDisplay();

    @Parameter(required = false)
    public abstract String getSettingsToHide();

    @Parameter(defaultValue = "true")
    public abstract boolean getRenderGroupTitle();

    public abstract void setRenderGroupTitle(boolean render);

    @Persist
    public abstract String getParentSettingName();

    public abstract void setParentSettingName(String settingName);

    /** REQUIRED PAGE PARAMETER */
    public abstract Setting getParentSetting();

    public abstract void setParentSetting(Setting parent);

    @Override
    protected void prepareForRender(IRequestCycle cycle) {
        Setting root = getService().getSettings();
        Setting parent = root.getSetting(getGroupToDisplay());
        setParentSetting(parent);
    }
}
