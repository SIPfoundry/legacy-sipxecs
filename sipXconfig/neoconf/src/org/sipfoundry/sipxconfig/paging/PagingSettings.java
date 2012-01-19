/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
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
