/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.registrar;

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.alias.AliasManager;
import org.sipfoundry.sipxconfig.alias.AliasOwner;
import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.BeanId;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.springframework.beans.factory.annotation.Required;

public class RegistrarSettings extends PersistableSettings implements DeployConfigOnEdit, AliasOwner {
    private static final String PICKUP_CODE = "call-pick-up/SIP_REDIRECT.100-PICKUP.DIRECTED_CALL_PICKUP_CODE";
    private static final String RETRIEVE_CODE = "call-pick-up/SIP_REDIRECT.100-PICKUP.CALL_RETRIEVE_CODE";
    private AliasManager m_aliasManager;

    public void assignAvailableAliases() {
        String pickup = m_aliasManager.getNextAvailableNumericBasedAlias(getDirectedCallPickupCode());
        setSettingValue(PICKUP_CODE, pickup);
        String retrieve = m_aliasManager.getNextAvailableNumericBasedAlias(getCallRetrieveCode());
        setSettingValue(RETRIEVE_CODE, retrieve);
    }

    public String getDirectedCallPickupCode() {
        return getSettingValue(PICKUP_CODE);
    }

    public String getCallRetrieveCode() {
        return getSettingValue(RETRIEVE_CODE);
    }

    public int getXmlRpcPort() {
        return (Integer) getSettingTypedValue("registrar-config/SIP_REGISTRAR_XMLRPC_PORT");
    }

    public int getSipTcpPort() {
        return (Integer) getSettingTypedValue("registrar-config/SIP_REGISTRAR_TCP_PORT");
    }

    public int getSipUdpPort() {
        return (Integer) getSettingTypedValue("registrar-config/SIP_REGISTRAR_UDP_PORT");
    }

    public int getMonitorPort() {
        return (Integer) getSettingTypedValue("registrar-config/SIP_REGISTRAR_REG_EVENT_PORT");
    }

    public int getPresencePort() {
        return (Integer) getSettingTypedValue("registrar-config/_SIP_REGISTRAR_PRESENCE_PORT");
    }

    public boolean isEarlyAliasResolutionEnabled() {
        return (Boolean) getSettingTypedValue("registrar-config/SIP_REGISTRAR_EARLY_ALIAS_RESOLUTION");
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxregistrar/sipxregistrar.xml");
    }

    @Required
    public void setAliasManager(AliasManager aliasManager) {
        m_aliasManager = aliasManager;
    }

    @Override
    public boolean isAliasInUse(String alias) {
        return alias.equals(getDirectedCallPickupCode()) || alias.equals(getCallRetrieveCode());
    }

    @Override
    public Collection getBeanIdsOfObjectsWithAlias(String alias) {
        return BeanId.createBeanIdCollection(Collections.singleton(getId()), this.getClass());
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) Registrar.FEATURE);
    }

    @Override
    public String getBeanId() {
        return "registrarSettings";
    }
}
