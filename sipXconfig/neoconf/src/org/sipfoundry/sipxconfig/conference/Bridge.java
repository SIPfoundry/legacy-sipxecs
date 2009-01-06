/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.conference;

import java.util.Collection;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.service.LocationSpecificService;
import org.sipfoundry.sipxconfig.setting.AbstractSettingVisitor;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.type.FileSetting;
import org.sipfoundry.sipxconfig.setting.type.SettingType;

public class Bridge extends BeanWithSettings {
    public static final String BEAN_NAME = "conferenceBridge";
    
    public static final String CONFERENCES_PROP = "conferences";
    public static final String SERVICE_URI_PROP = "serviceUri";    
    
    public static final String FREESWITCH_SIP_PORT = "freeswitch-config/FREESWITCH_SIP_PORT";
    public static final String FREESWITCH_XMLRPC_PORT = "freeswitch-config/FREESWITCH_XMLRPC_PORT";
    
    public static final String CALL_CONTROL_MUTE = "fs-conf-bridge/dtmf-commands/mute";
    public static final String CALL_CONTROL_DEAF_MUTE = "fs-conf-bridge/dtmf-commands/deaf-mute";
    public static final String CALL_CONTROL_ENERGY_UP = "fs-conf-bridge/dtmf-commands/energy/up";
    public static final String CALL_CONTROL_ENERGY_RESET = "fs-conf-bridge/dtmf-commands/energy/reset";
    public static final String CALL_CONTROL_ENERGY_DOWN = "fs-conf-bridge/dtmf-commands/energy/down";
    public static final String CALL_CONTROL_VOLUME_UP = "fs-conf-bridge/dtmf-commands/volume-listen/up";
    public static final String CALL_CONTROL_VOLUME_RESET = "fs-conf-bridge/dtmf-commands/volume-listen/reset";
    public static final String CALL_CONTROL_VOLUME_DOWN = "fs-conf-bridge/dtmf-commands/volume-listen/down";
    public static final String CALL_CONTROL_TALK_UP = "fs-conf-bridge/dtmf-commands/volume-talk/up";
    public static final String CALL_CONTROL_TALK_RESET = "fs-conf-bridge/dtmf-commands/volume-talk/reset";
    public static final String CALL_CONTROL_TALK_DOWN = "fs-conf-bridge/dtmf-commands/volume-talk/down";
    public static final String CALL_CONTROL_HANGUP = "fs-conf-bridge/dtmf-commands/hungup";
       
    private boolean m_enabled;

    private Set m_conferences = new HashSet();

    private DeviceDefaults m_systemDefaults;

    private String m_audioDirectory;
    
    /** The associated FreeSWITCH service. */
    private LocationSpecificService m_service;

    public String getHost() {
        return getService().getLocation().getFqdn();
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
        return getService().getLocation().getName();
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public String getName() {
        return getService().getLocation().getFqdn();
    }

    public Set getConferences() {
        return m_conferences;
    }

    public void setConferences(Set conferences) {
        m_conferences = conferences;
    }

    public void setSystemDefaults(DeviceDefaults systemDefaults) {
        m_systemDefaults = systemDefaults;
    }

    public void setAudioDirectory(String audioDirectory) {
        m_audioDirectory = audioDirectory;
    }
    
    public String getAudioDirectory() {
        return m_audioDirectory;
    }

    public int getSipPort() {
        return (Integer) getService().getSipxService().getSettingTypedValue(FREESWITCH_SIP_PORT);
    }
    
    public int getPort() {
        return (Integer) getService().getSipxService().getSettingTypedValue(FREESWITCH_XMLRPC_PORT);
    }
    
    @Override
    public void setSettings(Setting settings) {
        settings.acceptVisitor(new AudioDirectorySetter());
        super.setSettings(settings);
    }

    private class AudioDirectorySetter extends AbstractSettingVisitor {
        public void visitSetting(Setting setting) {
            SettingType type = setting.getType();
            if (type instanceof FileSetting) {
                FileSetting fileType = (FileSetting) type;
                fileType.setDirectory(m_audioDirectory);
            }
        }
    }

    public String getServiceUri() {
        return String.format("http://%s:%d/RPC2", getHost(), getPort());
    }    
}
