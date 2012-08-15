/**
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
package org.sipfoundry.sipxconfig.ftp;


import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.sbc.DefaultSbc;
import org.sipfoundry.sipxconfig.sbc.SbcManager;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class FtpSettings extends PersistableSettings implements DeployConfigOnEdit {
    private SbcManager m_sbcManager;

    public FtpSettings() {
        addDefaultBeanSettingHandler(new Defaults());
    }

    public class Defaults {
        @SettingEntry(path = "vsftp-config/pasv_address")
        public String getPassiveAddress() {
            DefaultSbc sbc = m_sbcManager.getDefaultSbc();
            return (sbc != null ? sbc.getAddress() : "");
        }
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) FtpManager.FTP_FEATURE);
    }

    public int getMinPasvPort() {
        return (Integer) getSettingTypedValue("vsftp-config/pasv_min_port");
    }

    public int getMaxPasvPort() {
        return (Integer) getSettingTypedValue("vsftp-config/pasv_max_port");
    }

    @Override
    public String getBeanId() {
        return "ftpSettings";
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("ftp/ftp.xml");
    }

    public void setSbcManager(SbcManager sbcManager) {
        m_sbcManager = sbcManager;
    }
}
