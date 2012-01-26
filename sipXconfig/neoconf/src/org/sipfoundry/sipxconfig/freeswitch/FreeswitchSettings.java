/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.freeswitch;

import java.util.ArrayList;
import java.util.List;

import org.sipfoundry.sipxconfig.commserver.SettingsWithLocation;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class FreeswitchSettings extends SettingsWithLocation {
    private static final String FREESWITCH_XMLRPC_PORT = "freeswitch-config/FREESWITCH_XMLRPC_PORT";
    private static final String FREESWITCH_SIP_PORT = "freeswitch-config/FREESWITCH_SIP_PORT";
    private static final String FREESWITCH_MOH_SOURCE = "freeswitch-config/MOH_SOURCE";
    private static final String FREESWITCH_CODECS = "freeswitch-config/FREESWITCH_CODECS";

    public static enum SystemMohSetting {
        FILES_SRC, SOUNDCARD_SRC, NONE;
        public static SystemMohSetting parseSetting(String mohSetting) {
            try {
                return valueOf(mohSetting);
            } catch (IllegalArgumentException e) {
                return FILES_SRC;
            }
        }
    }

    public int getEventSocketPort() {
        return 8084; // not configurable at this time, no particular reason. --Douglas
    }

    public int getXmlRpcPort() {
        return (Integer) getSettingTypedValue(FREESWITCH_XMLRPC_PORT);
    }

    public int getFreeswitchSipPort() {
        return (Integer) getSettingTypedValue(FREESWITCH_SIP_PORT);
    }

    public String getMusicOnHoldSource() {
        return (String) getSettingValue(FREESWITCH_MOH_SOURCE);
    }

    public class Defaults {
        @SettingEntry(path = FREESWITCH_CODECS)
        public List<String> getFreeswitchCodecs() {
            ArrayList<String> returnList = new ArrayList<String>();
            returnList.add("G722");
            returnList.add("PCMU@20i");
            returnList.add("PCMA@20i");
            returnList.add("speex");
            returnList.add("L16");
            return returnList;
        }
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("freeswitch/freeswitch.xml");
    }

    @Override
    public String getBeanId() {
        return "freeswithSettings";
    }
}
