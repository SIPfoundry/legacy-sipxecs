/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acccode;

import java.util.Collection;
import java.util.Collections;
import java.util.Set;

import org.sipfoundry.sipxconfig.alias.AliasOwner;
import org.sipfoundry.sipxconfig.common.BeanId;

public class AuthCodeAliases implements AliasOwner {
    private AuthCodes m_authCodes;

    @Override
    public Collection getBeanIdsOfObjectsWithAlias(String alias) {
        if (!m_authCodes.isEnabled()) {
            return Collections.emptyList();
        }
        AuthCodeSettings settings = m_authCodes.getSettings();
        Collection ids = Collections.emptyList();
        if (settings != null) {
            Set<String> aliases = settings.getAliasesAsSet();
            aliases.add(settings.getAuthCodeAliases());
            for (String serviceAlias : aliases) {
                if (serviceAlias.equals(alias)) {
                    ids = BeanId.createBeanIdCollection(Collections.singletonList(settings.getId()),
                            AuthCodeSettings.class);
                    break;
                }
            }
        }
        return ids;
    }

    @Override
    public boolean isAliasInUse(String alias) {
        AuthCodeSettings settings = m_authCodes.getSettings();
        Set<String> aliases = settings.getAliasesAsSet();
        aliases.add(settings.getAuthCodeAliases());
        for (String serviceAlias : aliases) {
            if (serviceAlias.equals(alias)) {
                return true;
            }
        }
        return false;
    }

    public void setAuthCodeManager(AuthCodes authCodes) {
        m_authCodes = authCodes;
    }
}
