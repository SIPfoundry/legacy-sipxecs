/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.polycom;


import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Responsible for generating ipmid.cfg
 */
public class SiteConfiguration extends ProfileContext {

    public SiteConfiguration(PolycomPhone device) {
        super(device, device.getTemplateDir() + "/site.cfg.vm");
    }
    
    public String[] getEmergencySetting() {
        String emergencyValue = getEndpointSettings().getSetting(PolycomPhone.EMERGENCY).getValue();
        return StringUtils.split(emergencyValue, ",");
    }

    public String[] getValueCodecs(Setting setting) {
        return StringUtils.split(setting.getValue(), "|");
    }

}
