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
package org.sipfoundry.sipxconfig.freeswitch;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.commserver.SettingsWithLocation;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class FreeswitchSettings extends SettingsWithLocation implements DeployConfigOnEdit {
    public static final int RTP_START_PORT = 11000;
    public static final int RTP_END_PORT = 12999;
    private static final String FREESWITCH_XMLRPC_PORT = "freeswitch-config/FREESWITCH_XMLRPC_PORT";
    private static final String FREESWITCH_SIP_PORT = "freeswitch-config/FREESWITCH_SIP_PORT";
    private static final String FREESWITCH_CODECS = "freeswitch-config/FREESWITCH_CODECS";
    private static final String FREESWITCH_BLIND_TRANSFER = "freeswitch-config/FREESWITCH_BLIND_TRANSFER";

    public int getEventSocketPort() {
        return 8084; // not configurable at this time, no particular reason. --Douglas
    }

    public int getAccEventSocketPort() {
        return 8184; // not configurable at this time, no particular reason. --Douglas
    }

    public int getXmlRpcPort() {
        return (Integer) getSettingTypedValue(FREESWITCH_XMLRPC_PORT);
    }

    public int getFreeswitchSipPort() {
        return (Integer) getSettingTypedValue(FREESWITCH_SIP_PORT);
    }

    public boolean isBlindTransferEnabled() {
        return (Boolean) getSettingTypedValue(FREESWITCH_BLIND_TRANSFER);
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

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) FreeswitchFeature.FEATURE);
    }
}
