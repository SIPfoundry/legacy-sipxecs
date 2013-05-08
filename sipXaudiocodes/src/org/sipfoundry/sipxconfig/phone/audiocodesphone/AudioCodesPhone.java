/*
 * Initial Version Copyright (C) 2011 AudioCodes, All Rights Reserved.
 * Licensed to the User under the LGPL license.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.phone.audiocodesphone;


import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.device.Device;
import org.sipfoundry.sipxconfig.device.Profile;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.device.ProfileFilter;
import org.sipfoundry.sipxconfig.device.ProfileLocation;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;

public class AudioCodesPhone extends Phone {

    public static final String USER_ID_SETTING = "voip/id";
    public static final String AUTH_ID_SETTING = "voip/auth_name";
    public static final String PASSWORD_SETTING = "voip/auth_password";
    public static final String DISPLAY_NAME = "voip/description";
    public static final String OUTBOUND_PROXY_ADDRESS = "voip/signalling/sip/sip_outbound_proxy/addr";
    public static final String MWI_SUBSCRIBE_ADDRESS = "voip/services/msg_waiting_ind/subscribe_address";
    public static final String VOICEMAIL_ACCESS_NUMBER = "voip/services/msg_waiting_ind/voice_mail_number";
    public static final String SYSLOG_SERVER = "system/syslog/server_address";
    public static final String TIME_SERVER_NAME = "systes/ntp/primary_server_address";
    public static final String ALTERNATE_TIME_SERVER_NAME = "system/ntp/secondary_server_address";
    public static final String MIME_TYPE_PLAIN = "text/plain";
    public static final String FILE_SEPERATOR = "/";
    public static final String SPEED_DIAL_FILE_NAME = "-speeddial.txt";

    private LocationsManager m_locationsManager;

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    @Override
    public void initialize() {
        AudioCodesPhoneDefaults defaults = new AudioCodesPhoneDefaults(getPhoneContext().getPhoneDefaults());
        addDefaultBeanSettingHandler(defaults);
    }

    @Override
    public void initializeLine(Line line) {
        line.addDefaultBeanSettingHandler(new AudioCodesPhoneLineDefaults(line));
    }

    @Override
    protected void setLineInfo(Line line, LineInfo info) {
        line.setSettingValue(DISPLAY_NAME, info.getDisplayName());
        line.setSettingValue(USER_ID_SETTING, info.getUserId());
        line.setSettingValue(PASSWORD_SETTING, info.getPassword());
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        LineInfo info = new LineInfo();
        info.setDisplayName(line.getSettingValue(DISPLAY_NAME));
        info.setUserId(line.getSettingValue(USER_ID_SETTING));
        info.setPassword(line.getSettingValue(PASSWORD_SETTING));
        return info;
    }

    @Override
    public Profile[] getProfileTypes() {
        Profile[] profileTypes;
        PhonebookManager phonebookManager = getPhonebookManager();
        if (phonebookManager.getPhonebookManagementEnabled()) {
            profileTypes = new Profile[] {
                new Profile(this), new SpeedDialProfile(getSpeedDialFileName())
            };
        } else {
            profileTypes = new Profile[] {
                new Profile(this)
            };
        }

        return profileTypes;
    }

    @Override
    public void removeProfiles(ProfileLocation location) {
        super.removeProfiles(location);
        location.removeProfile(getSpeedDialFileName());
    }

    @Override
    public String getProfileFilename() {
        return  getSerialNumber() + ".cfg";
    }

    static class SpeedDialProfile extends Profile {
        public SpeedDialProfile(String name) {
            super(name, MIME_TYPE_PLAIN);
        }

        @Override
        protected ProfileFilter createFilter(Device device) {
            return null;
        }

        @Override
        protected ProfileContext createContext(Device device) {
            Phone phone = (Phone) device;
            PhoneContext phoneContext = phone.getPhoneContext();
            SpeedDial speedDial = phoneContext.getSpeedDial(phone);
            return new AudioCodesPhoneSpeedDial(speedDial);
        }
    }

    public String getSpeedDialFileName() {
        return getSerialNumber() + SPEED_DIAL_FILE_NAME;
    }

    public void restart() {
        sendCheckSyncToMac();
    }

    public String[] getSelectedCodecs() {
        String codecs = this.getSettings().getSetting("codec/codec_info").getValue();
        return codecs.split("\\|");
    }

    public String getTftpServer() {
        return m_locationsManager.getPrimaryLocation().getAddress();
    }


    public String getModelName() {
        return this.getModelId().replaceFirst("audiocodes-", "") + "HD";
    }

    public String getFirmwareFileName() {
        return getModelName() + ".img";
    }
}
