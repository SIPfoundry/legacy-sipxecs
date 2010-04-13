/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.conference;

import java.io.File;
import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedHashSet;
import java.util.Set;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.service.LocationSpecificService;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;
import org.sipfoundry.sipxconfig.setting.AbstractSettingVisitor;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.type.FileSetting;
import org.sipfoundry.sipxconfig.setting.type.SettingType;

public class Bridge extends BeanWithSettings {
    public static final String BEAN_NAME = "conferenceBridge";
    public static final String PROMPTS_DEFAULT = "stdprompts";
    public static final String CONFERENCE = "conf";
    public static final String CONFERENCES_PROP = "conferences";
    public static final String SERVICE_URI_PROP = "serviceUri";

    public static final String CALL_CONTROL_MUTE = "fs-conf-bridge/dtmf-commands/mute";
    public static final String CALL_CONTROL_DEAF_MUTE = "fs-conf-bridge/dtmf-commands/deaf-mute";
    public static final String CALL_CONTROL_ENERGY_UP = "fs-conf-bridge/dtmf-commands/energy/up";
    public static final String CALL_CONTROL_ENERGY_RESET = "fs-conf-bridge/dtmf-commands/energy/reset";
    public static final String CALL_CONTROL_ENERGY_DOWN = "fs-conf-bridge/dtmf-commands/energy/down";
    public static final String CALL_CONTROL_VOLUME_UP = "fs-conf-bridge/dtmf-commands/audio-volume/up";
    public static final String CALL_CONTROL_VOLUME_RESET = "fs-conf-bridge/dtmf-commands/audio-volume/reset";
    public static final String CALL_CONTROL_VOLUME_DOWN = "fs-conf-bridge/dtmf-commands/audio-volume/down";
    public static final String CALL_CONTROL_TALK_UP = "fs-conf-bridge/dtmf-commands/audio-volume/mic-up";
    public static final String CALL_CONTROL_TALK_RESET = "fs-conf-bridge/dtmf-commands/audio-volume/mic-reset";
    public static final String CALL_CONTROL_TALK_DOWN = "fs-conf-bridge/dtmf-commands/audio-volume/mic-down";
    public static final String CALL_CONTROL_HANGUP = "fs-conf-bridge/dtmf-commands/hung-up/hungup";
    public static final String SLASH = System.getProperty("file.separator");

    private Set<Conference> m_conferences = new LinkedHashSet<Conference>();
    private String m_audioDirectory;
    private LocalizationContext m_localizationContext;
    private String m_promptsDir;

    /** The associated FreeSWITCH service. */
    private LocationSpecificService m_service;

    public void setLocalizationContext(LocalizationContext localizationContext) {
        m_localizationContext = localizationContext;
    }

    public void setPromptsDir(String promptsDir) {
        m_promptsDir = promptsDir;
    }

    public LocationSpecificService getService() {
        return m_service;
    }

    public void setService(LocationSpecificService service) {
        m_service = service;
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxconference/bridge.xml");
    }

    public void addConference(Conference conference) {
        getConferences().add(conference);
        conference.setBridge(this);
    }

    public void addConferences(Collection conferences) {
        for (Iterator iter = conferences.iterator(); iter.hasNext();) {
            Conference conf = (Conference) iter.next();
            addConference(conf);
        }
    }

    public void removeConference(Conference conference) {
        conference.setBridge(null);
        getConferences().remove(conference);
    }

    // trivial get/set
    public String getDescription() {
        return getLocation().getName();
    }

    public String getName() {
        return getService().getLocation().getFqdn();
    }

    public Set<Conference> getConferences() {
        return m_conferences;
    }

    public void setConferences(Set<Conference> conferences) {
        m_conferences = conferences;
    }

    public void setAudioDirectory(String audioDirectory) {
        m_audioDirectory = audioDirectory;
    }

    public String getAudioDirectory() {
        String path = m_promptsDir + SLASH + m_localizationContext.getCurrentLanguageDir();
        String tmpPath = path + SLASH + CONFERENCE;
        File testFile = new File(tmpPath);
        if (testFile.exists()) {
            m_audioDirectory = path;
        } else {
            m_audioDirectory = m_promptsDir + SLASH + PROMPTS_DEFAULT;
        }
        return m_audioDirectory;
    }

    @Override
    public void setSettings(Setting settings) {
        settings.acceptVisitor(new AudioDirectorySetter());
        super.setSettings(settings);
    }

    private class AudioDirectorySetter extends AbstractSettingVisitor {
        @Override
        public void visitSetting(Setting setting) {
            SettingType type = setting.getType();
            if (type instanceof FileSetting) {
                FileSetting fileType = (FileSetting) type;
                fileType.setDirectory(m_audioDirectory);
            }
        }
    }

    public String getServiceUri() {
        return getFreeswitchService().getServiceUri(getLocation());
    }

    public SipxFreeswitchService getFreeswitchService() {
        SipxFreeswitchService freeswitchService = (SipxFreeswitchService) m_service.getSipxService();
        return freeswitchService;
    }

    public Location getLocation() {
        return m_service.getLocation();
    }

    public String getHost() {
        return getLocation().getFqdn();
    }
}
