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
package org.sipfoundry.sipxconfig.presence;


import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.alias.AliasManager;
import org.sipfoundry.sipxconfig.alias.AliasOwner;
import org.sipfoundry.sipxconfig.common.BeanId;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.springframework.beans.factory.annotation.Required;

/**
 * Even though presence server can be on a specific location, there can only be one of them so
 * settings are not location specific.  Therefore we extend BeanWithSettings and not BeanWithLocation
 */
public class PresenceSettings extends PersistableSettings implements AliasOwner {
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

    @Override
    public String getBeanId() {
        return "presenceSettings";
    }
}
