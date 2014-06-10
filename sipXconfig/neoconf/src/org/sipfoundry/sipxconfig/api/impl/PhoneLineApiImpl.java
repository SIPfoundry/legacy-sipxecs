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

import org.sipfoundry.sipxconfig.api.PhoneLineApi;
import org.sipfoundry.sipxconfig.api.model.PhoneBean.LineBean;
import org.sipfoundry.sipxconfig.api.model.PhoneBean.LineList;
import org.sipfoundry.sipxconfig.api.model.SettingsList;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.Setting;

public class PhoneLineApiImpl implements PhoneLineApi {
    private static final String PHONE_NOT_FOUND = "Phone not found";
    private PhoneContext m_phoneContext;
    private CoreContext m_coreContext;

    @Override
    public Response deletePhoneLines(String phoneId) {
        Phone phone = getPhoneByIdOrMac(phoneId);
        if (phone == null) {
            return Response.status(Status.NOT_FOUND).entity(PHONE_NOT_FOUND).build();
        }
        for (Line line : phone.getLines()) {
            m_phoneContext.deleteLine(line);
        }
        return Response.ok().build();
    }

    @Override
    public Response newLine(String phoneId, LineBean lineBean) {
        Phone phone = getPhoneByIdOrMac(phoneId);
        if (phone == null) {
            return Response.status(Status.NOT_FOUND).entity(PHONE_NOT_FOUND).build();
        }
        Line line = phone.createLine();
        if (lineBean.getUser() != null) {
            User user = m_coreContext.loadUserByUserNameOrAlias(lineBean.getUser());
            if (user == null) {
                return Response.status(Status.NOT_FOUND).entity("User not found").build();
            }
            line.setUser(user);
        } else {
            LineInfo lineInfo = new LineInfo();
            lineInfo.setUserId(lineBean.getUserId());
            lineInfo.setDisplayName(lineBean.getDisplayName());
            lineInfo.setExtension(lineBean.getExtension());
            lineInfo.setPassword(lineBean.getPassword());
            lineInfo.setRegistrationServer(lineBean.getRegistrationServer());
            lineInfo.setRegistrationServerPort(lineBean.getRegistrationServerPort());
            lineInfo.setVoiceMail(lineBean.getVoicemail());
            line.setLineInfo(lineInfo);
        }
        phone.addLine(line);
        m_phoneContext.storePhone(phone);
        return Response.ok().build();
    }

    @Override
    public Response getPhoneLines(String phoneId) {
        Phone phone = getPhoneByIdOrMac(phoneId);
        if (phone != null) {
            return Response.ok().entity(LineList.convertLineList(phone.getLines())).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getPhoneLine(String phoneId, Integer lineId) {
        Line line = m_phoneContext.loadLine(lineId);
        if (line != null) {
            return Response.ok().entity(LineBean.convertLine(line)).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response deletePhoneLine(String phoneId, Integer lineId) {
        Line line = m_phoneContext.loadLine(lineId);
        if (line != null) {
            m_phoneContext.deleteLine(line);
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getPhoneLineSettings(String phoneId, Integer lineId, HttpServletRequest request) {
        Line line = m_phoneContext.loadLine(lineId);
        if (line != null) {
            Setting settings = line.getSettings();
            return Response.ok().entity(SettingsList.convertSettingsList(settings, request.getLocale())).build();
        }
        return null;
    }

    @Override
    public Response getPhoneLineSetting(String phoneId, Integer lineId, String path, HttpServletRequest request) {
        return ResponseUtils.buildSettingResponse(m_phoneContext.loadLine(lineId), path, request.getLocale());
    }

    @Override
    public Response setPhoneLineSetting(String phoneId, Integer lineId, String path, String value) {
        Line line = m_phoneContext.loadLine(lineId);
        if (line != null) {
            line.setSettingValue(path, value);
            m_phoneContext.storeLine(line);
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response deletePhoneLineSetting(String phoneId, Integer lineId, String path) {
        Line line = m_phoneContext.loadLine(lineId);
        if (line != null) {
            Setting setting = line.getSettings().getSetting(path);
            setting.setValue(setting.getDefaultValue());
            m_phoneContext.storeLine(line);
            return Response.ok().build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    private Phone getPhoneByIdOrMac(String id) {
        Phone phone = null;
        try {
            int phoneId = Integer.parseInt(id);
            phone = m_phoneContext.loadPhone(phoneId);
        } catch (NumberFormatException e) {
            // no id then it must be MAC
            phone = m_phoneContext.getPhoneBySerialNumber(id);
        }
        return phone;
    }

    public void setPhoneContext(PhoneContext context) {
        m_phoneContext = context;
    }

    public void setCoreContext(CoreContext context) {
        m_coreContext = context;
    }

}
