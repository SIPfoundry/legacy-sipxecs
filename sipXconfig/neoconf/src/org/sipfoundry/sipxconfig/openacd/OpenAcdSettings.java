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
package org.sipfoundry.sipxconfig.openacd;

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class OpenAcdSettings extends PersistableSettings implements DeployConfigOnEdit {
    private static final String FS_ENABLED = "freeswitch_media_manager/FREESWITCH_ENABLED";
    private static final String C_NODE = "freeswitch_media_manager/C_NODE";
    private static final String DIAL_STRING = "freeswitch_media_manager/DIAL_STRING";
    private static final String DIALPLAN_LISTENER = "agent_configuration/DIALPLAN_LISTENER";
    private static final String LOG_DIR = "openacd-config/log_dir";
    private DomainManager m_domainManager;
    private String m_audioDirectory;

    public OpenAcdSettings() {
        addDefaultBeanSettingHandler(new Defaults());
    }

    public class Defaults {
        @SettingEntry(path = C_NODE)
        public String getCNode() {
            // change this when installing on different locations will be supported
            return String.format("%s@127.0.0.1", "freeswitch");
        }

        @SettingEntry(path = DIAL_STRING)
        public String getDialString() {
            // change this when installing on different locations will be supported
            return String.format(
                    "{ignore_early_media=true}sofia/%s/$1;sipx-noroute=VoiceMail;sipx-userforward=false",
                    m_domainManager.getDomainName());
        }

        @SettingEntry(path = LOG_DIR)
        public String getLogDir() {
            return "/var/log/openacd";
        }
    }

    public String getCNode() {
        return getSettingValue(C_NODE);
    }

    public boolean isEnabled() {
        return (Boolean) getSettingTypedValue(FS_ENABLED);
    }

    public String getLogLevel() {
        return getSettingValue("openacd-config/log_level");
    }

    public String getLogDir() {
        return getSettingValue(LOG_DIR);
    }

    public String getDialString() {
        return getSettingValue(DIAL_STRING);
    }

    public boolean getDialPlanListener() {
        return (Boolean) getSettingTypedValue(DIALPLAN_LISTENER);
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("openacd/sipxopenacd.xml");
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) OpenAcdContext.FEATURE);
    }

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    public void setAudioDirectory(String audioDirectory) {
        m_audioDirectory = audioDirectory;
    }

    public String getAudioDirectory() {
        return m_audioDirectory;
    }

    @Override
    public String getBeanId() {
        return "openAcdSettings";
    }
}
