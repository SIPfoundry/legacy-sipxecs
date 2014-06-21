/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.api.impl;

import javax.servlet.http.HttpServletRequest;
import javax.ws.rs.core.Response;
import javax.ws.rs.core.Response.Status;

import org.sipfoundry.sipxconfig.api.ServiceSettingsApi;
import org.sipfoundry.sipxconfig.api.model.SettingsList;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public abstract class BaseServiceApiImpl extends PromptsApiImpl implements ServiceSettingsApi {

    @Override
    public Response getServiceSettings(HttpServletRequest request) {
        PersistableSettings serviceSettings = getSettings();
        if (serviceSettings != null) {
            Setting settings = serviceSettings.getSettings();
            return Response.ok().entity(SettingsList.convertSettingsList(settings, request.getLocale())).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getServiceSetting(String path, HttpServletRequest request) {
        PersistableSettings serviceSettings = getSettings();
        return ResponseUtils.buildSettingResponse(serviceSettings, path, request.getLocale());
    }

    @Override
    public Response setServiceSetting(String path, String value) {
        PersistableSettings serviceSettings = getSettings();
        if (serviceSettings != null) {
            serviceSettings.setSettingValue(path, value);
            saveSettings(serviceSettings);
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response deleteServiceSetting(String path) {
        PersistableSettings serviceSettings = getSettings();
        if (serviceSettings != null) {
            Setting setting = serviceSettings.getSettings().getSetting(path);
            setting.setValue(setting.getDefaultValue());
            saveSettings(serviceSettings);
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    protected abstract PersistableSettings getSettings();
    protected abstract void saveSettings(PersistableSettings settings);

}
