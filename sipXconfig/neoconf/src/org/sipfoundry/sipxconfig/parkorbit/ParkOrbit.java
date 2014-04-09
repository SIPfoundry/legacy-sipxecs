/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.parkorbit;

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.commserver.imdb.AliasMapping;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.systemaudit.ConfigChangeType;
import org.sipfoundry.sipxconfig.systemaudit.SystemAuditable;

public class ParkOrbit extends BackgroundMusic implements NamedObject, DeployConfigOnEdit, SystemAuditable {

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

    @Deprecated
    public AliasMapping generateAlias(String dnsDomain, String orbitServer) {
        String identity = AliasMapping.createUri(m_extension, dnsDomain);
        String contact = AliasMapping.createUri(m_extension, orbitServer);
        return new AliasMapping(identity, contact, "orbit");
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

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) ParkOrbitContext.FEATURE);
    }

    @Override
    public String getEntityIdentifier() {
        return getName();
    }

    @Override
    public ConfigChangeType getConfigChangeType() {
        return ConfigChangeType.CALL_PARK;
    }
}
