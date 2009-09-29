/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.parkorbit;

import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.setting.Setting;

public class ParkOrbit extends BackgroundMusic implements NamedObject {

    private String m_name;
    private String m_extension;
    private String m_description;

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public String getExtension() {
        return m_extension;
    }

    public void setExtension(String extension) {
        m_extension = extension;
    }

    public AliasMapping generateAlias(String dnsDomain, String orbitServer) {
        String identity = AliasMapping.createUri(m_extension, dnsDomain);
        String contact = AliasMapping.createUri(m_extension, orbitServer);
        return new AliasMapping(identity, contact);
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxpark/park-orbit.xml");
    }

    public boolean isParkTimeoutEnabled() {
        return (Boolean) getSettingTypedValue("general/enableTimeout");
    }

    public int getParkTimeout() {
        return (Integer) getSettingTypedValue("general/parkTimeout");
    }

    public boolean isMultipleCalls() {
        return (Boolean) getSettingTypedValue("general/multipleCalls");
    }

    public boolean isTransferAllowed() {
        return (Boolean) getSettingTypedValue("general/allowTransfer");
    }

    public String getTransferKey() {
        return (String) getSettingTypedValue("general/transferKey");
    }
}
