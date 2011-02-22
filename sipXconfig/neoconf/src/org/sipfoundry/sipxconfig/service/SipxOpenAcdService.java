/*
 *
 *
 * Copyright (C) 2010 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.service;

import java.util.Collections;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.openacd.FreeswitchMediaCommand;
import org.sipfoundry.sipxconfig.openacd.OpenAcdAgentConfigCommand;
import org.sipfoundry.sipxconfig.openacd.OpenAcdProvisioningContext;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class SipxOpenAcdService extends SipxService {
    public static final String BEAN_ID = "sipxOpenAcdService";
    public static final String FS_ENABLED = "freeswitch_media_manager/FREESWITCH_ENABLED";
    public static final String C_NODE = "freeswitch_media_manager/C_NODE";
    public static final String DIAL_STRING = "freeswitch_media_manager/DIAL_STRING";
    public static final String DIALPLAN_LISTENER = "agent_configuration/DIALPLAN_LISTENER";

    private String m_audioDirectory;
    private OpenAcdProvisioningContext m_provisioningContext;

    public String getAudioDir() {
        return m_audioDirectory;
    }

    public void setAudioDir(String audioDirectory) {
        m_audioDirectory = audioDirectory;
    }

    public void setProvisioningContext(OpenAcdProvisioningContext context) {
        m_provisioningContext = context;
    }

    @Override
    public void initialize() {
        addDefaultBeanSettingHandler(new DefaultSettings(getLocationsManager().getPrimaryLocation()));
    }

    @Override
    public void onInit() {
        saveOpenAcdSettings();
    }

    @Override
    public void onConfigChange() {
        saveOpenAcdSettings();
    }

    private void saveOpenAcdSettings() {
        Boolean enabled = (Boolean) getSettingTypedValue(FS_ENABLED);
        String cNode = getSettingValue(C_NODE);
        String dialString = getSettingValue(DIAL_STRING);
        m_provisioningContext.configure(Collections.singletonList(new FreeswitchMediaCommand(enabled, cNode,
                dialString)));
        Boolean dialPlanListener = (Boolean) getSettingTypedValue(DIALPLAN_LISTENER);
        m_provisioningContext.configure(Collections.singletonList(new OpenAcdAgentConfigCommand(dialPlanListener)));
    }

    public static class DefaultSettings {

        private Location m_location;

        public DefaultSettings(Location location) {
            m_location = location;
        }

        @SettingEntry(path = C_NODE)
        public String getCNode() {
            // change this when installing on different locations will be supported
            return String.format("%s@127.0.0.1", m_location.getHostname());
        }

        @SettingEntry(path = DIAL_STRING)
        public String getDialString() {
            // change this when installing on different locations will be supported
            return String.format("{ignore_early_media=true}sofia/%s/$1", m_location.getFqdn());
        }
    }
}
