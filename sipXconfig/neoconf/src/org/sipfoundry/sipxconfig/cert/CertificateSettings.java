/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cert;

import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class CertificateSettings extends PersistableSettings {

    public CertificateSettings() {
        addDefaultBeanSettingHandler(new Defaults());
    }

    public class Defaults {
        @SettingEntry(path = "csr/organization")
        public String getOrganization() {
            return Domain.getDomain().getName();
        }

        @SettingEntry(path = "csr/email")
        public String getEmail() {
            return "root@" + getOrganization();
        }
    }

    @Override
    public String getBeanId() {
        return "certificateSettings";
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("certificate/certificate.xml");
    }

    public void updateCertificateDetails(AbstractCertificateCommon info) {
        info.setCountry(getSettingValue("csr/country"));
        info.setState(getSettingValue("csr/state"));
        info.setLocality(getSettingValue("csr/locality"));
        info.setOrganization(getSettingValue("csr/organization"));
        info.setOrganizationUnit(getSettingValue("csr/organizationUnit"));
        info.setEmail(getSettingValue("csr/email"));
    }
}
