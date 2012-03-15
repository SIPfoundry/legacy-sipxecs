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
package org.sipfoundry.sipxconfig.paging;


import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class PagingSettings extends PersistableSettings implements DeployConfigOnEdit {
    private static final String PREFIX = "page-dial/prefix";

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxpage/sipxpage.xml");
    }

    public String getPrefix() {
        return getSettingValue(PREFIX);
    }

    public void setPrefix(String prefix) {
        setSettingValue(PREFIX, prefix);
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) PagingContext.FEATURE);
    }

    public int getSipTcpPort() {
        return (Integer) getSettingTypedValue("page-config/sip.tcpPort");
    }

    public int getSipUdpPort() {
        return (Integer) getSettingTypedValue("page-config/sip.udpPort");
    }

    public int getSipTlsPort() {
        return (Integer) getSettingTypedValue("page-config/sip.tlsPort");
    }

    public int getRtpPort() {
        return (Integer) getSettingTypedValue("page-config/rtp.port");
    }

    @Override
    public String getBeanId() {
        return "pagingSettings";
    }
}
