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
package org.sipfoundry.sipxconfig.snmp;


import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class SnmpSettings extends PersistableSettings implements DeployConfigOnEdit {
    private static final String FIX_DEAD_PROCESSES = "cfdat/fix_dead_processes";
    private boolean m_restartDefault;

    public SnmpSettings() {
        addDefaultBeanSettingHandler(new Defaults());
    }

    public class Defaults {
        @SettingEntry(path = FIX_DEAD_PROCESSES)
        public boolean getRestart() {
            return m_restartDefault;
        }
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) SnmpManager.FEATURE);
    }

    @Override
    public String getBeanId() {
        return "snmpSettings";
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("snmp/snmp.xml");
    }

    /**
     * NOTE: double negative because not sure how to do "not ${sys.src}" in spring def file
     */
    public void setNoRestartDefault(boolean noRestartDefault) {
        m_restartDefault = !noRestartDefault;
    }

    public boolean isFixDeadProcesses() {
        return (Boolean) getSettingTypedValue(FIX_DEAD_PROCESSES);
    }
}
