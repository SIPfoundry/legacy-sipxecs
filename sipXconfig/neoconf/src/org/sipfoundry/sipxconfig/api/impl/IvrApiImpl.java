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

import org.sipfoundry.sipxconfig.api.IvrApi;
import org.sipfoundry.sipxconfig.api.model.SettingsList;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.sipfoundry.sipxconfig.ivr.IvrSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class IvrApiImpl implements IvrApi {
    private Ivr m_ivr;

    @Override
    public Response getIvrSettings(HttpServletRequest request) {
        IvrSettings ivrSettings = m_ivr.getSettings();
        if (ivrSettings != null) {
            Setting settings = ivrSettings.getSettings();
            return Response.ok().entity(SettingsList.convertSettingsList(settings, request.getLocale())).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getIvrSetting(String path, HttpServletRequest request) {
        IvrSettings ivrSettings = m_ivr.getSettings();
        return ResponseUtils.buildSettingResponse(ivrSettings, path, request.getLocale());
    }

    @Override
    public Response setIvrSetting(String path, String value) {
        IvrSettings ivrSettings = m_ivr.getSettings();
        if (ivrSettings != null) {
            ivrSettings.setSettingValue(path, value);
            m_ivr.saveSettings(ivrSettings);
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response deleteIvrSetting(String path) {
        IvrSettings ivrSettings = m_ivr.getSettings();
        if (ivrSettings != null) {
            Setting setting = ivrSettings.getSettings().getSetting(path);
            setting.setValue(setting.getDefaultValue());
            m_ivr.saveSettings(ivrSettings);
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    public void setIvr(Ivr ivr) {
        m_ivr = ivr;
    }

}
