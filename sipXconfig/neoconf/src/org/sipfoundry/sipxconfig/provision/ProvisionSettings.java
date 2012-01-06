/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.provision;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class ProvisionSettings extends PersistableSettings {
    private static final String ADMIN_URL = "provision-config/provision.configUrl";
    private CoreContext m_coreContext;
    private AddressManager m_addressManager;
    private User m_user;
    private Address m_adminAddress;

    public ProvisionSettings() {
        addDefaultBeanSettingHandler(new Defaults());
    }

    public class Defaults {
        @SettingEntry(path = "provision-config/provision.sipxchangeDomainName")
        public String getDomainName() {
            return m_coreContext.getDomainName();
        }
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

    private Address getAdminAddress() {
        if (m_adminAddress == null) {
            m_adminAddress = m_addressManager.getSingleAddress(AdminContext.HTTPS_ADDRESS);
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
}
