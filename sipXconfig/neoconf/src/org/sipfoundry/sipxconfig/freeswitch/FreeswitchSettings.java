/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.freeswitch;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.log4j.Logger;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.commserver.BeanWithLocation;
import org.sipfoundry.sipxconfig.conference.FreeswitchApi;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcRemoteException;

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
    private static Boolean s_g729;
    private AddressManager m_addressManager;
    private ApiProvider<FreeswitchApi> m_freeswitchApiProvider;

    public boolean isCodecG729Installed() {
        if (s_g729 == null) {
            // cache the result as this is a high traffic call and an expensive call to make
            // ideally there would be a better place to put this code.
            Collection<Address> addresses = m_addressManager.getAddresses(FreeswitchFeature.XMLRPC_ADDRESS);
            for (Address address : addresses) {
                FreeswitchApi api = m_freeswitchApiProvider.getApi(address.toString());
                try {
                    String result = api.g729_status();
                    if (StringUtils.contains(result, G729_STATUS)) {
                        s_g729 = true;
                    } else {
                        // try also new FS detection algorithm
                        result = api.g729_available();
                        if (StringUtils.contains(result, "true")) {
                            s_g729 = true;
                        }
                    }
                } catch (XmlRpcRemoteException xrre) {
                    LOG.error(xrre);
                    s_g729 = false;
                }
            }
        }
        return s_g729;
    }

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
            if (isCodecG729Installed()) {
                returnList.add(G729);
            }
            return returnList;
        }
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("freeswitch/freeswitch.xml");
    }

    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }

    public void setFreeswitchApiProvider(ApiProvider<FreeswitchApi> freeswitchApiProvider) {
        m_freeswitchApiProvider = freeswitchApiProvider;
    }
}
