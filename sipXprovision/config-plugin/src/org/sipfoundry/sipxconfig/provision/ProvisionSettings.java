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
package org.sipfoundry.sipxconfig.provision;

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class ProvisionSettings extends PersistableSettings implements DeployConfigOnEdit {
    private static final String ADMIN_URL = "provision-config/provision.configUrl";
    private CoreContext m_coreContext;
    private AddressManager m_addressManager;
    private User m_user;
    private Address m_adminAddress;

    public ProvisionSettings() {
        addDefaultBeanSettingHandler(new Defaults());
    }

    public class Defaults {
        @SettingEntry(path = "provision-config/provision.username")
        public String getUserName() {
            return getProvisionUser().getUserName();
        }
        @SettingEntry(path = "provision-config/provision.password")
        public String getPassword() {
            return getProvisionUser().getSipPassword();
        }
        @SettingEntry(path = ADMIN_URL)
        public String getConfigUrl() {
            return getAdminAddress().toString();
        }
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxprovision/sipxprovision.xml");
    }

    public int getPort() {
        return (Integer) getSettingTypedValue("provision-config/provision.servlet.port");
    }

    public int getSecurePort() {
        return (Integer) getSettingTypedValue("provision-config/provision.servlet.securePort");
    }

    private Address getAdminAddress() {
        if (m_adminAddress == null) {
            m_adminAddress = m_addressManager.getSingleAddress(AdminContext.HTTP_ADDRESS);
        }
        return m_adminAddress;
    }

    private User getProvisionUser() {
        if (m_user == null) {
            m_user = m_coreContext.getSpecialUser(SpecialUserType.PHONE_PROVISION);
        }
        return m_user;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }

    @Override
    public String getBeanId() {
        return "provisionSettings";
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) Provision.FEATURE);
    }
}
