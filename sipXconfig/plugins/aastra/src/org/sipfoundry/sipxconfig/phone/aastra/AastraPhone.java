/*
 * 
 * 
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.phone.aastra;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Map;

import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.speeddial.Button;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;

public class AastraPhone extends Phone {
    static final String REGISTRATION_PATH = "server/registrar_ip";
    static final String DISPLAY_NAME_PATH = "sip_id/screen_name";
    static final String PASSWORD_PATH = "sip_id/password";
    static final String USER_ID_PATH = "sip_id/user_name";
    static final String AUTHORIZATION_ID_PATH = "sip_id/auth_name";

    private static final String REGISTRATION_PORT_PATH = "server/registrar_port";

    // private static final String EXTENSION_PATH = "sip/extension";

    public AastraPhone() {
    }

    @Override
    public void initialize() {
        AastraPhoneDefaults phoneDefaults = new AastraPhoneDefaults(getPhoneContext().getPhoneDefaults());
        addDefaultBeanSettingHandler(phoneDefaults);
        AastraIntercomDefaults intercomDefaults = new AastraIntercomDefaults(this);
        addDefaultBeanSettingHandler(intercomDefaults);

    }

    @Override
    public void initializeLine(Line line) {
        AastraLineDefaults lineDefaults = new AastraLineDefaults(getPhoneContext().getPhoneDefaults(), line);
        line.addDefaultBeanSettingHandler(lineDefaults);
    }

    @Override
    protected ProfileContext createContext() {
        SpeedDial speedDial = getPhoneContext().getSpeedDial(this);
        return new AastraProfileContext(this, speedDial, getModel().getProfileTemplate());
    }

    static class AastraProfileContext extends ProfileContext {
        private SpeedDial m_speeddial;

        AastraProfileContext(AastraPhone phone, SpeedDial speeddial, String profileTemplate) {
            super(phone, profileTemplate);
            m_speeddial = speeddial;
        }

        public Map<String, Object> getContext() {
            Map<String, Object> context = super.getContext();
            Phone phone = (Phone) getDevice();

            if (m_speeddial != null) {
                boolean hasBlf = false;
                Collection<Button> speeddials = new ArrayList<Button>();
                Collection<Button> buttons = m_speeddial.getButtons();
                for (Button button : buttons) {
                    if (button.isBlf()) {
                        hasBlf = true;
                        Line line = new Line();
                        line.setPhone(phone);
                        phone.initializeLine(line);
                        line.setSettingValue(DISPLAY_NAME_PATH, button.getLabel());
                        // line.setSettingValue(EXTENSION_PATH, button.getNumber());
                        phone.addLine(line);
                    } else {
                        speeddials.add(button);
                    }
                }
                context.put("has_blf", hasBlf);
                context.put("speeddials", speeddials);
                context.put("speeddial", m_speeddial);
            }

            int speeddialOffset = 0;
            Collection lines = phone.getLines();
            if (lines != null) {
                speeddialOffset = lines.size();
            }
            context.put("speeddial_offset", speeddialOffset);

            return context;
        }
    }

    @Override
    protected void setLineInfo(Line line, LineInfo externalLine) {
        line.setSettingValue(DISPLAY_NAME_PATH, externalLine.getDisplayName());
        line.setSettingValue(USER_ID_PATH, externalLine.getUserId());
        line.setSettingValue(PASSWORD_PATH, externalLine.getPassword());
        line.setSettingValue(AUTHORIZATION_ID_PATH, externalLine.getUserId());

        line.setSettingValue(REGISTRATION_PATH, externalLine.getRegistrationServer());
        line.setSettingValue(REGISTRATION_PORT_PATH, externalLine.getRegistrationServerPort());
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        LineInfo lineInfo = new LineInfo();
        lineInfo.setUserId(line.getSettingValue(USER_ID_PATH));
        lineInfo.setDisplayName(line.getSettingValue(DISPLAY_NAME_PATH));
        lineInfo.setPassword(line.getSettingValue(PASSWORD_PATH));
        lineInfo.setRegistrationServer(line.getSettingValue(REGISTRATION_PATH));
        lineInfo.setRegistrationServerPort(line.getSettingValue(REGISTRATION_PORT_PATH));
        return lineInfo;
    }

    @Override
    public void restart() {
        sendCheckSyncToFirstLine();
    }

    @Override
    public String getProfileFilename() {
        return getSerialNumber().toUpperCase() + ".cfg";
    }

}
