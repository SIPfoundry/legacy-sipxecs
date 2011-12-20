/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.presence;


import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.alias.AliasManager;
import org.sipfoundry.sipxconfig.alias.AliasOwner;
import org.sipfoundry.sipxconfig.common.BeanId;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.springframework.beans.factory.annotation.Required;

/**
 * Even though presence server can be on a specific location, there can only be one of them so
 * settings are not location specific.  Therefore we extend BeanWithSettings and not BeanWithLocation
 */
public class PresenceSettings extends BeanWithSettings implements AliasOwner {
    public static final String PRESENCE_SIGN_IN_CODE = "presence-config/SIP_PRESENCE_SIGN_IN_CODE";
    public static final String PRESENCE_SIGN_OUT_CODE = "presence-config/SIP_PRESENCE_SIGN_OUT_CODE";
    private AliasManager m_aliasManager;

    public void assignAvailableAliases() {
        String signIn = m_aliasManager.getNextAvailableNumericBasedAlias(getSignInCode());
        setSettingValue(PRESENCE_SIGN_IN_CODE, signIn);
        String signOut = m_aliasManager.getNextAvailableNumericBasedAlias(getSignOutCode());
        setSettingValue(PRESENCE_SIGN_OUT_CODE, signOut);
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxpresence/sipxpresence.xml");
    }

    public String getSignInCode() {
        return getSettingValue(PRESENCE_SIGN_IN_CODE);
    }

    public String getSignOutCode() {
        return getSettingValue(PRESENCE_SIGN_OUT_CODE);
    }

    public int getSipTcpPort() {
        return (Integer) getSettingTypedValue("presence-config/SIP_PRESENCE_TCP_PORT");
    }

    public int getSipUdpPort() {
        return (Integer) getSettingTypedValue("presence-config/SIP_PRESENCE_UDP_PORT");
    }

    public int getApiPort() {
        return (Integer) getSettingTypedValue("presence-config/SIP_PRESENCE_HTTP_PORT");
    }

    @Override
    public boolean isAliasInUse(String alias) {
        return alias.equals(getSignInCode()) || alias.equals(getSignOutCode());
    }

    @Override
    public Collection getBeanIdsOfObjectsWithAlias(String alias) {
        return BeanId.createBeanIdCollection(Collections.singleton(getId()), PresenceSettings.class);
    }

    @Required
    public void setAliasManager(AliasManager aliasManager) {
        m_aliasManager = aliasManager;
    }
}
