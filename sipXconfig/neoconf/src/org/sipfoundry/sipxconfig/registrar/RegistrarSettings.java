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

import static java.lang.String.format;

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.alias.AliasManager;
import org.sipfoundry.sipxconfig.alias.AliasOwner;
import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.BeanId;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.im.ImManager;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingImpl;
import org.springframework.beans.factory.annotation.Required;

public class RegistrarSettings extends BeanWithSettings implements DeployConfigOnEdit, AliasOwner {
    public static final int SIP_PORT = 5070; // not configurable at this time
    public static final int MONITOR_PORT = 9096; // not configurable at this time
    private static final String PICKUP_CODE = "call-pick-up/SIP_REDIRECT.100-PICKUP.DIRECTED_CALL_PICKUP_CODE";
    private static final String RETRIEVE_CODE = "call-pick-up/SIP_REDIRECT.100-PICKUP.CALL_RETRIEVE_CODE";
    private AliasManager m_aliasManager;
    private AddressManager m_addressManager;
    private DomainManager m_domainManager;

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

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxregistrar.xml");
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

    public Setting getInternalSettings(Location location) {
        SettingImpl internal = new SettingImpl();
        Address imXmlRpc = m_addressManager.getSingleAddress(ImManager.XMLRPC_ADDRESS);
        String openfireUrl = format("http://%s:%d/plugins/sipx-openfire-presence/status", imXmlRpc.getAddress(),
                imXmlRpc.getPort());
        internal.addSetting(newSetting("SIP_REDIRECT.900-PRESENCE.OPENFIRE_PRESENCE_SERVER_URL", openfireUrl));
        String presenceMonitorUrl = format("http://%s:%d/RPC2", location.getAddress(), MONITOR_PORT);
        internal.addSetting(newSetting("SIP_REDIRECT.900-PRESENCE.LOCAL_PRESENCE_MONITOR_SERVER_URL",
                presenceMonitorUrl));
        internal.addSetting(newSetting("SIP_REDIRECT.900-PRESENCE.REALM", m_domainManager.getAuthorizationRealm()));
        internal.addSetting(newSetting("SIP_REDIRECT.900-PRESENCE.SIP_DOMAIN", m_domainManager.getDomain()));
        return internal;
    }

    Setting newSetting(String name, Object value) {
        Setting s = new SettingImpl(name);
        s.setValue(value.toString());
        return s;
    }
}
