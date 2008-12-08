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

import java.util.List;

import org.apache.commons.collections.Closure;
import org.apache.commons.collections.CollectionUtils;
import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.io.OutputFormat;
import org.sipfoundry.sipxconfig.admin.dialplan.config.XmlFile;

public class ConferenceConfiguration extends XmlFile {
    private static final String NAME = "name";
    private static final String DESCRIPTION = "description";
    private static final String CALLER_CONTROLS = "caller-controls";
    private static final String SIPX_CALLER_CONTROLS = "sipx-default";
    
    private static final String DEFAULT_AUDIO_LOCKED = "conf/conf-locked.wav";
    private static final String DEFAULT_AUDIO_BEEP = "beep.wav";
    private static final String DEFAULT_AUDIO_MUTED = "conf/conf-muted.wav";
    private static final String DEFAULT_AUDIO_UNMUTED = "conf/conf-unmuted.wav";
    private static final String DEFAULT_AUDIO_ALONE = "conf/conf-alone.wav";
    private static final String DEFAULT_AUDIO_KICKED = "conf/conf-kicked.wav";
    private static final String DEFAULT_AUDIO_ISLOCKED = "conf/conf-islocked.wav";
    private static final String DEFAULT_AUDIO_ENTER_PIN = "conf/conf-pin.wav";
    private static final String DEFAULT_AUDIO_BAD_PIN = "conf/conf-bad-pin.wav";
          
    private Bridge m_bridge;

    private Document m_document;

    public Document getDocument() {
        return m_document;
    }
        
    public void generate(Bridge bridge, List conferences) {
        m_bridge = bridge;
        m_document = FACTORY.createDocument();
        Element configuration = m_document.addElement("configuration");
        configuration.addAttribute(NAME, "conference.conf");
        configuration.addAttribute(DESCRIPTION, "sipX MeetMe Audio Conference");        
        addCallerControls(configuration);
        Element profile = configuration.addElement("profiles");
        
        // Add default profile
        BuildConferenceProfile builder = new BuildConferenceProfile(profile, m_bridge);
        builder.execute(null);

        // Add each conference profile
        CollectionUtils.forAllDo(conferences, new BuildConferenceProfile(profile, m_bridge));
    }
    
    private void addCallerControls(Element parent) {
        Element context = parent.addElement(CALLER_CONTROLS);
        context = context.addElement("group");
        context.addAttribute(NAME, SIPX_CALLER_CONTROLS);
        
        String[][] settings = 
        {
            {"mute", Bridge.CALL_CONTROL_MUTE},
            {"deaf mute", Bridge.CALL_CONTROL_DEAF_MUTE},
            {"energy up", Bridge.CALL_CONTROL_ENERGY_UP},
            {"enery equ", Bridge.CALL_CONTROL_ENERGY_RESET},
            {"energy dn", Bridge.CALL_CONTROL_ENERGY_DOWN},
            {"vol talk up", Bridge.CALL_CONTROL_TALK_UP},
            {"vol talk zero", Bridge.CALL_CONTROL_TALK_RESET},
            {"vol talk dn", Bridge.CALL_CONTROL_TALK_DOWN},
            {"vol listen up", Bridge.CALL_CONTROL_VOLUME_UP},
            {"vol listen zero", Bridge.CALL_CONTROL_VOLUME_RESET},
            {"vol listen dn", Bridge.CALL_CONTROL_VOLUME_DOWN},
            {"hangup", Bridge.CALL_CONTROL_HANGUP},            
        };
        
        for (int i = 0; i < settings.length; i++) {
            String name = settings[i][0];
            String param = settings[i][1];
            String setting = (String) m_bridge.getSettingTypedValue(param);
            
            if (setting != null && setting.length() > 0) {
                context.addElement("control")
                        .addAttribute("action", name)
                        .addAttribute("digits", setting); 
                
            }        
        }
    }       

    private static class BuildConferenceProfile implements Closure {
        private final Bridge m_bridge;
        private final Element m_parent;

        public BuildConferenceProfile(Element parent, Bridge bridge) {
            m_parent = parent;
            m_bridge = bridge;
        }

        public void execute(Object input) {
            Conference conference = (Conference) input;
            
            Element profile = m_parent.addElement("profile");
            
            if (conference != null) {
                profile.addAttribute(NAME, conference.getExtension());
            } else {
                profile.addAttribute(NAME, "default");
            }
            
            addParam(profile, "domain", m_bridge.getSipDomain());
            addParam(profile, CALLER_CONTROLS, SIPX_CALLER_CONTROLS);
            addParam(profile, "rate", "8000");
            addParam(profile, "interval", "20");
            addParam(profile, "energy-level", "300");
            addParam(profile, "sound-prefix", m_bridge.getAudioDirectory());
            addParam(profile, "ack-sound", DEFAULT_AUDIO_BEEP);
            addParam(profile, "nack-sound", DEFAULT_AUDIO_BEEP);
            addParam(profile, "muted-sound", DEFAULT_AUDIO_MUTED);
            addParam(profile, "unmuted-sound", DEFAULT_AUDIO_UNMUTED);
            addParam(profile, "alone-sound", DEFAULT_AUDIO_ALONE);
            addParam(profile, "moh-sound", "$${hold_music}");
            addParam(profile, "enter-sound", DEFAULT_AUDIO_BEEP);
            addParam(profile, "exit-sound", DEFAULT_AUDIO_BEEP);
            addParam(profile, "kicked-sound", DEFAULT_AUDIO_KICKED);
            addParam(profile, "locked-sound", DEFAULT_AUDIO_LOCKED);
            addParam(profile, "max-members-sound", DEFAULT_AUDIO_LOCKED);
            addParam(profile, "is-locked-sound", DEFAULT_AUDIO_ISLOCKED);
            addParam(profile, "is-unlocked-sound", DEFAULT_AUDIO_BEEP);
            addParam(profile, "pin-sound", DEFAULT_AUDIO_ENTER_PIN);
            addParam(profile, "bad-pin-sound", DEFAULT_AUDIO_BAD_PIN);
            addParam(profile, "confort-noise-level", "1400");
            addParam(profile, "caller-id-name", "$${outbound_caller_name}");
            addParam(profile, "caller-id-number", "$${outbound_caller_id}");
            addParam(profile, "confort-noise", "true");
            if (conference != null) {
                Integer val = (Integer) conference.getSettingTypedValue(Conference.MAX_LEGS);
                if (val != null) {
                    addParam(profile, "max-members", val.toString());
                }
            }
        }
        
        protected void addParam(Element parent, String name, String value) {
            parent.addElement("param")
                .addAttribute(NAME, name)
                .addAttribute("value", value);        
        }
    }
    

    @Override
    public OutputFormat createFormat() {
        OutputFormat format = OutputFormat.createPrettyPrint();
        format.setOmitEncoding(true);
        format.setSuppressDeclaration(true);
        return format;
    }
}
