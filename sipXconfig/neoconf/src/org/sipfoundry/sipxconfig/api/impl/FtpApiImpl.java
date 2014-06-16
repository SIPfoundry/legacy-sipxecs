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

import org.sipfoundry.sipxconfig.api.FtpApi;
import org.sipfoundry.sipxconfig.api.model.SettingsList;
import org.sipfoundry.sipxconfig.ftp.FtpManager;
import org.sipfoundry.sipxconfig.ftp.FtpSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class FtpApiImpl implements FtpApi {
    private FtpManager m_ftpManager;

    @Override
    public Response getFtpSettings(HttpServletRequest request) {
        FtpSettings ftpSettings = m_ftpManager.getSettings();
        if (ftpSettings != null) {
            Setting settings = ftpSettings.getSettings();
            return Response.ok().entity(SettingsList.convertSettingsList(settings, request.getLocale())).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getFtpSetting(String path, HttpServletRequest request) {
        FtpSettings ftpSettings = m_ftpManager.getSettings();
        return ResponseUtils.buildSettingResponse(ftpSettings, path, request.getLocale());
    }

    @Override
    public Response setFtpSetting(String path, String value) {
        FtpSettings ftpSettings = m_ftpManager.getSettings();
        if (ftpSettings != null) {
            ftpSettings.setSettingValue(path, value);
            m_ftpManager.saveSettings(ftpSettings);
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response deleteFtpSetting(String path) {
        FtpSettings ftpSettings = m_ftpManager.getSettings();
        if (ftpSettings != null) {
            Setting setting = ftpSettings.getSettings().getSetting(path);
            setting.setValue(setting.getDefaultValue());
            m_ftpManager.saveSettings(ftpSettings);
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    public void setFtpManager(FtpManager manager) {
        m_ftpManager = manager;
    }

}
