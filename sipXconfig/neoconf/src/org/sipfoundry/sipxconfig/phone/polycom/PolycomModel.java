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

import java.util.HashSet;
import java.util.Set;

import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.phone.PhoneModel;

/**
 * Static differences in polycom phone models
 */
public final class PolycomModel extends PhoneModel {
    
    /** Firmware 2.0 or beyond */
    public static final DeviceVersion VER_2_0 = new DeviceVersion(PolycomPhone.BEAN_ID, "2.0");

    private static final String FEATURE_VOICE_QUALITY_MONITORING = "voiceQualityMonitoring";
    private static final String FEATURE_SINGLE_KEY_PRESS_CONFERENCE = "singleKeyPressConference";
    private static final String FEATURE_LOCAL_CONFERENCE_CALL_HOLD = "localConferenceCallHold";
    private static final String FEATURE_NWAY_CONFERENCE = "nway-conference";
    private static final String FEATURE_DISABLE_CALL_LIST = "disableCallList";
    
    private Set<String> m_supportedFeatures = new HashSet<String>();
    
    public PolycomModel() {
        super(PolycomPhone.BEAN_ID);
        setEmergencyConfigurable(true);
    }

    public void setQualityMonitoringSupported(boolean supported) {
        setFeatureSupported(FEATURE_VOICE_QUALITY_MONITORING, supported);
    }

    public boolean isQualityMonitoringSupported() {
        return isFeatureSupported(FEATURE_VOICE_QUALITY_MONITORING);
    }
    
    public void setSingleKeyPressConferenceSupported(boolean supported) {
        setFeatureSupported(FEATURE_SINGLE_KEY_PRESS_CONFERENCE, supported);
    }

    public boolean isSingleKeyPressConferenceSupported() {
        return isFeatureSupported(FEATURE_SINGLE_KEY_PRESS_CONFERENCE);
    }    
    
    public void setLocalConferenceCallHoldSupported(boolean supported) {
        setFeatureSupported(FEATURE_LOCAL_CONFERENCE_CALL_HOLD, supported);
    }

    public boolean isLocalConferenceCallHoldSupported() {
        return isFeatureSupported(FEATURE_LOCAL_CONFERENCE_CALL_HOLD);
    }
    
    public void setNwayConferenceSupported(boolean supported) {
        setFeatureSupported(FEATURE_NWAY_CONFERENCE, supported);
    }
    
    public boolean isNwayConferenceSupported() {
        return isFeatureSupported(FEATURE_NWAY_CONFERENCE);
    }
    
    public void setDisableCallListSupported(boolean supported) {
        setFeatureSupported(FEATURE_DISABLE_CALL_LIST, supported);
    }    
    
    public boolean isDisableCallListSupported() {
        return isFeatureSupported(FEATURE_DISABLE_CALL_LIST);
    }
    
    private void setFeatureSupported(String feature, boolean supported) {
        if (supported) {
            m_supportedFeatures.add(feature);
        } else {
            m_supportedFeatures.remove(feature);
        }
    }    

    private boolean isFeatureSupported(String feature) {
        return m_supportedFeatures.contains(feature);
    }
    
    public Set<String> getDefinitions() {
        Set<String> definitions = super.getDefinitions();
        definitions.addAll(m_supportedFeatures);
        return definitions;
    }
}
