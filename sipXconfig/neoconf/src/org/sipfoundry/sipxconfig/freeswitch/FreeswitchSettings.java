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

import org.apache.log4j.Logger;
import org.sipfoundry.sipxconfig.admin.commserver.BeanWithLocation;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class FreeswitchSettings extends BeanWithLocation {
    public static final String FREESWITCH_XMLRPC_PORT = "freeswitch-config/FREESWITCH_XMLRPC_PORT";
    public static final String FREESWITCH_SIP_PORT = "freeswitch-config/FREESWITCH_SIP_PORT";
    public static final String FREESWITCH_MOH_SOURCE = "freeswitch-config/MOH_SOURCE";
    public static final String RELOAD_XML_JOB_TITLE = "FreeSWITCH reload configuration";
    public static final String FREESWITCH_CODECS = "freeswitch-config/FREESWITCH_CODECS";
    public static final String BEAN_ID = "sipxFreeswitchService";
    public static final String LOG_SETTING = "freeswitch-config/FREESWITCH_SIP_DEBUG";
    public static final String G729 = "G729";
    public static final String G729_STATUS = "Permitted G.729AB channels";
    public static final Logger LOG = Logger.getLogger("SipxFreeswitchService.class");
    private FreeswitchFeature m_freeswitchFeature;

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
            if (m_freeswitchFeature.isCodecG729Installed()) {
                returnList.add(G729);
            }
            return returnList;
        }
    }

    public void setFreeswitchFeature(FreeswitchFeature freeswitchFeature) {
        m_freeswitchFeature = freeswitchFeature;
    }
}
