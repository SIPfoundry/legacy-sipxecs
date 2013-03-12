/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.phone.polycom;


import java.util.List;
import java.util.Map;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.cert.CertificateManager;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Responsible for generating ipmid.cfg
 */
public class SiteConfiguration extends ProfileContext {
    private CertificateManager m_certificateManager;

    public SiteConfiguration(PolycomPhone device, CertificateManager certificateManager) {
        super(device, device.getTemplateDir() + "/site.cfg.vm");
        m_certificateManager = certificateManager;
    }

    public String[] getEmergencySetting() {
        String emergencyValue = getEndpointSettings().getSetting(PolycomPhone.EMERGENCY).getValue();
        return StringUtils.split(emergencyValue, ",");
    }

    public String[] getValueCodecs(Setting setting) {
        return StringUtils.split(setting.getValue(), "|");
    }

    public int getLineCount() {
        PolycomPhone phone = (PolycomPhone) getDevice();
        List<Line> lines = phone.getLines();

        // Phones with no configured lines will register under the sipXprovision special user.
        if (lines.isEmpty()) {
            return 1;
        }

        return lines.size();
    }

    @Override
    public Map<String, Object> getContext() {
        Map<String, Object> context = super.getContext();
        getDevice().getSettings();
        context.put("lines", getLineCount());

        context.put("cert", m_certificateManager.getSelfSigningAuthorityText());

        return context;
    }

}
